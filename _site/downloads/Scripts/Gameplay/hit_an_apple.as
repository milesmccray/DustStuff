const int MAX_PLAYERS = 4;

class script {
	scene@ g;
	bool finished;

	script() {
		@g = get_scene();
		finished = false;
	}

	void step(int entities) {

		if (finished) {
			return;
		}

		for (int i = 0; i < entities; i++) {
			entity@ e = entity_by_index(i);
			if (@e == null) {
				continue;
			}

			// remove level end entities if they exist
			if (e.type_name() == "level_end" || e.type_name() == "level_end_prox") {
				g.remove_entity(e);
			}

			if (e.type_name() == "hittable_apple") {
				controllable@ apple = e.as_controllable();
				if (apple.state() == 20) {
					finished = true;
					g.end_level(apple.x(), apple.y());
					break;
				}

			}
		}

	}
}