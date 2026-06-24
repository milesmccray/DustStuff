class script {
    scene@ g;
    bool active = true;

    script() {
        @g = get_scene();
    }

    void step(int) {
        controllable@ p = controller_controllable(0);
        if (p is null) return;

        if (p.taunt_intent() > 0 and get_nexus_api() !is null) {
            active = not active;
        }
    }

    void draw(float sub_frame) {
        if (active) g.draw_rectangle_hud(0, 0, -800, -450, 800, 450, 0, 0xFF000000);
    }
}
