const int MAX_PLAYERS = 4;

class script : callback_base {

	void on_level_start() {
		set_callback();
	}

	void checkpoint_load() {
		set_callback();
	}

	void set_callback() {
		for (int i = 0; i < MAX_PLAYERS; i++) {
			controllable@ c = controller_controllable(i);
			if (@c == null)
				continue;
			dustman@ dm = c.as_dustman();
			if (@dm == null)
				continue;
			dm.on_subframe_end_callback(this, "subframe", 0);
		}
	}

	void subframe(dustman@ dm, int arg) {
		dm.y(dm.y() + 38400000);
		dm.y(dm.y() - 38400000);
	}
}