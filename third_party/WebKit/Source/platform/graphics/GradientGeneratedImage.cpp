/*
 * Copyright (C) 2008, 2009, 2010, 2012 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "platform/graphics/GradientGeneratedImage.h"

#include "platform/geometry/FloatRect.h"
#include "platform/geometry/IntSize.h"
#include "platform/graphics/GraphicsContext.h"

namespace blink {

void GradientGeneratedImage::draw(PaintCanvas* canvas,
                                  const PaintFlags& flags,
                                  const FloatRect& destRect,
                                  const FloatRect& srcRect,
                                  RespectImageOrientationEnum,
                                  ImageClampingMode) {
  SkRect visibleSrcRect = srcRect;
  if (!visibleSrcRect.intersect(
          SkRect::MakeIWH(m_size.width(), m_size.height())))
    return;

  const SkMatrix transform =
      SkMatrix::MakeRectToRect(srcRect, destRect, SkMatrix::kFill_ScaleToFit);
  SkRect visibleDestRect;
  transform.mapRect(&visibleDestRect, visibleSrcRect);

  PaintFlags gradientFlags(flags);
  m_gradient->applyToFlags(gradientFlags, transform);
  canvas->drawRect(visibleDestRect, gradientFlags);
}

void GradientGeneratedImage::drawTile(GraphicsContext& context,
                                      const FloatRect& srcRect) {
  // TODO(ccameron): This function should not ignore |context|'s color behavior.
  // https://crbug.com/672306
  PaintFlags gradientFlags(context.fillFlags());
  m_gradient->applyToFlags(gradientFlags, SkMatrix::I());

  context.drawRect(srcRect, gradientFlags);
}

bool GradientGeneratedImage::applyShader(PaintFlags& flags,
                                         const SkMatrix& localMatrix) {
  DCHECK(m_gradient);
  m_gradient->applyToFlags(flags, localMatrix);

  return true;
}

}  // namespace blink
