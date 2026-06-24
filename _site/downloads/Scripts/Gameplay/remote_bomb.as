class script : callback_base {

	scene@ g;
	dustman@ player;
	
	float x1, y1;
	bool placed;
	int r, kaboom;
	
	script() {
		@g = get_scene();
	}
	
	void on_level_start() {
		@player = controller_controllable(0).as_dustman();
		placed = false;
		kaboom = 0;
		x1 = 0;
		y1 = 0;
		r = 15;
	}
	
	void checkpoint_load() {
		@player = controller_controllable(0).as_dustman();
		placed = false;
		kaboom = 0;
		x1 = 0;
		y1 = 0;
		r = 15;
	}
	
	void entity_on_remove(entity@ e) {
		if (e.type_name() == "enemy_tutorial_square" && kaboom > 0) {
			player.dash(player.dash_max());
			player.skill_combo(player.skill_combo() + 1);
			player.combo_count(player.combo_count() + 1);
			player.combo_timer(1);
		}
	}
	
	void step(int entities) {
		if (player.heavy_intent() == 10) {
			player.heavy_intent(11);
			if (placed) {
				detonate();
				placed = false;
				kaboom = 3;
			} else {
				x1 = player.x();
				y1 = player.y() - 60;
				placed = true;
			}
		}
		if (kaboom > 0)
			kaboom--;
	}
	
	void detonate() {
		hitbox@ h = create_hitbox(player.as_controllable(), 0, x1, y1, -(r + 10), r + 10, -(r + 10), r + 10);
		g.add_entity(h.as_entity(), false);
	}
	
	void draw(float subframe) {
		if (placed) {
			g.draw_rectangle_world(18, 10, x1 - r, y1 - r, x1 + r, y1 + r, 0, 0xFF3EFF96);
		}
	}

}