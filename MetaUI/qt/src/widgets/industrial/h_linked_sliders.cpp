/* Copyright (c) 2026 Otto Link. Distributed under the terms of the GNU Lesser
 * General Public License. The full license is in the file LICENSE, distributed
 * with this software. */
#include <QHBoxLayout>
#include <QVBoxLayout>

#include "meta_qt/widgets/industrial/h_linked_sliders.hpp"
#include "meta_qt/widgets/industrial/mod_button.hpp"
#include "meta_qt/widgets/industrial/param_slider.hpp"

namespace meta::qt
{

HLinkedSliders::HLinkedSliders(QWidget *parent) : QWidget(parent)
{
  auto *row = new QHBoxLayout(this);
  row->setContentsMargins(0, 0, 0, 0);
  row->setSpacing(kSwatchGap);

  auto *rails = new QVBoxLayout();
  rails->setContentsMargins(0, 0, 0, 0);
  rails->setSpacing(kSwatchGap);

  this->x_ = new ParamSlider(this);
  this->y_ = new ParamSlider(this);
  this->x_->set_label("x");
  this->y_->set_label("y");

  rails->addWidget(this->x_);
  rails->addWidget(this->y_);
  row->addLayout(rails, 1);

  // The chip spans both rails, because it describes the pair rather than
  // either one of them.
  this->link_ = new ModButton("=", true, kAccent, this);
  this->link_->setFixedWidth(34);
  this->link_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
  this->link_->setToolTip("Lock x and y together");
  row->addWidget(this->link_);

  auto wire = [this](ParamSlider *from, ParamSlider *to)
  {
    this->connect(from,
                  &ParamSlider::value_changed,
                  this,
                  [this, from, to]()
                  {
                    this->mirror(from, to);
                    Q_EMIT this->value_changed();
                  });

    this->connect(from,
                  &ParamSlider::edit_ended,
                  this,
                  [this, from, to]()
                  {
                    this->mirror(from, to);
                    Q_EMIT this->edit_ended();
                  });
  };

  wire(this->x_, this->y_);
  wire(this->y_, this->x_);

  this->connect(this->link_,
                &ModButton::toggled,
                this,
                [this](bool on)
                {
                  if (!on)
                    return;

                  // Turning the link on adopts x for both, rather than
                  // leaving the pair mismatched while claiming to be locked.
                  this->mirror(this->x_, this->y_);
                  Q_EMIT this->value_changed();
                  Q_EMIT this->edit_ended();
                });
}

void HLinkedSliders::mirror(ParamSlider *from, ParamSlider *to)
{
  if (!this->link_->is_active() || this->mirroring_)
    return;

  // set_value() rather than glide_to(): the followed rail should track the
  // dragged one exactly, not chase it 260ms behind.
  this->mirroring_ = true;
  to->set_value(from->value());
  this->mirroring_ = false;
}

void HLinkedSliders::set_labels(const QString &x_label, const QString &y_label)
{
  this->x_->set_label(x_label);
  this->y_->set_label(y_label);
}

void HLinkedSliders::set_range(double from, double to, double step)
{
  this->x_->set_range(from, to, step);
  this->y_->set_range(from, to, step);
}

void HLinkedSliders::set_decimals(int decimals)
{
  this->x_->set_decimals(decimals);
  this->y_->set_decimals(decimals);
}

void HLinkedSliders::set_accent(const QColor &accent)
{
  this->x_->set_accent(accent);
  this->y_->set_accent(accent);
  this->link_->set_accent(accent);
}

void HLinkedSliders::set_defaults(double x, double y)
{
  this->x_->set_default_value(x);
  this->y_->set_default_value(y);
}

void HLinkedSliders::set_values(double x, double y)
{
  // Guarded: assigning x would otherwise mirror onto y and overwrite the
  // value we are about to assign to it.
  this->mirroring_ = true;
  this->x_->set_value(x);
  this->y_->set_value(y);
  this->mirroring_ = false;
}

void HLinkedSliders::set_linked(bool linked) { this->link_->set_active(linked); }

bool HLinkedSliders::is_linked() const { return this->link_->is_active(); }

double HLinkedSliders::x() const { return this->x_->value(); }

double HLinkedSliders::y() const { return this->y_->value(); }

} // namespace meta::qt
