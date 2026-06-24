#include '../lib/std.cpp';
#include '../lib/math/math.cpp';
#include '../lib/drawing/common.cpp';
#include '../lib/drawing/Sprite.cpp';
#include '../lib/enums/ColType.cpp';

enum Type { Any, Light, Heavy }

class script : SSScript { }

class SSScript
{
	
	[hidden] int instruction_count;
	
	[position,mode:world,layer:19,y:dust_y] 
	float dust_x; [hidden] float dust_y;
	[text] int layer = 17;
	[text] int any_sublayer = 16;
	[text] int light_sublayer = 17;
	[text] int heavy_sublayer = 18;
	[text] int dusted_sublayer = 19;
	[text] float any_scale = 1;
	[text] float light_scale = 1;
	[text] float heavy_scale = 1;
	
	bool in_game;
	scene@ g;
	
	SSScript()
	{
		@g = get_scene();
	}
	
	void on_level_start()
	{
		in_game = true;
		instruction_count = 0;
	}
	
	void on_level_end()
	{
		puts('instructions left: ' + instruction_count);
		
		if(instruction_count > 0)
		{
			update_tile();
		}
	}
	
	void update_tile()
	{
		int x = int(dust_x / 48);
		int y = int(dust_y / 48);
		tileinfo@ t = g.get_tile(x, y);
		t.set_dustblock(1);
		t.solid(true);
		g.set_tile(x, y, 19, t, true);
	}
	
	void editor_draw(float sub_frame)
	{
		draw_dot(g, 22, 22, dust_x, dust_y, 8, 0xaaff4444, 45);
	}
	
}

class AttackBox : trigger_base
{
	
	[hidden] float x1 = -100;
	[hidden] float y1 = -100;
	[hidden] float x2 =  100;
	[hidden] float y2 =  100;
	[text] int hits = 1;
	[option,0:Any,1:Light,2:Heavy] Type type = Type::Light;
	[angle] float dir;
	
	SSScript@ script;
	scripttrigger@ self;
	
	scene@ g;
	Sprite spr('props5', 'symbol_1', 0.5, 0.52);
	
	void init(script@ s, scripttrigger@ self)
	{
		@this.script = s;
		@this.self = self;
		
		@g = get_scene();
		
		if(script.in_game)
		{
			script.instruction_count += max(hits, 0);
		}
	}
	
	private float angle(const float angle)
	{
		return mod(round(angle / 45.0) * 45, 360.0);
	}
	
	void step()
	{
		if(hits <= 0)
			return;
		
		int i = g.get_entity_collision(self.y() - 10, self.y() + 10, self.x() - 10, self.x() + 10, ColType::Hitbox);
		
		while(i-- > 0)
		{
			hitbox@ hb = g.get_entity_collision_index(i).as_hitbox();
			
			if(!hb.triggered() || hb.metadata().get_int('used') == 1)
				return;
			
			if(type != 0 && hb.damage() != (type == Heavy ? 3 : 1))
				return;
			
			const float dir = angle(this.dir);
			float attack_dir = angle(hb.attack_dir());
			
			if(dir == 0)
			{
				if(attack_dir == 45 || attack_dir == 315)
					attack_dir = dir;
			}
			else if(dir == 180)
			{
				if(attack_dir == 225 || attack_dir == 135)
					attack_dir = dir;
			}
			
			if(attack_dir != dir)
				return;
			
			hits--;
			script.instruction_count--;
			
			controllable@ c  = hb.owner();
			dustman@ dm = @c != null ? c.as_dustman() : null;
			hb.metadata().set_int('used', 1);
			
			if(@dm == null)
				return;
			
			dm.combo_count(dm.combo_count() + 1);
			dm.skill_combo(dm.skill_combo() + 1);
			dm.combo_timer(1);
			dm.dash(dm.dash_max());
		}
	}
	
	void editor_step()
	{
//		if(!self.editor_selected())
//			return
//		
//		const float x = self.x();
//		const float y = self.y();
//		
//		if(g.mouse_state(0) & 4 != 0 && get_editor_api().key_check_gvb(10))
//		{
//			
//		}
		
//		script.ed_rel(x, y);
//		
//		if(script.ed_box(x1, y1, x2, y2, this, 0, false, -1, 19, 0xff4455aa) == Move)
//		{
//			script.ed_update_box(x1, y1, x2, y2);
//		}
//		
//		script.ed_abs();
	}
	
	void draw(float sub_frame)
	{
		const float scale = type == Any ? 1.0 : type == Light ? script.light_scale : script.heavy_scale;
		
		spr.draw(script.layer, hits <= 0 ? script.dusted_sublayer : (type == Light ? script.light_sublayer : script.heavy_sublayer), 0, 0, self.x(), self.y(),
			angle(dir - 90),
			scale, scale,
			0xffffffff);
	}
	
	void editor_draw(float sub_frame)
	{
		draw(sub_frame);
	}
	
}