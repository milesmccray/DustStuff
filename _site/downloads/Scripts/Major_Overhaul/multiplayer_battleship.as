class script : callback_base {

	scene@ g;
	dustman@ p1, p2;
	camera@ cam1, cam2;
	
	canvas@ text_can;
	textfield@ p1text, p2text;
	[text] pos p1textloc1;
	[text] pos p1textloc2;
	[text] pos p2textloc1;
	[text] pos p2textloc2;
	
	int gamemode; //0 for outside the game, 1 for placing ships, 2 for guessing, 3 for done
	bool is2p; //if there's two players or more
	int turn;
	
	[text] pos p1loc;
	[text] pos p1cam;
	[text] pos p2loc;
	[text] pos p2cam;
	[text] float scr_height;
	
	[entity] uint text1; //text for solo
	[entity] uint text2; //text for multi
	[entity] uint entrance;
	
	bool in_teleporter;
	
	[text] array<ship> p1ships;
	[text] array<ship> p2ships;
	
	//0 is empty, 1 is a ship, 2 is a miss, 3 is a hit
	array<array<int>> board1(10, array<int>(10));
	array<array<int>> board2(10, array<int>(10));
	
	int p1cx, p1cy, p2cx, p2cy;
	int p1cs, p2cs;
	bool p1ready, p2ready;
	
	bool p1xlock, p1ylock, p2xlock, p2ylock, p1jlock, p2jlock;
	[text] pos p1tl;
	[text] pos p2tl;
	
	[text] pos p1mtl;
	[text] pos p1mtl2;
	[text] pos p2mtl;
	[text] pos p2mtl2;
	
	int endtimer;
	int winner;
	
	script() {
		@g = get_scene();
		add_broadcast_receiver('in_tele', this, 'in_tele');
		
		@text_can = create_canvas(false, 20, 8);
		@p1text = create_textfield();
			p1text.set_font("ProximaNovaReg", 100);
			p1text.align_horizontal(0);
			p1text.align_vertical(0);
		@p2text = create_textfield();
			p2text.set_font("ProximaNovaReg", 100);
			p2text.align_horizontal(0);
			p2text.align_vertical(0);
	}
	
	void in_tele(string id, message@ msg) {
		in_teleporter = true;
	}
	
	void on_level_start() {
		@p1 = controller_controllable(0).as_dustman();
		@cam1 = get_camera(0);
		gamemode = 0;
		
		if (num_cameras() > 1) {
			@p2 = controller_controllable(1).as_dustman();
			@cam2 = get_camera(1);
			is2p = true;
			g.remove_entity(entity_by_id(text1));
		} else {
			is2p = false;
			g.remove_entity(entity_by_id(text2));
			g.remove_entity(entity_by_id(entrance));
		}
		
		
		for (int i = 0; i < 10; i++) {
			for (int j = 0; j < 10; j++) {
				board1[i][j] = 0;
				board2[i][j] = 0;
			}
		}
		
		endtimer = 5;
		winner = -1;
	}
	
	void step(int entities) {
		if (p1.y_intent() == -1 && in_teleporter) {
			init_battleship();
		}
		in_teleporter = false;
		
		if (gamemode > 0) {
			//all battleship function in here, gmaemode must be over 0 to do battleship stuff
			gamecams();
			g.disable_score_overlay(true);
			
			if (p1.x_intent() == -1) {
				if (!p1xlock && p1cx > 0) {
					move_left(1);
				}
				p1xlock = true;
			}
			if (p1.x_intent() == 1) {
				if (!p1xlock && p1cx < 9) {
					move_right(1);
				}
				p1xlock = true;
			}
			if (p1.y_intent() == -1) {
				if (!p1ylock && p1cy > 0) {
					move_up(1);
				}
				p1ylock = true;
			}
			if (p1.y_intent() == 1) {
				if (!p1ylock && p1cy < 9) {
					move_down(1);
				}
				p1ylock = true;
			}
			if (p1.x_intent() == 0)
				p1xlock = false;
			if (p1.y_intent() == 0)
				p1ylock = false;
			if (p1.light_intent() == 10 && !p1ready) 
				p1ships[p1cs].rotate();
			if (p1.jump_intent() == 1) {
				if (!p1ready)
					place_ship(1);
				else if (gamemode == 2 && turn == 1) {
					fire(1);
				}
			}
			
			if (is2p) {
				if (p2.x_intent() == -1) {
					if (!p2xlock && p2cx > 0) {
						move_left(2);
					}
					p2xlock = true;
				}
				if (p2.x_intent() == 1) {
					if (!p2xlock && p2cx < 9) {
						move_right(2);
					}
					p2xlock = true;
				}
				if (p2.y_intent() == -1) {
					if (!p2ylock && p2cy > 0) {
						move_up(2);
					}
					p2ylock = true;
				}
				if (p2.y_intent() == 1) {
					if (!p2ylock && p2cy < 9) {
						move_down(2);
					}
					p2ylock = true;
				}
				if (p2.x_intent() == 0)
					p2xlock = false;
				if (p2.y_intent() == 0)
					p2ylock = false;
				if (p2.light_intent() == 10 && !p2ready) 
					p2ships[p2cs].rotate();
				if (p2.jump_intent() == 1) {
					if (!p2ready)
						place_ship(2);
					else if (gamemode == 2 && turn == 2) {
						fire(2);
					}
				}
			}
			
			update_text();
			
			disable_controls(p1);
			if (is2p) {
				disable_controls(p2);
			}
		}
		
		if (endtimer < 4) {
			endtimer--;
			if (endtimer == 0)
				g.end_level(0, 0);
		}
	}
	
	void move_right(int player) {
		if (player == 1) {
			if (!p1ready) {
				p1ships[p1cs].move(1, 0);
				p1cx = p1ships[p1cs].x();
			} else {
				if (p1cx < 9)
					p1cx++;
			}
		} else if (player == 2) {
			if (!p2ready) {
				p2ships[p2cs].move(1, 0);
				p2cx = p2ships[p2cs].x();
			} else if (gamemode == 2) {
				if (p2cx < 9)
					p2cx++;
			}
		}
	}
	
	void move_left(int player) {
		if (player == 1) {
			if (!p1ready) {
				p1ships[p1cs].move(-1, 0);
				p1cx = p1ships[p1cs].x();
			} else {
				if (p1cx > 0)
					p1cx--;
			}
		} else if (player == 2) {
			if (!p2ready) {
				p2ships[p2cs].move(-1, 0);
				p2cx = p2ships[p2cs].x();
			} else {
				if (p2cx > 0)
					p2cx--;
			}
		}
	}
	
	void move_down(int player) {
		if (player == 1) {
			if (!p1ready) {
				p1ships[p1cs].move(0, 1);
				p1cy = p1ships[p1cs].y();
			} else {
				if (p1cy < 9)
					p1cy++;
			}
		} else if (player == 2) {
			if (!p2ready) {
				p2ships[p2cs].move(0, 1);
				p2cy = p2ships[p2cs].y();
			} else {
				if (p2cy < 9)
					p2cy++;
			}
		}
	}
	
	void move_up(int player) {
		if (player == 1) {
			if (!p1ready) {
				p1ships[p1cs].move(0, -1);
				p1cy = p1ships[p1cs].y();
			} else {
				if (p1cy > 0)
					p1cy--;
			}
		} else if (player == 2) {
			if (!p2ready) {
				p2ships[p2cs].move(0, -1);
				p2cy = p2ships[p2cs].y();
			} else {
				if (p2cy > 0)
					p2cy--;
			}
		}
	}
	
	void disable_controls(dustman@ player) {
		player.x_intent(0);
		player.y_intent(0);
		if (player.light_intent() == 10)
			player.light_intent(11);
		if (player.heavy_intent() == 10)
			player.heavy_intent(11);
		if (player.dash_intent() == 1)
			player.dash_intent(2);
		if (player.jump_intent() == 1)
			player.jump_intent(2);
		if (player.taunt_intent() == 1)
			player.taunt_intent(2);
	}
	
	void draw(float subframe) {
		if (gamemode > 0) {
			draw_cursors();
			draw_ships();
			if (p1ready) {
				text_can.draw_text(p1text, p1textloc2.x(), p1textloc2.y(), 1, 1, 0);
			} else {
				text_can.draw_text(p1text, p1textloc1.x(), p1textloc1.y(), 1, 1, 0);
			}
			if (p2ready) {
				text_can.draw_text(p2text, p2textloc2.x(), p2textloc2.y(), 1, 1, 0);
			} else {
				text_can.draw_text(p2text, p2textloc1.x(), p2textloc1.y(), 1, 1, 0);
			}
		}
		if (gamemode > 1) {
			draw_boards();
		}
	}
	
	void draw_ships() {
		for (int i = 0; i < p1ships.size(); i++) {
			if (p1ships[i].x() > -1) {
				p1ships[i].draw(p1tl, g);
			}
			if (p2ships[i].x() > -1) {
				p2ships[i].draw(p2tl, g);
			}
			
			p1ships[i].draw_mini(p1mtl, g, i, false);
			p2ships[i].draw_mini(p1mtl2, g, i, true);
			p1ships[i].draw_mini(p2mtl2, g, i, true);
			p2ships[i].draw_mini(p2mtl, g, i, false);
		}
	}
	
	void draw_cursors() {
		uint color = 0x661050ff;
		if (!p1ready)
			g.draw_rectangle_world(20, 4, p1tl.x() + 96 * p1cx, p1tl.y() + 96 * p1cy, p1tl.x() + 96 * (p1cx + 1) - 1, p1tl.y() + 96 * (p1cy + 1), 0, color);
		else
			g.draw_rectangle_world(20, 4, p1tl.x() + 96 * (11 + p1cx), p1tl.y() + 96 * p1cy, p1tl.x() + 96 * (p1cx + 12) - 1, p1tl.y() + 96 * (p1cy + 1), 0, color);
		if (!p2ready)
			g.draw_rectangle_world(20, 4, p2tl.x() + 96 * p2cx, p2tl.y() + 96 * p2cy, p2tl.x() + 96 * (p2cx + 1) - 1, p2tl.y() + 96 * (p2cy + 1), 0, color);
		else
			g.draw_rectangle_world(20, 4, p2tl.x() + 96 * (11 + p2cx), p2tl.y() + 96 * p2cy, p2tl.x() + 96 * (p2cx + 12) - 1, p2tl.y() + 96 * (p2cy + 1), 0, color);
	}
	
	void draw_boards() {
		for (int i = 0; i < 10; i++) {
			for (int j = 0; j < 10; j++) {
				draw_cell(board2[i][j], i, j, p1tl);
				draw_cell(board1[i][j], i, j, p2tl);
				draw_cell(board1[i][j], i + 11, j, p1tl);
				draw_cell(board2[i][j], i + 11, j, p2tl);
			}
		}
	}
	
	void update_text() {
		if (gamemode == 1) {
			if (p1ready)
				p1text.text("wait...");
			else 
				p1text.text("place ships");
			if (p2ready)
				p2text.text("wait...");
			else 
				p2text.text("place ships");
		} else if (gamemode == 2) {
			if (turn == 1) {
				p1text.text("your turn");
				p2text.text("wait...");
			} else {
				p2text.text("your turn");
				p1text.text("wait...");
			}
		} else if (gamemode == 3) {
			if (winner == 1) {
				p1text.text("you win!");
				p2text.text("you lose...");
			} else {
				p2text.text("you win!");
				p1text.text("you lose...");
			}
		}
	}
	
	void draw_cell(int val, int x, int y, pos tl) {
		//val 0 or 1 is nothing, 2 is miss 3 is hit
		if (val == 2) {
			g.draw_rectangle_world(20, 5, tl.x() + 18 + 96 * x, tl.y() + 38 + 96 * y, tl.x() + 78 + 96 * x, tl.y() + 58 + 96 * y, 135, 0xffffffff);
		} else if (val == 3) {
			g.draw_rectangle_world(20, 5, tl.x() + 18 + 96 * x, tl.y() + 18 + 96 * y, tl.x() + 78 + 96 * x, tl.y() + 38 + 96 * y, 0, 0xffff3030);
			g.draw_rectangle_world(20, 5, tl.x() + 18 + 96 * x, tl.y() + 58 + 96 * y, tl.x() + 78 + 96 * x, tl.y() + 78 + 96 * y, 0, 0xffff3030);
			g.draw_rectangle_world(20, 5, tl.x() + 18 + 96 * x, tl.y() + 38 + 96 * y, tl.x() + 38 + 96 * x, tl.y() + 58 + 96 * y, 0, 0xffff3030);
			g.draw_rectangle_world(20, 5, tl.x() + 58 + 96 * x, tl.y() + 38 + 96 * y, tl.x() + 78 + 96 * x, tl.y() + 58 + 96 * y, 0, 0xffff3030);
		}
	}
	
	void init_battleship() {
		p1.x(p1loc.x());
		p1.y(p1loc.y());
		if (is2p) {
			p2.x(p2loc.x());
			p2.y(p2loc.y());
		}
		
		p1cx = 0;
		p1cy = 0;
		p2cx = 0;
		p2cy = 0;
		p1cs = 0;
		p2cs = 0;
		
		p1ships[0].move(1, 1);
		p2ships[0].move(1, 1);
		
		p1xlock = false;
		p1ylock = false;
		p1jlock = false;
		p2xlock = false;
		p2ylock = false;
		p2jlock = false;
		
		p1ready = false;
		p2ready = false;
		
		for (int i = 0; i < p1ships.size(); i++) {
			p1ships[i].init();
			p2ships[i].init();
		}
		
		turn = 1;
		gamemode = 1;
	}
	
	void gamecams() {
		cam1.script_camera(true);
		cam1.x(p1cam.x());
		cam1.prev_x(p1cam.x());
		cam1.y(p1cam.y());
		cam1.prev_y(p1cam.y());
		cam1.scale_x(1080 / scr_height);
		cam1.prev_scale_x(1080 / scr_height);
		cam1.scale_y(1080 / scr_height);
		cam1.prev_scale_y(1080 / scr_height);
		
		if (is2p) {
			cam2.script_camera(true);
			cam2.x(p2cam.x());
			cam2.prev_x(p2cam.x());
			cam2.y(p2cam.y());
			cam2.prev_y(p2cam.y());
			cam2.scale_x(1080 / scr_height);
			cam2.prev_scale_x(1080 / scr_height);
			cam2.scale_y(1080 / scr_height);
			cam2.prev_scale_y(1080 / scr_height);
		}
	}
	
	void place_ship(int p) {
		if (p == 1) {
			if (p1ships[p1cs].can_place(p1ships, p1cs)) {
				p1cx = 0;
				p1cy = 0;
				
				if (p1cs < p1ships.size() - 1) {
					p1cs++;
					p1ships[p1cs].move(1, 1);
				} else {
					p1ready = true;
					p1cs = -1;
					if (p2ready)
						lock_boards();
				}
			}
		} else if (p == 2) {
			if (p2ships[p2cs].can_place(p2ships, p2cs)) {
				p2cx = 0;
				p2cy = 0;
				
				if (p2cs < p2ships.size() - 1) {
					p2cs++;
					p2ships[p2cs].move(1, 1);
				} else {
					p2ready = true;
					p2cs = -1;
					if (p1ready)
						lock_boards();
				}
			}
		}
	}
	
	void fire(int p) {
		if (p == 1) {
			if (board1[p1cx][p1cy] < 2) {
				bool miss = true;
				for (int i = 0; i < p2ships.size(); i++) {
					if (p2ships[i].has(p1cx, p1cy)) {
						miss = false;
						board1[p1cx][p1cy] = 3;
						p2ships[i].hit(p1cx, p1cy);
						hit_prism(p2tl, p1cx, p1cy);
					}
				}
				if (miss)
					board1[p1cx][p1cy] = 2;
			}
			
			turn = 2;
			
			bool killed = true;
			for (int i = 0; i < p2ships.size(); i++) {
				if (!p2ships[i].sunk())
					killed = false;
			}
			if (killed)
				done_game(1);
		} else if (p == 2) {
			if (board2[p2cx][p2cy] < 2) {
				bool miss = true;
				for (int i = 0; i < p1ships.size(); i++) {
					if (p1ships[i].has(p2cx, p2cy)) {
						miss = false;
						board2[p2cx][p2cy] = 3;
						p1ships[i].hit(p2cx, p2cy);
						hit_prism(p1tl, p2cx, p2cy);
					}
				}
				if (miss)
					board2[p2cx][p2cy] = 2;
			}
			
			turn = 1;
			
			bool killed = true;
			for (int i = 0; i < p1ships.size(); i++) {
				if (!p1ships[i].sunk())
					killed = false;
			}
			if (killed)
				done_game(2);
		}
	}
	
	void lock_boards() {
		gamemode = 2;
		for (int i = 0; i < p1ships.size(); i++) {
			int r1 = p1ships[i].horiz() ? 1 : 0;
			int r2 = p2ships[i].horiz() ? 1 : 0;
			for (int j = 0; j < p1ships[i].size(); j++) {
				place_prism(p1tl, p1ships[i].x() + j * r1, p1ships[i].y() + j * (1 - r1));
				place_prism(p2tl, p2ships[i].x() + j * r2, p2ships[i].y() + j * (1 - r2));
			}
		}
	}
	
	void place_prism(pos tl, int x, int y) {
		entity@ prism = create_entity("enemy_tutorial_square");
		prism.x(tl.x() + 48 + 96 * x);
		prism.y(tl.y() + 48 + 96 * y);
		prism.as_hittable().scale(0.7);
		g.add_entity(@prism);
	}
	
	void hit_prism(pos tl, int x, int y) {
		hitbox@ h = create_hitbox(p1.as_controllable(), 0, tl.x() + 48 + 96 * x, tl.y() + 48 + 96 * y, -5, 5, -5, 5);
		g.add_entity(h.as_entity(), false);
	}
	
	void done_game(int win) {
		gamemode = 3;
		endtimer = 3;
		winner = win;
	}
	
	void editor_draw(float subframe) {
		/*text_can.draw_text(p1text, p1textloc1.x(), p1textloc1.y(), 1, 1, 0);
		text_can.draw_text(p1text, p1textloc2.x(), p1textloc2.y(), 1, 1, 0);
		text_can.draw_text(p2text, p2textloc1.x(), p2textloc1.y(), 1, 1, 0);
		text_can.draw_text(p2text, p2textloc2.x(), p2textloc2.y(), 1, 1, 0);*/
	}
	
	void editor_step() {
		/*p1text.text("place ships");
		p2text.text("place ships");*/
	}

}

class ship {
	
	int X, Y;
	[text] uint s;
	bool right;
	array<bool> hits;
	
	ship() {
		X = -1;
		Y = -1;
		right = true;
	}
	
	void init() {
		array<bool> temp(s);
		for (int i = 0; i < temp.size(); i++)
			temp[i] = false;
		hits = temp;
	}
	
	int x() {
		return X;
	}
	
	int y() {
		return Y;
	}
	
	int size() {
		return s;
	}
	
	bool horiz() {
		return right;
	}
	
	bool sunk() {
		for (int i = 0; i < hits.size(); i++) {
			if (!hits[i])
				return false;
		}
		return true;
	}
	
	void move(int dx, int dy) {
		X += dx;
		Y += dy;
	}
	
	bool can_place(array<ship> ships, int index) {
		return check_spot(X, Y, right, ships, index);
	}
	
	void rotate() {
		right = !right;
	}
	
	bool has(int x, int y) {
		if (right) {
			return Y == y && X <= x && X + s - 1 >= x;
		} else {
			return X == x && Y <= y && Y + s - 1 >= y;
		}
	}
	
	void hit(int x, int y) {
		if (has(x, y)) {
			hits[x - X + y - Y] = true;
		}
	}
	
	bool check_spot(int new_x, int new_y, bool new_right, array<ship> ships, int index) {
		int temp = new_right ? 1 : 0;
		if (new_x < 0 || new_y < 0 || new_x + (temp * (s - 1)) > 9 || new_y + ((1 - temp) * (s - 1)) > 9)
			return false;
		for (int i = 0; i < ships.size(); i++) {
			if (i != index && ships[i].x() > -1) {
				int othertemp = ships[i].horiz() ? 1 : 0;
				for (int j = 0; j < ships[i].size(); j++) {
					for (int k = 0; k < s; k++) {
						if (new_x + (temp * k) == ships[i].x() + (othertemp * j) && new_y + ((1 - temp) * k) == ships[i].y() + ((1 - othertemp) * j))
							return false;
					}
				}
			}
		}
		return true;
	}
	
	void draw(pos tl, scene@ g) {
		int r = right ? 1 : 0;
		uint color = 0xaac93127;
		g.draw_rectangle_world(18, 1, 8 + tl.x() + 96 * X, 8 + tl.y() + 96 * Y, tl.x() + 96 * (1 + X + (r * (s - 1))) - 8, tl.y() + 96 * (1 + Y + ((1 - r) * (s - 1))) - 8, 0, color);
	}
	
	void draw_mini(pos tl, scene@ g, int index, bool opponent) {
		for (int i = 0; i < hits.size(); i++) {
			uint color = 0xffffffff;
			if (hits[i] && (!opponent || sunk()))
				color = 0xffff0000;
			g.draw_rectangle_world(20, 2, tl.x() + 23 * i, 23 * index + tl.y(), tl.x() + 23 * i + 20, 23 * index + tl.y() + 20, 0, color);
		}
	}
	
}

class teleporter : trigger_base {
	
	teleporter() {
		
	}
	
	void activate(controllable@ e) {
		if (e.player_index() == 0) {
			message@ msg = create_message();
			broadcast_message('in_tele', msg);
		}
	}
	
	
}

class pos {
	
	[position,mode:world,layer:19,y:Y] float X;
	[hidden] float Y;
	
	pos() {
		
	}
	
	float x() {
		return X;
	}
	
	float y() {
		return Y;
	}
	
}