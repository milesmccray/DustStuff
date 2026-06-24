const float APPLE_START_SIZE = 24; // No change actually happening

class script {
	scene@ g;
	[text] float GROWTH_SIZE = 0.75; // per enemy health stuck
	[text] float GROWTH_TIME = 1;//0.25;
	[text] float SIZE_BUFF = 16;
	[text] float STUCK_SNAP = 0.75;
	bool finished;
	uint appleId;
	float appleX;
	float appleY;
	float appleSize;
	float appleSize_checkpoint;
	dictionary stuck; // entity id: [dx, dy]
	dictionary stuck_checkpoint; // entity id: [dx, dy]
	int init_filth;
	int init_filth_block;
	int init_enemy;

	script() {
		@g = get_scene();
		g.get_filth_level(init_filth, init_filth_block, init_enemy);
		stuck = dictionary(); // entity id: [dx, dy]
		stuck_checkpoint = dictionary(); // entity id: [dx, dy]
		finished = false;
		appleSize = 0;
	}

	void on_level_start() {
		entity@ e = controller_entity(0);
		dustman@ dm = e.as_dustman();
		appleId = 0;

		if (g.level_type() != 1) {
			entity@ newApple = create_entity("hittable_apple");
			newApple.x(dm.x());
			newApple.y(dm.y());
			controllable@ nac = newApple.as_controllable();
			nac.team(1);
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

		entity@ apple = entity_by_id(appleId);

		for (int i = 0; i < entities; i++) {
			entity@ e = entity_by_index(i);
			if (@e == null) {
				continue;
			}

			// remove level end entities if they exist
			if (e.type_name() == "level_end" || e.type_name() == "level_end_prox") {
				g.remove_entity(e);
			}

			if (e.type_name() == "hit_box_controller") {
				hitbox@ h = e.as_hitbox();
				controllable@ owner = h.owner();
				if (@owner == null) {
					continue;
				}
				dustman@ dm = owner.as_dustman();
				if (@dm != null) {
					h.team(0);
					continue;
				}
				g.remove_entity(e);
			}

			controllable@ c = e.as_controllable();
			if (@c == null) {
				continue;
			}

			dustman@ dm = c.as_dustman();
			if (@dm != null) {
				dm.team(0);
				continue;
			}

			string id = "" + e.id();

			if (e.id() == appleId) {
				appleX = e.x();
				appleY = e.y();

			} else {
				// c.team(1);
				if (stuck.exists(id)) {
					// remove if apple died
					if (@apple == null) {
						g.remove_entity(e);
					}

					array<float>@ stpos = cast<array<float>>(stuck[id]);
					e.x(appleX + stpos[0]);
					e.y(appleY + stpos[1]);
				}
			}
		}

		if (@apple == null) {
			g.load_checkpoint();
			return;
		}
		controllable@ apple_c = apple.as_controllable();

		int collisions = g.get_entity_collision(appleY - appleSize - SIZE_BUFF, appleY + appleSize + SIZE_BUFF, appleX - appleSize - SIZE_BUFF, appleX + appleSize + SIZE_BUFF, 1);
		for (int c = 0; c < collisions; c++) {
			entity@ ce = g.get_entity_collision_index(c);
			if (ce.id() == appleId)
				continue;
			if (ce.type_name() == "z_script_enemy")
				continue;

			string ce_id = "" + ce.id();
			if (!stuck.exists(ce_id)) {
				array<float> temp = {(ce.x() - appleX) * STUCK_SNAP, (ce.y() - appleY) * STUCK_SNAP};
				stuck.set(ce_id, temp);

				controllable@ con = ce.as_controllable();
				if (@con == null) {
					continue;
				}

				// apple size
				if (!(ce.type_name() == "hittable_apple" || ce.type_name() == "enemy_scrolls" || ce.type_name() == "enemy_treasure" || ce.type_name() == "enemy_trash_bag")) {
					appleSize += con.life_initial();
					apple_c.scale((APPLE_START_SIZE + appleSize * GROWTH_SIZE) / APPLE_START_SIZE, true);
					apple.time_warp(APPLE_START_SIZE / (APPLE_START_SIZE + appleSize * GROWTH_TIME));
				}
			}
		}

		// count filth
		int now_filth;
		int now_filth_block;
		int now_enemy;
		g.get_filth_remaining(now_filth, now_filth_block, now_enemy);
		// puts("" + init_enemy + " " + now_enemy);

		if (appleSize >= now_enemy  && init_enemy > 0) {
			int stuck_dust_count = 0;
			array<entity@> stuck_entities;
			//clean enemies
			for (int i = 0; i < entities; i++) {
				entity@ e = entity_by_index(i);
				if (@e == null) {
					continue;
				}
				controllable@ c = e.as_controllable();
				if (@c == null) {
					continue;
				}
				dustman@ dm = c.as_dustman();
				if (@dm != null) {
					continue;
				}

				if (e.type_name() == "z_script_enemy")
					continue;

				string id = "" + e.id();
				if (e.type_name() == "hittable_apple" || e.type_name() == "enemy_scrolls" || e.type_name() == "enemy_treasure" || e.type_name() == "enemy_trash_bag" || e.type_name() == "z_script_enemy") {
					continue;
				}
				if (stuck.exists(id)) {
					stuck_dust_count += c.life();
					stuck_entities.insertLast(e);
				}
			}

			if (stuck_dust_count >= now_enemy) {
				for (int i = 0; i < stuck_entities.length(); i++) {
					g.remove_entity(stuck_entities[i]);
				}

				finished = true;
				g.end_level(appleX, appleY);
			}

		}
	}

	void checkpoint_save() {
		appleSize_checkpoint = appleSize;
		stuck_checkpoint = stuck;
	}

	void checkpoint_load() {
		appleSize = appleSize_checkpoint;
		stuck = stuck_checkpoint;

		entity@ e = controller_entity(0);
		dustman@ dm = e.as_dustman();
		dm.team(0);

		entity@ apple = entity_by_id(appleId);
		controllable@ apple_c = apple.as_controllable();
		apple_c.team(1);
		apple_c.scale((APPLE_START_SIZE + appleSize * GROWTH_SIZE) / APPLE_START_SIZE, true);
		apple.time_warp(APPLE_START_SIZE / (APPLE_START_SIZE + appleSize * GROWTH_TIME));
		apple.x(dm.x());
		apple.y(dm.y() - 24);
	}
}