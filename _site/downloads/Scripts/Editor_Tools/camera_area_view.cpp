#include '../lib/enums/ColType.cpp';

class script
{
	
	[persist] int layer = 21;
	[persist] int sublayer = 22;
	[colour,alpha] uint fill_clr = 0x33000000;
	
	scene@ g;
	camera@ cam;
	
	script()
	{
		@g = scene;
		@cam = active_camera;
	}
	
	void editor_draw(float sub_frame)
	{
		dictionary cams;
		
		const float zoom = 1 / cam.editor_zoom();
		float left, top, width, height;
		cam.get_layer_draw_rect(sub_frame, 19, left, top, width, height);
		const float padding = 960;
		const float view_x1 = left - padding;
		const float view_y1 = top - padding;
		const float view_x2 = view_x1 + width + padding;
		const float view_y2 = view_y1 + height + padding;
		
		int i = g.get_entity_collision(view_y1, view_y2, view_x1, view_x2, ColType::CameraNode);
		while(--i >= 0)
		{
			camera_node@ node1 = g.get_entity_collision_index(i).as_camera_node();
			const string id1 = node1.id();
			
			const float px1 = node1.x();
			const float py1 = node1.y();
			const float h1 = node1.screen_height();
			const float w1 = h1 * 1920 / 1080;
			const float node1_x1 = px1 - w1 * 0.5;
			const float node1_y1 = py1 - h1 * 0.5;
			const float node1_x2 = px1 + w1 * 0.5;
			const float node1_y2 = py1 + h1 * 0.5;
			
			bool fill_node1 = !cams.exists(id1);
			if(node1.num_edges() == 0)
				continue;
			
			for(int j = node1.num_edges() - 1; j >= 0; j--)
			{
				entity@ e = entity_by_id(node1.connected_node_id(j));
				camera_node@ node2 = @e != null ? e.as_camera_node() : null;
				if(@node2 == null)
					continue;
				const string id2 = node2.id();
				if(cams.exists(id1 + id2) || cams.exists(id2 + id1))
					continue;
				
				const float px2 = node2.x();
				const float py2 = node2.y();
				const float h2 = node2.screen_height();
				const float w2 = h2* 1920 / 1080;
				const float node2_x1 = px2 - w2 * 0.5;
				const float node2_y1 = py2 - h2 * 0.5;
				const float node2_x2 = px2 + w2 * 0.5;
				const float node2_y2 = py2 + h2 * 0.5;
				g.draw_line_world(layer, sublayer, px1, py1, px2, py2, 2 * zoom, fill_clr);
				
				// Node2 fully contained inside node1;
				if(node2_x1 >= node1_x1 && node2_x2 <= node1_x2 && node2_y1 >= node1_y1 && node2_y2 <= node1_y2)
					continue;
				// Node1 fully contained inside node2;
				if(node1_x1 >= node2_x1 && node1_x2 <= node2_x2 && node1_y1 >= node2_y1 && node1_y2 <= node2_y2)
				{
					if(!cams.exists(id2))
					{
						g.draw_rectangle_world(layer, sublayer, node2_x1, node2_y1, node2_x2, node2_y2, 0, fill_clr);
					}
					fill_node1 = false;
					continue;
				}
				
				// Right
				if(node2_x2 > node1_x2)
				{
					g.draw_quad_world(layer, sublayer, false,
						node1_x2, node1_y1,
						node1_x2, node1_y2,
						node2_x2, node2_y2,
						node2_x2, node2_y1,
						fill_clr, fill_clr, fill_clr, fill_clr);
				}
				// Left
				if(node2_x1 < node1_x1)
				{
					g.draw_quad_world(layer, sublayer, false,
						node1_x1, node1_y1,
						node1_x1, node1_y2,
						node2_x1, node2_y2,
						node2_x1, node2_y1,
						fill_clr, fill_clr, fill_clr, fill_clr);
				}
				// Bottom
				if(node2_y2 > node1_y2)
				{
					g.draw_quad_world(layer, sublayer, false,
						node1_x1, node1_y2,
						node1_x2, node1_y2,
						node2_x2, node2_y2,
						node2_x1, node2_y2,
						fill_clr, fill_clr, fill_clr, fill_clr);
				}
				// Top
				if(node2_y1 < node1_y1)
				{
					g.draw_quad_world(layer, sublayer, false,
						node1_x1, node1_y1,
						node1_x2, node1_y1,
						node2_x2, node2_y1,
						node2_x1, node2_y1,
						fill_clr, fill_clr, fill_clr, fill_clr);
				}
				
				cams[id1 + id2] = true;
				cams[id2 + id1] = true;
				cams[id2] = true;
			}
			
			if(fill_node1)
			{
				g.draw_rectangle_world(layer, sublayer, node1_x1, node1_y1, node1_x2, node1_y2, 0, fill_clr);
			}
			
			cams[id1] = true;
		}
	}
	
}
