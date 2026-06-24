#include '../lib/std.cpp';
#include '../lib/math/math.cpp';
#include '../lib/tiles/get_tile_edge_points.cpp';
#include '../lib/drawing/circle.cpp';

const float ASPECT_RATIO = 1920.0 / 1080;

class script
{
	scene@ g;
	controllable@ player;

	float player_cx;
	float player_cy;

	float light_x;
	float light_y;

	[text] bool enabled = true;
	[colour, alpha] uint shadow_colour = 0xFF000000;
	[text] uint shadow_layer = 20;
	[text] uint shadow_sublayer = 24;
	[text] float light_radius = 600;
	[text] int light_radius_segments = 8;

	script()
	{
		@g = get_scene();
	}

	void checkpoint_load()
	{
		@player = null;
	}

	void step(int entities)
	{
		if(!enabled)
			return;

		if(@player == null)
		{
			@player = controller_controllable(0);

			if(@player != null)
			{
				rectangle@ rect = player.base_rectangle();

				player_cx = (rect.left() + rect.right()) * 0.5;
				player_cy = (rect.top() + rect.bottom()) * 0.5;
			}
		}

		if(@player == null)
			return;

		light_x = player.x() + player_cx;
		light_y = player.y() + player_cy;
	}

	void draw(float sub_frame)
	{
		if(!enabled)
			return;

		const uint shadow_transparent_colour = shadow_colour & 0x00FFFFFF;

		if(light_radius > 0)
		{
			// Darkness outside the light radius
			drawing::fill_circle_outside(
				g,
				shadow_layer,
				shadow_sublayer,
				light_x,
				light_y,
				light_radius,
				light_radius_segments,
				shadow_colour,
				shadow_colour
			);

			// Fade from transparent center to dark edge
			drawing::fill_circle(
				g,
				shadow_layer,
				shadow_sublayer,
				light_x,
				light_y,
				light_radius,
				light_radius_segments,
				shadow_transparent_colour,
				shadow_colour
			);
		}
	}
}