/**************************************************************************/
/*  category_source.h                                                     */
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

// DREAMENGINE: DreamCategorySource — the bulk classification contract. Abstract seam
// with a single bulk `classify(region) -> PackedByteArray` obligation (one byte
// of category id per pixel, row-major). The trace primitive consumes this in a
// two-pass RLE; nothing makes per-pixel virtual calls. See
// docs/TILE_SYSTEM_FORK.md §3.1.

#ifndef CATEGORY_SOURCE_H
#define CATEGORY_SOURCE_H

#include "core/object/ref_counted.h"
#include "core/math/rect2i.h"
#include "core/variant/variant.h"

class DreamCategorySource : public RefCounted {
	GDCLASS(DreamCategorySource, RefCounted);

protected:
	static void _bind_methods();

	// DREAMENGINE: subclass hook — classify one pixel at coordinate p_coord
	// (image-space) into a category id byte. Implementations may read a backing
	// image. Default passes through to the unmatched handler.
	virtual int get_pixel_category(const Vector2i &p_coord) const { return 0; }

public:
	// DREAMENGINE: unmatched-pixel behavior. Warn maps to NONE and logs once per
	// color; Sentinel emits CATEGORY_UNCATEGORIZED so unclassified pixels are
	// visible in the output grid.
	enum UnmatchedBehavior {
		UNMATCHED_WARN,
		UNMATCHED_SENTINEL,
	};

	// DREAMENGINE: classify an image-space region into category id bytes.
	// Returns PackedByteArray of length p_region.size.x * p_region.size.y,
	// row-major, in p_region order.
	virtual PackedByteArray classify(const Rect2i &p_region) const;

	void set_unmatched_behavior(UnmatchedBehavior p_behavior);
	UnmatchedBehavior get_unmatched_behavior() const;

	UnmatchedBehavior unmatched_behavior = UNMATCHED_WARN;
	mutable LocalVector<uint32_t> warned_tokens; // DREAMENGINE: dedupe warn-once (truncation-safe RGBA token).
};

#endif // CATEGORY_SOURCE_H
