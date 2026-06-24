const dictionary V_ATTACK_DIRS = {
  { "groundstrikeu1", 1 },
  { "groundstrikeu2", 1 },
  { "groundstriked", -1 },
  { "groundstrike1", 0},
  { "groundstrike2", 0 },
  { "heavyu", 1 },
  { "heavyd", -1 },
  { "heavyf", 0 },

  { "airstriked1", -1 },
  { "airstriked2", -1},
  { "airheavyd", -1 }
};

class script : callback_base {
  [text] uint timer;
  dictionary enemies;
  dictionary bounced_enemies;
  scene@ g;
  int attack_dir = 2;
  bool play_impact_heavy = false;
  bool play_impact_light = false;
  bool callbacks_setup = false;
  uint count = 0;

  script() {
    @g = get_scene();
  }

  void step(int) {
    handle_hit_entities();
    if(play_impact_heavy) {
      play_effect("sfx_impact_heavy_1");
      play_effect("sfx_poly_heavy");
      play_impact_heavy = false;
    } else if(play_impact_light) {
      play_effect("sfx_poly_med");
      play_impact_light = false;
    }

    if(!callbacks_setup) {
      setup_callbacks();
    }
  }

  void entity_on_add(entity@ e) {
    if(e.type_name() == "filth_ball") {
      g.remove_entity(e);
    } else if(e.type_name() == "effect") {
      string spriteIndex = e.sprite_index();
      //For virtual characters, effects start with letter v
      int charLoc = spriteIndex.substr(0,1) == "v" ? 3 : 2;

      //Attempt to strip character specific characters off of sprite index
      string effect_name_suffix = spriteIndex.substr(charLoc);

      //Check if substring matches one of the attack direction sprites to determine attack direction
      if(V_ATTACK_DIRS.exists(effect_name_suffix)) {
        attack_dir = int(V_ATTACK_DIRS[effect_name_suffix]);
      }
    }
  }

  void play_effect(string sfx_name) {
    g.play_sound(sfx_name, 3, 0, 1, false, false);
  }

  //Call this function every step. This function traverses the dictionary of all corpses that should be flying,
  //handles stepping them, and handles cleaning them up
  void handle_hit_entities() {
    array<string> dict_keys = enemies.getKeys();

    for(uint i = 0; i < dict_keys.size(); i++) {
      flying_corpse@ f = cast<flying_corpse@>(enemies[dict_keys[i]]);
      int corpse_id = f.dead_c.as_entity().id();
      
      //If an enemy has collided with a wall, mark it for cleanup
      if(bounced_enemies.exists(""+corpse_id)) {
        bounced_enemies.delete(""+corpse_id);
        f.kill = true;
      }

      if(f.removed) {
        enemies.delete(dict_keys[i]);
        continue;
      }

      //We need to give one frame of leniency to get the attack direction
      if(!f.setup) {
        f.step();
        continue;
      }

      //Set what direciton the prism should go, wont work in multiplayer 100%
      if(!f.dirset && attack_dir != 2) {
        f.dirset = true;
        f.diry = attack_dir;
      }

      f.step();
    }
  }

  void entity_on_remove(entity@ e) {
    /** Called when an entity is removed from the scene. */
    if(enemies.exists(""+e.id())) {
       flying_corpse@ f = cast<flying_corpse@>(enemies[""+e.id()]);
       f.kill = true;
    }
  }

  void setup_callbacks() {
    for(uint i = 0; i < 4; i++) {
      dustman@ dm;
    
      if(@controller_entity(i) != null && @controller_entity(i).as_dustman() != null) {
        @dm = controller_entity(i).as_dustman();
      } else {
        continue;
      }
      dm.on_hit_callback(this, "player_hit_callback", i);
    }

    callbacks_setup = true;
  }

  void bounce_collision_callback(controllable@ ec, tilecollision@ tc, int side, bool moving, float snap_offset, int arg) {
    ec.check_collision(tc, side, moving, snap_offset);
    if(tc.hit() && ec.type_name() != "hittable_apple") {
      bounced_enemies[""+ec.id()] = true;
    }
  }

  void player_hit_callback(controllable@ attacker, controllable@ attacked, hitbox@ attack_hitbox, int arg) {
    if(@attacked == null || @attacked.as_hittable() == null) {
      return;
    }

    hittable@ h = attacked.as_hittable();

    if(h.life() <= attack_hitbox.damage() || attacked.type_name() == "hittable_apple") {
      //Store the hitbox damage as we will be setting it to 0 below. It is important
      //so the game knows the correct attack sound to play
      uint damage = attack_hitbox.damage();

      if(!enemies.exists(''+h.id())) {
        //Large prisms die automatically if the damage is 3, no matter what their life is
        attacked.set_collision_handler(this, "bounce_collision_callback", 0);

        //Tutorial enemies are weird, handle hardcoded damage/life values as a special case here. 
        //We need to basically do 0 damage and play the hit sounds ourselves.
        if(attacked.type_name() == "enemy_tutorial_hexagon") {
          play_impact_heavy = attack_hitbox.damage() == 3;
          play_impact_light = attack_hitbox.damage() == 1;
          attack_hitbox.damage(0);
        }

        //Set the enemy to have 99 life and put it in the dictionary of currently flying enemies
        h.life(99);
        flying_corpse@ f = flying_corpse(@attacked, timer, arg, attacker.attack_face(), damage);
        enemies[''+h.id()] = @f;
      }
    }
  }
}

class flying_corpse {
  scene@ g;
  controllable@ dead_c; //Handle to the dead controllable
  int timer = 0; //Timer which counts down how long until the enemy auto cleans
  uint player = 0; //Which player hit this corpse 
  float speed = 15; //Speed the enemy translates away
  float rotation_speed = 15; //How fast enemy should rotate when flying
  int dirx = 0; //-1 for left, 1 for right
  int diry  = 0; //1 for up -1 for down 0 for neither
  int damage = 1; //How much damage the killing blow was
  bool dead = false; //Flag used to state if the flying corpse has outlived its timer and should be cleaned one final time to get rid of it
  bool removed = false; //Flag used to help state if this corpse has had 1 frame to die in order for whatever datastructure holding these objects to have dead
                        //references removed
  bool dirset = false; //Flag used to check if this entity has its directions set, could have used enums for this but decided not to
  bool setup = false; //We need 1 frame after this entity dies in order for dustman's hit effects to spawn
  bool kill = false; //Set this flag to true whenever you want to explode the corpse
  bool offset = false; //Flag stating whether the flying corpse has been offset yet. This is done to alleviate some clipping

  /*arguments:
    c: the handle to the corpse that should be flying
    time: the time the corpse has before being cleansed
    player_: the player number that cleaned this enemy
    dirX_: the x direction the player hit this enemy
    damage_: the damage of the final blow to this enemy
   */
  flying_corpse(controllable@ c, uint time, uint player_, int dirX_, int damage_) {
    @dead_c = @c;
    timer = time;
    player = player_;
    @g = get_scene();
    dirx = dirX_;
    damage = damage_;
    dead = false;
    removed = false;
  }

  /* Each flying corpse needs to be stepped each frame */
  void step() {
    entity@ e = dead_c.as_entity();
    if(!setup) {
      setup = true;
      //Move entity slightly off surface to avoid clipping
    } else if(timer > 0 && !kill) {
      fly();
      timer--;
    } else if(!dead) {
      die();
      dead = true;
    } else {
      removed = true;
    }
  }

  /* Called when corpse needs to be rotated and translated.*/
  void fly() {
    entity@ e = dead_c.as_entity();
    if(!offset) {
      offset_corpse();
    }

    e.x(e.x() + (speed * dirx));
    dead_c.set_speed_xy(0,0);
    float delta_y = diry == 0 ? e.y() - speed/2: e.y() - (speed * diry);
    e.y(delta_y);
    e.rotation(e.rotation() + rotation_speed);
  }

  /* Call to mark corpse for cleanup and clean next frame */
  void die() {
    controllable@ c;
    if(@controller_entity(0) != null && @controller_entity(0).as_controllable() != null) {
      @c = controller_entity(0).as_controllable();
    } else {
      g.remove_entity(dead_c.as_entity());
      give_aircharges();
      return;
    }

    hittable@ h = @dead_c.as_hittable();
    //If for whatever reason the enemy is null, just give aircharges back and return
    if(@h == null) {
      give_aircharges();
      return;
    }
    
    //Do not spawn another hitbox on apples as they cant die
    if(dead_c.type_name() != "hittable_apple") {
      finish_enemy(c, h);
    }
  }

  /* Used to offset the corpse by some hand picked values to try and help alleviate clipping. Basically
   * we are moving the corpse away of the direction of travel. e.g if enemy was hit with an up-right heavy,
   * set the position to be a bit down left from the current position. Jank but kinda works. Could use raycasting probably to 
   * do this more precisely
   */
  void offset_corpse() {
    entity@ e = dead_c.as_entity();
    offset = true;
    if(diry != 0) {
      float newspeed = e.y() + (float(diry) * speed * 5);
      e.y(newspeed);
    }
    e.x(e.x() - (dirx * speed * 6.4));
  }

  /* Call to give player who did final blow to this enemy aircharges back when enemy dies */
  void give_aircharges() {
    if(@controller_controllable(player) != null) {
      if(@controller_controllable(player).as_dustman() != null) {
        dustman@ dm = controller_controllable(player).as_dustman();
        dm.dash(dm.dash_max());
      }
    }
  }

  /* Call to set the enemy's life to -1 and spawn a hitbox on it to finish it off */
  void finish_enemy(controllable@ c, hittable@ h) {
    h.life(-1);
    hitbox@ hb = create_hitbox(@c, 0, dead_c.x(), dead_c.y(), -1, 1, -1, 1);
    hb.damage(damage);
    g.add_entity(hb.as_entity());
    give_aircharges();
  }
}