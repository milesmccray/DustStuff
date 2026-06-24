// apple_counter.cpp
// Ends the level when a set number of apples have been hit.
// Only counts the first hit on each apple, draws a colour overlay on hit apples.
// Use the fog trigger to change the colour of the overlay layer/sublayer.

class script : callback_base {

    scene@ g;

    [text] int apples_required = 1;
    [text] int overlay_layer = 18;
    [text] int overlay_sublayer = 9;

    int apples_hit = 0;
    dictionary hit_apples;
    array<controllable@> hit_apple_list;

    script() {
        @g = get_scene();
    }

    void on_level_start() {
        apples_hit = 0;
        hit_apples.deleteAll();
        hit_apple_list.resize(0);
        register_apples();
    }

    void register_apples() {
        int count = g.get_entity_collision(-1e6, 1e6, -1e6, 1e6, 7);
        int registered = 0;
        for (int i = 0; i < count; i++) {
            entity@ e = g.get_entity_collision_index(i);
            if (@e == null) continue;
            if (e.type_name() != "hittable_apple") continue;
            hittable@ h = e.as_hittable();
            if (@h == null) continue;
            h.on_hurt_callback(this, "on_apple_hit", 0);
            registered++;
        }
        puts("apple_counter: registered " + registered + " apples, need " + apples_required);
    }

    void on_apple_hit(controllable@ attacked, controllable@ attacker, hitbox@ hb, int arg) {
        if (@attacked == null) return;
        entity@ e = attacked.as_entity();
        if (@e == null) return;

        string apple_id = "" + e.id();
        if (hit_apples.exists(apple_id)) return;
        hit_apples[apple_id] = true;

        controllable@ c = e.as_controllable();
        if (@c != null) hit_apple_list.insertLast(c);

        apples_hit++;
        puts("apple_counter: " + apples_hit + "/" + apples_required + " apples hit");

        if (apples_hit >= apples_required) {
            puts("apple_counter: goal reached, ending level!");
            g.end_level(0, 0);
        }
    }

    void draw(float sub_frame) {
        for (uint i = 0; i < hit_apple_list.length(); i++) {
            controllable@ c = hit_apple_list[i];
            if (@c == null || c.as_entity().destroyed()) continue;

            sprites@ spr = c.get_sprites();

            string sprite_name = c.sprite_index();
            uint frame = uint(c.state_timer()) % spr.get_animation_length(sprite_name);

            float x = c.prev_x() + (c.x() - c.prev_x()) * sub_frame;
            float y = c.prev_y() + (c.y() - c.prev_y()) * sub_frame;

            spr.draw_world(
                overlay_layer, overlay_sublayer,
                sprite_name, frame, 0,
                x + c.draw_offset_x(), y + c.draw_offset_y(),
                c.rotation(),
                c.face() * c.scale(), c.scale(),
                0xFFFFFFFF
            );
        }
    }

    void checkpoint_load() {
        apples_hit = 0;
        hit_apples.deleteAll();
        hit_apple_list.resize(0);
        register_apples();
    }
}