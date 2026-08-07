/* Copyright (c) 2026 Otto Link. Distributed under the terms of the GNU Lesser
 * General Public License. The full license is in the file LICENSE, distributed
 * with this software. */
#include <functional>

#include <QColorDialog>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QMouseEvent>
#include <QPainter>
#include <QRegularExpression>

#include "meta_qt/widgets/industrial/h_color.hpp"

namespace meta::qt
{

namespace
{

constexpr int kSwatchW = 46;
constexpr int kRowH = kFieldHeight + 4;
constexpr int kTextPad = 8;

/// "#RRGGBBAA", the form the field shows and accepts.
QString to_hex(const QColor &c)
{
  return QString("#%1%2%3%4")
      .arg(c.red(), 2, 16, QChar('0'))
      .arg(c.green(), 2, 16, QChar('0'))
      .arg(c.blue(), 2, 16, QChar('0'))
      .arg(c.alpha(), 2, 16, QChar('0'))
      .toUpper();
}

/// Accepts #RGB, #RRGGBB and #RRGGBBAA, with or without the hash. Returns an
/// invalid colour when the text is not a colour, so the caller can keep the
/// previous value rather than blanking it.
QColor from_hex(const QString &text)
{
  QString s = text.trimmed();
  if (s.startsWith('#'))
    s = s.mid(1);

  static const QRegularExpression re("^[0-9a-fA-F]+$");
  if (!re.match(s).hasMatch())
    return {};

  if (s.size() == 8)
  {
    // QColor's #AARRGGBB ordering is not the RRGGBBAA people paste, so the
    // alpha is peeled off and applied separately.
    QColor c(QStringLiteral("#") + s.left(6));
    if (!c.isValid())
      return {};
    c.setAlpha(s.mid(6, 2).toInt(nullptr, 16));
    return c;
  }

  if (s.size() == 6 || s.size() == 3)
    return QColor(QStringLiteral("#") + s);

  return {};
}

} // namespace

// ---------------------------------------------------------------------------
// ColorSwatch
// ---------------------------------------------------------------------------

class ColorSwatch : public QWidget
{
public:
  explicit ColorSwatch(QWidget *parent = nullptr) : QWidget(parent)
  {
    this->setFixedSize(kSwatchW, kRowH);
    this->setCursor(Qt::PointingHandCursor);
    this->setAttribute(Qt::WA_Hover, true);
    this->setAttribute(Qt::WA_OpaquePaintEvent, true);
  }

  void set_color(const QColor &c)
  {
    this->color_ = c;
    this->update();
  }

  std::function<void()> on_click;

protected:
  void paintEvent(QPaintEvent *) override
  {
    QPainter    p(this);
    const QRect r = this->rect().adjusted(0, 0, -1, -1);

    paint_checkerboard(p, this->rect());
    p.fillRect(this->rect(), this->color_);

    p.setPen(this->hovered_ ? kFieldBorderHover : kFieldBorder);
    p.setBrush(Qt::NoBrush);
    p.drawRect(r);
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
    if (e->button() == Qt::LeftButton && this->on_click &&
        this->rect().contains(e->pos()))
      this->on_click();
  }

private:
  QColor color_{Qt::white};
  bool   hovered_ = false;
};

// ---------------------------------------------------------------------------
// HexField
// ---------------------------------------------------------------------------

class HexField : public QWidget
{
public:
  explicit HexField(QWidget *parent = nullptr) : QWidget(parent)
  {
    this->setFixedHeight(kRowH);
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    this->setCursor(Qt::IBeamCursor);
    this->setAttribute(Qt::WA_Hover, true);
    this->setAttribute(Qt::WA_OpaquePaintEvent, true);
  }

  void set_text(const QString &t)
  {
    this->text_ = t;
    this->update();
  }

  std::function<void(const QString &)> on_committed;

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
    f.setFamily(mono_family());
    f.setPixelSize(kValuePx);
    p.setFont(f);
    p.setPen(kInkPrimary);
    p.drawText(r.adjusted(kTextPad, 0, -kTextPad, 0),
               Qt::AlignVCenter | Qt::AlignLeft,
               this->text_);
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

    auto *ed = new QLineEdit(this->text_, this);
    this->editor_ = ed;
    ed->setGeometry(this->rect().adjusted(1, 1, -1, -1));
    ed->setStyleSheet(QString("background-color: %1; color: %2;"
                              " border: 1px solid %3; padding-left: %4px;"
                              " font-family: '%5';")
                          .arg(kFieldEditing.name())
                          .arg(kInkModified.name())
                          .arg(kAccent.name())
                          .arg(kTextPad - 1)
                          .arg(mono_family()));
    ed->show();
    ed->setFocus();
    ed->selectAll();

    QObject::connect(ed, &QLineEdit::editingFinished, ed, [this, ed]()
                     { this->commit(ed->text()); });
  }

  void commit(const QString &text)
  {
    if (!this->editor_)
      return;

    QLineEdit *ed = this->editor_;
    this->editor_ = nullptr;
    ed->deleteLater();

    this->update();

    if (this->on_committed)
      this->on_committed(text);
  }

  QString    text_;
  bool       hovered_ = false;
  QLineEdit *editor_ = nullptr;
};

// ---------------------------------------------------------------------------
// HColor
// ---------------------------------------------------------------------------

HColor::HColor(QWidget *parent) : QWidget(parent)
{
  auto *row = new QHBoxLayout(this);
  row->setContentsMargins(0, 0, 0, 0);
  row->setSpacing(6);

  this->swatch_ = new ColorSwatch(this);
  this->field_ = new HexField(this);

  row->addWidget(this->swatch_);
  row->addWidget(this->field_, 1);

  this->swatch_->on_click = [this]()
  {
    const QColor picked = QColorDialog::getColor(this->color_,
                                                 this,
                                                 "Colour",
                                                 QColorDialog::ShowAlphaChannel);
    if (picked.isValid())
    {
      this->apply(picked);
      Q_EMIT this->value_changed();
      Q_EMIT this->edit_ended();
    }
  };

  this->field_->on_committed = [this](const QString &text)
  {
    const QColor parsed = from_hex(text);

    // Unparseable text restores the current colour rather than blanking it -
    // a typo should not silently turn the colour black.
    if (!parsed.isValid())
    {
      this->field_->set_text(to_hex(this->color_));
      return;
    }

    if (parsed == this->color_)
      return;

    this->apply(parsed);
    Q_EMIT this->value_changed();
    Q_EMIT this->edit_ended();
  };

  this->apply(this->color_);
}

void HColor::apply(const QColor &color)
{
  this->color_ = color;
  this->swatch_->set_color(color);
  this->field_->set_text(to_hex(color));
}

void HColor::set_color(const QColor &color)
{
  if (color.isValid())
    this->apply(color);
}

QColor HColor::color() const { return this->color_; }

} // namespace meta::qt
