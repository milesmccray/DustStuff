#include "../lib/pos.as"
#include "../lib/SeedGenerator.as"
const string EMBED_blue = "crimson-blue.png";
const string EMBED_green = "crimson-green.png";
const string EMBED_mask = "crimson-mask.png";
const string EMBED_red = "crimson-red.png";
const string EMBED_yellow = "crimson-yellow.png";

const string EMBED_hit = "crimson-shot.ogg";
const string EMBED_dead = "crimson-dead.ogg";
const string EMBED_song = "crimson-song.ogg";

const float PI = 3.1415926535897932384626433832795;
const float PI2 = PI * 2;
const float HALF_PI = PI / 2;
const float DEG2RAD = 1.0 / 180.0 * PI;
const float RAD2DEG = 1.0 / PI * 180.0;

class script : callback_base {

	scene@ g;
	dustman@ p;
	sprites@ spr;
	SeedGenerator sg;
	bool seedSet = false;
	
	bool watermode = false;
	[text] pos platformSpawner;
	array<platform> platforms;
	[text] pos waterSpawner;
	array<water> waters;
	[text] pos platformEnd;
	int minterval = 2;
	int maxterval = 6;
	int spawnTimer;
	
	[text] pos startPlat;
	platform starter;
	int levelTimer = 0;
	
	array<bullet> bullets;
	int facing = 1;
	
	[text] array<pos> bossLocs;
	[text] array<pos> hpBars;
	array<string> bosses = {"blue", "green", "red", "yellow"};
	int cyc = 0;
	array<int> bossHP = {20, 20, 20, 20};
	array<int> bossInvulnTimer = {0, 0, 0, 0};
	array<bool> drawBoss = {true, true, true, true};
	int cycleTimer = 300;
	int redShootTimer = 204;
	
	[text] array<pos> outNbounce;
	array<proj> projs;
	[text] array<pos> blueSpawns;
	[text] array<pos> greenSpawns;
	[text] array<pos> zKillSpots;
	[entity] array<uint> zKillTriggers;
	
	bool maskSpawned = false;
	float maskX, maskY;
	int maskTimer = 0;
	
	int prevState = 0;
	int hitsTaken = 0;
	
	int endTimer = -2;
	
	script() {
		@g = get_scene();
		@spr = create_sprites();
		sg = SeedGenerator();
		//spr.add_sprite_set("script");
	}
	
	void on_level_start() {
		@p = controller_controllable(0).as_dustman();
		spr.add_sprite_set("script");
		spr.add_sprite_set("props2");
		spawnTimer = 5;
		starter = platform(startPlat.x(), startPlat.y(), startPlat.x() + 96, startPlat.y() + 96, -1000);
		starter.spd = 0;
		g.play_persistent_stream("song", 1, true, 0.4, true);
		g.disable_score_overlay(true);
	}
	
	void step(int entities) {
		if (!seedSet) {
			sg.step();
			if (sg.ready()) {
				srand(sg.getSeed());
				seedSet = true;
			}
		} else {
			spawnTimer--;
			if (spawnTimer == 0) {
				if (!watermode) {
					float x = platformSpawner.x();
					float y = platformSpawner.y();
					platforms.insertLast(platform(x, y, x + 96, y + 96, platformEnd.x()));
				} else {
					float x = waterSpawner.x();
					float y = waterSpawner.y();
					waters.insertLast(water(x, y, x + 144, y + 144, platformEnd.x()));
				}
				spawnTimer = (rand() % (maxterval - minterval + 1) + minterval) * 40;
			}
		}
		
		
		levelTimer++;
		if (levelTimer == 205) {
			placeSpikes();
		}
		if (levelTimer == 325) {
			starter.cleanUp(g);
		} else if (levelTimer < 325) {
			starter.step(p, g);
		}
		
		if (p.x_intent() != 0)
			facing = p.x_intent();
		
		if (p.light_intent() == 10) {
			p.light_intent(11);
			if (bullets.length() < 5) {
				int spd = 35 * facing;
				bullets.insertLast(bullet(p.x(), p.y() - 64, spd));
			}
		}
		if (p.heavy_intent() == 10)
			p.heavy_intent(11);
		
		
		for (int i = 0; i < platforms.length(); i++) {
			if (platforms[i].pastEnd()) {
				platforms[i].cleanUp(g);
				platforms.removeAt(i);
			} else {
				platforms[i].step(p, g);
			}
		}
		for (int i = 0; i < waters.length(); i++) {
			if (waters[i].pastEnd()) {
				waters.removeAt(i);
			} else {
				waters[i].step(p);
			}
		}
		for (int i = bullets.length() - 1; i >= 0; i--) {
			if (bullets[i].hit() != 0) {
				hitBoss(bullets[i].hit());
				bullets.removeAt(i);
			} else if (bullets[i].outside()) {
				bullets.removeAt(i);
			} else {
				bullets[i].step();
			}
		}
		
		cycleTimer--;
		if (cycleTimer == 0) {
			cycle();
			cycleTimer = 240;
		}
		if (bossHP[2] > 0) {
			redShootTimer--;
			if (redShootTimer == 0) {
				redShoot();
				redShootTimer = 144;
			}
		}
		for (int i = 0; i < 4; i++) {
			if (bossInvulnTimer[i] > 0) {
				bossInvulnTimer[i]--;
				if (bossInvulnTimer[i] == 59 || bossInvulnTimer[i] == 20) {
					drawBoss[i] = false;
				} else if (bossInvulnTimer[i] == 40 || bossInvulnTimer[i] == 0) {
					drawBoss[i] = true;
				}
			}
		}
		for (int i = projs.length() - 1; i >= 0; i--) {
			if (projs[i].checkOut()) {
				projs.removeAt(i);
			} else {
				projs[i].step();
				if (projs[i].hitPlayer(p)) {
					projs[i].activeTimer = 20;
					hitbox@ h = create_hitbox(p.as_controllable(), 0, projs[i].x, projs[i].y, projs[i].hbr * -3, projs[i].hbr * 3, projs[i].hbr * -3, projs[i].hbr * 3);
					h.team(0);
					h.aoe(true);
					g.add_entity(h.as_entity(), false);
				}
			}
		}
		if (maskSpawned) {
			if (maskTimer > 0)
				maskTimer--;
			if (maskTimer == 0 && p.x() - 24 < maskX + 222 && p.x() + 24 > maskX && p.y() > maskY && p.y() - 96 < maskY + 137) {
				maskTimer = 10;
				hitbox@ h = create_hitbox(p.as_controllable(), 0, maskX + 111, maskY + 68.5, -68.5, 68.5, -111, 111);
				h.team(0);
				h.aoe(true);
				g.add_entity(h.as_entity(), false);
			}
			float xdif = p.x() - (maskX + 111);
			float ydif = p.y() - 48 - (maskY + 68.5);
			maskX += xdif * 7 / sqrt(xdif * xdif + ydif * ydif);
			maskY += ydif * 7 / sqrt(xdif * xdif + ydif * ydif);
		}
		
		if (prevState != 20 && p.state() == 20) {
			hitsTaken++;
		}
		prevState = p.state();
		
		if (endTimer >= 0) {
			if (endTimer == 0) {
				maskSpawned = false;
				g.combo_break_count(hitsTaken);
				g.end_level(0, 0);
			}
			endTimer--;
		}
		
		//debug, remove before finishing
		if (p.taunt_intent() == 1) {
			
		}
		
	}
	
	void draw(float subframe) {
		if (levelTimer <= 325) {
			starter.draw(g);
		}
		for (int i = 0; i < platforms.length(); i++) {
			platforms[i].draw(g);
		}
		for (int i = 0; i < waters.length(); i++) {
			waters[i].draw(g);
		}
		for (int i = 0; i < bullets.length(); i++) {
			bullets[i].draw(g);
		}
		for (int i = 0; i < 4; i++) {
			if (drawBoss[(i - cyc + 4) % 4]) {
				spr.draw_world(20, 1, bosses[i], 0, 1, bossLocs[i].x(), bossLocs[i].y(), 0, 1, 1, 0xFFFFFFFF);
			}
			g.draw_rectangle_world(20, 1, hpBars[i].x(), hpBars[i].y(), hpBars[i].x() + (110.4 * bossHP[i]), hpBars[i].y() + 48, 0, colorPick(i));
		}
		for (int i = 0; i < projs.length(); i++) {
			projs[i].draw(spr);
		}
		if (maskSpawned) {
			spr.draw_world(20, 1, "mask", 0, 1, maskX, maskY, 0, 1, 1, 0xFFFFFFFF);
		}
		g.draw_rectangle_world(18, 11, p.x() + 16 * facing, p.y() - 67, p.x() + 35 * facing, p.y() - 61, 0, 0xFF000000);
		g.draw_rectangle_world(18, 11, p.x() + 16 * facing, p.y() - 61, p.x() + 22 * facing, p.y() - 55, 0, 0xFF000000);
	}
	
	uint colorPick(int spot) {
		switch (spot) {
			case 0:
				return 0xFF0000FF;
			case 1:
				return 0xFF00FF00;
			case 2:
				return 0xFFFF0000;
			case 3:
				return 0xFFFFFF00;
		}
		return 0xFFFFFFFF;
	}
	
	void editor_draw(float subframe) {
		/*for (int i = 3; i >= 0; i--) {
			spr.draw_world(20, 1, bosses[i], 0, 1, bossLocs[0].x() + 300 * i, bossLocs[0].y(), 0, 1, 1, 0xFFFFFFFF);
		}*/
	}
	
	void hitBoss(int place) {
		int hit = (place - cyc + 3) % 4;
		if (bossHP[hit] > 0 && bossInvulnTimer[hit] == 0) {
			g.play_script_stream("hit", 0, 0, 0, false, 0.8);
			bossHP[hit]--;
			if (bossHP[hit] == 0) {
				drawBoss[hit] = false;
				bossInvulnTimer[hit] = -1;
				g.play_script_stream("dead", 0, 0, 0, false, 0.8);
				if (bosses[place - 1] == "green") {
					maskSpawned = true;
					maskX = bossLocs[place - 1].x() + 128;
					maskY = bossLocs[place - 1].y() + 3;
				}
				g.remove_entity(entity_by_id(zKillTriggers[hit]));
			} else {
				bossInvulnTimer[hit] = 60;
			}
			if (hit == 0 && bossHP[hit] == 8) {
				watermode = true;
			}
			bossShoot(place - 1);
			if (allDead()) {
				endTimer = 3;
				removeSpikes();
				for (int i = projs.length() - 1; i >= 0; i--) {
					projs.removeAt(i);
				}
			}
		}
	}
	
	void bossShoot(int spot) {
		if (bosses[spot] == "blue") {
			int count = min((23 - bossHP[0]) / 4, 4);
			for (int i = 0; i < count; i++) {
				projs.insertLast(proj(blueSpawns[i].x(), blueSpawns[i].y(), 0, 5.4, 45 + 90 * i, false, outNbounce, 2));
			}
		} else if (bosses[spot] == "green") {
			int count = min((23 - bossHP[1]) / 4, 4);
			for (int i = 0; i < count; i++) {
				float x = 0;
				float y = 0;
				if (i % 2 == 0) {
					x = greenSpawns[i].x();
					y = greenSpawns[i].y() + int(rand() % 240);
				} else {
					y = greenSpawns[i].y();
					x = greenSpawns[i].x() + int(rand() % 1248);
				}
				projs.insertLast(proj(x, y, 0, 4.8, 180 + 90 * i, false, outNbounce, 3));
			}
		} else if (bosses[spot] == "red") {
			if (bossHP[2] < 8) {
				projs.insertLast(proj(bossLocs[spot].x() + 220, bossLocs[spot].y() + 216, 0, 4.8, rand() % 360, true, outNbounce, 4));
			}
		} else if (bosses[spot] == "yellow") {
			float ang = getAngle(spot);
			float cur = 0;
			if (bossHP[3] < 9)
				cur = 0.6 - 0.01 * bossHP[3];
			for (int i = 0; i < 9; i++) {
				projs.insertLast(proj(bossLocs[spot].x() + 220, bossLocs[spot].y() + 216, cur, 7, ang + (40 * i), false, outNbounce, 5));
			}
		}
	}
	
	void redShoot() {
		int spot = (2 + cyc) % 4;
		float ang = getAngle(spot);
		projs.insertLast(proj(bossLocs[spot].x() + 220, bossLocs[spot].y() + 216, 0, 9.6, ang, false, outNbounce, 4));
	}
	
	bool allDead() {
		return bossHP[0] == 0 && bossHP[1] == 0 && bossHP[2] == 0 && bossHP[3] == 0;
	}
	
	float getAngle(int spot) {
		float xdif = p.x() - (bossLocs[spot].x() + 220);
		float ydif = (p.y() - 48) - (bossLocs[spot].y() + 216);
		float ang = atan(ydif / xdif) * RAD2DEG;
		if (p.x() < bossLocs[spot].x() + 220) {
			ang += 180;
		}
		return ang;
	}
	
	void cycle() {
		bosses.insert(0, bosses[3]);
		bosses.removeAt(4);
		cyc = (cyc + 1) % 4;
		for (int i = 0; i < 4; i++) {
			if (bossHP[i] > 0) {
				entity_by_id(zKillTriggers[i]).x(zKillSpots[(i + cyc) % 4].x());
				entity_by_id(zKillTriggers[i]).y(zKillSpots[(i + cyc) % 4].y());
			}
		}
	}
	
	void placeSpikes() {
		for (int i = -21; i < 22; i++) {
			g.set_tile_filth(i, 0, 11, 0, 0, 0, true, true);
		}
	}
	
	void removeSpikes() {
		for (int i = -21; i < 22; i++) {
			g.set_tile_filth(i, 0, 0, 0, 0, 0, true, true);
		}
	}
	
	void build_sprites(message@ msg) {
		msg.set_string("blue", "blue");
		msg.set_string("green", "green");
		msg.set_string("mask", "mask");
		msg.set_string("red", "red");
		msg.set_string("yellow", "yellow");
	}
	
	void build_sounds(message@ msg) {
		msg.set_string("hit", "hit");
		msg.set_string("dead", "dead");
		msg.set_string("song", "song");
		msg.set_int("song|loop", 233110);
	}

}


class proj {
	
	float x, y, curve, spd, dir, xspd, yspd;
	bool bounce;
	//should be 4 members in order: out top left, out bottom right, bounce top left, bounce bottom right
	array<pos> outNbounce;
	//hitbox radius, this is for tweaking the hitbox of the projectiles (and also it's more like half side length of the square but still)
	int hbr = 10;
	int activeTimer = 0;
	int sl;
	
	proj() {
		x = 0;
		y = 0;
		curve = 0;
		spd = 0;
		dir = 0;
		xspd = 0;
		yspd = 0;
		bounce = false;
	}
	
	//direction in degrees
	proj(float X, float Y, float Curve, float Speed, float Direction, bool Bounce, array<pos> OutAndBounce, int sublayer) {
		x = X;
		y = Y;
		curve = Curve;
		spd = Speed;
		dir = Direction;
		xspd = spd * cos(dir * DEG2RAD);
		yspd = spd * sin(dir * DEG2RAD);
		bounce = Bounce;
		outNbounce = OutAndBounce;
		sl = sublayer;
	}
	
	void step() {
		x += xspd;
		y += yspd;
		if (curve != 0) {
			dir += curve;
			xspd = spd * cos(dir * DEG2RAD);
			yspd = spd * sin(dir * DEG2RAD);
		}
		if (bounce) {
			tryBounce();
		}
		if (activeTimer > 0)
			activeTimer--;
	}
	
	bool hitPlayer(dustman@ p) {
		return activeTimer == 0 && x - hbr < p.x() + 24 && x + hbr > p.x() - 24 && y - hbr < p.y() && y + hbr > p.y() - 96;
	}
	
	void tryBounce() {
		if (x < outNbounce[2].x() || x > outNbounce[3].x()) {
			xspd *= -1;
		}
		if (y < outNbounce[2].y() || y > outNbounce[3].y()) {
			yspd *= -1;
		}
	}
	
	bool checkOut() {
		return x < outNbounce[0].x() || x > outNbounce[1].x() || y < outNbounce[0].y() || y > outNbounce[1].y();
	}
	
	void draw(sprites@ spr) {
		spr.draw_world(20, sl, "foliage_25", 1, 1, x, y - 5, 0, 1, 1, 0xFFFFFFFF);
		//g.draw_rectangle_world(20, 7, x - hbr, y - hbr, x + hbr, y + hbr, 0, 0xFFFF0000);
	}
	
}

class bullet {
	
	float x, y, spd;
	float minX = -1008;
	float maxX = 1008;
	float bossX = 624;
	array<float> bossYs = {-1392, -960, -480, -48};
	
	bullet() {
		x = 0;
		y = 0;
		spd = 40;
	}
	
	//use negative speed to go left
	bullet(float X, float Y, float speed) {
		x = X;
		y = Y;
		spd = speed;
	}
	
	void step() {
		x += spd;
	}
	
	void draw(scene@ g) {
		g.draw_rectangle_world(19, 5, x - 10, y - 5, x + 10, y + 5, 0, 0xFF000000);
		g.draw_rectangle_world(19, 5, x - 5, y - 10, x + 5, y + 10, 0, 0xFF000000);
		g.draw_rectangle_world(19, 6, x - 5, y - 5, x + 5, y + 5, 0, 0xFFFFFFFF);
	}
	
	int hit() {
		if (x < bossX * -1) {
			if (y >= bossYs[0] && y <= bossYs[1])
				return 1;
			if (y >= bossYs[2] && y <= bossYs[3])
				return 4;
		} else if (x > bossX) {
			if (y >= bossYs[0] && y <= bossYs[1])
				return 2;
			if (y >= bossYs[2] && y <= bossYs[3])
				return 3;
		}
		return 0;
	}
	
	bool outside() {
		return x < minX || x > maxX;
	}
	
}

class platform {
	//this platform moves without bringing the player along
	//the top of the platform should be a multiple of 48 or else it will look weird
	
	float x1, y1, x2, y2, endX;
	float spd = 2.4;
	array<tile> tiles;
	
	platform() {
		x1 = 0;
		y1 = 0;
		x2 = 0;
		y2 = 0;
		endX = 0;
	}
	
	platform(float X1, float Y1, float X2, float Y2, float EndX) {
		x1 = X1;
		y1 = Y1;
		x2 = X2;
		y2 = Y2;
		endX = EndX;
	}
	
	void step(dustman@ p, scene@ g) {
		cleanUp(g);
		if (xAligned(p)) {
			//some code mostly taken from alex's break the targets map
			int l = int(floor(x1/48.0));
			int r = int(ceil(x2/48.0));
			int h = int(floor(y1/48.0));
			for (int i = l; i < r; ++i) {
				if (!g.get_tile(i, h).solid()) {
					tile t;
					t.x = i;
					t.y = h;
					t.update(g, true);
					tiles.insertLast(t);
				}
			}
			
			if (p.jump_intent() == 1 && yAligned(p)) {
				p.y(y1);
				p.set_speed_xy(p.x_speed(), 0);
				p.jump_intent(2);
			}
		}
		
		x1 -= spd;
		x2 -= spd;
	}
	
	void cleanUp(scene@ g) {
		for (int i=0; i<tiles.length(); ++i) {
            tiles[i].update(g, false);
        }
        tiles.resize(0);
	}
	
	void draw(scene@ g) {
		g.draw_rectangle_world(19, 3, x1, y1, x2, y2, 0, 0xFF000000);
		g.draw_rectangle_world(19, 3, x1 + 4, y1 + 4, x2 - 4, y2 - 4, 0, 0xFFEE0000);
		g.draw_quad_world(19, 3, false, x1 + 48, y1, x1, y1 + 48, x1 + 48, y2, x2, y1 + 48, 0xFF440000, 0xFF440000, 0xFF440000, 0xFF440000);
	}
	
	bool xAligned(dustman@ p) {
		return p.x() + 24 > x1 && p.x() - 24 < x2;
	}
	
	bool yAligned(dustman@ p) {
		return p.y() > y1 && p.y() - 96 < y2;
	}
	
	bool pastEnd() {
		return x2 <= endX;
	}
	
}

class water {
	
	float x1, y1, x2, y2, endX;
	float spd = 2.4;
	float fallspd = 400;
	
	water() {
		x1 = 0;
		y1 = 0;
		x2 = 0;
		y2 = 0;
		endX = 0;
	}
	
	water(float X1, float Y1, float X2, float Y2, float EndX) {
		x1 = X1;
		y1 = Y1;
		x2 = X2;
		y2 = Y2;
		endX = EndX;
	}
	
	void step(dustman@ p) {
		if (playerIn(p)) {
			if (p.y_speed() > fallspd)
				p.set_speed_xy(p.x_speed(), fallspd);
			if ((p.jump_intent() == 1 || p.dash_intent() == 1) && p.dash() < p.dash_max()) {
				p.dash(p.dash_max());
			}
		}
		x1 -= spd;
		x2 -= spd;
	}
	
	void draw(scene@ g) {
		g.draw_rectangle_world(19, 3, x1, y1, x2, y2, 0, 0x8000AAFF);
	}
	
	bool playerIn (dustman@ p) {
		return (p.x() + 24 > x1 && p.x() - 24 < x2) && (p.y() > y1 && p.y() - 96 < y2);
	}
	
	bool pastEnd() {
		return x2 <= endX;
	}
	
}

//this class shamelessly stolen from alex's break the targets script
class tile {
    int x, y;
    tileinfo@ t;

    tile() {
        @t = @create_tileinfo();
        // Make the tile invisible
        t.sprite_tile(0);
        // Make the tile square
        t.type(0);
        // Keep only the top edge
        t.edge_top(15);
        t.edge_bottom(0);
        t.edge_left(0);
        t.edge_right(0);
    }

    void update(scene@ g, bool solid) {
        t.solid(solid);
        g.set_tile(x, y, 19, t, false);
    }
}