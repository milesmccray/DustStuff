class script : callback_base {
	scene@ g;

	script() {
		@g = get_scene();
	}

	void remove_charge(dustman@ dm, int arg) {
		dm.dash(0);
		if (dm.ground()) {
			dm.dash_max(1);
		} else {
			dm.dash_max(0);
		}
	}

	void register_callback() {
		int nc = num_cameras();
		for (int i = 0; i < nc; i++) {
			dustman@ dm = @controller_entity(i).as_dustman();
			if (@dm != null) {
				dm.on_subframe_end_callback(this, "remove_charge", 0);
			}
		}
	}

	void on_level_start() {
		register_callback();
	}

	void checkpoint_load() {
		register_callback();
	}
}