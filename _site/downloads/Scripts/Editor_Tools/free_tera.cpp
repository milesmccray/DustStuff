 #include '../lib/area.cpp'
 #include '../lib/math/math.cpp'

 class script {
    // Initializing player and scene
    scene@ g;
    dustman@ player;
    
    [text] array<area> Teras;

    int TerasCount = 0;

    int x = 0;
	
	script() {
	  // Getting scene
	  @g = get_scene();
	}
    
	void on_level_start() {

		// if no player is defined, get the player
		if(@player == null) {
			controllable@ c = controller_controllable(0);
			if(@c != null) {
				@player = c.as_dustman();
			}
		}

        TerasCount = Teras.length();

	}

	void checkpoint_load() {
		@player = null;
	}

	void step(int entities) {      
		//if no player is defined, get the player
		if(@player == null) {
			controllable@ c = controller_controllable(0);
		if(@c != null) {
		  @player = c.as_dustman();
		}
		//if there's no player, we can't step this frame
		if(@player == null)
		  return;
		}

        for(int i = 0; i < TerasCount; i++){
            if(Teras[i].player_in_area(player)){
                if(player.state() == 8 && player.x_speed() == 0){
                    x = player.x();

                    player.x((ceil(x/48.0)-0.5)*48.0);
                }
            }
        }

        
		
	  
	}

    void editor_draw(float sub_frame){
        for(int i = 0; i < Teras.length(); i++){
            g.draw_rectangle_world(20,20,Teras[i].left_x()*48,Teras[i].top_y()*48,
            Teras[i].right_x()*48+48,Teras[i].bottom_y()*48+48,0,0x70BB66E3);
        }
    }

 }