// multiple_music_trigger.cpp
// Embeds multiple music tracks and uses triggers to switch between them.
// Fades out current track and fades in new track when switching.

const string EMBED_track1 = "track1.ogg";
const string EMBED_track2 = "track2.ogg";
const string EMBED_track3 = "track3.ogg";
const string EMBED_track4 = "track4.ogg";
const string EMBED_track5 = "track5.ogg";
const string EMBED_track6 = "track6.ogg";

class script : callback_base {

    scene@ g;
    string current_track = "";
    string next_track = "";
    audio@ current_audio;
    array<string> all_tracks = {"track1", "track2", "track3", "track4", "track5", "track6"};

    float target_volume = 0.75;
    float fade_speed = 0.02; // How fast to fade per frame
    float current_volume = 0.0;
    bool fading_out = false;

    script() {
        @g = get_scene();
        add_broadcast_receiver("music.play", this, "on_music_play");
    }

    void build_sounds(message@ msg) {
        msg.set_string("track1", "track1");
        msg.set_string("track2", "track2");
        msg.set_string("track3", "track3");
        msg.set_string("track4", "track4");
        msg.set_string("track5", "track5");
		msg.set_string("track6", "track6");
    }

    void on_music_play(string id, message@ msg) {
        string track = msg.get_string("track");
        if (track == current_track) return;

        next_track = track;

        if (current_track == "") {
            // No current track, just start the new one
            start_next_track();
        } else {
            // Fade out current track first
            fading_out = true;
        }
    }

    void start_next_track() {
        stop_all();
        current_track = next_track;
        next_track = "";
        fading_out = false;
        current_volume = 0.0;
        @current_audio = g.play_persistent_stream(current_track, 1, true, 0.0, true);
        puts("music: playing " + current_track);
    }

    void stop_all() {
        for (uint i = 0; i < all_tracks.length(); i++) {
            g.stop_persistent_stream(all_tracks[i]);
        }
        @current_audio = null;
    }

    void step(int entities) {
        if (@current_audio == null) return;

        if (fading_out) {
            // Fade out
            current_volume -= fade_speed;
            if (current_volume <= 0.0) {
                current_volume = 0.0;
                start_next_track();
            } else {
                current_audio.volume(current_volume);
            }
        } else {
            // Fade in
            if (current_volume < target_volume) {
                current_volume += fade_speed;
                if (current_volume > target_volume) current_volume = target_volume;
                current_audio.volume(current_volume);
            }
        }
    }
}

class MusicTrigger : trigger_base {

    scripttrigger@ self;

    [text] string track_name = "track1";
    [text] bool repeat = false;

    bool fired = false;
    bool inside = false;

    MusicTrigger() {}

    void init(script@ s, scripttrigger@ self) {
        @this.self = self;
    }

    void step() {
        if (repeat && fired && !inside) fired = false;
        inside = false;
    }

    void activate(controllable@ e) {
        dustman@ dm = e.as_dustman();
        if (@dm == null) return;

        inside = true;
        if (fired) return;

        message@ msg = create_message();
        msg.set_string("track", track_name);
        broadcast_message("music.play", msg);

        fired = true;
    }

    void checkpoint_load() {
        fired = false;
        inside = false;
    }
}