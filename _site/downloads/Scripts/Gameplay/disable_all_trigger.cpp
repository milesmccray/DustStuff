// disable_all_trigger.cpp
// AttackTrigger: disables/enables light, heavy, special. Checked = disabled.
// MovementTrigger: disables/enables jump, dash, aircharge. Checked = disabled.
// DirectionTrigger: disables/enables left, right, up, down, downdash. Checked = disabled.
// repeat: if checked, trigger fires every time the player enters it.

class script : callback_base {

    bool light_disabled = false;
    bool heavy_disabled = false;
    bool disable_left = false;
    bool disable_right = false;
    bool disable_up = false;
    bool disable_down = false;
    bool disable_downdash = false;
    bool disable_aircharge = false;
    scene@ g;

    script() {
        @g = get_scene();
        add_broadcast_receiver("attack.set", this, "on_attack_set");
        add_broadcast_receiver("movement.set", this, "on_movement_set");
        add_broadcast_receiver("direction.set", this, "on_direction_set");
    }

    void on_attack_set(string id, message@ msg) {
        light_disabled = msg.get_int("light") == 1;
        heavy_disabled = msg.get_int("heavy") == 1;
        g.special_enabled(msg.get_int("special") == 0);
    }

    void on_movement_set(string id, message@ msg) {
        g.jump_enabled(msg.get_int("jump") == 1);
        g.dash_enabled(msg.get_int("dash") == 1);
        disable_aircharge = msg.get_int("aircharge") == 1;
    }

    void on_direction_set(string id, message@ msg) {
        disable_left     = msg.get_int("left") == 1;
        disable_right    = msg.get_int("right") == 1;
        disable_up       = msg.get_int("up") == 1;
        disable_down     = msg.get_int("down") == 1;
        disable_downdash = msg.get_int("downdash") == 1;
    }

    void step(int entities) {
        uint num_players = num_cameras();
        for (uint i = 0; i < num_players; i++) {
            controllable@ ctrl = controller_controllable(i);
            if (@ctrl == null) continue;

            if (light_disabled) ctrl.light_intent(0);
            if (heavy_disabled) ctrl.heavy_intent(0);

            if (disable_left && ctrl.x_intent() == -1)  ctrl.x_intent(0);
            if (disable_right && ctrl.x_intent() == 1)  ctrl.x_intent(0);
            if (disable_up && ctrl.y_intent() == -1)    ctrl.y_intent(0);
            if (disable_down && ctrl.y_intent() == 1)   ctrl.y_intent(0);
            if (disable_downdash && ctrl.fall_intent() == 1) ctrl.fall_intent(0);

            if (disable_aircharge) {
                dustman@ dm = ctrl.as_dustman();
                if (@dm != null) dm.dash(0);
            }
        }
    }
}

class AttackTrigger : trigger_base {

    scripttrigger@ self;

    [text] bool disable_light = false;
    [text] bool disable_heavy = false;
    [text] bool disable_special = false;
    [text] bool repeat = false;

    bool fired = false;
    bool inside = false;

    AttackTrigger() {}

    void init(script@ s, scripttrigger@ self) {
        @this.self = self;
    }

    void step() {
        if (repeat && fired && !inside) fired = false;
        inside = false;
    }

    void activate(controllable@ e) {
        dustman@ dm = e.as_dustman();
        if (@dm == null) return;

        inside = true;
        if (fired) return;

        message@ msg = create_message();
        msg.set_int("light", disable_light ? 1 : 0);
        msg.set_int("heavy", disable_heavy ? 1 : 0);
        msg.set_int("special", disable_special ? 1 : 0);
        broadcast_message("attack.set", msg);

        fired = true;
    }

    void checkpoint_load() {
        fired = false;
        inside = false;
    }
}

class MovementTrigger : trigger_base {

    scripttrigger@ self;

    [text] bool disable_jump = false;
    [text] bool disable_dash = false;
    [text] bool disable_aircharge = false;
    [text] bool repeat = false;

    bool fired = false;
    bool inside = false;

    MovementTrigger() {}

    void init(script@ s, scripttrigger@ self) {
        @this.self = self;
    }

    void step() {
        if (repeat && fired && !inside) fired = false;
        inside = false;
    }

    void activate(controllable@ e) {
        dustman@ dm = e.as_dustman();
        if (@dm == null) return;

        inside = true;
        if (fired) return;

        message@ msg = create_message();
        msg.set_int("jump", disable_jump ? 1 : 0);
        msg.set_int("dash", disable_dash ? 1 : 0);
        msg.set_int("aircharge", disable_aircharge ? 1 : 0);
        broadcast_message("movement.set", msg);

        fired = true;
    }

    void checkpoint_load() {
        fired = false;
        inside = false;
    }
}

class DirectionTrigger : trigger_base {

    scripttrigger@ self;

    [text] bool disable_left = false;
    [text] bool disable_right = false;
    [text] bool disable_up = false;
    [text] bool disable_down = false;
    [text] bool disable_downdash = false;
    [text] bool repeat = false;

    bool fired = false;
    bool inside = false;

    DirectionTrigger() {}

    void init(script@ s, scripttrigger@ self) {
        @this.self = self;
    }

    void step() {
        if (repeat && fired && !inside) fired = false;
        inside = false;
    }

    void activate(controllable@ e) {
        dustman@ dm = e.as_dustman();
        if (@dm == null) return;

        inside = true;
        if (fired) return;

        message@ msg = create_message();
        msg.set_int("left",     disable_left ? 1 : 0);
        msg.set_int("right",    disable_right ? 1 : 0);
        msg.set_int("up",       disable_up ? 1 : 0);
        msg.set_int("down",     disable_down ? 1 : 0);
        msg.set_int("downdash", disable_downdash ? 1 : 0);
        broadcast_message("direction.set", msg);

        fired = true;
    }

    void checkpoint_load() {
        fired = false;
        inside = false;
    }
}