class script {
    [entity] int trigger_id;
    [text] float speed;
    [text] bool activate = false;
    [hidden] bool _activate = false;

    void editor_step() {
        if (activate != _activate) {
            _activate = activate;

            entity@ trigger = entity_by_id(trigger_id);
            
            string var_name = "";
            if (trigger.type_name() == "fog_trigger") {
                var_name = "fog_speed";
            } else if (trigger.type_name() == "ambience_trigger") {
                var_name = "ambience_speed";
            } else if (trigger.type_name() == "music_trigger") {
                var_name = "music_speed";
            }

            if (var_name != "") {
                varstruct@ vars = trigger.vars();
                vars.get_var(var_name).set_float(speed);
            }
        }
    }
}
