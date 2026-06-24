const float PI = 3.1415926535897932384626433832795;
const float FRAME = 1.0 / 60.0;

const float LIGHT_DX = -310;
const float LIGHT_DY = -220;



class Light {
	[text] int layer = 20;
	[text] int sublayer = 20;
	[position, mode:world, layer:=layer.0, y:y] float x;
	[hidden] float y;
	[text] float scale = 1;
	[text|tooltip:"seconds"] float period = 4;
	[angle] float min_angle = 0;
	[angle] float max_angle = 0;

	[hidden] float rotation = 0;

	void step(float t) {
		rotation = min_angle + sin(2 * PI / period * t / 1000000) * (max_angle - min_angle);
		rotation %= 360;
	}
}

class script {
	scene@ g;
	camera@ cam;
	dustman@ dm;

	sprites@ props3;

	[text] array<Light> lights(0);

	script() {
		@g = get_scene();

		@props3 = create_sprites();
		props3.add_sprite_set("props3");
	}

	void on_level_start() {
		props3.add_sprite_set("props3");
	}

	void step(int entities) {
		float t = get_time_us();
		for (uint i = 0; i < lights.length; i++) {
			lights[i].step(t);
		}
	}

	void editor_step() {
		step(0);
	}

	void draw(float sf) {
		for (uint i = 0; i < lights.length; i++) {
			Light@ light = lights[i];
			float rad = light.rotation * PI / 180;
			float scale = light.scale * (light.layer <= 5 ? 2 : 1);
			float rx = (cos(rad) * LIGHT_DX - sin(rad) * LIGHT_DY) * scale;
			float ry = (sin(rad) * LIGHT_DX + cos(rad) * LIGHT_DY) * scale;
			if (light.layer <= 5) {
				rx *= 2 / g.layer_scale(light.layer);
				ry *= 2 / g.layer_scale(light.layer);
			}
			props3.draw_world(light.layer, light.sublayer, "backdrops_4", 1, 1, light.x + rx, light.y + ry, light.rotation, scale, scale, 0xFFFFFFFF);
		}
	}

	void editor_draw(float sf) {
		draw(sf);
	}
}
