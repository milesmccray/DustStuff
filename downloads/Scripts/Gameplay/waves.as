/* 
"Waves" script made for CMJ 3, which I used in my map "Hollow Elegy".
Thank you to Skyhawk, C, and AvengedRuler for help making the script.
http://atlas.dustforce.com/11175/hollow-elegy
*/

class script : callback_base {
	scene@ g;
	array <Wave> waves;
	array <hitbox@> hitboxes;
	int sound = 1;

	script() {
		@g = get_scene();
	}

	void entity_on_add(entity@ e) {
		message@ meta = e.metadata();
		if (meta.has_int("is_wavy")) return; // IF ENTITY IS PART OF THE WAVE, JUST SKIP IT

		if (e.type_name() == "effect") {
			effect@ fx = e.as_effect();
			string spr = fx.sprite_index();
			if (@fx.freeze_target() == null) return; // FILTER OUT NON-ATTACKS
			if (spr.findFirst('heavy') == -1 && spr.findFirst('strike') == -1) return;
			for(int i = int(hitboxes.length()) - 1; i >= 0; i--) { // SO HERE WE ITERATE BACKWARDS THROUGH ALL THE HITBOXES IN OUR ARRAY
				if (hitboxes[i].owner().is_same(@fx.freeze_target().as_dustman())) { // ONLY CHECK IF HITBOX AND EFFECT HAVE SAME OWNER
					if(hitboxes[i].state_timer() < hitboxes[i].activate_time() + 1) { // FILTER ORPHAN HITBOXES
						// RANDOMIZES SOUND THE ATTACK PLAYS IF IT HITS AN ENEMY
						sound = 1 + (sound + rand() % 2) % 3;
						Wave w(@fx, hitboxes[i], @fx.freeze_target().as_dustman(), sound);
						waves.insertLast(w);
					}
					hitboxes.removeAt(i);
				}
			}
		}

		// IF E IS A HITBOX AND FROM A PLAYER THEN SAVE IT
		if (@e.as_hitbox() != null && @e.as_hitbox().owner() != null && e.as_hitbox().owner().team() == 1) {
			hitboxes.insertLast(e.as_hitbox());
		}
	}
	
	void step(int entities) {
		//THIS STEPS EVERY EXISTING WAVE
		for(uint i = 0; i < waves.length(); i++) {
			waves[i].step(@g);
		}
	}

	void on_level_start() {
		initialize();
	}

	void checkpoint_load() {
		initialize();
	}
	
	void initialize() {
		for(uint i=0; i < num_cameras() && @controller_controllable(i) != null; i++) {
			controller_controllable(i).on_hit_callback(this, "on_hit", 0);
		}
	}

	void on_hit(controllable@ attacker, controllable@ attacked, hitbox@ attack_hitbox, int arg) {
		message@ meta = attack_hitbox.metadata();
		if (meta.has_int("is_wavy")) return;
		for(int i = int(hitboxes.length()) - 1; i >= 0; i--) {
			if (attack_hitbox.as_entity().is_same(hitboxes[i].as_entity())) {
				hitboxes.removeAt(i);
			}
		}
	}
}


class Wave	{
	effect@ fx;
	hitbox@ h;
	hitbox@ h_old;
	dustman@ dm;
	int sound;
	
	float x_offset = 0;
	float y_offset = 0;
	float speed = 35;
	int timer = 0;
	int status = 0;
	array <Ref> refs;
	int combo = 0;

	Wave(effect@ fx, hitbox@ h, dustman@ dm, int sound) {
		@this.fx = fx;
		@this.h = @this.h_old = h;
		@this.dm = dm;
		this.sound = sound;
	}

	void step(scene@ g) {
		if (status == 3) return;
		if (status == 0) {
			speed = speed - 1.4;
			//THIS SWITCH DEFINES THE ANGLES THE WAVES TRAVEL AT
			switch (h.attack_dir()) {
				case 30:
					x_offset = speed * 0.5;
					y_offset = speed * -sqrt(3) / 2;
					break;
				case 85:
					x_offset = speed;
					y_offset = 0;
					break;
				case 150:
				case 151:
					x_offset = speed * 0.5;
					y_offset = speed * sqrt(3) / 2;
					break;
				case -30:
					x_offset = speed * -0.5;
					y_offset = speed * -sqrt(3) / 2;
					break;
				case -85:
					x_offset = -speed;
					y_offset = 0;
					break;
				case -150:
				case -151:
					x_offset = speed * -0.5;
					y_offset = speed * sqrt(3) / 2;
					break;
			}

			// THIS SECTION MOVES THE EFFECT ENTITY
			fx.x(fx.x() + x_offset);
			fx.y(fx.y() + y_offset);
			fx.time_warp(0.5);

			// THIS SECTION CREATES AND PLACES A NEW HITBOX
			hitbox@ h_out = copy_hitbox(h); // CREATES A HITBOX COPY TO PLACE
			message@ meta = h_out.metadata(); // GETS A HANDLE FOR THE COPY'S METADATA
			meta.set_int("is_wavy", 1); // SETS A METADATA FLAG THAT THIS HITBOX IS PART OF THE WAVE
			h_out.activate_time(0); // MAKES IT ACTIVE ON THE FIRST FRAME POSSIBLE
			h_out.timer_speed(300); // CRANKS TIMER SPEED TO QUICKLY CULL HITBOXES AFTER ACTIVATION
			h_out.x(h_old.x() + x_offset); // SET X TO ADJUSTED VALUE
			h_out.y(h_old.y() + y_offset); // SET Y TO ADJUSTED VALUE
			h_out.attack_ff_strength(0); // SETS FREEZE EFFECT TO ZERO
			g.add_entity(h_out.as_entity()); // PLACES COPY ONTO THE SCENE
			@h_old = h_out; // SAVES THE OLD HITBOX FOR FUTURE REFERENCE

			// THIS SECTION USES PROJECT_TILE_FILTH TO CLEAN FILTH OFF SURFACES
			g.project_tile_filth(h_out.x(), h_out.y(), h_out.base_rectangle().get_width(), h_out.base_rectangle().get_height(), 0, h_out.attack_dir(), 200, 30, true, true, true, true, false, true);

			// THIS SECTION CHECKS TO SEE IF THE HITBOX COLLIDES WITH AN ENEMY AND STOPS IT IF SO
			int col_int = g.get_entity_collision(h_out.y() + h_out.base_rectangle().top(), h_out.y() + h_out.base_rectangle().bottom(), h_out.x() + h_out.base_rectangle().left(), h_out.x() + h_out.base_rectangle().right(), 7);
			for(int i = 0; i < col_int; i++) { // IF WE HIT A "HITTABLE"
				entity@ hit_ent = g.get_entity_collision_index(i); // GET THAT ENTITY'S HANDLE
				if (@hit_ent == null) continue; // STOP IF ENTITY IS NULL
				hittable@ hit_hit = @hit_ent.as_hittable(); // CAST ENTITY TO HITTABLE
				if (@hit_hit == null) continue; // STOP IF HITTABLE IS NULL
				if (hit_hit.team() != 0) continue; // STOP IF HITTABLE ISN'T ON "TEAM FILTH"

				string type = h_out.damage() == 1 ? "light" : "heavy"; // DETECTS ATTACK TYPE (THANKS C <3)
				g.play_sound("sfx_impact_" + type + "_" + sound, hit_ent.x(), hit_ent.y(), 1, false, true);
				
				// THIS SAVES INFO ABOUT THE HIT FOR FUTURE REFERENCE
				Ref r(@hit_ent, @h_out, h_out.damage(), hit_hit.life());
				refs.insertLast(r);
				status = 1;
			}

			// THIS SECTION CHECKS TO SEE IF THE HITBOX HIT A WALL AND STOPS IT IF SO
			// SOME TRIG TO CREATE A "REASONABLE" RAYCAST THROUGHT THE CENTER OF THE HITBOX
			float mid_x = (h_out.x() + (h_out.base_rectangle().left() + h_out.base_rectangle().right()) / 2);
			float mid_y = (h_out.y() + (h_out.base_rectangle().top() + h_out.base_rectangle().bottom()) / 2);
			float diag = sqrt((h_out.base_rectangle().get_width()*h_out.base_rectangle().get_width()) + (h_out.base_rectangle().get_height()*h_out.base_rectangle().get_height()));
			const float pi = 3.141592653589;
			float start_x = mid_x - sin(h_out.attack_dir() * pi / 180) * diag / 4;
			float start_y = mid_y + cos(h_out.attack_dir() * pi / 180) * diag / 4;
			float end_x = mid_x + sin(h_out.attack_dir() * pi / 180) * diag / 4;
			float end_y = mid_y - cos(h_out.attack_dir() * pi / 180) * diag / 4;
			raycast@ ray = g.ray_cast_tiles(start_x, start_y, end_x, end_y);
			if (ray.hit()) {
				status = 1;
			}

			// THIS SECTION INCREMENTS THE TIMER AND STOPS THE HITBOX IF IT TIMES OUT
			timer++;
			if (timer > 20) { 
				status = 1;
			}
		} else if (status == 1) {
			status = 2;
		} else if (status == 2) { // ONCE THE WAVE HAS STOPPED, WE LOOK AT ENTITIES IT HIT TO ALLOCATE COMBO
			for(uint i = 0; i < refs.length(); i++) {
				if (refs[i].h.hit_outcome() == 1) { // IN THE CASE OF A HIT
					if (refs[i].e.type_name() == "enemy_tutorial_hexagon") {
						combo = 3; // A SUCCESSFUL HIT ON A BIG PRISM ALWAYS GIVES 3 COMBO
					} else if (refs[i].e.type_name() == "base_projectile") {
						combo = 1; // HITTING A PORCUPINE QUILL ALWAYS GIVES 1 COMBO
					} else if (refs[i].e.type_name() == "enemy_spring_ball") {
						if (refs[i].life == refs[i].e.as_controllable().life()) { // IF THE HIT DIDN'T HURT THE SPRINGBLOB
							combo = refs[i].damage; // GIVE COMBO FOR FULL DAMAGE
						} else { // OTHERWISE NORMAL COMBO MATH APPLIES
							combo = refs[i].life > refs[i].damage ? refs[i].damage: refs[i].life;
						}
					} else {
						combo = refs[i].life > refs[i].damage ? refs[i].damage: refs[i].life;
					}
				} else if (refs[i].h.hit_outcome() == 3) { // IN THE CASE OF A PARRY
					combo = 2*refs[i].damage; // PARRY RETURNS TWICE THE DAMAGE AS COMBO
				} else {
					puts("Unexpected hit outcome: " + refs[i].h.hit_outcome());
				}
				dm.combo_count(dm.combo_count() + combo);
				dm.combo_timer(1);
			}
			status = 3;
		}
	}
	
	Wave() {
		//
	}
}

class Ref {
	entity@ e;
	hitbox@ h;
	int damage;
	int life;

	Ref(entity@ e, hitbox@ h, int damage, int life) {
		@this.e = e;
		@this.h = h;
		this.damage = damage;
		this.life = life;
	}
	
	Ref() {
		//
	}
}

//THIS COPIES THE PROPERTIES OF ONE HITBOX TO ANOTHER
hitbox@ copy_hitbox(hitbox@ hb) {
	hitbox@ hb_new;
	
	@hb_new = create_hitbox(hb.owner(), hb.activate_time(), hb.x(), hb.y(), hb.base_rectangle().top(), hb.base_rectangle().bottom(), hb.base_rectangle().left(), hb.base_rectangle().right());
	hb_new.damage(hb.damage());
	hb_new.filth_type(hb.filth_type());
	hb_new.state_timer(hb.state_timer());
	hb_new.timer_speed(hb.timer_speed());
	hb_new.attack_ff_strength(hb.attack_ff_strength());
	hb_new.parry_ff_strength(hb.parry_ff_strength());
	hb_new.stun_time(hb.stun_time());
	hb_new.can_parry(hb.can_parry());
	hb_new.attack_dir(hb.attack_dir());
	hb_new.attack_strength(hb.attack_strength());
	hb_new.team(hb.team());
	hb_new.attack_effect(hb.attack_effect());
	hb_new.effect_frame_rate(hb.effect_frame_rate());
	return(hb_new);
}