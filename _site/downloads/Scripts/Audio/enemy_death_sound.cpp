// enemy_death_sound.cpp
// Plays a custom sound effect when specific entities are removed from the scene.

const string EMBED_sound1 = "sound1.ogg";
const string EMBED_sound2 = "sound2.ogg";
const string EMBED_sound3 = "sound3.ogg";
const string EMBED_sound4 = "sound4.ogg";
const string EMBED_sound5 = "sound5.ogg";

class EnemySoundEntry {
    [entity] int entity_id = 0;
    [text] string sound_name = "sound1";
}

class script : callback_base {

    scene@ g;

    [text] array<EnemySoundEntry> entries;
    array<bool> was_alive;
    array<float> last_x;
    array<float> last_y;
    bool initialised = false;

    script() {
        @g = get_scene();
    }

    void build_sounds(message@ msg) {
        msg.set_string("sound1", "sound1");
        msg.set_string("sound2", "sound2");
        msg.set_string("sound3", "sound3");
        msg.set_string("sound4", "sound4");
        msg.set_string("sound5", "sound5");
    }

    void init_arrays() {
        uint len = entries.length();
        was_alive.resize(len);
        last_x.resize(len);
        last_y.resize(len);
        for (uint i = 0; i < len; i++) {
            entity@ e = entity_by_id(entries[i].entity_id);
            was_alive[i] = (@e != null);
            last_x[i] = @e != null ? e.x() : 0;
            last_y[i] = @e != null ? e.y() : 0;
        }
        initialised = true;
    }

    void on_level_start() {
        init_arrays();
    }

    void step(int entities) {
        if (!initialised) init_arrays();
        if (entries.length() == 0) return;

        for (uint i = 0; i < entries.length(); i++) {
            if (i >= was_alive.length()) break;

            entity@ e = entity_by_id(entries[i].entity_id);
            bool alive = (@e != null);

            if (alive) {
                last_x[i] = e.x();
                last_y[i] = e.y();
            } else if (was_alive[i]) {
                // Just died — play sound at last known position
                g.play_script_stream(entries[i].sound_name, 0, last_x[i], last_y[i], false, 1.0);
                puts("enemy_death_sound: played " + entries[i].sound_name);
            }

            was_alive[i] = alive;
        }
    }

    void checkpoint_load() {
        init_arrays();
    }
}