#include "../lib/std.cpp";

class script : callback_base {
  scene@ g;

  controllable@ stoneboss;

  bool initialized = false;

  script() {
    @g = get_scene();
  }

  void step( int entities ) {
    if ( !initialized ) {
      for ( int i = 0; i < entities; i++ ) {
        entity@ e = entity_by_index( i );
        if ( @e is null ) {
          continue;
        }

        if ( e.type_name() == "enemy_stoneboss" ) {
          e.time_warp( 10 );
          @stoneboss = e.as_controllable();
          stoneboss.on_hurt_callback( this, "on_hurt", 0 );
        }
      }

      initialized = true;
    }
  }

  void on_hurt(controllable@ attacked, controllable@ attacker, hitbox@ attack_hitbox, int arg) {
    attacked.life( attacked.life() + attack_hitbox.damage() );
  }
}
