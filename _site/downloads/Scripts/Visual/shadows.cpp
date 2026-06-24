#include '../lib/std.cpp';
#include '../lib/math/math.cpp';
#include '../lib/tiles/get_tile_edge_points.cpp';
#include '../lib/drawing/circle.cpp';

const float ASPECT_RATIO = 1920.0 / 1080;

class script
{
	
	scene@ g;
	controllable@ player;
	
	// !! DEBUG !!
	//int debug_edges_size = 400;
	//int debug_edges_count;
	//array<float> edges_debug(debug_edges_size);
	//textfield@ debug_text;
	
	int facing_edges_size = 400;
	int facing_edges_count;
	array<float> facing_edges(facing_edges_size);
	
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
	[text] bool dont_render_outside_light = true;
	
	script()
	{
		@g = get_scene();
		
		// !! DEBUG !!
		//@debug_text = create_textfield();
		//debug_text.align_vertical(-1);
		//debug_text.align_horizontal(-1);
		//debug_text.colour(0xFFFF0000);
		//debug_text.set_font('ProximaNovaReg', 26);
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
			@player = controller_controllable(0);
			if(@player != null)
			{
				rectangle@ rect = player.base_rectangle();
				player_cx = (rect.left() + rect.right()) * 0.5;
				player_cy = (rect.top() + rect.bottom()) * 0.5;
			}
			
		if(player is null)
			return;
		
		const bool ignore_outside_light = dont_render_outside_light and light_radius > 0;
		
		const float light_x = this.light_x = player.x() + player_cx;
		const float light_y = this.light_y = player.y() + player_cy;
		
		camera@ cam = get_active_camera();
		const float cam_x = cam.x();
		const float cam_y = cam.y();
		const float screen_width = cam.screen_height() * ASPECT_RATIO / 2;
		const float screen_height = cam.screen_height() / 2;
		float left = min(cam.x(), cam.prev_x()) - screen_width;
		float right = max(cam.x(), cam.prev_x()) + screen_width;
		float top = min(cam.y(), cam.prev_y()) - screen_height;
		float bottom = max(cam.y(), cam.prev_y()) + screen_height;
		
		if(ignore_outside_light)
		{
			if(light_x - light_radius > left)
				left = light_x - light_radius;
			if(light_x + light_radius < right)
				right = light_x + light_radius;
			if(light_y - light_radius > top)
				top = light_y - light_radius;
			if(light_y + light_radius < bottom)
				bottom = light_y + light_radius;
		}
		
		const int tile_start_x = int(floor(left / 48));
		const int tile_start_y = int(floor(top / 48));
		const int tile_count_x = int(ceil(right / 48)) - tile_start_x;
		const int tile_count_y = int(ceil(bottom / 48)) - tile_start_y;
		
		float x1, y1, x2, y2;
		
		// !! DEBUG !!
		//int debug_edges_count = 0;
		//int debug_edges_size = this.debug_edges_size;
		
		int facing_edges_count = 0;
		int facing_edges_size = this.facing_edges_size;
		
		for(int tile_x = 0; tile_x <= tile_count_x; tile_x++)
		{
			for(int tile_y = 0; tile_y <= tile_count_y; tile_y++)
			{
				tileinfo@ tile_info = g.get_tile(tile_start_x + tile_x, tile_start_y + tile_y);
				if(!tile_info.solid()) continue;
				
				const float x = (tile_start_x + tile_x) * 48;
				const float y = (tile_start_y + tile_y) * 48;
				
				if(facing_edges_count + 16 > facing_edges_size)
				{
					facing_edges_size += 100;
					facing_edges.resize(facing_edges_size);
				}
				// !! DEBUG !!
				//if(debug_edges_count + 16 > debug_edges_size)
				//{
				//	debug_edges_size += 100;
				//	edges_debug.resize(debug_edges_size);
				//}
				
				const int type = tile_info.type();
				
				//
				//
				//
				
				if(tile_info.edge_left() & 0x8 != 0)
				{
					get_tile_left_edge_points(type, x1, y1, x2, y2);
					x1 += x; x2 += x;
					y1 += y; y2 += y;
					
					const float dx1 = x1 - light_x, dy1 = y1 - light_y;
					const float dx2 = -(y2 - y1), dy2 = x2 - x1;
					// Dot product to determine of this edge is facing the player
					if(dx1 * dx2 + dy1 * dy2 > 0)
					{
						facing_edges[facing_edges_count++] = x1; facing_edges[facing_edges_count++] = y1;
						facing_edges[facing_edges_count++] = x2; facing_edges[facing_edges_count++] = y2;
					}
					
					// !! DEBUG !!
					//edges_debug[debug_edges_count++] = x1; edges_debug[debug_edges_count++] = y1;
					//edges_debug[debug_edges_count++] = x2; edges_debug[debug_edges_count++] = y2;
				}
				
				if(tile_info.edge_right() & 0x8 != 0)
				{
					get_tile_right_edge_points(type, x1, y1, x2, y2);
					x1 += x; x2 += x;
					y1 += y; y2 += y;
					
					const float dx1 = x1 - light_x, dy1 = y1 - light_y;
					const float dx2 = -(y2 - y1), dy2 = x2 - x1;
					// Dot product to determine of this edge is facing the player
					if(dx1 * dx2 + dy1 * dy2 > 0)
					{
						facing_edges[facing_edges_count++] = x1; facing_edges[facing_edges_count++] = y1;
						facing_edges[facing_edges_count++] = x2; facing_edges[facing_edges_count++] = y2;
					}
					
					// !! DEBUG !!
					//edges_debug[debug_edges_count++] = x1; edges_debug[debug_edges_count++] = y1;
					//edges_debug[debug_edges_count++] = x2; edges_debug[debug_edges_count++] = y2;
				}
				
				if(tile_info.edge_top() & 0x8 != 0)
				{
					get_tile_top_edge_points(type, x1, y1, x2, y2);
					x1 += x; x2 += x;
					y1 += y; y2 += y;
					
					const float dx1 = x1 - light_x, dy1 = y1 - light_y;
					const float dx2 = -(y2 - y1), dy2 = x2 - x1;
					// Dot product to determine of this edge is facing the player
					if(dx1 * dx2 + dy1 * dy2 > 0)
					{
						facing_edges[facing_edges_count++] = x1; facing_edges[facing_edges_count++] = y1;
						facing_edges[facing_edges_count++] = x2; facing_edges[facing_edges_count++] = y2;
					}
					
					// !! DEBUG !!
					//edges_debug[debug_edges_count++] = x1; edges_debug[debug_edges_count++] = y1;
					//edges_debug[debug_edges_count++] = x2; edges_debug[debug_edges_count++] = y2;
				}
				
				if(tile_info.edge_bottom() & 0x8 != 0)
				{
					get_tile_bottom_edge_points(type, x1, y1, x2, y2);
					x1 += x; x2 += x;
					y1 += y; y2 += y;
					
					const float dx1 = x1 - light_x, dy1 = y1 - light_y;
					const float dx2 = -(y2 - y1), dy2 = x2 - x1;
					// Dot product to determine of this edge is facing the player
					if(dx1 * dx2 + dy1 * dy2 > 0)
					{
						facing_edges[facing_edges_count++] = x1; facing_edges[facing_edges_count++] = y1;
						facing_edges[facing_edges_count++] = x2; facing_edges[facing_edges_count++] = y2;
					}
					
					// !! DEBUG !!
					//edges_debug[debug_edges_count++] = x1; edges_debug[debug_edges_count++] = y1;
					//edges_debug[debug_edges_count++] = x2; edges_debug[debug_edges_count++] = y2;
				}
			}
		}
		
		this.facing_edges_count = facing_edges_count;
		this.facing_edges_size = facing_edges_size;
		
		// !! DEBUG !!
		//this.debug_edges_count = debug_edges_count;
		//debug_text.text(
		//	'Edges tested: ' + (this.debug_edges_count / 4) + '\n' +
		//	'Edges drawn : ' + (this.facing_edges_count / 4) + '\n'
		//);
	}
	
	void draw(float sub_frame)
	{
		if(!enabled)
			return;
			
		// !! DEBUG !!
		//for(int i = 0; i < debug_edges_count; i += 4)
		//{
		//	const float x1 = edges_debug[i];
		//	const float y1 = edges_debug[i + 1];
		//	const float x2 = edges_debug[i + 2];
		//	const float y2 = edges_debug[i + 3];
		//	g.draw_line(21, 19, x1, y1, x2, y2, 3, 0xFFFF0000);
		//}
		
		// !! DEBUG !!
		//for(int i = 0; i < facing_edges_count; i += 4)
		//{
		//	const float x1 = facing_edges[i];
		//	const float y1 = facing_edges[i + 1];
		//	const float x2 = facing_edges[i + 2];
		//	const float y2 = facing_edges[i + 3];
		//	g.draw_line(22, 20, x1, y1, x2, y2, 4, 0xFF0000FF);
		//}
		
		const uint shadow_colour = this.shadow_colour;
		const uint shadow_layer = this.shadow_layer;
		const uint shadow_sublayer = this.shadow_sublayer;
		
		for(int i = 0; i < facing_edges_count; i += 4)
		{
			const float x1 = facing_edges[i];
			const float y1 = facing_edges[i + 1];
			const float x2 = facing_edges[i + 2];
			const float y2 = facing_edges[i + 3];
			
			const float dx1 = (x1 - light_x);
			const float dy1 = (y1 - light_y);
			const float dx2 = (x2 - light_x);
			const float dy2 = (y2 - light_y);
			const float dist = 1 / sqrt(dx1 * dx1 + dy1 * dy1) * 3000;

			g.draw_quad_world(shadow_layer, shadow_sublayer, false,
				x1, y1, x2, y2,
				x2 + dx2 * dist, y2 + dy2 * dist,
				x1 + dx1 * dist, y1 + dy1 * dist,
				shadow_colour, shadow_colour, shadow_colour, shadow_colour);
			
			// !! DEBUG !!
			//g.draw_line(22, 20, x1, y1, x1 + dx1 * dist, y1 + dy1 * dist, 1, 0xFF0000FF);
			//g.draw_line(22, 20, x2, y2, x2 + dx2 * dist, y2 + dy2 * dist, 1, 0xFF0000FF);
		}
		
		const uint shadow_transparent_colour = shadow_colour & 0xFFFFFF;
		
		if(light_radius > 0)
		{
			drawing::fill_circle_outside(g, shadow_layer, shadow_sublayer, light_x, light_y, light_radius, light_radius_segments, shadow_colour, shadow_colour);
			drawing::fill_circle(g, shadow_layer, shadow_sublayer, light_x, light_y, light_radius, light_radius_segments, shadow_transparent_colour, shadow_colour);
		}
		
		// !! DEBUG !!
		//debug_text.draw_hud(22, 22, SCREEN_LEFT + 10, SCREEN_TOP + 10, 1, 1, 0);
	}
	
}
