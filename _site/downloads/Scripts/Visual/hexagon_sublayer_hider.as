class script {
	scene@ g;

	[text] int layer = 15;
	[text] int min_sublayer = 0;
	[text] int max_sublayer = 24;

	[hidden] int current_sublayer;

	script() {
		@g = get_scene();
		current_sublayer = 0;
	}

	void on_editor_start() {
		g.layer_visible(layer, true);
	}

	void on_level_start() {
		current_sublayer = min_sublayer;
		g.sub_layer_visible(layer, min_sublayer, true);
		for (int i = min_sublayer + 1; i < max_sublayer + 1; i++) {
			g.sub_layer_visible(layer, i, false);
		}
	}

	void entity_on_remove(entity@ e) {
		if (e.type_name() == "enemy_tutorial_hexagon") {   // CHANGE ENEMY NAME HERE
			current_sublayer++;
			if (current_sublayer > max_sublayer)
				current_sublayer = max_sublayer;
			else
				g.sub_layer_visible(layer, current_sublayer, true);
		}
	}
}
