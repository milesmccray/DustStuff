// Cartoon jizz meme — stringy streams
#include '../lib/math/math.cpp'
#include '../lib/drawing/circle.cpp'

class FluidParticle
{
	float x, y;
	float vx, vy;
	int stream_id;     // which stream/spurt this belongs to
	int order;         // birth order within stream

	FluidParticle() { x = 0; y = 0; vx = 0; vy = 0; stream_id = 0; order = 0; }
}

class script
{
	scene@ g;
	array<FluidParticle@> particles;
	int frame_count;

	// ---- Physics ----
	[slider,min:10,max:1000] int max_particles = 300;
	[slider,min:50,max:1500] float launch_speed = 420;
	[slider,min:0,max:50] float launch_spread = 8;
	[slider,min:0,max:500] float gravity = 153.6;
	[slider,min:0.9,max:1.0,step:0.001] float damping = 0.994;
	[slider,min:100,max:2000] float velocity_cap = 600.0;
	[slider,min:0,max:1,step:0.01] float bounce_damping = 0.3;
	[slider,min:0,max:1,step:0.01] float slide_friction = 0.92;

	// ---- Rendering ----
	[slider,min:1,max:50] float strand_width = 14.0;
	[slider,min:1,max:40] float drip_radius = 10.0;
	[slider,min:20,max:400] float max_stretch = 90.0;

	// ---- Rhythm ----
	[slider,min:1,max:120] int spurt_duration = 22;
	[slider,min:0,max:300] int spurt_pause = 70;
	[slider,min:1,max:10] int spurts_per_cycle = 3;
	[slider,min:0,max:600] int cycle_pause = 120;

	int spurt_frame;
	int pause_frame;
	int spurt_index;
	bool in_spurt;
	bool in_cycle_pause;
	int current_stream;
	int particle_order;

	// ---- Penis Geometry ----
	[slider,min:-500,max:500] float penis_x = -100;
	[slider,min:-500,max:500] float penis_y = -50;
	[angle,radian] float penis_angle = -1.15;
	[slider,min:20,max:400] float shaft_length = 130;
	[slider,min:10,max:120] float shaft_width = 40;
	[slider,min:5,max:100] float head_radius = 30;
	[slider,min:5,max:100] float ball_radius = 32;
	[slider,min:5,max:100] float ball_gap = 36;

	float tip_x;
	float tip_y;
	float tip_dx;
	float tip_dy;

	// ---- Colours ----
	[colour,alpha] uint fluid_col = 0xFFFFFFFF;
	[colour,alpha] uint fluid_semi = 0xDDFFFFFF;
	[colour,alpha] uint skin_col = 0xFFDEA882;
	[colour,alpha] uint skin_dk = 0xFFC48E5C;
	[colour,alpha] uint head_col = 0xFFE8928A;
	[colour,alpha] uint head_dk = 0xFFD07070;

	script()
	{
		@g = get_scene();
		frame_count = 0;
		spurt_frame = 0;
		pause_frame = 0;
		spurt_index = 0;
		in_spurt = true;
		in_cycle_pause = false;
		current_stream = 0;
		particle_order = 0;
		update_tip();
	}

	void checkpoint_save() {}
	void checkpoint_load()
	{
		particles.resize(0);
		frame_count = 0;
		spurt_frame = 0;
		pause_frame = 0;
		spurt_index = 0;
		in_spurt = true;
		in_cycle_pause = false;
		current_stream = 0;
		particle_order = 0;
	}

	// Emit 2-3 particles per frame as a tight stream
	void emit()
	{
		int count = 2 + rand() % 2;  // 2 or 3
		float px = -tip_dy;
		float py =  tip_dx;

		for(int i = 0; i < count; i++)
		{
			if(int(particles.length()) >= max_particles) return;

			float across = (float(rand() % 9) - 4.0) * 0.5;
			float sx = tip_x + px * across;
			float sy = tip_y + py * across;

			FluidParticle@ p = FluidParticle();
			p.x = sx;
			p.y = sy;
			p.stream_id = current_stream;
			p.order = particle_order;
			particle_order++;

			// Slight speed variation for natural stream
			float spd = launch_speed + float(rand() % 41) - 20.0;
			p.vx = tip_dx * spd + (float(rand() % int(launch_spread * 2 + 1)) - launch_spread);
			p.vy = tip_dy * spd + (float(rand() % int(launch_spread * 2 + 1)) - launch_spread);
			particles.insertLast(p);
		}
	}

	void simulate()
	{
		int n = int(particles.length());
		for(int i = 0; i < n; i++)
		{
			FluidParticle@ p = particles[i];
			p.vy += gravity * DT;
			p.vx *= damping;
			p.vy *= damping;

			float v_sq = p.vx * p.vx + p.vy * p.vy;
			if(v_sq > velocity_cap * velocity_cap)
			{
				float s = velocity_cap / sqrt(v_sq);
				p.vx *= s;
				p.vy *= s;
			}

			float old_x = p.x;
			float old_y = p.y;
			p.x += p.vx * DT;
			p.y += p.vy * DT;

			// Collide with layer 19 tiles
			raycast@ rc = g.ray_cast_tiles(old_x, old_y, p.x, p.y);

			if(rc.hit())
			{
				p.x = rc.hit_x();
				p.y = rc.hit_y();

				// Normal based on tile side: 0=top, 1=bottom, 2=left, 3=right
				int side = rc.tile_side();
				float nx = 0, ny = 0;
				if(side == 0)      { nx =  0; ny = -1; }
				else if(side == 1) { nx =  0; ny =  1; }
				else if(side == 2) { nx = -1; ny =  0; }
				else               { nx =  1; ny =  0; }

				// Reflect velocity: separate into normal and tangent
				float vn = p.vx * nx + p.vy * ny;
				if(vn < 0)  // only if moving into surface
				{
					p.vx -= (1.0 + bounce_damping) * vn * nx;
					p.vy -= (1.0 + bounce_damping) * vn * ny;
				}

				// Friction on tangent
				float vn2 = p.vx * nx + p.vy * ny;
				float tx = p.vx - vn2 * nx;
				float ty = p.vy - vn2 * ny;
				p.vx = vn2 * nx + tx * slide_friction;
				p.vy = vn2 * ny + ty * slide_friction;

				// Push slightly off surface
				p.x += nx * 0.5;
				p.y += ny * 0.5;
			}
		}
	}

	void cull_particles()
	{
		for(int i = int(particles.length()) - 1; i >= 0; i--)
		{
			FluidParticle@ p = particles[i];
			if(p.y > penis_y + 1000 || p.x > penis_x + 1500
				|| p.x < penis_x - 600 || p.y < penis_y - 1200)
				particles.removeAt(i);
		}
	}

	void step(int entities)
	{
		if(frame_count == 0)
			update_tip();
		frame_count++;

		if(in_cycle_pause)
		{
			pause_frame++;
			if(pause_frame >= cycle_pause)
			{
				in_cycle_pause = false;
				spurt_index = 0;
				in_spurt = true;
				spurt_frame = 0;
				current_stream++;
				particle_order = 0;
			}
		}
		else if(in_spurt)
		{
			emit();
			spurt_frame++;
			if(spurt_frame >= spurt_duration)
			{
				in_spurt = false;
				pause_frame = 0;
				spurt_index++;
			}
		}
		else
		{
			pause_frame++;
			if(pause_frame >= spurt_pause)
			{
				if(spurt_index >= spurts_per_cycle)
				{
					in_cycle_pause = true;
					pause_frame = 0;
				}
				else
				{
					in_spurt = true;
					spurt_frame = 0;
					current_stream++;
					particle_order = 0;
				}
			}
		}

		if(particles.length() > 0)
		{
			simulate();
			if(frame_count % 60 == 0)
				cull_particles();
		}
	}

	// ---- Draw penis ----

	void draw_penis()
	{
		float ca = cos(penis_angle);
		float sa = sin(penis_angle);
		float px = -sa;
		float py =  ca;
		float hw = shaft_width * 0.5;

		float ex = penis_x + ca * shaft_length;
		float ey = penis_y + sa * shaft_length;

		// Balls
		float bbx = penis_x - ca * ball_radius * 0.5;
		float bby = penis_y - sa * ball_radius * 0.5;
		drawing::fill_circle(g, 19, 0,
			bbx + px * ball_gap * 0.5, bby + py * ball_gap * 0.5,
			ball_radius, 14, skin_col, skin_dk);
		drawing::fill_circle(g, 19, 0,
			bbx - px * ball_gap * 0.5, bby - py * ball_gap * 0.5,
			ball_radius, 14, skin_col, skin_dk);

		// Shaft
		g.draw_quad_world(19, 1, false,
			penis_x + px * hw, penis_y + py * hw,
			penis_x - px * hw, penis_y - py * hw,
			ex - px * hw, ey - py * hw,
			ex + px * hw, ey + py * hw,
			skin_col, skin_col, skin_dk, skin_dk);

		// Head
		float hx = ex + ca * head_radius * 0.3;
		float hy = ey + sa * head_radius * 0.3;
		drawing::fill_circle(g, 19, 2, hx, hy,
			head_radius, 14, head_col, head_dk);

		// Ridge
		drawing::fill_circle(g, 19, 1,
			ex - ca * 2, ey - sa * 2,
			hw + 4, 14, head_dk, skin_dk);

		// Urethra
		float ux = ex + ca * head_radius * 0.85;
		float uy = ey + sa * head_radius * 0.85;
		drawing::fill_circle(g, 19, 3, ux, uy, 4, 6,
			0xFF804040, 0xFF603030);
	}

	// ---- Draw fluid ----

	void draw(float sub_frame)
	{
		int n = int(particles.length());

		// Sort-free approach: connect consecutive particles in the same stream.
		// Since particles are appended in order and same-stream particles
		// are contiguous in the array (unless culled), we just walk the array
		// and connect neighbors with matching stream_id.

		for(int i = 0; i < n - 1; i++)
		{
			FluidParticle@ a = particles[i];
			FluidParticle@ b = particles[i + 1];

			// Only connect particles from the same stream
			if(a.stream_id != b.stream_id) continue;

			float dx = b.x - a.x;
			float dy = b.y - a.y;
			float d = sqrt(dx * dx + dy * dy);

			// Break if stretched too far (string snaps)
			if(d > max_stretch) continue;

			// Width: thick near tip, thins as string stretches
			float stretch_t = 1.0 - d / max_stretch;
			float w = strand_width * (0.3 + 0.7 * stretch_t);

			g.draw_line_world(18, 0,
				a.x, a.y, b.x, b.y,
				w, fluid_col);
		}

		// Small drip blobs at each particle
		for(int i = 0; i < n; i++)
		{
			FluidParticle@ p = particles[i];
			drawing::fill_circle(g, 18, 1,
				p.x, p.y, drip_radius, 8,
				fluid_col, fluid_semi);
		}

		draw_penis();
	}

	void update_tip()
	{
		float ca = cos(penis_angle);
		float sa = sin(penis_angle);
		tip_x = penis_x + ca * (shaft_length + head_radius * 0.9);
		tip_y = penis_y + sa * (shaft_length + head_radius * 0.9);
		tip_dx = ca;
		tip_dy = sa;
	}

	void editor_step()
	{
		update_tip();
		step(0);
	}

	void editor_draw(float sub_frame)
	{
		draw(sub_frame);
	}
}