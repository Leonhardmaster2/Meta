/* Copyright (c) 2026 Otto Link. Distributed under the terms of the GNU Lesser
 * General Public License. The full license is in the file LICENSE, distributed
 * with this software. */

/**
 * @file tokens.hpp
 * @brief Design tokens for the industrial widget set.
 *
 * Single source of truth for every colour and metric the custom-painted
 * widgets in this directory use.
 *
 * The two rules these tokens exist to enforce:
 *   - the rail fill is ALWAYS the group accent, it never encodes state;
 *   - only TEXT encodes state (white modified / grey default / dim locked).
 *
 * @note The colour tokens are VARIABLES, not compile-time constants, despite
 * the k-prefix. Assign a Theme through apply_theme() to restyle the whole
 * widget set at once - the library ships the industrial palette as its
 * default but is not locked to it. The geometry and timing tokens below are
 * genuinely constant: they encode the proportions the design depends on, and
 * changing them individually breaks the relationships between widgets.
 *
 * Call apply_theme() BEFORE constructing any widget. Widgets sample these at
 * paint time, so a later change needs an update() on everything to show.
 */
#pragma once
#include <QColor>
#include <QRect>
#include <QString>

class QPainter;

namespace meta::qt
{

// ---------------------------------------------------------------- surfaces
inline QColor kPage{"#2b2b2b"};        ///< panel background
inline QColor kBar{"#262626"};         ///< top / bottom chrome strips
inline QColor kApplyBar{"#2e2e2e"};    ///< apply strip
inline QColor kSectionHeader{"#333333"};
inline QColor kSectionHeaderHover{"#383838"};
inline QColor kSectionHeaderPress{"#303030"};
inline QColor kRailWell{"#1c1c1c"};    ///< slider track groove
inline QColor kField{"#1f1f1f"};       ///< value box, switch track (off)
inline QColor kFieldHover{"#262626"};
inline QColor kFieldEditing{"#161616"};
inline QColor kPadSurface{"#242424"};
inline QColor kPopup{"#262626"};

// ------------------------------------------------------- bevels, hairlines
inline QColor kBevelTop{"#3d3d3d"};    ///< section header top edge
inline QColor kBevelBottom{"#232323"}; ///< section header bottom edge
inline QColor kHairline{"#1a1a1a"};
inline QColor kRailBorder{"#161616"};
inline QColor kFieldBorder{"#4a4a4a"};
inline QColor kFieldBorderHover{"#5a5a5a"};
inline QColor kButtonBorder{"#1f1f1f"};
inline QColor kChipBorder{"#222222"};
inline QColor kChipBorderHover{"#565656"};

// ------------------------------------------------------- raised gradients
inline QColor kButtonTop{"#454545"};
inline QColor kButtonBottom{"#383838"};
inline QColor kButtonTopPress{"#333333"};
inline QColor kButtonBottomPress{"#2a2a2a"};
inline QColor kChipTop{"#3a3a3a"};
inline QColor kChipBottom{"#313131"};
inline QColor kChipTopHover{"#404040"};
inline QColor kChipBottomHover{"#363636"};
inline QColor kChipTopPress{"#303030"};
inline QColor kChipBottomPress{"#282828"};
inline QColor kChipTopActive{"#4a4a4a"};
inline QColor kChipBottomActive{"#3a3a3a"};

// ------------------------------------------------------------------- ink
inline QColor kInkPrimary{"#e0e0e0"};
inline QColor kInkTitle{"#d0d0d0"};
inline QColor kInkDefault{"#9a9a9a"};  ///< parameter at its default
inline QColor kInkDim{"#8a8a8a"};
inline QColor kInkLocked{"#606060"};   ///< parameter locked
inline QColor kInkModified{"#ffffff"}; ///< parameter changed
inline QColor kInkIcon{"#c9c9c9"};

// ----------------------------------------------------------------- metal
inline QColor kThumbTop{"#d6d6d6"};
inline QColor kThumbBottom{"#a8a8a8"};
inline QColor kThumbBorder{"#1a1a1a"};
inline QColor kThumbNotch{"#5f5f5f"};
inline QColor kKnobOnTop{"#e8e8e8"};
inline QColor kKnobOnBottom{"#b8b8b8"};
inline QColor kKnobOffTop{"#8a8a8a"};
inline QColor kKnobOffBottom{"#6a6a6a"};

// --------------------------------------------------------------- accents
inline QColor kAccent{"#e08a2e"};      ///< chrome accent, selection
inline QColor kAccentErosion{"#cfa143"};
inline QColor kAccentDowncutting{"#3aa899"};
inline QColor kAccentScale{"#7d9cc0"};
inline QColor kAccentFlow{"#c06478"};
inline QColor kAccentSelective{"#a08bb8"};
inline QColor kAccentOther{"#9a9a9a"};

// ---------------------------------------------------------- editor extras
inline QColor kPadBorder{"#4a4a4a"};
inline QColor kPadGrid{"#3a3a3a"};
inline QColor kPadCrosshair{"#7d9cc0"};
inline QColor kPadHandle{"#e0e0e0"};
inline QColor kRangeTrack{"#3a3a3a"};
inline QColor kRangeSpan{"#7d9cc0"};
inline QColor kPathPoint{"#d03030"};

/**
 * @brief The full colour palette, as a value.
 *
 * Field names mirror the token names without the k-prefix. Start from
 * default_theme() and override what you need rather than value-initialising,
 * so a token added in a later version does not silently become black.
 */
struct Theme
{
  QColor page, bar, apply_bar;
  QColor section_header, section_header_hover, section_header_press;
  QColor rail_well, field, field_hover, field_editing, pad_surface, popup;

  QColor bevel_top, bevel_bottom, hairline, rail_border;
  QColor field_border, field_border_hover, button_border;
  QColor chip_border, chip_border_hover;

  QColor button_top, button_bottom, button_top_press, button_bottom_press;
  QColor chip_top, chip_bottom, chip_top_hover, chip_bottom_hover;
  QColor chip_top_press, chip_bottom_press, chip_top_active, chip_bottom_active;

  QColor ink_primary, ink_title, ink_default, ink_dim, ink_locked;
  QColor ink_modified, ink_icon;

  QColor thumb_top, thumb_bottom, thumb_border, thumb_notch;
  QColor knob_on_top, knob_on_bottom, knob_off_top, knob_off_bottom;

  QColor accent, accent_erosion, accent_downcutting, accent_scale;
  QColor accent_flow, accent_selective, accent_other;

  QColor pad_border, pad_grid, pad_crosshair, pad_handle;
  QColor range_track, range_span, path_point;
};

/// The industrial palette the library ships with.
Theme default_theme();

/// Install a palette. Call before constructing widgets.
void apply_theme(const Theme &theme);

/// The palette currently installed.
Theme current_theme();

/**
 * @brief Group accents in presentation order.
 *
 * Sections cycle through these so consecutive groups stay visually distinct.
 * Reads the live tokens, so it follows apply_theme().
 */
QColor group_accent(int index);

// -------------------------------------------------------------- opacities
inline constexpr double kFillOpacity = 0.9;        ///< rail fill, normal
inline constexpr double kFillOpacityLocked = 0.3;  ///< rail fill, locked
inline constexpr double kThumbOpacityLocked = 0.4;
inline constexpr double kRangeSpanIdle = 0.5;

// -------------------------------------------------------------- geometry
inline constexpr int kRowHeight = 36;
inline constexpr int kRowSpacing = 10;
inline constexpr int kRailHeight = 6;
inline constexpr int kRailRadius = 1;
inline constexpr int kThumbWidth = 10;
inline constexpr int kThumbHeight = 18;
inline constexpr int kThumbRadius = 2;
inline constexpr int kNotchWidth = 2;
inline constexpr int kNotchHeight = 8;
inline constexpr int kGap = 12;
inline constexpr int kFieldHeight = 24;
inline constexpr int kFieldRadius = 2;
inline constexpr int kSectionHeaderHeight = 38;
inline constexpr int kBodyPadX = 20;
inline constexpr int kBodyPadXNarrow = 12;
inline constexpr int kBodyPadY = 12;
inline constexpr int kStackedRowHeight = 56;
inline constexpr int kCheckRowHeight = 28;
inline constexpr int kCheckBoxRowHeight = 26;
inline constexpr int kSwitchWidth = 36;
inline constexpr int kSwitchHeight = 18;
inline constexpr int kKnobSize = 12;
inline constexpr int kBoxSize = 18;
inline constexpr int kChipHeight = 26;

// ------------------------------------------------------- gradient editor
inline constexpr int kGradientBarHeight = 48;
inline constexpr int kGradientGutter = 18;    ///< stop-marker strip below the bar
inline constexpr int kGradientStopW = 11;
inline constexpr int kGradientStopH = 14;
inline constexpr int kGradientCheckSize = 6;  ///< alpha checkerboard cell
inline constexpr int kSwatchHeight = 22;
inline constexpr int kSwatchMinWidth = 56;
inline constexpr int kSwatchGap = 6;

/// Below this width a row switches to its compact metrics. NOTE: this is
/// compared against the ROW's own width, not the window's - inside a section
/// body a row is roughly 40px narrower than the panel.
inline constexpr int kNarrowThreshold = 430;

inline int label_width(int row_width)
{
  return qBound(90, static_cast<int>(row_width * 0.3), 168);
}

inline int field_width(int row_width)
{
  return row_width < kNarrowThreshold ? 64 : 74;
}

// ------------------------------------------------------------- animation
inline constexpr int kGlideMs = 260;      ///< every value change glides
inline constexpr int kHoverMs = 120;
inline constexpr int kCollapseMs = 220;
inline constexpr int kSwitchMs = 150;
inline constexpr int kDoubleClickMs = 450;
inline constexpr int kDragThresholdPx = 6; ///< below this, it stays a click

// ------------------------------------------------------------ typography
inline constexpr int kLabelPx = 12;
inline constexpr int kValuePx = 13;
inline constexpr int kTitlePx = 12;
inline constexpr int kIndexPx = 11;
inline constexpr int kChipPx = 11;

/**
 * @brief Fill `r` with the alpha checkerboard.
 *
 * Anything that shows a colour with an alpha channel needs it, or a
 * half-transparent colour just reads as a darker opaque one. Backed by a
 * cached tile: as a nested drawRect loop this ran to roughly a thousand calls
 * per repaint, on every frame of a collapse animation.
 */
void paint_checkerboard(QPainter &p, const QRect &r);

/**
 * @brief A monospace family that actually exists on this machine.
 *
 * Sizes are set in PIXELS throughout, never points, so the widgets render
 * identically regardless of the host's DPI scaling setting.
 */
QString mono_family();

} // namespace meta::qt
