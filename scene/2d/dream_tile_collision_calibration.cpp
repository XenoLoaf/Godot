/**************************************************************************/
/*  dream_tile_collision_calibration.cpp                                  */
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

// DREAMENGINE: see dream_tile_collision_calibration.h.

#include "dream_tile_collision_calibration.h"

#include "core/config/project_settings.h"
#include "core/math/geometry_2d.h"
#include "core/math/math_funcs.h"

// DREAMENGINE: calibration tolerances. Each defaults to "off" (upstream behavior).
//   - min_polygon_area: drop merged collision polygons whose area is below this
//     many square pixels (slivers/degnerate cells from pixel traces).
//   - merge_epsilon:   weld polygon vertices closer than this many pixels.
//   - corner_rounding: inflate each outline by this many pixels (rounded join),
//     widening thin walls into walkable-blocking masses.
namespace {
const char *SETTING_MIN_POLYGON_AREA = "dream_engine/tile/min_polygon_area";
const char *SETTING_MERGE_EPSILON = "dream_engine/tile/merge_epsilon";
const char *SETTING_CORNER_ROUNDING = "dream_engine/tile/corner_rounding";

real_t _get_setting(const char *p_name, real_t p_default) {
	Variant value = ProjectSettings::get_singleton()->get_setting(p_name, p_default);
	if (value.get_type() == Variant::FLOAT) {
		return value;
	}
	if (value.get_type() == Variant::INT) {
		return real_t(int(value));
	}
	return p_default;
}

real_t _polygon_area(const Vector<Vector2> &p_polygon) {
	if (p_polygon.size() < 3) {
		return 0.0;
	}
	real_t area = 0.0;
	for (int i = 0; i < p_polygon.size(); i++) {
		const Vector2 &a = p_polygon[i];
		const Vector2 &b = p_polygon[(i + 1) % p_polygon.size()];
		area += a.x * b.y - b.x * a.y;
	}
	return Math::abs(area) * 0.5;
}

void _weld_vertices(Vector<Vector2> &p_polygon, real_t p_epsilon) {
	if (p_polygon.size() < 3) {
		return;
	}
	const real_t epsilon_sq = p_epsilon * p_epsilon;
	Vector<Vector2> out;
	for (const Vector2 &v : p_polygon) {
		if (out.is_empty() || out[out.size() - 1].distance_squared_to(v) > epsilon_sq) {
			out.push_back(v);
		}
	}
	// Close the loop: drop the last vertex if it coincides with the first.
	if (out.size() >= 3 && out[out.size() - 1].distance_squared_to(out[0]) <= epsilon_sq) {
		out.remove_at(out.size() - 1);
	}
	if (out.size() >= 3) {
		p_polygon = out;
	}
}

void _drop_small_polygons(Vector<Vector<Vector2>> &p_polygons, real_t p_min_area) {
	Vector<Vector<Vector2>> out;
	for (const Vector<Vector2> &poly : p_polygons) {
		if (_polygon_area(poly) >= p_min_area) {
			out.push_back(poly);
		}
	}
	p_polygons = out;
}

void _inflate_polygons(Vector<Vector<Vector2>> &p_polygons, real_t p_delta) {
	Vector<Vector<Vector2>> out;
	for (const Vector<Vector2> &poly : p_polygons) {
		Vector<Vector<Vector2>> offset = Geometry2D::offset_polygon(poly, p_delta, Geometry2D::JOIN_ROUND);
		for (const Vector<Vector2> &o : offset) {
			out.push_back(o);
		}
	}
	p_polygons = out;
}
} // namespace

void DreamTileCollisionCalibration::calibrate(Vector<Vector<Vector2>> &p_polygons, Vector<Vector<Vector2>> &p_holes) {
	const real_t min_area = _get_setting(SETTING_MIN_POLYGON_AREA, 0.0);
	const real_t merge_epsilon = _get_setting(SETTING_MERGE_EPSILON, 0.0);
	const real_t corner_rounding = _get_setting(SETTING_CORNER_ROUNDING, 0.0);
	if (min_area <= 0.0 && merge_epsilon <= 0.0 && corner_rounding == 0.0) {
		return;
	}

	// 1. Weld near-coincident vertices.
	if (merge_epsilon > 0.0) {
		for (Vector<Vector2> &poly : p_polygons) {
			_weld_vertices(poly, merge_epsilon);
		}
		for (Vector<Vector2> &poly : p_holes) {
			_weld_vertices(poly, merge_epsilon);
		}
	}

	// 2. Drop slivers below the area threshold.
	if (min_area > 0.0) {
		_drop_small_polygons(p_polygons, min_area);
		_drop_small_polygons(p_holes, min_area);
	}

	// 3. Inflate outlines (rounded join). Holes are left untouched: inflating a
	// hole would shrink the opening, and the calibration target is thin *walls*.
	if (corner_rounding != 0.0) {
		_inflate_polygons(p_polygons, corner_rounding);
	}
}
