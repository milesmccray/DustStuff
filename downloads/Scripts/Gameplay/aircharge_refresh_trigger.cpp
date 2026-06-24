class script {}

class AirchargeRefresher : trigger_base {
  dustman@ player;

  bool activated = false;
  bool active_this_frame = false;

  bool aircharges_refreshed = false;

  [persistent] int amount = 1;

	AirchargeRefresher() {}

  void step() {
    if ( @player == null ) {
      controllable@ c = controller_controllable( 0 );
      if ( @c != null ) {
        @player = c.as_dustman();
      }
    }

    if ( @player == null ) {
      return;
    }

    if ( activated ) {
      if ( not active_this_frame ) {
        activated = false;
      }
      active_this_frame = false;
    }
  }

	void activate( controllable@ e ) {
    if ( e.player_index() == 0 ) {
      if ( not activated ) {
        activated = true;

        player.dash( amount );
      }
      active_this_frame = true;
    }
	}
}
