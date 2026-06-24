//Genuinely dont remember if this is my script but im checking it in to keep it :)
class script {
  [option, 0:None, 1:Mansion, 2:Forest, 3:City, 4:Lab, 5:Virtual] int Spread_Type;
  script() {

  }

  void entity_on_add(entity@ e) {
    filth_ball@ fb = e.as_filth_ball();
    if (@fb == null) {
      return;
    } if (fb.metadata().has_int('ignore')) {
      return;
    } 

    // Choose a random dust type.
    fb.filth_type(Spread_Type);
  }
}

// enum FilthType
// {
	
// 	Clean	= 0,
// 	Dust	= 1,
// 	Leaf	= 2,
// 	Trash	= 3,
// 	Slime	= 4,
// 	Poly	= 5,
// 	None	= 6,
// 	Default	= 7,
	
// }