#include "../lib/enums/VK.cpp";
class script{
  scene@ g;
  array<int> entity_ids;
  input_api@ input;
  bool enable = true;
  void update_ent_list() {
    entity_ids.resize(0);
    @g = get_scene();
    camera@ cam = get_active_camera();
    float view1_x, view1_y, view1_w, view1_h;
    float view2_x, view2_y, view2_w, view2_h;
    float padding_x = 0;
    float padding_y = 0;
    cam.get_layer_draw_rect(0, 19, view1_x, view1_y, view1_w, view1_h);
    cam.get_layer_draw_rect(1, 19, view2_x, view2_y, view2_w, view2_h);
    view1_x -= padding_x; view1_y -= padding_x;
    view2_x -= padding_y; view2_y -= padding_y;
    view1_w += padding_x * 2; view1_h += padding_x * 2;
    view2_w += padding_y * 2; view2_h += padding_y * 2;

    const float view_x1 = min(view1_x, view2_x);
    const float view_y1 = min(view1_y, view2_y);
    const float view_x2 = max(view1_x + view1_w, view2_x + view2_w);
    const float view_y2 = max(view1_y + view1_h, view2_y + view2_h);
    const int count = g.get_entity_collision(view_y1, view_y2, view_x1, view_x2, 1); // 1 = col_type_enemy

    for(int i = 0; i < count; i++) {
      entity@ e = g.get_entity_collision_index(i);
      if(e.type_name() != "enemy_key") continue;
      entity_ids.push_back(e.id());
    }
  }

  script() {
    update_ent_list();
    @input = get_input_api();
  }

  void editor_step() {
    if((input.key_check_vk(VK::LeftMenu) || input.key_check_vk(VK::RightMenu)) && input.key_check_pressed_vk(VK::K)) {
      enable = !enable;
    }
    if(!enable) return;
    update_ent_list();
  }

  void editor_draw(float sub_frame) {
    if(!enable) return;

    for(uint i = 0; i < entity_ids.size(); i++) {
      entity@ e = entity_by_id(entity_ids[i]);
      if(@e == null) continue;
      entity@ door = entity_by_id(e.vars().get_var("door_id").get_int32());
      if(@e != null && @door != null) {
        g.draw_line_world(22, 20, e.x(), e.y(), door.x(), door.y(), 7, 0xAAFF0000);
        g.draw_line_world(22, 20, e.x(), e.y(), door.x(), door.y(), 3 , 0xAAFFFFFF);
      } else if (@e != null && @door == null) {
        //void draw_rectangle_world(uint layer, uint sub_layer, float x1, float y1, float x2, float y2, float rotation, uint colour);
        g.draw_rectangle_world(22, 20, e.x()-10, e.y()-10, e.x()+10, e.y()+10, 0, 0xFFFF0000);
      } else {
        //idk man
      }
    }
  }
}