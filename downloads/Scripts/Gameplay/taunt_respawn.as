class script
{
    float x, y;

    void on_level_start()
    {
        controllable@ p = controller_controllable(0);
        x = p.x();
        y = p.y();
    }

    void step(int)
    {
        controllable@ p = controller_controllable(0);
        if (p.taunt_intent() > 0)
        {
            if (abs(x - p.x()) < 10000 and abs(y - p.y()) < 10000)
            {
                p.x(x);
                p.y(y);
            }
            else
            {
                float dx = x > p.x() ? 10000 : -10000;
                float dy = y > p.y() ? 10000 : -10000;
                p.x(p.x() + dx);
                p.y(p.y() + dy);
            }
        }
    }
}
