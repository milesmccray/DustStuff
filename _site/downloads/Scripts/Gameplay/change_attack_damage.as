class script : callback_base {
    
    array<camera@> players;
    int num_players;
    
    [text] int light_damage = 1;
    [text] int heavy_damage = 3;

    script() {
        num_players = num_cameras();
        players.resize(num_players);
        
        for(int i = 0; i < num_players; i++) {
            @players[i] = get_camera(i);
        }
    }
    
    void on_level_start() {
        initialize();
    }

    void checkpoint_load() {
        initialize();
    }

    void initialize() {
        for (int i = 0; i < num_players; i++) {
            players[i].puppet().as_controllable().on_hit_callback(this, "on_hit", 0);
        }        
    }

    void on_hit(controllable@ attacker, controllable@ attacked, hitbox@ attack_hitbox, int arg) {
        switch (attacker.attack_state()) {
            case 1:
                attack_hitbox.damage(light_damage);
                break;
            case 2:
                attack_hitbox.damage(heavy_damage);
                break;
        }
    }
}