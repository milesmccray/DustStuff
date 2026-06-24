class edge_data
{
	int x, y;
	uint8 et, eb, el, er;
}

class script
{
	scene@ g;

	[position,tiles,mode:world,layer:19,y:area_y1] int area_x1;
	[hidden] int area_y1;
	[position,tiles,mode:world,layer:19,y:area_y2] int area_x2;
	[hidden] int area_y2;

	[text] float distance_threshold = 500;

	array<edge_data> saved_edges;
	bool edges_removed = false;
	bool initialised = false;

	script()
	{
		@g = get_scene();
	}

	void checkpoint_load()
	{
		initialised = false;
		edges_removed = false;
		saved_edges.resize(0);
	}

	void init_edges()
	{
		saved_edges.resize(0);

		int tx1 = area_x1 < area_x2 ? area_x1 : area_x2;
		int ty1 = area_y1 < area_y2 ? area_y1 : area_y2;
		int tx2 = area_x1 > area_x2 ? area_x1 : area_x2;
		int ty2 = area_y1 > area_y2 ? area_y1 : area_y2;

		for(int tx = tx1; tx < tx2; tx++)
		{
			for(int ty = ty1; ty < ty2; ty++)
			{
				tileinfo@ t = g.get_tile(tx, ty, 19);
				if(!t.solid()) continue;

				if(t.edge_top() == 0 && t.edge_bottom() == 0 &&
				   t.edge_left() == 0 && t.edge_right() == 0)
					continue;

				edge_data d;
				d.x = tx;
				d.y = ty;
				d.et = t.edge_top();
				d.eb = t.edge_bottom();
				d.el = t.edge_left();
				d.er = t.edge_right();
				saved_edges.insertLast(d);
			}
		}

		initialised = true;
	}

	void step(int entities)
	{
		if(!initialised) init_edges();

		entity@ p = controller_entity(0);
		if(@p == null) return;

		float cx = (area_x1 + area_x2) * 0.5 * 48;
		float cy = (area_y1 + area_y2) * 0.5 * 48;
		float dx = p.x() - cx;
		float dy = p.y() - cy;
		float dist = sqrt(dx * dx + dy * dy);

		if(dist > distance_threshold && !edges_removed)
		{
			remove_edges();
			edges_removed = true;
		}
		else if(dist <= distance_threshold && edges_removed)
		{
			restore_edges();
			edges_removed = false;
		}
	}

	void remove_edges()
	{
		for(uint i = 0; i < saved_edges.length(); i++)
		{
			edge_data@ d = @saved_edges[i];
			tileinfo@ t = g.get_tile(d.x, d.y, 19);
			if(!t.solid()) continue;

			t.edge_top(0);
			t.edge_bottom(0);
			t.edge_left(0);
			t.edge_right(0);
			g.set_tile(d.x, d.y, 19, t, false);
		}
	}

	void restore_edges()
	{
		for(uint i = 0; i < saved_edges.length(); i++)
		{
			edge_data@ d = @saved_edges[i];
			tileinfo@ t = g.get_tile(d.x, d.y, 19);
			if(!t.solid()) continue;

			t.edge_top(d.et);
			t.edge_bottom(d.eb);
			t.edge_left(d.el);
			t.edge_right(d.er);
			g.set_tile(d.x, d.y, 19, t, false);
		}
	}

	void editor_draw(float sub_frame)
	{
		// Draw the selected area
		float x1 = 48.0 * (area_x1 < area_x2 ? area_x1 : area_x2);
		float y1 = 48.0 * (area_y1 < area_y2 ? area_y1 : area_y2);
		float x2 = 48.0 * (area_x1 > area_x2 ? area_x1 : area_x2);
		float y2 = 48.0 * (area_y1 > area_y2 ? area_y1 : area_y2);
		g.draw_rectangle_world(22, 22, x1, y1, x2, y2, 0, 0x2244AAFF);

		// Draw outline
		g.draw_line_world(22, 22, x1, y1, x2, y1, 2, 0xAA44AAFF);
		g.draw_line_world(22, 22, x2, y1, x2, y2, 2, 0xAA44AAFF);
		g.draw_line_world(22, 22, x2, y2, x1, y2, 2, 0xAA44AAFF);
		g.draw_line_world(22, 22, x1, y2, x1, y1, 2, 0xAA44AAFF);

		// Draw distance threshold circle from center of area
		float cx = (x1 + x2) * 0.5;
		float cy = (y1 + y2) * 0.5;
		draw_circle(cx, cy, distance_threshold, 0x4400FF00);
	}

	void draw_circle(float cx, float cy, float radius, uint colour)
	{
		int segments = 48;
		float step = 6.283185307 / segments;
		for(int i = 0; i < segments; i++)
		{
			float a1 = i * step;
			float a2 = (i + 1) * step;
			g.draw_line_world(22, 22,
				cx + cos(a1) * radius, cy + sin(a1) * radius,
				cx + cos(a2) * radius, cy + sin(a2) * radius,
				2, colour);
		}
	}
}
