// teleport_trigger.cpp
// A trigger zone that teleports any entity that enters it to a set destination.
//
// Install:
//   1. Place in %DUSTFORCE_USER%/script_src/
//   2. Attach to your level via the Script tab in the editor, set Level Type to Dustmod.
//   3. Place a script trigger in the editor and set its class to TeleportTrigger.
//   4. Set the destination x/y in the trigger properties.

class script : callback_base {
    void build_sprites(message@ msg) {}
}

class TeleportTrigger : trigger_base {

    scripttrigger@ self;

    [position, mode:world, layer:19, y:dest_y]
    float dest_x = 0;
    [hidden]
    float dest_y = 0;

    // How long to wait before the trigger can fire again (in frames)
    [text] int cooldown = 30;

    int cooldown_timer = 0;

    TeleportTrigger() {}

    void init(script@ s, scripttrigger@ self) {
        @this.self = self;
    }

    void step() {
        if (cooldown_timer > 0)
            cooldown_timer--;
    }

    void activate(controllable@ e) {
        if (cooldown_timer > 0)
            return;

        e.set_xy(dest_x, dest_y);
        e.set_speed_xy(0, 0);

        cooldown_timer = cooldown;

        puts("teleport_trigger: teleported " + e.type_name() + " to " + dest_x + ", " + dest_y);
    }
}
