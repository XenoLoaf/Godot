/**************************************************************************/
/*  exact_color_source.h                                                  */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

// DREAMENGINE: DreamExactColorSource — Mode A of DreamCategorySource: exact color →
// category projection with a visible fallback. Keys on the image's raw packed
// RGBA bytes (truncation-safe 32-bit token), not round-tripped Color floats, so
// a configured literal and the value read back from storage produce the same
// key. This is the authoring format; mask ("D") mode is a peer implementation.

#ifndef EXACT_COLOR_SOURCE_H
#define EXACT_COLOR_SOURCE_H

#include "category_source.h"

#include "core/templates/hash_map.h"
#include "core/variant/variant.h"

class Image;
class Color;

class DreamExactColorSource : public DreamCategorySource {
	GDCLASS(DreamExactColorSource, DreamCategorySource);

	Ref<Image> image;
	HashMap<uint32_t, int> color_to_category; // DREAMENGINE: 0xRRGGBBAA token -> slot id.

protected:
	static void _bind_methods();
	int get_pixel_category(const Vector2i &p_coord) const override;

	static uint32_t encode_color(const Color &p_color);

public:
	void set_image(const Ref<Image> &p_image);
	Ref<Image> get_image() const;

	void set_color_map(const Dictionary &p_color_to_category);
	Dictionary get_color_map() const;

	int classify_color(const Color &p_color) const;
};

#endif // EXACT_COLOR_SOURCE_H
