// ============================================================
// Entity Diagnostic Script
// Dumps extensive info about all entities near the camera.
// Attach as a script trigger to any map.
// ============================================================

class script {
    scene@ g;
    dictionary logged_ids;   // avoid spamming for same entity
    dictionary logged_types; // track which types we've fully dumped
    int frame = 0;

    script() {
        @g = get_scene();
    }

    void on_level_start() {}

    string var_type_str(int id) {
        if (id == 1) return "Bool";
        if (id == 2) return "Int8";
        if (id == 3) return "Int16";
        if (id == 4) return "Int32";
        if (id == 5) return "Int64";
        if (id == 6) return "Float";
        if (id == 7) return "String";
        if (id == 8) return "Array";
        if (id == 9) return "Struct";
        if (id == 10) return "Vec2";
        return "Unknown(" + id + ")";
    }

    string var_val_str(varvalue@ v) {
        if (@v == null) return "null";
        int id = v.type_id();
        if (id == 1) return "" + v.get_bool();
        if (id == 2) return "" + v.get_int8();
        if (id == 3) return "" + v.get_int16();
        if (id == 4) return "" + v.get_int32();
        if (id == 5) return "" + v.get_int64();
        if (id == 6) return "" + v.get_float();
        if (id == 7) return "'" + v.get_string() + "'";
        if (id == 10) return "<" + v.get_vec2_x() + "," + v.get_vec2_y() + ">";
        if (id == 8) {
            vararray@ arr = v.get_array();
            if (@arr == null) return "Array(null)";
            string s = "Array[" + arr.size() + "] eltype=" + var_type_str(arr.element_type_id()) + " {";
            uint lim = arr.size();
            if (lim > 8) lim = 8;
            for (uint ai = 0; ai < lim; ai++) {
                if (ai > 0) s += ", ";
                s += var_val_str(arr.at(ai));
            }
            if (arr.size() > 8) s += ", ...";
            s += "}";
            return s;
        }
        if (id == 9) {
            varstruct@ st = v.get_struct();
            if (@st == null) return "Struct(null)";
            string s = "Struct(" + st.type_name() + ")[" + st.num_vars() + "]";
            return s;
        }
        return "?type=" + id;
    }

    void dump_vars(entity@ e, string prefix) {
        varstruct@ vars = e.vars();
        if (@vars == null) {
            puts(prefix + "vars: null");
            return;
        }
        puts(prefix + "vars: struct_type=" + vars.type_name() + " count=" + vars.num_vars());
        for (uint i = 0; i < vars.num_vars(); i++) {
            string name = vars.var_name(i);
            varvalue@ val = vars.get_var(i);
            puts(prefix + "  " + name + "[" + var_type_str(val.type_id()) + "] = " + var_val_str(val));
            // If it's a struct, dump one level deeper
            if (val.type_id() == 9) {
                varstruct@ st = val.get_struct();
                if (@st != null) {
                    for (uint j = 0; j < st.num_vars(); j++) {
                        string sn = st.var_name(j);
                        varvalue@ sv = st.get_var(j);
                        puts(prefix + "    " + sn + "[" + var_type_str(sv.type_id()) + "] = " + var_val_str(sv));
                    }
                }
            }
        }
    }

    void dump_sprite_info(entity@ e, string prefix) {
        sprites@ spr = e.get_sprites();
        if (@spr == null) {
            puts(prefix + "sprites: null");
            return;
        }

        // Try to enumerate sprite sets based on entity type
        string tn = e.type_name();

        // The entity's own sprite_index
        string si = e.sprite_index();
        if (si == "") {
            puts(prefix + "sprite_index='' (empty, skipping sprite queries)");
            return;
        }
        int anim_len = spr.get_animation_length(si);
        uint pal_count = spr.get_palette_count(si);
        puts(prefix + "sprite_index='" + si + "' anim_len=" + anim_len + " palette_count=" + pal_count);
        // Also check if variant-numbered versions exist
        for (int v = 1; v <= 6; v++) {
            int vlen = spr.get_animation_length(si + v);
            if (vlen > 0) {
                puts(prefix + "  variant: '" + si + v + "' anim_len=" + vlen);
            } else {
                break;
            }
        }

        // Try the entity type name as a sprite set
        uint sc = spr.get_sprite_count(tn);
        if (sc > 0) {
            puts(prefix + "sprite_set('" + tn + "'): " + sc + " sprites");
            uint lim = sc;
            if (lim > 40) lim = 40;
            for (uint i = 0; i < lim; i++) {
                string sn = spr.get_sprite_name(tn, i);
                int al = spr.get_animation_length(sn);
                uint pc = spr.get_palette_count(sn);
                puts(prefix + "  [" + i + "] " + sn + " frames=" + al + " palettes=" + pc);
            }
            if (sc > 40) puts(prefix + "  ... +" + (sc - 40) + " more");
        }

        // Also try without "enemy_" prefix
        if (tn.substr(0, 6) == "enemy_") {
            string short_name = tn.substr(6);
            uint sc2 = spr.get_sprite_count(short_name);
            if (sc2 > 0 && sc2 != sc) {
                puts(prefix + "sprite_set('" + short_name + "'): " + sc2 + " sprites");
                uint lim = sc2;
                if (lim > 40) lim = 40;
                for (uint i = 0; i < lim; i++) {
                    string sn = spr.get_sprite_name(short_name, i);
                    int al = spr.get_animation_length(sn);
                    uint pc = spr.get_palette_count(sn);
                    puts(prefix + "  [" + i + "] " + sn + " frames=" + al + " palettes=" + pc);
                }
                if (sc2 > 40) puts(prefix + "  ... +" + (sc2 - 40) + " more");
            }
        }

        // Try cleansed sprite sets
        string cleansed_sets = "entity_cleansed,entity_cleansed_full,entity_cleansed_walk";
        // We can't split easily, just try them individually
        uint c1 = spr.get_sprite_count("entity_cleansed");
        uint c2 = spr.get_sprite_count("entity_cleansed_full");
        uint c3 = spr.get_sprite_count("entity_cleansed_walk");
        if (c1 > 0 || c2 > 0 || c3 > 0) {
            puts(prefix + "cleansed sets: entity_cleansed=" + c1 + " entity_cleansed_full=" + c2 + " entity_cleansed_walk=" + c3);
        }
    }

    void dump_entity(entity@ e) {
        string tn = e.type_name();
        string key = tn + "_" + e.id();

        // Full var/sprite dump once per type
        bool full_dump = !logged_types.exists(tn);
        if (full_dump) logged_types.set(tn, true);

        // Per-entity basic dump once per id
        if (logged_ids.exists(key)) return;
        logged_ids.set(key, true);

        puts("========================================");
        puts("DIAG: type=" + tn + " id=" + e.id());
        puts("  pos=" + e.x() + "," + e.y());
        puts("  face=" + e.face() + " rotation=" + e.rotation());
        puts("  palette=" + e.palette());
        puts("  layer=" + e.layer());
        puts("  sprite_index='" + e.sprite_index() + "'");
        puts("  time_warp=" + e.time_warp());

        // Hittable info
        hittable@ h = e.as_hittable();
        if (@h != null) {
            puts("  [hittable] scale=" + h.scale() + " life=" + h.life() + "/" + h.life_initial() + " team=" + h.team());
            puts("  [hittable] x_speed=" + h.x_speed() + " y_speed=" + h.y_speed());
            puts("  [hittable] freeze_frame_timer=" + h.freeze_frame_timer());
        }

        // Controllable info
        controllable@ c = e.as_controllable();
        if (@c != null) {
            puts("  [ctrl] state=" + c.state() + " state_timer=" + c.state_timer());
            puts("  [ctrl] sprite_index='" + c.sprite_index() + "'");
            puts("  [ctrl] attack_state=" + c.attack_state() + " attack_timer=" + c.attack_timer());
            puts("  [ctrl] attack_sprite_index='" + c.attack_sprite_index() + "'");
            puts("  [ctrl] attack_face=" + c.attack_face());
            puts("  [ctrl] stun_timer=" + c.stun_timer());
            puts("  [ctrl] ground=" + c.ground() + " roof=" + c.roof() + " wall_l=" + c.wall_left() + " wall_r=" + c.wall_right());
            puts("  [ctrl] draw_offset=" + c.draw_offset_x() + "," + c.draw_offset_y());
            puts("  [ctrl] filth_type=" + c.filth_type());
        }

        // Effect info
        effect@ ef = e.as_effect();
        if (@ef != null) {
            puts("  [effect] sprite_set='" + ef.sprite_set() + "'");
            puts("  [effect] state_timer=" + ef.state_timer() + " total_frames=" + ef.total_frames());
            puts("  [effect] frame_rate=" + ef.frame_rate());
            puts("  [effect] scale=" + ef.scale_x() + "," + ef.scale_y());
            puts("  [effect] sub_layer=" + ef.sub_layer() + " colour=0x" + formatUint(ef.colour()));
            puts("  [effect] speed=" + ef.x_speed() + "," + ef.y_speed());
        }

        // Filth ball info
        filth_ball@ fb = e.as_filth_ball();
        if (@fb != null) {
            puts("  [filth_ball] filth_type=" + fb.filth_type() + " direction=" + fb.direction() + " distance=" + fb.distance());
        }

        // Full dump for first entity of each type
        if (full_dump) {
            puts("  --- FULL VAR DUMP ---");
            dump_vars(e, "  ");
            puts("  --- SPRITE INFO ---");
            dump_sprite_info(e, "  ");
        }

        puts("========================================");
    }

    // Utility to format uint as hex
    string formatUint(uint val) {
        string digits = "0123456789ABCDEF";
        string hex = "";
        uint v = val;
        for (int i = 0; i < 8; i++) {
            uint d = v & 0xF;
            hex = digits.substr(d, 1) + hex;
            v = v >> 4;
        }
        return hex;
    }

    void step(int entities) {
        frame++;
        // Only scan every 30 frames to reduce spam
        if (frame % 30 != 1) return;

        camera@ cam = get_active_camera();
        float cx = cam.x();
        float cy = cam.y();

        for (int i = 0; i < entities; i++) {
            entity@ e = entity_by_index(i);
            string tn = e.type_name();

            // Skip non-enemy types
            if (tn == "dust_man") continue;
            if (tn == "camera_node") continue;
            if (tn == "trigger") continue;
            if (tn == "base_trigger") continue;
            if (tn == "text_trigger") continue;
            if (tn == "fog_trigger") continue;
            if (tn == "ambience_trigger") continue;
            if (tn == "music_trigger") continue;
            if (tn == "special_trigger") continue;
            if (tn == "kill_zone") continue;
            if (tn == "checkpoint") continue;
            if (tn == "level_start") continue;
            if (tn == "level_end") continue;
            if (tn == "emitter") continue;
            if (tn == "AI_controller") continue;
            if (tn == "hit_box_controller") continue;
            if (tn == "dust_worth") continue;
            if (tn == "filth_ball") continue;
            if (tn == "effect") continue;

            // Distance check
            float dx = e.x() - cx;
            float dy = e.y() - cy;
            if (dx > 2000 || dx < -2000 || dy > 2000 || dy < -2000) continue;

            dump_entity(e);
        }
    }

    void draw(float sub_frame) {}
}
