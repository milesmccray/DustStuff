class script {
//Various Actions change Character. Dashing/Falling: become worth. 
//Jumping: become kid without getting the extra aircharge. 
//Heavying: become worth for the start of the windup, and then kid for the attack. 
//Lights: become a random character. Taunt: become girl with a small chance of becoming man instead.
	scene@ g;
	dustman@ player;
	
	script() {
    		// script must have a constructor that takes no arguments.
    		puts("Script instantiated");
		@g = get_scene();
		
  	}

	uint32 assign = rand();

	void step(int entities){
		if(@player == null){
            		controllable@ c = controller_controllable(0);
            		if(@c != null){
				@player = c.as_dustman();
			}
		}
		if(player.taunt_intent() != 0){
			if(assign <= 5000000){
				player.character('dustman');
			}else{
				player.character('dustgirl');
			}
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
		if(player.light_intent() == 0){
			assign = rand();
		}else{
			if(assign <= 250000000){
				player.character('dustworth');
			}else{
				if(assign <= 500000000){
					player.character('dustkid');
				}else{
					if(assign <= 750000000){
						player.character('dustman');
					}else{
						player.character('dustgirl');
					}
				}
			}
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