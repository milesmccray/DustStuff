#include "../lib/std.cpp";

const int MAX_PLAYERS = 4;

class Box {
	float left;
	float top;
	float right;
	float bottom;

	Box( float left = 0, float top = 0, float right = 0, float bottom = 0 ) {
		this.left = left;
		this.top = top;
		this.right = right;
		this.bottom = bottom;
	}

	void set( float left, float top, float right, float bottom ) {
		this.left = left;
		this.top = top;
		this.right = right;
		this.bottom = bottom;
	}

	bool is_inside( float box_pos_x, float box_pos_y, float x, float y ) {
		return ( x >= box_pos_x + left ) &&
    ( x <= box_pos_x + right ) &&
    ( y >= box_pos_y + top ) &&
    ( y <= box_pos_y + bottom );
	}
}

class script {
  scene@ g;

  array<dustman@> players = {};

// CHANGE END CORDS HERE
  float level_end_x = -24123;
  float level_end_y = -8173;

  Box@ end_box = Box( -144, -144, 144, 144 );

  textfield@ editor_text;

  script() {
    @g = get_scene();

    @editor_text = create_textfield();
    editor_text.set_font( "ProximaNovaReg", 42 );
    editor_text.align_horizontal( 0 );
    editor_text.align_vertical( 0 );
    editor_text.text( "Selling end box coordinates\nfor 5 hearts on Atlas." );
    editor_text.colour( 0xFFFFFFFF );
  }

  void step( int entities ) {
    uint cams = num_cameras();
    for ( uint i = 0; i < cams; i++ ) {
      controllable@ c = controller_controllable( i );
      if ( @c == null ) {
        continue;
      }

      dustman@ dm = c.as_dustman();
      if ( @dm == null ) {
        continue;
      }

      bool broke = false;
      for ( uint j = 0; j < players.length; j++ ) {
        if ( players[ j ].is_same( dm ) ) {
          broke = true;
          break;
        }
      }
      if ( broke ) {
        continue;
      }

      players.insertLast( dm );
    }

    if ( players.length <= 0 ) {
      return;
    }

    for ( uint i = 0; i < players.length; i++ ) {
      if ( end_box.is_inside( level_end_x, level_end_y, players[ i ].x(), players[ i ].y() ) ) {
        g.end_level( level_end_x, level_end_y );
      }
    }
  }

  void editor_draw( float sub_frame ) {
    editor_text.draw_hud( 22, 22, 0, 200, 1.0, 1.0, 0 );
  }
}
