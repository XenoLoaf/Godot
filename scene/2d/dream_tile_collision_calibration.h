/**************************************************************************/
/*  dream_tile_collision_calibration.h                                    */
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

// DREAMENGINE: DreamTileCollisionCalibration — an additive, opt-in tolerance
// pre-pass for tile collision generation (docs/TILE_SYSTEM_FORK.md §5). Applied
// to the merged tile-collision polygons in TileMapLayer::_physics_update before
// convex decomposition. Every tolerance is a ProjectSettings flag under
// `dream_engine/tile/`, defaulting to "off" so upstream behavior is preserved
// unless a project opts in.

#ifndef DREAM_TILE_COLLISION_CALIBRATION_H
#define DREAM_TILE_COLLISION_CALIBRATION_H

#include "core/math/vector2.h"
#include "core/templates/vector.h"

class DreamTileCollisionCalibration {
public:
	// DREAMENGINE: applies the configured tolerances to the merged collision
	// polygons (and holes) in place. No-op when no `dream_engine/tile/*`
	// tolerance is set (the default, upstream path).
	static void calibrate(Vector<Vector<Vector2>> &p_polygons, Vector<Vector<Vector2>> &p_holes);
};

#endif // DREAM_TILE_COLLISION_CALIBRATION_H
