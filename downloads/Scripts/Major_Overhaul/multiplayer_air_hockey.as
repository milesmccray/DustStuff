#include "../lib/pos.as"
#include "../lib/drawing/circle.cpp"
class script : callback_base {

	scene@ g;
	dustman@ p1, p2;
	
	float puckX, puckY;
	float puckXspeed, puckYspeed;
	
	[text] posHud topleft;
	[text] posHud botright;
	
	float p1x, p2x, p1y, p2y;
	float p1lx, p2lx, p1ly, p2ly; //paddle last frame positions to calculate velocity
	float radius = 44;
	float pradius = 28;
	float goalheight = 90;
	
	bool goalmode;
	int p1score, p2score;
	
	int endtimer;
	
	canvas@ can;
	textfield@ scoreText1, scoreText2;
	
	script() {
		@g = get_scene();
		
		@can = create_canvas(true, 20, 15);
		@scoreText1 = create_textfield();
		scoreText1.set_font("ProximaNovaReg", 72);
        scoreText1.align_horizontal(1);
        scoreText1.align_vertical(-1);
		scoreText1.text("0");
		@scoreText2 = create_textfield();
		scoreText2.set_font("ProximaNovaReg", 72);
        scoreText2.align_horizontal(-1);
        scoreText2.align_vertical(-1);
		scoreText2.text("0");
	}
	
	void on_level_start() {
		@p1 = controller_controllable(0).as_dustman();
		if (num_cameras() > 1)
			@p2 = controller_controllable(1).as_dustman();
		puckX = 0;
		puckY = 0;
		
		g.disable_score_overlay(true);
		
		puckXspeed = 0;
		puckYspeed = 0;
		
		p1x = -600;
		p1y = 0;
		p1lx = -600;
		p1ly = 0;
		p2x = 600;
		p2y = 0;
		p2lx = 600;
		p2ly = 0;
		
		goalmode = false;
		p1score = p2score = 0;
		endtimer = -2;
	}
	
	void step(int entities) {
		p1.x_intent(0);
		p1.y_intent(0);
		p1.jump_intent(0);
		if (num_cameras() > 1) {
			p2.x_intent(0);
			p2.y_intent(0);
			p2.jump_intent(0);
		}
		
		
		if (checkHit(1)) {
			float relXvel = p1x - p1lx - puckXspeed;
			float relYvel = p1y - p1ly - puckYspeed;
			float dx = puckX - p1x;
			float dy = puckY - p1y;
			float dscal = sqrt(dx * dx + dy * dy);
			dx /= dscal;
			dy /= dscal;
			float scalar = relXvel * dx + relYvel * dy;
			puckXspeed += scalar * dx;
			puckYspeed += scalar * dy;
			puckX = p1x + (radius + pradius) * dx;
			puckY = p1y + (radius + pradius) * dy;
		}
		if (checkHit(2)) {
			float relXvel = p2x - p2lx - puckXspeed;
			float relYvel = p2y - p2ly - puckYspeed;
			float dx = puckX - p2x;
			float dy = puckY - p2y;
			float dscal = sqrt(dx * dx + dy * dy);
			dx /= dscal;
			dy /= dscal;
			float scalar = relXvel * dx + relYvel * dy;
			puckXspeed += scalar * dx;
			puckYspeed += scalar * dy;
			puckX = p2x + (radius + pradius) * dx;
			puckY = p2y + (radius + pradius) * dy;
		}
		
		puckX += puckXspeed;
		puckY += puckYspeed;
		float sqpuckvel = puckXspeed * puckXspeed + puckYspeed * puckYspeed;
		puckXspeed *= 1 - sqpuckvel / 100000;
		puckYspeed *= 1 - sqpuckvel / 100000;
		
		p1lx = p1x;
		p1ly = p1y;
		p1x = g.mouse_x_hud(0);
		p1x -= p1x % 1;
		if (p1x > -1 * radius)
			p1x = -1 * radius;
		if (p1x < topleft.x() + radius)
			p1x = topleft.x() + radius;
		p1y = g.mouse_y_hud(0);
		p1y -= p1y % 1;
		if (p1y > botright.y() - radius)
			p1y = botright.y() - radius;
		if (p1y < topleft.y() + radius)
			p1y = topleft.y() + radius;
		if (num_cameras() > 1) {
			p2lx = p2x;
			p2ly = p2y;
			p2x = g.mouse_x_hud(1);
			p2x -= p2x % 1;
			if (p2x < radius)
				p2x = radius;
			if (p2x > botright.x() - radius)
				p2x = botright.x() - radius;
			p2y = g.mouse_y_hud(1);
			p2y -= p2y % 1;
			if (p2y > botright.y() - radius)
				p2y = botright.y() - radius;
			if (p2y < topleft.y() + radius)
				p2y = topleft.y() + radius;
		}
		
		if (puckX - pradius <= topleft.x() || puckX + pradius >= botright.x()) {
			if (abs(puckY) < goalheight) {
				if (abs(puckX) > botright.x() - pradius / 1.4)
					goalmode = true;
			} else if (!goalmode) {
				puckXspeed *= -1;
				puckX = max(puckX, topleft.x() + pradius);
				puckX = min(puckX, botright.x() - pradius);
			}
		}
		if (puckY - pradius <= topleft.y() || puckY + pradius >= botright.y()) {
			puckYspeed *= -1;
			puckY = max(puckY, topleft.y() + pradius);
			puckY = min(puckY, botright.y() - pradius);
		}
		
		if (goalmode && endtimer == -2) {
			if (abs(puckY) > goalheight - pradius / 1.4) {
				puckYspeed *= -1;
				puckY = max(puckY, -1 * goalheight - pradius / 1.4);
				puckY = min(puckY, goalheight + pradius / 1.4);
			}
			
			if (abs(puckX) > botright.x() + pradius) {
				goalmode = false;
				puckXspeed = puckYspeed = puckY = 0;
				if (puckX < 0) {
					p2score++;
					scoreText2.text("" + p2score);
					if (p2score >= 5) {
						endtimer = 3;
						puckX = 0;
					} else {
						puckX = -400;
					}
				} else {
					p1score++;
					scoreText1.text("" + p1score);
					if (p1score >= 5) {
						endtimer = 3;
						puckX = 0;
					} else {
						if (num_cameras() > 1)
							puckX = 400;
						else 
							puckX = -400;
					}
				}
			}
		}
		
		if (endtimer >= 0) {
			endtimer--;
			if (endtimer == 0)
				g.end_level(0, 0);
		}
	}
	
	bool checkHit(int player) {
		if (player == 1) {
			float dx = p1x - puckX;
			float dy = p1y - puckY;
			return sqrt(dx * dx + dy * dy) <= radius + pradius;
		} else if (player == 2) {
			float dx = p2x - puckX;
			float dy = p2y - puckY;
			return sqrt(dx * dx + dy * dy) <= radius + pradius;
		}
		return false;
	}
	
	void draw(float subframe) {
		if (endtimer == -2) {
			drawing::fill_circle(g, 12, 2, p1x, p1y, radius, 26, 0xFFFF0000, 0xFFBB0000, false, 1);
			drawing::fill_circle(g, 12, 2, p2x, p2y, radius, 26, 0xFF0000FF, 0xFF0000BB, false, 1);
			
			drawing::fill_circle(g, 12, 3, p1x, p1y, 19, 26, 0xFFCC0000, 0xFFAA0000, false, 1);
			drawing::fill_circle(g, 12, 3, p2x, p2y, 19, 26, 0xFF0000CC, 0xFF0000AA, false, 1);
			
			drawing::fill_circle(g, 12, 1, puckX, puckY, pradius, 26, 0xFFFF9900, 0xFFCC8800, false, 1);
		}
		
		g.draw_rectangle_hud(19, 17, -860, goalheight + pradius / 1.4, -758, -1 * goalheight - pradius / 1.4, 0, 0xFF000000);
		g.draw_rectangle_hud(19, 17, 860, goalheight + pradius / 1.4, 758, -1 * goalheight - pradius / 1.4, 0, 0xFF000000);
		
		can.draw_text(scoreText1, -60, 370, 1, 1, 0);
		can.draw_text(scoreText2, 60, 370, 1, 1, 0);
	}
	
	void editor_draw(float subframe) {
		/*drawing::fill_circle(g, 12, 2, -200, 100, 48, 26, 0xFFFF0000, 0xFFBB0000, false, 1);
		drawing::fill_circle(g, 12, 2, 200, -50, 48, 26, 0xFF0000FF, 0xFF0000BB, false, 1);
		
		drawing::fill_circle(g, 12, 3, -200, 100, 20, 26, 0xFFCC0000, 0xFFAA0000, false, 1);
		drawing::fill_circle(g, 12, 3, 200, -50, 20, 26, 0xFF0000CC, 0xFF0000AA, false, 1);
		
		drawing::fill_circle(g, 12, 1, 50, -100, 31, 25, 0xFFFF9900, 0xFFCC8800, false, 1);*/
	}

}