class script
{
    bool in_game = false;
    array<Door@> doors;
    scene@ g;
    
    script()
    {
        @g = get_scene();
    }
    
    void on_level_start()
    {
        in_game = true;
    }
    
    void step(int entities)
    {
        if(!in_game) return;
        for(uint i = 0; i < doors.length(); i++)
        {
            doors[i].check();
        }
    }
}

class Door : trigger_base
{
    scene@ g;
    script@ s;
    scripttrigger@ self;
    
    [position,mode:world,layer:19,y:tile_y1] float tile_x1 = 0;
    [hidden] float tile_y1 = 0;
    [position,mode:world,layer:19,y:tile_y2] float tile_x2 = 0;
    [hidden] float tile_y2 = 0;
    
    Door()
    {
        @g = get_scene();
    }
    
    void init(script@ s, scripttrigger@ self)
    {
        @this.s = @s;
        @this.self = @self;
        self.square(true);
        self.radius(100);
        s.doors.insertLast(this);
    }
    
    void editor_init(script@ s, scripttrigger@ self)
    {
        @this.self = @self;
        self.square(true);
        self.radius(100);
    }
    
    void check()
    {
        if(@self == null || self.as_entity().destroyed()) return;
        
        float r = self.radius();
        int count = g.get_entity_collision(
            self.y() - r, self.y() + r,
            self.x() - r, self.x() + r, 7);
        
        for(int i = 0; i < count; i++)
        {
            entity@ e = g.get_entity_collision_index(i);
            if(@e == null || e.type_name() != 'hittable_apple') continue;
            
            int x1 = min(int(floor(tile_x1 / 48)), int(floor(tile_x2 / 48)));
            int y1 = min(int(floor(tile_y1 / 48)), int(floor(tile_y2 / 48)));
            int x2 = max(int(floor(tile_x1 / 48)), int(floor(tile_x2 / 48)));
            int y2 = max(int(floor(tile_y1 / 48)), int(floor(tile_y2 / 48)));
            
            for(int x = x1; x <= x2; x++)
            {
                for(int y = y1; y <= y2; y++)
                {
                    g.set_tile(x, y, 19, false, 0, 0, 0, 0);
                }
            }
            
            g.remove_entity(e);
            g.remove_entity(self.as_entity());
            return;
        }
    }
    
    void editor_draw(float sub_frame)
    {
        // Draw the trigger zone
        g.draw_rectangle_world(21, 21,
            self.x() - self.radius(), self.y() - self.radius(),
            self.x() + self.radius(), self.y() + self.radius(),
            0, 0x4400FF00);
        
        // Draw the tile deletion zone
        float x1 = min(tile_x1, tile_x2);
        float y1 = min(tile_y1, tile_y2);
        float x2 = max(tile_x1, tile_x2);
        float y2 = max(tile_y1, tile_y2);
        x1 = floor(x1 / 48) * 48;
        y1 = floor(y1 / 48) * 48;
        x2 = floor(x2 / 48) * 48 + 48;
        y2 = floor(y2 / 48) * 48 + 48;
        g.draw_rectangle_world(21, 22, x1, y1, x2, y2, 0, 0x44FF0000);
    }
}