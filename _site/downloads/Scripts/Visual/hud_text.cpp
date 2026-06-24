// hud_text.cpp
// Displays customizable text on the HUD when inside a TextTrigger zone.
// HUD coords: x ranges -800 to 800 (0 = center), y ranges -450 to 450 (410 = near bottom)
// permanent: text stays until overwritten by another trigger.

class script : callback_base {

    scene@ g;
    textfield@ tf;
    string current_text = "";
    bool is_permanent = false;
    float current_x = 0;
    float current_y = 410;

    script() {
        @g = get_scene();
        @tf = create_textfield();
        tf.align_horizontal(0);
        tf.align_vertical(0);
        add_broadcast_receiver("hud.text.set", this, "on_text_set");
        add_broadcast_receiver("hud.text.clear", this, "on_text_clear");
    }

    void on_text_set(string id, message@ msg) {
        current_text = msg.get_string("text");
        is_permanent = msg.get_int("permanent") == 1;
        current_x = msg.get_float("x");
        current_y = msg.get_float("y");
        tf.set_font("sans_bold", msg.get_int("size"));
        tf.colour(uint(msg.get_int("colour")));
    }

    void on_text_clear(string id, message@ msg) {
        if (!is_permanent) current_text = "";
    }

    void draw(float sub_frame) {
        if (current_text == "") return;
        tf.text(current_text);
        tf.draw_hud(22, 24, current_x, current_y, 1, 1, 0);
    }

    void editor_draw(float sub_frame) {
        draw(sub_frame);
    }
}

class TextTrigger : trigger_base {

    scripttrigger@ self;
    bool inside = false;
    bool was_inside = false;

    [text] string text = "";
    [colour, alpha] uint colour = 0xFFFFFFFF;
    // Valid sizes: 20, 26, 36
    [text] int size = 26;
    [text] bool permanent = false;
    [text] float x = 700;
    [text] float y = -120;

    TextTrigger() {}

    void init(script@ s, scripttrigger@ self) {
        @this.self = self;
    }

    void step() {
        if (was_inside && !inside) {
            message@ msg = create_message();
            broadcast_message("hud.text.clear", msg);
        }
        was_inside = inside;
        inside = false;
    }

    void activate(controllable@ e) {
        dustman@ dm = e.as_dustman();
        if (@dm == null) return;
        inside = true;

        message@ msg = create_message();
        msg.set_string("text", text);
        msg.set_int("colour", int(colour));
        msg.set_int("size", size);
        msg.set_int("permanent", permanent ? 1 : 0);
        msg.set_float("x", x);
        msg.set_float("y", y);
        broadcast_message("hud.text.set", msg);
    }
}
