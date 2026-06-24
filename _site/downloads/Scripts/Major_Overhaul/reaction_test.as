#include "../lib/SeedGenerator.as"
class script : callback_base {

	scene@ g;
	dustman@ player;
	camera@ cam;
	SeedGenerator s;
	bool seed_set;
	
	[position, mode:WORLD, layer:19, y:cam_y]
    float cam_x;
    [hidden]
    float cam_y;
	
	[text] float cam_scale;
	
	int done, goal, fun, topress, fue; //fun stands for "frames until next"
	
	canvas@ can;
	textfield@ tex;
	
	script() {
		@g = get_scene();
		
		@can = create_canvas(true, 22, 15);
		@tex = create_textfield();
			tex.set_font("ProximaNovaReg", 72);
			tex.align_horizontal(0);
			tex.align_vertical(0);
	}
	
	void on_level_start() {
		@player = controller_controllable(0).as_dustman();
		@cam = get_active_camera();
		cam.script_camera(true);
		fix_cam();
		
		seed_set = false;
		done = 0;
		goal = 25;
		fun = 69;
		topress = -1;
		fue = -1;
	}
	
	void step(int entities) {
		if (!seed_set) {
			if (s.ready()) {
				srand(s.getSeed());
				seed_set = true;
			} else {
				s.step();
			}
		}
		if (fun > 0) {
			fun--;
			if (fun == 0) {
				topress = int32(rand() % 9);
				fun = -1;
			}
		}
		
		if (player.dash_intent() == 1) {
			player.dash_intent(2);
			pressed_button(0);
		}
		if (player.jump_intent() == 1) {
			player.jump_intent(2);
			pressed_button(1);
		}
		if (player.light_intent() == 10) {
			player.light_intent(11);
			pressed_button(2);
		}
		if (player.heavy_intent() == 10) {
			player.heavy_intent(11);
			pressed_button(3);
		}
		if (player.taunt_intent() == 1) {
			player.taunt_intent(2);
			pressed_button(4);
		}
		if (player.x_intent() == -1) {
			player.x_intent(0);
			pressed_button(5);
		}
		if (player.x_intent() == 1) {
			player.x_intent(0);
			pressed_button(6);
		}
		if (player.y_intent() == -1) {
			player.y_intent(0);
			pressed_button(7);
		}
		if (player.y_intent() == 1) {
			player.y_intent(0);
			pressed_button(8);
		}
		
		if (fue > 0) {
			fue--;
			if (fue == 0) {
				g.end_level(0,0);
				fue = -2;
			}
		}
		
		string toprint = "";
		if (fue != -1) {
			toprint = "Complete!";
		}
		if (fun != -1) {
			toprint = "Wait...";
		} else if (fue == -1) {
			toprint = "Press ";
			switch (topress) {
				case 0:
					toprint = toprint + "dash!";
					break;
				case 1:
					toprint = toprint + "jump!";
					break;
				case 2:
					toprint = toprint + "light!";
					break;
				case 3:
					toprint = toprint + "heavy!";
					break;
				case 4:
					toprint = toprint + "taunt!";
					break;
				case 5:
					toprint = toprint + "left!";
					break;
				case 6:
					toprint = toprint + "right!";
					break;
				case 7:
					toprint = toprint + "up!";
					break;
				case 8:
					toprint = toprint + "down!";
					break;
			}
		}
		tex.text(toprint);
		
		player.skill_combo((100 * done) / goal);
	}
	
	void draw(float subframe) {
		can.draw_text(tex, 0, 0, 1, 1, 0);
	}
	
	void fix_cam() {
		cam.x(cam_x);
		cam.y(cam_y);
		cam.prev_x(cam_x);
		cam.prev_y(cam_y);
		if (cam_scale > 0) {
			cam.scale_x(1080 / cam_scale);
			cam.scale_y(1080 / cam_scale);
			cam.prev_scale_x(1080 / cam_scale);
			cam.prev_scale_y(1080 / cam_scale);
		}
	}
	
	void pressed_button(int b) {
		if (fun == -1) {
			if (topress == b) {
				fun = 31;
				done++;
				if (done == goal) {
					fue = 3;
					fun = -1;
				}
			} else {
				player.kill(true);
			}
		}
	}

}