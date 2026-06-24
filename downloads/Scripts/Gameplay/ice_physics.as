class script {
	scene@ g;
	dustman@ player;
	
	script() {
    		// script must have a constructor that takes no arguments.
    		puts("Script instantiated");
		@g = get_scene();
	}
	
	void on_level_start() {
		if(@player == null){
            		controllable@ c = controller_controllable(0);
            		if(@c != null){
				@player = c.as_dustman();
			}
		}

		add_icy_physics(player);
		
  	}

	void step(int entities){

	}

	void checkpoint_load(){
		controllable@ c = controller_controllable(0);
            	if(@c != null){
			@player = c.as_dustman();
		}

		add_icy_physics(player);
	}

	void add_icy_physics(dustman@ player){
		float run_max = player.run_max();
		float run_accel = player.run_accel();
		float slope_slide_speed = player.slope_slide_speed();
		float slope_max = player.slope_max();
		float idle_fric = player.idle_fric();
		float skid_fric = player.skid_fric();
		float land_fric = player.land_fric();
		float roof_fric = player.roof_fric();
		float wall_slide_speed = player.wall_slide_speed();
		float wall_run_length = player.wall_run_length();

		player.run_max(run_max*1.5);
		player.run_accel(run_accel*0.2);
		player.slope_slide_speed(slope_slide_speed*1.5);
		player.slope_max(slope_max*1.5);
		player.idle_fric(idle_fric*0.01);
		player.skid_fric(skid_fric*0.01);
		player.land_fric(land_fric*0.01);
		player.roof_fric(roof_fric*0.01);
		player.wall_slide_speed(wall_slide_speed*10);
		player.wall_run_length(wall_run_length*0.7);
	}	
}