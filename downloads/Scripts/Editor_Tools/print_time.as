class script
{
    uint now = timestamp_now();

    void update_now()
    {
        uint new_now = timestamp_now();
        if (new_now != now)
        {
            now = new_now;
            puts("" + now);
        }
    }

    void editor_step()
    {
        update_now();
    }

    void editor_draw(float)
    {
        update_now();
    }

    void step(int)
    {
        update_now();
    }

    void draw(float)
    {
        update_now();
    }
}
