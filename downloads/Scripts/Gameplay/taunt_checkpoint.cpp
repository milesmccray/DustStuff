#include '../lib/std.cpp';
#include '../lib/enums/EmitterId.cpp';
#include '../lib/emitters/common.cpp';

class script
{
	
	[hidden] array<CPData> cp;
	
	scene@ g;
	int num_players;
	array<controllable@> players;
	array<dustman@> dms;
	
	script()
	{
		@g = get_scene();
		num_players = num_cameras();
		players.resize(num_players);
		dms.resize(num_players);
		cp.resize(num_players);
	}
	
	void on_level_start()
	{
		init_players();
	}
	
	void checkpoint_load()
	{
		init_players();
	}
	
	private void init_players()
	{
		for(int i = 0; i < num_players; i++)
		{
			controllable@ c = controller_controllable(i);
			dustman@ dm = c.as_dustman();
			@players[i] = c;
			@dms[i] = dm;
			
			CPData@ d = @cp[i];
			c.set_speed_xy(d.vel_x, d.vel_y);
			
			if(d.facing != 0)
			{
				c.face(d.facing);
			}
			
			if(@dm != null && d.charges > -1)
			{
				dm.dash(d.charges);
			}
		}
	}
	
	void step(int)
	{
		for(int i = 0; i < num_players; i++)
		{
			controllable@ c = players[i];
			
			if(c.taunt_intent() == 1)
			{
				for(int j = 0; j < num_players; j++)
				{
					dustman@ dm = dms[j];
					CPData@ d = @cp[j];
					d.vel_x = c.x_speed();
					d.vel_y = c.y_speed();
					d.facing = c.face();
					d.charges = @dm != null ? int(max(dm.dash(), 0)) : 0;
				}
				
				g.save_checkpoint(0, 0, false);
				
				const float x = c.x();
				const float y = c.y();
				rectangle@ r = c.collision_rect();
				const float cx = x + r.left() + r.width * 0.5;
				const float cy = y + r.top() + r.height * 0.5;
				g.play_sound('sfx_hill_beep', cx, cy, 1, false, true);
				
				entity@ em = create_emitter(EmitterId::DustGroundCreate, cx, cy, 48, 48, 18, 7, 0);
				g.add_entity(em, false);
				
				break;
			}
		}
	}
	
}

class CPData
{
	
	[persist] float vel_x;
	[persist] float vel_y;
	[persist] int charges = -1;
	[persist] int facing;
	
}
