#include "../lib/std.cpp"
#include "../lib/math/math.cpp"
#include "../lib/drawing/common.cpp"
#include "../lib/utils/colour.cpp"

// -- Replay-Compatible Randomness Framework --
class replay_rand_dummy : enemy_base {
  void init(script@, scriptenemy@ self) {
    self.auto_physics(false);
  }
}

class replay_rand {
  scene@ g;
	array<uint> encoders;
  uint seed;
  int ecx;
  int ecy;
  uint frame_counter;

  replay_rand() {
    @g = @get_scene();
  }

  void step() {
    frame_counter++;
  }
}


// -- Vector3 Class Definition --
class Vec3
{
    double x, y, z;

    Vec3() { x = 0; y = 0; z = 0; }
    Vec3(double x, double y, double z) { this.x = x; this.y = y; this.z = z; }
    Vec3(const Vec3 &in other) { x = other.x; y = other.y; z = other.z; }
    
    void set(double x, double y, double z) { this.x = x; this.y = y; this.z = z; }

    double magnitude() const { return sqrt(x*x + y*y + z*z); }
    double sqr_magnitude() const { return x*x + y*y + z*z; } 
    
    Vec3 normalize() const {
        double mag = magnitude();
        if (mag > 1e-9) { return Vec3(x / mag, y / mag, z / mag); }
        return Vec3(0,0,0);
    }

    Vec3 opAdd(const Vec3 &in other) const { return Vec3(x + other.x, y + other.y, z + other.z); }
    Vec3 opSub(const Vec3 &in other) const { return Vec3(x - other.x, y - other.y, z - other.z); }
    Vec3 opMul(double scalar) const { return Vec3(x * scalar, y * scalar, z * scalar); }
    Vec3& opAssign(const Vec3 &in other) { x = other.x; y = other.y; z = other.z; return this; }
};

// Function to linearly interpolate between two Vec3 points.
Vec3 lerp(const Vec3 &in v1, const Vec3 &in v2, double t) {
    return v1.opMul(1.0 - t).opAdd(v2.opMul(t));
}

// -- Quaternion Class Definition for 3D Rotation --
class Quaternion
{
    double w, x, y, z;

    Quaternion(double w=1, double x=0, double y=0, double z=0) { this.w = w; this.x = x; this.y = y; this.z = z; }
    
    Quaternion& opAssign(const Quaternion &in other) {
        w = other.w; x = other.x; y = other.y; z = other.z;
        return this;
    }

    Quaternion opMul(const Quaternion &in r) const {
        return Quaternion(
            w * r.w - x * r.x - y * r.y - z * r.z,
            w * r.x + x * r.w + y * r.z - z * r.y,
            w * r.y - x * r.z + y * r.w + z * r.x,
            w * r.z + x * r.y - y * r.x + z * r.w
        );
    }
    
    Quaternion opAdd(const Quaternion &in r) const { return Quaternion(w + r.w, x + r.x, y + r.y, z + r.z); }
    Quaternion opMul(double s) const { return Quaternion(w * s, x * s, y * s, z * s); }
    Quaternion opNeg() const { return Quaternion(-w, -x, -y, -z); }
    Quaternion inverse() const { return Quaternion(w, -x, -y, -z); }
    void normalize() {
        double mag = sqrt(w*w + x*x + y*y + z*z);
        if (mag > 1e-9) { w /= mag; x /= mag; y /= mag; z /= mag; }
    }
    
    Vec3 rotate(const Vec3 &in v) const {
        Quaternion p(0, v.x, v.y, v.z);
        Quaternion result = this.opMul(p).opMul(this.inverse());
        return Vec3(result.x, result.y, result.z);
    }
};

// -- Math & Projection Helpers --
double dot(const Vec3 &in v1, const Vec3 &in v2) { return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z; }
Vec3 cross(const Vec3 &in a, const Vec3 &in b) {
    return Vec3(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
}
double dot(const Quaternion &in q1, const Quaternion &in q2) {
    return q1.x * q2.x + q1.y * q2.y + q1.z * q2.z + q1.w * q2.w;
}

Quaternion slerp(Quaternion q1, Quaternion q2, double t) {
    double cos_omega = dot(q1, q2);
    if (cos_omega < 0.0) { q2 = q2.opNeg(); cos_omega = -cos_omega; }
    if (cos_omega > 0.9995) {
        Quaternion result = q1.opMul(1.0 - t).opAdd(q2.opMul(t));
        result.normalize(); return result;
    }
    double omega = acos(cos_omega), sin_omega = sin(omega);
    double scale1 = sin((1.0 - t) * omega) / sin_omega, scale2 = sin(t * omega) / sin_omega;
    return q1.opMul(scale1).opAdd(q2.opMul(scale2));
}

void project(const Vec3 &in p, const Quaternion &in view_orientation, const Vec3 &in camera_pos, double fov, float &out screen_x, float &out screen_y, float &out screen_scale) {
    Vec3 pos_in_cam_space = view_orientation.inverse().rotate(p - camera_pos);
    if (pos_in_cam_space.x < 0.1) { screen_scale = -1; return; }
    screen_scale = float(fov / pos_in_cam_space.x);
    screen_x = float(-pos_in_cam_space.y * screen_scale);
    screen_y = float(pos_in_cam_space.z * screen_scale); 
}

void draw_triangle_hud(scene@ g, uint layer, uint sub_layer, float x1, float y1, float x2, float y2, float x3, float y3, uint c1, uint c2, uint c3)
{
    g.draw_quad_hud(layer, sub_layer, false, x1, y1, x2, y2, x3, y3, x3, y3, c1, c2, c3, c3);
}

// -- Color Math Helpers --
double math_clamp(double val, double min_val, double max_val) {
    if (val < min_val) return min_val;
    if (val > max_val) return max_val;
    return val;
}

uint multiply_color(uint col, double factor) {
    int a = (col >> 24) & 0xFF;
    int r = int(double((col >> 16) & 0xFF) * factor);
    int g = int(double((col >> 8) & 0xFF) * factor);
    int b = int(double(col & 0xFF) * factor);
    
    r = (r > 255) ? 255 : ((r < 0) ? 0 : r);
    g = (g > 255) ? 255 : ((g < 0) ? 0 : g);
    b = (b > 255) ? 255 : ((b < 0) ? 0 : b);

    return (uint(a) << 24) | (uint(r) << 16) | (uint(g) << 8) | uint(b);
}

uint lerp_color(uint c1, uint c2, double t) {
    if (t <= 0) return c1;
    if (t >= 1) return c2;
    
    int a1 = (c1 >> 24) & 0xFF, r1 = (c1 >> 16) & 0xFF, g1 = (c1 >> 8) & 0xFF, b1 = c1 & 0xFF;
    int a2 = (c2 >> 24) & 0xFF, r2 = (c2 >> 16) & 0xFF, g2 = (c2 >> 8) & 0xFF, b2 = c2 & 0xFF;
    
    uint a = uint(a1 + (a2 - a1) * t);
    uint r = uint(r1 + (r2 - r1) * t);
    uint g = uint(g1 + (g2 - g1) * t);
    uint b = uint(b1 + (b2 - b1) * t);
    
    return (a << 24) | (r << 16) | (g << 8) | b;
}

// -- Render Queue Face Object --
class FaceData {
    Vec3 p1, p2, p3, p4;
    uint colour;
    double sort_distance;

    FaceData(const Vec3 &in p1, const Vec3 &in p2, const Vec3 &in p3, const Vec3 &in p4, uint colour, double sort_distance) {
        this.p1 = p1; this.p2 = p2; this.p3 = p3; this.p4 = p4;
        this.colour = colour;
        this.sort_distance = sort_distance;
    }
}

// QuickSort algorithm to depth-sort faces (Furthest to Nearest)
void sort_faces(array<FaceData@>@ arr, int left, int right) {
    if (left >= right) return;
    int i = left, j = right;
    double pivot_dist = arr[(left + right) / 2].sort_distance;

    while (i <= j) {
        while (arr[i].sort_distance > pivot_dist) i++; 
        while (arr[j].sort_distance < pivot_dist) j--;
        if (i <= j) {
            FaceData@ tmp = arr[i];
            @arr[i] = arr[j];
            @arr[j] = tmp;
            i++;
            j--;
        }
    }
    if (left < j) sort_faces(arr, left, j);
    if (i < right) sort_faces(arr, i, right);
}


// -- Sphere Controller Class --
class SphereController : enemy_base {
    scene@ g; scriptenemy@ self;
    Vec3 pos, prev_pos, vel, angular_velocity;
    Quaternion orientation, prev_orientation;
    
    double radius = 48.0, mass = 500.0, moment_of_inertia;
    double gravity = 980.0, input_torque = 2e7;
    double friction_coefficient = 0.7, restitution = 0.5;
    double linear_air_damping = 0.1, linear_angular_damping = 0.3;

    Vec3 camera_pos, prev_camera_pos;
    Quaternion camera_orientation, prev_camera_orientation;
    double camera_yaw = 0.0, camera_distance = 400.0, camera_height = 150.0;
    double camera_fov = 800.0, camera_smoothing = 0.08;

    double terrain_offset_x = 6283.0; 
    double terrain_offset_y = 0.0;

    Vec3 sun_dir = Vec3(0.5, 0.4, 1.0).normalize();
    uint fog_bg_color = 0xFF222233; 
    double fog_start = 1500.0;
    double fog_end = 4800.0; 

    // --- MEMORY POOLS & REUSED ARRAYS ---
    array<FaceData@> render_queue;
    int active_faces = 0;

    array<Vec3> world_verts(4);
    array<Vec3@> cam_verts(4);
    array<Vec3@> clipped_verts(8);
    array<float> sx(8);
    array<float> sy(8);

    SphereController() {
        @g = get_scene(); moment_of_inertia = (2.0/5.0) * mass * radius * radius;
    }

    void init(script@ s, scriptenemy@ self) {
        @this.self = self; self.auto_physics(false);
        pos.set(double(self.x()), double(self.y()), radius + 400); 
        prev_pos = pos;
        orientation.w = 1; prev_orientation = orientation;
        camera_yaw = -PI / 2.0; 
        update_camera();
        prev_camera_pos = camera_pos; prev_camera_orientation = camera_orientation;

        // Initialize reusable array memory pools
        for(int i = 0; i < 4; i++) { @cam_verts[i] = @Vec3(); }
        for(int i = 0; i < 8; i++) { @clipped_verts[i] = @Vec3(); }
        for(int i = 0; i < 2500; i++) {
            render_queue.insertLast(FaceData(Vec3(), Vec3(), Vec3(), Vec3(), 0, 0));
        }
    }

    double get_height(double x, double y) {
        double px = x + terrain_offset_x;
        double py = y + terrain_offset_y;

        return 800.0 * sin(px/4000.0) + 
               600.0 * cos(py/3500.0) + 
               300.0 * sin(px/1500.0) * cos(py/1500.0) - 200.0;
    }

    Vec3 get_normal(double x, double y) {
        double px = x + terrain_offset_x;
        double py = y + terrain_offset_y;

        double dz_dx = (800.0/4000.0) * cos(px/4000.0) + 
                       (300.0/1500.0) * cos(px/1500.0) * cos(py/1500.0);
                       
        double dz_dy = -(600.0/3500.0) * sin(py/3500.0) - 
                       (300.0/1500.0) * sin(px/1500.0) * sin(py/1500.0);
                       
        return Vec3(-dz_dx, -dz_dy, 1.0).normalize();
    }
    
    void handle_collisions(double dt) {
        double surface_z = get_height(pos.x, pos.y);
        Vec3 normal = get_normal(pos.x, pos.y);
        Vec3 surface_point(pos.x, pos.y, surface_z);
        
        double penetration = radius - dot(pos - surface_point, normal);

        if (penetration > 0) {
            pos = pos + normal * penetration;

            Vec3 contact_point_offset = normal * -radius;
            Vec3 contact_point_velocity = vel + cross(angular_velocity, contact_point_offset);
            double closing_vel = dot(contact_point_velocity, normal);
            
            if (closing_vel >= 0) return;

            double j = -(1.0 + restitution) * closing_vel;
            j /= (1.0 / mass) + dot(cross(cross(contact_point_offset, normal), contact_point_offset), normal) / moment_of_inertia;

            Vec3 impulse = normal * j;
            vel = vel + impulse * (1.0 / mass);
            angular_velocity = angular_velocity + cross(contact_point_offset, impulse) * (1.0 / moment_of_inertia);
            
            Vec3 tangent_vel = contact_point_velocity - normal * closing_vel;
            double tangent_speed = tangent_vel.magnitude();
            if(tangent_speed > 1e-9) {
                Vec3 friction_dir = tangent_vel * (-1.0 / tangent_speed);
                double friction_impulse_mag = j * friction_coefficient;
                Vec3 friction_impulse = friction_dir * friction_impulse_mag;
                vel = vel + friction_impulse * (1.0 / mass);
                angular_velocity = angular_velocity + cross(contact_point_offset, friction_impulse) * (1.0 / moment_of_inertia);
            }
        }
    }

    void step() {
        prev_pos = pos; prev_orientation = orientation; prev_camera_pos = camera_pos; prev_camera_orientation = camera_orientation;
        const double x_intent = self.x_intent(), y_intent = self.y_intent(), CAMERA_ROTATION_SPEED = 2.5;
        double yaw_input = 0;
        if (self.light_intent() > 0) yaw_input += CAMERA_ROTATION_SPEED;
        if (self.heavy_intent() > 0) yaw_input -= CAMERA_ROTATION_SPEED;
        const int substeps = 5; const double sub_dt = DT / substeps;

        for (int i = 0; i < substeps; i++) {
            camera_yaw += yaw_input * sub_dt;
            Vec3 cam_forward(cos(camera_yaw), sin(camera_yaw), 0), cam_right(sin(camera_yaw), -cos(camera_yaw), 0);
            Vec3 torque_input = cam_right * (y_intent * input_torque) + cam_forward * (x_intent * input_torque);
            Vec3 total_force = Vec3(0, 0, -gravity * mass);
            vel = vel * (1.0 - linear_air_damping * sub_dt);
            angular_velocity = angular_velocity * (1.0 - linear_angular_damping * sub_dt);
            Vec3 total_torque = torque_input;
            Vec3 angular_acceleration = total_torque * (1.0 / moment_of_inertia);
            angular_velocity = angular_velocity + angular_acceleration * sub_dt;
            Vec3 acceleration = total_force * (1.0 / mass);
            vel = vel + acceleration * sub_dt;
            pos = pos + vel * sub_dt;
            handle_collisions(sub_dt);
            double angle = angular_velocity.magnitude() * sub_dt;
            if(angle > 1e-9) {
                Vec3 axis = angular_velocity.normalize();
                Quaternion rot_delta(cos(angle/2.0), axis.x * sin(angle/2.0), axis.y * sin(angle/2.0), axis.z * sin(angle/2.0));
                orientation = rot_delta.opMul(orientation);
                orientation.normalize();
            }
        }
        self.set_xy(float(pos.x), float(pos.y)); update_camera();
    }
    
    void update_camera() {
        Vec3 base_offset(-camera_distance, 0, 0);
        Quaternion yaw_rotation(cos(camera_yaw / 2.0), 0, 0, sin(camera_yaw / 2.0));
        Vec3 final_offset = yaw_rotation.rotate(base_offset) + Vec3(0, 0, camera_height);
        Vec3 target_camera_pos = pos + final_offset;
        camera_pos = camera_pos * (1.0 - camera_smoothing) + target_camera_pos * camera_smoothing;
        Vec3 fwd = (pos - camera_pos).normalize(), left = cross(Vec3(0,0,1), fwd).normalize(), up = cross(fwd, left);
        double m00 = fwd.x, m01 = left.x, m02 = up.x, m10 = fwd.y, m11 = left.y, m12 = up.y, m20 = fwd.z, m21 = left.z, m22 = up.z;
        double tr = m00 + m11 + m22;
        if (tr > 0) { double S = sqrt(tr + 1.0) * 2; camera_orientation.w = 0.25 * S; camera_orientation.x = (m21 - m12) / S; camera_orientation.y = (m02 - m20) / S; camera_orientation.z = (m10 - m01) / S;
        } else if ((m00 > m11) && (m00 > m22)) { double S = sqrt(1.0 + m00 - m11 - m22) * 2; camera_orientation.w = (m21 - m12) / S; camera_orientation.x = 0.25 * S; camera_orientation.y = (m10 + m01) / S; camera_orientation.z = (m20 + m02) / S;
        } else if (m11 > m22) { double S = sqrt(1.0 + m11 - m00 - m22) * 2; camera_orientation.w = (m02 - m20) / S; camera_orientation.x = (m10 + m01) / S; camera_orientation.y = 0.25 * S; camera_orientation.z = (m21 + m12) / S;
        } else { double S = sqrt(1.0 + m22 - m00 - m11) * 2; camera_orientation.w = (m10 - m01) / S; camera_orientation.x = (m20 + m02) / S; camera_orientation.y = (m21 + m12) / S; camera_orientation.z = 0.25 * S; }
    }

    void draw(float sub_frame) {
        Vec3 interp_pos = lerp(prev_pos, pos, sub_frame);
        Quaternion interp_orientation = slerp(prev_orientation, orientation, sub_frame);
        Vec3 interp_camera_pos = lerp(prev_camera_pos, camera_pos, sub_frame);
        Quaternion interp_camera_orientation = slerp(prev_camera_orientation, camera_orientation, sub_frame);
        
        g.draw_quad_hud(0, 0, false, -1000, -1000, 1000, -1000, 1000, 1000, -1000, 1000, fog_bg_color, fog_bg_color, fog_bg_color, fog_bg_color);

        active_faces = 0; // Reset queue counter, reusing the objects safely
        draw_world(interp_camera_pos);
        draw_sphere(interp_pos, interp_orientation, interp_camera_pos);

        if (active_faces > 0) {
            sort_faces(@render_queue, 0, active_faces - 1);

            for (int i = 0; i < active_faces; i++) {
                FaceData@ face = render_queue[i];
                draw_clipped_face(face.p1, face.p2, face.p3, face.p4, face.colour, interp_camera_orientation, interp_camera_pos);
            }
        }
    }

    void draw_world(const Vec3 &in cam_pos) {
        const int draw_radius_cells = 20;
        const double tile_size = 250.0;
        int center_ix = floor_int(cam_pos.x / tile_size);
        int center_iy = floor_int(cam_pos.y / tile_size);

        for(int i = center_ix - draw_radius_cells; i < center_ix + draw_radius_cells; i++) {
            for(int j = center_iy - draw_radius_cells; j < center_iy + draw_radius_cells; j++) {
                double x = i * tile_size;
                double y = j * tile_size;

                Vec3 p1(x, y, get_height(x, y));
                Vec3 p2(x + tile_size, y, get_height(x + tile_size, y));
                Vec3 p3(x + tile_size, y + tile_size, get_height(x + tile_size, y + tile_size));
                Vec3 p4(x, y + tile_size, get_height(x, y + tile_size));
                
                Vec3 face_center = (p1 + p2 + p3 + p4) * 0.25;
                Vec3 view_dir = face_center - cam_pos;
                double dist = view_dir.magnitude();

                Vec3 normal = get_normal(face_center.x, face_center.y);

                if (dot(normal, view_dir) > 0) continue; 

                double light_intensity = max(0.15, dot(normal, sun_dir));
                uint base_colour = (i + j) % 2 == 0 ? 0xFF888888 : 0xFF999999;
                uint lit_colour = multiply_color(base_colour, light_intensity);

                double fog_t = math_clamp((dist - fog_start) / (fog_end - fog_start), 0.0, 1.0);
                uint final_colour = lerp_color(lit_colour, fog_bg_color, fog_t);

                double sort_dist = max(max((p1 - cam_pos).sqr_magnitude(), (p2 - cam_pos).sqr_magnitude()), 
                                       max((p3 - cam_pos).sqr_magnitude(), (p4 - cam_pos).sqr_magnitude()));

                if (active_faces < int(render_queue.length())) {
                    FaceData@ face = render_queue[active_faces];
                    face.p1 = p1; face.p2 = p2; face.p3 = p3; face.p4 = p4;
                    face.colour = final_colour;
                    face.sort_distance = sort_dist;
                    active_faces++;
                }
            }
        }
    }

    void draw_sphere(const Vec3 &in center, const Quaternion &in rotation, const Vec3 &in cam_pos) {
        const int lats = 10, longs = 10;
        
        for(int i = 0; i < lats; i++) {
            for(int j = 0; j < longs; j++) {
                double lat0 = PI*(-0.5+double(i)/lats), z0=sin(lat0), zr0=cos(lat0);
                double lat1 = PI*(-0.5+double(i+1)/lats), z1=sin(lat1), zr1=cos(lat1);
                double lng0 = 2*PI*double(j)/longs, x0=cos(lng0), y0=sin(lng0);
                double lng1 = 2*PI*double(j+1)/longs, x1=cos(lng1), y1=sin(lng1);
                
                Vec3 p1(x0*zr0,y0*zr0,z0), p2(x1*zr0,y1*zr0,z0), p3(x1*zr1,y1*zr1,z1), p4(x0*zr1,y0*zr1,z1);
                Vec3 face_center_local = (p1+p2+p3+p4)*0.25;
                Vec3 face_world_center = center + rotation.rotate(face_center_local * radius);
                
                Vec3 normal = rotation.rotate(face_center_local).normalize();
                Vec3 view_dir = face_world_center - cam_pos;
                double dist = view_dir.magnitude();

                if (dot(normal, view_dir) > 0) continue;

                double light_intensity = max(0.15, dot(normal, sun_dir));
                uint base_colour = (i+j)%2==0 ? 0xFFFFFFFF : 0xFFDDDDDD;
                uint lit_colour = multiply_color(base_colour, light_intensity);

                double fog_t = math_clamp((dist - fog_start) / (fog_end - fog_start), 0.0, 1.0);
                uint final_colour = lerp_color(lit_colour, fog_bg_color, fog_t);

                Vec3 world_p1 = center + rotation.rotate(p1 * radius);
                Vec3 world_p2 = center + rotation.rotate(p2 * radius);
                Vec3 world_p3 = center + rotation.rotate(p3 * radius);
                Vec3 world_p4 = center + rotation.rotate(p4 * radius);

                double sort_dist = max(max((world_p1 - cam_pos).sqr_magnitude(), (world_p2 - cam_pos).sqr_magnitude()), 
                                       max((world_p3 - cam_pos).sqr_magnitude(), (world_p4 - cam_pos).sqr_magnitude()));

                if (active_faces < int(render_queue.length())) {
                    FaceData@ face = render_queue[active_faces];
                    face.p1 = world_p1; face.p2 = world_p2; face.p3 = world_p3; face.p4 = world_p4;
                    face.colour = final_colour;
                    face.sort_distance = sort_dist;
                    active_faces++;
                }
            }
        }
    }
        
    void draw_clipped_face(const Vec3 &in p1, const Vec3 &in p2, const Vec3 &in p3, const Vec3 &in p4, uint colour, const Quaternion &in view_orientation, const Vec3 &in cam_pos) {
        // Reuse pre-allocated arrays to bypass GC
        world_verts[0] = p1; world_verts[1] = p2; world_verts[2] = p3; world_verts[3] = p4;
        
        Quaternion q_inv = view_orientation.inverse();
        for(uint i=0; i<4; i++) { 
            cam_verts[i] = q_inv.rotate(world_verts[i] - cam_pos); 
        }
        
        int clipped_count = 0;
        for (uint i=0; i<4; i++) {
            Vec3@ v1 = cam_verts[i]; 
            Vec3@ v2 = cam_verts[(i + 1) % 4];
            bool v1_inside = v1.x >= 0.1, v2_inside = v2.x >= 0.1;
            
            if (v1_inside != v2_inside) {
                double t = (0.1 - v1.x) / (v2.x - v1.x);
                clipped_verts[clipped_count].x = v1.x + t * (v2.x - v1.x);
                clipped_verts[clipped_count].y = v1.y + t * (v2.y - v1.y);
                clipped_verts[clipped_count].z = v1.z + t * (v2.z - v1.z);
                clipped_count++;
            } 
            if (v2_inside) { 
                clipped_verts[clipped_count].x = v2.x;
                clipped_verts[clipped_count].y = v2.y;
                clipped_verts[clipped_count].z = v2.z;
                clipped_count++;
            }
        }
        
        if(clipped_count < 3) return;
        
        for(int i=0; i<clipped_count; i++) {
            Vec3@ v = clipped_verts[i]; 
            float scale = float(camera_fov / v.x);
            sx[i] = float(-v.y * scale); 
            sy[i] = float(v.z * scale);
        }
        
        for(int i=1; i<clipped_count-1; i++) {
            draw_triangle_hud(g, 1, 1, sx[0], -sy[0], sx[i], -sy[i], sx[i+1], -sy[i+1], colour, colour, colour);
        }
    }
};

// -- Main Script Class --
class script {
    script() {}
    void spawn_player(message@ msg) {
        scriptenemy@ ent = create_scriptenemy(SphereController());
        ent.x(msg.get_float("x"));
        ent.y(msg.get_float("y"));
        msg.set_entity("player", @ent.as_entity());
    }
};