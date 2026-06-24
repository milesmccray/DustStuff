const int MIN_LAYER = 16;
const int NUM_LAYERS = 4;
const int LOOP_TIME = 20;

//must be odd numbers
const int NUM_CELLS_X = 7;
const int NUM_CELLS_Y = 5;

class script {
	scene@ g;
	dustman@ player;
	//the number of tiles wide / tall each cell should be
	[text] int cell_width;
	[text] int cell_height;
	[text] array<RoomTemplate> templates;
	int prev_cell_x;
	int prev_cell_y;
	array<array<bool>> taken;

	script() {
		@g = get_scene();
	}
	
	void on_level_start() {
		puts("LEVEL START");
		
		if(@player == null) {
			controllable@ c = controller_controllable(0);
			if(@c != null) {
				@player = c.as_dustman();
				player.x(96);
				player.y(288);
				prev_cell_x = nearest_cell_x(player.x());
				prev_cell_y = nearest_cell_y(player.y());
			}
		}
		
		taken = array<array<bool>>(NUM_CELLS_Y+2, array<bool>(NUM_CELLS_X+2, false));
		
		puts(cell_height+" "+cell_width);
		
		//construct the templates
		for(uint i=0; i < templates.length(); i++) {
			puts("Building template "+i);
			templates[i].build_template(cell_width, cell_height, g);
		}
		
		//creates the first room
		if(templates.length() > 0) {
			puts("building rooms");
			
			for(int x  = -NUM_CELLS_X/2; x <= NUM_CELLS_X/2; x++) {
				for(int y = -NUM_CELLS_Y/2; y <= NUM_CELLS_Y/2; y++) {
					int i = rand_int(0, templates.length());
					templates[i].build_room(x*cell_width, y*cell_height, g);
				}
			}
		}
	}
	
	int rand_int(int min, int max) {
		//get a random number between 0 and 1 (0 inclusive)
		double frac = rand() / 1073741824.0;
		
		//fit the number into the range
		int random = int((frac * (max - min)) + min);
		
		return random;
	}
	
	void seed_rand(dustman@ player) {
		uint32 result = rand();
		
		//re-seed the randomizer for next frame, for maximum random!
		//use the players inputs so different people get different results
		uint32 salt = 0;
		if(@player != null) {
			salt += (player.x_intent()+1)   * 1;
			salt += (player.y_intent()+1)   * 10;
			salt += (player.heavy_intent()) * 100;
			salt += (player.light_intent()) * 1000;
			salt += (player.dash_intent())  * 10000;
			salt += (player.jump_intent())  * 100000;
		}
		
		result += salt;
		srand( result );
	}
	
	//returns the X cell coordinate of the current cell
	int nearest_cell_x(float pos_x) {
		int res = int(pos_x - ((pos_x % (cell_width * 48)+(cell_width * 48))%(cell_width * 48))) / 48;
		//puts("x:"+pos_x+" "+res);
		return res;
	}
	
	//returns the Y cell coordinate of the current cell
	int nearest_cell_y(float pos_y) {
		int res = int(pos_y - ((pos_y % (cell_height * 48))+(cell_height * 48))%(cell_height * 48)) / 48;
		//puts("y:"+pos_y+" "+res);
		return res;
	}
	
	void clear_cell(int left_x, int top_y, scene@ g) {
		tileinfo@ air_tile = create_tileinfo();
		air_tile.solid(false);
		tilefilth@ no_filth = create_tilefilth();
		
		for(int l = 0; l < NUM_LAYERS; l++) {
			if(l+MIN_LAYER == 18)
				continue;
			for(int dy = 0; dy < cell_height; dy++) {
				for(int dx = 0; dx < cell_width; dx++) {
					g.set_tile(left_x+dx, top_y+dy, l+MIN_LAYER, air_tile, true);
					if(l+MIN_LAYER == 19) {
						g.set_tile_filth(left_x+dx, top_y+dy, no_filth);
					}
				}
			}
		}
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
		seed_rand(player);
		
		int nearest_x = nearest_cell_x(player.x());
		int nearest_y = nearest_cell_y(player.y());
		
		//puts("prev = ( "+prev_cell_x+", "+prev_cell_y+" )");
		//puts("near = ( "+nearest_x+", "+nearest_y+" )");
		
		if(prev_cell_x != nearest_x) {
			int dir = 1;
			if(nearest_x < prev_cell_x) {
				dir = -1;
			}
			prev_cell_x += dir*cell_width;
			
			if(templates.length() > 0) {
				for(int y  = -NUM_CELLS_Y/2-1; y <= NUM_CELLS_Y/2+1; y++) {
					clear_cell(nearest_x - dir*(NUM_CELLS_X/2+1)*cell_width, nearest_y + y*cell_height, g);
				}
				for(int y  = -NUM_CELLS_Y/2; y <= NUM_CELLS_Y/2; y++) {
					int i = rand_int(0, templates.length());
					templates[i].build_room(nearest_x + dir*(NUM_CELLS_X/2)*cell_width, nearest_y + y*cell_height, g);
				}
			}
		}
		if(prev_cell_y != nearest_y) {
			int dir = 1;
			if(nearest_y < prev_cell_y) {
				dir = -1;
			}
			prev_cell_y += dir*cell_height;
			
			if(templates.length() > 0) {
				for(int x  = -NUM_CELLS_X/2-1; x <= NUM_CELLS_X/2+1; x++) {
					clear_cell(nearest_x + x*cell_width, nearest_y - dir*(NUM_CELLS_Y/2+1)*cell_height, g);
				}
				for(int x  = -NUM_CELLS_X/2; x <= NUM_CELLS_X/2; x++) {
					int i = rand_int(0, templates.length());
					templates[i].build_room(nearest_x + x*cell_width, nearest_y + dir*(NUM_CELLS_Y/2)*cell_height, g);
				}
			}
		}
	}
	
	void checkpoint_save() {
		
	}
	
	void checkpoint_load() {
		//the player handle is no longer valid when a checkpoint loads
		@player = null;
	}
	
	void editor_draw(float sub_frame) {
		for(uint i=0; i < templates.length(); i++) {
			if(templates[i].show_rectangle) {
				
				//DRAWING_CONSTANTS
				const int WIDTH = 5;
				
				//drawing vars
				int left_x = templates[i].left_x()*48;
				int top_y = templates[i].top_y()*48;
				int right_x = (templates[i].right_x()+1)*48;
				int bottom_y = (templates[i].bottom_y()+1)*48;
				
				int num_cells_w = templates[i].num_cells_w(cell_width);
				int num_cells_h = templates[i].num_cells_h(cell_height);
				int far_x, far_y, boundary_x, boundary_y;
				
				far_x = left_x + num_cells_w * cell_width * 48;
				far_y = top_y + num_cells_h * cell_height * 48;
				
				if(templates[i].width() % cell_width != 0 || templates[i].height() % cell_height != 0) {
					
					for(int cell_n = 0; cell_n <= num_cells_w; cell_n++){
						boundary_x = left_x + cell_n * cell_width * 48;
						g.draw_line_world(22, 24, boundary_x, top_y, boundary_x, far_y, WIDTH, 0xFFFF0000);
					}
					for(int cell_n = 0; cell_n <= num_cells_h; cell_n++){
						boundary_y = top_y + cell_n * cell_height * 48;
						g.draw_line_world(22, 24, left_x, boundary_y, far_x, boundary_y, WIDTH, 0xFFFF0000);
					}
				}
				else {
					for(int cell_n = 0; cell_n <= num_cells_w; cell_n++){
						boundary_x = left_x + cell_n * cell_width * 48;
						g.draw_line_world(22, 24, boundary_x, top_y, boundary_x, far_y, WIDTH, 0x40FFFFFF);
					}
					for(int cell_n = 0; cell_n <= num_cells_h; cell_n++){
						boundary_y = top_y + cell_n * cell_height * 48;
						g.draw_line_world(22, 24, left_x, boundary_y, far_x, boundary_y, WIDTH, 0x40FFFFFF);
					}
				}
				
				g.draw_line_world(22, 24, left_x, top_y, left_x, bottom_y, WIDTH, 0xFFFFFFFF);
				g.draw_line_world(22, 24, left_x, top_y, right_x, top_y, WIDTH, 0xFFFFFFFF);
				g.draw_line_world(22, 24, right_x, bottom_y, left_x, bottom_y, WIDTH, 0xFFFFFFFF);
				g.draw_line_world(22, 24, right_x, bottom_y, right_x, top_y, WIDTH, 0xFFFFFFFF);
			}
		}
	}
	
}

class RoomTemplate {
	[position,mode:world,layer:19,y:y1] int x1 = 0;
	[hidden] int y1 = 0;
	[position,mode:world,layer:19,y:y2] int x2 = 0;
	[hidden] int y2 = 0;
	[boolean] bool show_rectangle = true;
	
	array<array<array<tileinfo@>>> room_tiles;
	array<array<tilefilth@>> room_filth;
	
	RoomTemplate() {
	}
	
	//finds the proper orientation of pos1 and pos2
	//and converts unit co-ords to tile co-ords
	int left_x(){
		int left_x;
		int dx1, dx2;
		
		//I have to do some dumb math to get everything to round
		//to the top left of the tile, because default behaviour
		//causes things to round toward (0, 0)
		dx1 = (x1%48+48)%48;
		dx2 = (x2%48+48)%48;
		
		if(x1 < x2) {
			left_x = (x1-dx1)/48;
		} else {
			left_x = (x2-dx2)/48;
		}
		
		return left_x;
	}
	
	int right_x(){
		int right_x;
		int dx1, dx2;
		
		dx1 = (x1%48+48)%48;
		dx2 = (x2%48+48)%48;
		
		if(x1 < x2) {
			right_x = (x2-dx2)/48;
		} else {
			right_x = (x1-dx1)/48;
		}
		
		return right_x;
	}
	
	int top_y(){
		int top_y;
		int dy1, dy2;
		
		dy1 = (y1%48+48)%48;
		dy2 = (y2%48+48)%48;
		
		if(y1 < y2) {
			top_y = (y1-dy1)/48;
		} else {
			top_y = (y2-dy2)/48;
		}
		
		return top_y;
	}
	
	int bottom_y(){
		int bottom_y;
		int dy1, dy2;
		
		dy1 = (y1%48+48)%48;
		dy2 = (y2%48+48)%48;
		
		if(y1 < y2) {
			bottom_y = (y2-dy2)/48;
		} else {
			bottom_y = (y1-dy1)/48;
		}
		
		return bottom_y;
	}
	
	uint width(){
		return right_x() - left_x() + 1;
	}
	
	uint height(){
		return bottom_y() - top_y() +1;
	}
	
	//the number of cells wide and tall this room uses.
	//determines the number of connections that are possible
	uint num_cells_w(int cell_width){
		int num_cells_w = width() / cell_width;
		if(width() % cell_width > 0)
			num_cells_w++;
		return num_cells_w;
	}
	
	uint num_cells_h(int cell_height){
		int num_cells_h = height() / cell_height;
		if(height() % cell_height > 0)
			num_cells_h++;
		return num_cells_h;
	}
	
	void build_template(uint cell_width, uint cell_height, scene@ g) {
		//I dont want to re-calculate them every time
		//maybe this is a premature optimization?
		int top_y = this.top_y();
		int left_x = this.left_x();
		uint width = this.width();
		uint height = this.height();
		tileinfo@ tile;
		tilefilth@ filth;
		
		//copies the tiles of the room to make a template
		room_tiles =
			array<array<array<tileinfo@>>>(NUM_LAYERS,
				array<array<tileinfo@>>(num_cells_h(cell_height) * cell_height,
					array<tileinfo@>(num_cells_w(cell_width) * cell_width,
						null)));
		room_filth =
			array<array<tilefilth@>>(num_cells_h(cell_height) * cell_height,
				array<tilefilth@>(num_cells_w(cell_width) * cell_width,
					null));
		
		for(uint l = 0; l < room_tiles.length(); l++) {
			if(l+MIN_LAYER == 18)
				continue;
			for(uint y = 0; y < room_tiles[0].length(); y++) {
				for(uint x = 0; x < room_tiles[0][0].length(); x++) {
					if(x < width && y < height) {
						@tile = g.get_tile(left_x+x, top_y+y, l+MIN_LAYER);
						if(tile.solid()) {
							@room_tiles[l][y][x] = tile;
						}
						if(l+MIN_LAYER == 19) {
							@filth = g.get_tile_filth(left_x+x, top_y+y);
							if(filth.top() > 0 || filth.bottom() > 0 || filth.left() > 0 || filth.right() > 0)
								@room_filth[y][x] = filth;
						}
					} else {
						//fill any excess space with a copy of a tile
						@room_tiles[l][y][x] = room_tiles[l][0][0];
					}
				}
			}
		}
		// puts("height="+room_tiles.length()+" width="+room_tiles[0].length());
	}
	
	void build_room(int left_x, int top_y, scene@ g) {
		tileinfo@ air_tile = create_tileinfo();
		air_tile.solid(false);
		tilefilth@ no_filth = create_tilefilth();
		
		for(uint l = 0; l < room_tiles.length(); l++) {
			if(l+MIN_LAYER == 18)
				continue;
			for(uint dy = 0; dy < room_tiles[l].length(); dy++) {
				for(uint dx = 0; dx < room_tiles[l][dy].length(); dx++) {
					if(@room_tiles[l][dy][dx] != null) {
						g.set_tile(left_x+dx, top_y+dy, l+MIN_LAYER, room_tiles[l][dy][dx], true);
						if(l+MIN_LAYER == 19) {
							if(@room_filth[dy][dx] != null)
								g.set_tile_filth(left_x+dx, top_y+dy, room_filth[dy][dx]);
							else
								g.set_tile_filth(left_x+dx, top_y+dy, no_filth);
						}
					} else {
						g.set_tile(left_x+dx, top_y+dy, l+MIN_LAYER, air_tile, true);
					}
				}
			}
		}
	}
	
}

