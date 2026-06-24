const int MAX_PLAYERS = 4;


class script {
	// track combo values from previous frames and add all changes to shared values
	int last_skill_combo;
	int last_combo_count;
	int shared_skill_combo;
	int shared_combo_count;
	float shared_combo_timer;
	bool change_values;

	script() {
		// @g = get_scene();

		last_skill_combo = 0;
		last_combo_count = 0;
		shared_skill_combo = 0;
		shared_combo_count = 0;
		shared_combo_timer = 0.0f;
		change_values = false;

	}

	void step(int entities) {
		
		bool check_timer = true;
		
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

			// cases
			int skill_combo = dm.skill_combo();
			int combo_count = dm.combo_count();
			bool combo_up = combo_count > last_combo_count;
			bool used_super = skill_combo < last_skill_combo;
			bool got_hit = not combo_up and skill_combo > last_skill_combo;
			bool lost_combo = combo_count < last_combo_count and check_timer;
			// bool attack_state_special = dm.attack_state() == 3;

			// avoid duplicating timer dropping combo for everyone
			if (dm.combo_timer() == 0 and check_timer) {
				check_timer = false;
			}

			// update shared values
			if (combo_up or used_super or got_hit or lost_combo) {
				// puts(dm.character() + " " + combo_up + " " + used_super + " " + got_hit + " " + lost_combo + " " + dm.combo_timer());
				change_values = true;
				shared_skill_combo += skill_combo - last_skill_combo;
				shared_combo_count += combo_count - last_combo_count;
				shared_combo_timer = dm.combo_timer();
			}

			/*/ take away super charge once used but doesn't prevent same-frame usage
			if (attack_state_special) {
				change_values = true;
				shared_skill_combo = 0;
				shared_combo_timer = dm.combo_timer();
			}*/
		}

		// apply changes
		if (change_values) {
			change_values = false;
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

				// set values
				dm.skill_combo(shared_skill_combo);
				dm.combo_count(shared_combo_count);
				dm.combo_timer(shared_combo_timer);
				// puts(dm.character() + " " + last_skill_combo + " " + shared_skill_combo + " " + last_combo_count + " " + shared_combo_count + " " + shared_combo_timer);
			}

			// reset values to check again next step
			last_skill_combo = shared_skill_combo;
			last_combo_count = shared_combo_count;
		}


	}
}
