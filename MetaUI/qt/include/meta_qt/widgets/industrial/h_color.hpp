/* Copyright (c) 2026 Otto Link. Distributed under the terms of the GNU Lesser
 * General Public License. The full license is in the file LICENSE, distributed
 * with this software. */

/**
 * @file h_color.hpp
 * @brief Colour swatch with an editable hex readout, for glm::vec4.
 *
 * A swatch over the alpha checkerboard, and the value as #RRGGBBAA in a
 * recessed monospace field. Click the swatch for the picker, click the field
 * to type - pasting a hex code is how colours usually arrive, and a picker
 * alone makes that impossible.
 */
#pragma once
#include <QColor>
#include <QWidget>

#include "meta_qt/widgets/industrial/tokens.hpp"

namespace meta::qt
{

class ColorSwatch; ///< internal, defined in the .cpp
class HexField;    ///< internal, defined in the .cpp

class HColor : public QWidget
{
  Q_OBJECT

public:
  explicit HColor(QWidget *parent = nullptr);

  void   set_color(const QColor &color);
  QColor color() const;

signals:
  void value_changed();
  void edit_ended();

private:
  void apply(const QColor &color);

  ColorSwatch *swatch_ = nullptr;
  HexField    *field_ = nullptr;
  QColor       color_{Qt::white};
};

} // namespace meta::qt
