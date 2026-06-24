// character_unlock_trigger.cpp
// Trigger zones that unlock characters. Taunt cycles through unlocked characters.
//
// Place UnlockCharacterTrigger zones in the level, set the character to unlock in properties.
// The player starts with only their default character.
// Taunt cycles through all currently unlocked characters.

class script : callback_base {

    array<string> unlocked;
    int current_index = 0;
    array<int> prev_taunt;

    script() {
        add_broadcast_receiver("character.unlock", this, "on_unlock");
    }

    void on_unlock(string id, message@ msg) {
        string char_name = msg.get_string("character");

        for (uint i = 0; i < unlocked.length(); i++) {
            if (unlocked[i] == char_name) return;
        }

        unlocked.insertLast(char_name);
        puts("character_unlock: unlocked " + char_name + " (total: " + unlocked.length() + ")");
    }

    void on_level_start() {
        uint num_players = num_cameras();
        prev_taunt.resize(num_players);
        for (uint i = 0; i < num_players; i++) {
            prev_taunt[i] = 0;
            controllable@ ctrl = controller_controllable(i);
            if (@ctrl == null) continue;
            dustman@ dm = ctrl.as_dustman();
            if (@dm == null) continue;
            string char_name = dm.character();
            bool found = false;
            for (uint j = 0; j < unlocked.length(); j++) {
                if (unlocked[j] == char_name) { found = true; break; }
            }
            if (!found) unlocked.insertLast(char_name);
        }
    }

    void step(int entities) {
        if (unlocked.length() < 2) return;

        uint num_players = num_cameras();

        while (prev_taunt.length() < num_players) {
            prev_taunt.resize(prev_taunt.length() + 1);
            prev_taunt[prev_taunt.length() - 1] = 0;
        }

        for (uint i = 0; i < num_players; i++) {
            controllable@ ctrl = controller_controllable(i);
            if (@ctrl == null) continue;
            dustman@ dm = ctrl.as_dustman();
            if (@dm == null) continue;

            int taunt = ctrl.taunt_intent();

            if (taunt == 1 && prev_taunt[i] == 0) {
                current_index = (current_index + 1) % int(unlocked.length());
                dm.character(unlocked[current_index]);
                puts("character_unlock: switched to " + unlocked[current_index]);
                ctrl.taunt_intent(2);
            }

            prev_taunt[i] = taunt;
        }
    }

    void checkpoint_load() {
        unlocked.resize(0);
        current_index = 0;
        on_level_start();
    }
}

class UnlockCharacterTrigger : trigger_base {

    scripttrigger@ self;

    // Options: dustman, dustgirl, dustkid, dustworth, dustwraith, leafsprite, trashking, slimeboss
    [text] string character = "dustgirl";

    UnlockCharacterTrigger() {}

    void init(script@ s, scripttrigger@ self) {
        @this.self = self;
    }

    void activate(controllable@ e) {
        dustman@ dm = e.as_dustman();
        if (@dm == null) return;

        message@ msg = create_message();
        msg.set_string("character", character);
        broadcast_message("character.unlock", msg);
    }
}