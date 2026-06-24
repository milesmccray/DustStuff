class script {}

class trigger : trigger_base, callback_base {
    [entity] int enemy;
    [text] string event;

    controllable@ c;

    scene@ g;
    script@ s;
    scripttrigger@ self;

    void init(script@ s, scripttrigger@ self) {
        @g = get_scene();
        @this.s = s;
        @this.self = self;

        entity@ e = entity_by_id(enemy);
        if (e !is null) {
            @c = e.as_controllable();
            if (c !is null) {
                c.on_hurt_callback(this, "on_hurt_callback", 0);
            }
        }
    }

    void on_hurt_callback(
        controllable@ attacker,
        controllable@ attacked,
        hitbox@ attack_hitbox,
        int arg
    ) {
        message@ msg = create_message();

        msg.set_int("entity_id", attacked.id());

        broadcast_message(event, msg);
    }

    void editor_draw(float sub_frame) {
        entity@ e = entity_by_id(enemy);
        if (e !is null) {
            g.draw_line_world(22, 22, self.x(), self.y(), e.x(), e.y(), 4, 0xFFFF0000);
        }
    }
}
