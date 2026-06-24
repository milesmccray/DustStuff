class script {
	scene@ g;
	array<int> totem_ids;
	array<int> totem_ids_checkpoint;
	dictionary totems; // totem id: [x, y]
	dictionary totems_checkpoint; // totem id: [x, y]
	dictionary stuck; // entity id: [totem
	dictionary stuck_checkpoint; // entity id: [totem, dx, dy]

	script() {
		@g = get_scene();
		array<int> totem_ids;
		array<int> totem_ids_checkpoint;
		totems = dictionary(); // totem id: [x, y]
		totems_checkpoint = dictionary(); // totem id: [x, y]
		stuck = dictionary(); // entity id: [totem, dx, dy]
		stuck_checkpoint = dictionary(); // entity id: [totem, dx, dy]
	}

	void step(int entities) {

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

			string id = "" + e.id();

			if (e.type_name() == "enemy_stonebro") {
				if (totems.exists(id)) {
					array<float>@ temp = cast<array<float>>(totems[id]);
					temp[0] = e.x();
					temp[1] = e.y() - 24;
				} else {
					totem_ids.insertLast(e.id());
					array<float> temp = {e.x(), e.y() - 24};
					totems.set(id, temp);
				}

			} else if (stuck.exists(id)) {
				array<float>@ stpos = cast<array<float>>(stuck[id]);

				// unstick unloaded/dead totems
				entity@ tot = entity_by_id(stpos[0]);
				if (@tot == null) {
					stuck.delete(id);
					continue;
				}

				string tot_id = "" + stpos[0];
				array<float>@ tpos = cast<array<float>>(totems[tot_id]);
				e.x(tpos[0] + stpos[1]);
				e.y(tpos[1] + stpos[2]);
			}
		}

		for (int i = 0; i < totem_ids.length(); i++) {
			int t = totem_ids[i];
			string tid = "" + t;

			// ignore unloaded/dead totems
			entity@ e = entity_by_id(t);
			if (@e == null) {
				continue;
			}

			array<float>@ tpos = cast<array<float>>(totems[tid]);
			float tx = tpos[0];
			float ty = tpos[1];
			int collisions = g.get_entity_collision(ty - 24, ty + 24, tx - 24, tx + 24, 1);
			for (int c = 0; c < collisions; c++) {
				entity@ ce = g.get_entity_collision_index(c);
				if (ce.type_name() == "enemy_stonebro") {
					continue;
				}

				string ce_id = "" + ce.id();
				if (!stuck.exists(ce_id)) {
					array<float> temp = {t, ce.x() - tx, ce.y() - ty};
					stuck.set(ce_id, temp);
				}
			}
		}
	}

	void checkpoint_save() {
		totem_ids_checkpoint = totem_ids;
		totems_checkpoint = totems;
		stuck_checkpoint = stuck;
	}

	void checkpoint_load() {
		totem_ids = totem_ids_checkpoint;
		totems = totems_checkpoint;
		stuck = stuck_checkpoint;
	}
}