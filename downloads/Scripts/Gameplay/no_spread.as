class script : callback_base {
	scene@ g;
	
	script() {
		@g = get_scene();
	}
	
	void entity_on_add(entity@ e) {
	  if (e.type_name() == 'filth_ball') {
		  g.remove_entity(e);
	  }
	}
}