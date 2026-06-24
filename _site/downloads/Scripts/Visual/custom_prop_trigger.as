// Change this number to match the number of embed variables
const int NUM_PROPS = 1;

// Change these strings to change the embedded props
const string EMBED_prop0 = "p1.png";
//const string EMBED_prop1 = "p2.png";
//const string EMBED_prop2 = "p3.png";
//const string EMBED_prop3 = "p4.png";
//const string EMBED_prop4 = "p5.png";
//const string EMBED_prop5 = "p6.png";
//const string EMBED_prop6 = "p7.png";
//const string EMBED_prop7 = "p8.png";
//const string EMBED_prop8 = "p9.png";
//const string EMBED_prop9 = "p10.png";
//const string EMBED_prop10 = "p11.png";


class script {
	void build_sprites(message@ msg) {
		for (int i=0; i < NUM_PROPS; i++) {
			msg.set_string("embed_prop"+i, "prop"+i);
			msg.set_int("embed_prop"+i+"|offsetx", 0);
			msg.set_int("embed_prop"+i+"|offsety", 0);
		}
	}
}

class prop_trigger : trigger_base {
	[text] int layer = 20;
	[text] int sub_layer = 19;
	[text] float rotation = 0.0;
	[text] int index = 0;
	[slider, min:1, max:10] float scale_x = 1.0;
	[slider, min:1, max:10] float scale_y = 1.0;
	uint32 colour = 0xffffffff;
	
	scripttrigger@ self;
	sprites@ spr;
	void init(script@ s, scripttrigger@ st) {
		@self = st;
		@spr = create_sprites();
		spr.add_sprite_set("script");
	}
	
	void draw(float sf) {_draw();}
	void editor_draw(float sf) {_draw();}
	
	void _draw() {
		spr.draw_world(layer, sub_layer, "embed_prop"+index, 1, 1, self.x(), self.y(), rotation, scale_x, scale_y, colour);
	}
}