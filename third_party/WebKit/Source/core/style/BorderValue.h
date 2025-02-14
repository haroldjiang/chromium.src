/*
 * Copyright (C) 2000 Lars Knoll (knoll@kde.org)
 *           (C) 2000 Antti Koivisto (koivisto@kde.org)
 *           (C) 2000 Dirk Mueller (mueller@kde.org)
 * Copyright (C) 2003, 2005, 2006, 2007, 2008 Apple Inc. All rights reserved.
 * Copyright (C) 2006 Graham Dennis (graham.dennis@gmail.com)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#ifndef BorderValue_h
#define BorderValue_h

#include "core/css/StyleColor.h"
#include "core/style/ComputedStyleConstants.h"
#include "platform/graphics/Color.h"
#include "wtf/Allocator.h"

namespace blink {

// In order to conserve memory, the border width uses fixed point,
// which can be bitpacked.  This fixed point implementation is
// essentially the same as in LayoutUnit.  Six bits are used for the
// fraction, which leaves 20 bits for the integer part, making 1048575
// the largest number.

static const int kBorderWidthFractionalBits = 6;
static const int kBorderWidthDenominator = 1 << kBorderWidthFractionalBits;
static const int kMaxForBorderWidth = ((1 << 26) - 1) / kBorderWidthDenominator;

class BorderValue {
  DISALLOW_NEW();
  friend class ComputedStyle;

 public:
  BorderValue()
      : m_color(0),
        m_colorIsCurrentColor(true),
        m_style(BorderStyleNone),
        m_isAuto(OutlineIsAutoOff) {
    setWidth(3);
  }

  bool nonZero() const { return width() && (m_style != BorderStyleNone); }

  bool isTransparent() const {
    return !m_colorIsCurrentColor && !m_color.alpha();
  }

  bool operator==(const BorderValue& o) const {
    return m_width == o.m_width && m_style == o.m_style &&
           m_color == o.m_color &&
           m_colorIsCurrentColor == o.m_colorIsCurrentColor;
  }

  // The default width is 3px, but if the style is none we compute a value of 0
  // (in ComputedStyle itself)
  bool visuallyEqual(const BorderValue& o) const {
    if (m_style == BorderStyleNone && o.m_style == BorderStyleNone)
      return true;
    if (m_style == BorderStyleHidden && o.m_style == BorderStyleHidden)
      return true;
    return *this == o;
  }

  bool operator!=(const BorderValue& o) const { return !(*this == o); }

  void setColor(const StyleColor& color) {
    m_color = color.resolve(Color());
    m_colorIsCurrentColor = color.isCurrentColor();
  }

  StyleColor color() const {
    return m_colorIsCurrentColor ? StyleColor::currentColor()
                                 : StyleColor(m_color);
  }

  float width() const {
    return static_cast<float>(m_width) / kBorderWidthDenominator;
  }
  void setWidth(float width) { m_width = widthToFixedPoint(width); }

  // Since precision is lost with fixed point, comparisons also have
  // to be done in fixed point.
  bool widthEquals(float width) const {
    return widthToFixedPoint(width) == m_width;
  }

  EBorderStyle style() const { return static_cast<EBorderStyle>(m_style); }
  void setStyle(EBorderStyle style) { m_style = style; }

 protected:
  static unsigned widthToFixedPoint(float width) {
    DCHECK_GE(width, 0);
    return static_cast<unsigned>(std::min<float>(width, kMaxForBorderWidth) *
                                 kBorderWidthDenominator);
  }

  Color m_color;
  unsigned m_colorIsCurrentColor : 1;

  unsigned m_width : 26;  // Fixed point width
  unsigned m_style : 4;  // EBorderStyle

  // This is only used by OutlineValue but moved here to keep the bits packed.
  unsigned m_isAuto : 1;  // OutlineIsAuto
};

}  // namespace blink

#endif  // BorderValue_h
