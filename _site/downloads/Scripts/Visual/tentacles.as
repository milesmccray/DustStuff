#include "../lib/std.cpp"
#include "../lib/math/math.cpp"

class script {
    [bool] bool debug = false;
}

class Tentacle : trigger_base
{
    [text] int layer = 19;
    [text] int sub_layer = 19;
    [color,alpha] uint colour = 0xff000000;

    [angle] float angle = 0;
    [text] int segment_count = 20;
    [text] int segment_length = 40;
    [text] float base_width = 100;
    [text] float wiggle = 1.0;
    [text] float delay = 1.0;

    array<Segment> segments;

    scene@ g;
    script@ s;
    scripttrigger@ self;

    void init(script@ s, scripttrigger@ self)
    {
        srand(get_time_us());

        @this.g = get_scene();
        @this.s = s;
        @this.self = self;

        segments.resize(segment_count);
        for (uint i = 0; i < segments.size(); ++i)
        {
            float segment_width = base_width * (1.0 - float(i + 1) / segments.size());
            float max_angle = 2 * float(i) * segment_count / 20;
            segments[i].init(segment_length, segment_width, max_angle, wiggle, delay);
        }
    }

    void step()
    {
        float current_angle = 0;
        for (uint i = 0; i < segments.size(); ++i)
        {
            current_angle += segments[i].angle;
            float difference = normalize_degress(current_angle - angle);
            if (abs(difference) > 30)
                segments[i].vel -= sign(difference) * (abs(difference) - 30) / 500.0;
            segments[i].step();
        }
    }

    void draw(float sub_frame)
    {
        editor_draw(sub_frame);
    }

    void editor_draw(float sub_frame)
    {
        float cur_angle = angle - 90;
        float cc = cos(DEG2RAD * cur_angle);
        float cs = sin(DEG2RAD * cur_angle);
        float cur_x = self.x();
        float cur_y = self.y();
        Segment@ cur_segment = segments[0];

        float prev_angle, pc, ps, prev_x, prev_y;
        Segment@ prev_segment;

        for (uint i = 1; i < segments.size(); ++i)
        {
            @prev_segment = cur_segment;
            prev_angle = cur_angle;
            pc = cc;
            ps = cs;
            prev_x = cur_x;
            prev_y = cur_y;

            @cur_segment = segments[i];
            cur_angle = prev_angle + prev_segment.angle;
            cc = cos(DEG2RAD * cur_angle);
            cs = sin(DEG2RAD * cur_angle);
            cur_x = prev_x + prev_segment.length * cc;
            cur_y = prev_y + prev_segment.length * cs;

            float prev_width = prev_segment.width;
            float cur_width = cur_segment.width;

            float prev_nx = -ps; // cos(DEG2RAD * (prev_angle + 90));
            float prev_ny =  pc; // sin(DEG2RAD * (prev_angle + 90));

            float cur_nx = -cs; // cos(DEG2RAD * (cur_angle + 90));
            float cur_ny = cc;  // sin(DEG2RAD * (cur_angle + 90));

            if (s.debug)
            {
                g.draw_line_world(22, 0, prev_x, prev_y, cur_x, cur_y, 2, 0x88ff0000);
                g.draw_line_world(
                    22, 0,
                    prev_x - prev_width * prev_nx, prev_y - prev_width * prev_ny,
                    prev_x + prev_width * prev_nx, prev_y + prev_width * prev_ny,
                    2, 0x8800ff00
                );
                g.draw_line_world(
                    22, 0,
                    cur_x + cur_width * cur_nx, cur_y + cur_width * cur_ny,
                    cur_x - cur_width * cur_nx, cur_y - cur_width * cur_ny,
                    2, 0x8800ff00
                );
            }

            g.draw_quad_world(
                layer, sub_layer, false,
                prev_x - prev_width * prev_nx, prev_y - prev_width * prev_ny,
                prev_x + prev_width * prev_nx, prev_y + prev_width * prev_ny,
                cur_x + cur_width * cur_nx, cur_y + cur_width * cur_ny,
                cur_x - cur_width * cur_nx, cur_y - cur_width * cur_ny,
                colour, colour, colour, colour
            );
        }
    }
}

class Segment
{
    float length;
    float max_angle;
    float width;
    float wiggle;
    float delay;

    float angle;

    float mass;
    float vel;
    float force;
    float prev_force;
    float target_force;
    float timer;
    float move_time;

    Segment() {}

    void init(float length, float width, float max_angle, float wiggle, float delay)
    {
        this.length = length;
        this.width = width;
        this.max_angle = max_angle;
        this.wiggle = wiggle;
        this.delay = delay;

        mass = length * max(1.0, width);
        angle = 0.5 * rand_range(-max_angle, max_angle);

        update_target();
    }

    void step()
    {
        if (timer >= move_time)
            update_target();
        timer += 1.0 / 60;
        force = (timer / move_time) * (target_force - prev_force) + prev_force;
        vel += force / mass;
        float resistance_force = -0.5 * angle;
        if (abs(angle) > 0.8 * max_angle)
            resistance_force += -sign(angle) * 10 * (abs(angle) - 0.8 * max_angle);
        vel += resistance_force / mass;
        vel *= 0.95;
        angle += vel;
    }

    void update_target()
    {
        prev_force = force;
        target_force = wiggle * rand_range(-50.0, 50.0);
        timer = 0;
        move_time = delay * rand_range(0.5, 1.5);
    }
}
