#include "../lib/triggers/EnterExitTrigger.cpp";

class script : trigger_base, EnterExitTrigger {
  scene@ g;

  bool active;

  [persistent] int letterbox_max_width = 50;

  int letterbox_width = 0;
  bool letterbox_drawn = false;
  bool overlay_disabled = false;

  script() {
    @g = get_scene();
  }

  void activate( controllable@ c ) {
    activate_enter_exit( c );
  }

  void step( int entities ) {
    step_enter_exit();
  }

  void draw( float sub_frame ) {
    if ( active ) {
      if ( !overlay_disabled ) {
        // disable the score overlay, since we're blocking it
        g.disable_score_overlay( true );
        overlay_disabled = true;
      }

      if ( !letterbox_drawn ) {
        uint increase = ( letterbox_max_width - letterbox_width ) / 7;

        if ( increase == 0 ) {
          increase = 1;
        }
        letterbox_width += increase;

        if ( letterbox_width >= letterbox_max_width ) {
          letterbox_drawn = true;
        }
      }

      int screen_radius = 450;

      // draw bars on the top and bottom side (for aesthetics)
      g.draw_rectangle_hud(
        20, 1, -800, -450, 1600, ( -screen_radius + letterbox_width ), 0, 0xFF000000
      );
      g.draw_rectangle_hud(
        20, 1, -800, 450, 1600, ( screen_radius - letterbox_width ), 0, 0xFF000000
      );
    }
  }

  bool can_trigger_enter_exit( controllable@ c ) {
		return ( c.player_index() != -1 );
	}

  void on_trigger_enter( controllable@ c ) {
    active = true;
  }
}
