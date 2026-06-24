class script : callback_base {

	scene@ g;
	dustman@ p;
	
	int direction;
	
	array<string> replaceSprite;
	
	script() {
		@g = get_scene();
	}
	
	void on_level_start() {
		@p = controller_controllable(0).as_dustman();
		direction = 1;
		
		array<string> temp = {"sprint", "land", "superskidf", "risingf", "hoverf", "hover", "fall", "dblrisingf", "dblhoverf", "dblfallf", "dblhover", "dblfall"};
		replaceSprite = temp;
	}
	
	void checkpoint_load() {
		
	}
	
	void step(int entities) {
		if (p.x_intent() != 0) {
			direction = p.x_intent();
			p.face(direction);
		}
		
		if (!p.wall_left() && !p.wall_right())
			p.set_speed_xy(direction * 800, p.y_speed());
		
		for (int i = 0; i < replaceSprite.length(); i++) {
			if (p.sprite_index() == replaceSprite[i]) {
				p.sprite_index("idle");
			}
		}
		
		p.heavy_intent(0);
		p.dash_intent(0);
	}
	
	void draw(float subframe) {
		
	}

}