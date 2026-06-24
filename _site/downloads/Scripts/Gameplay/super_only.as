const int MAX_PLAYERS = 4;

class script {

	void step(int entities) {
		for (int i = 0; i < MAX_PLAYERS; i++) {
			/* Try to get a 'dustman' object handle for each player. */
			controllable@ c = controller_controllable(i);
			if (@c == null) {
				continue;
			}
			dustman@ dm = c.as_dustman();
			if (@dm == null) {
				continue;
			}

			// infinite skill_combo
			dm.skill_combo(dm.skill_combo_max());

			if (dm.attack_state() == 1 || dm.attack_state() == 2) {
				dm.attack_state(3);
			}

		}

	}
}