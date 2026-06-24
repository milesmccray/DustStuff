#include "lib/std.cpp"

class script : callback_base {
    scene@ g;
    controllable@ p;
    dustman@ dm;

    script() {
        @g = get_scene();
    }

    void on_level_start() {
        @p = controller_controllable(0);
        if (p is null) return;

        @dm = p.as_dustman();
        if (dm is null) return;

        dm.on_subframe_end_callback(this, "on_subframe_end", 0);
    }

    void checkpoint_load() {
        @p = controller_controllable(0);
        if (p is null) return;

        @dm = p.as_dustman();
        if (dm is null) return;

        dm.on_subframe_end_callback(this, "on_subframe_end", 0);
    }

    void on_subframe_end(dustman@ dm, int arg) {

    }

    void step(int entities) {

    }
}
