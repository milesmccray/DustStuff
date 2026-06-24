class script : callback_base{
  scene@ g;
  script() {
    @g = get_scene();
    add_broadcast_receiver('OnMyCustomEventName', this, 'OnMyCustomEventName');
  }

  void step(int) {
  }

  void OnMyCustomEventName(string id, message@ msg) {
    if(msg.get_string('rotate') == 'true') {  
      if(@get_camera(0) != null) {
        get_camera(0).rotation((msg.get_int('angle')));
      }
    }
  }

}



class edge_trigger : trigger_base, callback_base {
    bool activated;
    bool active_this_frame;
    controllable@ trigger_entity;
    [angle] float angle;

    void init(script@ s, scripttrigger@ self) {
        activated = false;
        active_this_frame = false;
    }
    
    void rising_edge(controllable@ e) {
        @trigger_entity = @e;
        message@ msg = create_message();
        msg.set_string('rotate', "true");
        msg.set_int('angle', angle);
        broadcast_message('OnMyCustomEventName', msg); 
    }

    void falling_edge(controllable@ e) {
        @trigger_entity = null;
        // do stuff
    }
    
    void step() {
        if(activated) {
            if(not active_this_frame) {
                activated = false;
                falling_edge(@trigger_entity);
            }
            active_this_frame = false;
        }
    }
    
    void activate(controllable@ e) {
        if(e.player_index() == 0) {
            if(not activated) {
                rising_edge(@e);
                activated = true;
            }
            active_this_frame = true;
        }
    }
}