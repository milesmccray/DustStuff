
class script : callback_base {
  bool disable;
  controllable@ contPlayer;
  float force;
  int framesBetweenShakes;
  int frames_between_shakes;
  bool isShaking;
  
  script() {
    // Initialize all this crap
    disable = false;
    force = 0;
    frames_between_shakes = 0;
    isShaking = false;
    add_broadcast_receiver('OnMyCustomEventNameAck', this, 'OnMyCustomEventNameAck');
  }
  
  void step(int entities) {
    if(disable) {
      return;
    }
    
    // Grab the active player so we can get their x and y position
    @contPlayer = controller_controllable(get_active_player());

    if(framesBetweenShakes <= 0 && !disable) {
      get_active_camera().add_screen_shake(contPlayer.prev_x(), contPlayer.prev_y(), 0, 0);
      isShaking = false;
      framesBetweenShakes = frames_between_shakes;
    } 
    
    else {
        if(!isShaking) {
          puts("shaking with float value of " + force);
          isShaking = true;
          get_active_camera().add_screen_shake(contPlayer.prev_x(), contPlayer.prev_y(), 0, force);
        }
        framesBetweenShakes--;
    } 
  }
  
  void OnMyCustomEventNameAck(string id, message@ msg) {
    if (msg.get_string('disable_shake') == "true") {
      puts("disable_shake");
      force = 0;
      disable = true;
    } else if (msg.get_string('disable_shake') == "false") {
      puts("enable_shake");
      disable = false;
    }
    
    // Get the force...
    if(msg.get_float('force') >= 0) {
      force = msg.get_float('force');
      puts("force: " + force);
    }
    
    // get the frames_between_shakes...
    if(msg.get_int('frames_between_shakes') >= 0) {
      frames_between_shakes = msg.get_int('frames_between_shakes');
      puts("frames_between_shakes: " + frames_between_shakes);
    }
  }
}

class mytrigger : trigger_base, callback_base{
  // Denotes if this trigger should disable shaking
  [text] bool disable_shake_trigger;
  
  // How many frames to wait in-between shakes
  [text] int frames_between_shakes;
  
  // Force of the shake.  If this is too high, it destroys the camera :(
  [text] float force;

  mytrigger() {
    // Initialize all this crap
    disable_shake_trigger = false;
    frames_between_shakes = 1;
    force = 10;
  }
  
  void activate(controllable@ e) {
    // If this trigger is used to disable shaking...
    if(disable_shake_trigger) {
      // Send the script a message to stop all shaking
      message@ msg = create_message();
      msg.set_string('disable_shake', 'true');
      broadcast_message('OnMyCustomEventNameAck', msg);
    }
    
    // If this trigger is used to enable shaking...
    else { 
     /*  
      *  Grab all the user's selections, and pass it off to the script.
      *  If a player walks through two separate triggers which have their
      *  disable_shake_trigger bool set to false, the force and frames_between_shakes
      *  values will be updated to the most recently activated trigger's value
      */
      message@ msg = create_message();
      msg.set_string('disable_shake', 'false');
      msg.set_float('force', force);
      msg.set_int('frames_between_shakes', frames_between_shakes);
      broadcast_message('OnMyCustomEventNameAck', msg);
    }
  }
  
}