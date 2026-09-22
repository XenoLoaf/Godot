/**************************************************************************/
/*  dream_tile_image_source.h                                             */
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

// DREAMENGINE: DreamTileImageSource — a TileSetAtlasSource whose tiles are cut
// live from a master image over arbitrary offsets, with collision derived from
// pixel categories rather than hand-authored. Reuses the stock atlas render
// path (get_runtime_texture / get_runtime_tile_texture_region) by treating the
// master image as a virtual atlas; traced collision is injected into each
// TileData at create_tile time via the public TileData setters.
//
// Base-class note (docs/TILE_SYSTEM_FORK.md §2): this subclasses
// TileSetAtlasSource (not the abstract TileSetSource) because TileMapLayer
// obtains every TileData* by Object::cast_to<TileSetAtlasSource>(source); a bare
// TileSetSource subclass would be stored but invisible to render/physics.

#ifndef DREAM_TILE_IMAGE_SOURCE_H
#define DREAM_TILE_IMAGE_SOURCE_H

#include "scene/resources/2d/tile_set.h"

class Image;
class DreamCategorySource;

class DreamTileImageSource : public TileSetAtlasSource {
	GDCLASS(DreamTileImageSource, TileSetAtlasSource);

	Ref<Image> master_image; // DREAMENGINE: the source image (virtual atlas).
	Ref<DreamCategorySource> category_source; // DREAMENGINE: pixel → category classifier.

	void _trace_tile_collision(const Vector2i &p_atlas_coords);
	void _ensure_physics_layer();
	void _retrace_all_tiles();

protected:
	static void _bind_methods();

public:
	void set_master_image(const Ref<Image> &p_image);
	Ref<Image> get_master_image() const;

	void set_category_source(const Ref<DreamCategorySource> &p_source);
	Ref<DreamCategorySource> get_category_source() const;

	// DREAMENGINE: cut a tile from the master image at atlas coords (a size.x-tile
	// wide region in the virtual atlas). Sets texture_region_size and creates the
	// tile + alternative 0, then traces and injects collision from the category
	// source.
	void generate_tile(const Vector2i &p_atlas_coords, const Vector2i &p_size);

	// DREAMENGINE: called when the source is attached to a TileSet (or its layers
	// change) — ensures a physics layer exists and (re)traces collision into all
	// generated tiles, because TileData collision depends on the hosting TileSet's
	// physics-layer count.
	virtual void set_tile_set(const TileSet *p_tile_set) override;
	virtual void notify_tile_data_properties_should_change() override;
};

#endif // DREAM_TILE_IMAGE_SOURCE_H
