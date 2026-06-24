#include "../lib/std.cpp"
#include "../lib/math/math.cpp"
#include "../lib/drawing/common.cpp"

// //////////////////////////////////////////////////////////
// Constants
// //////////////////////////////////////////////////////////

const int WIN_SCORE = 5;
const float PADDLE_WIDTH = 15;
const float PADDLE_HEIGHT = 80;
const float BALL_SIZE = 8;
const float PADDLE_SPEED = 450;
const float BALL_START_SPEED_X = 400;
const float BALL_START_SPEED_Y = 200;
const float BALL_MAX_SPEED_X = 1000;
const float BALL_SPEED_MULTIPLIER = 1.075;
const float AI_PADDLE_SPEED = 380;
const float COURT_WIDTH = 750;
const float COURT_HEIGHT = 450;


// //////////////////////////////////////////////////////////
// Pong Game
// //////////////////////////////////////////////////////////

class Pong : enemy_base
{
	scene@ g;
	scriptenemy@ self;

	// Game state
	float ai_paddle_y;
	float ai_paddle_x;

	float ball_x, ball_y;
	float ball_vx, ball_vy;
	
	int player_score = 0;
	int ai_score = 0;

	bool game_started = true;

	textfield@ score_text;
	textfield@ title_text;

	Pong()
	{
		@g = get_scene();
	}

	void init(script@ s, scriptenemy@ self)
	{
		@this.self = self;
		self.auto_physics(false);
		self.base_rectangle(
			-PADDLE_HEIGHT / 2, PADDLE_HEIGHT / 2,
			-PADDLE_WIDTH / 2, PADDLE_WIDTH / 2
		);
		self.hit_rectangle(
			-PADDLE_HEIGHT / 2, PADDLE_HEIGHT / 2,
			-PADDLE_WIDTH / 2, PADDLE_WIDTH / 2
		);

		// Initial positions
		self.x(-COURT_WIDTH / 2);
		ai_paddle_x = COURT_WIDTH / 2;
		ai_paddle_y = 0;
		
		reset_ball(frand() > 0.5 ? 1 : -1);

		// Setup score text field
		@score_text = create_textfield();
		score_text.set_font("ProximaNovaReg", 36);
		score_text.align_horizontal(0);
		score_text.align_vertical(0);

		@title_text = create_textfield();
		title_text.align_horizontal(0);
		title_text.align_vertical(0);
	}

	/// Resets the ball to the center.
	/// @param direction 1 to serve to AI, -1 to serve to player.
	void reset_ball(int direction)
	{
		ball_x = 0;
		ball_y = 0;

		// Random start direction
		float angle = rand_range(-PI * 0.25, PI * 0.25);
		if(frand() > 0.5) angle += PI;
		ball_vx = cos(angle) * BALL_START_SPEED_X * direction;
		ball_vy = sin(angle) * BALL_START_SPEED_Y;
	}

	void step()
	{
		controllable@ c = self.as_controllable();
		if (!game_started) {
			if (c.y_intent() != 0) {
				game_started = true;
			}
			return;
		}

		// Player paddle movement
		float move = c.y_intent() * PADDLE_SPEED * DT;
		self.y(clamp(
			self.y() + move,
			-COURT_HEIGHT / 2 + PADDLE_HEIGHT / 2,
			 COURT_HEIGHT / 2 - PADDLE_HEIGHT / 2
		));

		// AI paddle movement - simple logic to follow the ball's y-position
		float dy = ball_y - ai_paddle_y;
		move = clamp(dy, -AI_PADDLE_SPEED * DT, AI_PADDLE_SPEED * DT);
		ai_paddle_y = clamp(
			ai_paddle_y + move,
			-COURT_HEIGHT / 2 + PADDLE_HEIGHT / 2,
			 COURT_HEIGHT / 2 - PADDLE_HEIGHT / 2
		);

		// Ball movement
		ball_x += ball_vx * DT;
		ball_y += ball_vy * DT;

		// Ball collision with top/bottom walls
		if(ball_y - BALL_SIZE < -COURT_HEIGHT / 2)
		{
			ball_y = -COURT_HEIGHT / 2 + BALL_SIZE;
			ball_vy *= -1;
			g.play_sound("sfx_dust_light_1", ball_x, ball_y, 1, false, true);
		}
		if(ball_y + BALL_SIZE > COURT_HEIGHT / 2)
		{
			ball_y = COURT_HEIGHT / 2 - BALL_SIZE;
			ball_vy *= -1;
			g.play_sound("sfx_dust_light_1", ball_x, ball_y, 1, false, true);
		}

		// Ball collision with paddles
		const float p1_x = self.x();
		const float p1_y = self.y();
		
		// Player paddle
		if(ball_vx < 0 &&
			ball_x - BALL_SIZE < p1_x + PADDLE_WIDTH / 2 &&
			ball_x + BALL_SIZE > p1_x - PADDLE_WIDTH / 2 &&
			ball_y + BALL_SIZE > p1_y - PADDLE_HEIGHT / 2 &&
			ball_y - BALL_SIZE < p1_y + PADDLE_HEIGHT / 2)
		{
			ball_x = p1_x + PADDLE_WIDTH / 2 + BALL_SIZE;
			ball_vx = min(abs(ball_vx) * BALL_SPEED_MULTIPLIER, BALL_MAX_SPEED_X);
			
			float hit_factor = (ball_y - p1_y) / (PADDLE_HEIGHT / 2);
			ball_vy = hit_factor * abs(ball_vx) * 0.75; // Redirect vertical momentum
			g.play_sound("sfx_jump_ground", ball_x, ball_y, 1, false, true);
		}

		// AI paddle
		if(ball_vx > 0 &&
			ball_x + BALL_SIZE > ai_paddle_x - PADDLE_WIDTH / 2 &&
			ball_x - BALL_SIZE < ai_paddle_x + PADDLE_WIDTH / 2 &&
			ball_y + BALL_SIZE > ai_paddle_y - PADDLE_HEIGHT / 2 &&
			ball_y - BALL_SIZE < ai_paddle_y + PADDLE_HEIGHT / 2)
		{
			ball_x = ai_paddle_x - PADDLE_WIDTH / 2 - BALL_SIZE;
			ball_vx = -min(abs(ball_vx) * BALL_SPEED_MULTIPLIER, BALL_MAX_SPEED_X);
			
			float hit_factor = (ball_y - ai_paddle_y) / (PADDLE_HEIGHT / 2);
			ball_vy = hit_factor * abs(ball_vx) * 0.75; // Redirect vertical momentum
			g.play_sound("sfx_jump_ground", ball_x, ball_y, 1, false, true);
		}

		// Scoring
		if(ball_x < -COURT_WIDTH / 2 - BALL_SIZE * 2)
		{
			ai_score++;
			reset_ball(-1); // Serve to Player
			g.play_sound("sfx_book_page_turn_1", 0, 0, 1, false, true);
			check_win();
		}
		else if(ball_x > COURT_WIDTH / 2 + BALL_SIZE * 2)
		{
			player_score++;
			reset_ball(1); // Serve to AI
			g.play_sound("sfx_book_page_turn_2", 0, 0, 1, false, true);
			check_win();
		}
	}

	void check_win()
	{
		if(player_score >= WIN_SCORE)
		{
			// Player wins, end the level
			g.end_level(self.x(), self.y());
		}
		else if(ai_score >= WIN_SCORE)
		{
			// AI wins, restart from checkpoint
			g.load_checkpoint();
		}
	}

	void draw(float sub_frame)
	{
		const uint clr = 0xFFFFFFFF;

		if (!game_started) {
			title_text.set_font("envy_bold", 128);
			title_text.text("PONG");
			title_text.draw_hud(20, 20, 0, -100, 1, 1, 0);

			title_text.set_font("ProximaNovaReg", 48);
			title_text.text("First to " + WIN_SCORE + " wins\nMove paddle to start");
			title_text.draw_hud(20, 20, 0, 50, 1, 1, 0);
			return;
		}

		// Draw court boundaries
		g.draw_rectangle_world(18, 18, -COURT_WIDTH / 2 - PADDLE_WIDTH, -COURT_HEIGHT / 2, COURT_WIDTH / 2 + PADDLE_WIDTH, -COURT_HEIGHT / 2 + 2, 0, clr);
		g.draw_rectangle_world(18, 18, -COURT_WIDTH / 2 - PADDLE_WIDTH, COURT_HEIGHT / 2 - 2, COURT_WIDTH / 2 + PADDLE_WIDTH, COURT_HEIGHT / 2, 0, clr);
		
		// Dotted center line
		for(float y = -COURT_HEIGHT / 2; y < COURT_HEIGHT / 2; y += 20)
		{
			g.draw_rectangle_world(18, 18, -1, y, 1, y + 10, 0, clr);
		}

		// Draw player paddle
		float px = self.x();
		float py = self.y();
		g.draw_rectangle_world(19, 19, 
			px - PADDLE_WIDTH / 2, py - PADDLE_HEIGHT / 2,
			px + PADDLE_WIDTH / 2, py + PADDLE_HEIGHT / 2,
			0, clr);
			
		// Draw AI paddle
		float ax = ai_paddle_x;
		g.draw_rectangle_world(19, 19, 
			ax - PADDLE_WIDTH / 2, ai_paddle_y - PADDLE_HEIGHT / 2,
			ax + PADDLE_WIDTH / 2, ai_paddle_y + PADDLE_HEIGHT / 2,
			0, clr);
			
		// Draw Ball
		float draw_x = ball_x;
		float draw_y = ball_y;
		g.draw_rectangle_world(19, 19, 
			draw_x - BALL_SIZE / 2, draw_y - BALL_SIZE / 2,
			draw_x + BALL_SIZE / 2, draw_y + BALL_SIZE / 2,
			0, clr);
		
		// Draw scores
		const float SCORE_X_OFFSET = 250;
		const float SCORE_Y_POS = -350;
		const uint SCORE_CLR = 0xFFFFFFFF;
		const uint SCORE_BG_CLR = 0xAA000000;

		g.draw_rectangle_hud(20, 19, -350, -420, 350, -320, 0, SCORE_BG_CLR);
		
		score_text.text(player_score + "");
		score_text.colour(SCORE_CLR);
		score_text.draw_hud(20, 20, -SCORE_X_OFFSET, SCORE_Y_POS, 1, 1, 0);

		score_text.text(ai_score + "");
		score_text.colour(SCORE_CLR);
		score_text.draw_hud(20, 20, SCORE_X_OFFSET, SCORE_Y_POS, 1, 1, 0);
	}
}

class script
{
	scene@ g;
	camera@ cam;

	script()
	{
		@g = get_scene();
		@cam = get_camera(0);
	}

	void spawn_player(message@ msg)
	{
		// Create the Pong game object and set it as the player's controllable entity.
		// This replaces the default Dustman character.
		scriptenemy@ pl = create_scriptenemy(Pong());
		pl.y(0);
		msg.set_entity("player", pl.as_entity());
	}
	
	void step(int entities)
	{
		// Center camera on the court and zoom out to fit everything.
		cam.x(0);
		cam.y(0);
		cam.screen_height(COURT_HEIGHT + 150);
	}
}