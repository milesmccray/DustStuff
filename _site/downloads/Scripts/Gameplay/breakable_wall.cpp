#include '../lib/std.cpp';
#include '../lib/enums/ColType.cpp';
#include '../lib/enums/EmitterId.cpp';
#include '../lib/drawing/common.cpp';
#include '../lib/emitters.cpp';
#include '../lib/RemoveTimer.cpp';
#include '../lib/Fx.cpp';

class script { }

class BreakableWall : trigger_base
{
	
	script@ script;
	scene@ g;
	scripttrigger@ self;
	
	[text] string name = '';
	[text] string first_hit_event = '';
	[text] string chunk_destroy_event = '';
	[text] string destroy_event = '';
	
	[text] array<BreakableChunk> chunks;
	
	[hidden] bool has_been_hit = false;
	
	BreakableWall()
	{
		@g = get_scene();
	}
	
	void init(script@ s, scripttrigger@ self)
	{
		@this.script = s;
		@this.self = self;
		
		for(int i = int(chunks.size()) - 1; i >=0 ; i--)
		{
			chunks[i].init(g);
		}
	}
	
	void step()
	{
		for(int i = int(chunks.size()) - 1; i >=0 ; i--)
		{
			switch(chunks[i].step(g))
			{
				case ChunkReturnValue::Destroyed:
					trigger_event(chunk_destroy_event);
					chunks.removeAt(i);
				case ChunkReturnValue::Attacked:
					if(!has_been_hit)
					{
						trigger_event(first_hit_event);
						has_been_hit = true;
					}
					break;
			}
		}
		
		if(chunks.size() == 0)
		{
			trigger_event(destroy_event);
			g.remove_entity(self.as_entity());
		}
	}
	
	void trigger_event(const string& in message_name)
	{
		if(message_name != '')
		{
			message@ msg = create_message();
			msg.set_string('name', name);
			msg.set_entity('trigger', self.as_entity());
			broadcast_message(message_name, msg);
		}
	}
	
//	void draw(float sub_frame)
//	{
//		editor_draw(sub_frame);
//	}
	
	void editor_draw(float sub_frame)
	{
		for(int i = int(chunks.size()) - 1; i >=0 ; i--)
		{
			chunks[i].draw(g);
		}
	}
	
}

class BreakableChunk
{
	
	[text] int tile_health = 4;
	[position,mode:world,layer:19,y:y1] float x1;
	[hidden] float y1;
	[position,mode:world,layer:19,y:y2] float x2;
	[hidden] float y2;
	[text] int emitter_id = -1;
	
	[hidden] array<int> tiles_health;
	[hidden] int tile_count = 0;
	
	BreakableChunk(){};
	
	void init(scene@ g)
	{
		if(tiles_health.size() == 0)
		{
			const float tx1 = x1, tx2 = x2;
			const float ty1 = y1, ty2 = y2;
			x1 = min(tx1, tx2);
			x2 = max(tx1, tx2);
			y1 = min(ty1, ty2);
			y2 = max(ty1, ty2);
			
			const int tile_x1 = int(floor(x1 / 48));
			const int tile_y1 = int(floor(y1 / 48));
			const int tile_x2 = int(floor(x2 / 48)) + 1;
			const int tile_y2 = int(floor(y2 / 48)) + 1;
			
			tiles_health.resize( (tile_x2 - tile_x1) * (tile_y2 - tile_y1) );
			
			int block_index = 0;
			tile_count = 0;
			for(int x = tile_x1; x < tile_x2; x++)
			{
				for(int y = tile_y1; y < tile_y2; y++)
				{
					tileinfo@ tile = g.get_tile(x, y);
					if(tile.solid())
					{
						tile_count++;
						tiles_health[block_index] = tile_health;
					}
					
					block_index++;
				}
			}
		}
	}
	
	ChunkReturnValue step(scene@ g)
	{
		const int tile_x1 = int(floor(x1 / 48));
		const int tile_y1 = int(floor(y1 / 48));
		const int tile_x2 = int(floor(x2 / 48));
		const int tile_y2 = int(floor(y2 / 48));
		
		int hit_fx_framerate;
		float hit_fx_x, hit_fx_y;
		int hit_damage = 0;
		int hit_dir;
		string hit_fx;
		
		int block_index = 0;
		for(int x = tile_x1; x <= tile_x2; x++)
		{
			for(int y = tile_y1; y <= tile_y2; y++)
			{
				if(tiles_health[block_index] > 0)
				{
					int count = g.get_entity_collision(
						y * 48, y * 48 + 48,
						x * 48, x * 48 + 48,
						ColType::Hitbox
					);
					
					for(int i = 0; i < count; i++)
					{
						hitbox@ hit = g.get_entity_collision_index(i).as_hitbox();
						if(hit is null) continue;
						
						if(hit.triggered() && hit.state_timer() == hit.activate_time())
						{
							tiles_health[block_index] -= hit.damage();
							
							if(hit.damage() > hit_damage)
							{
								hit_damage = hit.damage();
								hit_fx = hit.attack_effect();
								hit_fx_framerate = hit.effect_frame_rate();
								hit_dir = hit.attack_dir();
								rectangle@ r = hit.base_rectangle();
								hit_fx_x = hit.x() + r.left() + r.get_width() * 0.5;
								hit_fx_y = hit.y() + r.top() + r.get_height() * 0.5;
							}
							
							if(tiles_health[block_index] <= 0)
							{
								tileinfo@ tile = g.get_tile(x, y);
								tile.solid(false);
								g.set_tile(x, y, 19, tile, true);
								
								if(emitter_id != 0)
								{
									entity@ emitter = create_emitter(
										emitter_id == -1 ? get_emitter_id_for_area(tile.sprite_set()) : emitter_id,
										x * 48 + 24, y * 48 + 24,
										48, 48, 19, 12);
									g.add_entity(emitter);
									remove_timer(emitter, 1);
								}
								
								tile_count--;
								break;
							}
						}
					}
				}
				
				block_index++;
			}
		}
		
		if(hit_damage > 0)
		{
			const string impact = hit_damage < 3 ? 'light' : 'heavy';
			g.play_sound('sfx_impact_' + impact + '_' + rand_range(1, 3), hit_fx_x, hit_fx_y, 1, false, true);
			create_fx('effects', hit_fx,
				hit_fx_x, hit_fx_y, hit_dir - 90.0f,
				1.0f, 1.0f, hit_fx_framerate);
			
			return tile_count <= 0
				? ChunkReturnValue::Destroyed
				: ChunkReturnValue::Attacked;
		}
		
		return ChunkReturnValue::None;
	}
	
	// Small wrapper for creating an fx because scene::add_effect doesn't work right now
	void create_fx(
		string sprite_set, string sprite_name,
		float x, float y, float rotation,
		float scale_x, float scale_y, float frame_rate)
	{
//			g.add_effect(sprite_set, sprite_name,
//				x, y, rotation,
//				scale_x, scale_y, frame_rate);
			spawn_fx(x, y, sprite_set, sprite_name, 0, frame_rate, rotation, scale_x, scale_y);
	}
	
	void draw(scene@ g)
	{
		const float tx1 = floor(x1 / 48) * 48;
		const float ty1 = floor(y1 / 48) * 48;
		const float tx2 = floor(x2 / 48) * 48 + 48;
		const float ty2 = floor(y2 / 48) * 48 + 48;
		
		g.draw_rectangle_world(22, 22,
			tx1, ty1, tx2, ty2,
			0, 0x44FF0000);
		outline_rect(g, 22, 22,
			tx1, ty1, tx2, ty2,
			1, 0x88FF0000);
	}
	
}

enum ChunkReturnValue
{
	
	None,
	Attacked,
	Destroyed,
	
}