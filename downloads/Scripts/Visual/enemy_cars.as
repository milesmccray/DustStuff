class script : callback_base{
  scene@ g;
  [text] array<cars> enemies;
  [text] int knockback = 1;
  textfield@ txtf = null;

  script() {
    @g = get_scene();
    srand(timestamp_now());
  }

  void on_hit(controllable@ attacker, controllable@ attacked, hitbox@ attack_hitbox, int arg) {
    puts("HIT");
    attack_hitbox.attack_strength(knockback);
  }

  void on_level_start() {
    for(uint i = 0; i < enemies.size(); i++) {
      @enemies[i].p = create_prop();
      prop@ c = enemies[i].p;
      entity@ e = entity_by_id(enemies[i].id);
      //apple
      if(enemies[i].car_pallette == 3) {
        c.prop_set( 2 );
        c.prop_group( 5 );
        c.prop_index( 25 );
        c.palette( 0 );
      } else {
        c.prop_set( 3 );
        c.prop_group( 26 );
        c.prop_index( 2 );
        c.palette( enemies[i].car_pallette);
      }
      
      c.layer( 17 );
      c.sub_layer( 22 );
      c.scale_x( enemies[i].scale );
      c.scale_y( enemies[i].scale );
      g.add_prop(c);

      hittable @h = e.as_hittable();
      h.on_hit_callback(this, "on_hit", 0);
    }
  }

  void editor_draw(float sub_frame) {
    for(uint i = 0; i < enemies.size(); i++) {
      entity @e = entity_by_id(enemies[i].id);
      
      if(enemies[i].txt == null) {
        @enemies[i].txt = create_textfield();
      }
      
      enemies[i].txt.text(enemies[i].id+"");
      enemies[i].txt.colour(0xFF000000);
      enemies[i].txt.set_font("Caracteres", 36  );

      enemies[i].txt.draw_world(20, 20, e.x(), e.y()-50, 1, 1, 0);
      enemies[i].txt.colour(0xFFFFFFFF);
      enemies[i].txt.draw_world(20, 20, e.x()-2, e.y()-52, 1, 1, 0);
      g.draw_rectangle_world(20, 20, e.x()-20, e.y()-20, e.x()+20, e.y()+20, 0, 0xffffffff);
    }
  }

  void step(int entities) {
    g.sub_layer_visible(18, 8, false);
    g.sub_layer_visible(18, 2, false);
    // update prop positions
    for(uint i = 0; i < enemies.size(); i++) {
      entity@ e = entity_by_id(enemies[i].id);

      // Enemy died, remove prop
      if(@e == null) {
         continue;
      }

      enemies[i].p.x(e.x());
      
      hittable @h = e.as_hittable();

      if(@h == null) {
        continue;
      }

      if(enemies[i].car_pallette != 3) {
        hitbox@ hb = h.hitbox();

        enemies[i].p.y(e.y()-75);
        h.scale(2);
      } else {
        enemies[i].p.y(e.y()-10);
      }

      int face = h.x_speed() == 0 ? e.face() : abs(h.x_speed())/h.x_speed();
      enemies[i].p.scale_x(abs(enemies[i].p.scale_x()) * -face);
    }
  }
}
//2 5 25 apple
class cars {
  [entity] int id;
  [text] float scale = 1;
  [option,0:car1,1:car2,2:car3,3:apple]int car_pallette;
  textfield@ txt = null;
  prop@ p;

  cars() {

  }
}