class script {
    [entity] int entity_id;
    [bool] bool delete;

    void editor_step() {
        if (!delete) return;
        delete = false;
        editor_sync_vars_menu();

        scene@ g = get_scene();
        controllable@ e = controllable_by_id(entity_id);
        g.remove_entity(e.ai_controller().as_entity());
    }
}
