class trigger : base_trigger {
    script@ s;
    scripttrigger@ self;

    void init(script@ s, scripttrigger@ self) {
        @this.s = @s;
        @this.self = @self;
    }

    void activate(controllable@ c) {
        if (c.player_index() != -1) {

        }
    }
}
