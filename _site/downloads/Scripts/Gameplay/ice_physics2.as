class script {}

class edge_trigger : trigger_base {
    bool activated;
    bool active_this_frame;
    controllable@ trigger_entity;
    scripttrigger@ self;
    script@ s;

    controllable@ p;
    dustman@ d;

    void init(script@ s, scripttrigger@ self) {
        activated = false;
        active_this_frame = false;
        @this.s = @s;
        @this.self = @self;

        @p = controller_controllable(0);
        @d = p.as_dustman();
    }
    
    void rising_edge(controllable@ e) {
        @trigger_entity = @e;
    }

    void falling_edge(controllable@ e) {
        @trigger_entity = null;

        d.idle_fric(1728);
        d.skid_fric(1152);
        d.land_fric(1728);
        d.roof_fric(1000);
        d.run_start(299);
    }
    
    void step() {
        if (activated) {
            if (not active_this_frame) {
                activated = false;
                falling_edge(@trigger_entity);
            }
            active_this_frame = false;
        }
    }
    
    void activate(controllable@ e) {
        if (e.player_index() == 0) {
            @trigger_entity = @e;
            if (not activated) {
                rising_edge(@e);
                activated = true;
            }
            active_this_frame = true;

            d.idle_fric(0);
            d.land_fric(0);
            d.skid_threshold(0);
            if (p.ground() and p.state() != 10) {
                d.run_start(100);
                d.skid_fric(2500);
                if (p.x_speed() > 0 and p.x_intent() != 1) {
                    if (p.x_intent() == -1) d.skid_fric(3500);
                    p.state(3);
                }
                if (p.x_speed() < -200 and p.x_intent() != -1) {
                    if (p.x_intent() == 1) d.skid_fric(3500);
                    p.state(3);
                }
            }

            if (not p.ground()) {
                d.run_start(299);
            }
        }
    }
}
