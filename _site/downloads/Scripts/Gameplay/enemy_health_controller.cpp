class script {
    [entity] array<int> entity_id;
    [text] array<int> health;
    array<bool> changed_health;
    scene@ g;

    script() {
        @g = get_scene();
    }
    
    void on_level_start(){
       changed_health.resize(entity_id.length());

       for(int i = 0; i < changed_health.length(); i++){
            changed_health[i] = false;
       }
    }

    void step(int entities) {
        for(int i = 0; i < entity_id.length(); i++){
            if(not changed_health[i]) {
                //puts(""+entity_by_id(entity_id));
                check_entity_health(entity_by_id(entity_id[i]),i);
            }
        }
    }
    
    void checkpoint_load() {
        for(int i = 0; i < entity_id.length(); i++){
            changed_health[i] = false;
            //puts("loading checkpoint");
        }
    }
    
    void check_entity_health(entity@ e, int id) {
            if(e != null && e.id() == entity_id[id] ) {
                //puts("adding entity");
                if(not changed_health[id]) {
                    e.as_controllable().life(health[id]);
                    changed_health[id] = true;
                    //puts("changing health");
                }
            }
    }
}