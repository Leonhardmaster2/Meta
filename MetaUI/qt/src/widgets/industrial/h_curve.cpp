/* Copyright (c) 2026 Otto Link. Distributed under the terms of the GNU Lesser
 * General Public License. The full license is in the file LICENSE, distributed
 * with this software. */
#include <algorithm>
#include <cmath>
#include <functional>

#include <QHBoxLayout>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QVBoxLayout>

#include "meta_qt/widgets/industrial/h_curve.hpp"
#include "meta_qt/widgets/industrial/mod_button.hpp"

namespace meta::qt
{

namespace
{

constexpr int    kHandleR = 4;
constexpr int    kHitSlop = 6;
constexpr double kAspect = 0.47;
constexpr int    kPadMinH = 140;
constexpr int    kPadMaxH = 240;

} // namespace

// ---------------------------------------------------------------------------
// HCurveCanvas
// ---------------------------------------------------------------------------

/// No Q_OBJECT: reports through std::function, so no moc pass of its own.
class HCurveCanvas : public QWidget
{
public:
  explicit HCurveCanvas(QWidget *parent = nullptr) : QWidget(parent)
  {
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    this->setMinimumHeight(kPadMinH);
    this->setMouseTracking(true);
    this->setAttribute(Qt::WA_OpaquePaintEvent, true);
  }

  void set_values(const QVector<double> &v)
  {
    if (this->dragging_)
      return; // a sync must not re-seat the vector under a live drag

    this->values_ = v;
    if (this->selected_ >= this->values_.size())
      this->selected_ = -1;
    this->update();
  }

  const QVector<double> &values() const { return this->values_; }

  void set_bounds(double min_y, double max_y)
  {
    this->min_y_ = min_y;
    this->max_y_ = max_y;
    this->update();
  }

  std::function<void()> on_changed;
  std::function<void()> on_committed;

protected:
  void resizeEvent(QResizeEvent *e) override
  {
    QWidget::resizeEvent(e);

    // width-only: a height derived from inside the layout pass that just set
    // it makes that pass run repeatedly
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
    QPainter    p(this);
    const QRect r = this->rect();

    p.fillRect(r, kPadSurface);

    // --- dashed grid

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

    if (this->values_.size() >= 2)
    {
      p.setRenderHint(QPainter::Antialiasing, true);

      // --- the curve

      QPainterPath path;
      path.moveTo(this->to_widget(0));
      for (int i = 1; i < this->values_.size(); ++i)
        path.lineTo(this->to_widget(i));

      p.setPen(QPen(kPadCrosshair, 2));
      p.setBrush(Qt::NoBrush);
      p.drawPath(path);

      // --- handles

      for (int i = 0; i < this->values_.size(); ++i)
      {
        const bool sel = (i == this->selected_);
        p.setPen(QPen(sel ? kAccent : kThumbBorder, 1));
        p.setBrush(sel ? kAccent : kInkPrimary);
        p.drawEllipse(this->to_widget(i), kHandleR, kHandleR);
      }

      p.setRenderHint(QPainter::Antialiasing, false);
    }

    // --- readout

    QFont f = this->font();
    f.setFamily(mono_family());
    f.setPixelSize(kIndexPx);
    p.setFont(f);
    p.setPen(kInkDefault);
    p.drawText(QRect(r.left() + 8, r.bottom() - 22, 160, 16),
               Qt::AlignLeft | Qt::AlignVCenter,
               QString("%1 pts").arg(this->values_.size()));

    p.setPen(kPadBorder);
    p.setBrush(Qt::NoBrush);
    p.drawRect(r.adjusted(0, 0, -1, -1));
  }

  void mousePressEvent(QMouseEvent *e) override
  {
    if (e->button() != Qt::LeftButton)
      return;

    const int hit = this->hit_test(e->pos());
    if (hit < 0)
      return;

    this->selected_ = hit;
    this->dragging_ = true;
    this->update();
  }

  void mouseMoveEvent(QMouseEvent *e) override
  {
    if (!this->dragging_ || this->selected_ < 0 ||
        this->selected_ >= this->values_.size())
      return;

    this->values_[this->selected_] = this->value_at(e->pos().y());
    this->update();

    if (this->on_changed)
      this->on_changed();
  }

  void mouseReleaseEvent(QMouseEvent *e) override
  {
    if (e->button() != Qt::LeftButton || !this->dragging_)
      return;

    this->dragging_ = false;
    if (this->on_committed)
      this->on_committed();
  }

private:
  QPointF to_widget(int i) const
  {
    const int n = this->values_.size();
    const double u = (n > 1) ? static_cast<double>(i) / (n - 1) : 0.0;
    const double span = this->max_y_ - this->min_y_;
    const double v = (span > 0.0) ? (this->values_[i] - this->min_y_) / span : 0.0;

    return QPointF(u * (this->width() - 1),
                   (1.0 - std::clamp(v, 0.0, 1.0)) * (this->height() - 1));
  }

  double value_at(int y) const
  {
    const double v = 1.0 - std::clamp(static_cast<double>(y) /
                                          std::max(1, this->height() - 1),
                                      0.0,
                                      1.0);
    return this->min_y_ + v * (this->max_y_ - this->min_y_);
  }

  int hit_test(const QPoint &pt) const
  {
    for (int i = 0; i < this->values_.size(); ++i)
    {
      const QPointF c = this->to_widget(i);
      const double  dx = c.x() - pt.x();
      const double  dy = c.y() - pt.y();
      const double  r = kHandleR + kHitSlop;
      if (dx * dx + dy * dy <= r * r)
        return i;
    }
    return -1;
  }

  QVector<double> values_;
  int             selected_ = -1;
  bool            dragging_ = false;
  double          min_y_ = 0.0;
  double          max_y_ = 1.0;
};

// ---------------------------------------------------------------------------
// HCurve
// ---------------------------------------------------------------------------

HCurve::HCurve(QWidget *parent) : QWidget(parent)
{
  auto *box = new QVBoxLayout(this);
  box->setContentsMargins(0, 0, 0, 0);
  box->setSpacing(8);

  this->canvas_ = new HCurveCanvas(this);
  box->addWidget(this->canvas_);

  auto *row = new QWidget(this);
  auto *h = new QHBoxLayout(row);
  h->setContentsMargins(0, 0, 0, 0);
  h->setSpacing(6);

  this->reset_btn_ = new ModButton("Reset", false, kAccent, row);
  this->reset_btn_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  h->addWidget(this->reset_btn_);
  box->addWidget(row);

  this->canvas_->on_changed = [this]() { Q_EMIT this->value_changed(); };
  this->canvas_->on_committed = [this]() { Q_EMIT this->edit_ended(); };

  this->connect(this->reset_btn_,
                &ModButton::clicked,
                this,
                [this]()
                {
                  // back to the identity ramp, which is what a curve editor
                  // resets to - a flat line would discard the mapping entirely
                  QVector<double> v = this->canvas_->values();
                  const int       n = v.size();
                  for (int i = 0; i < n; ++i)
                    v[i] = (n > 1) ? static_cast<double>(i) / (n - 1) : 0.0;

                  this->canvas_->set_values(v);
                  Q_EMIT this->value_changed();
                  Q_EMIT this->edit_ended();
                });
}

void HCurve::set_bounds(double min_y, double max_y)
{
  this->canvas_->set_bounds(min_y, max_y);
}

void HCurve::set_values(const QVector<double> &values)
{
  this->canvas_->set_values(values);
}

QVector<double> HCurve::values() const { return this->canvas_->values(); }

} // namespace meta::qt
