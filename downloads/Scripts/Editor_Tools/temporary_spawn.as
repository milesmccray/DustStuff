#include '../lib/enums/VK.cpp';
#include '../lib/const/ColorConsts.as'
/*
 * Plugin that allows a user to set a temporary spawn without overriding the default level spawn
 */
class script
{
    scene@ g;
    input_api@ i;
    camera@ cam;

    [hidden] bool is_spawn_set = false;
    [hidden] float spawn_x = 0;
    [hidden] float spawn_y = 0;
    [hidden] float facing = 1;

    bool enable_key_pressed = false;
    bool alt_key_pressed = false;
    bool ctrl_key_pressed = false;
    bool dir_key_pressed = false;
    bool waiting_for_release = false;
    bool break_camera = false;
    script() {
      @g = get_scene();
      @i = get_input_api();
    }

    void on_level_start() {
      // == 1. At level start, move player to temporary start if it exists and update camera position ==
      if(is_spawn_set) {
        entity@ c = controller_entity(0);
        camera@ cam = get_camera(0);
        cam.script_camera(true);
        cam.x(spawn_x);
        cam.y(spawn_y);
        c.x(spawn_x);
        c.y(spawn_y);
        c.face(facing);
        //== 2. After player and camera has been moved, reset camera so it centers on player ==
        cam.script_camera(false);
        reset_camera(0);
        
      }
    } 
    
    void editor_step() {
      // == 1. Abort all functionality if user is entering text elsewhere (text trigger, compiling script, etc.) ==
      if(@i != null && i.is_polling_keyboard()) {
        return;
      }

      // == 2. Collect user input
      enable_key_pressed = i.key_check_vk(VK::F);
      alt_key_pressed = i.key_check_vk(VK::Menu);
      ctrl_key_pressed = i.key_check_vk(VK::Control);
      dir_key_pressed = i.key_check_pressed_vk(VK::Left) || i.key_check_pressed_vk(VK::Right);

      if(waiting_for_release) {
        waiting_for_release = alt_key_pressed || enable_key_pressed;
      }

      // == 3. Update spawn while enable key (F) is held. If alt + enable key is pressed, remove temporary spawn
      if(enable_key_pressed && !ctrl_key_pressed) { // check ctrl key to avoid false positives with ctrl+z
        if(!alt_key_pressed && !waiting_for_release) {
          is_spawn_set = true;
          spawn_x = i.mouse_x_world(19);
          spawn_y = i.mouse_y_world(19);
          if(dir_key_pressed) { // updating facing if user presses direction key
            facing = facing == 1 ? -1 : 1;
          }
        } else {
          waiting_for_release = true;
          is_spawn_set = false;
        }
      }
    }

    void editor_draw(float sub_frame) {
      // == 1. Abort all functionality if user is entering text elsewhere (text trigger, compiling script, etc.) ==
      if(@i != null && i.is_polling_keyboard()) {
        return;
      }

     // == 2. If a spawn point has been set, draw temporary spawn point sprite ==
      if(is_spawn_set) {
        sprites@ spr = create_sprites();
        spr.add_sprite_set("dustman");
        spr.draw_world(22, 0, "idle", 0, 0, spawn_x-1, spawn_y-1, 0, facing, 1, BLACK);
				spr.draw_world(22, 0, "idle", 0, 0, spawn_x, spawn_y, 0, facing, 1, TRANSPARENT_GREEN);
				rectangle@ r = spr.get_sprite_rect("idle", 0);

				g.draw_line_world(22, 0, spawn_x - r.get_width()/3, spawn_y, spawn_x + r.get_width()/3, spawn_y, 2, GREEN);
				g.draw_line_world(22, 0, spawn_x, spawn_y, spawn_x, spawn_y-12, 2, GREEN);
      }
    }
}
