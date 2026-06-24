class script {
    scene@ g;
    controllable@ player;

    bool on_treadmill = false;

    script() {
        @g = get_scene();
    }

    void on_level_start() {
        @player = controller_controllable(0);
    }

    void checkpoint_load() {
        @player = controller_controllable(0);
    }

    void step(int entities) {
        if (player !is null) {
            if (player.y() == 0) {
                if (!on_treadmill) {
                    on_treadmill = true;
                }
                float x = player.x();
                player.x(x + 5);
            } else {
                if (on_treadmill) {
                    float x_speed = player.x_speed();
                    float y_speed = player.y_speed();
                    player.set_speed_xy(x_speed+300, y_speed);
                    on_treadmill = false;
                }
            }
        }
    }
}
