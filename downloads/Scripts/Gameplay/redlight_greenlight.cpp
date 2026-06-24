// Redlight / Greenlight for Dustforce.
// - Cycles between GREEN and RED phases.
// - During RED, if any player has any speed, they die.
// - Draws a small traffic light + label on the HUD.
#include "../lib/SeedGenerator.as"
const int MAX_PLAYERS = 4;

// How long (frames @ 60fps) each phase lasts. Tweak in editor.
class script
{
	scene@ g;
	textfield@ label;
	textfield@ phase_tf;

	[text|tooltip:"Min frames a phase lasts (60 = 1s)"] int min_phase_frames = 90;
	[text|tooltip:"Max frames a phase lasts"] int max_phase_frames = 240;

	// HUD position (centered coords; positive Y down)
	[slider,min:-640,max:640] float hud_x = -560;
	[slider,min:-360,max:360] float hud_y = -300;
	[slider,min:0.5,max:3] float hud_scale = 1.0f;

	bool is_green = true;
	int phase_timer = 0;
	int next_phase_len = 120;
	bool seeded = false;
	SeedGenerator@ sg;
	script()
	{
		@g = get_scene();
	}

	void on_level_start()
	{
		@sg = SeedGenerator();
		@label = create_textfield();
		label.set_font("Caracteres", 36);
		label.colour(0xFFFFFFFF);
		label.align_horizontal(0);
		label.align_vertical(0);

		@phase_tf = create_textfield();
		phase_tf.set_font("Caracteres", 36);
		phase_tf.colour(0xFFFFFFFF);
		phase_tf.align_horizontal(0);
		phase_tf.align_vertical(0);

		// initial phase (length is rolled once RNG is seeded in step())
		is_green = true;
		next_phase_len = min_phase_frames; // placeholder; replaced after seeding
		phase_timer = 0;
		update_phase_text();
	}

	int pick_phase_len()
	{
		if(max_phase_frames <= min_phase_frames)
			return min_phase_frames;
		return min_phase_frames + int(rand() % uint(max_phase_frames - min_phase_frames));
	}

	void update_phase_text()
	{
		if(@phase_tf == null) return;
		if(is_green)
		{
			phase_tf.text("GREENLIGHT");
			phase_tf.colour(0xFF00FF40);
		}
		else
		{
			phase_tf.text("REDLIGHT");
			phase_tf.colour(0xFFFF3030);
		}
	}

	void step(int)
	{
		if(@sg == null) return;
		sg.step();
		if(!sg.ready()) return;
		if(!seeded)
		{
			srand(sg.getSeed()); // seed RNG with the generated seed for replay-safe randomness
			seeded = true;
			// roll the first phase length now that RNG is deterministic
			next_phase_len = pick_phase_len();
			phase_timer = 0;
		}

		// advance phase
		phase_timer++;
		if(phase_timer >= next_phase_len)
		{
			is_green = !is_green;
			phase_timer = 0;
			next_phase_len = pick_phase_len();
			update_phase_text();
			g.play_sound(is_green ? "sfx_hud_level_start" : "sfx_damage", 0, 0, 1, false, false);
		}

		// during red, kill anyone holding any key
		if(!is_green)
		{
			for(int i = 0; i < MAX_PLAYERS; i++)
			{
				controllable@ c = controller_controllable(i);
				if(@c == null) continue;
				dustman@ dm = c.as_dustman();
				if(@dm == null) continue;
				if(dm.dead()) continue;

				if(any_key_held(dm))
				{
					dm.kill(true);
				}
			}
		}
	}

	void draw(float sub_frame)
	{
		if(@phase_tf == null) return;

		float s = hud_scale;
		float light_w = 60 * s;
		float light_h = 160 * s;
		float cx = hud_x;
		float cy = hud_y;

		// housing
		g.draw_rectangle_hud(22, 18,
			cx - light_w * 0.5f - 6, cy - light_h * 0.5f - 6,
			cx + light_w * 0.5f + 6, cy + light_h * 0.5f + 6,
			0, 0xFF202020);
		g.draw_rectangle_hud(22, 19,
			cx - light_w * 0.5f, cy - light_h * 0.5f,
			cx + light_w * 0.5f, cy + light_h * 0.5f,
			0, 0xFF000000);

		float bulb_r = 22 * s;
		float top_y = cy - light_h * 0.25f;
		float bot_y = cy + light_h * 0.25f;

		uint red_col = is_green ? 0xFF400000 : 0xFFFF3030;
		uint green_col = is_green ? 0xFF00FF40 : 0xFF003010;

		draw_disc(cx, top_y, bulb_r, red_col);
		draw_disc(cx, bot_y, bulb_r, green_col);

		// label
		phase_tf.draw_hud(22, 21, cx, cy + light_h * 0.5f + 30 * s, s, s, 0);
	}

	// Approximate a disc with a rotated square (no native circle).
	void draw_disc(float cx, float cy, float r, uint col)
	{
		// stack two rotated squares to approximate an octagon
		g.draw_rectangle_hud(22, 20, cx - r, cy - r, cx + r, cy + r, 0, col);
		g.draw_rectangle_hud(22, 20, cx - r, cy - r, cx + r, cy + r, 0.7853982f, col);
	}

	// Returns true if the player is physically holding any key right now.
	// Per the API docs:
	//   x_intent / y_intent : -1 / 0 / 1 directional hold
	//   heavy_intent / light_intent : 10 = pressed, 11 = pressed and used.
	//     Values 1..9 are the post-release decay buffer and do NOT mean the
	//     key is being held, so we only treat >= 10 as "currently held".
	//   dash_intent / jump_intent : 1 = pushed this frame, 2 = pushed and used.
	//   taunt_intent : 1 = pressed, 2 = pressed and used.
	bool any_key_held(dustman@ dm)
	{
		if(dm.x_intent() != 0) return true;
		if(dm.y_intent() != 0) return true;
		if(dm.heavy_intent() >= 10) return true;
		if(dm.light_intent() >= 10) return true;
		int d = dm.dash_intent();  if(d == 1 || d == 2) return true;
		int j = dm.jump_intent();  if(j == 1 || j == 2) return true;
		int t = dm.taunt_intent(); if(t == 1 || t == 2) return true;
		return false;
	}
}
