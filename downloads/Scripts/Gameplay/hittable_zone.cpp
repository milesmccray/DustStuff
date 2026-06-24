// hittable_zone.cpp
// An invisible hittable zone. When hit gives an air charge (optional).
// Respawns after a delay. Does not count toward completion score.

class script : callback_base {

    scene@ g;

    script() {
        @g = get_scene();
    }

    void on_zone_hit(controllable@ attacked, controllable@ attacker, hitbox@ hb, int arg) {
        if (@attacker == null) return;
        dustman@ dm = attacker.as_dustman();
        if (@dm == null) return;
        dm.dash(dm.dash_max());
        puts("hittable_zone: gave air charge");
    }

    void build_sprites(message@ msg) {}
}

class HittableZone : enemy_base {

    scriptenemy@ self;
    scene@ g;
    script@ s;

    float spawn_x;
    float spawn_y;
    int respawn_timer = 0;
    int respawn_delay = 120;
    bool give_aircharge = true;
    bool is_dead = false;

    HittableZone() {}

    void init(script@ s, scriptenemy@ self) {
        @this.self = self;
        @this.s = s;
        @g = get_scene();

        spawn_x = self.x();
        spawn_y = self.y();

        // Never set life — keeps life_initial at 0 so it doesn't count toward SS
        self.auto_physics(false);

        self.as_hittable().on_hit_callback(s, "on_zone_hit", 0);
    }

    void step() {
        if (is_dead) {
            respawn_timer++;
            if (respawn_timer >= respawn_delay) {
                self.x(spawn_x);
                self.y(spawn_y);
                is_dead = false;
                respawn_timer = 0;
                puts("hittable_zone: respawned");
            }
        }
    }
}

class HittableZoneSpawner : trigger_base {

    scripttrigger@ self;
    bool spawned = false;

    [text] int respawn_delay = 120;
    [text] bool give_aircharge = true;

    HittableZoneSpawner() {}

    void init(script@ s, scripttrigger@ self) {
        @this.self = self;
    }

    void on_add() {
        if (spawned) return;

        HittableZone@ zone = HittableZone();
        zone.respawn_delay = respawn_delay;
        zone.give_aircharge = give_aircharge;

        scriptenemy@ enemy = create_scriptenemy(zone);
        enemy.x(self.x());
        enemy.y(self.y());
        get_scene().add_entity(enemy.as_entity());
        spawned = true;
        puts("hittable_zone: spawned at " + self.x() + ", " + self.y());
    }
}