/* Copyright (c) 2026 Otto Link. Distributed under the terms of the GNU Lesser
 * General Public License. The full license is in the file LICENSE, distributed
 * with this software. */

/**
 * @file h_linked_sliders.hpp
 * @brief Two parameter rails sharing a link toggle, for glm::vec2.
 *
 * The "LinkedSliders" flavour of a vec2: an x rail and a y rail stacked, with
 * a chip that ties them together. Wavenumber is the common case, where the
 * two components are usually meant to stay equal and only occasionally not.
 *
 * Linking is not just a convenience: with it on, dragging either rail moves
 * both, so the isotropic case - which is most of them - stays a one-handed
 * operation.
 */
#pragma once
#include <QWidget>

#include "meta_qt/widgets/industrial/tokens.hpp"

namespace meta::qt
{

class ParamSlider;
class ModButton;

class HLinkedSliders : public QWidget
{
  Q_OBJECT

public:
  explicit HLinkedSliders(QWidget *parent = nullptr);

  void set_labels(const QString &x_label, const QString &y_label);
  void set_range(double from, double to, double step);
  void set_decimals(int decimals);
  void set_accent(const QColor &accent);
  void set_defaults(double x, double y);

  /// Assign without animating - for model -> widget sync.
  void set_values(double x, double y);

  void set_linked(bool linked);
  bool is_linked() const;

  double x() const;
  double y() const;

signals:
  void value_changed();
  void edit_ended();

private:
  /// Push `v` into the other rail when linked, without re-entering.
  void mirror(ParamSlider *from, ParamSlider *to);

  ParamSlider *x_ = nullptr;
  ParamSlider *y_ = nullptr;
  ModButton   *link_ = nullptr;
  bool         mirroring_ = false;
};

} // namespace meta::qt
