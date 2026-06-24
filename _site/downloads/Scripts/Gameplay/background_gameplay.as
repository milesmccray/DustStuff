class script {
    scene@ g;

    script() {
        @g = get_scene();
        g.default_collision_layer(19);
    }

    void on_level_start() {
        g.default_collision_layer(15);
        g.layer_scale(15, 0.5);
        g.layer_scale(18, 0.5);
        g.swap_layer_order(15, 18);
        g.swap_layer_order(16, 18);
        g.swap_layer_order(15, 16);

        controllable@ p = controller_controllable(0);
        g.get_tile(p.x(), p.y());
    }
}

class layer_switch : trigger_base {
    bool a = false;
    void activate(controllable@ e) {
        if (!a) {
            a = true;
            scene@ g = get_scene();
            g.default_collision_layer(19);
            g.layer_scale(18, 1);
            g.swap_layer_order(16, 18);
        }
    }
}
