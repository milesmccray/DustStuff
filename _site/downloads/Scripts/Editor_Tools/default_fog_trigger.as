#include '../lib/enums/VK.cpp';
#include '../lib/const/ColorConsts.as'
/*
 * Plugin that allows a user to jump to saved default trigger
 */
class script
{
    scene@ g;
    input_api@ i;
    editor_api@ e;

    [persist]entity@ trigger;
    [persist]bool is_trigger_set = false;
    [persist]int trigger_id = -1;

    bool alt_key_pressed = false;
    bool ctrl_key_pressed = false;
    bool enable_key_pressed = false;

    script() {
      @g = get_scene();
      @i = get_input_api();
      @e = get_editor_api();
    }

    void on_editor_start() {

    }

    void editor_step() {
      @trigger = entity_by_id(trigger_id);
      if(@trigger == null) {
        is_trigger_set = false;
      }

      // == 1. Abort all functionality if user is entering text elsewhere (text trigger, compiling script, etc.) ==
      if(@i != null && i.is_polling_keyboard()) {
        return;
      }

      // == 2. Collect user input
      enable_key_pressed = i.key_check_vk(VK::N);
      alt_key_pressed = i.key_check_vk(VK::Menu);
      ctrl_key_pressed = i.key_check_vk(VK::Control);
      

      if(enable_key_pressed) { // check ctrl key to avoid false positives with ctrl+z
        if(!alt_key_pressed) {
          if(!is_trigger_set) {
            @trigger = e.get_selected_trigger();
            if(@trigger == null) {
              trigger_id = -1;
              is_trigger_set = false;
            } else {
              trigger_id = trigger.id();
              is_trigger_set = true;
            }
          } else {
            if(@e.get_selected_trigger() != @trigger) {
              e.editor_tab("Triggers");
              e.set_selected_trigger(trigger);
            }
          }
        } else {
          trigger_id = -1;
          is_trigger_set = false;
          @trigger = null;
        }
      }
    }

    void editor_draw(float sub_frame) {
      if(@trigger != null) {
        float size = 10;
        float size2 = size + 1;
        sprites@ spr = create_sprites();
        spr.add_sprite_set("editor");
        spr.draw_world(23, 20, "newhighscore", 0, 0, trigger.x(), trigger.y(), 0, .45, .45, BLUEVIOLET);
        spr.draw_world(23, 20, "newhighscore", 0, 0, trigger.x(), trigger.y(), 0, .35, .35, YELLOW);

      }
    }

    void draw_rect_outline(float x1, float y1, float x2, float y2, uint color) {
      //Top Line
      g.draw_line_world(22, 22, 
                        x1, y1, x2, y1, 2, color);

      //Right Line
      g.draw_line_world(22, 22, 
                        x2, y1, x2, y2, 2, color);

      //Bottom Line
      g.draw_line_world(22, 22, 
                        x1, y2, x2, y2, 2, color);

      //Left Line
      g.draw_line_world(22, 22, 
                        x1, y1, x1, y2, 2, color);
    }
}
