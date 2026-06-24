const int MAX_PLAYERS = 4;

class script {
	bool set_teams;

	script() {
		set_teams = true;
	}

	void step(int entities) {
		if (set_teams) {
			for (int i = 0; i < MAX_PLAYERS; i++) {
				/* Try to get a 'dustman' object handle for each player. */
				entity@ e = controller_entity(i);
				if (@e == null) {
					continue;
				}
				dustman@ dm = e.as_dustman();
				if (@dm == null) {
					continue;
				}

				dm.team(i + 2);
				set_teams = false;
			}
		}

	}
}