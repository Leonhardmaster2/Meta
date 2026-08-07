/* Copyright (c) 2026 Otto Link. Distributed under the terms of the GNU Lesser
 * General Public License. The full license is in the file LICENSE, distributed
 * with this software. */

/**
 * @file h_filename.hpp
 * @brief Path field with a Browse action, for std::filesystem::path.
 *
 * A recessed field showing the path plus a chip that opens the right dialog.
 * The field is editable: typing a path is often faster than navigating to it,
 * and pasting one is the only sane way to reach a network share.
 *
 * Long paths elide from the LEFT when not being edited - the filename is what
 * identifies the file, so it is the tail that must stay visible.
 */
#pragma once
#include <QString>
#include <QWidget>

#include "meta_qt/widgets/industrial/tokens.hpp"

namespace meta::qt
{

class ModButton;
class PathField; // internal, defined in the .cpp

class HFilename : public QWidget
{
  Q_OBJECT

public:
  enum class Mode
  {
    Open,      ///< pick an existing file
    Save,      ///< name a file that need not exist
    Directory, ///< pick a folder
  };

  explicit HFilename(QWidget *parent = nullptr);

  void set_mode(Mode mode);
  void set_filter(const QString &filter);
  void set_path(const QString &path);

  QString path() const;

signals:
  void value_changed();
  void edit_ended();

private:
  void browse();

  PathField *field_ = nullptr;
  ModButton *browse_btn_ = nullptr;
  Mode       mode_ = Mode::Open;
  QString    filter_;
};

} // namespace meta::qt
