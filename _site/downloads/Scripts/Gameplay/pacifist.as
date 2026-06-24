const int MAX_PLAYERS = 4;

// can hit enemy then save checkpoint, similar to death warp
// apples included in detection
// get filth has issues with long levels unloading dust without checkpoints

class EntityData {
	[entity] uint id;
}

class script {
	scene@ g;
	bool finished;
	bool has_dust;
	[text] array<EntityData> enemies(0);

	script() {
		@g = get_scene();
		finished = false;

		int init_filth;
		int init_filth_block;
		int init_enemy;
		g.get_filth_level(init_filth, init_filth_block, init_enemy);
		has_dust = init_filth + init_filth_block > 0;
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
		if (finished || !has_dust)
			return;

		for (int i = 0; i < entities; i++) {
			entity@ e = entity_by_index(i);
			if (@e == null)
				continue;

			// check if player hitboxes hit things
			if (e.type_name() == "hit_box_controller") {
				hitbox@ hb = e.as_hitbox();
				if (hb.team() != 1)
					continue;

				// hit, parry, parry hit
				if (hb.triggered_outcome() == 1 || hb.triggered_outcome() == 3 || hb.triggered_outcome() == 5)
					g.load_checkpoint();

			// check if any enemies are in stun (handles super)
			} else {
				controllable@ c = e.as_controllable();
				if (@c == null)
					continue;
				dustman@ d = c.as_dustman();
				if (@d != null)
					continue;

				if (c.stun_timer() > 0)
					g.load_checkpoint();
			}
		}

		int now_filth;
		int now_filth_block;
		int now_enemy;
		g.get_filth_remaining(now_filth, now_filth_block, now_enemy);
		// puts("" + now_filth + " " + now_filth_block + " " + now_enemy);

		// end level if collected all dust
		if (now_filth == 0 && now_filth_block == 0) {
			controllable@ dmc = controller_controllable(0);
			if (@dmc == null)
				return;

			finished = true;
			remove_enemies();
			g.end_level(dmc.x(), dmc.y());
		}
	}
}