const int MAX_PLAYERS = 4;

class script {
	scene@ g;
	bool finished;
	uint appleId;
	float appleX;
	float appleY;

	script() {
		@g = get_scene();
		finished = false;
	}

	void on_level_start() {
		entity@ e = controller_entity(0);
		dustman@ dm = e.as_dustman();
		appleId = 0;

		if (g.level_type() != 1) {
			entity@ newApple = create_entity("hittable_apple");
			newApple.x(dm.x());
			newApple.y(dm.y());
			g.add_entity(newApple);
			appleId = newApple.id();
			appleX = newApple.x();
			appleY = newApple.y();
		}
	}

	void step(int entities) {

		if (finished || g.level_type() == 1) {
			return;
		}

		if (appleId == 0) {
			return;
		}

		bool found = false;
		for (int i = 0; i < entities; i++) {
			entity@ e = entity_by_index(i);
			if (@e == null) {
				continue;
			}

			// remove level end entities if they exist
			if (e.type_name() == "level_end" || e.type_name() == "level_end_prox") {
				g.remove_entity(e);
			}

			if (e.id() == appleId) {
				found = true;
				appleX = e.x();
				appleY = e.y();
				continue;
			}
		}

		if (found) {
			for (int i = 0; i < entities; i++) {
				entity@ e = entity_by_index(i);
				if (@e == null) {
					continue;
				}

				if (e.id() == appleId) {
					continue;
				}

				if (e.type_name() == "hittable_apple") {
					if (abs(appleX - e.x()) < 48 && abs(appleY - e.y()) < 48) {
						finished = true;
						g.end_level(appleX, appleY);
						break;
					}
				}
			}
		}

	}
}