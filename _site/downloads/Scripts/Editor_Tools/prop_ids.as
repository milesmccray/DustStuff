class script {
    [position,mode:world,layer:19,y:box_t] float box_l;
    [hidden] float box_t;
    [position,mode:world,layer:19,y:box_b] float box_r;
    [hidden] float box_b;

    [text] bool run = false;
    [hidden] bool last_run = false;

    void editor_step() {
        if (run != last_run) {
            last_run = run;

            scene@ g = get_scene();
            int n = g.get_prop_collision(box_t, box_b, box_l, box_r);
            for (int i=0; i<n; ++i) {
                prop@ p = g.get_prop_collision_index(i);
                puts(formatInt(p.id()));
            }
        }
    }
}
