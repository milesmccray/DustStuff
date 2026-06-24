class script {
  scene@ g;

  float flip_timer = 0;
  [text] float flip_speed = 10; 
  uint prev_state = 1000;
  uint prev_prev_state = 1000;
  bool flipping = false;

  script() {
    @g = get_scene();
  }

  void step(int entities) {
    entity@ p = controller_entity(0);
    if (@p == null) return;

    dustman@ dm = p.as_dustman();
    if (@dm == null) return;

    prev_prev_state = prev_state;
    prev_state = dm.state();

    // Rotate player while flipping
    if (flipping) {
      flip_timer += flip_speed;
      p.rotation(-flip_timer);
    }

    // Stop flip after full rotation
    if (flip_timer >= 360) {
      puts("STOP");
      flip_timer = 0;
      flipping = false;
    }
  }

  void step_post(int entities) {
    entity@ p = controller_entity(0);
    if (@p == null) return;

    dustman@ dm = p.as_dustman();
    if (@dm == null) return;

    if (prev_state == 1000) return;

    bool was_attack =
      (prev_state == 11 || prev_state == 12 || prev_state == 13) ||
      ((prev_prev_state == 11 || prev_prev_state == 12 || prev_prev_state == 13) && prev_state == 10);

    bool is_valid_start =
      (dm.state() == 8 || dm.state() == 7 || dm.state() == 5);

    if (!flipping && was_attack) {
      if (is_valid_start && flip_timer < 360) {
        flipping = true;
        puts("FLIPPING");
      }
    }
  }
}