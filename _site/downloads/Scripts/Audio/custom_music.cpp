const string EMBED_music = "music.ogg";


class script
{
  
  scene@ g;
  audio@ music1;
  //audio@ music2;
  
  script()
	{
    @g = get_scene();
  }
  
  void build_sounds(message@ msg)
  {
    msg.set_string("music", "music");
    //msg.set_int("music|loop", 3985302); // 90.36935 samples * 44100khz
  }
  
  void on_level_start()
  {
    @music1 = g.play_persistent_stream('music', 1, true, .75, true);
  }
}