// minimap.cpp
// Scrolling minimap that follows the player and draws nearby tiles, enemies, and apples.

class script {

    scene@ g;

    [text] float map_x = 610;
    [text] float map_y = -320;
    [text] float map_w = 180;
    [text] float map_h = 180;
    [text|tooltip:'Controls zoom level. Recommended: 20-40. Expect lag over 100.'] int radius = 20;

    [colour, alpha] uint bg_colour      = 0xAA000000;
    [colour, alpha] uint tile_colour    = 0xAAFFFFFF;
    [colour, alpha] uint border_colour  = 0xFFAAAAAA;
    [colour, alpha] uint player_colour  = 0xFFFFFF00;
    [colour, alpha] uint enemy_colour   = 0xFFFF4444;
    [colour, alpha] uint apple_colour   = 0xFF44FF44;

    int diameter;
    float player_x = 0;
    float player_y = 0;
    int cache_x1 = 0;
    int cache_y1 = 0;
    array<bool> tile_cache;
    bool has_player = false;

    script() {
        @g = get_scene();
        diameter = radius * 2;
        tile_cache.resize(diameter * diameter);
    }

    void step(int entities) {
        diameter = radius * 2;
        if (int(tile_cache.length()) != diameter * diameter)
            tile_cache.resize(diameter * diameter);

        dustman@ dm = controller_controllable(0).as_dustman();
        if (@dm == null) return;

        player_x = dm.x();
        player_y = dm.y();
        has_player = true;

        cache_x1 = int(player_x / 48) - radius;
        cache_y1 = int(player_y / 48) - radius;

        for (int ty = 0; ty < diameter; ty++) {
            for (int tx = 0; tx < diameter; tx++) {
                tileinfo@ tile = g.get_tile(cache_x1 + tx, cache_y1 + ty, 19);
                tile_cache[ty * diameter + tx] = (@tile != null && tile.solid());
            }
        }
    }

    void draw(float sub_frame) {
        if (!has_player) return;

        float tile_w = map_w / float(diameter);
        float tile_h = map_h / float(diameter);
        float border = 2;

        // Border
        g.draw_rectangle_hud(22, 21,
            map_x - border, map_y - border,
            map_x + map_w + border, map_y + map_h + border,
            0, border_colour);

        // Background
        g.draw_rectangle_hud(22, 22, map_x, map_y, map_x + map_w, map_y + map_h, 0, bg_colour);

        // Tiles
        for (int ty = 0; ty < diameter; ty++) {
            for (int tx = 0; tx < diameter; tx++) {
                if (!tile_cache[ty * diameter + tx]) continue;
                float sx = map_x + tx * tile_w;
                float sy = map_y + ty * tile_h;
                g.draw_rectangle_hud(22, 23, sx, sy, sx + tile_w, sy + tile_h, 0, tile_colour);
            }
        }

        // Entities — skip players
        int count = g.get_entity_collision(-1e6, 1e6, -1e6, 1e6, 7);
        for (int i = 0; i < count; i++) {
            entity@ e = g.get_entity_collision_index(i);
            if (@e == null) continue;

            controllable@ c = e.as_controllable();
            if (@c != null && c.player_index() != -1) continue;

            float ex = map_x + (e.x() / 48.0 - cache_x1) / float(diameter) * map_w;
            float ey = map_y + (e.y() / 48.0 - cache_y1) / float(diameter) * map_h;
            if (ex < map_x || ex > map_x + map_w || ey < map_y || ey > map_y + map_h) continue;

            uint col = e.type_name() == "hittable_apple" ? apple_colour : enemy_colour;
            g.draw_rectangle_hud(22, 24, ex - 2, ey - 3, ex + 2, ey + 1, 0, col);
        }

        // Player dot at center
        float px = map_x + map_w * 0.5;
        float py = map_y + map_h * 0.5;
        g.draw_rectangle_hud(22, 25, px - 3, py - 4, px + 3, py + 2, 0, player_colour);
    }
}