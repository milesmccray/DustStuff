
class EntityData {
	[entity,enemy] uint id;
	[text] float rate = 1;
}

class script {
	scene@ g;
	[text] array<EntityData> enemies(0);
	[text] float default_porc_rate = 0.2;

	script() {
		@g = get_scene();
	}

	void remove_enemies() {
		for (uint i = 0; i < enemies.length(); i++) {
			entity@ e = entity_by_id(enemies[i].id);
			if (@e == null)
				continue;
			g.remove_entity(e);
		}
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
				if (c.attack_state() == 0) // force attack
					c.attack_state(1);
			}
		}

	}

	float get_porc_speed(uint p) {
		for (uint i = 0; i < enemies.length; i++) {
			if (p == enemies[i].id)
				return enemies[i].rate;
		}
		return default_porc_rate;
	}

	void entity_on_add(entity@ e) {
		hitbox@ h = e.as_hitbox();
		if (@h != null) {
			if (h.team() == 0 && h.attack_strength() == 0) {
				h.team(1);

				controllable@ o = h.owner();
				if (@o != null)
					h.timer_speed(h.timer_speed() * get_porc_speed(o.id()));
			}
		}
	}

	void editor_draw(float sf) {
		for (uint i = 0; i < enemies.length; i++) {
			entity@ e = entity_by_id(enemies[i].id);
			if (@e == null)
				continue;
			g.draw_rectangle_world(22, 0, e.x() - 20, e.y() - 40, e.x() + 20, e.y(), 0, 0x8844FF44);
		}
	}
}
