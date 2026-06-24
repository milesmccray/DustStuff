#include '../lib/drawing/spriteConfig.as'
const string EMBED_water = "image1.png";
class script {
  [text] bool showSprite = true;
  [text] bool animateSprite;
  [text] array<SpriteConfig> water1;
  sprites@ spr;
  scene@ g;
  uint draw_frame = 0;
  uint physics_frame = 0;


  script() {
     @spr = create_sprites();
     @g = get_scene();
  }
  
  void init() {
    for(uint i = 0; i < water1.size(); i++) {
      water1[i].init("water", spr);
    }
  }

  void on_level_start() {
    spr.add_sprite_set("script");
    init();
  }

  void build_sprites(message@ msg) {    
    msg.set_string("water","water");
  }

  void step(int entities) {
    if(g.get_script_fx_level() != 3) return;
      for(uint i = 0; i < water1.size(); i++) {
        water1[i].update();
      }
  }

  void draw(float sub_frame) {
    if(g.get_script_fx_level() != 3) return;
    for(uint i = 0; i < water1.size(); i++) {
      water1[i].draw();
    }
  }

  void editor_step() {
    spr.add_sprite_set("script");
    if(animateSprite) {
      for(uint i = 0; i < water1.size(); i++) {
        water1[i].update();
      }
    }
  }

  void editor_var_changed(var_info@ info) {
    init();
  }

  void editor_draw(float sub_frame) {
    if(showSprite) {
      for(uint i = 0; i < water1.size(); i++) {
        water1[i].draw();
      }
    }
  }

  void on_editor_start() {
    init();
  }
}

