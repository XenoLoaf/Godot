/**************************************************************************/
/*  dream_tile_image_source.cpp                                           */
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

// DREAMENGINE: see dream_tile_image_source.h.

#include "dream_tile_image_source.h"

#include "category_source.h"
#include "image_category_trace.h"

#include "core/io/image.h"
#include "core/object/class_db.h"

void DreamTileImageSource::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_master_image", "image"), &DreamTileImageSource::set_master_image);
	ClassDB::bind_method(D_METHOD("get_master_image"), &DreamTileImageSource::get_master_image);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "master_image", PROPERTY_HINT_RESOURCE_TYPE, "Image"), "set_master_image", "get_master_image");

	ClassDB::bind_method(D_METHOD("set_category_source", "source"), &DreamTileImageSource::set_category_source);
	ClassDB::bind_method(D_METHOD("get_category_source"), &DreamTileImageSource::get_category_source);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "category_source", PROPERTY_HINT_RESOURCE_TYPE, "DreamCategorySource"), "set_category_source", "get_category_source");

	ClassDB::bind_method(D_METHOD("generate_tile", "atlas_coords", "size"), &DreamTileImageSource::generate_tile);
}

void DreamTileImageSource::set_master_image(const Ref<Image> &p_image) {
	master_image = p_image;
}

Ref<Image> DreamTileImageSource::get_master_image() const {
	return master_image;
}

void DreamTileImageSource::set_category_source(const Ref<DreamCategorySource> &p_source) {
	category_source = p_source;
}

Ref<DreamCategorySource> DreamTileImageSource::get_category_source() const {
	return category_source;
}

void DreamTileImageSource::_ensure_physics_layer() {
	// DREAMENGINE: a fresh TileSet starts with zero physics layers; TileData
	// collision injection (add_collision_polygon(layer 0)) requires layer 0 to
	// exist. Ensure exactly one if none are present, without disturbing a
	// caller that already configured layers.
	TileSet *ts = TileSetSource::get_tile_set();
	if (ts == nullptr) {
		return;
	}
	if (ts->get_physics_layers_count() == 0) {
		ts->add_physics_layer();
	}
}

void DreamTileImageSource::_retrace_all_tiles() {
	// DREAMENGINE: re-derive collision for every generated tile. Only runs once
	// the source is hosted by a TileSet (physics layer count known).
	if (master_image.is_null() || category_source.is_null()) {
		return;
	}
	for (int i = 0; i < get_tiles_count(); i++) {
		_trace_tile_collision(get_tile_id(i));
	}
}

void DreamTileImageSource::set_tile_set(const TileSet *p_tile_set) {
	TileSetAtlasSource::set_tile_set(p_tile_set);
	if (p_tile_set != nullptr) {
		_ensure_physics_layer();
		_retrace_all_tiles();
	}
}

void DreamTileImageSource::notify_tile_data_properties_should_change() {
	TileSetAtlasSource::notify_tile_data_properties_should_change();
	if (TileSetSource::get_tile_set() != nullptr) {
		_ensure_physics_layer();
		_retrace_all_tiles();
	}
}

void DreamTileImageSource::_trace_tile_collision(const Vector2i &p_atlas_coords) {
	// DREAMENGINE: derive collision from the tile's region and inject it into the
	// TileData (alternative 0) in the tile's local frame (origin at tile center,
	// span [-size/2, +size/2]) — so the physics update's
	// tile_set->map_to_local(coords) translation lands it correctly.
	ERR_FAIL_COND(master_image.is_null());
	ERR_FAIL_COND(category_source.is_null());

	TileData *tile_data = get_tile_data(p_atlas_coords, 0);
	ERR_FAIL_NULL(tile_data);

	// Region in image pixels: mirrors TileSetAtlasSource::get_tile_texture_region
	// with margins/separation zero — origin = coords * region_size, extent =
	// region_size * size_in_atlas.
	const Vector2i region_size = get_texture_region_size();
	const Vector2i size_in_atlas = get_tile_size_in_atlas(p_atlas_coords);
	const Vector2i px_origin = p_atlas_coords * region_size;
	const Vector2i px_size = region_size * size_in_atlas;

	const Rect2i region = Rect2i(px_origin, px_size);
	ERR_FAIL_COND_MSG(!Rect2i(Vector2i(), master_image->get_size()).encloses(region), "DreamTileImageSource: tile region exceeds master image.");

	// Bulk classify the region.
	const PackedByteArray grid = category_source->classify(region);

	// Trace into per-category rectangles (each a polygon).
	LocalVector<LocalVector<Vector2>> polygons;
	LocalVector<int> categories;
	DreamImageCategoryTrace::trace_to_polygons(grid, px_size, polygons, categories);

	_ensure_physics_layer();

	// Reset layer-0 collision polygons, then rebuild from the trace.
	const int existing = tile_data->get_collision_polygons_count(0);
	for (int i = existing - 1; i >= 0; i--) {
		tile_data->remove_collision_polygon(0, i);
	}
	tile_data->set_collision_polygons_count(0, polygons.size());

	// Tile-local orientation: grid cell (0,0) is the tile's top-left. The tile's
	// local origin is its center, so shift every vertex by -half the region size.
	const Vector2 half = Vector2(px_size) * 0.5f;
	for (uint32_t i = 0; i < polygons.size(); i++) {
		Vector<Vector2> v;
		v.resize(polygons[i].size());
		for (uint32_t j = 0; j < polygons[i].size(); j++) {
			v.write[j] = polygons[i][j] - half;
		}
		// set_collision_polygon_points decomposes into convex shapes internally.
		tile_data->set_collision_polygon_points(0, i, v);
	}
}

void DreamTileImageSource::generate_tile(const Vector2i &p_atlas_coords, const Vector2i &p_size) {
	ERR_FAIL_COND(master_image.is_null());
	ERR_FAIL_COND_MSG(master_image->get_format() != Image::FORMAT_RGBA8 && master_image->get_format() != Image::FORMAT_RGB8, "DreamTileImageSource: master_image must be RGB8 or RGBA8.");

	// The master image is the virtual atlas: margins/separation are zero, and the
	// atlas origin math reduces to "cell coords * region_size == pixel offset".
	set_margins(Vector2i());
	set_separation(Vector2i());

	// Wrap the master image as the runtime atlas texture.
	Ref<ImageTexture> tex;
	tex.instantiate();
	tex->set_image(master_image);
	set_texture(tex);

	// Create the tile through the stock machinery, then inject traced collision.
	create_tile(p_atlas_coords, p_size);
	// DREAMENGINE: trace immediately if already hosted (physics layer known);
	// otherwise collision is traced lazily on set_tile_set().
	if (TileSetSource::get_tile_set() != nullptr) {
		_ensure_physics_layer();
		_trace_tile_collision(p_atlas_coords);
	}
}
