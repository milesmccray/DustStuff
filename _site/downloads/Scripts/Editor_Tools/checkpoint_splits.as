/*
 * CheckpointSplits.as
 *
 * Notes:
 * - Script state is transient: it will be reset on level restart. We do not persist
 *   or track state across level restarts in this script.
 * - Writing files from scripts is disallowed by the DustAPI for security reasons,
 *   so this script prints checkpoint frame data to the console using puts() so you
 *   can copy it manually after a run.
 */

class script {
	scene@ g;

	// Globals
	int frame_count = -57;        // total frames since level start (incremented in step). Start at -57 to match game timer offset
	int checkpoint_count = 0;   // number of times checkpoint_save() was called
    
	// Stores the frame number for each checkpoint in the current run (runtime)
	array<int> checkpoint_frames;

	// If present, stores the expected split frames loaded from an embed
	array<int> loaded_splits;

	// Index of the next expected split in `loaded_splits` (incremented each checkpoint)
	int next_split_index = 0;

	// Textfield used to draw the frame count when a checkpoint is saved
	textfield@ tf;

	// Default colour for the HUD text (white)
	uint DEFAULT_COLOUR = 0xFFFFFFFF;

	// Default HUD layout and colours (can be overridden by splits.ini embed)
	float hud_x = 16.0;
	float hud_y = -400.0;
	uint AHEAD_COLOUR = 0xFF66FF66; // green when ahead
	uint BEHIND_COLOUR = 0xFFFF6666; // red when behind

	// How many frames the HUD message should remain visible after a checkpoint
	int display_duration = 180; // default: 180 frames (~3s at 60fps)
	int display_frames_left = 0; // frames remaining to display

	script() {
		@g = get_scene();
		@tf = create_textfield();
		tf.colour(DEFAULT_COLOUR);
	}

	// Helper: trim trailing CR ('\r') from a line
	string trim_cr(const string &in s) {
		if (s.length() == 0) return s;
		// s[index] returns a byte code (uint8); check for CR (13)
		if (s[s.length() - 1] == 13)
			return s.substr(0, s.length() - 1);
		return s;
	}

	// Helper: simple decimal parser (ignores non-digit characters)
	int parse_positive_int(const string &in s) {
		int v = 0;
		for (uint i = 0; i < s.length(); i++) {
			// s[i] returns a byte code (uint8)
			uint8 ch = s[i];
			if (ch >= 48 && ch <= 57) { // '0' == 48, '9' == 57
				v = v * 10 + (ch - 48);
			}
		}
		return v;
	}

	// Constants for time conversion
	int FPS = 60;

	// Format a frame-difference (can be negative) into a +/-seconds.mmm string
	string format_frame_diff_time(int diff_frames) {
		int abs_d = diff_frames < 0 ? -diff_frames : diff_frames;
		int secs = abs_d / FPS;
		int rem_frames = abs_d % FPS;
		int millis = rem_frames * 1000 / FPS; // integer milliseconds

		string sign = diff_frames >= 0 ? "+" : "-";
		// pad millis to 3 digits
		string ms = "" + millis;
		while (ms.length() < 3) ms = "0" + ms;
		return sign + secs + "." + ms;
	}

	// Return the expected splits embed filename for the current map (centralized).
	string get_splits_filename() {
		string mapname = g.map_name();
		return mapname + " splits.txt";
	}

	// Attempt to load an embed named '<mapname> splits.txt' and parse its lines
	// into `checkpoint_frames`. Returns true if an embed was found and parsed.
	bool try_load_splits_embed() {
        string fname = get_splits_filename();
        bool ok = load_embed('splits', fname);

		string content = get_embed_value('splits');
		if (content.length() == 0) return true;

		string cur = "";
		for (uint i = 0; i < content.length(); i++) {
			// content[i] returns a byte code (uint8)
			uint8 ch = content[i];
			if (ch == 10) { // '\n' == 10
				string line = trim_cr(cur);
				if (line.length() > 0) {
					int v = parse_positive_int(line);
					loaded_splits.insertLast(v);
				}
				cur = "";
			} else {
				// append the single-character substring so we build the line string properly
				cur += content.substr(i, 1);
			}
		}
		if (cur.length() > 0) {
			string line = trim_cr(cur);
			if (line.length() > 0) {
					int v = parse_positive_int(line);
					loaded_splits.insertLast(v);
			}
		}
		return true;
	}

	// Helper: trim whitespace from both ends
	string trim_ws(const string &in s) {
		int a = 0;
		int b = int(s.length()) - 1;
		while (a <= b && (s[a] == 32 || s[a] == 9)) a++; // space or tab
		while (b >= a && (s[b] == 32 || s[b] == 9 || s[b] == 13)) b--; // space/tab/CR
		if (b < a) return "";
		return s.substr(a, b - a + 1);
	}

	// Parse a float from a string; accepts optional leading +/-, integer or fractional part
	float parse_float(const string &in s) {
		string t = trim_ws(s);
		if (t.length() == 0) return 0.0;
		float sign = 1.0;
		uint idx = 0;
		if (t[0] == 45) { sign = -1.0; idx = 1; } // '-'
		if (t[0] == 43) { idx = 1; } // '+'
		float intpart = 0.0;
		for (; idx < t.length(); idx++) {
			uint8 ch = t[idx];
			if (ch >= 48 && ch <= 57) {
				intpart = intpart * 10.0 + float(ch - 48);
			} else break;
		}
		float frac = 0.0;
		if (idx < t.length() && t[idx] == 46) { // '.'
			idx++;
			float place = 0.1;
			for (; idx < t.length(); idx++) {
				uint8 ch = t[idx];
				if (ch >= 48 && ch <= 57) {
					frac += place * float(ch - 48);
					place *= 0.1;
				} else break;
			}
		}
		return sign * (intpart + frac);
	}

	// Parse a hex color from strings like "0xFF66FF66", "#FF66FF66", or "FF66FF66".
	uint parse_hex_color(const string &in s, uint fallback) {
		string t = trim_ws(s);
		if (t.length() == 0) return fallback;
		// remove prefix (support '#' and '0x' prefixes) using byte-safe checks
		if (t.length() > 1) {
			// use uint8 temporaries to avoid signed/unsigned and const-string conversion issues
			uint8 c0 = t[0];
			uint8 c1 = t[1];
			// check if first char is '#' (35)
			if (c0 == 35) {
				t = t.substr(1, t.length() - 1);
			}
			// check if starts with '0' and 'x'/'X'
			else if (c0 == 48 && (c1 == 88 || c1 == 120)) { // '0' and 'X' or 'x'
				t = t.substr(2, t.length() - 2);
			}
		}
		// Now parse hex digits
		uint v = 0;
		for (uint i = 0; i < t.length(); i++) {
			uint8 ch = t[i];
			v <<= 4;
			if (ch >= 48 && ch <= 57) v |= (ch - 48);
			else if (ch >= 65 && ch <= 70) v |= (ch - 55); // A-F
			else if (ch >= 97 && ch <= 102) v |= (ch - 87); // a-f
			else return fallback;
		}
		// If the value is RGB (6 digits), assume full alpha
		if (t.length() == 6) v |= 0xFF000000;
		return v;
	}

	// Attempt to load a small config embed named 'splits.ini' in the same 'splits' namespace.
	// Supported keys (case-sensitive): hudX, hudY, aheadColor, behindColor
	bool try_load_config_embed() {
		string fname = "splits.ini";
		bool ok = load_embed('splits', fname);
		string content = get_embed_value('splits');
		if (content.length() == 0) return false;
		string cur = "";
		for (uint i = 0; i < content.length(); i++) {
			uint8 ch = content[i];
			if (ch == 10) {
				string line = trim_cr(cur);
				line = trim_ws(line);
				if (line.length() > 0 && line[0] != 59) { // ignore ';' comments
					// find separator '=' or ':'
					int sep = -1;
					for (uint j = 0; j < line.length(); j++) {
						if (line[j] == 61 || line[j] == 58) { sep = int(j); break; } // '=' or ':'
					}
					if (sep >= 0) {
						string key = trim_ws(line.substr(0, sep));
						string val = trim_ws(line.substr(sep + 1, line.length() - sep - 1));
						if (key == "hudX") hud_x = parse_float(val);
						else if (key == "hudY") hud_y = parse_float(val);
						else if (key == "aheadColor") AHEAD_COLOUR = parse_hex_color(val, AHEAD_COLOUR);
						else if (key == "behindColor") BEHIND_COLOUR = parse_hex_color(val, BEHIND_COLOUR);
					}
				}
				cur = "";
			} else {
				cur += content.substr(i, 1);
			}
		}
		if (cur.length() > 0) {
			string line = trim_cr(cur);
			line = trim_ws(line);
			if (line.length() > 0 && line[0] != 59) {
				int sep = -1;
				for (uint j = 0; j < line.length(); j++) {
					if (line[j] == 61 || line[j] == 58) { sep = int(j); break; }
				}
				if (sep >= 0) {
					string key = trim_ws(line.substr(0, sep));
					string val = trim_ws(line.substr(sep + 1, line.length() - sep - 1));
					if (key == "hudX") hud_x = parse_float(val);
					else if (key == "hudY") hud_y = parse_float(val);
					else if (key == "aheadColor") AHEAD_COLOUR = parse_hex_color(val, AHEAD_COLOUR);
					else if (key == "behindColor") BEHIND_COLOUR = parse_hex_color(val, BEHIND_COLOUR);
				}
			}
		}
		// Apply any color overrides immediately to the default text colour
		tf.colour(DEFAULT_COLOUR);
		return true;
	}

	// Called every game frame. `entities` is the number of entities available via entity_by_index.
	void step(int entities) {
		// Increment global frame counter
		frame_count += 1;

		// Countdown HUD display timer
		if (display_frames_left > 0) {
			display_frames_left -= 1;
			// If the timer just expired, reset text colour to default
			if (display_frames_left == 0) {
				tf.colour(DEFAULT_COLOUR);
			}
		}

		// Try to find the player entity and keep a reference (optional usage example)
		for (int i = 0; i < entities; i++) {
			entity@ e = entity_by_index(i);
			if (@e == null) continue;
			dustman@ dm = e.as_dustman();
			if (@dm != null) {
				// Found the player dustman. You can access properties on `dm` if needed.
				// Leave as a no-op for now; we only needed to demonstrate fetching the dustman.
				break;
			}
		}
	}

	// Called by the game when a checkpoint is saved.
	void checkpoint_save() {
		checkpoint_count += 1;

		// Compute diff against loaded splits (if available)
		string diff_str = "";
		if (next_split_index < int(loaded_splits.length())) {
			int expected = loaded_splits[next_split_index];
			int diff = frame_count - expected; // negative => earlier (ahead of split)
			// format as time string using FPS
			diff_str = format_frame_diff_time(diff);
			// choose color: behind vs ahead using configured colours
			uint col = diff >= 0 ? BEHIND_COLOUR : AHEAD_COLOUR;
			// apply to textfield
			tf.colour(col);
			// advance to next expected split
			next_split_index += 1;
		} else {
			diff_str = "(no expected)";
		}

		// Update textfield with current frame_count, checkpoint number, and diff
		tf.text("Checkpoint " + checkpoint_count + ": frame=" + frame_count + " diff=" + diff_str);

		// Start HUD display timer
		display_frames_left = display_duration;

		// Record the frame this checkpoint was triggered on
		checkpoint_frames.insertLast(frame_count);
	}

	// Called at the end of a level / replay — print recorded checkpoint frames to console
	void on_level_end()
	{
		// Print each frame number on its own line so it's easy to copy
		for (uint i = 0; i < checkpoint_frames.length(); i++) {
			puts(checkpoint_frames[i] + "");
		}

		// Help text for the user showing how to save the splits for future runs
		string fname = get_splits_filename();
		puts("");
		puts("To save these splits for future runs:");
		puts("1) Create a file named: Dustforce/content/plugins/embeds/" + fname);
		puts("2) Paste the frame numbers (one per line) exactly as printed above.");
		puts("3) Reload the level; the script will load the embed on level start.");
		puts("");
	}

	void on_level_start()
	{
		// Load optional splits and a small configuration file
		bool cfgLoaded = try_load_config_embed();
		// Load config and optional splits embed (no debug output)
		try_load_config_embed();
		try_load_splits_embed();
	}

	// Draw to the HUD so the text stays fixed relative to the screen and follows the camera.
	void draw(float sub_frame) {
		// Example HUD position: top-left corner with small offsets.
		float hud_x_local = hud_x;
		float hud_y_local = hud_y;
		float scale = 0.75;
		// draw_hud(layer, sublayer, x, y, sx, sy, rot)
		if (display_frames_left > 0) {
			tf.draw_hud(22, 22, hud_x_local, hud_y_local, scale, scale, 0);
		}
	}
}

