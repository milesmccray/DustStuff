class script : callback_base {

	scene@ g;
	camera@ cam;
	
	float theta = 0;
	float r = 240;
	float sx = 350;
	float sy = -330;
	
	float vspacing = r * 1.5;
	float hspacing = r * sqrt(3);
	
	float pi = 3.141592f;
	float a1, a2, a3, b1, b2, b3;
	int si, ei, sj, ej;
	
	script() {
		@g = get_scene();
	}
	
	void on_level_start() {
		@cam = get_active_camera();
	}
	
	void checkpoint_load() {
		@cam = get_active_camera();
	}
	
	void step(int entities) {
		theta += 0.03;
		a1 = cos(theta);
		b1 = sin(theta);
		a2 = cos(theta + 2*pi/3);
		b2 = sin(theta + 2*pi/3);
		a3 = cos(theta + 4*pi/3);
		b3 = sin(theta + 4*pi/3);
		
		si = (cam.x() - sx) / hspacing - 4;
		ei = si + 9;
		sj = (cam.y() - sy) / vspacing - 3;
		ej = sj + 7;
	}
	
	void draw(float subframe) {
		for (int i = si; i < ei; i++) {
			for (int j = sj; j < ej; j++) {
				g.draw_quad_world(13, 10, false, sx + r*a1 + i * hspacing + (j % 2) * hspacing / 2, sy + r*b1 + j*vspacing, sx + r*a2 + i * hspacing + (j % 2) * hspacing / 2, sy + r*b2 + j*vspacing, sx + r*a3 + i * hspacing + (j % 2) * hspacing / 2, sy + r*b3 + j*vspacing, sx + r*a3 + i * hspacing + (j % 2) * hspacing / 2, sy + r*b3 + j*vspacing, 0xFF971EFF, 0xFF971EFF, 0xFF971EFF, 0xFF971EFF);
			}
		}
	}
	
	void editor_step() {
		theta = pi / 2;
		a1 = cos(theta);
		b1 = sin(theta);
		a2 = cos(theta + 2*pi/3);
		b2 = sin(theta + 2*pi/3);
		a3 = cos(theta + 4*pi/3);
		b3 = sin(theta + 4*pi/3);
	}
	
	void editor_draw(float subframe) {
		for (int i = -4; i < 35; i++) {
			for (int j = -3; j < 8; j++) {
				g.draw_quad_world(13, 10, false, sx + r*a1 + i * hspacing + (j % 2) * hspacing / 2, sy + r*b1 + j*vspacing, sx + r*a2 + i * hspacing + (j % 2) * hspacing / 2, sy + r*b2 + j*vspacing, sx + r*a3 + i * hspacing + (j % 2) * hspacing / 2, sy + r*b3 + j*vspacing, sx + r*a3 + i * hspacing + (j % 2) * hspacing / 2, sy + r*b3 + j*vspacing, 0xFF971EFF, 0xFF971EFF, 0xFF971EFF, 0xFF971EFF);
			}
		}
	}

}