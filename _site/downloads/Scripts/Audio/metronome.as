class script
{
    [position,mode:world,layer:19,y:y] float x;
    [hidden] float y;

    scene@ g;
    int timer = 0;
    bool left = false;

    script()
    {
        @g = get_scene();
    }

    void step(int)
    {
        if (--timer <= 0)
        {
            g.play_sound("sfx_hud_select_1", x, y, 1.0, false, true);
            left = not left;
            if (left) timer = 3;
            else timer = 19;
        }
    }
}
