const int CAMERA_BUFFER = 100;

class script {
	scene@ g;
	camera@ cam;
	float cl, cr, ct, cb;

	int layer = 20;
	int sublayer = 20;

	[text] int blur = 20;

	script() {
		@g = get_scene();
	}

	void step(int entities) {
		// if (!enabled)
			// return;
		if (@cam == null)
			@cam = get_active_camera();

		cl = cam.x() - (cam.screen_width() / 2) / g.layer_scale(layer) - CAMERA_BUFFER;
		cr = cam.x() + (cam.screen_width() / 2) / g.layer_scale(layer) + CAMERA_BUFFER;
		ct = cam.y() - (cam.screen_height() / 2) / g.layer_scale(layer) - CAMERA_BUFFER;
		cb = cam.y() + (cam.screen_height() / 2) / g.layer_scale(layer) + CAMERA_BUFFER;
	}

	void draw(float sf) {
		// if (enabled) {
			for (uint i = 0; i < blur; i++) {
				g.draw_glass_world(layer, sublayer, cl, ct, cr, cb, 0, 0);
			}
		// }
	}

	void editor_draw(float sf) {
		draw(sf);
	}
}
