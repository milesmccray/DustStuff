class script {
	
	// the region in which props will scroll
	[position,mode:world,layer:19,y:y1] int x1 = 0;
	[hidden] int y1 = 0;
	[position,mode:world,layer:19,y:y2] int x2 = 0;
	[hidden] int y2 = 0;
	// the speed that props will scroll at, for each layer
	[text] array<int> layer_speed(21,0);
	
	scene@ g;
	
	array<prop@> scroll_props;
	uint prop_count;

	script() {
		@g = get_scene();
		
	}
	
	void query_props() {
		// if props move very close to the border during a checkpoint save,
		// they may not be re-queried when the checkpoint loads. Adding this buffer catches them
		const int BUFFER = 60;
		
		// list all props within the given rectangle
		scroll_props = array<prop@>(g.get_prop_collision(y1, y2, x1-BUFFER, x2+BUFFER));
		prop_count = 0;
		
		// record all props that need to be moved
		prop@ p;
		for(uint i=0; i< scroll_props.length(); i++) {
			@p = g.get_prop_collision_index(i);
			if (layer_speed[p.layer()] != 0) {
				@scroll_props[prop_count] = @p;
				prop_count++;
			}
		}
	}
	
	void on_level_start() {
		// make sure that point 1 is the top left, and point 2 is the bottom right
		if(x1 > x2) {
			int temp = x1;
			x1 = x2;
			x2 = temp;
		}
		if(y1 > y2) {
			int temp = y1;
			y1 = y2;
			y2 = temp;
		}
		
		query_props();
	}

	void step(int entities) {
		// move each prop in the region
		for(uint i=0; i < prop_count; i++) {
			if(scroll_props[i] !is null) {
				scroll_props[i].x( scroll_props[i].x() + layer_speed[scroll_props[i].layer()] );
				
				// if the prop has passed the edge, wrap around to the opposite edge
				if(scroll_props[i].x() >= x2)
					scroll_props[i].x( scroll_props[i].x() - (x2-x1));
			}
		}
	}
	
	void checkpoint_save() {
		
	}
	
	void checkpoint_load() {
		query_props();
	}
	
	void editor_draw(float sub_frame) {
		//DRAWING_CONSTANTS
		const int WIDTH = 5;
		
		g.draw_line_world(22, 24, x1, y1, x1, y2, WIDTH, 0xFFFFFFFF);
		g.draw_line_world(22, 24, x1, y1, x2, y1, WIDTH, 0xFFFFFFFF);
		g.draw_line_world(22, 24, x2, y2, x1, y2, WIDTH, 0xFFFFFFFF);
		g.draw_line_world(22, 24, x2, y2, x2, y1, WIDTH, 0xFFFFFFFF);
	}
}