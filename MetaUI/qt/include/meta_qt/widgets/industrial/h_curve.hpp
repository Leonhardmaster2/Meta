/* Copyright (c) 2026 Otto Link. Distributed under the terms of the GNU Lesser
 * General Public License. The full license is in the file LICENSE, distributed
 * with this software. */

/**
 * @file h_curve.hpp
 * @brief Curve editor for a std::vector<float> of evenly spaced samples.
 *
 * The vector holds y values only; x is implied by the index, evenly spaced
 * across the domain. So handles move vertically and never horizontally - the
 * sample count is the curve's resolution, not something the user drags around.
 */
#pragma once
#include <QVector>
#include <QWidget>

#include "meta_qt/widgets/industrial/tokens.hpp"

namespace meta::qt
{

class ModButton;
class HCurveCanvas; // internal, defined in the .cpp

class HCurve : public QWidget
{
  Q_OBJECT

public:
  explicit HCurve(QWidget *parent = nullptr);

  void set_bounds(double min_y, double max_y);

  void            set_values(const QVector<double> &values);
  QVector<double> values() const;

signals:
  void value_changed();
  void edit_ended();

private:
  HCurveCanvas *canvas_ = nullptr;
  ModButton   *reset_btn_ = nullptr;
};

} // namespace meta::qt
