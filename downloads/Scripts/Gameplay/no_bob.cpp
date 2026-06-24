class script {
    scene@ g;
    array<string> allowedEnemies = {"enemy_critter", "enemy_gargoyle_small", "enemy_trash_ball", "enemy_slime_ball", "enemy_key"};

    script() {
        @g = get_scene();
    }

    void step(int entities) {
        for (int i = 0; i < entities; i++) {
            entity@ e = entity_by_index(i);
            if (@e == null)
                continue;
            controllable@ c = e.as_controllable();
            if (@c == null)
                continue;
            if (allowedEnemies.find(c.type_name()) > -1) {
                if (c.state() != 20 && c.life() > 0)
                    c.set_speed_xy(0, 0);
            }
        }
    }
}