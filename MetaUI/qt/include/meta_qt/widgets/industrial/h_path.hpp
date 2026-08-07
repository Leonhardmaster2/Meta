/* Copyright (c) 2026 Otto Link. Distributed under the terms of the GNU Lesser
 * General Public License. The full license is in the file LICENSE, distributed
 * with this software. */

/**
 * @file h_path.hpp
 * @brief Point and path editor: dashed grid, draggable points, colorbar.
 *
 * Renders a std::vector<glm::vec3> attribute, where x and y place the point
 * and z is its value. Two modes: Points draws the set unordered, Path joins
 * them in order and can close the loop.
 *
 * Qt-only, like the rest of this directory - it speaks PathPoint, not glm, so
 * the conversion stays in the renderer.
 */
#pragma once
#include <QVector>
#include <QWidget>

#include "meta_qt/widgets/industrial/tokens.hpp"

namespace meta::qt
{

class ModButton;

struct PathPoint
{
  double x = 0.0;
  double y = 0.0;
  double z = 1.0; ///< the point's value, shown through the colorbar ramp
};

class PathCanvas; // internal, defined in the .cpp

class HPath : public QWidget
{
  Q_OBJECT

public:
  enum class Mode
  {
    Points, ///< unordered set
    Path,   ///< joined in order
  };

  explicit HPath(QWidget *parent = nullptr);

  void set_mode(Mode mode);
  void set_closed(bool closed);
  void set_bounds(double min_x, double max_x, double min_y, double max_y);
  void set_z_step(double step);

  void               set_points(const QVector<PathPoint> &points);
  QVector<PathPoint> points() const;

signals:
  /// Incremental - fires continuously while a point is dragged.
  void value_changed();
  /// Committed: drag released, point added or removed, toolbar action.
  void edit_ended();

private:
  PathCanvas *canvas_ = nullptr;
  ModButton  *clear_btn_ = nullptr;
  ModButton  *random_btn_ = nullptr;
  ModButton  *csv_btn_ = nullptr;
};

} // namespace meta::qt
