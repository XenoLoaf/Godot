/**************************************************************************/
/*  image_category_trace.h                                                */
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

// DREAMENGINE: DreamImageCategoryTrace — the image → category → collision trace
// primitive. Classifies an image region into a dense category byte grid via a
// DreamCategorySource, run-length encodes it, merges runs into maximal rectangles,
// and emits per-category convex collision polygons. Pure data + math, no scene
// or physics coupling. See docs/TILE_SYSTEM_FORK.md §3.

#ifndef IMAGE_CATEGORY_TRACE_H
#define IMAGE_CATEGORY_TRACE_H

#include "core/object/ref_counted.h"
#include "core/math/rect2i.h"
#include "core/templates/local_vector.h"
#include "core/variant/variant.h"

class Image;
class DreamCategorySource;

class DreamImageCategoryTrace : public RefCounted {
	GDCLASS(DreamImageCategoryTrace, RefCounted);

public:
	// DREAMENGINE: category ids (indexed identity — slot ids 0..N; canonical).
	enum Category {
		CATEGORY_NONE = 0,
		CATEGORY_GROUND = 1,
		CATEGORY_WALL = 2,
		CATEGORY_FLUID = 3,
		// Sentinel for pixels that matched no category (see DreamCategorySource
		// unmatched behavior). Keep last; not a real slot id.
		CATEGORY_UNCATEGORIZED = 255,
	};

	// DREAMENGINE: one horizontal run of a single category in a grid scan.
	struct Run {
		int category = CATEGORY_NONE;
		int x0 = 0;
		int y = 0;
		int length = 0;
	};

	// DREAMENGINE: one maximal merged rectangle of a single category (grid cells).
	struct Rect {
		int category = CATEGORY_NONE;
		int x = 0;
		int y = 0;
		int w = 0;
		int h = 0;
	};

	// DREAMENGINE: scan a dense category grid (row-major, size.x wide) into runs.
	// CATEGORY_NONE cells are skipped (absence of geometry); all others emit runs.
	static LocalVector<Run> trace_runs(const PackedByteArray &p_grid, const Vector2i &p_size);

	// DREAMENGINE: coalesce runs into maximal per-category rectangles
	// (vertical-adjacent runs of equal extent on the same category column merge).
	static LocalVector<Rect> merge_runs(const LocalVector<Run> &p_runs);

	// DREAMENGINE: trace a category grid into convex collision polygons,
	// bucketed by category. Each polygon is emitted as a Vector<Vector2>. Returns
	// a flat list; use the parallel categories array to know which slot each
	// polygon belongs to (only solid categories are meaningfully consumed).
	static void trace_to_polygons(const PackedByteArray &p_grid, const Vector2i &p_size, LocalVector<LocalVector<Vector2>> &r_polygons, LocalVector<int> &r_categories);

protected:
	static void _bind_methods();
};

#endif // IMAGE_CATEGORY_TRACE_H
