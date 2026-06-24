class script {
    [text] float delay = 600;

    int wobble = 0;
    int frame = 0;

    camera@ c;

    script() {
        @c = get_camera(0);
    }

    void step(int) {
        if (wobble > 0) {
            frame += 1;
            wobble -= 1;

            float t = (pow(1.3, (19.0 * wobble / delay)) - 1.0) / 200.0;

            c.rotation(90.0 * t * sin(frame / 30.0));
            c.scale_x(1 + t / 2.0 * sin(frame / 17.134));
            c.scale_y(1 + t / 3.0 * sin(frame / 19.471));

        } else {
            frame = 0;
        }
    }
}

class Wobble : trigger_base {
    [text] bool instant = true;
    [text] bool shake = false;

    script@ s;
    scripttrigger@ self;

    void init(script@ s, scripttrigger@ self) {
        @this.s = @s;
        @this.self = @self;
    }

    void activate(controllable@ c) {
        camera@ cam = get_camera(0);
        if (c.player_index() != -1) {
            if (instant) {
                s.wobble = s.delay;
            } else if (s.wobble < s.delay) {
                s.wobble = min(s.delay, s.wobble+2);
            }
            if (shake) cam.add_screen_shake(c.x(), c.y(), rand()%360, 10);
        }
    }
}
