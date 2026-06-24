const int MAX_PLAYERS = 4;

class script {
	int frame;

	[text] bool force_x = false;
	[text] int intent_x = 0;
	[text] bool force_y = false;
	[text] int intent_y = 0;
	[text] bool force_jump = false;
	[text] int intent_jump = 0;
	[text] bool force_dash = false;
	[text] int intent_dash = 0;
	[text] bool force_fall = false;
	[text] int intent_fall = 0;
	[text] bool force_light = false;
	[text] int intent_light = 0;
	[text] bool force_heavy = false;
	[text] int intent_heavy = 0;
	[text] bool force_taunt = false;
	[text] int intent_taunt = 0;

	script() {
		frame = 0;
	}

	void step(int entities) {
		frame++;
		if (frame <= 55)
			return;

		for (int i = 0; i < MAX_PLAYERS; i++) {
			controllable@ c = controller_controllable(i);
			if (@c == null)
				continue;
			dustman@ dm = c.as_dustman();
			if (@dm == null)
				continue;

			if (force_x)
				dm.x_intent(intent_x);
			if (force_y)
				dm.y_intent(intent_y);
			if (force_jump)
				dm.jump_intent(intent_jump);
			if (force_dash)
				dm.dash_intent(intent_dash);
			if (force_fall)
				dm.fall_intent(intent_fall);
			if (force_light)
				dm.light_intent(intent_light);
			if (force_heavy)
				dm.heavy_intent(intent_heavy);
			if (force_taunt)
				dm.taunt_intent(intent_taunt);
		}

	}
}