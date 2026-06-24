const string EMBED_MOON = "moontheme.ogg";

class script : callback_base {

	scene@ g;
	dustman@ player;
	
	bool moonmode, moonstarted;
	float persist_y, persist_speed;
	
	script() {
		@g = get_scene();
		add_broadcast_receiver('initiate_moon', this, 'initiate_moon');
	}
	
	void initiate_moon(string id, message@ msg) {
		if (msg.get_int("into") == 0 && !moonstarted) {
			moonmode = true;
			persist_y = player.y();
			persist_speed = player.x_speed();
			audio@ ab = g.play_script_stream("MOON", 1, 0, 0, true, 1);
			moonstarted = true;
		} else if (msg.get_int("into") == 1) {
			moonmode = false;
		}
	}
	
	void on_level_start() {
		@player = controller_controllable(0).as_dustman();
		
		moonmode = false;
		moonstarted = false;
	}
	
	void build_sounds(message@ msg) {
		msg.set_string("MOON", "MOON");
		msg.set_int("MOON|loop", 677685);
	}
	
	void step(int entities) {
		if (moonmode) {
			player.x_intent(0);
			player.y_intent(0);
			player.light_intent(0);
			player.heavy_intent(0);
			player.jump_intent(0);
			player.dash_intent(0);
			player.taunt_intent(0);
			
			player.set_speed_xy(persist_speed, 0);
			player.y(persist_y);
			player.combo_timer(1);
		}
	}
	
	void draw(float subframe) {
		
	}

}

class moontrigger : trigger_base {
	scene@ g;
	scripttrigger@ self;
	bool activated;
	bool active_this_frame;
	controllable@ trigger_entity;
	[text] int position;

	moontrigger() {
		@g = get_scene();
	}

	void init(script@ s, scripttrigger@ self) {
		@this.self = @self;
		activated = false;
		active_this_frame = false;
	}

	void rising_edge(controllable@ e) {
		@trigger_entity = @e;
		notifyScript();
	}

	void step() {
		if(activated) {
			if(not active_this_frame) {
				activated = false;
			}
			active_this_frame = false;
		}
	}

	void activate(controllable@ e) {
		if(e.player_index() == 0) {
			if(not activated) {
				rising_edge(@e);
				activated = true;
			}
			active_this_frame = true;
		}
	}

	void notifyScript() {
		message@ msg = create_message();
		msg.set_int('into', position);
		msg.set_string('triggerType',"moonTrigger");
		broadcast_message('initiate_moon', msg);
	}
}