#include '../lib/std.cpp';
#include '../lib/math/math.cpp';
#include '../lib/string/parse_range_list.cpp';
#include '../lib/tiles/common.cpp';
#include '../lib/tiles/TileEdge.cpp';
#include '../lib/tiles/TileShape.cpp';

class script
{
	
	scene@ g;
	bool in_game = false;
	
	controllable@ player = null;
	camera@ cam;
	
	script()
	{
		@g = get_scene();
		@cam = get_camera(0);
	}
	
	void on_level_start()
	{
		in_game = true;
	}
	
}

[hidden]
class TileBaseTrigger : trigger_base
{
	
	[persist] bool run = false;
	[persist] bool run_continuous = false;
	[persist] int layer = 19;
	
	[hidden] float prev_x = 0;
	[hidden] float prev_y = 0;
	
	protected scene@ g;
	protected scripttrigger @self;
	
	protected bool run_tile = true;
	protected bool run_filth = false;
	
	protected uint trigger_colour = 0xff6e007f;
	protected uint trigger_colour_active = 0xffcc00ff;
	
	protected array<int> parse_results;
	
	TileBaseTrigger()
	{
		@g = get_scene();
	}
	
	void init(script@ s, scripttrigger@ self)
	{
		@this.self = self;
		self.square(true);
		
		self.editor_colour_circle(trigger_colour);
		self.editor_colour_inactive(trigger_colour);
		self.editor_colour_active(trigger_colour_active);
		
		prev_y = self.y();
		prev_x = self.x();
		
		on_init();
	}
	
	void editor_var_changed(var_info@ info)
	{
		string path = info.get_path(true);
		
		if(path == 'run')
		{
			_run();
		}
		else if(path == 'run_continuous')
		{
			if(run_continuous)
			{
				prev_y = self.y();
				prev_x = self.x();
				_run();
			}
		}
		else
		{
			on_var_changed(path);
		}
	}
	
	protected void on_init() {}
	
	protected void on_var_changed(const string &in path) {}
	
	protected void before_run() {}
	
	void _run()
	{
		before_run();
		
		const float sx = self.x();
		const float sy = self.y();
		const float w = self.radius();
		const float h = self.height() != -1 ? float(self.height()) : w;
		const int x1 = int(floor((sx - w) / 48));
		const int y1 = int(floor((sy - w) / 48));
		const int x2 = int(floor((sx + h) / 48));
		const int y2 = int(floor((sy + h) / 48));
		
		for(int x = x1; x <= x2; x++)
		{
			for(int y = y1; y <= y2; y++)
			{
				tileinfo@ tile;
				
				if(run_tile)
				{
					@tile = g.get_tile(x, y, layer);
					this.update_tile(x, y, tile);
				}
				
				if(run_filth)
				{
					if(@tile == null || layer != 19)
					{
						@tile = g.get_tile(x, y, 19);
					}
					
					tilefilth@ filth = g.get_tile_filth(x, y);
					this.update_filth(x, y, tile, filth);
				}
			}
		}
	}
	
	void update_tile(int x, int y, tileinfo@ tile)
	{
		
	}
	
	void update_filth(int x, int y, tileinfo@ tile, tilefilth@ filth)
	{
		
	}
	
	protected bool check_filter(tileinfo@ tile, const int filter_set, int filter_tile, const int filter_palette)
	{
		return
			(filter_set == -1 || filter_set == int(tile.sprite_set())) &&
			(filter_tile == -1 || filter_tile == int(tile.sprite_tile())) &&
			(filter_palette == -1 || filter_palette == int(tile.sprite_palette()));
	}
	
	protected bool check_filter(tileinfo@ tile, array<TileFilter>@ filters)
	{
		const int sprite_set = int(tile.sprite_set());
		const int sprite_tile = int(tile.sprite_tile());
		const int sprite_palette = int(tile.sprite_palette());
		
		for(uint i = 0; i < filters.length; i++)
		{
			TileFilter@ filter = @filters[i];
			
			if(check_filter(tile, filter.sprite_set, filter.sprite_tile, filter.sprite_palette))
				return true;
		}
		
		return false;
	}
	
	void editor_step()
	{
		if(run_continuous && (prev_x != self.x() || prev_y != self.y()))
		{
			prev_y = self.y();
			prev_x = self.x();
			_run();
		}
	}
	
	void activate(controllable @e) {}
	
	protected void parse_range(const string &in range_str, array<int>@ result_layers, int min_val=-0x7FFFFFFF, int max_val=0x7FFFFFFF)
	{
		parse_results.resize(0);
		string::parse_range_list(range_str, @parse_results);
		
		result_layers.resize(0);
		for(uint i = 0; i < parse_results.length; i += 3)
		{
			const int flags = parse_results[i];
			const int start_index = max(parse_results[i + 1], min_val);
			const int end_index = min(parse_results[i + 2], max_val);
			for(int j = start_index; j <= end_index; j++)
			{
				const int existing_index = result_layers.find(j);
				
				// Exclusion.
				if(flags & 1 != 0)
				{
					result_layers.removeAt(existing_index);
				}
				else if(existing_index == -1)
				{
					result_layers.insertLast(j);
				}
			}
		}
	}
	
}

class CopyTileEdges: TileBaseTrigger
{
	
	[persist] bool ignore_dustblocks = false;
	[persist] string target_layer = '17';
	[persist] int sprite_set = 2;
	[persist] int sprite_tile = 8;
	[persist] int sprite_palette = 1;
	
	[persist] array<TileFilter> filter = { TileFilter() };
	
	private array<int> target_layers;
	private string parsed_target_layer;
	
	protected void before_run() override
	{
		if(parsed_target_layer == target_layer)
			return;
		
		parsed_target_layer = target_layer;
		parse_range(target_layer, @target_layers, 6, 20);
	}
	
	void update_tile(int x, int y, tileinfo@ tile)
	{
		if(!tile.solid())
			return;
		
		if(ignore_dustblocks && tile.is_dustblock())
			return;
		
		if(!check_filter(tile, filter))
			return;
		
		if(tile.edge_top() & 8 == 0 && tile.edge_bottom() & 8 == 0 && tile.edge_left() & 8 == 0 && tile.edge_right() & 8 == 0)
			return;
		
		tile.sprite_set(sprite_set);
		tile.sprite_tile(sprite_tile);
		tile.sprite_palette(sprite_palette);
		
		for(uint i = 0; i < target_layers.length; i++)
		{
			g.set_tile(x, y, target_layers[i], tile, false);
		}
	}
	
}

class MoveTiles: TileBaseTrigger
{
	
	[persist] bool copy = false;
	[persist] bool ignore_dustblocks = false;
	[persist] string target_layer = '20';
	[persist] array<TileFilter> filter = { TileFilter() };
	
	private array<int> target_layers;
	private string parsed_target_layer;
	
	protected void before_run() override
	{
		if(parsed_target_layer == target_layer)
			return;
		
		parsed_target_layer = target_layer;
		parse_range(target_layer, @target_layers, 6, 20);
	}
	
	void update_tile(int x, int y, tileinfo@ tile)
	{
		if(!tile.solid())
			return;
		
		if(ignore_dustblocks && tile.is_dustblock())
			return;
		
		if(!check_filter(tile, filter))
			return;
		
		for(uint i = 0; i < target_layers.length; i++)
		{
			g.set_tile(x, y, target_layers[i], tile, false);
		}
		
		if(!copy)
		{
			tile.solid(false);
			g.set_tile(x, y, layer, tile, false);
		}
	}
	
}

class SetTileSprites: TileBaseTrigger
{
	
	[persist] bool ignore_dustblocks = false;
	
	[persist] int target_set = -1;
	[persist] int target_tile = -1;
	[persist] int target_palette = -1;
	
	[persist] array<TileFilter> filter = { TileFilter() };
	
	void update_tile(int x, int y, tileinfo@ tile)
	{
		if(!tile.solid())
			return;
			
		if(ignore_dustblocks && tile.is_dustblock())
			return;
		
		if(!check_filter(tile, filter))
			return;
		
		if(target_set != -1)
			tile.sprite_set(target_set);
			
		if(target_tile != -1)
			tile.sprite_tile(target_tile);
			
		if(target_palette != -1)
			tile.sprite_palette(target_palette);
		
		g.set_tile(x, y, layer, tile, false);
	}
	
}

class MakeTilesInvisible: TileBaseTrigger
{
	
	void update_tile(int x, int y, tileinfo@ tile)
	{
		if(!tile.solid())
			return;
		
		tile.sprite_set(0);
		tile.sprite_tile(1);
		tile.sprite_palette(0);
		
		g.set_tile(x, y, layer, tile, false);
	}
	
}

class SetFilth: TileBaseTrigger
{
	
	[persist] bool dust = true;
	[persist] bool leaves;
	[persist] bool trash;
	[persist] bool slime;
	[persist] bool polygons;
	[persist] bool spikes;
	[persist] bool thorns;
	[persist] bool cones;
	[persist] bool wires;
	[persist] bool virtual_spikes;
	[persist] bool toggle_all;
	[persist] bool toggle_filth;
	[persist] bool toggle_spikes;
	[option,0:Clear,1:Dust,2:Leaves,3:Trash,4:Slime,5:Polygons,9:Spikes,10:Thorns,11:Cones,12:Wires,13:Virtual Spikes] int new_type;
	
	private uint AllFilth = 62;
	private uint AllSpikes = 15872;
	private uint type_mask;
	
	SetFilth()
	{
		super();
		
		run_tile = false;
		run_filth = true;
	}
	
	void init(script@ s, scripttrigger@ self)
	{
		TileBaseTrigger::init(s, self);
		update_type_mask();
	}
	
	void editor_var_changed(var_info@ info)
	{
		const string name = info.get_name(0);
		
		if(name == 'toggle_all')
		{
			const bool toggle = type_mask == 0;
			dust = toggle;
			leaves = toggle;
			trash = toggle;
			slime = toggle;
			polygons = toggle;
			spikes = toggle;
			thorns = toggle;
			cones = toggle;
			wires = toggle;
			virtual_spikes = toggle;
			self.editor_sync_vars_menu();
		}
		else if(name == 'toggle_filth')
		{
			const bool toggle = (type_mask & AllFilth) == 0;
			dust = toggle;
			leaves = toggle;
			trash = toggle;
			slime = toggle;
			polygons = toggle;
			self.editor_sync_vars_menu();
		}
		else if(name == 'toggle_spikes')
		{
			const bool toggle = (type_mask & AllSpikes) == 0;
			spikes = toggle;
			thorns = toggle;
			cones = toggle;
			wires = toggle;
			virtual_spikes = toggle;
			self.editor_sync_vars_menu();
		}
		
		update_type_mask();
	}
	
	private void update_type_mask()
	{
		type_mask = 0;
		type_mask |= dust ? 1 << 1 : 0;
		type_mask |= leaves ? 1 << 2 : 0;
		type_mask |= trash ? 1 << 3 : 0;
		type_mask |= slime ? 1 << 4 : 0;
		type_mask |= polygons ? 1 << 5 : 0;
		type_mask |= spikes ? 1 << 9 : 0;
		type_mask |= thorns ? 1 << 10 : 0;
		type_mask |= cones ? 1 << 11 : 0;
		type_mask |= wires ? 1 << 12 : 0;
		type_mask |= virtual_spikes ? 1 << 13 : 0;
	}
	
	void update_filth(int x, int y, tileinfo@ tile, tilefilth@ filth)
	{
		if(!tile.solid())
			return;
		
		uint8 top = filth.top();
		uint8 bottom = filth.bottom();
		uint8 left = filth.left();
		uint8 right = filth.right();
		
		bool requires_update = false;
		if(update_edge(top, top))
		{
			filth.top(top);
			requires_update = true;
		}
		if(update_edge(bottom, bottom))
		{
			filth.bottom(bottom);
			requires_update = true;
		}
		if(update_edge(left, left))
		{
			filth.left(left);
			requires_update = true;
		}
		if(update_edge(right, right))
		{
			filth.right(right);
			requires_update = true;
		}
		
		if(requires_update)
		{
			g.set_tile_filth(x, y, filth);
		}
	}
	
	private bool update_edge(uint8 filth, uint8 &out result)
	{
		if(filth != 0 && (1 << filth) & type_mask != 0)
		{
			result = new_type;
			return true;
		}
		
		result = filth;
		return false;
	}
	
}

class RemoveHiddenTiles: TileBaseTrigger
{
	
	[persist] bool ignore_transparent_tiles = true;
	
	RemoveHiddenTiles()
	{
		TileBaseTrigger();
		trigger_colour = 0xffff3344;
		trigger_colour_active = 0xffff55bb;
	}
	
	void update_tile(int x, int y, tileinfo@ tile)
	{
		if(!tile.solid())
			return;
		
		for(int i = layer + 1; i <= 20; i++)
		{
			tileinfo@ t= g.get_tile(x, y, i);
			if(!t.solid() || t.type() != TileShape::Full || t.is_dustblock())
				continue;
			
			if(ignore_transparent_tiles)
			{
				const uint8 set = t.sprite_set();
				const uint8 sprite = t.sprite_tile();
				if(
					set == 0 && (sprite > 1) ||
					set == 1 && (sprite >= 18 && sprite <= 20) ||
					set == 2 && (sprite == 4 || sprite == 5 || sprite >= 10 && sprite <= 12) ||
					set == 3 && (sprite == 3) ||
					set == 4 && (sprite == 8) ||
					set == 5 && (sprite == 1)
				)
					continue;
			}
			
			tile.solid(false);
			g.set_tile(x, y, layer, tile, false);
			break;
		}
	}
	
}

class TileFilter
{
	
	[persist] int sprite_set = -1;
	[persist] int sprite_tile = -1;
	[persist] int sprite_palette = -1;
	
}
