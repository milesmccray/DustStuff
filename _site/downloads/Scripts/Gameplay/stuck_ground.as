
class script : callback_base {
	scene@ g;

	script() {
		@g = get_scene();
	}

	void on_level_start() {
		set_callback();
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
		if (dm.ground())
			dm.set_speed_xy(0, 0);
		// if (dm.wall_left() || dm.wall_right())
			// dm.set_speed_xy(0, 0);
		// if (dm.roof())
			// dm.set_speed_xy(0, 0);
	}
}