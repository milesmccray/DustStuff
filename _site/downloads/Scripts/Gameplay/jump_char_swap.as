class script {
	bool is_jumping;

	script() {
		is_jumping = false;
	}

	string next_char(string c) {
		if (c == "dustman")
			return "dustgirl";
		if (c == "dustgirl")
			return "dustkid";
		if (c == "dustkid")
			return "dustworth";
		if (c == "dustworth")
			return "dustman";
		if (c == "vdustman")
			return "vdustgirl";
		if (c == "vdustgirl")
			return "vdustkid";
		if (c == "vdustkid")
			return "vdustworth";
		if (c == "vdustworth")
			return "vdustman";
		return "dustman";
	}

	void step(int e) {
		controllable@ c = controller_controllable(0);
		if (@c == null)
			return;
		dustman@ dm = c.as_dustman();
		if (@dm == null)
			return;

		if (dm.jump_intent() > 0 && !is_jumping) {
			is_jumping = true;
			dm.character(next_char(dm.character()));
		} else if (dm.jump_intent() == 0) {
			is_jumping = false;
		}
	}
}
