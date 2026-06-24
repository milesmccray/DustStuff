// entity_id_display.cpp
//
// Editor-only plugin. Hovering the mouse over an entity shows its ID
// in text positioned just below that entity on screen.
//
// Install as a plugin via Dustmod > Mods > Tools, then enable under
// Dustmod > Mods > Plugins.

class script : callback_base {

    scene@ g;
    editor_api@ ed;
    input_api@ inp;

    textfield@ tf;

    bool has_hover = false;
    int hover_id = -1;
    float hover_world_x = 0;
    float hover_world_y = 0;

    float search_radius = 24;

    script() {
        @g = get_scene();

        @tf = create_textfield();
        tf.align_horizontal(0);
        tf.align_vertical(0);
        tf.set_font("sans_bold", 20);
        tf.colour(0xFFFFFFFF);
    }

    void editor_step() {
        @ed = get_editor_api();
        @inp = get_input_api();
        if (@inp == null || @ed == null) {
            has_hover = false;
            return;
        }

        if (ed.mouse_in_gui()) {
            has_hover = false;
            return;
        }

        int layer = ed.get_selected_layer();
        float wx = inp.mouse_x_world(layer);
        float wy = inp.mouse_y_world(layer);

        entity@ found = find_closest_in_radius(wx, wy, 7); // col_type_hittable
        if (@found == null) {
            @found = find_closest_in_radius(wx, wy, 16); // col_type_trigger
        }
        if (@found == null) {
            @found = find_closest_in_radius(wx, wy, 1); // col_type_enemy
        }

        if (@found == null) {
            has_hover = false;
            return;
        }

        has_hover = true;
        hover_id = found.id();
        hover_world_x = found.x();
        hover_world_y = found.y();
    }

    entity@ find_closest_in_radius(float wx, float wy, uint col_type) {
        int count = g.get_entity_collision(
            wy - search_radius, wy + search_radius,
            wx - search_radius, wx + search_radius, col_type);

        entity@ closest = null;
        float closest_dist_sq = search_radius * search_radius;

        for (int i = 0; i < count; i++) {
            entity@ e = g.get_entity_collision_index(i);
            if (@e == null) continue;

            float dx = e.x() - wx;
            float dy = e.y() - wy;
            float dist_sq = dx * dx + dy * dy;

            if (dist_sq < closest_dist_sq) {
                closest_dist_sq = dist_sq;
                @closest = e;
            }
        }

        return closest;
    }

    void editor_draw(float sub_frame) {
        if (!has_hover) return;

        tf.text("ID: " + hover_id);
        tf.draw_world(22, 22, hover_world_x, hover_world_y - 40, 1, 1, 0);
    }
}