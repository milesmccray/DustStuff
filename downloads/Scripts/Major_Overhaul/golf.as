class script : callback_base {

	scene@ g;
	dustman@ player;
	camera@ cam;
	[entity] uint ball;
	
	float prev_speed;
	int fub; //next frame bounce
	float bfbs, bfbys, afbs; //before bounce speed, after bounce speed
	
	float last_y;
	array<int> stopcheck(7); //checks if the ball has the same speed for 10 frames 
	
	int aim;
	float aim_x1, aim_y1, aim_x2, aim_y2;
	float radcon; //just to convert degrees to radians
	float power;
	int charge;
	bool charging, charge_up;
	
	bool stopped;
	
	int hole, next_hole_timer;
	[text] array<pos> cam_left;
	[text] array<pos> cam_right;
	[text] array<pos> hole_starts;
	
	[text] pos stroke_loc;
	[text] pos total_stroke_loc;
	[text] pos par_loc;
	[text] pos title_loc;
	[text] pos cam_text_loc;
	canvas@ text_canvas;
	textfield@ stroke_text, total_stroke_text, par_text, hole_title, hole_finish, tutorial, cam_text, reset_text;
	int strokes, total_strokes;
	string hole_end_text;
	bool tutor;
	
	[text] array<pos> secret_cam;
	
	array<bool> in_secret = {false,false,false,false,false}; 
	array<bool> got_secret = {false,false,false,false,false};
	
	[entity] uint apple;
	bool can_reset;
	[text] pos reset_spot;
	
	float cam_disp;
	bool cam_mode;
	
	bool ready_to_end;
	int end_timer;
	
	int secret_count;
	
	array<int> par_list = {6, 6, 6, 8, 9};
	array<string> title_list = {"Shaded Fairway", "Li-par-ary", "Development Drive", "Aerodynamics Research", "Giga Par-fficult"};
	
	script() {
		@g = get_scene();
		add_broadcast_receiver("OnMyCustomEventName", this, "OnMyCustomEventName");
		
		@text_canvas = create_canvas(true, 22, 15);
		@stroke_text = create_textfield();
			stroke_text.set_font("sans_bold", 26);
			stroke_text.align_horizontal(-1);
			stroke_text.align_vertical(-1);
		@total_stroke_text = create_textfield();
			total_stroke_text.set_font("sans_bold", 26);
			total_stroke_text.align_horizontal(-1);
			total_stroke_text.align_vertical(-1);
		@par_text = create_textfield();
			par_text.set_font("sans_bold", 26);
			par_text.align_horizontal(1);
			par_text.align_vertical(-1);
		@hole_title = create_textfield();
			hole_title.set_font("sans_bold", 26);
			hole_title.align_horizontal(1);
			hole_title.align_vertical(-1);
		@hole_finish = create_textfield();
			hole_finish.set_font("ProximaNovaReg", 72);
			hole_finish.align_horizontal(0);
			hole_finish.align_vertical(0);
		@tutorial = create_textfield();
			tutorial.set_font("sans_bold", 36);
			tutorial.align_horizontal(0);
			tutorial.align_vertical(-1);
		@cam_text = create_textfield();
			cam_text.set_font("sans_bold", 26);
			cam_text.align_horizontal(1);
			cam_text.align_vertical(1);
		@reset_text = create_textfield();
			reset_text.set_font("sans_bold", 36);
			reset_text.align_horizontal(0);
			reset_text.align_vertical(-1);
	}
	
	void OnMyCustomEventName(string id, message@ msg) {
		if (msg.get_string("triggerType") == "hole") {
			finished_hole();
		}
		if (msg.get_string("triggerType") == "secret") {
			if (msg.get_int("entrance") == 1) {
				in_secret[msg.get_int("secretid")] = true;
				got_secret[msg.get_int("secretid")] = true;
				//puts("in_secret \n");
			}
			else 
				in_secret[msg.get_int("secretid")] = false;
				//puts("out_secret \n");
				//puts(""+msg.get_int("secretid"));
		}
		if (msg.get_string("triggerType") == "reset") {
			can_reset = true;
		}
	}
	
	void on_level_start() {
		@player = controller_controllable(0).as_dustman();
		@cam = get_active_camera();
		cam.script_camera(true);
		g.disable_score_overlay(true);
		
		tutor = true;
		tutorial.text("Hold jump to charge");
		cam_text.text("Press heavy to control camera");
		reset_text.text("Press taunt to go back to the mainland");
		
		prev_speed = -1;
		bfbys = -1;
		fub = -1;
		bfbs = -1;
		afbs = -1;
		
		last_y = 6969;
		for (int i = 0; i < 7; i++) {
			stopcheck[i] = -1;
		}
		
		stopped = false;
		
		aim = 0;
		aim_x1 = 0;
		aim_y1 = 0;
		aim_x2 = 0;
		aim_y2 = 0;
		power = 0;
		radcon = 3.1415926535 / 180;
		charge = -1;
		charging = false;
		charge_up = true;
		
		hole = 0;
		next_hole_timer = -1;
		strokes = 0;
		total_strokes = 0;
		
		//in_secret = {false};
		//got_secret = {false};
		can_reset = false;
		
		cam_disp = 0;
		cam_mode = false;
		
		ready_to_end = false;
		end_timer = -1;
		
		secret_count = 0;
	}
	
	void step(int entities) {
		stroke_text.text("Strokes: " + strokes);
		total_stroke_text.text("Total Strokes: " + total_strokes);
		par_text.text("Par " + par_list[hole]);
		hole_title.text("" + title_list[hole]);
		if (end_timer == -1) {
			if (next_hole_timer >= 0 || next_hole_timer <= -2) {
				if (next_hole_timer > 0 && next_hole_timer != 3) {
					next_hole_timer--;
					if (next_hole_timer < 3)
						controllable_by_id(ball).set_speed_xy(0, -1);
				}
				if (next_hole_timer == 3) {
					if (player.light_intent() > 0) {
						next_hole_timer = 0;
						player.light_intent(11);
					}
				} else if (next_hole_timer == 0) {
					if (hole == 4) {
						ready_to_end = true;
						next_hole_timer = -1;
					} else {
						hole++;
						stopped = false;
						controllable_by_id(ball).set_speed_xy(0, -1); 
						controllable_by_id(ball).set_xy(hole_starts[hole - 1].x(), hole_starts[hole - 1].y());
						//hole minus 1 since the first hole doesn't need one
						next_hole_timer = -7;
						aim = 0;
						charge = 0;
						strokes = 0;
					}
				} else if (next_hole_timer <= -2) {
					controllable_by_id(ball).set_speed_xy(0, -1); 
					next_hole_timer++;
				}
			}
			
			//puts(""+got_secret[0]+" "+got_secret[1]+" \n");
			fix_cam(in_secret);
			controllable_by_id(ball).life(4);
			
			fub--;
			
			if (player.taunt_intent() > 0 && can_reset) {
				controllable_by_id(ball).set_xy(reset_spot.x(), reset_spot.y());
				controllable_by_id(ball).set_speed_xy(0, 0); 
				stopped = false;
				strokes++;
				total_strokes++;
				in_secret[4] = false;
				can_reset = false;
			}
			
			if (stopped) {				
				//aiming mode
				controllable_by_id(ball).set_speed_xy(0, 0);
				aim_x1 = controllable_by_id(ball).x();
				aim_y1 = controllable_by_id(ball).y();
				aim_x2 = aim_x1 + 200 * cos((aim - 90) * radcon);
				aim_y2 = aim_y1 + 200 * sin((aim - 90) * radcon);
				
				if (player.heavy_intent() > 0 && player.heavy_intent() < 11 && next_hole_timer == -1) {
					if (cam_mode) {
						cam_mode = false;
						cam_disp = 0;
						cam_text.text("");
					} else {
						cam_mode = true;
						cam_text.text("Press heavy to return");
					}
					player.heavy_intent(11);
				}
				
				if (!cam_mode) {
					if (player.x_intent() == 1) {
						aim += 2;
						player.x_intent(0);
					}
					if (player.x_intent() == -1) {
						aim -= 2;
						player.x_intent(0);
					}
				} else {
					if (player.x_intent() == 1) {
						cam_disp += 20;
						player.x_intent(0);
					}
					if (player.x_intent() == -1) {
						cam_disp -= 20;
						player.x_intent(0);
					}
				}
				
				if (player.jump_intent() > 0 && next_hole_timer == -1) {
					if (charging) {
						if (charge_up) {
							if (charge >= 120) {
								charge = 120;
								charge_up = false;
							} else {
								charge += 2;
							}
						} else {
							if (charge <= 0) {
								charge = 0;
								charge_up = true;
							} else {
								charge -= 2;
							}
						}
					} else {
						charge = 2;
						charging = true;
					}
				} else {
					if (charging) {
						power = 1700 * charge / 120 + 500;
						fire();
						charge = 0;
						charging = false;
					}
				}
			} else {
				//moving mode
				
				if (detect_bounce()) {
					afbs = bfbs * 0.6;
					fub = 1;
				} else if (fub == 0) {
					if (bounce_was_on_ground() && afbs <= 200) {
						stopped = true;
					}
					controllable_by_id(ball).set_speed_direction(int32(afbs), int32(controllable_by_id(ball).direction()));
					fub = -1;
				} else {
					slow_roll();
					if (uphill_roll()) {
						controllable_by_id(ball).set_speed_xy(0, 0);
					}
				}
				
				track_bouncing_speed();
			}
			if (ready_to_end) {
				controllable_by_id(ball).life(0);
				hitbox@ h = create_hitbox(player.as_controllable(), 0, entity_by_id(ball).x(), entity_by_id(ball).y(), -50, 50, -50, 50);
				g.add_entity(h.as_entity(), false);
				ready_to_end = false;
				end_timer = 2;
				
				for(int i = 0; i < 5; i++){
					if(got_secret[i] == true)
						secret_count++;
					else
						break;
				}
				
						
				if (secret_count == 5) {
					hitbox@ hi = create_hitbox(player.as_controllable(), 0, entity_by_id(apple).x(), entity_by_id(apple).y(), -5, 5, -5, 5);
					g.add_entity(hi.as_entity(), false);
				}
			}
		} else if (end_timer >= 0) {
			end_timer--;
			if (end_timer == 0) {
				g.end_level(0, 0);
				end_timer = -2;
			}
		}
		if (player.heavy_intent() > 0) {
			player.heavy_intent(11);
		}
		if (player.light_intent() > 0) {
			player.light_intent(11);
		}
		if (player.dash_intent() > 0) {
			player.dash_intent(2);
		}
	}
	
	void finished_hole() {
		next_hole_timer = 10;
		
		int score = strokes - par_list[hole];
		string sample = "";
		switch (score) {
			case -3:
				sample = sample + "Albatross\n";
				break;
			case -2:
				sample = sample + "Eagle\n";
				break;
			case -1:
				sample = sample + "Birdie\n";
				break;
			case 0:
				sample = sample + "Par\n";
				break;
			case 1:
				sample = sample + "Bogey\n";
				break;
			case 2:
				sample = sample + "Double Bogey\n";
				break;
			case 3:
				sample = sample + "Triple Bogey\n";
				break;
		}
		if (score < 0)
			sample = sample + score;
		else
			sample = sample + "+" + score;
		
		hole_finish.text(sample);
	}
	
	void draw(float subframe) {
		if (end_timer == -1 && next_hole_timer == -1) {
			if (stopped)
				g.draw_line_world(20, 16, aim_x1, aim_y1, aim_x2, aim_y2, 5, 0xFFFF0000);
			if (charging) {
				g.draw_line_world(20, 16, aim_x1 - 125, aim_y1 + 48, aim_x1 + 125, aim_y1 + 48, 5, 0xFF000000); //top
				g.draw_line_world(20, 16, aim_x1 - 125, aim_y1 + 88, aim_x1 + 125, aim_y1 + 88, 5, 0xFF000000); //bottom
				g.draw_line_world(20, 16, aim_x1 - 123, aim_y1 + 46, aim_x1 - 123, aim_y1 + 90, 5, 0xFF000000); //left
				g.draw_line_world(20, 16, aim_x1 + 123, aim_y1 + 46, aim_x1 + 123, aim_y1 + 90, 5, 0xFF000000); //right
				g.draw_rectangle_world(20, 16, aim_x1 - 120, aim_y1 + 50, aim_x1 - 120 + 2 * charge, aim_y1 + 85, 0, 0xFFFFFFFF);
			}
		}
		if (not in_secret[0] and not in_secret[1] and not in_secret[2] and not in_secret[3] and not in_secret[4]) {
			text_canvas.draw_text(stroke_text, stroke_loc.x(), stroke_loc.y(), 1, 1, 0);
			text_canvas.draw_text(total_stroke_text, total_stroke_loc.x(), total_stroke_loc.y(), 1, 1, 0);
			text_canvas.draw_text(par_text, par_loc.x(), par_loc.y(), 1, 1, 0);
			text_canvas.draw_text(hole_title, title_loc.x(), title_loc.y(), 1, 1, 0);
		}
		if (next_hole_timer > 0) {
			text_canvas.draw_text(hole_finish, 0, 0, 1, 1, 0);
			text_canvas.draw_text(tutorial, 0, -440, 1, 1, 0);
		}
		if (tutor) {
			text_canvas.draw_text(tutorial, 0, -440, 1, 1, 0);
		}
		if (can_reset) {
			text_canvas.draw_text(reset_text, 0, -440, 1, 1, 0);
		}
		text_canvas.draw_text(cam_text, cam_text_loc.x(), cam_text_loc.y(), 1, 1, 0);
	}
	
	void editor_step() {
		/*stroke_text.text("Strokes: 0");
		total_stroke_text.text("Total Strokes: 0");
		par_text.text("Par 0");
		hole_title.text("Placeholder Title");
		hole_finish.text("Birdie\n-1");
		cam_text.text("Press heavy to enter camera mode.");*/
	}
	
	void editor_draw(float subframe) {
		/*text_canvas.draw_text(stroke_text, stroke_loc.x(), stroke_loc.y(), 1, 1, 0);
		text_canvas.draw_text(total_stroke_text, total_stroke_loc.x(), total_stroke_loc.y(), 1, 1, 0);
		text_canvas.draw_text(par_text, par_loc.x(), par_loc.y(), 1, 1, 0);
		text_canvas.draw_text(hole_title, title_loc.x(), title_loc.y(), 1, 1, 0);
		text_canvas.draw_text(hole_finish, 0, 0, 1, 1, 0);
		text_canvas.draw_text(cam_text, cam_text_loc.x(), cam_text_loc.y(), 1, 1, 0);*/
	}
	
	void fix_cam(array<bool> secret) {
		
		int temp = 0;
		
		//puts(""+temp);
		
		for(int i=0; i < 5; i++){
			if(secret[i] == true){
				break;
				}
			else{
				if(i == 4)
					temp = -1;
				else
					temp++;
				}
		}
		
		//puts(""+temp);
		
		if (temp == -1) {
			float x = controllable_by_id(ball).x();
			if (x  < cam_left[hole].x()) {
				x = cam_left[hole].x();
			}
			if (x > cam_right[hole].x()) {
				x = cam_right[hole].x();
			}
			if (x + cam_disp < cam_left[hole].x()) {
				cam_disp = cam_left[hole].x() - x;
			}
			if (x + cam_disp > cam_right[hole].x()) {
				cam_disp = cam_right[hole].x() - x;
			}
			cam.x(x + cam_disp);
			cam.prev_x(x + cam_disp);
			cam.y(cam_left[hole].y());
			cam.prev_y(cam_left[hole].y());
			player.x(x);
			float h = 1250;
			cam.scale_x(1080 / h);
			cam.prev_scale_x(1080 / h);
			cam.scale_y(1080 / h);
			cam.prev_scale_y(1080 / h);
		} else {
			cam.x(secret_cam[temp].x());
			cam.prev_x(secret_cam[temp].x());
			cam.y(secret_cam[temp].y());
			cam.prev_y(secret_cam[temp].y());
			player.x(secret_cam[temp].x());
			float h = 700;
			cam.scale_x(1080 / h);
			cam.prev_scale_x(1080 / h);
			cam.scale_y(1080 / h);
			cam.prev_scale_y(1080 / h);
		}
	}
	
	void track_bouncing_speed() {
		if (controllable_by_id(ball).speed() != 0) {
			bfbs = controllable_by_id(ball).speed();
		}
		if (controllable_by_id(ball).y_speed() != 0 && fub <= 0) {
			bfbys = controllable_by_id(ball).y_speed();
		}
	}
	
	bool detect_bounce() {
		if (prev_speed == 0 && controllable_by_id(ball).speed() == 0) {
			prev_speed = -1;
			return true;
		}
		prev_speed = controllable_by_id(ball).speed();
		return false;
	}
	
	bool bounce_was_on_ground() {
		if (bfbys > 0 && controllable_by_id(ball).y_speed() < 0) {
			return true;
		}
		return false;
	}
	
	void slow_roll() {
		if (last_y == controllable_by_id(ball).y() && controllable_by_id(ball).y_speed() > 33 && controllable_by_id(ball).y_speed() < 34) {
			controllable_by_id(ball).set_speed_xy(controllable_by_id(ball).x_speed() * 0.97, controllable_by_id(ball).y_speed());
			if (abs(controllable_by_id(ball).x_speed()) <= 150 && abs(int32(controllable_by_id(ball).x_speed())) > 0) {
				controllable_by_id(ball).set_speed_xy(controllable_by_id(ball).x_speed() * 0.8, controllable_by_id(ball).y_speed());
				if (controllable_by_id(ball).speed() <= 60 && controllable_by_id(ball).speed() > 0)
					stopped = true;
			}
			
		}
		last_y = controllable_by_id(ball).y();
	}
	
	bool uphill_roll() {
		for (int i = 0; i < 6; i++) {
			stopcheck[i] = stopcheck[i + 1];
		}
		stopcheck[6] = int32(controllable_by_id(ball).speed());
		bool allsame = true;
		for (int i = 0; i < 6; i++) {
			if (stopcheck[i] != stopcheck[i + 1] || stopcheck[i] < 0)
				allsame = false;
		}
		return allsame;
	}
	
	void fire() {
		controllable_by_id(ball).set_speed_direction(power, aim);
		stopped = false;
		tutor = false;
		tutorial.text("Press light to continue");
		aim = 0;
		strokes++;
		total_strokes++;
	}

}

class hole : trigger_base {
	
	scene@ g;
	scripttrigger@ self;
	controllable@ trigger_entity;
	[text] int holenum;
	[entity] uint ball;
	bool go_once;

	hole() {
		@g = get_scene();
		go_once = true;
	}

	void init(script@ s, scripttrigger@ self) {
		@this.self = @self;
	}
  
	void activate(controllable@ e) {
		if (e.is_same(controllable_by_id(ball))) {
			if (go_once) {
				go_once = false;
				notifyScript();
			}
		}
	}

	void notifyScript() {
		message@ msg = create_message();
		msg.set_int("holenum", holenum);
		msg.set_string("triggerType","hole");
		broadcast_message("OnMyCustomEventName", msg);
	}
	
	
}

class trig_secret : trigger_base {
	
	scene@ g;
	scripttrigger@ self;
	controllable@ trigger_entity;
	[text] bool entrance;
	[text] uint secretid;
	[text] uint ball = 2286;
	
	trig_secret() {
		@g = get_scene();
	}
	
	void init(script@ s, scripttrigger@ self) {
		@this.self = @self;
	}
  
	void activate(controllable@ e) {
		if (e.is_same(controllable_by_id(ball))) {
			notifyScript();
		}
	}
	
	void notifyScript() {
		message@ msg = create_message();
		if (entrance){
			msg.set_int("entrance", 1);
			msg.set_int("secretid", secretid);
			}
		else {
			msg.set_int("entrance", 0);
			msg.set_int("secretid", secretid);
			}
		msg.set_string("triggerType","secret");
		broadcast_message("OnMyCustomEventName", msg);
	}
	
}

class trig_reset : trigger_base {
	
	scene@ g;
	scripttrigger@ self;
	controllable@ trigger_entity;
	[entity] uint ball;
	
	trig_reset() {
		@g = get_scene();
	}
	
	void init(script@ s, scripttrigger@ self) {
		@this.self = @self;
	}
  
	void activate(controllable@ e) {
		if (e.is_same(controllable_by_id(ball))) {
			notifyScript();
		}
	}
	
	void notifyScript() {
		message@ msg = create_message();
		msg.set_string("triggerType","reset");
		broadcast_message("OnMyCustomEventName", msg);
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