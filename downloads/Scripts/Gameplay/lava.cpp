const uint COLOUR = 0xA0731008;
const uint BOTTOMCOLOUR = 0xA0FFA000;
const float Y = 240.0;
const float AMPLITUDE = 10.0;
const float WAVELENGTH = 480.0;
const float RECTWIDTH = 12.0;
const float YSPEED = 2;
const float XOFFSET = 0.0;
const float XSPEED = 20.0;

class script{
	
	Lava lava(COLOUR, BOTTOMCOLOUR, Y, AMPLITUDE, WAVELENGTH, RECTWIDTH, YSPEED, XOFFSET, XSPEED);
	
	void step(int entities){
		lava.step();
	}
	
	void checkpoint_load(){
		lava.checkpoint_load();
	}
	
	void checkpoint_save(){
		lava.checkpoint_save();
	}
	
	void draw(float sub_frame){
		lava.draw(sub_frame);
	}
}


const float PI = 3.141592654;

class Lava{
	
	uint colour;
	uint bottomColour;
	float y;
	float amplitude;
	float wavelength;
	float rectWidth;
	float ySpeed;
	float xOffset;
	float xSpeed;
	
	float checkY;
	float checkX;
	
	bool isDead;
	
	int num_players;
	
	array<camera@> cam;
	scene@ g;
	array<dustman@> dm;
	
	Lava(uint colour, uint bottomColour, float y, float amplitude, float wavelength, float rectWidth, float ySpeed, float xOffset, float xSpeed){
		this.colour = colour;
		this.bottomColour = bottomColour;
		this.y = y;
		this.amplitude = amplitude;
		this.wavelength = wavelength;
		this.rectWidth = rectWidth;
		this.ySpeed = ySpeed;
		this.xOffset = xOffset;
		this.xSpeed = xSpeed;
		@g = get_scene();
		
		this.isDead = false;
		
		num_players = num_cameras();
		dm.resize(num_players);
		cam.resize(num_players);
		
		for(int i = 0; i < num_players; ++i){
			@cam[i] = get_camera(i);
		}
	}
	
	array<float> getBounds(){ //returns left, right, bottom, and height, in that order
		camera@ cam = get_active_camera();
		array<float> result = {cam.x(), cam.x(), cam.y(), cam.screen_height()};
		
		return result;
	}
	
	void draw(float sub_frame){
		array<float> bounds = getBounds();
		float left = bounds[0] - bounds[3]; //Should work as long as nobody's using a monitor with a greater than 2:1 aspect ratio
		float right = bounds[1] + bounds[3];
		
		for(float x = floor(left/rectWidth)*rectWidth; x < right+rectWidth; x += rectWidth){
			float top = y + sin((2*PI*(x+(rectWidth/2)) - xOffset)/wavelength)*amplitude;
			
			g.draw_gradient_world(20, 1, x, bounds[2] + bounds[3], x + rectWidth, top, bottomColour, bottomColour, colour, colour);
		}
	}
	
	void kill(){
		for(int i = 0; i < num_players; ++i){
			if(@dm[i] == null){
				entity@ e = controller_entity(i);
				if(@e != null){
					@dm[i] = e.as_dustman();
				}
			}
			else{
				if(dm[i].y() - 10 > y + sin((2*PI*dm[i].x() - xOffset)/wavelength)*amplitude && not isDead){
					g.combo_break_count(g.combo_break_count() + 1);
					if(num_players == 1){
						dm[i].kill(false);
						isDead = true;
					} else {
						scriptenemy@ dead_player = create_scriptenemy(NullPlayer());
						dead_player.x(dm[i].x());
						dead_player.y(dm[i].y());
						g.add_entity(dead_player.as_entity());
						controller_entity(i, dead_player.as_controllable());
						g.remove_entity(dm[i].as_entity());
						@dm[i] = null;
					}
				}
			}
		}
	}
	
	void step(){
		this.kill();
		
		y -= ySpeed;
		xOffset += xSpeed;
	}
	
	void checkpoint_save(){
		checkY = y;
		checkX = xOffset;
	}
	
	void checkpoint_load(){
		@dm[0] = null;
		isDead = false;
		y = checkY;
		xOffset = checkX;
	}
	
}

class NullPlayer : enemy_base
{
	NullPlayer()
	{
	}
}