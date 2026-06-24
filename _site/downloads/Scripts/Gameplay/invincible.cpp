class script {
  scene@ g;

  dustman@ player;

  script() {
    @g = get_scene();
  }

  void on_level_start() {
    controllable@ c = controller_controllable( 0 );
    if ( @c != null ) {
      @player = c.as_dustman();
      player.dead(true);
    }
  }

  void checkpoint_load() {
    controllable@ c = controller_controllable( 0 );
    if ( @c != null ) {
      @player = c.as_dustman();
      player.dead(true);
    }
  }
}
