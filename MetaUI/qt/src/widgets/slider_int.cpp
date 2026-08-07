/* Copyright (c) 2026 Otto Link. Distributed under the terms of the GNU General
   Public License. The full license is in the file LICENSE, distributed with
   this software. */

#include <algorithm>
#include <format>
#include <random>

#include <QFontDatabase>
#include <QFontMetrics>
#include <QHoverEvent>
#include <QLinearGradient>
#include <QMenu>
#include <QPainter>

#include "meta/logger.hpp"

#include "meta_qt/widgets/helpers.hpp"
#include "meta_qt/widgets/slider_int.hpp"

namespace meta::qt
{

namespace
{
// Reference geometry (ParamSliderStacked): label row on top, 24px track zone
// holding a 6px rail with a machined thumb, monospace value box on the right.
constexpr int VALUE_BOX_W = 74;
constexpr int VALUE_BOX_H = 24;
constexpr int BOX_GAP = 12;
constexpr int ZONE_H = 24;
constexpr int RAIL_H = 6;
constexpr int THUMB_W = 10;
constexpr int THUMB_H = 18;
} // namespace

SliderInt::SliderInt(const std::string &label_,
                     int                value_init_,
                     int                vmin_,
                     int                vmax_,
                     bool               add_plus_minus_buttons_,
                     const std::string &value_format_,
                     QWidget           *parent)
    : QWidget(parent),
      value_init(value_init_),
      value(value_init_),
      vmin(vmin_),
      vmax(vmax_),
      add_plus_minus_buttons(add_plus_minus_buttons_),
      value_format(value_format_)
{
  this->label = helpers::truncate_string(label_, this->style.label_max_len());

  this->setMouseTracking(true);
  this->setAttribute(Qt::WA_Hover);
  this->setContextMenuPolicy(Qt::CustomContextMenu);

  this->update_geometry();
  this->connect(this,
                &SliderInt::edit_ended,
                [this]() { this->update_geometry(); });

  // Inline editor style: recessed editing state (matches the reference value
  // box: bg #161616, accent border while editing, monospace digits).
  const QPalette &pal = this->palette();
  const QString   accent = pal.color(QPalette::Highlight).name();
  this->style_sheet =
      "background-color: #161616; color: #ffffff; border: 1px solid " +
      accent.toStdString() +
      "; font-family: 'Consolas','Menlo',monospace;"
      " selection-background-color: " +
      accent.toStdString() + "; selection-color: #161616;";

  this->value_edit = new QLineEdit(this);
  this->value_edit->setVisible(false);
  this->value_edit->setFixedHeight(VALUE_BOX_H - 4);
  this->value_edit->setAlignment(Qt::AlignCenter);
  this->value_edit->setStyleSheet(this->style_sheet.c_str());
  this->connect(this->value_edit,
                &QLineEdit::editingFinished,
                this,
                &SliderInt::apply_text_edit_value);
}

void SliderInt::apply_text_edit_value()
{
  bool ok = false;
  int  new_value = this->value_edit->text().toInt(&ok);
  if (ok && this->set_value(new_value)) Q_EMIT this->edit_ended();

  this->value_edit->setVisible(false);
  this->update();
}

bool SliderInt::event(QEvent *event)
{
  switch (event->type())
  {
  case QEvent::HoverEnter:
    this->is_hovered = true;
    this->update();
    this->setCursor(Qt::SizeHorCursor);
    break;

  case QEvent::HoverLeave:
    this->is_hovered = false;
    this->is_minus_hovered = false;
    this->is_plus_hovered = false;
    this->is_bar_hovered = false;
    this->is_box_hovered = false;
    this->update();
    this->setCursor(Qt::ArrowCursor);
    break;

  case QEvent::HoverMove:
  {
    auto        *hover = static_cast<QHoverEvent *>(event);
    const QPoint pos = hover->position().toPoint();
    this->is_minus_hovered = this->rect_minus.contains(pos);
    this->is_plus_hovered = this->rect_plus.contains(pos);
    this->is_bar_hovered = this->rect_bar.contains(pos);
    this->is_box_hovered = this->rect_box.contains(pos);
    this->update();
    this->setCursor(this->is_bar_hovered   ? Qt::SizeHorCursor
                    : this->is_box_hovered ? Qt::IBeamCursor
                                           : Qt::ArrowCursor);
    break;
  }

  default: break;
  }
  return QWidget::event(event);
}

int SliderInt::get_value() const { return this->value; }

std::string SliderInt::get_value_as_string() const
{
  return std::vformat(this->value_format, std::make_format_args(this->value));
}

void SliderInt::mouseDoubleClickEvent(QMouseEvent *)
{
  const bool is_bounded = this->is_range_bounded();
  const int  delta = is_bounded ? std::max(1,
                                          (this->vmax - this->vmin) /
                                              this->style.button_ticks())
                                : 1;

  if (this->is_bar_hovered || this->is_box_hovered)
  {
    this->value_edit->setText(QString::number(this->value));
    this->value_edit->setGeometry(this->rect_box.adjusted(2, 2, -2, -2));
    this->value_edit->setVisible(true);
    this->value_edit->setFocus();
    this->value_edit->selectAll();
    this->update();
  }
  else if (this->is_minus_hovered)
  {
    if (this->set_value(this->value - delta)) Q_EMIT this->edit_ended();
  }
  else if (this->is_plus_hovered)
  {
    if (this->set_value(this->value + delta)) Q_EMIT this->edit_ended();
  }
}

void SliderInt::mouseMoveEvent(QMouseEvent *event)
{
  if (!this->is_dragging)
  {
    QWidget::mouseMoveEvent(event);
    return;
  }

  // For integer sliders we accumulate sub-integer motion to avoid jitter
  // when the range is large relative to the bar width.
  float ppu;

  if (!this->is_range_bounded())
    ppu = PPU_UNBOUNDED;
  else
    ppu = float(this->rect_bar.width()) / float(this->vmax - this->vmin);

  const Qt::KeyboardModifiers mods = event->modifiers();
  this->force_edit_ended_emit = false;

  if ((mods & Qt::ControlModifier) && (mods & Qt::ShiftModifier))
    this->force_edit_ended_emit = true;
  else if (mods & Qt::ControlModifier)
    ppu *= PPU_MULT_FINE;
  else if (mods & Qt::ShiftModifier)
    ppu /= PPU_MULT_FINE;

  const int dx = event->position().toPoint().x() - this->pos_x_before_dragging;
  const float dv = float(dx) / ppu;

  // Accumulate fractional motion; only commit whole integer steps.
  this->drag_dx = dx;
  this->drag_accumulator = dv;
  const int delta = static_cast<int>(this->drag_accumulator);

  if (delta != 0) this->set_value(this->value_before_dragging + delta);

  this->update(); // the handle moves even between integer steps

  QWidget::mouseMoveEvent(event);
}

void SliderInt::mousePressEvent(QMouseEvent *event)
{
  if (event->button() == Qt::LeftButton)
  {
    const bool is_bounded = this->is_range_bounded();
    const int  delta = is_bounded ? std::max(1,
                                            (this->vmax - this->vmin) /
                                                this->style.button_ticks())
                                  : 1;

    // Clicking the value box enters edit mode (reference interaction).
    if (this->is_box_hovered)
    {
      this->mouseDoubleClickEvent(event);
      return;
    }

    if (this->is_bar_hovered)
    {
      this->value_before_dragging = this->value;
      this->pos_x_before_dragging = event->position().toPoint().x();
      this->drag_accumulator = 0.f;
      this->set_is_dragging(true);
    }
    else if (this->is_minus_hovered)
    {
      if (this->set_value(this->value - delta)) Q_EMIT this->edit_ended();
    }
    else if (this->is_plus_hovered)
    {
      if (this->set_value(this->value + delta)) Q_EMIT this->edit_ended();
    }
  }
}

void SliderInt::mouseReleaseEvent(QMouseEvent *event)
{
  if (this->is_dragging && event->button() == Qt::LeftButton)
  {
    this->set_is_dragging(false);
    if (this->value != this->value_before_dragging) Q_EMIT this->edit_ended();
  }
}

bool SliderInt::is_range_bounded() const
{
  return this->vmin != INT_MIN && this->vmax != INT_MAX &&
         this->vmax > this->vmin;
}

// Handle of an unbounded slider: centred at rest, following the drag while one
// is in progress, clamped so it never leaves the track. Its travel is only an
// affordance - the value keeps changing once the handle hits an end.
QRect SliderInt::handle_rect() const
{
  const int track_w = std::max(this->rect_bar.width() - 2, 2);
  const int handle_w = std::clamp(3 * this->base_dx, 2, track_w);
  const int span = (track_w - handle_w) / 2;

  QRect r = this->rect_bar.adjusted(1, 1, -1, -1);
  r.setWidth(handle_w);
  r.moveLeft(this->rect_bar.left() + 1 + span +
             std::clamp(this->drag_dx, -span, span));

  return r;
}

void SliderInt::paintEvent(QPaintEvent *)
{
  QPainter p(this);
  p.setRenderHint(QPainter::Antialiasing);

  const QPalette &pal = this->palette();
  const QColor    c_well = pal.color(QPalette::Base);      // rail well
  const QColor    c_rail = pal.color(QPalette::Dark);      // rail hairline
  const QColor    c_fill = pal.color(QPalette::Highlight); // accent fill
  const QColor    c_text = pal.color(QPalette::Text);      // default state
  const QColor    c_modified = pal.color(QPalette::BrightText);
  const QColor    c_border = pal.color(QPalette::Mid);     // value-box border

  const bool is_editing = this->value_edit->isVisible();
  const bool modified = this->value != this->value_init;
  const QColor c_label = modified ? c_modified : c_text;

  QFontMetrics fm(this->font());
  const int label_h = fm.height();

  // --- label row (uppercase, state encoded by colour only)
  p.setFont(this->font());
  p.setPen(c_label);
  p.drawText(QRect(0, 0, this->width(), label_h),
             Qt::AlignLeft | Qt::AlignVCenter,
             QString::fromStdString(this->label).toUpper());

  // --- rail well (6px, recessed)
  const QRect zone = this->rect_bar;
  const QRect rail(0,
                   zone.center().y() - RAIL_H / 2,
                   zone.width(),
                   RAIL_H);
  p.setPen(QPen(c_rail, 1));
  p.setBrush(c_well);
  p.drawRoundedRect(rail.adjusted(0, 0, -1, -1), 1, 1);

  // Machined metal thumb shared by the bounded and unbounded variants.
  auto draw_thumb = [&](int x, int cy)
  {
    const QRect t(x, cy - THUMB_H / 2, THUMB_W, THUMB_H);
    QLinearGradient g(t.topLeft(), t.bottomLeft());
    g.setColorAt(0, QColor("#d6d6d6"));
    g.setColorAt(1, QColor("#a8a8a8"));
    p.setPen(QPen(QColor("#1a1a1a"), 1));
    p.setBrush(g);
    p.drawRoundedRect(t.adjusted(0, 0, -1, -1), 2, 2);
    // centre notch
    p.setPen(Qt::NoPen);
    p.setBrush(QColor("#5f5f5f"));
    p.drawRect(t.center().x() - 1, t.center().y() - 4, 2, 8);
  };

  if (!is_editing)
  {
    if (this->is_range_bounded())
    {
      // Accent fill (always the accent, ~0.9 opacity).
      const float r = float(this->value - this->vmin) /
                      float(std::max(1, this->vmax - this->vmin));
      QRect fill = rail.adjusted(1, 1, -1, -1);
      fill.setWidth(std::max(0, int(r * float(fill.width()))));
      QColor fill_c = c_fill;
      fill_c.setAlpha(230);
      p.setPen(Qt::NoPen);
      p.setBrush(fill_c);
      p.drawRoundedRect(fill, 1, 1);

      draw_thumb(int(std::clamp(r, 0.f, 1.f) *
                       float(std::max(zone.width() - THUMB_W, 0))),
                 rail.center().y());
    }
    else
    {
      // Unbounded: drag handle following the motion, centred at rest.
      if (this->is_dragging)
      {
        const int x_mid = zone.center().x();
        p.setPen(QPen(c_border, 1));
        p.drawLine(QPoint(x_mid, zone.top() + 3), QPoint(x_mid, zone.bottom() - 3));
      }
      draw_thumb(this->handle_rect().center().x() - THUMB_W / 2,
                 rail.center().y());
    }
  }

  // --- value box (right side, monospace readout)
  const QRect box = this->rect_box;
  QColor      box_bg = c_well.lighter(115); // ≈ #1f1f1f over the #1c1c1c well
  QColor      box_border = c_border;
  if (is_editing)
    box_border = c_fill;
  else if (this->is_box_hovered)
  {
    box_bg = c_well.lighter(140); // ≈ #262626
    box_border = c_border.lighter(120);
  }
  p.setPen(QPen(box_border, 1));
  p.setBrush(box_bg);
  p.drawRoundedRect(box.adjusted(0, 0, -1, -1), 2, 2);

  if (!is_editing)
  {
    QFont mono = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    mono.setPixelSize(std::max(11, label_h - 3));
    p.setFont(mono);
    p.setPen(c_label);
    p.drawText(box.adjusted(8, 0, -6, 0),
               Qt::AlignLeft | Qt::AlignVCenter,
               QString::fromStdString(this->get_value_as_string()));
  }

  // ◁/▶ step buttons (only when enabled)
  if (this->add_plus_minus_buttons)
  {
    p.setFont(this->font());
    p.setPen(c_text);
    p.drawText(this->rect_minus,
               Qt::AlignCenter | Qt::AlignVCenter,
               this->is_minus_hovered ? "◀" : "◁");
    p.drawText(this->rect_plus,
               Qt::AlignCenter | Qt::AlignVCenter,
               this->is_plus_hovered ? "▶" : "▷");
  }
}

void SliderInt::resizeEvent(QResizeEvent *event)
{
  this->update_geometry();
  QWidget::resizeEvent(event);
}

void SliderInt::set_is_dragging(bool new_state)
{
  this->is_dragging = new_state;
  if (!new_state) this->drag_accumulator = 0.f; // reset on release
  this->drag_dx = 0; // an unbounded slider's handle recentres on release
  this->setCursor(new_state ? Qt::SizeHorCursor : Qt::ArrowCursor);
  this->update();
}

bool SliderInt::set_value(int new_value)
{
  new_value = std::clamp(new_value, this->vmin, this->vmax);

  if (new_value == this->value) return false;

  this->value = new_value;
  this->update();
  Q_EMIT this->value_changed();

  if (this->force_edit_ended_emit) Q_EMIT this->edit_ended();

  return true;
}

QSize SliderInt::sizeHint() const
{
  return QSize(this->slider_width, this->base_dy);
}

void SliderInt::update_geometry()
{
  QFontMetrics fm(this->font());
  this->base_dx = fm.horizontalAdvance(QString("M"));

  // Stacked layout: label row on top, then the 24px track zone; the value box
  // sits at the right of the track zone.
  const int label_h = fm.height();
  const int track_y = label_h + 6;
  this->base_dy = track_y + ZONE_H + 4;

  const int label_w = helpers::text_width(this, this->label);
  this->slider_width = label_w + this->style.horizontal_spacing() +
                       10 * fm.horizontalAdvance(QString("0")) +
                       6 * this->base_dx;

  this->slider_width_min = label_w + this->style.horizontal_spacing() +
                           fm.horizontalAdvance(QString::fromStdString(
                               this->get_value_as_string())) +
                           6 * this->base_dx;

  this->setMinimumWidth(this->slider_width_min);
  this->setMinimumHeight(this->sizeHint().height());
  this->setMaximumHeight(this->sizeHint().height());

  const int gap = this->add_plus_minus_buttons ? 2 * this->base_dx : 0;

  this->rect_bar = QRect(gap,
                         track_y,
                         std::max(this->width() - gap * 2 - BOX_GAP - VALUE_BOX_W,
                                  40),
                         ZONE_H);

  this->rect_box = QRect(this->width() - VALUE_BOX_W, track_y, VALUE_BOX_W, ZONE_H);

  if (this->add_plus_minus_buttons)
  {
    this->rect_minus = QRect(0, track_y, 2 * this->base_dx, ZONE_H);
    this->rect_plus = QRect(this->rect_bar.right() - 2 * this->base_dx + 1,
                            track_y,
                            2 * this->base_dx,
                            ZONE_H);
  }
  else
  {
    this->rect_minus = QRect();
    this->rect_plus = QRect();
  }
}

} // namespace meta::qt
