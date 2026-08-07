/* Copyright (c) 2026 Otto Link. Distributed under the terms of the GNU Lesser
 * General Public License. The full license is in the file LICENSE, distributed
 * with this software. */
#include <algorithm>
#include <cmath>
#include <functional>

#include <QFile>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QLinearGradient>
#include <QMouseEvent>
#include <QPainter>
#include <QRandomGenerator>
#include <QRegularExpression>
#include <QTextStream>
#include <QVBoxLayout>
#include <QWheelEvent>

#include "meta_qt/widgets/industrial/h_path.hpp"
#include "meta_qt/widgets/industrial/mod_button.hpp"

namespace meta::qt
{

namespace
{

constexpr int kPointRadius = 5;
constexpr int kHitSlop = 4;
constexpr int kColorbarW = 90;
constexpr int kColorbarH = 8;
constexpr int kPadInset = 8;

/// Height follows width so the pad stays usable from a narrow pane to a wide
/// docked panel. The reference is 200px at roughly 430 wide.
constexpr double kAspect = 0.47;
constexpr int    kPadMinH = 150;
constexpr int    kPadMaxH = 260;

/// The value ramp, shared by the point fills and the colorbar legend so a
/// point's colour can actually be read off the bar.
QColor value_color(double t)
{
  static const struct
  {
    double pos;
    QColor color;
  } ramp[] = {{0.00, QColor("#2040d0")},
              {0.35, QColor("#30c060")},
              {0.65, QColor("#d0d030")},
              {1.00, QColor("#d03020")}};

  t = std::clamp(t, 0.0, 1.0);

  for (int i = 1; i < 4; ++i)
    if (t <= ramp[i].pos)
    {
      const double span = ramp[i].pos - ramp[i - 1].pos;
      const double k = (span > 0.0) ? (t - ramp[i - 1].pos) / span : 0.0;
      const QColor &a = ramp[i - 1].color;
      const QColor &b = ramp[i].color;
      return QColor::fromRgbF(a.redF() + (b.redF() - a.redF()) * k,
                              a.greenF() + (b.greenF() - a.greenF()) * k,
                              a.blueF() + (b.blueF() - a.blueF()) * k);
    }

  return ramp[3].color;
}

} // namespace

// ---------------------------------------------------------------------------
// PathCanvas
// ---------------------------------------------------------------------------

/// The pad itself. No Q_OBJECT: it reports through std::function, so it needs
/// no moc pass of its own.
class PathCanvas : public QWidget
{
public:
  explicit PathCanvas(QWidget *parent = nullptr) : QWidget(parent)
  {
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    this->setMinimumHeight(kPadMinH);
    this->setCursor(Qt::CrossCursor);
    this->setMouseTracking(true);
    // paintEvent covers the whole rect, so Qt need not clear it first
    this->setAttribute(Qt::WA_OpaquePaintEvent, true);
  }

  void set_points(const QVector<PathPoint> &pts)
  {
    if (this->dragging_)
      return; // never re-seat the vector under a live drag

    this->points_ = pts;
    if (this->selected_ >= this->points_.size())
      this->selected_ = -1;
    this->update();
  }

  const QVector<PathPoint> &points() const { return this->points_; }

  void set_mode(HPath::Mode m)
  {
    this->mode_ = m;
    this->update();
  }

  void set_closed(bool c)
  {
    this->closed_ = c;
    this->update();
  }

  void set_bounds(double min_x, double max_x, double min_y, double max_y)
  {
    this->min_x_ = min_x;
    this->max_x_ = max_x;
    this->min_y_ = min_y;
    this->max_y_ = max_y;
    this->update();
  }

  void set_z_step(double s) { this->z_step_ = (s > 0.0) ? s : 0.05; }

  std::function<void()> on_changed;
  std::function<void()> on_committed;

protected:
  void resizeEvent(QResizeEvent *e) override
  {
    QWidget::resizeEvent(e);

    // Only a width change alters the derived height. Reacting to a
    // height-only resize would call setFixedHeight from inside the layout
    // pass that just resized us, making one pass run several times.
    if (e->oldSize().width() == e->size().width())
      return;

    const int h = std::clamp(static_cast<int>(std::lround(this->width() * kAspect)),
                             kPadMinH,
                             kPadMaxH);
    if (h != this->height())
      this->setFixedHeight(h);
  }

  void paintEvent(QPaintEvent *) override
  {
    QPainter p(this);
    const QRect r = this->rect();

    p.fillRect(r, kPadSurface);

    // --- dashed grid, three interior lines each way

    QPen grid(kPadGrid, 1, Qt::CustomDashLine);
    grid.setDashPattern({3, 4});
    p.setPen(grid);

    for (int i = 1; i < 4; ++i)
    {
      const int x = r.left() + r.width() * i / 4;
      const int y = r.top() + r.height() * i / 4;
      p.drawLine(x, r.top(), x, r.bottom());
      p.drawLine(r.left(), y, r.right(), y);
    }

    // --- path segments, drawn under the points

    if (this->mode_ == HPath::Mode::Path && this->points_.size() > 1)
    {
      p.setRenderHint(QPainter::Antialiasing, true);
      p.setPen(QPen(kInkDim, 1));

      for (int i = 1; i < this->points_.size(); ++i)
        p.drawLine(this->to_widget(this->points_[i - 1]),
                   this->to_widget(this->points_[i]));

      if (this->closed_)
        p.drawLine(this->to_widget(this->points_.back()),
                   this->to_widget(this->points_.front()));
    }

    // --- points, coloured by their value

    p.setRenderHint(QPainter::Antialiasing, true);

    for (int i = 0; i < this->points_.size(); ++i)
    {
      const QPointF c = this->to_widget(this->points_[i]);
      const bool    sel = (i == this->selected_);

      p.setPen(QPen(sel ? kAccent : kThumbBorder, 1));
      p.setBrush(value_color(this->points_[i].z));
      p.drawEllipse(c, kPointRadius, kPointRadius);
    }

    p.setRenderHint(QPainter::Antialiasing, false);

    // --- readout

    QFont f = this->font();
    f.setFamily(mono_family());
    f.setPixelSize(kIndexPx);
    p.setFont(f);
    p.setPen(kInkDefault);
    p.drawText(QRect(r.left() + kPadInset,
                     r.bottom() - 22,
                     140,
                     16),
               Qt::AlignLeft | Qt::AlignVCenter,
               QString("%1 pts").arg(this->points_.size()));

    // --- colorbar legend

    const QRect bar(r.right() - kPadInset - kColorbarW,
                    r.bottom() - kPadInset - kColorbarH,
                    kColorbarW,
                    kColorbarH);

    QLinearGradient g(bar.topLeft(), bar.topRight());
    g.setColorAt(0.00, value_color(0.00));
    g.setColorAt(0.35, value_color(0.35));
    g.setColorAt(0.65, value_color(0.65));
    g.setColorAt(1.00, value_color(1.00));
    p.fillRect(bar, g);

    p.setPen(kHairline);
    p.setBrush(Qt::NoBrush);
    p.drawRect(bar.adjusted(0, 0, -1, -1));

    // --- pad border

    p.setPen(kPadBorder);
    p.drawRect(r.adjusted(0, 0, -1, -1));
  }

  void mousePressEvent(QMouseEvent *e) override
  {
    const int hit = this->hit_test(e->pos());

    if (e->button() == Qt::RightButton)
    {
      if (hit >= 0)
      {
        this->points_.remove(hit);
        this->selected_ = -1;
        this->update();
        this->fire(this->on_changed);
        this->fire(this->on_committed);
      }
      return;
    }

    if (e->button() != Qt::LeftButton)
      return;

    if (hit >= 0)
    {
      this->selected_ = hit;
      this->dragging_ = true;
    }
    else
    {
      // click on empty pad adds a point there
      this->points_.push_back(this->from_widget(e->pos()));
      this->selected_ = this->points_.size() - 1;
      this->dragging_ = true;
      this->fire(this->on_changed);
    }

    this->update();
  }

  void mouseMoveEvent(QMouseEvent *e) override
  {
    if (!this->dragging_ || this->selected_ < 0 ||
        this->selected_ >= this->points_.size())
      return;

    const double z = this->points_[this->selected_].z;
    this->points_[this->selected_] = this->from_widget(e->pos());
    this->points_[this->selected_].z = z; // dragging moves, it does not revalue

    this->update();
    this->fire(this->on_changed);
  }

  void mouseReleaseEvent(QMouseEvent *e) override
  {
    if (e->button() != Qt::LeftButton || !this->dragging_)
      return;

    this->dragging_ = false;
    this->fire(this->on_committed);
  }

  /// Wheel over a point changes its value, which is what the colorbar reads.
  void wheelEvent(QWheelEvent *e) override
  {
    const int hit = this->hit_test(e->position().toPoint());
    if (hit < 0)
    {
      e->ignore();
      return;
    }

    const double delta = (e->angleDelta().y() > 0 ? 1.0 : -1.0) * this->z_step_;
    this->points_[hit].z = std::clamp(this->points_[hit].z + delta, 0.0, 1.0);

    this->selected_ = hit;
    this->update();
    this->fire(this->on_changed);
    this->fire(this->on_committed);
    e->accept();
  }

private:
  QPointF to_widget(const PathPoint &p) const
  {
    const double u = (this->max_x_ > this->min_x_)
                         ? (p.x - this->min_x_) / (this->max_x_ - this->min_x_)
                         : 0.0;
    const double v = (this->max_y_ > this->min_y_)
                         ? (p.y - this->min_y_) / (this->max_y_ - this->min_y_)
                         : 0.0;

    // y is flipped: the model's origin is bottom-left, the widget's is top-left
    return QPointF(u * (this->width() - 1), (1.0 - v) * (this->height() - 1));
  }

  PathPoint from_widget(const QPoint &pt) const
  {
    const double u = std::clamp(static_cast<double>(pt.x()) / std::max(1, this->width() - 1),
                                0.0,
                                1.0);
    const double v = std::clamp(static_cast<double>(pt.y()) / std::max(1, this->height() - 1),
                                0.0,
                                1.0);

    PathPoint p;
    p.x = this->min_x_ + u * (this->max_x_ - this->min_x_);
    p.y = this->min_y_ + (1.0 - v) * (this->max_y_ - this->min_y_);
    p.z = 1.0;
    return p;
  }

  int hit_test(const QPoint &pt) const
  {
    // reverse so the point drawn last (on top) wins an overlap
    for (int i = this->points_.size() - 1; i >= 0; --i)
    {
      const QPointF c = this->to_widget(this->points_[i]);
      const double  dx = c.x() - pt.x();
      const double  dy = c.y() - pt.y();
      const double  r = kPointRadius + kHitSlop;
      if (dx * dx + dy * dy <= r * r)
        return i;
    }
    return -1;
  }

  static void fire(const std::function<void()> &fn)
  {
    if (fn)
      fn();
  }

  QVector<PathPoint> points_;
  HPath::Mode        mode_ = HPath::Mode::Path;
  bool               closed_ = false;
  int                selected_ = -1;
  bool               dragging_ = false;
  double             min_x_ = 0.0, max_x_ = 1.0, min_y_ = 0.0, max_y_ = 1.0;
  double             z_step_ = 0.05;
};

// ---------------------------------------------------------------------------
// HPath
// ---------------------------------------------------------------------------

HPath::HPath(QWidget *parent) : QWidget(parent)
{
  auto *box = new QVBoxLayout(this);
  box->setContentsMargins(0, 0, 0, 0);
  box->setSpacing(8);

  this->canvas_ = new PathCanvas(this);
  box->addWidget(this->canvas_);

  auto *row = new QWidget(this);
  auto *h = new QHBoxLayout(row);
  h->setContentsMargins(0, 0, 0, 0);
  h->setSpacing(6);

  this->clear_btn_ = new ModButton("Clear", false, kAccent, row);
  this->random_btn_ = new ModButton("Randomize", false, kAccent, row);
  this->csv_btn_ = new ModButton("From CSV...", false, kAccent, row);

  for (auto *b : {this->clear_btn_, this->random_btn_, this->csv_btn_})
  {
    b->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    h->addWidget(b);
  }

  box->addWidget(row);

  this->canvas_->on_changed = [this]() { Q_EMIT this->value_changed(); };
  this->canvas_->on_committed = [this]() { Q_EMIT this->edit_ended(); };

  this->connect(this->clear_btn_,
                &ModButton::clicked,
                this,
                [this]()
                {
                  this->canvas_->set_points({});
                  Q_EMIT this->value_changed();
                  Q_EMIT this->edit_ended();
                });

  this->connect(this->random_btn_,
                &ModButton::clicked,
                this,
                [this]()
                {
                  // keep the current count so randomize rerolls the shape
                  // rather than resetting how many points there are
                  const int n = std::max(4,
                                         static_cast<int>(
                                             this->canvas_->points().size()));

                  QVector<PathPoint> pts;
                  pts.reserve(n);
                  for (int i = 0; i < n; ++i)
                    pts.push_back({QRandomGenerator::global()->generateDouble(),
                                   QRandomGenerator::global()->generateDouble(),
                                   1.0});

                  this->canvas_->set_points(pts);
                  Q_EMIT this->value_changed();
                  Q_EMIT this->edit_ended();
                });

  this->connect(this->csv_btn_,
                &ModButton::clicked,
                this,
                [this]()
                {
                  const QString fname = QFileDialog::getOpenFileName(
                      this,
                      "Load points from CSV",
                      QString(),
                      "Comma-separated values (*.csv);;All files (*)");

                  if (fname.isEmpty())
                    return;

                  QFile file(fname);
                  if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
                    return;

                  QVector<PathPoint> pts;
                  QTextStream        in(&file);

                  while (!in.atEnd())
                  {
                    const QString line = in.readLine().trimmed();
                    if (line.isEmpty() || line.startsWith('#'))
                      continue;

                    const QStringList f = line.split(QRegularExpression("[,;\\s]+"),
                                                     Qt::SkipEmptyParts);
                    if (f.size() < 2)
                      continue;

                    bool ok_x = false, ok_y = false;
                    const double x = f[0].toDouble(&ok_x);
                    const double y = f[1].toDouble(&ok_y);
                    if (!ok_x || !ok_y)
                      continue; // a header row lands here and is skipped

                    const double z = (f.size() > 2) ? f[2].toDouble() : 1.0;
                    pts.push_back({x, y, z});
                  }

                  if (pts.isEmpty())
                    return;

                  this->canvas_->set_points(pts);
                  Q_EMIT this->value_changed();
                  Q_EMIT this->edit_ended();
                });
}

void HPath::set_mode(Mode mode) { this->canvas_->set_mode(mode); }

void HPath::set_closed(bool closed) { this->canvas_->set_closed(closed); }

void HPath::set_bounds(double min_x, double max_x, double min_y, double max_y)
{
  this->canvas_->set_bounds(min_x, max_x, min_y, max_y);
}

void HPath::set_z_step(double step) { this->canvas_->set_z_step(step); }

void HPath::set_points(const QVector<PathPoint> &points)
{
  this->canvas_->set_points(points);
}

QVector<PathPoint> HPath::points() const { return this->canvas_->points(); }

} // namespace meta::qt
