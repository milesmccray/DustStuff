class script {
   scene@ g;
   fog_setting@ f;
   camera@ c;
   bool active = true;

  script() {
    @g = get_scene();
    @c = get_camera(0);
    @f = c.get_fog();
  }
  void step_post(int) {
    controllable@ p = controller_controllable(0);
    if (p is null) return; {
	f.layer_colour(22, 0xFF000000);
	f.layer_percent(22, 100.0f);   
    f.layer_colour(21, 0xFF000000);
	f.layer_percent(21, 100.0f);
    f.layer_colour(20, 0xFF000000);
	f.layer_percent(20, 100.0f);
    f.layer_colour(19, 0xFF000000);
	f.layer_percent(19, 100.0f);
	f.layer_colour(18, 0xFF000000);
	f.colour(18, 10, 0xFFFFFFFFFF); //player color
	f.layer_percent(18, 100.0f);
    f.layer_colour(17, 0xFF000000);
	f.layer_percent(17, 100.0f);
    f.layer_colour(16, 0xFF000000);
	f.layer_percent(16, 100.0f);
    f.layer_colour(15, 0xFF000000);
	f.layer_percent(15, 100.0f);	
	f.layer_colour(14, 0xFF000000);
	f.layer_percent(14, 100.0f);
    f.layer_colour(13, 0xFF000000);
	f.layer_percent(13, 100.0f);
    f.layer_colour(12, 0xFF000000);
	f.layer_percent(12, 100.0f);
    f.layer_colour(11, 0xFF000000);
	f.layer_percent(11, 100.0f);
    f.layer_colour(10, 0xFF000000);
	f.layer_percent(10, 100.0f);
    f.layer_colour(9, 0xFF000000);
	f.layer_percent(9, 100.0f);
    f.layer_colour(8, 0xFF000000);
	f.layer_percent(8, 100.0f);
    f.layer_colour(7, 0xFF000000);
	f.layer_percent(7, 100.0f);
    f.layer_colour(6, 0xFF000000);
	f.layer_percent(6, 100.0f);
    f.layer_colour(5, 0xFF000000);
	f.layer_percent(5, 100.0f);
    f.layer_colour(4, 0xFF000000);
	f.layer_percent(4, 100.0f);
    f.layer_colour(3, 0xFF000000);
	f.layer_percent(3, 100.0f);
    f.layer_colour(2, 0xFF000000);
	f.layer_percent(2, 100.0f);
    f.layer_colour(1, 0xFF000000);
	f.layer_percent(1, 100.0f);
    f.layer_colour(0, 0xFF000000);
	f.layer_percent(0, 100.0f);
	f.bg_top(0xFF000000);
	f.bg_mid(0xFF000000);
	f.bg_bot(0xFF000000);
    c.change_fog(f, 1);
	}
  }
 }