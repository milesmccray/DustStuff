const float DT = 1.0 / 60;

class script
{
	
	[text] PropsWind props_wind;
	array<camera@> players;
	int num_players;
	
	script()
	{
		num_players = num_cameras();
		players.resize(num_players);
		
		for(int i = 0; i < num_players; i++)
		{
			@players[i] = get_camera(i);
		}
	}
	
	void checkpoint_load()
	{
		props_wind.checkpoint_load();
	}
	
	void step(int entities)
	{
		for(int i = 0; i < num_players; i++)
		{
			props_wind.player(players[i]);
		}
		
		props_wind.step();
	}
	
}

class PropsWind : callback_base
{
	
	float cell_size = 12 * 48;
	array<int> step_cells;
	array<string> step_cells_hash;
	
	dictionary cells;
	dictionary prop_ids;
	
	float t = 0;
	[text] float wind_speed = 3;
	[text] float wind_strength = 4;
	float speed_factor = 1;
	float strength_factor = 1;
	float target_speed_factor = 1;
	float target_strength_factor = 1;
	
	dictionary props = {
		// Grass
		{'1/5/12', PropSettings(1.25, 1)},
		{'1/5/13', PropSettings(1.25, 1)},
		{'1/5/14', PropSettings(1.25, 1)},
		{'2/5/4',  PropSettings(1.25, 1)},
		// "Leaves" (small foliage)
		{'2/5/1',  PropSettings(1.5, 1)},
		// Chains >>
		{'1/2/2',  PropSettings(0.35, 0.35)},
		{'1/2/1',  PropSettings(0.35, 0.35)},
		{'1/2/3',  PropSettings(0.35, 0.35)}
	};
	
	PropsWind()
	{
		add_broadcast_receiver("wind_strength", this, "on_wind_strength");
	}
	
	void on_wind_strength(string id, message@ msg)
	{
		target_speed_factor = msg.get_float("speed");
		target_strength_factor = msg.get_float("strength");
	}
	
	void player(camera@ player)
	{
		const float width  = player.screen_width();
		const float height = player.screen_height();
		const int left   = int(floor((player.x() - width * 0.5 - 50) / cell_size));
		const int right  = int(floor((player.x() + width * 0.5 + 50) / cell_size));
		const int top    = int(floor((player.y() - height * 0.5 - 50) / cell_size));
		const int bottom = int(floor((player.y() + height * 0.5 + 50) / cell_size));
		
		for(int ix = left; ix <= right; ix++)
		{
			for(int iy = top; iy <= bottom; iy++)
			{
				const string key = ix + "," + iy;
				if(step_cells_hash.find(key) < 0)
				{
					step_cells_hash.insertLast(key);
					step_cells.insertLast(ix);
					step_cells.insertLast(iy);
				}
			}
		}
	}
	
	void checkpoint_load()
	{
		step_cells.resize(0);
		step_cells_hash.resize(0);
		cells.deleteAll();
		prop_ids.deleteAll();
	}
	
	void step()
	{
		for(uint i = 0; i < step_cells.length(); i += 2)
		{
			const int x = step_cells[i];
			const int y = step_cells[i + 1];
			
			const string key = x + "," + y;
			PropsWindCell@ cell = cast<PropsWindCell>(cells[key]);
			
			if(cell is null)
			{
				@cells[key] = @cell = PropsWindCell(x, y, cell_size, @props, @prop_ids);
			}
			
			cell.step(t, wind_strength * strength_factor);
		}
		
		t += wind_speed * DT * speed_factor;
		step_cells.resize(0);
		step_cells_hash.resize(0);
		
		speed_factor += (target_speed_factor - speed_factor) * 0.025;
		strength_factor += (target_strength_factor - strength_factor) * 0.025;
	}
	
}

class PropsWindCell
{
	array<PropData@> prop_list;
	
	PropsWindCell(int x, int y, float cell_size, dictionary@ props, dictionary@ prop_ids)
	{
		scene@ g = get_scene();
		const float cx = x * cell_size;
		const float cy = y * cell_size;
		const int prop_count = g.get_prop_collision(
			cy, cy + cell_size,
			cx, cx + cell_size 
		);
		
		for(int i = 0; i < prop_count; i++)
		{
			prop@ p = g.get_prop_collision_index(i);
			const string key = p.prop_set() + "/" + p.prop_group() + "/" + p.prop_index();
			
			if(prop_ids.exists(p.id() + ''))
				continue;
			
			PropSettings@ settings = cast<PropSettings>(props[key]);
			if(settings is null) continue;
			
			prop_list.insertLast(PropData(p, settings));
			prop_ids[p.id() + ''] = true;
		}
	}
	
	void step(float t, float wind_strength)
	{
		const uint count = prop_list.length();
		
		for(uint i = 0; i < count; i++)
		{
			PropData@ data = prop_list[i];
			prop@ p = data.prop;
			const float pr = (sin(p.x() + p.y()) * 0.25 + 1) * data.settings.wind_speed;
			const float wind = sin(t * pr + (p.x() + p.y()) * 0.1f) * wind_strength * data.settings.wind_strength;
			p.rotation(data.rotation + wind);
		}
	}
	
}

class PropSettings
{
	
	float wind_strength;
	float wind_speed;
	
	PropSettings(float wind_strength=1, float wind_speed=1)
	{
		this.wind_strength = wind_strength;
		this.wind_speed = wind_speed;
	}
	
}

class PropData
{
	
	uint id;
	float rotation;
	PropSettings@ settings;
	prop@ prop = null;
	
	PropData(prop@ p, PropSettings@ settings)
	{
		@this.prop = p;
		id = p.id();
		rotation = p.rotation();
		@this.settings = settings;
	}
	
}

class Windtrigger : trigger_base
{
	
	[text] float strength = 1;
	[text] float speed = 1;
	
	Windtrigger()
	{}
	
	void init(script@ s, scripttrigger@ self)
	{
		
	}
	
	void activate(controllable@ e)
	{
		for(uint i = 0; i < num_cameras(); i++)
		{
			entity@ player = controller_entity(i);
			if(@player != null and e.is_same(player))
			{
				message@ msg = create_message();
				msg.set_float("strength", strength);
				msg.set_float("speed", speed);
				broadcast_message("wind_strength", msg);
				return;
			}
		}
	}
	
}