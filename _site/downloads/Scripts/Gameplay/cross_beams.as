class script : callback_base {

	scene@ g;
	dustman@ p;
	
	[text] bool vertical = true;
	[text] bool horizontal = true;
	[text] int aim_length;
	[text] int shot_delay;
	[text] int active_frames;
	//The beams cycle through three phases: aiming, powering up, active. These ints determine the length of each phase.
	
	int timer;
	float chx, chy, r;
	uint chc;
	
	script() {
		@g = get_scene();
		
		chx = chy = 0;
	}
	
	void on_level_start() {
		@p = controller_controllable(0).as_dustman();
		
		timer = aim_length + shot_delay + active_frames + 55;
		r = 0;
		chc = 0xFF33FFFF;
	}
	
	void checkpoint_load() {
		@p = controller_controllable(0).as_dustman();
		timer = aim_length + shot_delay + active_frames;
		r = 0;
		chc = 0xFF33FFFF;
	}
	
	void on_level_end() {
		timer = -3;
	}
	
	void step(int entities) {
		if (timer > active_frames + shot_delay) {
			chx = p.x();
			chy = p.y() - 48;
			r = 90 * (timer - active_frames - shot_delay) / aim_length;
			timer--;
		} else if (timer > active_frames) {
			chc = 0xFF333333;
			chc += (204 - 204 * (timer - active_frames) / shot_delay) * 0x00010000; //red
			chc += (204 * (timer - active_frames) / shot_delay) * 0x00000101; //green & blue
			timer--;
		} else if (timer > 0) {
			chc = 0xFFFF3333;
			if ((p.x() <= chx + 22 && p.x() >= chx - 22) || (p.y() - 48 <= chy + 32 && p.y() - 48 >= chy - 32)) {
				p.kill(true);
				timer = 0;
			}
			timer--;
		} else if (timer == 0) {
			chc = 0xFF33FFFF;
			timer = aim_length + shot_delay + active_frames;
		}
	}
	
	void draw(float subframe) {
		if (timer >= 0) {
			if (timer <= active_frames + 2) {
				if (horizontal)
					g.draw_rectangle_world(20, 16, chx - 10000, chy - 10, chx + 10000, chy + 10, 0, 0xFFFF3333);
				if (vertical)
					g.draw_rectangle_world(20, 16, chx - 10, chy - 10000, chx + 10, chy + 10000, 0, 0xFFFF3333);
			}
			g.draw_rectangle_world(20, 17, chx - 15, chy - 15, chx + 15, chy + 15, r, chc);
		}
	}

}