class script : callback_base {
  [position,mode:world,layer:19,y:Y1, tiles] float X1;
  [position,mode:world,layer:19,y:Y2, tiles] float X2;
  [hidden] float Y1;
  [hidden] float Y2;
  scene@ g;
  bool hit = false;
  bool done = false;
  script() {
    @g = get_scene();
    add_broadcast_receiver('OnMyCustomEventName', this, 'OnMyCustomEventName');
  }

  void step(int entities) {
    if(hit && !done) {
      for(int i = X1; i <= X2; i++) {
        for(int j = Y1; j <= Y2; j++) {
          tileinfo@ t = g.get_tile(X1 - 1, Y1, 19);
          g.set_tile(i, j, 19, t, true);
        }
      }
      hit = false;
      done = true;
    }

  }

  void OnMyCustomEventName(string id, message@ msg) {
    if(msg.get_string('hit') == 'true') {   
      hit = true;
    }
  }

  void on_level_end() {
    if(g.user_id() == 000000 || g.user_id() == 000000) {           //REPLACE 000000 WITH ID YOU WANT TO ALLOW
      //nothing
    } else {
      g.combo_break_count(1);
    }
  }
}

class youWin: trigger_base, callback_base {
	script@ script;
	scene@ g;
	scripttrigger @scr;
  
  youWin() {
    @g = get_scene();  
  }
  
  void init(script@ s, scripttrigger@ self) {  
    @scr = self;
  }
    
  void editor_step() {

  }
  
  void activate(controllable @e) {   
   if(g.user_id() == 000000 || g.user_id() == 000000) {           //REPLACE 000000 WITH ID YOU WANT TO ALLOW
      if (e.as_dustman() == null) {
          return;
      }
      message@ msg;
      @msg = create_message(); 
      msg.set_string('hit', 'true');
      broadcast_message('OnMyCustomEventName', msg);
   }
  }
}