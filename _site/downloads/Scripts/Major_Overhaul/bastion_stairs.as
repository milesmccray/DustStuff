#include "../lib/SeedGenerator.as"
class script : callback_base {

	scene@ g;
	dustman@ player;
	camera@ cam;
	
	uint seed;
	[text] SeedGenerator s;
	bool trySeed, facingRight;
	
	[text] uint goal = 20;
	array<int> stairs;
	
	uint currentStair;
	[text] int stairWidth = 75;
	[text] int stairHeight = 50;
	
	int eft;
	
	script() {
		@g = get_scene();
		seed = 0;
		s = SeedGenerator();
		trySeed = true;
		currentStair = 0;
		facingRight = true;
		eft = 0;
	}
	
	void on_level_start() {
		@player = controller_controllable(0).as_dustman();
		
		@cam = get_active_camera();
		camera_goto(72, -300, 1080);
		cam.script_camera(true);
	}
	
	void camera_goto(float x, float y, float h) {
		cam.x(x);
		cam.prev_x(x);
		cam.y(y);
		cam.prev_y(y);
		cam.scale_x(1080 / h);
		cam.prev_scale_x(1080 / h);
		cam.scale_y(1080 / h);
		cam.prev_scale_y(1080 / h);
	}
	
	void step(int entities) {
		// Seeding stuff
		if (trySeed) {
			if (!s.ready())
				s.step();
			else {
				seed = s.getSeed();
				srand(seed);
				trySeed = false;
				makeStairs();
			}
		}
		
		if (currentStair >= goal - 1) {
			if (eft == 3)
				win();
			eft++;
		}
		
		player.x_intent(0);
		player.y_intent(0);
		player.light_intent(0);
		player.heavy_intent(0);
		player.taunt_intent(0);
		if (player.jump_intent() == 1) {
			if (eft == 0)
				jumpNorm();
			player.jump_intent(2);
		}
		if (player.dash_intent() == 1) {
			if (eft == 0)
				jumpTurn();
			player.dash_intent(2);
		}
	}
	
	void jumpNorm() {
		if (facingRight) {
			if (stairs[currentStair+1] - stairs[currentStair] == 1)
				currentStair++;
			else
				lose();
		}
		else {
			if (stairs[currentStair+1] - stairs[currentStair] == -1)
				currentStair++;
			else
				lose();
		}
	}
	
	void jumpTurn() {
		if (facingRight) {
			if (stairs[currentStair+1] - stairs[currentStair] == -1)
				currentStair++;
			else
				lose();
			
			facingRight = false;
			player.face(-1);
		}
		else {
			if (stairs[currentStair+1] - stairs[currentStair] == 1)
				currentStair++;
			else
				lose();
			
			facingRight = true;
			player.face(1);
		}
	}
	
	void lose() {
		player.kill(true);
	}
	
	void win() {
		g.end_level(0,0);
	}
	
	void makeStairs() {
		array<int> tstairs(goal);
		stairs = tstairs;
		stairs[0] = 0;
		for (uint i = 1; i < goal; i++) {
			stairs[i] = stairs[i-1] + ((rand() % 2) * 2 - 1);
		}
	}
	
	void draw(float subframe) {
		drawStairs();
	}
	
	void drawStairs() {
		for (uint i = currentStair; i < stairs.size(); i++) {
			g.draw_line_world(20, 16, (stairs[i] - stairs[currentStair]) * stairWidth, (currentStair - i) * stairHeight, (stairs[i] - stairs[currentStair] + 1) * stairWidth, (currentStair - i) * stairHeight, 10, 0xFF00FFFF);
		}
	}
	

}