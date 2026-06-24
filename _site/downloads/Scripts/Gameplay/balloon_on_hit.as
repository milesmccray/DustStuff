const string EMBED_blow = "blow.ogg";
const string EMBED_deflate = "deflate.ogg";
const string EMBED_bloons = "bloons.ogg";
const float PI = 3.141592654;

class script : callback_base{
  scene@ g;

  array<bool> started(4);
  array<int> size(4);
  audio@ blowSound = null;
  audio@ deflateSound = null;
  int deflate_count = 0;
  // Track visibility override for default player sublayer (layer 18, sublayer 10)
  bool sublayer_hidden = false;

  // Inertial turning flight state
  array<float> heading(4);    // degrees, like dm.direction()
  array<float> ang_vel(4);    // angular velocity (deg/frame)

  // Tunable flight params (can be edited in editor if [text])
  [text] float move_accel = 0.15f;           // acceleration per frame while deflating
  [text] float move_max_speed = 7.0f;        // clamp max speed while deflating
  
  [text] float forward_speed = 4.0f;         // legacy constant speed (unused if accel enabled)
  [text] float move_start_speed = 2.5f;      // starting speed when deflation begins
  [text] float turn_accel_deg = 0.9f;        // deg/frame^2 when holding a turn key
  [text] float turn_decel_deg = 1.2f;        // passive deg/frame^2 decay toward 0 when no key
  [text] float max_turn_speed_deg = 7.0f;    // clamp on |ang_vel|
  [text] float reverse_brake_deg = 2.5f;     // extra deg/frame^2 applied when reversing turn direction
  [text] bool lock_gravity = true;           // if true, overrides natural falling
  [text] bool debug_logs = false;             // enable verbose logs to debug deflate flight
  [text] int debug_player = -1;              // -1: all players; else only index 0..3
  [text] bool use_polar_velocity = true;     // if true, use set_speed_direction during deflate instead of manual set_xy
  [text] float pivot_offset_y = -24.0f;      // adjust rotated overlay upward to simulate center pivot (tune per character)
  [text] float pivot_offset_x = 0.0f;        // horizontal pivot tweak if needed
  float max_deflate_audio = 1;
  float current_deflate_audio = 0;
  float deflate_audio_step = 0.01;
  float dir = 0;
  [text] float scale_step = .05;
  [text] float deflate_speed = 1;
  // Track if we've just entered deflating for per-player init
  array<bool> deflatingStarted(4);
  // Script-driven position while deflating to ignore engine gravity drift
  array<float> px(4);
  array<float> py(4);
  array<float> cur_speed(4); // per-player current speed

  script() {
    @g = get_scene();
  }

  float degtorad(float deg){
    return deg * (PI / 180.0f);
  }

  void build_sounds(message@ msg)
  {
    msg.set_string("blow", "blow");
    msg.set_string("deflate", "deflate");
    msg.set_string("bloons", "bloons");
  }

  void on_level_start(){
    // Ensure base player sublayer starts visible at level load
    g.sub_layer_visible(18, 10, true);
    sublayer_hidden = false;
    g.play_persistent_stream('bloons', 1, true, .5, true);
    for(uint i = 0; i < 4; i++) {

      started[i] = false;
      size[i] = 0;
      heading[i] = 0;
      ang_vel[i] = 0;
  deflatingStarted[i] = false;
      px[i] = 0;
      py[i] = 0;
  cur_speed[i] = 0;

      entity@ e = controller_entity(i);
      if (@e == null) continue;

      dustman@ dm = e.as_dustman();
      if (@dm == null) continue;

      dm.on_subframe_end_callback(this, "subframe_end", 0);
    }
  }

  //skill combo
  void step(int entities) {

    if(@blowSound != null && debug_logs) {
      // Global sound state (no player binding), printed only when debugging all
      
    }
  bool any_deflating = false; // Track if any player is currently in deflating phase
  for(uint i = 0; i < 4; i++) {
      entity@ e = controller_entity(i);
      if (@e == null) continue;

      dustman@ dm = e.as_dustman();
      if (@dm == null) continue;
      dm.combo_timer(100.0); //Prevent combo from decreasing
      if(dm.skill_combo() > 0 && !started[i]) {
        started[i] = true;
        size[i] = 0;
        // Initialize heading to current facing direction when balloon mode starts
        heading[i] = dm.direction();
        ang_vel[i] = 0;
      }

      //Scale dustman to combo size
      if(started[i]) {
        //puts("deflate_count: "+deflate_count+"");

        // (Turning controls only applied while deflating now)

        // Inflating
        if (@blowSound == null || (!blowSound.is_playing() && dm.skill_combo() > size[i])) {
          dir = dm.face();
          @blowSound = g.play_script_stream("blow", 3, 0, 0, false, 1.0);
          deflate_count = dm.skill_combo() * 1/scale_step;
          dm.skill_combo(deflate_count * scale_step);
          size[i] = dm.skill_combo();
          if(@deflateSound != null) deflateSound.stop();
          current_deflate_audio = 0;

            //puts("[balloon] p"+i+" inflate: combo="+dm.skill_combo()+" size="+size[i]);
        }

        // Deflating (apply inertial heading + manual position movement)
        else if(@blowSound != null && !blowSound.is_playing() && deflate_count > 0) {
          any_deflating = true; // Mark deflating active so we hide original draw sublayer
          // Initialise heading once at deflate start based on facing
          if(!deflatingStarted[i]){
            // Face 1 -> 0 deg (right), Face -1 -> 180 deg (left)
            heading[i] = (dm.face() == 1 ? 0.0f : 180.0f);
            ang_vel[i] = 0.0f;
            deflatingStarted[i] = true;
            // Seed script position from current player position
            px[i] = dm.x();
            py[i] = dm.y();

              //puts("[balloon] p"+i+" deflate start: face="+dm.face()+" heading="+heading[i]+" deflate_count="+deflate_count);
          }
          // --- Inertial turning controls (only while deflating) ---
          int xIn = dm.x_intent(); // -1 left, 1 right, 0 none
          float av_before = ang_vel[i];
          if(xIn < 0){
            if(ang_vel[i] > 0) ang_vel[i] -= reverse_brake_deg;
            ang_vel[i] -= turn_accel_deg;
          }else if(xIn > 0){
            if(ang_vel[i] < 0) ang_vel[i] += reverse_brake_deg;
            ang_vel[i] += turn_accel_deg;
          }else{
            if(ang_vel[i] > 0){
              ang_vel[i] -= turn_decel_deg;
              if(ang_vel[i] < 0) ang_vel[i] = 0;
            }else if(ang_vel[i] < 0){
              ang_vel[i] += turn_decel_deg;
              if(ang_vel[i] > 0) ang_vel[i] = 0;
            }
          }
          if(ang_vel[i] >  max_turn_speed_deg) ang_vel[i] =  max_turn_speed_deg;
          if(ang_vel[i] < -max_turn_speed_deg) ang_vel[i] = -max_turn_speed_deg;
          heading[i] += ang_vel[i];
          if(heading[i] > 180) heading[i] -= 360;
          else if(heading[i] < -180) heading[i] += 360;

          // Update speed with acceleration and clamp
          if(!deflatingStarted[i]){
            cur_speed[i] = move_start_speed;
          }else{
            cur_speed[i] += move_accel;
            if(cur_speed[i] > move_max_speed) cur_speed[i] = move_max_speed;
            if(cur_speed[i] < 0) cur_speed[i] = 0; // safety
          }

          float rad = degtorad(heading[i]);
          float dx = cos(rad) * cur_speed[i];
          float dy = sin(rad) * cur_speed[i];
          if(dx != 0) dm.face(dx > 0 ? 1 : -1);

          if(use_polar_velocity){
            int dir_int = int(round(heading[i] + 90.0f));
            if(dir_int > 180) dir_int -= 360;
            else if(dir_int <= -180) dir_int += 360;
            float speed_ps = cur_speed[i] * 60.0f; // convert per-frame speed to px/s
            dm.set_speed_direction(speed_ps, dir_int);
            if(debug_logs && (debug_player==-1 || int(i)==debug_player)){
              //puts("[balloon] p"+i+" deflating: xIn="+xIn+
                  // " ang_vel="+av_before+"->"+ang_vel[i]+" heading="+heading[i]+" dir="+dir_int+
                 //  " speed_ps="+(forward_speed*60.0f)+
                 //  " pos=("+dm.x()+","+dm.y()+") deflate_count="+deflate_count);
            }
          }else{
            // --- Collision-aware manual movement with sliding ---
            // Use script-maintained position to avoid engine gravity drift
            float start_x = px[i];
            float start_y = py[i];
            float target_x = start_x + dx;
            float target_y = start_y + dy;
            // Simple tile collision rays (cast horizontal and vertical separately)
            // Horizontal: if blocked, zero dx but keep dy so we slide vertically
            raycast@ rx = g.ray_cast_tiles(start_x, start_y, target_x, start_y, 1);
            bool hhit = false, vhit = false;
            if(rx.hit()) {
              target_x = start_x; // cancel horizontal move
              dx = 0;
              hhit = true;
            }
            // Vertical: cast using (possibly adjusted) x; if blocked, zero dy but keep dx so we slide horizontally
            raycast@ ry = g.ray_cast_tiles(target_x, start_y, target_x, target_y, 1);
            if(ry.hit()) {
              target_y = start_y; // cancel vertical move
              dy = 0;
              vhit = true;
            }
            // Apply position directly (bypassing velocity) and store back
            px[i] = target_x;
            py[i] = target_y;
            dm.set_xy(px[i], py[i]);
            float new_x = dm.x();
            float new_y = dm.y();
            if(debug_logs && (debug_player==-1 || int(i)==debug_player)){
            //  puts("[balloon] p"+i+" deflating: xIn="+xIn+
              //     " ang_vel="+av_before+"->"+ang_vel[i]+" heading="+heading[i]+" rad="+rad+
              //     " start=("+start_x+","+start_y+") target=("+target_x+","+target_y+") actual=("+new_x+","+new_y+")"+
              //     " dxdy=("+dx+","+dy+") H="+(hhit?"1":"0")+" V="+(vhit?"1":"0")+
              //     " deflate_count="+deflate_count);
            }
          }
          //puts("deflating");
          if (@deflateSound == null || !deflateSound.is_playing()) {
            @deflateSound = g.play_script_stream("deflate", 3, 0, 0, true, current_deflate_audio);
          } else if (current_deflate_audio < max_deflate_audio) {
            current_deflate_audio += deflate_audio_step;
            if (current_deflate_audio > max_deflate_audio) current_deflate_audio = max_deflate_audio;
            deflateSound.volume(current_deflate_audio);
          }
          dm.skill_combo(deflate_count * scale_step);
          
          size[i] = deflate_count * scale_step;
          deflate_count = deflate_count - deflate_speed > 0 ? deflate_count - deflate_speed : 0;
        } else if(@deflateSound != null) {
          // Reset deflate sound volume for next time
          current_deflate_audio = 0;
          deflateSound.stop();
          // Reset deflating flag so next deflation re-initialises heading
          deflatingStarted[i] = false;

        }
        dm.combo_count(deflate_count * scale_step);
        dm.scale((size[i] * scale_step) + 1, true);
      }
    } 

    // Toggle visibility of the original player sublayer while deflating to avoid double draw
    if(any_deflating){
      if(!sublayer_hidden){
        g.sub_layer_visible(18, 10, false);
        sublayer_hidden = true;
        //if(debug_logs && (debug_player==-1)) puts("[balloon] hide base player sublayer 18:10");
      }
    }else{
      if(sublayer_hidden){
        g.sub_layer_visible(18, 10, true);
        sublayer_hidden = false;
        //if(debug_logs && (debug_player==-1)) puts("[balloon] show base player sublayer 18:10");
      }
    }
  }

  // Ensure neutral state (state 0) is applied at subframe end while deflating
  void subframe_end(dustman@ dm, int a){
    // Minimal: while a player is in the deflating phase, force state 0.
    // We rely only on deflatingStarted[i] (set at deflate begin) and deflate_count>0.
    for(uint i = 0; i < 4; i++) {
      if(!deflatingStarted[i]) continue;
      if(deflate_count <= 0) continue;
      entity@ e = controller_entity(i);
      if(@e == null) continue;
      dustman@ dm = e.as_dustman();
      if(@dm == null) continue;
      //puts("SET STATE 0");
      dm.state(0);
    }
  }

  // Draw a second, rotated copy of the player while deflating so the head faces movement
  void draw(float sub_frame){
    for(uint i = 0; i < 4; i++){
      if(!deflatingStarted[i]) continue;
      if(deflate_count <= 0) continue;
      entity@ e = controller_entity(i);
      if(@e == null) continue;
      dustman@ dm = e.as_dustman();
      if(@dm == null) continue;

      sprites@ spr = dm.get_sprites();
      if(@spr == null) continue;

      // Use the current state sprite/frame; keep it simple (no attack special-casing)
      string sprite_name = dm.sprite_index();
      uint frame = uint(dm.state_timer() > 0 ? dm.state_timer() : 0);

      // Interpolate position for smooth sub-frame rendering
      const float x = dm.prev_x() + (dm.x() - dm.prev_x()) * sub_frame + dm.draw_offset_x();
      const float y = dm.prev_y() + (dm.y() - dm.prev_y()) * sub_frame + dm.draw_offset_y();

      // Rotate to face the inertial movement heading; don't flip horizontally for this copy
      const float rotation = heading[i];
      const float xscale = dm.scale();
      const float yscale = dm.scale();

      // Draw on a fixed layer/sublayer per request
      const int layer = 18;
      const int sublayer = 9;


      // Apply pivot compensation: shift draw origin so rotation appears around centre
      // Negative pivot_offset_y lifts the sprite because default pivot is bottom middle.
      spr.draw_world(layer, sublayer, sprite_name, frame, 0,
        x + pivot_offset_x,
        y - 48 * dm.scale(),
        rotation+90, xscale, yscale, 0xFFFFFFFF);
    }
  }
}
