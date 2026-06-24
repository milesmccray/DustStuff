#include "../lib/math/math.cpp";
#include "../lib/utils/colour.cpp";
#include "../lib/const/ColorConsts.as"

const uint NUM_PLAYERS = 4;
class script{

array<camera@> cams(NUM_PLAYERS);
  [text] float fog_change_time;
  [text] uint layer;
  [text] uint sublayer;
  [text] float frequency;
  [position,mode:world,layer:18,y:bY1] float bX1;
  [hidden] float bY1;
  bool firstRun = true;
  scene@ g;
  uint curHue = 0;
  uint frame_count = 0;
  float max_color_len = 32;
  uint curCol = BLUE;
  uint curColSpikes = BLUE;
  script() {
    fog_change_time = 0;
    frequency = .3;
    @g = get_scene();
  }
  
  void on_level_start() {
    for(uint i = 0; i < cams.size(); i++) {
      @cams[i] = get_camera(i);
    }
  }

  void step(int entities) {
    if(firstRun) {
      firstRun = false;
      curCol = cams[0].get_fog().colour(layer, sublayer);
      curColSpikes = cams[0].get_fog().colour(19, 15);
    }

    if(frame_count % 30 == 0) {
      update_color();
    }
    
    frame_count++;
  }

  void updateFog() {
    for(uint i = 0; i < cams.size(); i++) {
      if(@cams[i] != null) {
        fog_setting@ f = cams[i].get_fog(); 
        f.colour(layer, sublayer, curCol);
        f.colour(19, 15, curColSpikes);
        f.colour(14, 10, curColSpikes);
        f.colour(14, 13, curColSpikes);
        cams[i].change_fog(f, fog_change_time);
      }
    }
  }
  
  void editor_step() {
    if(frame_count % 10 == 0) {
      update_color();
    }
    
    frame_count++;
  }

  void editor_draw(float) {
    g.draw_rectangle_world(17, 20, bX1, bY1, bX1+50, bY1+50, 0, 
    curCol);
  }

  void update_color() {
    curCol = TransformH2(curCol, frequency);
    curColSpikes = TransformH2(curColSpikes, frequency);
    updateFog(); 
  }

  uint TransformH2(uint col, const float fHue) {
    return adjust(col, fHue, 0, 0);
  }
}