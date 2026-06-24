class script
{
  scene@ g;
  [text] array<myClassNameHere> enemies;
  entity@ enemy;
    script() // Ran when your script is initialized
    {
      @g = get_scene();
    }

    void on_level_start() // ran on level start
    {
      for(uint i = 0; i < enemies.size(); i++) {
        @enemy = entity_by_id( enemies[i].enemy_id );
        if(@enemy == null) continue;
        enemy.time_warp( enemies[i].speed_id );
      }
    }

    void checkpoint_load() // ran when a checkpoint is loaded
    {
      for(uint i = 0; i < enemies.size(); i++) {
        @enemy = entity_by_id( enemies[i].enemy_id );
        if(@enemy == null) continue;
        enemy.time_warp( enemies[i].speed_id );
      }
    }

    void step(int) // ran every physics step (60 times per second)
    {

    }
}

class myClassNameHere {
  [entity] int enemy_id;
  [text] float speed_id = 10;
  myClassNameHere() {

  }
  //Put whatever functions you want here
}