#include "../lib/std.cpp"
#include "../lib/enums/EntityState.cpp"

enum TrackerState
{
	
	Idle,
	WaitingForHover,
	WaitingForDownDash,
	WaitingForLand,
	
}

class script
{
  float pos_y;
	controllable@ player = null;
  scene@ g;
	TrackerState state = TrackerState::Idle;
	
	bool debug_states = false;
	
	// The frame count from first entering hover to the last dash
	int from_hover_counter = -2;
	// The frame count from te down dash to the dash
	int from_downdash_counter = -2;
	
	int hover_subframes = 0;
	int dash_subframes = 0;
	bool has_dashed = false;
	
	array<Total> totals;
	array<Total> totals_hover;
	
	textfield@ current_txt;
	textfield@ current_subframes_txt;
	int current_display_timer = 0;
	int current_display_max = 180;
	
	textfield@ totals_txt;
	bool show_totals = false;
	
	script()
	{
    @g = get_scene();
		const string font = 'ProximaNovaReg';
		const int font_size = 26;
		@current_txt = create_textfield();
		current_txt.colour(0xFFFFFFFF);
		current_txt.set_font(font, font_size);
		current_txt.align_horizontal(0);
		current_txt.align_vertical(1);
		@current_subframes_txt = create_textfield();
		current_subframes_txt.colour(0xAAFFFFFF);
		current_subframes_txt.set_font(font, font_size);
		current_subframes_txt.align_horizontal(-1);
		current_subframes_txt.align_vertical(-1);
		
		@totals_txt = create_textfield();
		totals_txt.set_font('ProximaNovaReg', 36);
		totals_txt.align_horizontal(-1);
		totals_txt.align_vertical(-1);
	}
	
	void debug(string message)
	{
		if(debug_states) puts(message);
	}
	
	void go_idle()
	{
		state = TrackerState::Idle;
		from_hover_counter = -2;
		from_downdash_counter = -2;
		hover_subframes = 0;
		dash_subframes = 0;
	}
	
	void update_total(Total@ total)
	{
		total.count++;
		total.hover_frames = from_hover_counter;
		total.downdash_frames = from_downdash_counter;
		total.hover_subframes = from_hover_counter + (hover_subframes / 5.0);
		total.downdash_subframes = from_downdash_counter + (dash_subframes / 5.0);
	}
	
	void add_ledge_cancel()
	{
		if(from_downdash_counter >= int(totals.size()))
			totals.resize(from_downdash_counter + 1);
			
		if(from_hover_counter >= int(totals_hover.size()))
			totals_hover.resize(from_hover_counter + 1);
			
		dash_subframes = player.state() == EntityState::Dash ? -int(player.state_timer() / 0.08) : 0;
		hover_subframes + dash_subframes;
		
		Total@ total = @totals_hover[from_hover_counter];
		update_total(total);
		@total = @totals[from_downdash_counter];
		update_total(total);
		
		current_subframes_txt.text(total.get_subframe_text());
//		puts('Ledge Cancel: ' + total.get_frame_text() + ' - ' + total.get_subframe_text());
		
		current_txt.text('Ledge Cancel: ' + total.get_frame_text());
		current_display_timer = current_display_max;
	}
	
	void checkpoint_load()
	{
		@player = null;
		go_idle();
	}
	
  void teleport() {
    if(@player != null) {
      dustman@ dm = player.as_dustman();

      //Spawn large totem above dustman in attacking state and add to scene
      entity@ totem = create_entity("enemy_stoneboss");
      totem.as_controllable().scale(5, false);
      totem.set_xy(dm.x(), dm.y()-10);
      totem.as_controllable().attack_state(1);
      g.add_entity(totem);


      for(uint i = 0; i < 10; i++) {
        @totem = create_entity("enemy_stoneboss");
        totem.as_controllable().scale(5, false);
        totem.set_xy(dm.x()+(i*(2*48)), dm.y()-100);
        totem.as_controllable().attack_state(1);
        g.add_entity(totem);
      }

      for(uint i = 0; i < 10; i++) {
        @totem = create_entity("enemy_stoneboss");
        totem.as_controllable().scale(5, false);
        totem.set_xy(dm.x()-(i*(2*48)), dm.y()-100);
        totem.as_controllable().attack_state(1);
        g.add_entity(totem);
      }


      for(uint i = 0; i < 10; i++) {
        @totem = create_entity("enemy_stoneboss");
        totem.as_controllable().scale(5, false);
        totem.set_xy(dm.x()+(i*(2*48)), dm.y()-300);
        totem.as_controllable().attack_state(1);
        g.add_entity(totem);
      }
      
      for(uint i = 0; i < 10; i++) {
        @totem = create_entity("enemy_stoneboss");
        totem.as_controllable().scale(5, false);
        totem.set_xy(dm.x()-(i*(2*48)), dm.y()-300);
        totem.as_controllable().attack_state(1);
        g.add_entity(totem);
      }

            for(uint i = 0; i < 10; i++) {
        @totem = create_entity("enemy_stoneboss");
        totem.as_controllable().scale(5, false);
        totem.set_xy(dm.x()+(i*(2*48)), dm.y()-500);
        totem.as_controllable().attack_state(1);
        g.add_entity(totem);
      }
      
      for(uint i = 0; i < 10; i++) {
        @totem = create_entity("enemy_stoneboss");
        totem.as_controllable().scale(5, false);
        totem.set_xy(dm.x()-(i*(2*48)), dm.y()-500);
        totem.as_controllable().attack_state(1);
        g.add_entity(totem);
      }
    }
  }

	void on_level_end()
	{
		string output = '-------------\n';
		for(int i = 0, count = int(totals.size()); i < count; i++)
		{
			Total@ total = @totals[i];
			if(total.count == 0) continue;
			output += total.downdash_frames + ' x ' + total.count + '\n';
		}
		
		output += '\n\n-------------\n';
		for(int i = 0, count = int(totals_hover.size()); i < count; i++)
		{
			Total@ total = @totals_hover[i];
			if(total.count == 0) continue;
			output += '[' + total.hover_frames + '] x ' + total.count + '\n';
		}
		
		totals_txt.text(output);
		show_totals = true;
	}
	
	void step(int num_entities)
	{
		if(player is null)
		{
			@player = controller_controllable(0);
			return;
		}
		
		if(current_display_timer > 0)
			current_display_timer--;
		
		const int player_state = player.state();
		
		if(from_hover_counter >= -2)
			from_hover_counter++;
		if(from_downdash_counter >= -2)
			from_downdash_counter++;
		
		switch(state)
		{
			case TrackerState::Idle:
			{
				if(player_state == EntityState::WallRun)
				{
					debug(' Entered WallRun >> WaitingForHover ');
					state = TrackerState::WaitingForHover;
				}
			}
				break;
				
			case TrackerState::WaitingForHover:
			{
				if(player_state == EntityState::Hover)
				{
					debug(' Entered hover >> WaitingForDownDash ' + player.state_timer() / 0.04);
					state = TrackerState::WaitingForDownDash;
					hover_subframes = int(player.state_timer() / 0.04);
					from_hover_counter = -1;
				}
				else if(player_state != EntityState::WallRun)
				{
					debug(' Exit WallRun >> Idle (Canceled)');
					// If the state is not WallRun or Hover, assume the wallrun has ended for other reasons?
					go_idle();
				}
			}
				break;
				
			case TrackerState::WaitingForDownDash:
			{
				if(player.fall_intent() == 1)
				{
					debug(' Entered fall >> WaitingForLand ' + player.state_timer() + ' ' + player_state);
					state = TrackerState::WaitingForLand;
					has_dashed = false;
					from_downdash_counter = -1;
				}
				else if(player_state != EntityState::Hover)
				{
					debug(' Exit Hover >> Idle (Canceled)');
					go_idle();
				}
			}
				break;
				
			case TrackerState::WaitingForLand:
			{
				if(player.dash_intent() == 1 && player.ground())
					has_dashed = true;
				
				if(player_state == EntityState::Dash || player_state == EntityState::CrouchJump)
				{
					if(has_dashed)
					{
						//add_ledge_cancel();
            teleport();
						go_idle();
					}
					else
					{
						debug(' Exit Fall 1 >> Idle (Canceled)');
						go_idle();
					}
				}
				else if(player_state != EntityState::Fall && player_state != EntityState::Land)
				{
					debug(' Exit Fall 2 >> Idle (Canceled) ' + player_state);
					go_idle();
				}
			}
				break;
		}
	}
	
	void draw(float sub_frame)
	{
		if(show_totals)
		{
			totals_txt.draw_hud(22, 22, 800 - 20 - totals_txt.text_width(), -450 + 20, 1, 1, 0);
		}
		
		if(current_display_timer > 0)
		{
			const float x = 0;
			const float y = 450 - 20;
			current_txt.draw_hud(22, 22, x, y, 1, 1, 0);
			current_subframes_txt.draw_hud(22, 22,
				x + current_txt.text_width() * 0.5 + 10,
				y - current_txt.text_height(), 0.7, 0.7, 0);
		}
	}
	
}

class Total
{
	
	int count = 0;
	int hover_frames = 0;
	int downdash_frames = 0;
	float hover_subframes = 0;
	float downdash_subframes = 0;
	
	string get_frame_text()
	{
		return downdash_frames + '[' + hover_frames + ']';
	}
	string get_subframe_text()
	{
		return downdash_subframes + '[' + hover_subframes + ']';
	}
	
}