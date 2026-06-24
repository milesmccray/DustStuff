class script : callback_base {
    [text] float turn_speed = 3.5;

    scene@ g;
    dustman@ dm;
    camera@ cam;

    float start_x;
    float start_y;
    float start_vx;
    float start_vy;

    bool held = false;
    bool fly = false;
    float direction;
    float speed;

    script() {
        @g = get_scene();
    }

    void on_level_start() {
        controllable@ c = controller_controllable(0);
        if (c !is null) {
            @dm = c.as_dustman();
            if (dm !is null) {
                dm.on_subframe_end_callback(this, "on_subframe_end", 0);
            }
        }
    }

    void on_subframe_end(dustman@, int) {
        if (fly) {
            if (dm.ground() or dm.wall_left() or dm.wall_right() or dm.roof()) {
                fly = false;
                return;
            }

            dm.x(start_x + start_vx / 60.0);
            dm.y(start_y + start_vy / 60.0);
            dm.set_speed_xy(start_vx, start_vy);
        }
    }

    void step(int) {
        if (dm !is null) {
            start_x = dm.x();
            start_y = dm.y();
            start_vx = dm.x_speed();
            start_vy = dm.y_speed();

            if (not held and dm.light_intent() == 10) {
                held = true;
                dm.light_intent(11);
            } else if (held and dm.light_intent() == 0) {
                held = false;
            }

            if (not fly and held and dm.y_speed() > 0) {
                fly = true;
                direction = dm.direction();
                speed = dm.speed();
                puts(formatFloat(start_vx));
                puts(formatFloat(start_vy));
            } else if (fly and (not held or dm.speed() < 100)) {
                fly = false;
                dm.state(19);
            }
        }
    }

    void step_post(int) {
        if (fly) {
            // Find the target angle
            float target = dm.x_intent() * (90 + 45 * dm.y_intent());
            if (dm.x_intent() == 0 and dm.y_intent() == 1) target = 180;

            // FLY
            if (dm.x_intent() != 0 or dm.y_intent() != 0) {
                float new_direction = absolute_towards(dm.direction(), target, turn_speed);
                dm.set_speed_direction(dm.speed(), new_direction);
            }

            // Artificial gravity
            dm.set_speed_xy(dm.x_speed(), dm.y_speed() + dm.hover_accel() / 60.0);
        }

    }

}

/* Both direction and target are in (-180, 180] */
float absolute_towards(float direction, float target, float amount) {
    float clockwise, anticlock;
    if (direction < target) {
        clockwise = target - direction;
        anticlock = 360 - clockwise;
    } else {
        anticlock = direction - target;
        clockwise = 360 - anticlock;
    }

    if (clockwise < anticlock) {
        if (clockwise < amount) return target;
        else return fix_angle(direction + amount);
    } else {
        if (anticlock < amount) return target;
        else return fix_angle(direction - amount);
    }
}

/* Fix an angle that is slightly out of bounds */
float fix_angle(float angle) {
    if (angle < -180) return angle + 360;
    if (angle >  180) return angle - 360;
    return angle;
}
