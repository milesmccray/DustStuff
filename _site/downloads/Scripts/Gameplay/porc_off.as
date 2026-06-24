class script {
	scene@ g;
	script() {
		@g = get_scene();
	}

	void step(int entities) {
		for (int i = 0; i < entities; i++) {
			entity@ e = entity_by_index(i);
			if (@e == null)
				continue;
			controllable@ c = e.as_controllable();
			if (@c == null)
				continue;

			if (c.type_name() == "enemy_porcupine") {
				if (c.attack_state() == 1) {
					c.attack_state(0);
				}
			}
		}
	}

	void entity_on_add(entity@ e) {
		hitbox@ h = e.as_hitbox();
		if (@h != null) {
			if (h.team() == 0 && h.attack_strength() == 0)
				g.remove_entity(e);
		}
	}
}
