class script {
  scene @g;

  int frames = 0;

  script() {
    @g = get_scene();
  }

  void step( int entities ) {
    if ( frames % 60 == 0 ) {
      for ( int i = 1; i < entities; i++ ) {
        hittable@ h = entity_by_index( i ).as_hittable();

        if ( @h != null ) {
          if ( h.is_same( controller_controllable( 0 ).as_dustman() ) ) {
            continue;
          }

          if ( h.type_name() == "enemy_trash_bag" || h.type_name() == "enemy_trash_can" ) {
            continue;
          }

          h.team( 1 );
        }
      }
    }
  }
}
