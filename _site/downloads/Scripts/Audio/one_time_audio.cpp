const string EMBED_m1 = 'm1.ogg';
const string EMBED_m2 = 'm2.ogg';
const string EMBED_m3 = 'm3.ogg';
const string EMBED_m4 = 'm4.ogg';
const string EMBED_m5 = 'm5.ogg';
const uint WHITE = 4286583372;
class script
{
	
	void build_sounds(message@ msg)
	{
		msg.set_string('m1', 'm1');
		msg.set_string('m2', 'm2');
		msg.set_string('m3', 'm3');
		msg.set_string('m4', 'm4');
		msg.set_string('m5', 'm5');
	}
	
}

class PlaySfx : trigger_base
{
	
	[text] string sound;
	[text] float volume = 1;
	
	int activated;
	
	scene@ g;
	scripttrigger@ self;
	scripttrigger @scr;
	
	
	void init(script@ s, scripttrigger@ self)
	{
		@g = get_scene();
		@this.self = self;
		   @scr = self;
	scr.editor_show_radius(true);
	scr.editor_colour_circle(WHITE);
    scr.editor_colour_inactive(WHITE);
    scr.editor_colour_active(WHITE);
    
	}
	
	void activate(controllable@ c)
	{
		if(c.player_index() == -1)
			return;
		
		if(activated == 0)
		{
			if(sound != '')
			{
				g.play_script_stream(sound, 0, self.x(), self.y(), false, volume);
			}
		}
		
		activated = 4;
	}
	
	void step()
	{
		if(activated > 0)
		{
			activated--;
		}
	}
	
}
