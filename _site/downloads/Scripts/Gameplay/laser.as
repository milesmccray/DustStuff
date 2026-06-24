class script {
    scene@ g;
    dustman@ player;

    script() {
        @g = get_scene();
    }

    void on_level_start() {
        @player = controller_controllable(0).as_dustman();
    }

    void checkpoint_load() {
        @player = controller_controllable(0).as_dustman();
    }
}

class laser : trigger_base {
    [slider, min:1, max:50] float drag = 20;
    [text] float range = 5000;
    [text] int max_bounces = 3;

    script@ script;
    scripttrigger@ self;

    float target_angle;

    int num_points;
    array<point@> points;

    void init(script@ script, scripttrigger@ self) {
        @this.script = @script;
        @this.self = @self;

        points.resize(max_bounces+2);
        @points[0] = point(self.x(), self.y());

        if (script.player !is null) {
            target_angle = atan2(script.player.y()-48-self.y(), script.player.x() - self.x());
        }
    }

    void editor_draw(float sub_frame) {
        script.g.draw_rectangle_world(20, 0, self.x()-48, self.y()-48, self.x()+48, self.y()+48, 0, 0x3FFFFFFF);
    }

    void draw(float sub_frame) {
        for (int l=1; l<num_points; ++l) {
            script.g.draw_line_world(19, 0, points[l-1].x, points[l-1].y, points[l].x, points[l].y, 24, 0x7FFF0000);
        }
    }

    point@ find_connected_tile(int tile_x, int tile_y, tileinfo@ tile) {
        int type = tile.type();
        if (1 <= type && type <= 16) {
            // The type that the connected tile would be if it is there
            int connected_type = (type % 2 == 1) ? type + 1 : type - 1;

            int connected_x = tile_x;
            int connected_y = tile_y;
            if (type == 1 || type == 6 || type == 10 || type == 13) ++connected_x;
            if (type == 2 || type == 5 || type ==  9 || type == 14) --connected_x;
            if (type == 3 || type == 8 || type == 11 || type == 16) ++connected_y;
            if (type == 4 || type == 7 || type == 12 || type == 15) --connected_y;

            tileinfo@ connected_tile = script.g.get_tile(connected_x, connected_y);
            if (connected_tile.type() == connected_type) {
                return point(connected_x, connected_y);
            }
        }
        return null;
    }

    void fire_laser(point dest) {
        num_points = 1;
        continue_ray(point(self.x(), self.y()), dest);
    }

    void continue_ray(point start, point dest, raycast@ ray=null) {
        if (ray is null) {
            @ray = script.g.ray_cast_tiles(start.x, start.y, dest.x, dest.y);
        }
        tileinfo@ tile = script.g.get_tile(ray.tile_x(), ray.tile_y());

        // If this is a window tile
        if (tile.sprite_set() == 4 && tile.sprite_tile() == 8) {
            // Remove the connected tile
            // This eliminates weird bugs where raycasts enter a tile without crossing an edge
            tileinfo@ connected_tile;
            point@ connected_tile_pos = find_connected_tile(ray.tile_x(), ray.tile_y(), tile);
            if (connected_tile_pos !is null) {
                @connected_tile = script.g.get_tile(connected_tile_pos.x, connected_tile_pos.y);
                script.g.set_tile(connected_tile_pos.x, connected_tile_pos.y, 19, false, 0, 0, 0, 0);
            }

            // Remove the hit tile
            script.g.set_tile(ray.tile_x(), ray.tile_y(), 19, false, 0, 0, 0, 0);

            // Continue the raycast
            continue_ray(point(ray.hit_x(), ray.hit_y()), dest);

            // Replace the hit tile
            script.g.set_tile(ray.tile_x(), ray.tile_y(), 19, tile, true);

            // Replace the connected tile
            if (connected_tile_pos !is null) {
                script.g.set_tile(connected_tile_pos.x, connected_tile_pos.y, 19, connected_tile, true);
            }
        } else if (tile.sprite_set() == 4 && tile.sprite_tile() == 6 && tile.sprite_palette() == 2) {
            @points[num_points++] = point(ray.hit_x(), ray.hit_y());
            if (num_points > max_bounces+1) return;

            int angle = ray.angle();

            point offset = point(dest.x - ray.hit_x(), dest.y - ray.hit_y());
            float magnitude = hypot(offset.x, offset.y);

            point new_offset = point(offset.x, offset.y);

            if (angle ==   0 || angle == 180) { new_offset = point( offset.x, -offset.y); }
            if (angle ==  90 || angle == 270) { new_offset = point(-offset.x,  offset.y); }
            
            if (angle ==  45 || angle == 225) { new_offset = point( offset.y,  offset.x); }
            if (angle == -45 || angle == 135) { new_offset = point(-offset.y, -offset.x); }

            if (angle ==  27 || angle == 207) { new_offset = point( 0.6*offset.x+0.8*offset.y,  0.8*offset.x-0.6*offset.y); }
            if (angle == -27 || angle == 153) { new_offset = point( 0.6*offset.x-0.8*offset.y, -0.8*offset.x-0.6*offset.y); }
            if (angle ==  63 || angle == 243) { new_offset = point(-0.6*offset.x+0.8*offset.y,  0.8*offset.x+0.6*offset.y); }
            if (angle == -63 || angle == 117) { new_offset = point(-0.6*offset.x-0.8*offset.y, -0.8*offset.x+0.6*offset.y); }

            dest.x = new_offset.x + ray.hit_x();
            dest.y = new_offset.y + ray.hit_y();
    
            // Move one unit away from the mirror then continue
            tileinfo@ tile = script.g.get_tile(ray.tile_x(), ray.tile_y());
            script.g.set_tile(ray.tile_x(), ray.tile_y(), 19, false, 0, 0, 0, 0);
            raycast@ new_ray = script.g.ray_cast_tiles(ray.hit_x(), ray.hit_y(), dest.x, dest.y);
            script.g.set_tile(ray.tile_x(), ray.tile_y(), 19, tile, true);

            continue_ray(point(ray.hit_x(), ray.hit_y()), dest, new_ray);
        } else if (ray.hit()){
            @points[num_points++] = point(ray.hit_x(), ray.hit_y());
        } else {
            @points[num_points++] = dest; // TODO: maybe point(dest);
        }
    }

    bool hit_player() {
        for (int l=1; l<num_points; ++l) {
            if (player_line_intersect(script.player, line(points[l-1], points[l]))) {
                return true;
            }
        }
        return false;
    }

    void step() {
        if (@script.player !is null) {
            float player_angle = atan2(script.player.y()-48-self.y(), script.player.x() - self.x());

            // Imagine turning anticlockwise
            float anti_angle = player_angle - target_angle;
            if (player_angle < target_angle) {
                anti_angle += 2 * 3.14159;
            }
            
            // Imagine turning clockwise
            float clock_angle = target_angle - player_angle;
            if (target_angle < player_angle) {
                clock_angle += 2 * 3.14159;
            }

            // Find the best way to turn to reach player_angle
            float delta_angle = anti_angle < clock_angle ? anti_angle : -clock_angle;

            target_angle += delta_angle / drag;
            target_angle %= 2 * 3.14159;

            float dest_x = self.x() + range * cos(target_angle);
            float dest_y = self.y() + range * sin(target_angle);

            fire_laser(point(dest_x, dest_y));

            if (hit_player() && !script.player.dead()) {
                script.player.stun(0, 0);
                script.player.kill(false);
                script.g.combo_break_count(script.g.combo_break_count()+1);
            }
        }
    }
}

float hypot(float a, float b) {
    return sqrt(a*a + b*b);
}

class point {
    float x, y;

    point() {
        x = 0;
        y = 0;
    }

    point(float _x, float _y) {
        x = _x;
        y = _y;
    }
}; 

class line {
    point p, q;

    line() {
        p = point();
        q = point();
    }

    line(point _p, point _q) {
        p = _p;
        q = _q;
    }
};

bool onSegment(point p, point q, point r) { 
    return (q.x <= max(p.x, r.x) && q.x >= min(p.x, r.x) && 
            q.y <= max(p.y, r.y) && q.y >= min(p.y, r.y));
} 
  
int orientation(point p, point q, point r) { 
    float val = (q.y - p.y) * (r.x - q.x) - (q.x - p.x) * (r.y - q.y); 
    if (val == 0) return 0; // colinear 
    return (val > 0)? 1: 2; // clock/counterclock wise 
} 
  
bool line_line_intersect(line l1, line l2) { 
    point p1 = l1.p;
    point q1 = l1.q;
    point p2 = l2.p;
    point q2 = l2.q;

    int o1 = orientation(p1, q1, p2); 
    int o2 = orientation(p1, q1, q2); 
    int o3 = orientation(p2, q2, p1); 
    int o4 = orientation(p2, q2, q1); 
  
    if (o1 != o2 && o3 != o4) return true; 
    if (o1 == 0 && onSegment(p1, p2, q1)) return true; 
    if (o2 == 0 && onSegment(p1, q2, q1)) return true; 
    if (o3 == 0 && onSegment(p2, p1, q2)) return true; 
    if (o4 == 0 && onSegment(p2, q1, q2)) return true; 

    return false;
} 

bool rect_line_intersect(rectangle@ r1, line l1) {
    point tl = point(r1.left(), r1.top());
    point tr = point(r1.right(), r1.top());
    point bl = point(r1.left(), r1.bottom());
    point br = point(r1.right(), r1.bottom());

    line t = line(tl, tr);
    line b = line(bl, br);
    line l = line(tl, bl);
    line r = line(tr, br);

    return line_line_intersect(t, l1) ||
           line_line_intersect(b, l1) ||
           line_line_intersect(l, l1) ||
           line_line_intersect(r, l1);
}

bool player_line_intersect(dustman@ player, line l) {
    rectangle@ r = player.collision_rect();
    r.left(r.left() + player.x());
    r.right(r.right() + player.x());
    r.top(r.top() + player.y());
    r.bottom(r.bottom() + player.y());
    return rect_line_intersect(r, l);
}
