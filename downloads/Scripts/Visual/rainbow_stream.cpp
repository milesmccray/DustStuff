#include '../lib/math/math.cpp'
#include '../lib/utils/colour.cpp'

class script
{
    script() {}
}

class Star
{
    float x, y;
    float vel_x, vel_y;
    float size;
    float life, life_max;
    float rotation, vel_rotation;
    float prev_x, prev_y, prev_rotation;
    
    Star(){}
    
    Star(float x, float y, float vel_x, float vel_y, float size, float life, float vel_rotation)
    {
        this.x = prev_x = x;
        this.y = prev_y = y;
        this.vel_x = vel_x;
        this.vel_y = vel_y;
        this.size = size;
        this.life = round(life_max = life);
        this.vel_rotation = vel_rotation;
        rotation = prev_rotation = rand_range(0, 360);
    }
    
    bool step()
    {
        if(--life <= 0)
            return false;
        
        prev_rotation = rotation;
        prev_x = x;
        prev_y = y;
        
        x += vel_x * DT;
        y += vel_y * DT;
        rotation += vel_rotation;
        
        return true;
    }
    
    void draw(scene@ g, sprites@ spr, float sub_frame, int layer, int sub_layer, uint colour)
    {
        const float l = life < 24 ? (life / 24) : (life > life_max - 24 ? 1 - (life - life_max - 24) / 24 : 1);
        const uint alpha = uint(255 * l) << 24;
        spr.draw_world(layer, sub_layer, 'star', 0, 0,
            lerp(prev_x, x, sub_frame), lerp(prev_y, y, sub_frame), lerp(prev_rotation, rotation, sub_frame),
            size, size, alpha | (colour & 0xFFFFFF));
    }
}

class RainbowStream : trigger_base
{
    scene@ g;
    scripttrigger@ self;
    sprites@ star_spr;
    
    [position,mode:world,layer:19,y:start_y] float start_x;
    [hidden] float start_y;
    [position,mode:world,layer:19,y:end_y] float end_x;
    [hidden] float end_y;
    [text] int layer = 17;
    [text] int sub_layer = 9;
    [text] float resolution = 24;
    [text] float beam_width = 35;
    [text] float beam_length = 48 * 7;
    [text] float wave_speed = 3;
    [text] float wave_size = 24;
    [text] float wave_magnitude = 48;
    [text] int colours_start = -1;
    [text] uint alpha = 255;
    [text] float adjust_lightness = 0.1;
    [text] int star_layer = 17;
    [text] int star_sub_layer = 14;
    [colour,alpha] uint star_colour = 0xFFFFFFFF;
    [text] float star_spawn_offset_min = 0;
    [text] float star_spawn_offset_max = 24;
    [text] float star_density_min = 0.4;
    [text] float star_density_max = 0.8;
    [text] float star_speed_min = 38;
    [text] float star_speed_max = 78;
    [text] float star_size_min = 0.35;
    [text] float star_size_max = 0.75;
    [text] float star_life_min = 80;
    [text] float star_life_max = 240;
    [text] float star_spin_min = -5;
    [text] float star_spin_max = 5;
    
    float dx, dy;
    float angle, length;
    float real_beam_length;
    float real_beam_width;
    float beam_count;
    float star_spawn_timer;
    float t = 0;
    
    array<Star> stars;
    array<uint> base_colours = {0xff0000, 0xff6600, 0xffee00, 0x00ff00, 0x0099ff, 0x4400FF, 0x9900FF};
    int num_colours = int(base_colours.size());
    array<uint> colours;
    
    RainbowStream()
    {
        @g = get_scene();
        @star_spr = create_sprites();
    }
    
    void init(script@ s, scripttrigger@ self)
    {
        @this.self = self;
        
        if(colours_start == -1)
            colours_start = rand_range(0, num_colours - 1);
        if(colours_start < 0)
            colours_start = 0;
        
        if(start_x == 0 and start_y == 0 and end_x == 0 and end_y == 0)
        {
            start_x = self.x() - 96;
            start_y = self.y();
            end_x = self.x() + 96;
            end_y = self.y();
        }
        
        calc();
        star_spr.add_sprite_set('script');
        set_star_spawn_timer();
    }
    
    void set_star_spawn_timer()
    {
        star_spawn_timer = 60 / rand_range(star_density_min * (length / 48), star_density_max * (length / 48));
    }
    
    void calc()
    {
        dx = end_x - start_x;
        dy = end_y - start_y;
        angle = atan2(dy, dx);
        length = magnitude(dx, dy);
        
        if(length == 0)
            length = 0.000001;
        
        real_beam_length = ceil(beam_length / resolution) * resolution - resolution;
        beam_count = ceil(length / beam_width);
        real_beam_width = length / beam_count;
        
        if(adjust_lightness != 0 or colours.size() == 0)
        {
            colours.resize(num_colours);
            for(int i = 0; i < num_colours; i++)
            {
                float h, s, l;
                uint c = base_colours[i];
                const uint r = (c >> 16) & 0xFF;
                const uint gr = (c >> 8) & 0xFF;
                const uint b = (c) & 0xFF;
                rgb_to_hsl(r, gr, b, h, s, l);
                l += adjust_lightness;
                colours[i] = hsl_to_rgb(h, s, l);
            }
        }
    }
    
    void step()
    {
        t -= DT;
        
        if(star_spawn_timer-- <= 0)
        {
            const float nx = -dy / length;
            const float ny = dx / length;
            const float speed = rand_range(star_speed_min, star_speed_max);
            const float u = frand();
            const float v = rand_range(star_spawn_offset_min, star_spawn_offset_max);
            stars.insertLast(Star(
                start_x + (dx * u) + (nx * v), start_y + (dy * u) + (ny * v),
                nx * speed, ny * speed,
                rand_range(star_size_min, star_size_max),
                rand_range(star_life_min, star_life_max),
                rand_range(star_spin_min, star_spin_max)
            ));
            set_star_spawn_timer();
        }
        
        for(int i = int(stars.size()) - 1; i >= 0; i--)
        {
            if(!stars[i].step())
                stars.removeAt(i);
        }
    }
    
    void editor_step()
    {
        calc();
        step();
    }
    
    void draw(float sub_frame)
    {
        const float ndx = dx / length;
        const float ndy = dy / length;
        const float nx = -ndy;
        const float ny = ndx;
        const float beam_width_step = real_beam_width / length;
        int beam_colour_index = colours_start;
        
        for(float beam_index = 0; beam_index < beam_count; beam_index++, beam_colour_index++)
        {
            const float u = beam_index / beam_count;
            const float beam_x1 = u * dx;
            const float beam_y1 = u * dy;
            const float beam_x2 = (u + beam_width_step) * dx;
            const float beam_y2 = (u + beam_width_step) * dy;
            float x1 = beam_x1;
            float y1 = beam_y1;
            float x2 = beam_x2;
            float y2 = beam_y2;
            const uint beam_colour = colours[beam_colour_index % num_colours];
            
            for(float y = resolution; y < beam_length; y += resolution)
            {
                const float v0 = (y - resolution) / real_beam_length;
                const float v = y / real_beam_length;
                const float wave = sin(y / wave_magnitude + t * wave_speed + HALF_PI * 2) * wave_size * min((y / resolution) / 3.0f, 1.0f);
                const float x3 = beam_x2 + v * nx * real_beam_length + wave * ndx;
                const float y3 = beam_y2 + v * ny * real_beam_length + wave * ndy;
                const float x4 = beam_x1 + v * nx * real_beam_length + wave * ndx;
                const float y4 = beam_y1 + v * ny * real_beam_length + wave * ndy;
                
                const uint alpha1 = uint((1 - v0) * alpha) << 24;
                const uint alpha2 = uint((1 - v) * alpha) << 24;
                const uint colour1 = beam_colour | alpha1;
                const uint colour2 = beam_colour | alpha2;
                
                g.draw_quad_world(layer, sub_layer, false,
                    start_x + x1, start_y + y1,
                    start_x + x2, start_y + y2,
                    start_x + x3, start_y + y3,
                    start_x + x4, start_y + y4,
                    colour1, colour1,
                    colour2, colour2);
                
                x1 = x4; y1 = y4;
                x2 = x3; y2 = y3;
            }
        }
        
        for(int i = int(stars.size()) - 1; i >= 0; i--)
        {
            stars[i].draw(g, star_spr, sub_frame, star_layer, star_sub_layer, star_colour);
        }
    }
    
    void editor_draw(float sub_frame)
    {
        draw(sub_frame);
        g.draw_line_world(22, 22, start_x, start_y, end_x, end_y, 1.5, 0xFFFF0000);
    }
}