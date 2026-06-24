class script : callback_base {

	scene@ g;
	dustman@ p;
	
	[text] float maxSpeed;
	
	script() {
		@g = get_scene();
	}
	
	void on_level_start() {
		@p = controller_controllable(0).as_dustman();
	}
	
	void checkpoint_load() {
		@p = controller_controllable(0).as_dustman();
	}
	
	void step(int entities) {
		if (p.x_speed() > maxSpeed) {
			p.set_speed_xy(maxSpeed, p.y_speed());
		} else if (p.x_speed() < -maxSpeed) {
			p.set_speed_xy(-maxSpeed, p.y_speed());
		}
	}

}