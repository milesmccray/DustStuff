#include '../lib/std.cpp';
#include '../lib/emitters.cpp';
#include '../lib/enums/EmitterId.cpp';
#include '../lib/drawing/Sprite.cpp';

class script
{
	
	[text] int emitter_count = 4;
	
	scene@ g;
	array<controllable@> players;
	int num_players;
	bool init = true;
	
	Sprite sprite('props3', 'backdrops_3');
	array<array<entity@>> player_emitters;
	
	script()
	{
		@g = get_scene();
		
		num_players = num_cameras();
		players.resize(num_players);
		player_emitters.resize(num_players);
		
		emitter_count = clamp(int(abs(emitter_count)), 0, 100);
		
		for(int i = 0; i < num_players; i++)
		{
			player_emitters[i].resize(emitter_count);
		}
	}
	
	void on_level_start()
	{
	}
	
	void checkpoint_load()
	{
		init = true;
		
		for(int i = 0; i < num_players; i++)
		{
			array<entity@>@ emitters = @player_emitters[i];
			
			for(int j = 0; j < emitter_count; j++)
			{
				g.remove_entity(emitters[j]);
			}
		}
	}
	
	void step(int entities)
	{
		if(init)
		{
			int emitter_index = 0;
			
			for(int i = 0; i < num_players; i++)
			{
				controllable@ player = @players[i] = controller_controllable(i);
				array<entity@>@ emitters = @player_emitters[i];
				
				const float px = player.x();
				const float py = player.y() - 48;
				
				for(int j = 0; j < emitter_count; j++)
				{
					@emitters[j] = create_emitter(EmitterId::KingZoneRed, px, py, 40, 80, 18, 9);
					g.add_entity(emitters[j], false);
				}
			}
			
			init = false;
		}
		
		for(int i = 0; i < num_players; i++)
		{
			controllable@ player = @players[i];
			array<entity@>@ emitters = @player_emitters[i];
			
			const float px = player.x();
			const float py = player.y() - 48;
			
			for(int j = 0; j < emitter_count; j++)
			{
				emitters[j].set_xy(px, py);
			}
		}
	}
	
	void draw(float sub_frame)
	{
		if(init)
			return;
		
		for(int i = 0; i < num_players; i++)
		{
			controllable@ player = @players[i];
			
			const float x = lerp(player.prev_x(), player.x(), sub_frame);
			const float y = lerp(player.prev_y(), player.y(), sub_frame);
			
			sprite.draw(18, 9, 0, 0, x, y - 48, player.rotation() + 90, 0.15, 0.25, 0x38ffffff);
		}
	}
	
}