class script {
  scene@ g;
  float time_in_level;
  float real_time;

  float delta_real;
  float delta_game;

  uint num_samples_real;
  uint num_samples_game;

  float total_real = 0;
  float total_game = 0;
  script() {
  }

  
  void on_level_start() {
    @g = get_scene();
    time_in_level = 0;
    real_time = 0;
    time_in_level = 0;
  }

  void step(int) {
    if(real_time == 0) {
      real_time = timestamp_now();
    }
    delta_real = timestamp_now() - real_time;
    delta_game = (g.time_in_level() - time_in_level)*1000;
    total_real += delta_real;
    total_game += delta_game;
    real_time = timestamp_now();
    time_in_level = g.time_in_level();
    puts("diff "+abs(delta_real-delta_game));
    num_samples_real++;
    num_samples_game++;
   // puts("average real "+(total_real/num_samples_real)+" average game "+(total_game/num_samples_game));
  }
}