class script : callback_base {
    scene@ g;
    dustman@ player;
    bool handler_set = false;
    bool noclip_enabled = false;
    int prev_taunt = 0;

    script() {
        @g = get_scene();
    }

    void on_level_start() {
        controllable@ c = controller_controllable(0);
        if (@c != null) {
            @player = c.as_dustman();
        }
    }

    void checkpoint_load() {
        handler_set = false;
        controllable@ c = controller_controllable(0);
        if (@c != null) {
            @player = c.as_dustman();
        }
    }

    void step(int entities) {
        if (@player == null) return;

        int taunt = player.taunt_intent();
        if (taunt == 1 && prev_taunt == 0) {
            noclip_enabled = !noclip_enabled;
            puts("noclip: " + (noclip_enabled ? "ON" : "OFF"));
            player.taunt_intent(2);
        }
        prev_taunt = taunt;

        if (noclip_enabled && !handler_set) {
            player.set_collision_handler(this, "collision_handler", 0);
            handler_set = true;
        } else if (!noclip_enabled && handler_set) {
            player.set_collision_handler(null, "collision_handler", 0);
            handler_set = false;
        }
    }

    void collision_handler(controllable@ ec, tilecollision@ tc, int side, bool moving, float snap_offset, int arg) {
        tc.hit(false);
    }
}