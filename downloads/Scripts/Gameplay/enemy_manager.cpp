class enemyData {
	[entity] int id;
	[text] int hp = 3;
	[text] bool lock_position = false;
	//[text] float bob_amount = 10; 
	//[text] float bob_speed = 0.05;
	[text] float time_warp = 1.0;
	[text] int attack_interval = 0;
	[hidden] float lock_x;
	[hidden] float lock_y;
	[hidden] int attack_timer;
}

class script {
	scene@ g;

	[text] array<enemyData> enemys;
	[text] float global_enemy_timescale = 1.0;

	int frame = 0;

	script() {
		@g = get_scene();
	}

	void step(int entities) {
		for (uint i = 0; i < enemys.length(); i++) {
			enemyData@ b = @enemys[i];
			entity@ e = entity_by_id(b.id);
			if (@e == null) continue;

			controllable@ c = e.as_controllable();
			if (@c == null) continue;

			// Enforce HP
			if (c.life() > b.hp) {
				c.life(b.hp);
			}

			// Apply time warp (affects movement speed, not attack animation)
			e.time_warp(b.time_warp);

			// Reduce attack endlag when time warped
			if (b.time_warp > 1.0 && c.attack_state() != 0) {
				float extra = b.time_warp - 1.0;
				c.state_timer(c.state_timer() + extra);
				c.attack_timer(c.attack_timer() + extra);
			}

			// Force attack on interval (frames, 60 = 1 second)
			if (b.attack_interval > 0) {
				b.attack_timer++;
				if (b.attack_timer >= b.attack_interval && c.attack_state() == 0) {
					c.attack_state(1);
					c.attack_timer(0);
					b.attack_timer = 0;
				}
			}

			// Lock position if enabled, with vertical bob
			if (b.lock_position) {
				float bob_offset = sin(frame * b.bob_speed) * b.bob_amount;
				e.x(b.lock_x);
				e.y(b.lock_y + bob_offset);
				c.set_speed_xy(0, 0);
			}
		}

		frame++;

		// Apply global timescale to all enemies in the scene
		if (global_enemy_timescale != 1.0) {
			for (int i = 0; i < entities; i++) {
				entity@ e = entity_by_index(i);
				if (e.type_name() == "enemy_trash_bag") {
					e.time_warp(global_enemy_timescale);
				}
			}
		}
	}

	void entity_on_add(entity@ e) {
		hitbox@ h = e.as_hitbox();
		if (@h == null) return;

		controllable@ owner = h.owner();
		if (@owner == null) return;

		// Check if the hitbox owner is one of our tracked enemys
		for (uint i = 0; i < enemys.length(); i++) {
			if (int(owner.id()) == enemys[i].id && enemys[i].time_warp != 1.0) {
				h.timer_speed(h.timer_speed() * enemys[i].time_warp);
				break;
			}
		}
	}

	void editor_var_changed(var_info@ info) {
		// Auto-populate HP when an entity is assigned
		if (info.get_name() == "id" && info.get_path_length() > 0) {
			int idx = info.get_index(0);
			if (idx >= 0 && uint(idx) < enemys.length()) {
				entity@ e = entity_by_id(enemys[idx].id);
				if (@e != null) {
					controllable@ c = e.as_controllable();
					if (@c != null) {
						enemys[idx].hp = c.life();
						editor_sync_vars_menu();
					}
				}
			}
		}
	}

	void on_level_start() {
		// Apply initial HP to all enemys
		for (uint i = 0; i < enemys.length(); i++) {
			enemyData@ b = @enemys[i];
			entity@ e = entity_by_id(b.id);
			if (@e == null) continue;

			controllable@ c = e.as_controllable();
			if (@c == null) continue;

			c.life(b.hp);

			// Store initial position for locking
			if (b.lock_position) {
				b.lock_x = e.x();
				b.lock_y = e.y();
			}
		}
	}
}
