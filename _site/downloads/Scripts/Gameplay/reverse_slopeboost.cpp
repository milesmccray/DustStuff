// all slope boosts are reverse slope boosts
class script {
  scene@ g;
  dustman@ player;

  bool reversed = false;
  int intent = 0;
  int face = 0;
  int speed_cutoff = 700;
  int not_dashed_frames = 0;

  script() {
    @g = get_scene();
  }

  void on_level_start() {
    controllable@ c = controller_controllable( 0 );
    if ( @c != null ) {
      @player = c.as_dustman();
    }
  }

  void checkpoint_load() {
    controllable@ c = controller_controllable( 0 );
    if ( @c != null ) {
      @player = c.as_dustman();
    }
  }

  void step( int entities ) {
    if ( reversed ) {
      // keep the player locked in their boost until they stop dashing and get
      // below a certain speed cutoff point
      player.x_intent( intent );
      player.face( face );

      if ( player.dash_intent() == 1 ) {
        not_dashed_frames = 0;
      }
      not_dashed_frames++;

      if ( not_dashed_frames > 18 || ( ( player.x_speed() >= -speed_cutoff ) && ( player.x_speed() <= speed_cutoff && player.ground_surface_angle() == 0 ) ) ) {
        // stop determining the player's intent when they stop boosting for some
        // longer amount of time
        reversed = false;
        not_dashed_frames = 0;
      }
    }

    if ( player.x_speed() >= -speed_cutoff && player.x_speed() <= speed_cutoff ) {
      // don't reverse unless the player actually tries to slope boost and has
      // some amount of speed
      return;
    }

    if ( !reversed && player.ground() && ( player.ground_surface_angle() == 45 || player.ground_surface_angle() == -45 ) ) {
      if ( player.dash_intent() == 1 ) {
        intent = -player.face();
        face = -player.face();

        player.x_intent( intent );
        player.face( face );

        player.set_speed_xy( -player.x_speed(), -player.y_speed() );
        reversed = true;
      }
    }
  }
}
