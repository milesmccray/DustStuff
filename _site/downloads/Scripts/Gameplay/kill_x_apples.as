class script {
    scene@ g;
    bool finished;
    int apples;
    int apples_cp;
    textfield@ debug_apples;
    
    [text] int total_apples = 20;
    
    script() {
        @g = get_scene();
        finished = false;
        apples = 0;
        apples_cp = 0;
        @debug_apples = @create_textfield();
        debug_apples.set_font("Caracteres", 36);
    }
    void entity_on_remove(entity@ e) {
        if (e.type_name() == "hittable_apple")
            apples++;
    }
    void checkpoint_save() {
        apples_cp = apples;
    }
    void checkpoint_load() {
        apples = apples_cp;
    }
    void step(int entities) {
        if (finished)
            return;
            // remove level end entities if they exist
        for (int i = 0; i < entities; i++) {
            entity@ e = entity_by_index(i);
            if (@e == null)
                continue;
            if (e.type_name() == "level_end" || e.type_name() == "level_end_prox")
                g.remove_entity(e);
        }
        if (apples >= total_apples) {
            finished = true;
            float x = 0;
            float y = 0;
            controllable@ c = controller_controllable(0);
            if (@c != null) {
                dustman@ dm = c.as_dustman();
                if (@dm != null) {
                    x = dm.x();
                    y = dm.y();
                }
            }
            g.end_level(x, y);
        }
        // debug_apples.text("A: " + apples + " -- C: " + apples_cp);
    }
    void draw(float sf) {
        debug_apples.draw_hud(0, 0, 0, 400, 1, 1, 0);
    }
}