class script : callback_base {
  scene@ g;

  dustman@ player;

  bool handler_set = false;

  script() {
    @g = get_scene();
  }

  void on_level_start() {
    controllable@ c = controller_controllable(0);
    if (@c != null) {
      @player = c.as_dustman();
    }
  }

  void checkpoint_load() {
    handler_set = false;

    controllable@ c = controller_controllable(0);
    if (@c != null) {
      @player = c.as_dustman();
    }
  }

  void step(int entities) {
    if (@player != null && !handler_set) {
      player.set_collision_handler(this, "collision_handler", 0);
      handler_set = true;
    }
  }

  void collision_handler(controllable@ ec, tilecollision@ tc, int side, bool moving, float snap_offset, int arg) {
    tc.hit(false);
  }
}
