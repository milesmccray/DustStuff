class DustmanState {
    float x;
    float y;
    float prev_x;
    float prev_y;
    int face;
    float x_speed;
    float y_speed;
    int state;
    float state_timer;
    bool ground;
    bool wall_left;
    bool wall_right;

    string opImplConv() const {
        return (
            "State("
                + "x=" + x + ", "
                    + "y=" + y + ", "
                        + "prev_x=" + prev_x + ", "
                            + "prev_y=" + prev_y + ", "
                                + "face=" + face + ", "
                                    + "x_speed=" + x_speed + ", "
                                        + "y_speed=" + y_speed + ", "
                                            + "state=" + state + ", "
                                                + "state_timer=" + state_timer + ", "
                                                    + "ground=" + ground + ", "
                                                        + "wall_left=" + wall_left + ", "
                                                            + "wall_right=" + wall_right
                                                                + ")"
        );
    }

    string json() const {
        return "{"
            + "\"x\":" + fpToIEEE(x) + ", "
                + "\"y\":" + fpToIEEE(y) + ", "
                    + "\"prev_x\":" + fpToIEEE(prev_x) + ", "
                        + "\"prev_y\":" + fpToIEEE(prev_y) + ", "
                            + "\"face\":" + face + ", "
                                + "\"x_speed\":" + fpToIEEE(x_speed) + ", "
                                    + "\"y_speed\":" + fpToIEEE(y_speed) + ", "
                                        + "\"state\":" + state + ", "
                                            + "\"state_timer\":" + fpToIEEE(state_timer) + ", "
                                                + "\"ground\":" + ground + ", "
                                                    + "\"wall_left\":" + wall_left + ", "
                                                        + "\"wall_right\":" + wall_right
                                                            + "}";
    }
}

DustmanState dustman_state_from_controllable(dustman@ dm) {
    DustmanState state;
    state.x = dm.x();
    state.y = dm.y();
    state.prev_x = dm.prev_x();
    state.prev_y = dm.prev_y();
    state.face = dm.face();
    state.x_speed = dm.x_speed();
    state.y_speed = dm.y_speed();
    state.state = dm.state();
    state.state_timer = dm.state_timer();
    state.ground = dm.ground();
    state.wall_left = dm.wall_left();
    state.wall_right = dm.wall_right();
    return state;
}

class script: callback_base {
    dustman@ dm;

    void on_level_start() {
        initialise();
    }

    void checkpoint_load() {
        initialise();
    }

    private void initialise() {
        controllable@ p = controller_controllable(0);
        if (p is null) return;
        @dm = p.as_dustman();
        dm.on_subframe_end_callback(this, "on_subframe_end", 0);
    }

    void on_subframe_end(dustman@ dm, int arg) {
        puts(dustman_state_from_controllable(dm).json());
        puts(formatFloat(fpFromIEEE(fpToIEEE(dm.y_speed())), "", 0, 50));
        puts(formatFloat(dm.y_speed(), "", 0, 50));
    }
}
