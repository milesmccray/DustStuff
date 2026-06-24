class script {
    void entity_on_add(entity@ e) {
        if (e.type_name() == "entity_cleansed" ||
            e.type_name() == "entity_cleansed_walk" ||
            e.type_name() == "entity_cleansed_full")
        {
            get_scene().remove_entity(e);
        }
    }
}
