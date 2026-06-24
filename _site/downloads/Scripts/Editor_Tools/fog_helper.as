#include "../lib/fog.as"

class script {
    [entity] int trigger_id;

    [color] int bg = 0x123abc;
    [color] int spikes = 0x456abc;
    [color] int tiles = 0x789abc;
    [color] int dust = 0xabcabc;

    [hidden] int last_bg = 0x123abc;
    [hidden] int last_spikes = 0x456abc;
    [hidden] int last_tiles = 0x789abc;
    [hidden] int last_dust = 0xabcabc;

    scene@ g;
    camera@ cam;

    script() {
        @g = get_scene();
        @cam = get_camera(0);
    }

    void editor_step() {
        if (bg != last_bg) {
            last_bg = bg;
            update();
        } else if (spikes != last_spikes) {
            last_spikes = spikes;
            update();
        } else if (tiles != last_tiles) {
            last_tiles = tiles;
            update();
        } else if (dust != last_dust) {
            last_dust = dust;
            update();
        }
    }

    void update() {
        entity@ trigger = entity_by_id(trigger_id);
        fog_setting@ fog;
        float fog_speed, trigger_size;
        get_fog_setting(trigger, fog, fog_speed, trigger_size);
        
        fog.stars_top(0);
        fog.stars_mid(0);
        fog.stars_bot(0);

        for (int i=0; i<=20; ++i) {
            fog.layer_percent(i, 1);
        }

        fog.bg_top(bg);
        fog.bg_mid(bg);
        fog.bg_bot(bg);

        fog.layer_colour(19, tiles);
        fog.colour(19, 15, spikes);
        fog.colour(19,  0, dust);
        fog.colour(19, 14, dust);
        fog.colour(19, 16, dust);
        fog.layer_colour(18, dust);

        set_fog_trigger(trigger, fog, fog_speed, trigger_size);
        cam.change_fog(fog, 0);
    }
}
