/**************************************************************************/
/*  category_source.cpp                                                   */
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

// DREAMENGINE: see category_source.h.

#include "category_source.h"

#include "core/object/class_db.h"

VARIANT_ENUM_CAST(DreamCategorySource::UnmatchedBehavior);

void DreamCategorySource::_bind_methods() {
	ClassDB::bind_method(D_METHOD("classify", "region"), &DreamCategorySource::classify);

	ClassDB::bind_method(D_METHOD("set_unmatched_behavior", "behavior"), &DreamCategorySource::set_unmatched_behavior);
	ClassDB::bind_method(D_METHOD("get_unmatched_behavior"), &DreamCategorySource::get_unmatched_behavior);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "unmatched_behavior", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_NONE), "set_unmatched_behavior", "get_unmatched_behavior");

	BIND_ENUM_CONSTANT(UNMATCHED_WARN);
	BIND_ENUM_CONSTANT(UNMATCHED_SENTINEL);
}

void DreamCategorySource::set_unmatched_behavior(UnmatchedBehavior p_behavior) {
	unmatched_behavior = p_behavior;
}

DreamCategorySource::UnmatchedBehavior DreamCategorySource::get_unmatched_behavior() const {
	return unmatched_behavior;
}

PackedByteArray DreamCategorySource::classify(const Rect2i &p_region) const {
	PackedByteArray out;
	out.resize(p_region.size.x * p_region.size.y);
	int idx = 0;
	for (int y = p_region.position.y; y < p_region.position.y + p_region.size.y; y++) {
		for (int x = p_region.position.x; x < p_region.position.x + p_region.size.x; x++) {
			out.write[idx++] = get_pixel_category(Vector2i(x, y));
		}
	}
	return out;
}
