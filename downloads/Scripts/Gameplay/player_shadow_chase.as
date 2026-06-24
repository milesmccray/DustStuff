const float KILL_RADIUS = 24;
const int MAX_GHOSTS = 8; // safety cap for the editor array

class script
{
    scene@ g;
    controllable@ p;
    dustman@ dm;
    sprites@ spr;

    [hidden] bool first_time = true;
    [hidden] array<int> emitter_ids(8); // sized to MAX_GHOSTS so it persists regardless of ghost_count

    [text] bool shadows_kill = false;
    [text] int ghost_count = 3;
    [text] float spawn_delay = 1.0;
    [text] float ghost_stagger = 0.4;
    [text] float activation_delay = 2.0;
    [text] int startup_countdown_frames = 55; // frames before player has control on initial level load only

    [text] int draw_layer = 18;
    [text] int draw_sub_layer = 9;

    array<entity@> emitters;
    array<bool> ghost_spawned;
    array<bool> ghost_active;
    array<int> ghost_activation_counter;

    array<State> history;
    int frame_counter = 0;
    int countdown_remaining = 0;

    void on_level_start()
    {
        @g = get_scene();
        @p = controller_controllable(0);
        @dm = p.as_dustman();
        @spr = p.get_sprites();

        int count = clamp_count(ghost_count);

        emitters.resize(count);
        ghost_spawned.resize(count);
        ghost_active.resize(count);
        ghost_activation_counter.resize(count);

        if (first_time)
        {
            first_time = false;
            for (int i = 0; i < count; i++)
            {
                entity@ e = create_emitter(122, p.x(), p.y() - 48, 24, 96, draw_layer, draw_sub_layer);
                g.add_entity(e);
                @emitters[i] = e;
                emitter_ids[i] = e.id();
            }
        }
        else
        {
            for (int i = 0; i < count; i++)
            {
                @emitters[i] = entity_by_id(emitter_ids[i]);
            }
        }

        for (int i = 0; i < count; i++)
        {
            ghost_spawned[i] = false;
            ghost_active[i] = false;
            ghost_activation_counter[i] = 0;
        }

        history.resize(0);
        frame_counter = 0;

        // Only the initial level load has the player-control countdown.
        // Checkpoint respawns drop you straight back into control, so this
        // skip only applies here.
        countdown_remaining = startup_countdown_frames;
    }

    void checkpoint_load()
    {
        int count = clamp_count(ghost_count);

        // Re-fetch the player/dustman/sprites handles - these go stale
        // after a checkpoint load.
        @p = controller_controllable(0);
        @dm = (@p != null) ? p.as_dustman() : null;
        @spr = (@p != null) ? p.get_sprites() : null;

        // Re-fetch emitter handles - they may be stale after checkpoint load,
        // same as how player/dustman handles need requerying.
        for (int i = 0; i < count; i++)
        {
            @emitters[i] = entity_by_id(emitter_ids[i]);
        }

        for (int i = 0; i < count; i++)
        {
            ghost_spawned[i] = false;
            ghost_active[i] = false;
            ghost_activation_counter[i] = 0;
        }
        history.resize(0);
        frame_counter = 0;

        // No countdown on checkpoint respawn - player has control immediately.
        countdown_remaining = 0;
    }

    int clamp_count(int c)
    {
        if (c < 1) return 1;
        if (c > MAX_GHOSTS) return MAX_GHOSTS;
        return c;
    }

    int ghost_delay_frames(int ghost_index)
    {
        float delay_seconds = spawn_delay + ghost_stagger * float(ghost_index);
        return int(delay_seconds * 60);
    }

    void step(int)
    {
        if (p is null) return;

        if (countdown_remaining > 0)
        {
            countdown_remaining--;
            return;
        }

        State s;
        save_state(s);
        history.insertLast(s);
        frame_counter++;

        int count = clamp_count(ghost_count);

        for (int i = 0; i < count; i++)
        {
            int delay_frames = ghost_delay_frames(i);

            if (!ghost_spawned[i] && frame_counter >= delay_frames)
            {
                ghost_spawned[i] = true;
            }

            if (ghost_spawned[i] && !ghost_active[i])
            {
                ghost_activation_counter[i]++;
                if (ghost_activation_counter[i] >= int(activation_delay * 60))
                {
                    ghost_active[i] = true;
                }
            }

            int playback_index = ghost_spawned[i] ? (frame_counter - 1 - delay_frames) : -1;

            if (playback_index >= 0 && playback_index < int(history.length()))
            {
                if (@emitters[i] == null) continue;

                State@ s2 = history[playback_index];
                entity@ emitter = emitters[i];
                emitter.x(s2.x);
                emitter.y(s2.y - 48);
                varstruct@ vars = emitter.vars();
                vars.get_var("e_rotation").set_int32(int(s2.rotation));

                if (shadows_kill && ghost_active[i] && @dm != null && !dm.dead())
                {
                    float dx = s2.x - p.x();
                    float dy = s2.y - p.y();
                    float dist_sq = dx * dx + dy * dy;
                    if (dist_sq < KILL_RADIUS * KILL_RADIUS)
                    {
                        dm.kill(false);
                    }
                }
            }
        }
    }

    void draw(float sub_frame)
    {
        int count = clamp_count(ghost_count);

        for (int i = 0; i < count; i++)
        {
            if (!ghost_spawned[i]) continue;

            int delay_frames = ghost_delay_frames(i);
            int playback_index = frame_counter - 1 - delay_frames;

            if (playback_index < 0 || playback_index >= int(history.length())) continue;

            draw_state(history[playback_index], sub_frame, ghost_active[i]);
        }
    }

    void save_state(State@ s)
    {
        s.x = p.x();
        s.y = p.y();
        s.prev_x = p.prev_x();
        s.prev_y = p.prev_y();
        s.rotation = p.rotation();
        s.face = p.face();
        s.attack_face = p.attack_face();
        s.state_timer = int(p.state_timer());
        s.attack_timer = int(p.attack_timer());
        s.sprite_index = p.sprite_index();
        s.attack_sprite_index = p.attack_sprite_index();

        if (p.state() == 5) s.state_timer %= 4;
    }

    void draw_state(State@ s, float sub_frame, bool is_active)
    {
        float x = s.prev_x + sub_frame * (s.x - s.prev_x);
        float y = s.prev_y + sub_frame * (s.y - s.prev_y);
        float dist = pow(pow(x - p.x(), 2) + pow(y - p.y(), 2), 0.5);
        uint opacity = int(min(0xBB, floor(0xFF * dist / 150.0)));

        if (shadows_kill && !is_active)
        {
            opacity = uint(float(opacity) * 0.4);
        }

        uint colour = opacity << 24;

        if (s.attack_sprite_index == "")
            spr.draw_world(draw_layer, draw_sub_layer, s.sprite_index, s.state_timer, 1, x, y, s.rotation, s.face, 1, colour);
        else
            spr.draw_world(draw_layer, draw_sub_layer, s.attack_sprite_index, s.attack_timer, 1, x, y, s.rotation, s.attack_face, 1, colour);
    }
}

class State
{
    float x, y;
    float prev_x, prev_y;
    float rotation;
    int face, attack_face;
    int state_timer, attack_timer;
    string sprite_index, attack_sprite_index;
}

entity@ create_emitter(const int id, const float x, const float y, const int width, const int height, const int layer, const int sub_layer, const int rotation=0) {
    entity@ emitter = create_entity("entity_emitter");
    varstruct@ vars = emitter.vars();
    emitter.layer(layer);
    vars.get_var("emitter_id").set_int32(id);
    vars.get_var("width").set_int32(width);
    vars.get_var("height").set_int32(height);
    vars.get_var("draw_depth_sub").set_int32(sub_layer);
    vars.get_var("r_area").set_bool(true);
    vars.get_var("e_rotation").set_int32(rotation);
    emitter.set_xy(x, y);
    
    return emitter;
}