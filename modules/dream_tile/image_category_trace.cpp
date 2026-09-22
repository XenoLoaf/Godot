/**************************************************************************/
/*  image_category_trace.cpp                                              */
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

// DREAMENGINE: see image_category_trace.h.

#include "image_category_trace.h"

#include <utility>

#include "core/object/class_db.h"
#include "core/variant/variant.h"

VARIANT_ENUM_CAST(DreamImageCategoryTrace::Category);

void DreamImageCategoryTrace::_bind_methods() {
	BIND_ENUM_CONSTANT(CATEGORY_NONE);
	BIND_ENUM_CONSTANT(CATEGORY_GROUND);
	BIND_ENUM_CONSTANT(CATEGORY_WALL);
	BIND_ENUM_CONSTANT(CATEGORY_FLUID);
	BIND_ENUM_CONSTANT(CATEGORY_UNCATEGORIZED);

	// DREAMENGINE: no GDScript bindings yet — the Run/Rect structs and
	// LocalVector returns are C++-only; the engine-side consumer is
	// DreamTileImageSource. The GDScript trace mirror already lives in the
	// base engine addon (category_source.gd).
}

namespace {
// DREAMENGINE: comparator for merge_runs — sort runs by (category, x0, y, len).
struct RunComparator {
	_FORCE_INLINE_ bool operator()(const DreamImageCategoryTrace::Run &p_a, const DreamImageCategoryTrace::Run &p_b) const {
		if (p_a.category != p_b.category) {
			return p_a.category < p_b.category;
		}
		if (p_a.x0 != p_b.x0) {
			return p_a.x0 < p_b.x0;
		}
		if (p_a.y != p_b.y) {
			return p_a.y < p_b.y;
		}
		return p_a.length < p_b.length;
	}
};
} // namespace

LocalVector<DreamImageCategoryTrace::Run> DreamImageCategoryTrace::trace_runs(const PackedByteArray &p_grid, const Vector2i &p_size) {
	LocalVector<Run> runs;
	int w = p_size.x;
	int h = p_size.y;
	ERR_FAIL_COND_V(p_grid.size() != w * h, runs);

	for (int y = 0; y < h; y++) {
		int x = 0;
		while (x < w) {
			int category = p_grid[y * w + x];
			if (category == CATEGORY_NONE) {
				x += 1;
				continue;
			}
			int length = 1;
			while (x + length < w && p_grid[y * w + x + length] == category) {
				length += 1;
			}
			Run run;
			run.category = category;
			run.x0 = x;
			run.y = y;
			run.length = length;
			runs.push_back(run);
			x += length;
		}
	}
	return runs;
}

LocalVector<DreamImageCategoryTrace::Rect> DreamImageCategoryTrace::merge_runs(const LocalVector<Run> &p_runs) {
	LocalVector<Rect> rects;
	if (p_runs.is_empty()) {
		return rects;
	}

	// Copy into a sortable working buffer (LocalVector is move-only, so fill by value).
	LocalVector<Run> runs;
	runs.resize(p_runs.size());
	for (uint32_t i = 0; i < p_runs.size(); i++) {
		runs[i] = p_runs[i];
	}
	runs.sort_custom<RunComparator>();

	for (uint32_t i = 0; i < runs.size();) {
		const Run &seed = runs[i];
		int category = seed.category;
		int x0 = seed.x0;
		int len = seed.length;
		int top = seed.y;
		int bottom = seed.y;
		i += 1;
		while (i < runs.size() && runs[i].category == category && runs[i].x0 == x0 && runs[i].length == len && runs[i].y == bottom + 1) {
			bottom = runs[i].y;
			i += 1;
		}
		Rect rect;
		rect.category = category;
		rect.x = x0;
		rect.y = top;
		rect.w = len;
		rect.h = bottom - top + 1;
		rects.push_back(rect);
	}
	return rects;
}

void DreamImageCategoryTrace::trace_to_polygons(const PackedByteArray &p_grid, const Vector2i &p_size, LocalVector<LocalVector<Vector2>> &r_polygons, LocalVector<int> &r_categories) {
	r_polygons.clear();
	r_categories.clear();

	LocalVector<Run> runs = trace_runs(p_grid, p_size);
	LocalVector<Rect> rects = merge_runs(runs);

	for (const Rect &rect : rects) {
		// Emit one axis-aligned rectangle as a polygon (grid cells, top-left
		// corner origin). Consumers translate to their own frame.
		LocalVector<Vector2> poly;
		poly.push_back(Vector2(rect.x, rect.y));
		poly.push_back(Vector2(rect.x + rect.w, rect.y));
		poly.push_back(Vector2(rect.x + rect.w, rect.y + rect.h));
		poly.push_back(Vector2(rect.x, rect.y + rect.h));
		r_polygons.push_back(std::move(poly));
		r_categories.push_back(rect.category);
	}
}
