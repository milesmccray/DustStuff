#include "../lib/pos.as"
#include "../lib/SeedGenerator.as"
class script : callback_base {

	scene@ g;
	dustman@ p;
	
	SeedGenerator s;
	bool seedset = false;
	
	[text] pos base;
	[text] float length;
	[entity] uint thetire;
	
	pinata pin;
	
	int bags, endtimer;
	bool sct; //suspend combo timer
	
	script() {
		@g = get_scene();
		s = SeedGenerator();
	}
	
	void on_level_start() {
		@p = controller_controllable(0).as_dustman();
		pin = pinata(thetire, length, base.x(), base.y());
		entity_by_id(thetire).as_hittable().on_hurt_callback(this, "tire_hit", 3);
		g.override_sound("sfx_trashbag_explode", "sfx_hud_cancel_4", false);
		bags = 0;
		endtimer = -2;
		sct = false;
	}
	
	void step(int entities) {
		if (!seedset) {
			if (!s.ready()) {
				s.step();
			} else {
				srand(s.getSeed());
				seedset = true;
			}
		}
		
		if (endtimer >= 0) {
			endtimer--;
			if (endtimer == 0) {
				g.end_level(0, 0);
			}
		}
		
		if (sct) {
			p.combo_timer(1);
		}
		
		pin.step();
	}
	
	void entity_on_remove(entity@ e) {
		if (e.type_name() == "enemy_trash_tire") {
			sct = true;
		}
		if (e.type_name() == "enemy_trash_bag") {
			bags++;
			if (bags >= 225) {
				endtimer = 3;
			}
		}
	}
	
	void tire_hit(controllable@ attacked, controllable@ attacker, hitbox@ attack_hitbox, int arg) {
		attack_hitbox.attack_strength(attack_hitbox.attack_strength() * 2.5);
		for (int i = 0; i < attack_hitbox.damage() * 5; i++) {
			entity@ e = create_entity("enemy_trash_bag");
			e.x(attacked.x());
			e.y(attacked.y());
			g.add_entity(e);
			e.as_controllable().set_speed_direction(450 + rand() % 300, -45 + rand() % 90);
			e.as_hittable().scale(0.5);
			e.as_hittable().life(100);
		}
	}
	
	void draw(float subframe) {
		pin.draw(g);
	}

}

class pinata {
	
	entity@ tire;
	float l, basex, basey;
	
	pinata() {
		l = 1;
		basex = 0;
		basey = 0;
	}
	
	pinata(uint enemy, float length, float base_x, float base_y) {
		@tire = entity_by_id(enemy);
		tire.as_hittable().life(45);
		l = length;
		basex = base_x;
		basey = base_y;
	}
	
	void step() {
		float d = dist_from_base();
		if (d > l) {
			//snap to string
			tire.x(basex + (tire.x() - basex) * l / d);
			tire.y(basey + (tire.y() - basey) * l / d);
			//randomly jerk towards string
			tire.as_controllable().set_speed_direction(400 + rand() % 200, dir_to_base() - 45 + rand() % 90);
		}
	}
	
	void draw(scene@ g) {
		g.draw_line_world(17, 3, basex, basey, tire.x(), tire.y(), 5, 0xFFFFFFFF);
	}
	
	float dist_from_base() {
		return sqrt(pow(tire.x() - basex, 2) + pow(tire.y() - basey, 2));
	}
	
	int dir_to_base() {
		int dir = int32((180 / 3.14159) * atan(-1 * (tire.x() - basex) / (tire.y() - basey)));
		if (basey > tire.y())
			dir += 180;
		return dir;
	}
	
}