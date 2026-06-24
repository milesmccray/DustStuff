#include '../lib/drawing/SpriteGroup.cpp';

class script
{
	
	script()
	{
		
	}
	
}

class Clock : trigger_base
{
	
	scripttrigger@ self;
	
	SpriteGroup hour_hand(
		array<string>={'props3','lighting_1','props3','lighting_1','props3','lighting_1','props3','lighting_1',},
		array<int>={8,23,8,23,8,23,8,24,},
		array<float>={0.5,0.5,70.237,-43.5229,59.9963,0.442637,0.442637,0.5,0.5,68.237,-43.5229,59.9963,0.442637,0.442637,0.5,0.5,69.237,-41.5229,59.9963,0.442637,0.442637,0.5,0.5,69.237,-42.5229,59.9963,0.442637,0.442637,},
		array<uint>={0,0,0xFFFFFFFF,0,0,0xFFFFFFFF,0,0,0xFFFFFFFF,0,0,0xFFFFFFFF,});
	SpriteGroup minute_hand(
		array<string>={'props3','lighting_1','props3','lighting_1','props3','lighting_1','props3','sidewalk_10','props3','sidewalk_10','props3','sidewalk_10','props3','sidewalk_10','props3','lighting_1','props3','lighting_1','props3','lighting_1','props3','sidewalk_10','props3','sidewalk_10','props3','lighting_1','props3','sidewalk_10','props3','sidewalk_10','props3','sidewalk_10',},
		array<int>={8,23,8,23,8,23,8,23,8,23,8,24,8,23,8,23,8,23,8,23,8,23,8,23,8,24,8,24,8,23,8,23,},
		array<float>={0.5,0.5,52.3159,-93.1204,29.9982,0.613238,0.613238,0.5,0.5,52.3159,-95.1204,29.9982,0.613238,0.613238,0.5,0.5,50.3159,-95.1204,29.9982,0.613238,0.613238,0.5,0.5,-2.91035,-3.29648,336.083,0.442637,0.442637,0.5,0.5,-1.91035,-1.29648,336.083,0.442637,0.442637,0.5,0.5,-0.910355,-2.29648,336.083,0.442637,0.442637,0.5,0.5,-0.910355,-4.29648,336.083,0.442637,0.442637,0.5,0.5,52.3159,-94.1204,29.9982,0.613238,0.613238,0.5,0.5,52.3159,-95.1204,29.9982,0.613238,0.613238,0.5,0.5,50.3159,-95.1204,29.9982,0.613238,0.613238,0.5,0.5,1.08965,-2.29648,336.083,0.442637,0.442637,0.5,0.5,-1.91035,-3.29648,336.083,0.442637,0.442637,0.5,0.5,51.3159,-94.1204,29.9982,0.613238,0.613238,0.5,0.5,-0.910355,-2.29648,336.083,0.442637,0.442637,0.5,0.5,0.0896454,-2.29648,336.083,0.442637,0.442637,0.5,0.5,-0.910355,-1.29648,336.083,0.442637,0.442637,},
		array<uint>={0,0,0xFFFFFFFF,0,0,0xFFFFFFFF,0,0,0xFFFFFFFF,0,0,0xFFFFFFFF,0,0,0xFFFFFFFF,0,0,0xFFFFFFFF,0,0,0xFFFFFFFF,0,0,0xFFFFFFFF,0,0,0xFFFFFFFF,0,0,0xFFFFFFFF,0,0,0xFFFFFFFF,0,0,0xFFFFFFFF,0,0,0xFFFFFFFF,0,0,0xFFFFFFFF,0,0,0xFFFFFFFF,0,0,0xFFFFFFFF,});
	SpriteGroup second_hand(
		array<string>={'props3','lighting_1','props3','sidewalk_1','props3','sidewalk_1','props3','sidewalk_1','props3','lighting_1','props3','lighting_1',},
		array<int>={8,23,8,23,8,24,8,23,8,24,8,23,},
		array<float>={0.5,0.5,1.08923,-25.9906,180,0.230614,0.230614,0.5,0.5,-0.573029,-82.3453,0,0.442637,0.442637,0.5,0.5,-1.57303,-83.3453,0,0.442637,0.442637,0.5,0.5,-2.57303,-83.3453,0,0.442637,0.442637,0.5,0.5,0.0892258,-25.9906,180,0.230614,0.230614,0.5,0.5,-0.910774,-25.9906,180,0.230614,0.230614,},
		array<uint>={0,0,0xFFFFFFFF,0,0,0xFFFFFFFF,0,0,0xFFFFFFFF,0,0,0xFFFFFFFF,0,0,0xFFFFFFFF,0,0,0xFFFFFFFF,});
	
	float hour_hand_angle_offset = 30;
	float minute_hand_angle_offset = 60;
	float second_hand_angle_offset = 84;
	
	int frame = 0;
	int hours = 0;
	int minutes = 0;
	int seconds = 0;
	
	void init(script@ s, scripttrigger@ self)
	{
		@this.self = self;
		
		self.editor_handle_size(5);
		update_time();
	}
	
	void update_time()
	{
		timedate@ time = localtime();
		hours = time.hour();
		minutes = time.min();
		seconds = time.sec();
	}
	
	void step()
	{
		if(++frame >= 60)
		{
			if(++seconds % 15 == 0)
			{
				update_time();
			}
			
			frame = 0;
		}
	}
	
	void editor_step()
	{
		step();
	}
	
	void draw(float sub_frame)
	{
		const float x = self.x();
		const float y = self.y();
		
		const float seconds_t = (seconds + 1) / 60.0;
		const float seconds_radians = seconds_t * PI * 2 - HALF_PI;
		const float minutes_t = minutes / 60.0;
		const float minutes_radians = minutes_t * PI2 - HALF_PI;
		const float hours_radians = ((hours + minutes_t) / 12.0) * PI2 - HALF_PI;
		
		hour_hand.draw(x, y, hours_radians * RAD2DEG + hour_hand_angle_offset, 1);
		minute_hand.draw(x, y, minutes_radians * RAD2DEG + minute_hand_angle_offset, 1);
		second_hand.draw(x, y, seconds_radians * RAD2DEG + second_hand_angle_offset, 1);
	}
	
	void editor_draw(float sub_frame)
	{
		draw(sub_frame);
	}
	
}