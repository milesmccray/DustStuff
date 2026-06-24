class script {
//Various Actions change Character. No RNG
	scene@ g;
	dustman@ player;
	
	script() {
    		// script must have a constructor that takes no arguments.
    		puts("Script instantiated");
		@g = get_scene();
		
  	}

	void step(int entities){
		if(@player == null){
            		controllable@ c = controller_controllable(0);
            		if(@c != null){
				@player = c.as_dustman();
			}
		}
		if(player.taunt_intent() > 0){
			player.character('dustman');
		}
		if(player.dash_intent() == 1){
			player.character('dustworth');
		}
		if(player.fall_intent() == 1){
			player.character('dustworth');
		}
		if(player.state() == 5){
			player.character('dustworth');
		}
		if(player.jump_intent() == 1){
			player.character('dustkid');
			player.dash_max(1);
		}
		if(player.light_intent() > 0){
			player.character('dustgirl');
		}
		if(player.heavy_intent() > 0){
			player.character('dustkid');
		}else{	
			if(player.attack_timer() > 0){
				if(player.attack_state() == 2){
					if(player.attack_timer() < 3){
						player.character('dustworth');
					}else{
						player.character('dustkid');
					}
				}
			}
		}
	}
}