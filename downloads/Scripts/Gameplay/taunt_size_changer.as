const int MAX_PLAYERS = 4;
const float SCALE_RATE = 0.25;
const float MIN_SCALE = 0.25;

class script {
	scene@ g;
	float current_scale;
	float cp_scale;

	script() {
		@g = get_scene();
		current_scale = 1;
		cp_scale = 1;
	}

	void step(int entities) {

		for (int i = 0; i < MAX_PLAYERS; i++) {
			/* Try to get a 'dustman' object handle for each player. */
			controllable@ c = controller_controllable(i);
			if (@c == null) {
				continue;
			}
			dustman@ dm = c.as_dustman();
			if (@dm == null) {
				continue;
			}

			if (dm.taunt_intent() == 1) {
				if (dm.y_intent() == 1) {
					current_scale -= SCALE_RATE;
				} else {
					current_scale += SCALE_RATE;
				}
				if (current_scale < MIN_SCALE) {
					current_scale = MIN_SCALE;
				}
				dm.scale(current_scale, false);
			}
		}
	}

	void checkpoint_save() {
		cp_scale = current_scale;
	}

	void checkpoint_load() {
		current_scale = cp_scale;
	}

}