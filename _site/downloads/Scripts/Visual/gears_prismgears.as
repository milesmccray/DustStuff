const float PI = 3.1415926535897932384626433832795;
const float FRAME = 1.0 / 60.0;

const float HEXAGON_ROTATE_SPEED = -0.706;
const float SQUARE_ROTATE_SPEED = 1;

const float GEAR_DX = -103 - 175 * 0.5;
const float GEAR_DY = -102 - 178 * 0.5;
const float GEAR_WIDTH = 175;
const float GEAR_HEIGHT = 178;



class EntityData {
	[entity|tooltip:"select a hexagon"] uint id;
	[text|tooltip:"multiplier to default prism rotation, use ints"] float rotation_speed = 1;
}

class Gear {
	[text] int layer = 20;
	[text] int sublayer = 20;
	[position, mode:world, layer:=layer.0, y:y] float x;
	[hidden] float y;
	[text] float scale = 1;
	[text|tooltip:"default is prism rotate speed"] float speed = -0.706;

	[hidden] float rotation = 0;

	void step() {
		rotation += speed;
		rotation %= 360;
	}
}

class script {
	scene@ g;
	camera@ cam;
	dustman@ dm;

	sprites@ props3;

	[text] array<Gear> gears(0);

	[text] int prism_gear_layer = 15;
	[text] int prism_gear_sublayer = 21;
	[text] float prism_gear_scale = 1;
	[text] array<EntityData> prisms(0);

	float hexagon_rotation;
	float square_rotation;

	script() {
		@g = get_scene();
		// g.override_stream_sizes(32, 8);

		@props3 = create_sprites();
		props3.add_sprite_set("props3");

		hexagon_rotation = 0;
		square_rotation = 0;
	}

	void on_level_start() {
		props3.add_sprite_set("props3");
		// g.override_stream_sizes(8, 8);
	}

	void entity_on_remove(entity@ e) {
		if (e.type_name() == "enemy_tutorial_square" || e.type_name() == "enemy_tutorial_hexagon") {
			bool hexagon = e.type_name() == "enemy_tutorial_hexagon";
			for (uint i = 0; i < prisms.length; i++) {
				if (e.id() == prisms[i].id) {
					prop@ p = create_prop();
					p.prop_set(3);
					p.prop_group(27);
					p.prop_index(10);
					p.palette(1);
					p.layer(prism_gear_layer);
					p.sub_layer(prism_gear_sublayer);
					p.scale_x(prism_gear_scale);
					p.scale_y(prism_gear_scale);

					p.rotation((hexagon ? hexagon_rotation : square_rotation) * prisms[i].rotation_speed * e.face());
					float rad = (hexagon ? hexagon_rotation : square_rotation) * PI / 180 * prisms[i].rotation_speed * e.face();
					float rx = (cos(rad) * GEAR_DX - sin(rad) * GEAR_DY) * prism_gear_scale;
					float ry = (sin(rad) * GEAR_DX + cos(rad) * GEAR_DY) * prism_gear_scale;
					p.x(e.x() + rx);
					p.y(e.y() + ry);

					// add prop
					g.add_prop(p);
				}
			}
		}
	}

	void step(int entities) {
		for (uint i = 0; i < gears.length; i++) {
			gears[i].step();
		}

		hexagon_rotation += HEXAGON_ROTATE_SPEED;
		hexagon_rotation %= 360;
		square_rotation += SQUARE_ROTATE_SPEED;
		square_rotation %= 360;

		for (uint p = 0; p < prisms.length; p++) {
			entity@ e = entity_by_id(prisms[p].id);
			if (@e != null) {
				// if (PRISM_ROTATE_SPEED != 1)
				bool hexagon = e.type_name() == "enemy_tutorial_hexagon";
				e.rotation(e.rotation() + (prisms[p].rotation_speed - 1) * (hexagon ? HEXAGON_ROTATE_SPEED : SQUARE_ROTATE_SPEED) * e.face());
			}
		}
	}

	void editor_step() {
		step(0);
	}

	void draw(float sf) {
		for (uint i = 0; i < gears.length; i++) {
			Gear@ gear = gears[i];
			float rad = gear.rotation * PI / 180;
			float scale = gear.scale * (gear.layer <= 5 ? 2 : 1);
			float rx = (cos(rad) * GEAR_DX - sin(rad) * GEAR_DY) * scale;
			float ry = (sin(rad) * GEAR_DX + cos(rad) * GEAR_DY) * scale;
			if (gear.layer <= 5) {
				rx *= 2 / g.layer_scale(gear.layer);
				ry *= 2 / g.layer_scale(gear.layer);
			}
			props3.draw_world(gear.layer, gear.sublayer, "sidewalk_10", 1, 1, gear.x + rx, gear.y + ry, gear.rotation, scale, scale, 0xFFFFFFFF);
		}

		for (uint i = 0; i < prisms.length; i++) {
			entity@ e = entity_by_id(prisms[i].id);
			if (@e != null) {
				bool hexagon = e.type_name() == "enemy_tutorial_hexagon";
				float rad = (hexagon ? hexagon_rotation : square_rotation) * PI / 180 * prisms[i].rotation_speed * e.face();
				float rx = (cos(rad) * GEAR_DX - sin(rad) * GEAR_DY) * prism_gear_scale;
				float ry = (sin(rad) * GEAR_DX + cos(rad) * GEAR_DY) * prism_gear_scale;
				props3.draw_world(prism_gear_layer, prism_gear_sublayer, "sidewalk_10", 1, 1, e.x() + rx, e.y() + ry, (hexagon ? hexagon_rotation : square_rotation) * prisms[i].rotation_speed * e.face(), prism_gear_scale, prism_gear_scale, 0xFFFFFFFF);
			}
		}
	}

	void editor_draw(float sf) {
		draw(sf);
	}
}
