class script : callback_base {
  scene@ g;

  dustman@ player;

  [persistent] bool infinite_combo = false;
  [persistent] bool keep_combo_on_death = false;

  int timer = 0;

  int combo = 0;
  int spawn_timer = 0;
  bool checkpoint_loaded = false;

  int refresh_combo_interval = 60;

  script() {
    @g = get_scene();
  }

  void on_level_start() {
    controllable@ c = controller_controllable( 0 );
    if ( @c != null ) {
      @player = c.as_dustman();
    }
  }

  void checkpoint_save() {
    combo = player.combo_count();
  }

  void checkpoint_load() {
    controllable@ c = controller_controllable( 0 );
    if ( @c != null ) {
      @player = c.as_dustman();
    }

    checkpoint_loaded = true;
  }

  void step( int entities ) {
    timer++;

    if (timer % refresh_combo_interval == 0) {
      player.combo_timer(1);
    }

    // set the combo timer manually as just setting `combo_count()` makes
    // the timer immediately blink from the start; delay this to account for
    // the fade-in before the player gains control on checkpoint load
    if (keep_combo_on_death && checkpoint_loaded) {
      if (spawn_timer <= 4 || combo <= 0) {
        spawn_timer++;
        return;
      }

      player.combo_count(combo);
      player.skill_combo(combo);
      player.combo_timer(1.0);
      g.combo_break_count(g.combo_break_count()-1);
      spawn_timer = 0;
      checkpoint_loaded = false;
    }
  }
}
