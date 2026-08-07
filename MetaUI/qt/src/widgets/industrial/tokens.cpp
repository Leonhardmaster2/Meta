/* Copyright (c) 2026 Otto Link. Distributed under the terms of the GNU Lesser
 * General Public License. The full license is in the file LICENSE, distributed
 * with this software. */
#include <iterator>

#include <QBrush>
#include <QFontDatabase>
#include <QPainter>
#include <QPixmap>

#include "meta_qt/widgets/industrial/tokens.hpp"

namespace meta::qt
{

namespace
{
const QColor kCheckA{"#2a2a2a"};
const QColor kCheckB{"#343434"};

const QBrush &checker_brush()
{
  static const QBrush brush = []()
  {
    const int s = kGradientCheckSize;
    QPixmap   tile(2 * s, 2 * s);
    QPainter  p(&tile);
    p.fillRect(tile.rect(), kCheckA);
    p.fillRect(0, 0, s, s, kCheckB);
    p.fillRect(s, s, s, s, kCheckB);
    return QBrush(tile);
  }();

  return brush;
}
} // namespace

void paint_checkerboard(QPainter &p, const QRect &r)
{
  p.save();
  // anchor the pattern to the rect rather than to the window origin
  p.setBrushOrigin(r.topLeft());
  p.fillRect(r, checker_brush());
  p.restore();
}

Theme default_theme()
{
  // The shipped palette, captured from the token defaults so the two can never
  // drift apart.
  Theme t;

  t.page = QColor("#2b2b2b");
  t.bar = QColor("#262626");
  t.apply_bar = QColor("#2e2e2e");
  t.section_header = QColor("#333333");
  t.section_header_hover = QColor("#383838");
  t.section_header_press = QColor("#303030");
  t.rail_well = QColor("#1c1c1c");
  t.field = QColor("#1f1f1f");
  t.field_hover = QColor("#262626");
  t.field_editing = QColor("#161616");
  t.pad_surface = QColor("#242424");
  t.popup = QColor("#262626");

  t.bevel_top = QColor("#3d3d3d");
  t.bevel_bottom = QColor("#232323");
  t.hairline = QColor("#1a1a1a");
  t.rail_border = QColor("#161616");
  t.field_border = QColor("#4a4a4a");
  t.field_border_hover = QColor("#5a5a5a");
  t.button_border = QColor("#1f1f1f");
  t.chip_border = QColor("#222222");
  t.chip_border_hover = QColor("#565656");

  t.button_top = QColor("#454545");
  t.button_bottom = QColor("#383838");
  t.button_top_press = QColor("#333333");
  t.button_bottom_press = QColor("#2a2a2a");
  t.chip_top = QColor("#3a3a3a");
  t.chip_bottom = QColor("#313131");
  t.chip_top_hover = QColor("#404040");
  t.chip_bottom_hover = QColor("#363636");
  t.chip_top_press = QColor("#303030");
  t.chip_bottom_press = QColor("#282828");
  t.chip_top_active = QColor("#4a4a4a");
  t.chip_bottom_active = QColor("#3a3a3a");

  t.ink_primary = QColor("#e0e0e0");
  t.ink_title = QColor("#d0d0d0");
  t.ink_default = QColor("#9a9a9a");
  t.ink_dim = QColor("#8a8a8a");
  t.ink_locked = QColor("#606060");
  t.ink_modified = QColor("#ffffff");
  t.ink_icon = QColor("#c9c9c9");

  t.thumb_top = QColor("#d6d6d6");
  t.thumb_bottom = QColor("#a8a8a8");
  t.thumb_border = QColor("#1a1a1a");
  t.thumb_notch = QColor("#5f5f5f");
  t.knob_on_top = QColor("#e8e8e8");
  t.knob_on_bottom = QColor("#b8b8b8");
  t.knob_off_top = QColor("#8a8a8a");
  t.knob_off_bottom = QColor("#6a6a6a");

  t.accent = QColor("#e08a2e");
  t.accent_erosion = QColor("#cfa143");
  t.accent_downcutting = QColor("#3aa899");
  t.accent_scale = QColor("#7d9cc0");
  t.accent_flow = QColor("#c06478");
  t.accent_selective = QColor("#a08bb8");
  t.accent_other = QColor("#9a9a9a");

  t.pad_border = QColor("#4a4a4a");
  t.pad_grid = QColor("#3a3a3a");
  t.pad_crosshair = QColor("#7d9cc0");
  t.pad_handle = QColor("#e0e0e0");
  t.range_track = QColor("#3a3a3a");
  t.range_span = QColor("#7d9cc0");
  t.path_point = QColor("#d03030");

  return t;
}

void apply_theme(const Theme &t)
{
  kPage = t.page;
  kBar = t.bar;
  kApplyBar = t.apply_bar;
  kSectionHeader = t.section_header;
  kSectionHeaderHover = t.section_header_hover;
  kSectionHeaderPress = t.section_header_press;
  kRailWell = t.rail_well;
  kField = t.field;
  kFieldHover = t.field_hover;
  kFieldEditing = t.field_editing;
  kPadSurface = t.pad_surface;
  kPopup = t.popup;

  kBevelTop = t.bevel_top;
  kBevelBottom = t.bevel_bottom;
  kHairline = t.hairline;
  kRailBorder = t.rail_border;
  kFieldBorder = t.field_border;
  kFieldBorderHover = t.field_border_hover;
  kButtonBorder = t.button_border;
  kChipBorder = t.chip_border;
  kChipBorderHover = t.chip_border_hover;

  kButtonTop = t.button_top;
  kButtonBottom = t.button_bottom;
  kButtonTopPress = t.button_top_press;
  kButtonBottomPress = t.button_bottom_press;
  kChipTop = t.chip_top;
  kChipBottom = t.chip_bottom;
  kChipTopHover = t.chip_top_hover;
  kChipBottomHover = t.chip_bottom_hover;
  kChipTopPress = t.chip_top_press;
  kChipBottomPress = t.chip_bottom_press;
  kChipTopActive = t.chip_top_active;
  kChipBottomActive = t.chip_bottom_active;

  kInkPrimary = t.ink_primary;
  kInkTitle = t.ink_title;
  kInkDefault = t.ink_default;
  kInkDim = t.ink_dim;
  kInkLocked = t.ink_locked;
  kInkModified = t.ink_modified;
  kInkIcon = t.ink_icon;

  kThumbTop = t.thumb_top;
  kThumbBottom = t.thumb_bottom;
  kThumbBorder = t.thumb_border;
  kThumbNotch = t.thumb_notch;
  kKnobOnTop = t.knob_on_top;
  kKnobOnBottom = t.knob_on_bottom;
  kKnobOffTop = t.knob_off_top;
  kKnobOffBottom = t.knob_off_bottom;

  kAccent = t.accent;
  kAccentErosion = t.accent_erosion;
  kAccentDowncutting = t.accent_downcutting;
  kAccentScale = t.accent_scale;
  kAccentFlow = t.accent_flow;
  kAccentSelective = t.accent_selective;
  kAccentOther = t.accent_other;

  kPadBorder = t.pad_border;
  kPadGrid = t.pad_grid;
  kPadCrosshair = t.pad_crosshair;
  kPadHandle = t.pad_handle;
  kRangeTrack = t.range_track;
  kRangeSpan = t.range_span;
  kPathPoint = t.path_point;
}

Theme current_theme()
{
  Theme t;

  t.page = kPage;
  t.bar = kBar;
  t.apply_bar = kApplyBar;
  t.section_header = kSectionHeader;
  t.section_header_hover = kSectionHeaderHover;
  t.section_header_press = kSectionHeaderPress;
  t.rail_well = kRailWell;
  t.field = kField;
  t.field_hover = kFieldHover;
  t.field_editing = kFieldEditing;
  t.pad_surface = kPadSurface;
  t.popup = kPopup;

  t.bevel_top = kBevelTop;
  t.bevel_bottom = kBevelBottom;
  t.hairline = kHairline;
  t.rail_border = kRailBorder;
  t.field_border = kFieldBorder;
  t.field_border_hover = kFieldBorderHover;
  t.button_border = kButtonBorder;
  t.chip_border = kChipBorder;
  t.chip_border_hover = kChipBorderHover;

  t.button_top = kButtonTop;
  t.button_bottom = kButtonBottom;
  t.button_top_press = kButtonTopPress;
  t.button_bottom_press = kButtonBottomPress;
  t.chip_top = kChipTop;
  t.chip_bottom = kChipBottom;
  t.chip_top_hover = kChipTopHover;
  t.chip_bottom_hover = kChipBottomHover;
  t.chip_top_press = kChipTopPress;
  t.chip_bottom_press = kChipBottomPress;
  t.chip_top_active = kChipTopActive;
  t.chip_bottom_active = kChipBottomActive;

  t.ink_primary = kInkPrimary;
  t.ink_title = kInkTitle;
  t.ink_default = kInkDefault;
  t.ink_dim = kInkDim;
  t.ink_locked = kInkLocked;
  t.ink_modified = kInkModified;
  t.ink_icon = kInkIcon;

  t.thumb_top = kThumbTop;
  t.thumb_bottom = kThumbBottom;
  t.thumb_border = kThumbBorder;
  t.thumb_notch = kThumbNotch;
  t.knob_on_top = kKnobOnTop;
  t.knob_on_bottom = kKnobOnBottom;
  t.knob_off_top = kKnobOffTop;
  t.knob_off_bottom = kKnobOffBottom;

  t.accent = kAccent;
  t.accent_erosion = kAccentErosion;
  t.accent_downcutting = kAccentDowncutting;
  t.accent_scale = kAccentScale;
  t.accent_flow = kAccentFlow;
  t.accent_selective = kAccentSelective;
  t.accent_other = kAccentOther;

  t.pad_border = kPadBorder;
  t.pad_grid = kPadGrid;
  t.pad_crosshair = kPadCrosshair;
  t.pad_handle = kPadHandle;
  t.range_track = kRangeTrack;
  t.range_span = kRangeSpan;
  t.path_point = kPathPoint;

  return t;
}

QColor group_accent(int index)
{
  // Reads the live tokens rather than a static snapshot, so the cycle follows
  // apply_theme() instead of freezing whatever was installed at first call.
  const QColor palette[] = {kAccentErosion,
                            kAccentDowncutting,
                            kAccentScale,
                            kAccentFlow,
                            kAccentSelective,
                            kAccentOther};

  const int count = static_cast<int>(std::size(palette));
  return palette[((index % count) + count) % count];
}

QString mono_family()
{
  // Resolved once, per machine. The reference design specifies Menlo, which
  // only exists on macOS - naming it directly elsewhere silently falls back to
  // a proportional face and every numeric readout loses its column alignment.
  // The candidates below cover macOS, Windows and the common Linux font
  // packages, with the platform's own fixed-pitch font as a last resort, so
  // the widgets never render numerics in a proportional face on any host.
  static const QString family = []() -> QString
  {
    for (const QString &candidate : {QStringLiteral("Menlo"),
                                     QStringLiteral("Consolas"),
                                     QStringLiteral("DejaVu Sans Mono"),
                                     QStringLiteral("Liberation Mono"),
                                     QStringLiteral("Noto Sans Mono"),
                                     QStringLiteral("Ubuntu Mono")})
      if (QFontDatabase::families().contains(candidate))
        return candidate;

    return QFontDatabase::systemFont(QFontDatabase::FixedFont).family();
  }();

  return family;
}

} // namespace meta::qt
