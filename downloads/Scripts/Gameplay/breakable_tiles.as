const uint EDITOR_COLOR = 0x8800FF00;


void hit_effect(scene@ g, hitbox@ hb) { //
	dustman@ o = hb.owner().as_dustman();
	if (@o == null)
		return;

	// player state
	if (o.y_speed() > 0) {
		o.set_speed_xy(o.x_speed(), 0);
		o.state(7);
	} else if (o.y_speed() < 0) {
		o.state(19);
	}
	o.freeze_frame_timer(1);

	// effect

	// sound
	g.play_sound("sfx_impact_light_1", hb.x(), hb.y(), 1, false, true);
}


class SnowTile {
	[position, mode:world, layer:19, y:y] float x;
	[hidden] float y;
	int tile_x;
	int tile_y;
	bool active;
	tileinfo@ ti;

	void init(scene@ g) {
		active = true;
		tile_x = int(floor(x / 48));
		tile_y = int(floor(y / 48));
		x = tile_x * 48;
		y = tile_y * 48;
		@ti = g.get_tile(tile_x, tile_y);
	}

	void hit(scene@ g) { // per tile
		active = false;
		ti.solid(false);
		g.set_tile(tile_x, tile_y, 19, ti, true);

		// hb.hit_outcome(1); // unused because funny iteration order, gives hitrise

		// effect

	}

	hitbox@ check_hitbox(scene@ g) {
		if (active) {
			int hits = g.get_entity_collision(y, y + 48, x, x + 48, 8);
			for (int i = 0; i < hits; i++) {
				hitbox@ hb = g.get_entity_collision_index(i).as_hitbox();
				if (@hb == null)
					continue;
				if (hb.triggered() && hb.state_timer() == hb.activate_time()) {
					dustman@ o = hb.owner().as_dustman();
					if (@o == null)
						continue;
					if (o.character() == "dustworth" ||
						o.character() == "dustman" ||
						o.character() == "dustgirl" ||
						o.character() == "dustkid") {
							hit(g);
							return @hb;
}
				}
			}
		}
		//
		return null;
	}

	void editor_draw(scene@ g) {
		tile_x = int(floor(x / 48));
		tile_y = int(floor(y / 48));
		x = tile_x * 48;
		y = tile_y * 48;
		g.draw_rectangle_world(22, 0, x, y, x + 48, y + 48, 0, EDITOR_COLOR);
	}
}


class script {
	scene@ g;
	[text] array<SnowTile> tiles(0);
	[hidden] array<SnowTile> tiles_cp(0);

	script() {
		@g = get_scene();
	}

	void on_level_start() {
		for (uint i = 0; i < tiles.length; i++) {
			tiles[i].init(g);
		}
	}

	void step(int entities) {
		bool hit_success = false;
		hitbox@ hb;
		for (uint i = 0; i < tiles.length; i++) {
			hitbox@ h = tiles[i].check_hitbox(g);
			if (@h != null) {
				hit_success = true;
				@hb = @h;
			}
		}

		if (hit_success) {
			hit_effect(g, hb);
		}
	}

	void checkpoint_save() {
		tiles_cp = tiles;
	}

	void checkpoint_load() {
		tiles = tiles_cp;
	}

	void editor_draw(float sf) {
		for (uint i = 0; i < tiles.length; i++) {
			tiles[i].editor_draw(g);
		}
	}
}
