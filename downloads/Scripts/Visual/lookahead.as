#include "../lib/drawing/Sprite.cpp"
#include "../lib/props/common.cpp"

class script{
  scene@ g;
  camera@ c;
  dustman@ dm;
  uint frames = 0;
  bool camera_toggle = false;

  bool is_dustkid = false;

  Sprite spr1;
  [text|tooltip:"Only used when smooth_scroll is off\nValue for how far camera will stretch in px",delay:20,font:sans_bold,size:20,colour/color:0xFFFFFFFF] int stretch;
  [text|tooltip:"Measured in px/frame",delay:20,font:sans_bold,size:20,colour/color:0xFFFFFFFF] int scroll_speed_horz = 50;
  [text|tooltip:"Measured in px/frame",delay:20,font:sans_bold,size:20,colour/color:0xFFFFFFFF] int scroll_speed_vert = 50;
  [text] bool show_telescope = true;
  [text|tooltip:"Moves screen like arrow keys in starcraft would\ninstead of in \"chunks\"",delay:20,font:sans_bold,size:20,colour/color:0xFFFFFFFF] bool smooth_scroll = false;
  [hidden] string sprSet, sprName;

  float target_horz = 0;
  float target_vert = 0;

  bool held_horz = false;
  bool held_vert = false;

  bool update_horz = false;
  bool update_vert = false;

  float telescope_x = 0;
  float telescope_y = 0;

  script() {

  }

  void on_level_start() {
    @c = get_active_camera();
    c.script_camera(true);
  }

  void build_sprites(message@ msg) {
      msg.set_string("spr1","spr1");
  }

  void draw(float) {
    if(show_telescope && camera_toggle) {
      sprite_from_prop(1, 12, 2, sprSet, sprName);
      spr1.set(sprSet, sprName);
      float scale_y = is_dustkid ? .80f : 1.3f;
      spr1.draw(18, 9, 0, 0,
      telescope_x, telescope_y, //x,y
       0, 1, scale_y, 0xffffffff);
    }
  }

  void step(int) {
    if(@controller_entity(0) != null) {
      @dm = controller_entity(0).as_dustman();
    } else {
      return;
    }

    is_dustkid = dm.character() == "dustkid" || dm.character() == "v_dustkid";

    if(dm.ground()) {
      camera_toggle = dm.taunt_intent() == 1 ? !camera_toggle : camera_toggle;
    } 
    
    @c = get_active_camera();
    frames++;
    
    if(!camera_toggle) {
      target_horz = c.x();
      target_vert = c.y();
      scroll_speed_horz = abs(scroll_speed_horz);
      scroll_speed_vert = abs(scroll_speed_vert);
      c.script_camera(false);
    } else {
      c.script_camera(true);
    }
    
    if(@dm != null && dm.x_intent() == 0) {
      held_horz = false;
    }

    if(@dm != null && dm.y_intent() == 0) {
      held_vert = false;
    }

    if(@dm != null && dm.x_intent() != 0 && !held_horz) {
      update_horz = true;
      held_horz = true;
      if(@c != null) {
        scroll_speed_horz = abs(scroll_speed_horz) * abs(dm.x_intent())/dm.x_intent();
        target_horz = c.x() + (c.screen_width() * (abs(dm.x_intent())/dm.x_intent()));
      }
    }

    if(@dm != null && dm.y_intent() != 0 && !held_vert) {
      update_vert = true;
      held_vert = true;
      if(@c != null) {
        scroll_speed_vert = abs(scroll_speed_vert) * abs(dm.y_intent())/dm.y_intent();
        target_vert = c.y() + (c.screen_height() * (abs(dm.y_intent())/dm.y_intent()));
      }
    }

    if(@dm != null && 
       dm.ground() == true &&
       camera_toggle &&
       @c != null) {
        if(smooth_scroll) {
          if(held_horz && check_unload(dm, c.x() + scroll_speed_horz, c.y())) {
            c.x(c.x() + scroll_speed_horz);
          }
          
          if(held_vert && check_unload(dm, c.x(), c.y() + scroll_speed_vert)) {
            c.y(c.y() + scroll_speed_vert);
          } 
        } else {
          if(update_horz && 
          check_unload(dm, c.x() + scroll_speed_horz, c.y()) && 
          scroll_speed_horz > 0 && 
          c.x() + scroll_speed_horz < target_horz) {
            c.x(c.x() + scroll_speed_horz);
          } else if(update_horz && 
          check_unload(dm, c.x() + scroll_speed_horz, c.y()) && 
          scroll_speed_horz < 0 && 
          c.x() + scroll_speed_horz > target_horz) {
            c.x(c.x() + scroll_speed_horz);
          } else {
            puts("else x");
            update_horz = false;
          }
          
          if(update_vert && 
          check_unload(dm, c.x(), c.y() + scroll_speed_vert) && 
          scroll_speed_vert > 0 && 
          c.y() + scroll_speed_vert < target_vert) {
            c.y(c.y() + scroll_speed_vert);
          } else if(update_vert && 
          check_unload(dm, c.x(), c.y() + scroll_speed_vert) && 
          scroll_speed_vert < 0 && 
          c.y() + scroll_speed_vert > target_vert) {
            c.y(c.y() + scroll_speed_vert);
          } else {
            update_vert = false;
          }
        }

        telescope_x = dm.x() + get_x_offset(dm);
        telescope_y = dm.y() + get_y_offset(dm);
        dm.x_intent(0);
        dm.y_intent(0);
        dm.jump_intent(0);
        dm.heavy_intent(0);
        dm.light_intent(0);
        dm.dash_intent(0);
    }
  }

  bool check_unload(dustman @dm, float cx, float cy) {
    return abs(dm.x() - cx) < 2304 && 
           abs(dm.y() - cy) < 2304;
  }

  float get_y_offset(dustman @dm) {
    //dustman, dustgirl, dustkid, dustworth
    if(dm.character() == "dustman" || dm.character() == "v_dustman") {
      return -73;
    }
    
    else if(dm.character() == "dustgirl" || dm.character() == "v_dustgirl") {
      return -74;
    }

    else if(dm.character() == "dustkid" || dm.character() == "v_dustkid") {
      return -50;
    }

    else if (dm.character() == "dustworth" || dm.character() == "v_dustworth") {
      return -70;
    }

    else {
      //who knows just return dustman value
      return -70;
    }
  }

  float get_x_offset(dustman @dm) {
    //dustman, dustgirl, dustkid, dustworth
    if(dm.character() == "dustman" || dm.character() == "v_dustman") {
      return 54;
    }
    
    else if(dm.character() == "dustgirl" || dm.character() == "v_dustgirl") {
      return 48;
    }

    else if(dm.character() == "dustkid" || dm.character() == "v_dustkid") {
      return 63;
    }

    else if (dm.character() == "dustworth" || dm.character() == "v_dustworth") {
      return 59;
    }

    else {
      //who knows just return dustman value
      return 48;
    }
  }
}