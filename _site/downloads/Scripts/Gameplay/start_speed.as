const int MAX_PLAYERS = 4;
const int FIRST_FRAME = 55;

class script {
	scene@ g;
	[text] float x_speed = 0;
	[text] float y_speed = 0;
	int frames;

	script() {
		@g = get_scene();
		frames = 0;
	}

	void step(int entities) {
		frames++;
		if (frames == FIRST_FRAME) {
			for (int i = 0; i < MAX_PLAYERS; i++) {
				controllable@ c = controller_controllable(i);
				if (@c == null)
					continue;
				dustman@ dm = c.as_dustman();
				if (@dm == null)
					continue;
				dm.set_speed_xy(x_speed, y_speed);
			}
		}
	}
}