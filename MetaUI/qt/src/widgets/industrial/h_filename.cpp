/* Copyright (c) 2026 Otto Link. Distributed under the terms of the GNU Lesser
 * General Public License. The full license is in the file LICENSE, distributed
 * with this software. */
#include <functional>

#include <QFileDialog>
#include <QFileInfo>
#include <QFocusEvent>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLineEdit>
#include <QMouseEvent>
#include <QPainter>

#include "meta_qt/widgets/industrial/h_filename.hpp"
#include "meta_qt/widgets/industrial/mod_button.hpp"

namespace meta::qt
{

namespace
{
constexpr int kFieldPad = 8;
}

// ---------------------------------------------------------------------------
// PathField
// ---------------------------------------------------------------------------

/// Recessed, custom-painted path display that becomes a QLineEdit on click.
/// No Q_OBJECT: reports through std::function.
class PathField : public QWidget
{
public:
  explicit PathField(QWidget *parent = nullptr) : QWidget(parent)
  {
    this->setFixedHeight(kFieldHeight + 4);
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    this->setCursor(Qt::IBeamCursor);
    this->setAttribute(Qt::WA_Hover, true);
  }

  void set_path(const QString &p)
  {
    this->path_ = p;
    this->update();
  }

  const QString &path() const { return this->path_; }

  std::function<void()> on_committed;

protected:
  void paintEvent(QPaintEvent *) override
  {
    QPainter    p(this);
    const QRect r = this->rect().adjusted(0, 0, -1, -1);

    p.fillRect(this->rect(), this->hovered_ ? kFieldHover : kField);

    p.setPen(this->hovered_ ? kFieldBorderHover : kFieldBorder);
    p.setBrush(Qt::NoBrush);
    p.drawRect(r);

    QFont f = this->font();
    f.setPixelSize(kLabelPx);
    p.setFont(f);

    const QRect text_rect = r.adjusted(kFieldPad, 0, -kFieldPad, 0);

    if (this->path_.isEmpty())
    {
      p.setPen(kInkLocked);
      p.drawText(text_rect, Qt::AlignVCenter | Qt::AlignLeft, "not set");
      return;
    }

    // Elide from the left: the filename identifies the file, so the tail is
    // the part that must survive.
    p.setPen(kInkPrimary);
    p.drawText(text_rect,
               Qt::AlignVCenter | Qt::AlignLeft,
               p.fontMetrics().elidedText(this->path_,
                                          Qt::ElideLeft,
                                          text_rect.width()));
  }

  void enterEvent(QEnterEvent *e) override
  {
    this->hovered_ = true;
    this->update();
    QWidget::enterEvent(e);
  }

  void leaveEvent(QEvent *e) override
  {
    this->hovered_ = false;
    this->update();
    QWidget::leaveEvent(e);
  }

  void mouseReleaseEvent(QMouseEvent *e) override
  {
    if (e->button() == Qt::LeftButton)
      this->begin_edit();
  }

private:
  void begin_edit()
  {
    if (this->editor_)
      return;

    auto *ed = new QLineEdit(this->path_, this);
    this->editor_ = ed;
    ed->setGeometry(this->rect().adjusted(1, 1, -1, -1));
    ed->setStyleSheet(QString("background-color: %1; color: %2;"
                              " border: 1px solid %3; padding-left: %4px;")
                          .arg(kFieldEditing.name())
                          .arg(kInkModified.name())
                          .arg(kAccent.name())
                          .arg(kFieldPad - 1));
    ed->show();
    ed->setFocus();
    ed->selectAll();

    QObject::connect(ed, &QLineEdit::editingFinished, ed, [this, ed]()
                     { this->commit_edit(ed->text()); });

    QObject::connect(ed, &QLineEdit::returnPressed, ed, [this, ed]()
                     { this->commit_edit(ed->text()); });
  }

  void commit_edit(const QString &text)
  {
    if (!this->editor_)
      return;

    QLineEdit *ed = this->editor_;
    this->editor_ = nullptr;
    ed->deleteLater();

    const bool changed = (text != this->path_);
    this->path_ = text;
    this->update();

    if (changed && this->on_committed)
      this->on_committed();
  }

  QString    path_;
  bool       hovered_ = false;
  QLineEdit *editor_ = nullptr;
};

// ---------------------------------------------------------------------------
// HFilename
// ---------------------------------------------------------------------------

HFilename::HFilename(QWidget *parent) : QWidget(parent)
{
  auto *row = new QHBoxLayout(this);
  row->setContentsMargins(0, 0, 0, 0);
  row->setSpacing(6);

  this->field_ = new PathField(this);
  this->browse_btn_ = new ModButton("Browse...", false, kAccent, this);

  row->addWidget(this->field_, 1);
  row->addWidget(this->browse_btn_);

  this->field_->on_committed = [this]()
  {
    Q_EMIT this->value_changed();
    Q_EMIT this->edit_ended();
  };

  this->connect(this->browse_btn_,
                &ModButton::clicked,
                this,
                [this]() { this->browse(); });
}

void HFilename::browse()
{
  const QString current = this->field_->path();
  const QString start = current.isEmpty() ? QString()
                                          : QFileInfo(current).absolutePath();

  QString picked;

  switch (this->mode_)
  {
  case Mode::Save:
    picked = QFileDialog::getSaveFileName(this, "Select file", current, this->filter_);
    break;
  case Mode::Directory:
    picked = QFileDialog::getExistingDirectory(this,
                                               "Select folder",
                                               start,
                                               QFileDialog::ShowDirsOnly |
                                                   QFileDialog::DontResolveSymlinks);
    break;
  case Mode::Open:
  default:
    picked = QFileDialog::getOpenFileName(this, "Select file", current, this->filter_);
    break;
  }

  if (picked.isEmpty())
    return;

  this->field_->set_path(picked);
  Q_EMIT this->value_changed();
  Q_EMIT this->edit_ended();
}

void HFilename::set_mode(Mode mode) { this->mode_ = mode; }

void HFilename::set_filter(const QString &filter) { this->filter_ = filter; }

void HFilename::set_path(const QString &path) { this->field_->set_path(path); }

QString HFilename::path() const { return this->field_->path(); }

} // namespace meta::qt
