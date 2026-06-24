class script {
  scene @g;

  controllable@ player;

  int frames = 0;

  script() {
    @g = get_scene();
  }

  void step( int entities ) {
    if ( @player == null ) {
      @player = controller_controllable( 0 );
    }

    if ( frames % 30 == 0 ) {
      for ( int i = 1; i < entities; i++ ) {
        entity@ e = entity_by_index( i );

        if ( @e != null ) {
          if ( e.is_same( @player ) ) {
            continue;
          }

          if ( e.type_name() == "enemy_tutorial_hexagon" || e.type_name() == "enemy_tutorial_square" ) {
            entity@ barrel = create_entity( "enemy_slime_barrel" );

            barrel.x( e.x() );
            barrel.y( e.y() );
            g.add_entity( barrel );
            g.remove_entity( e );

            entity@ ai = create_entity("AI_controller");
            ai.x(barrel.x());
            ai.y(barrel.y());

            varstruct@ vars = ai.vars();
            vars.get_var("puppet_id").set_int32(barrel.id());
            vararray@ nodes = vars.get_var("nodes").get_array();
            vararray@ nodes_wait_time = vars.get_var("nodes_wait_time").get_array();

            nodes.resize(2);
            nodes_wait_time.resize(2);
            nodes.at(0).set_vec2(barrel.x(), barrel.y());
            nodes_wait_time.at(0).set_int32(0);


            nodes.at(1).set_vec2(barrel.x() + 100, barrel.y() );
            nodes_wait_time.at(1).set_int32(0);

            g.add_entity( ai );
          }
        }
      }
    }

    frames++;
  }
}
