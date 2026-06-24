const int MAX_PLAYERS = 4;
class script {
	void step(int entities) {
		for (int i = 0; i < MAX_PLAYERS; i++) {
			controllable@ c = controller_controllable(i);
			if (@c == null)
				continue;
			dustman@ dm = c.as_dustman();
			if (@dm == null)
				continue;
			dm.dash_max(10);
			dm.dash(10);
		}
	}
}