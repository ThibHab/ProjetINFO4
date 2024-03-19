/*
 * DISTRHO Plugin Framework (DPF)
 * Copyright (C) 2012-2021 Filipe Coelho <falktx@falktx.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any purpose with
 * or without fee is hereby granted, provided that the above copyright notice and this
 * permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD
 * TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN
 * NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
 * IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef DGL_IMAGE_WIDGETS_HPP_INCLUDED
#define DGL_IMAGE_WIDGETS_HPP_INCLUDED

#include "Image.hpp"
#include "ImageBaseWidgets.hpp"

START_NAMESPACE_DGL

#ifdef DGL_CAIRO
typedef CairoImageAboutWindow ImageAboutWindow;
typedef CairoImageButton ImageButton;
typedef CairoImageKnob ImageKnob;
typedef CairoImageSlider ImageSlider;
typedef CairoImageSwitch ImageSwitch;
#else
typedef OpenGLImageAboutWindow ImageAboutWindow;
typedef OpenGLImageButton ImageButton;
typedef OpenGLImageKnob ImageKnob;
typedef OpenGLImageSlider ImageSlider;
typedef OpenGLImageSwitch ImageSwitch;
#endif

END_NAMESPACE_DGL

#endif // DGL_IMAGE_WIDGETS_HPP_INCLUDED
