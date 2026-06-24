
class script : callback_base {
	scene@ g;
	int frame;
	int floor_sf;

	void on_level_start() {
		frame = 0;
		floor_sf = 0;
		set_callback();
	}

	void on_level_end() {
		g.plugin_score(floor_sf);
	}

	void checkpoint_load() {
		set_callback();
	}

	void set_callback() {
		controllable@ c = controller_controllable(get_active_player());
		dustman@ dm = c.as_dustman();
		dm.on_subframe_end_callback(this, "subframe", 0);
	}

	void subframe(dustman@ dm, int arg) {
		if (dm.ground() && frame >= 65)
			floor_sf++;
	}

	void step(int entities) {
		frame++;
		if (frame < 65) // 55 + bonus 10 (jumpsquat 8)
			return;

		for (int i = 0; i < 4; i++) {
			entity@ e = controller_entity(i);
			controllable@ c = controller_controllable(i);
			if (@c == null) {
				continue;
			}
			dustman@ dm = c.as_dustman();
			if (@dm == null) {
				return;
			}

			dm.combo_count(floor_sf);
			dm.combo_timer(1);
		}
	}
}