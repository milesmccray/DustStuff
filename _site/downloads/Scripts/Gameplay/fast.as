const int MAX_PLAYERS = 4;
const float FRICTION = 0;

class script {
	scene@ g;

	script() {
		@g = get_scene();
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


			if (dm.idle_fric() != FRICTION) {
				dm.idle_fric(FRICTION);
			}
			if (dm.skid_fric() != FRICTION) {
				dm.skid_fric(FRICTION);
			}
			if (dm.land_fric() != FRICTION) {
				dm.land_fric(FRICTION);
			}
			if (dm.roof_fric() != FRICTION) {
				dm.roof_fric(FRICTION);
			}

			float s = max(522, abs(dm.speed()));
			s *= 1.15;
			float j = -max(800, abs(dm.speed() * 0.8));
			float fast_fall_multiplier = -1.5;
			if (j < -2000) {
				float fast_fall_multiplier = -1.2;
			}

			dm.dash_speed(s);
			dm.slope_max(s);
			dm.run_max(s);
			dm.run_start(s * 0.5);
			dm.jump_a(j);
			dm.hop_a(j);
			dm.fall_max(-j * 1.75);

			if (dm.fall_intent() == 1 && dm.state() == 5) {
				dm.set_speed_xy(dm.x_speed(), j * fast_fall_multiplier);
			}

		}

	}
}