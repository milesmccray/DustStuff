class EnemyEntry {
    [entity] int enemy_id;
    [text] float init_health = 100;
    [text] float start_scale = 2;
    [text] float end_scale = 0.75;
}

class script : callback_base {

    [text] array<EnemyEntry> enemies;

    array<entity@> enemy_entities;
    array<int> saved_life;
    bool apply_checkpoint = false;

    void on_level_start() {
        enemy_entities.resize(enemies.length());
        saved_life.resize(enemies.length());

        for (uint i = 0; i < enemies.length(); i++) {
            @enemy_entities[i] = entity_by_id(enemies[i].enemy_id);
            if (@enemy_entities[i] != null) {
                enemy_entities[i].as_controllable().life(int(enemies[i].init_health));
                saved_life[i] = int(enemies[i].init_health);
            }
        }
    }

    void step(int entities) {
        for (uint i = 0; i < enemies.length(); i++) {
            @enemy_entities[i] = entity_by_id(enemies[i].enemy_id);
            if (@enemy_entities[i] == null) continue;

            controllable@ c = enemy_entities[i].as_controllable();
            float health_ratio = float(c.life()) / enemies[i].init_health;
            float range = enemies[i].start_scale - enemies[i].end_scale;
            float new_scale = (health_ratio * health_ratio) * range + enemies[i].end_scale;
            c.scale(new_scale);
        }
    }

    void step_post(int entities) {
        if (!apply_checkpoint) return;

        for (uint i = 0; i < enemies.length(); i++) {
            @enemy_entities[i] = entity_by_id(enemies[i].enemy_id);
            if (@enemy_entities[i] == null) continue;
            enemy_entities[i].as_controllable().life(saved_life[i]);
        }
        apply_checkpoint = false;
    }

    void checkpoint_save() {
        for (uint i = 0; i < enemies.length(); i++) {
            if (@enemy_entities[i] != null)
                saved_life[i] = enemy_entities[i].as_controllable().life();
        }
    }

    void checkpoint_load() {
        apply_checkpoint = true;
    }
}
