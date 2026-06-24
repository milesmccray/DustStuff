enum Step
{
    Right,
    Neutral,
    Left,
    Wait,
}

class Attempt
{
    int right = 0;
    int neutral = 0;
    int left = 0;
    int wait = 0;

    private bool flipped = false;
    private Step step = Right;

    Attempt() {}

    /// Return true if this attempt is over.
    bool update(int x_intent)
    {
        if (flipped)
            x_intent = -x_intent;

        if (step == Right)
        {
            if (x_intent == 1) ++this.right;
            else if (x_intent == 0) step = Neutral;
            else if (x_intent == -1) step = Left;
            else return true;
        }

        if (step == Neutral)
        {
            if (x_intent == 0) ++this.neutral;
            else if (x_intent == -1) step = Left;
            else return true;
        }

        if (step == Left)
        {
            if (x_intent == -1) ++this.left;
            else if (x_intent == 0) step = Wait;
            else return true;
        }

        if (step == Wait)
        {
            if (x_intent == 0) ++this.wait;
            else return true;
        }

        return false;
    }

    void draw(scene@ g, float x, float y, float scale)
    {
        int offset = 0;
        g.draw_rectangle_world(
            20, 0,
            x + scale * offset, y,
            x + scale * (offset + right), y + scale,
            0, 0xFFFF0000);

        offset += right;
        g.draw_rectangle_world(
            20, 0,
            x + scale * offset, y,
            x + scale * (offset + neutral), y + scale,
            0, 0xFF00FF00);

        offset += neutral;
        g.draw_rectangle_world(
            20, 0,
            x + scale * offset, y,
            x + scale * (offset + left), y + scale,
            0, 0xFF0000FF);

        offset += left;
        g.draw_rectangle_world(
            20, 0,
            x + scale * offset, y,
            x + scale * (offset + wait), y + scale,
            0, 0xFF000000);
    }

    string to_string()
    {
        return right + ", " + neutral + ", " + left + ", " + wait;
    }
}

class script
{
    [position, mode: world, layer: 19, y: display_y] float display_x;
    [hidden] float display_y;

    [position, mode: world, layer: 19, y: upper_y] float upper_x;
    [hidden] float upper_y;
    [position, mode: world, layer: 19, y: lower_y] float lower_x;
    [hidden] float lower_y;

    scene@ g;
    controllable@ p;

    Attempt@ current_attempt;
    array<Attempt> old_attempts;

    script()
    {
        @g = get_scene();
    }

    void on_level_start()
    {
        @p = controller_controllable(0);
    }

    void update()
    {
        if (current_attempt !is null)
        {
             bool done = current_attempt.update(p.x_intent());
             if (done)
             {
                 puts(current_attempt.to_string());
                 old_attempts.insert(0, current_attempt);
                 @current_attempt = null;
             }
        }
    }

    void step(int)
    {
        update();
        if (current_attempt is null and p.x_intent() == 1)
        {
            @current_attempt = Attempt();
            update();
        }
    }

    void editor_draw(float)
    {
        g.draw_line_world(20, 0, display_x, display_y, display_x + 48, display_y, 2.0, 0x88888888);
        g.draw_line_world(20, 0, display_x, display_y, display_x, display_y - 48, 2.0, 0x88888888);
    }

    void draw(float)
    {
        if (current_attempt !is null)
            current_attempt.draw(g, display_x, display_y - 24.0, 24.0);
        for (int i = old_attempts.size() - 1; i >= 0; --i)
            old_attempts[i].draw(g, display_x, display_y - 24.0 * (i + 2), 24.0);

        g.draw_line_world(20, 0, display_x + 24.0 *  3, 0, display_x + 24.0 *  3, 1000.0, 1.0, 0xFFFF0000);
        g.draw_line_world(20, 0, display_x + 24.0 * 12, 0, display_x + 24.0 * 12, 1000.0, 1.0, 0xFF0000FF);
        g.draw_line_world(20, 0, display_x + 24.0 * 22, 0, display_x + 24.0 * 22, 1000.0, 1.0, 0xFF000000);
    }
}
