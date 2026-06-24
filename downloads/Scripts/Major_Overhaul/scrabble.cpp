#include "../lib/std.cpp"
#include "../lib/math/math.cpp"
#include "../lib/drawing/circle.cpp"
#include "../lib/drawing/common.cpp"
#include "../lib/string/common.cpp"
#include "../lib/input/Mouse.cpp"
#include "../lib/easing/cubic.cpp"

// -- Replay-Consistent Randomness --
const uint NUM_ENCODERS = 4;

// Dummy entity for encoding the random seed
class replay_rand_dummy : enemy_base {};

// Manages the replay-consistent random seed
class replay_rand {
  scene@ g;
	array<uint> encoders;
  uint seed;

  int ecx;
  int ecy;
  uint frame_counter;

  replay_rand() {
    @g = @get_scene();
  }

  void step() {
    if (encoders.size() == 0) {
      init_encoders();
      return;
    }
    if (frame_counter == 10) {
      encode_seed();
    }
    if (frame_counter == 20) {
      decode_seed();
    }
    ++frame_counter;
  }

  bool seed_set() {
    return frame_counter >= 20;
  }

  void init_encoders() {
    controllable@ ec = @controller_controllable(0);
    if (@ec == null) {
      return;
    }

    ecx = int(ec.x());
    ecy = int(ec.y());
    for (uint i = 0; i < NUM_ENCODERS; i++) {
      scriptenemy@ ent = create_scriptenemy(replay_rand_dummy());
      ent.x(ecx);
      ent.y(ecy);
      g.add_entity(@ent.as_entity());
      encoders.insertLast(ent.id());
    }
  }

  void encode_seed() {
    if (is_replay()) {
      return;
    }

    srand(timestamp_now() + get_time_us());
    seed = rand();
    puts("seed is " + seed);
    for (uint i = 0; i < NUM_ENCODERS; i++) {
      entity@ ent = @entity_by_id(encoders[i]);
      if (@ent == null) {
        puts("encoding seed failed");
        return;
      }
      ent.x(ecx - 128 + ((seed >> (i * 8)) & 0xFF));
      ent.y(ecy + 100);
    }
  }

  void decode_seed() {
    seed = 0;
    for (uint i = 0; i < NUM_ENCODERS; i++) {
      entity@ ent = @entity_by_id(encoders[i]);
      if (@ent == null) {
        puts("seed reconstruction failed");
        return;
      }
      int enc_x = int(round(ent.x())) + 128 - ecx;
      int enc_y = int(round(ent.y()));
      if (enc_y == ecy) {
        puts("desync frames missing, seed reconstruction failed");
        return;
      }
      seed |= enc_x << (i * 8);
    }
    puts("constructed seed " + seed);
    srand(seed);
  }
}


// -- Scrabble Game Constants --
const int BOARD_SIZE = 15;
const int RACK_SIZE = 7;

// Enum for different tile types on the board
enum TileType
{
    Normal,
    DoubleLetter,
    TripleLetter,
    DoubleWord,
    TripleWord,
    Start
};

// Represents a single Scrabble tile with its character and score.
class Tile
{
    string tileChar;
    int tileScore;
    // Flag to track if the tile was placed in the current turn
    bool placed_this_turn = false;
    // Flag for swap selection
    bool is_selected_for_swap = false;
    // Flag to identify if a tile was originally a blank
    bool is_blank = false;

    Tile(string c, int s)
    {
        this.tileChar = c;
        this.tileScore = s;
        if (c == "-")
        {
            this.is_blank = true;
        }
    }
}

// Class to handle a single tile's animation.
class TileAnimation
{
    Tile@ tile;
    float from_x, from_y;
    float to_x, to_y;
    int target_rack_index = -1;
    int target_board_r = -1;
    int target_board_c = -1;
    float progress, duration;
    float delay;
    bool is_swap = false;
    int from_rack_index = -1; // For rack animations

    TileAnimation(Tile@ t, float fx, float fy, float tx, float ty, float dur, float del = 0)
    {
        @tile = t;
        from_x = fx; from_y = fy;
        to_x = tx; to_y = ty;
        duration = dur;
        progress = 0;
        delay = del;
    }
}

// Returns the point value for a given letter.
int ScoreOfLetter(string c)
{
    if (c == "E" || c == "A" || c == "I" || c == "O" || c == "N" || c == "R" || c == "T" || c == "L" || c == "S" || c == "U") return 1;
    if (c == "D" || c == "G") return 2;
    if (c == "B" || c == "C" || c == "M" || c == "P") return 3;
    if (c == "F" || c == "H" || c == "V" || c == "W" || c == "Y") return 4;
    if (c == "K") return 5;
    if (c == "J" || c == "X") return 8;
    if (c == "Q" || c == "Z") return 10;
    return 0; // Blank tile or invalid
}

// Returns the number of tiles for a given letter in a standard Scrabble set.
int NumOfLetters(string c)
{
    if (c == "Z" || c == "Q" || c == "X" || c == "J" || c == "K") return 1;
    if (c == "Y" || c == "W" || c == "V" || c == "F" || c == "H" || c == "P" || c == "M" || c == "C" || c == "B") return 2;
    if (c == "D" || c == "S" || c == "U" || c == "L") return 4;
    if (c == "G") return 3;
    if (c == "N" || c == "R" || c == "T") return 6;
    if (c == "O") return 8;
    if (c == "A" || c == "I") return 9;
    if (c == "E") return 12;
    return 0;
}

// Manages the bag of all Scrabble tiles using counts for each letter.
class AllTiles
{
    dictionary letter_counts;
    int total_tiles;

    AllTiles()
    {
        MakeTiles();
    }

    // Creates the full set of 100 Scrabble tiles by counting them.
    void MakeTiles()
    {
        total_tiles = 0;
        letter_counts.deleteAll();
        for (uint i = 0; i < 26; i++)
        {
            string c = string::chr(int(65 + i));
            int count = NumOfLetters(c);
            if(count > 0) 
            {
                letter_counts[c] = count;
                total_tiles += count;
            }
        }
        // Add two blank tiles
        letter_counts["-"] = 2;
        total_tiles += 2;
    }

    // Returns a random tile from the bag and removes it from the count.
    Tile@ deal_tile()
    {
        if (total_tiles == 0) return null;
        
        // Select a random tile from the remaining ones
        int random_index = rand() % total_tiles;
        array<string>@ keys = letter_counts.getKeys();
        
        string letter_to_deal = "";
        for (int i = 0; i < int(keys.length()); ++i)
        {
            string letter = keys[i];
            int count = int(letter_counts[letter]);
            if (random_index < count)
            {
                letter_to_deal = letter;
                break;
            }
            random_index -= count;
        }

        if (letter_to_deal != "")
        {
            letter_counts[letter_to_deal] = int(letter_counts[letter_to_deal]) - 1;
            total_tiles--;
            return Tile(letter_to_deal, ScoreOfLetter(letter_to_deal));
        }
        
        // Fallback in case of an issue, though it should not be reached.
        return null;
    }
}

// Holds data for a single player.
class Player
{
    int id;
    int score = 0;
    array<Tile@> PlayingTiles;

    Player(int id)
    {
        this.id = id;
        PlayingTiles.resize(RACK_SIZE);
    }
}

// -- Main Game Logic Class --
class ScrabbleGame : enemy_base
{
    // 2D array to represent the board's special tiles
    array<array<int>> board_layout;
    // 2D array to store the tiles placed on the board
    array<array<Tile@>> board_tiles;
    
    // Game objects
    AllTiles@ tile_bag;
    Player@ player;

    // Word list for validation
    dictionary@ word_list;
    bool first_move = true;

    // Drawing objects
    sprites@ board_sprites;
    canvas@ hud_canvas;
    textfield@ ui_text;

    // Game state variables
    scene@ g;
    scriptenemy@ self;
    Mouse mouse;
    bool is_swapping = false;
    bool is_selecting_blank = false;
    bool game_initialized = false;

    // Interaction state
    Tile@ selected_tile = null;
    int selected_tile_rack_index = -1;
    int drag_target_rack_index = -1;
    // Store original board position of a selected tile
    int selected_tile_board_r = -1;
    int selected_tile_board_c = -1;
    array<TileAnimation@> active_animations;

    // Blank tile selection state
    Tile@ blank_tile_to_set = null;
    int blank_tile_board_r = -1;
    int blank_tile_board_c = -1;

    // Board dimensions
    float board_size_px;
    float tile_size;
    float board_x;
    float board_y;

    // HUD Button properties
    array<string> button_labels = {"Swap", "Submit"};
    array<uint> button_colours = {0xFFDC143C, 0xFF32CD32}; // Crimson, LimeGreen
    float button_width = 150;
    float button_height = 50;
    float button_spacing = 15;
    float button_x = -550;

    // Combo break and shake effect variables
    int combo_breaks = 0;
    float shake_timer = 0;
    float shake_duration_initial = 0.0;
    float shake_intensity_initial = 0.0;

    ScrabbleGame(dictionary@ word_list, float board_scale = 1.25) // Board size adjusted
    {
        @g = get_scene();
        @this.word_list = word_list;
        this.board_size_px = 675.0 * board_scale;
        this.tile_size = this.board_size_px / BOARD_SIZE;
        
        // Initialize board arrays in the constructor
        board_layout.resize(BOARD_SIZE);
        board_tiles.resize(BOARD_SIZE);
        for(int i = 0; i < BOARD_SIZE; i++)
        {
            board_layout[i].resize(BOARD_SIZE);
            board_tiles[i].resize(BOARD_SIZE);
        }
    }

    void init(script@, scriptenemy@ self)
    {
        @this.self = self;
        @board_sprites = create_sprites();
        @hud_canvas = create_canvas(true, 22, 22);
        @ui_text = create_textfield();
        mouse.hud = true;
        mouse.scale_hud = true; // Fix for mouse coordinate mismatch
        
        initialize_board();

        // Initialize player, but wait for random seed to deal tiles
        @player = Player(0);
    }
    
    void initialize_game_state()
    {
        if(game_initialized) return;

        @tile_bag = AllTiles();
        for (int j = 0; j < RACK_SIZE; j++)
        {
            @player.PlayingTiles[j] = tile_bag.deal_tile();
        }
        game_initialized = true;
    }

    void trigger_combo_break()
    {
        combo_breaks++;
        start_shake(0.25, 2.5); // Use start_shake for consistency
        g.combo_break_count(combo_breaks);
    }

    void start_shake(float duration, float intensity) {
        if (shake_timer > 0 && intensity < shake_intensity_initial) {
            return;
        }
        shake_timer = duration;
        shake_duration_initial = duration;
        shake_intensity_initial = intensity;
    }

    // Method to check if a word is valid
    bool is_word_valid(const string &in word)
    {
        if(word.length() < 2) return false;
        // The word list is all uppercase.
        return word_list.exists(word);
    }
    
    // Calculate the score for a single word
    int calculate_word_score(const array<Tile@> &in word_tiles, const array<int> &in r_coords, const array<int> &in c_coords)
    {
        int word_score = 0;
        int word_multiplier = 1;
        int new_tiles_count = 0;

        for (uint i = 0; i < word_tiles.length(); i++)
        {
            Tile@ tile = word_tiles[i];
            int r = r_coords[i];
            int c = c_coords[i];
            int letter_score = tile.tileScore;

            if (tile.placed_this_turn)
            {
                new_tiles_count++;
                switch (board_layout[r][c])
                {
                    case DoubleLetter: letter_score *= 2; break;
                    case TripleLetter: letter_score *= 3; break;
                    case DoubleWord: word_multiplier *= 2; break;
                    case TripleWord: word_multiplier *= 3; break;
                    case Start: word_multiplier *= 2; break;
                }
            }
            word_score += letter_score;
        }

        word_score *= word_multiplier;
        
        // Bingo bonus for using all 7 tiles
        if(new_tiles_count == 7)
        {
            word_score += 50;
        }

        return word_score;
    }

    // Method to validate the move, score it, and advance the turn
    void validate_and_submit_move()
    {
        // 1. Find all newly placed tiles
        array<Tile@> new_tiles;
        array<int> new_tiles_r;
        array<int> new_tiles_c;
        for (int r = 0; r < BOARD_SIZE; ++r)
        {
            for (int c = 0; c < BOARD_SIZE; ++c)
            {
                if (@board_tiles[r][c] != null && board_tiles[r][c].placed_this_turn)
                {
                    new_tiles.insertLast(board_tiles[r][c]);
                    new_tiles_r.insertLast(r);
                    new_tiles_c.insertLast(c);
                }
            }
        }

        if (new_tiles.length() == 0)
        {
            puts("No tiles placed.");
            return;
        }

        // 2. Check placement validity (single line)
        bool is_horizontal = true;
        bool is_vertical = true;
        int first_r = new_tiles_r[0];
        int first_c = new_tiles_c[0];
        for (uint i = 1; i < new_tiles.length(); ++i)
        {
            if (new_tiles_r[i] != first_r) is_horizontal = false;
            if (new_tiles_c[i] != first_c) is_vertical = false;
        }

        if (!is_horizontal && !is_vertical && new_tiles.length() > 1)
        {
            puts("Invalid placement: Tiles must be in a single line.");
            trigger_combo_break();
            return_placed_tiles();
            return;
        }

        // 3. Check connectivity
        bool is_connected = false;
        if (first_move)
        {
            for(uint i = 0; i < new_tiles.length(); ++i)
            {
                if(new_tiles_r[i] == 7 && new_tiles_c[i] == 7)
                {
                    is_connected = true;
                    break;
                }
            }
            if(!is_connected)
            {
                puts("Invalid first move: Must cover the center tile.");
                trigger_combo_break();
                return_placed_tiles();
                return;
            }
        }
        else
        {
            for (uint i = 0; i < new_tiles.length(); ++i)
            {
                int r = new_tiles_r[i];
                int c = new_tiles_c[i];
                // Check neighbors
                if ((r > 0 && @board_tiles[r - 1][c] != null && !board_tiles[r - 1][c].placed_this_turn) ||
                    (r < BOARD_SIZE - 1 && @board_tiles[r + 1][c] != null && !board_tiles[r + 1][c].placed_this_turn) ||
                    (c > 0 && @board_tiles[r][c - 1] != null && !board_tiles[r][c - 1].placed_this_turn) ||
                    (c < BOARD_SIZE - 1 && @board_tiles[r][c + 1] != null && !board_tiles[r][c + 1].placed_this_turn))
                {
                    is_connected = true;
                    break;
                }
            }
            if (!is_connected)
            {
                puts("Invalid move: Must connect to existing tiles.");
                trigger_combo_break();
                return_placed_tiles();
                return;
            }
        }

        // 4. Collect and validate words
        array<string> words_to_check;
        array<array<Tile@>> word_tiles_list;
        array<array<int>> word_r_coords_list;
        array<array<int>> word_c_coords_list;
        
        if(is_horizontal || new_tiles.length() == 1)
        {
            // Main horizontal word
            int start_c = first_c;
            while(start_c > 0 && @board_tiles[first_r][start_c-1] != null) start_c--;
            int end_c = first_c;
            while(end_c < BOARD_SIZE-1 && @board_tiles[first_r][end_c+1] != null) end_c++;
            if(start_c != end_c)
            {
                 string word = "";
                 array<Tile@> tiles;
                 array<int> r_coords;
                 array<int> c_coords;
                 for(int c = start_c; c <= end_c; ++c) 
                 {
                     word += board_tiles[first_r][c].tileChar;
                     tiles.insertLast(board_tiles[first_r][c]);
                     r_coords.insertLast(first_r);
                     c_coords.insertLast(c);
                 }
                 words_to_check.insertLast(word);
                 word_tiles_list.insertLast(tiles);
                 word_r_coords_list.insertLast(r_coords);
                 word_c_coords_list.insertLast(c_coords);
            }
           
            // Vertical cross-words
            for(uint i = 0; i < new_tiles.length(); ++i)
            {
                int r = new_tiles_r[i];
                int c = new_tiles_c[i];
                int start_r = r;
                while(start_r > 0 && @board_tiles[start_r-1][c] != null) start_r--;
                int end_r = r;
                while(end_r < BOARD_SIZE-1 && @board_tiles[end_r+1][c] != null) end_r++;
                if(start_r != end_r)
                {
                    string word = "";
                    array<Tile@> tiles;
                    array<int> r_coords;
                    array<int> c_coords;
                    for(int cr = start_r; cr <= end_r; ++cr) 
                    {
                        word += board_tiles[cr][c].tileChar;
                        tiles.insertLast(board_tiles[cr][c]);
                        r_coords.insertLast(cr);
                        c_coords.insertLast(c);
                    }
                    words_to_check.insertLast(word);
                    word_tiles_list.insertLast(tiles);
                    word_r_coords_list.insertLast(r_coords);
                    word_c_coords_list.insertLast(c_coords);
                }
            }
        }

        if(is_vertical || new_tiles.length() == 1)
        {
            // Main vertical word
            int start_r = first_r;
            while(start_r > 0 && @board_tiles[start_r-1][first_c] != null) start_r--;
            int end_r = first_r;
            while(end_r < BOARD_SIZE-1 && @board_tiles[end_r+1][first_c] != null) end_r++;
            if(start_r != end_r)
            {
                 string word = "";
                 array<Tile@> tiles;
                 array<int> r_coords;
                 array<int> c_coords;
                 for(int r = start_r; r <= end_r; ++r) 
                 {
                     word += board_tiles[r][first_c].tileChar;
                     tiles.insertLast(board_tiles[r][first_c]);
                     r_coords.insertLast(r);
                     c_coords.insertLast(first_c);
                 }
                 words_to_check.insertLast(word);
                 word_tiles_list.insertLast(tiles);
                 word_r_coords_list.insertLast(r_coords);
                 word_c_coords_list.insertLast(c_coords);
            }

            // Horizontal cross-words
            for(uint i = 0; i < new_tiles.length(); ++i)
            {
                int r = new_tiles_r[i];
                int c = new_tiles_c[i];
                int start_c = c;
                while(start_c > 0 && @board_tiles[r][start_c-1] != null) start_c--;
                int end_c = c;
                while(end_c < BOARD_SIZE-1 && @board_tiles[r][end_c+1] != null) end_c++;
                if(start_c != end_c)
                {
                    string word = "";
                    array<Tile@> tiles;
                    array<int> r_coords;
                    array<int> c_coords;
                    for(int cc = start_c; cc <= end_c; ++cc) 
                    {
                        word += board_tiles[r][cc].tileChar;
                        tiles.insertLast(board_tiles[r][cc]);
                        r_coords.insertLast(r);
                        c_coords.insertLast(cc);
                    }
                    words_to_check.insertLast(word);
                    word_tiles_list.insertLast(tiles);
                    word_r_coords_list.insertLast(r_coords);
                    word_c_coords_list.insertLast(c_coords);
                }
            }
        }
        
        if (words_to_check.length() == 0)
        {
            puts("Invalid move: No word formed.");
            trigger_combo_break();
            return_placed_tiles();
            return;
        }

        // 5. Validate all found words
        for (uint i = 0; i < words_to_check.length(); ++i)
        {
            if (!is_word_valid(words_to_check[i]))
            {
                puts("Invalid word found: " + words_to_check[i]);
                trigger_combo_break();
                return_placed_tiles();
                return;
            }
        }
        
        puts("Valid move! Words formed: " + join(words_to_check, ", "));

        // 6. Calculate score
        int move_score = 0;
        for(uint i = 0; i < word_tiles_list.length(); ++i)
        {
            move_score += calculate_word_score(word_tiles_list[i], word_r_coords_list[i], word_c_coords_list[i]);
        }
        player.score += move_score;
        puts("Score for this move: " + move_score + ", Total Score: " + player.score);

        // 7. Finalize move and check for game end condition
        for (uint i = 0; i < new_tiles.length(); ++i)
        {
            new_tiles[i].placed_this_turn = false;
        }
        first_move = false;

        if(player.score > 100)
        {
            puts("Congratulations! You've scored over 100 points!");
            g.end_level(self.x(), self.y());
        }

        // 8. Refill player's hand with animation
        refill_hand_with_animation();
    }

    void refill_hand_with_animation()
    {
        float animation_delay = 0;
        const float animation_stagger = 0.1;
        const float animation_duration = 0.5;

        float bag_x = 750;
        float bag_y = 0;
        float current_tile_size = tile_size;
        float tile_spacing = 10;
        float rack_width = current_tile_size * RACK_SIZE + (RACK_SIZE + 1) * tile_spacing;
        float rack_x_start = -rack_width / 2;
        float rack_y_start = 450 - (current_tile_size + 20) - 20;

        array<int> empty_slots;
        for (int i = 0; i < RACK_SIZE; ++i)
        {
            if (@player.PlayingTiles[i] == null)
            {
                empty_slots.insertLast(i);
            }
        }

        for (int i = 0; i < int(empty_slots.length()); ++i)
        {
            int rack_index = empty_slots[i];
            Tile@ new_tile_data = tile_bag.deal_tile();
            if (@new_tile_data == null) break;

            float dest_x = rack_x_start + tile_spacing + rack_index * (current_tile_size + tile_spacing) + current_tile_size / 2;
            float dest_y = rack_y_start + tile_spacing + current_tile_size / 2;

            TileAnimation@ anim = TileAnimation(new_tile_data, bag_x, bag_y, dest_x, dest_y, animation_duration, animation_delay);
            anim.target_rack_index = rack_index;

            active_animations.insertLast(anim);
            animation_delay += animation_stagger;
        }
    }


    void initialize_board()
    {
        // Define the layout of the special tiles on the Scrabble board
        const TileType TW = TripleWord;
        const TileType DW = DoubleWord;
        const TileType TL = TripleLetter;
        const TileType DL = DoubleLetter;
        const TileType ST = Start;
        const TileType __ = Normal;

        const array<array<TileType>> layout = {
            {TW, __, __, DL, __, __, __, TW, __, __, __, DL, __, __, TW},
            {__, DW, __, __, __, TL, __, __, __, TL, __, __, __, DW, __},
            {__, __, DW, __, __, __, DL, __, DL, __, __, __, DW, __, __},
            {DL, __, __, DW, __, __, __, DL, __, __, __, DW, __, __, DL},
            {__, __, __, __, DW, __, __, __, __, __, DW, __, __, __, __},
            {__, TL, __, __, __, TL, __, __, __, TL, __, __, __, TL, __},
            {__, __, DL, __, __, __, DL, __, DL, __, __, __, DL, __, __},
            {TW, __, __, DL, __, __, __, ST, __, __, __, DL, __, __, TW},
            {__, __, DL, __, __, __, DL, __, DL, __, __, __, DL, __, __},
            {__, TL, __, __, __, TL, __, __, __, TL, __, __, __, TL, __},
            {__, __, __, __, DW, __, __, __, __, __, DW, __, __, __, __},
            {DL, __, __, DW, __, __, __, DL, __, __, __, DW, __, __, DL},
            {__, __, DW, __, __, __, DL, __, DL, __, __, __, DW, __, __},
            {__, DW, __, __, __, TL, __, __, __, TL, __, __, __, DW, __},
            {TW, __, __, DL, __, __, __, TW, __, __, __, DL, __, __, TW}
        };

        for (int r = 0; r < BOARD_SIZE; r++)
        {
            for (int c = 0; c < BOARD_SIZE; c++)
            {
                board_layout[r][c] = layout[r][c];
            }
        }
    }

    void handle_swap_click()
    {
        float rack_width = tile_size * RACK_SIZE + (RACK_SIZE + 1) * 10;
        float rack_height = tile_size + 20;
        float rack_x = -rack_width / 2;
        float rack_y = 450 - rack_height - 20;

        for (int i = 0; i < RACK_SIZE; i++)
        {
            if (@player.PlayingTiles[i] == null) continue;

            float tile_x = rack_x + 10 + i * (tile_size + 10);
            float tile_y = rack_y + 10;

            if (mouse.x >= tile_x && mouse.x <= tile_x + tile_size &&
                mouse.y >= tile_y && mouse.y <= tile_y + tile_size)
            {
                player.PlayingTiles[i].is_selected_for_swap = !player.PlayingTiles[i].is_selected_for_swap;
                return;
            }
        }
    }

    void execute_swap()
    {
        array<Tile@> tiles_to_swap;
        array<int> rack_indices_to_swap;

        for (int i = 0; i < RACK_SIZE; i++)
        {
            if (@player.PlayingTiles[i] != null && player.PlayingTiles[i].is_selected_for_swap)
            {
                tiles_to_swap.insertLast(player.PlayingTiles[i]);
                rack_indices_to_swap.insertLast(i);
                @player.PlayingTiles[i] = null;
            }
        }

        if (tiles_to_swap.length() == 0)
        {
            is_swapping = false;
            return;
        }

        // Add swapped tiles back to the bag
        for (uint i = 0; i < tiles_to_swap.length(); i++)
        {
            string c = tiles_to_swap[i].tileChar;
            tile_bag.letter_counts[c] = int(tile_bag.letter_counts[c]) + 1;
            tile_bag.total_tiles++;
        }

        // Animate tiles going to the bag
        float bag_x = 750;
        float bag_y = 0;

        for (uint i = 0; i < tiles_to_swap.length(); i++)
        {
            float from_x, from_y;
            get_rack_position(0, rack_indices_to_swap[i], from_x, from_y);
            TileAnimation@ anim = TileAnimation(tiles_to_swap[i], from_x, from_y, bag_x, bag_y, 0.5, i * 0.1);
            anim.is_swap = true;
            active_animations.insertLast(anim);
        }

        // Refill hand with animation after a delay
        refill_hand_with_animation();

        is_swapping = false;
        // Player's turn continues after swapping in solitaire mode
    }

    void step()
    {
        if(!game_initialized) return;

        mouse.step();

        // Process animations every frame
        for (int i = int(active_animations.length()) - 1; i >= 0; i--)
        {
            TileAnimation@ anim = active_animations[i];
            
            if (anim.delay > 0)
            {
                anim.delay -= DT;
                continue;
            }

            anim.progress += DT;
            if (anim.progress >= anim.duration)
            {
                if (anim.target_rack_index != -1) // Refill or return-to-rack animation
                {
                    @player.PlayingTiles[anim.target_rack_index] = anim.tile;
                }
                else if (anim.target_board_r != -1) // Move-to-board animation
                {
                    @board_tiles[anim.target_board_r][anim.target_board_c] = anim.tile;
                }
                active_animations.removeAt(i);
            }
        }

        if (is_selecting_blank)
        {
            handle_blank_selection();
            return;
        }

        if (is_swapping)
        {
            if (mouse.left_press)
            {
                // Check for tile selection clicks
                handle_swap_click();

                // Check for "Confirm" button click
                float confirm_button_y = -(button_height * 2 + button_spacing) / 2;
                if (mouse.x >= button_x && mouse.x <= button_x + button_width &&
                    mouse.y >= confirm_button_y && mouse.y <= confirm_button_y + button_height)
                {
                    execute_swap();
                }
            }
            return;
        }

        // Handle HUD button clicks
        if (mouse.left_press)
        {
            float current_y = -(button_height * 2 + button_spacing) / 2;
            for(uint i = 0; i < button_labels.length(); i++)
            {
                float x1 = button_x;
                float y1 = current_y;
                float x2 = x1 + button_width;
                float y2 = y1 + button_height;

                if (mouse.x >= x1 && mouse.x <= x2 && mouse.y >= y1 && mouse.y <= y2)
                {
                    if (i == 0) // Swap
                    {
                       is_swapping = true;
                       return_placed_tiles();
                    }
                    else if (i == 1) // Submit
                    {
                        validate_and_submit_move();
                    }
                }
                current_y += button_height + button_spacing;
            }
        }

        // Handle tile pickup from rack or board
        if (mouse.left_press && @selected_tile == null)
        {
            float rack_width = tile_size * RACK_SIZE + (RACK_SIZE + 1) * 10;
            float rack_height = tile_size + 20;
            float rack_x = -rack_width / 2;
            float rack_y = 450 - rack_height - 20;

            // Check rack first
            for (int i = 0; i < RACK_SIZE; i++)
            {
                if (@player.PlayingTiles[i] == null) continue;

                float tile_x = rack_x + 10 + i * (tile_size + 10);
                float tile_y = rack_y + 10;

                if (mouse.x >= tile_x && mouse.x <= tile_x + tile_size &&
                    mouse.y >= tile_y && mouse.y <= tile_y + tile_size)
                {
                    @selected_tile = player.PlayingTiles[i];
                    @player.PlayingTiles[i] = null;
                    selected_tile_rack_index = i;
                    drag_target_rack_index = i;
                    selected_tile_board_r = -1;
                    selected_tile_board_c = -1;
                    return; // Exit after picking up a tile
                }
            }

            // If no tile from rack, check board for movable tiles
            float world_mouse_x, world_mouse_y;
            g.mouse_world(self.player_index() >= 0 ? self.player_index() : 0, self.layer(), 10, world_mouse_x, world_mouse_y);

            if (world_mouse_x >= board_x && world_mouse_x <= board_x + board_size_px &&
                world_mouse_y >= board_y && world_mouse_y <= board_y + board_size_px)
            {
                int c = floor_int((world_mouse_x - board_x) / tile_size);
                int r = floor_int((world_mouse_y - board_y) / tile_size);

                if (is_on_board(r, c) && @board_tiles[r][c] != null && board_tiles[r][c].placed_this_turn)
                {
                    @selected_tile = board_tiles[r][c];
                    @board_tiles[r][c] = null;
                    selected_tile_rack_index = -1;
                    selected_tile_board_r = r;
                    selected_tile_board_c = c;
                }
            }
        }
        // Handle dragging a tile
        else if (@selected_tile != null && mouse.left_down)
        {
            if (selected_tile_rack_index != -1) // Dragging from rack
            {
                float rack_width = tile_size * RACK_SIZE + (RACK_SIZE + 1) * 10;
                float rack_x = -rack_width / 2;
                
                int hover_index = -1;
                for(int i = 0; i < RACK_SIZE; i++)
                {
                    float tile_center_x = rack_x + 10 + i * (tile_size + 10) + tile_size / 2;
                    if (mouse.x < tile_center_x)
                    {
                        hover_index = i;
                        break;
                    }
                }
                if (hover_index == -1) hover_index = RACK_SIZE;

                if (hover_index != drag_target_rack_index)
                {
                    // Cancel any existing rack animations
                    for(int i = int(active_animations.length()) - 1; i >= 0; i--) {
                        if(active_animations[i].from_rack_index != -1) {
                            // Snap unfinished animations to their end state before removing
                            TileAnimation@ anim = active_animations[i];
                            if (anim.target_rack_index >= 0 && anim.target_rack_index < RACK_SIZE) {
                                @player.PlayingTiles[anim.target_rack_index] = anim.tile;
                            }
                            active_animations.removeAt(i);
                        }
                    }
                    
                    // The tile being dragged is at selected_tile_rack_index.
                    // The potential new slot is hover_index.
                    // The actual index it will be inserted at is what we need to calculate.
                    int actual_target_index = hover_index;
                    if (hover_index > selected_tile_rack_index) {
                        actual_target_index--;
                    }

                    if (actual_target_index != selected_tile_rack_index) {
                        // Create a snapshot of the current rack to calculate animations from
                        array<Tile@> original_rack = player.PlayingTiles;

                        // Create the new logical arrangement of tiles
                        array<Tile@> next_rack_state;
                        for(int i = 0; i < RACK_SIZE; i++) {
                            if (i != selected_tile_rack_index) {
                                next_rack_state.insertLast(player.PlayingTiles[i]);
                            }
                        }
                        next_rack_state.insertAt(actual_target_index, selected_tile);

                        // Animate tiles that need to move
                        for (int i = 0; i < RACK_SIZE; i++) {
                            if (i == selected_tile_rack_index) continue;

                            Tile@ tile_to_move = original_rack[i];
                            if (@tile_to_move == null) continue;

                            int new_index = -1;
                            for (int j = 0; j < RACK_SIZE; j++) {
                                if (@next_rack_state[j] == @tile_to_move) {
                                    new_index = j;
                                    break;
                                }
                            }

                            if (new_index != -1 && new_index != i) {
                                float from_x, from_y, to_x, to_y;
                                get_rack_position(0, i, from_x, from_y);
                                get_rack_position(0, new_index, to_x, to_y);
                                
                                TileAnimation@ anim = TileAnimation(tile_to_move, from_x, from_y, to_x, to_y, 0.2);
                                anim.from_rack_index = i;
                                anim.target_rack_index = new_index;
                                active_animations.insertLast(anim);
                            }
                        }
                        
                        // Update the logical rack, placing a placeholder for the dragged tile
                        player.PlayingTiles = next_rack_state;
                        @player.PlayingTiles[actual_target_index] = null;
                        selected_tile_rack_index = actual_target_index;
                    }
                    drag_target_rack_index = hover_index;
                }
            }
        }
        // Handle dropping a tile
        else if (mouse.left_release && @selected_tile != null)
        {
            bool placed = false;
            float world_mouse_x, world_mouse_y;
            g.mouse_world(self.player_index() >= 0 ? self.player_index() : 0, self.layer(), 10, world_mouse_x, world_mouse_y);

            // Check for drop on board
            if (world_mouse_x >= board_x && world_mouse_x <= board_x + board_size_px &&
                world_mouse_y >= board_y && world_mouse_y <= board_y + board_size_px)
            {
                int c = floor_int((world_mouse_x - board_x) / tile_size);
                int r = floor_int((world_mouse_y - board_y) / tile_size);

                if (is_on_board(r, c))
                {
                    // If target is empty, place it
                    if (@board_tiles[r][c] == null)
                    {
                        if (selected_tile.is_blank)
                        {
                            is_selecting_blank = true;
                            @blank_tile_to_set = selected_tile;
                            blank_tile_board_r = r;
                            blank_tile_board_c = c;
                            @selected_tile = null; // Don't return it to rack
                            return;
                        }

                        @board_tiles[r][c] = selected_tile;
                        selected_tile.placed_this_turn = true;
                        placed = true;
                    }
                    // If target is occupied by a turn-placed tile, swap
                    else if (board_tiles[r][c].placed_this_turn)
                    {
                        Tile@ tile_from_board = board_tiles[r][c];
                        @board_tiles[r][c] = selected_tile;
                        selected_tile.placed_this_turn = true;

                        // Put the swapped tile back to the selected tile's origin
                        if (selected_tile_rack_index != -1) // Origin was rack
                        {
                            @player.PlayingTiles[selected_tile_rack_index] = tile_from_board;
                            tile_from_board.placed_this_turn = false;
                        }
                        else // Origin was board
                        {
                            @board_tiles[selected_tile_board_r][selected_tile_board_c] = tile_from_board;
                        }
                        placed = true;
                    }
                }
            }
            
            // Check for drop on rack
            if (!placed && selected_tile_rack_index != -1)
            {
                // The tile is already logically in the correct spot, just need to fill the null placeholder
                @player.PlayingTiles[selected_tile_rack_index] = selected_tile;
                placed = true;
            }

            // If not placed, return to origin
            if (!placed)
            {
                if (selected_tile_rack_index != -1)
                {
                    if (selected_tile.is_blank)
                    {
                        selected_tile.tileChar = "-";
                    }
                    @player.PlayingTiles[selected_tile_rack_index] = selected_tile;
                }
                else
                {
                    @board_tiles[selected_tile_board_r][selected_tile_board_c] = selected_tile;
                }
            }

            // Reset selection state
            @selected_tile = null;
            selected_tile_rack_index = -1;
            drag_target_rack_index = -1;
            selected_tile_board_r = -1;
            selected_tile_board_c = -1;
        }
        // Handle right-click actions
        else if (mouse.right_press && @selected_tile == null)
        {
            bool removed_one = false;
            float world_mouse_x, world_mouse_y;
            g.mouse_world(self.player_index() >= 0 ? self.player_index() : 0, self.layer(), 10, world_mouse_x, world_mouse_y);

            // Check if right-clicking a specific, movable tile on the board
            if (world_mouse_x >= board_x && world_mouse_x <= board_x + board_size_px &&
                world_mouse_y >= board_y && world_mouse_y <= board_y + board_size_px)
            {
                int c = floor_int((world_mouse_x - board_x) / tile_size);
                int r = floor_int((world_mouse_y - board_y) / tile_size);

                if (is_on_board(r, c) && @board_tiles[r][c] != null && board_tiles[r][c].placed_this_turn)
                {
                    Tile@ tile_to_return = board_tiles[r][c];
                    if (tile_to_return.is_blank)
                    {
                        tile_to_return.tileChar = "-";
                    }
                    @board_tiles[r][c] = null;
                    tile_to_return.placed_this_turn = false;

                    // Find an empty rack slot to return the tile to
                    int empty_rack_slot = -1;
                    for(int i = 0; i < RACK_SIZE; i++)
                    {
                        if(@player.PlayingTiles[i] == null)
                        {
                            empty_rack_slot = i;
                            break;
                        }
                    }
                    if(empty_rack_slot != -1)
                    {
                         @player.PlayingTiles[empty_rack_slot] = tile_to_return;
                    }
                    // Note: If the rack is full, the tile will disappear. This case should be rare.
                    
                    removed_one = true;
                }
            }

            // If an individual tile wasn't removed, execute the "return all" logic
            if (!removed_one)
            {
                return_placed_tiles();
            }
        }
    }
    
    // Returns tiles placed in the current turn to the player's rack.
    void return_placed_tiles()
    {
        array<bool> rack_slot_reserved(RACK_SIZE, false);

        for (int r = 0; r < BOARD_SIZE; r++)
        {
            for (int c = 0; c < BOARD_SIZE; c++)
            {
                Tile@ tile = board_tiles[r][c];
                // Only return tiles that were placed this turn
                if (@tile != null && tile.placed_this_turn)
                {
                    if (tile.is_blank)
                    {
                        tile.tileChar = "-";
                    }
                    int rack_index = -1;
                    for(int i = 0; i < RACK_SIZE; i++)
                    {
                        if(@player.PlayingTiles[i] == null && !rack_slot_reserved[i])
                        {
                            rack_index = i;
                            rack_slot_reserved[i] = true;
                            break;
                        }
                    }

                    if (rack_index != -1)
                    {
                        float rack_width = tile_size * RACK_SIZE + (RACK_SIZE + 1) * 10;
                        float rack_height = tile_size + 20;
                        float rack_x = -rack_width / 2;
                        float rack_y = 450 - rack_height - 20;

                        float from_x = board_x + c * tile_size + tile_size / 2;
                        float from_y = board_y + r * tile_size + tile_size / 2;
                        
                        float from_sx, from_sy;
                        world_to_hud(from_x, from_y, from_sx, from_sy);
                        
                        float to_x = rack_x + 10 + rack_index * (tile_size + 10) + tile_size / 2;
                        float to_y = rack_y + 10 + tile_size / 2;

                        tile.placed_this_turn = false; // Reset flag
                        TileAnimation@ anim = TileAnimation(tile, from_sx, from_sy, to_x, to_y, 0.5);
                        anim.target_rack_index = rack_index;
                        active_animations.insertLast(anim);
                        @board_tiles[r][c] = null;
                    }
                }
            }
        }
    }

    // Helper to convert world coordinates to HUD coordinates for animations
    void world_to_hud(float world_x, float world_y, float &out hud_x, float &out hud_y)
    {
        camera@ cam = get_camera(self.player_index() >= 0 ? self.player_index() : 0);
        float screen_x, screen_y, screen_w, screen_h;
        cam.get_layer_draw_rect(0, self.layer(), screen_x, screen_y, screen_w, screen_h);
        
        hud_x = ((world_x - screen_x) / screen_w) * 1600 - 800;
        hud_y = ((world_y - screen_y) / screen_h) * 900 - 450;
    }

    void get_rack_position(int player_index, int rack_index, float &out x, float &out y)
    {
        float current_tile_size = tile_size;
        float tile_spacing = 10;
        float rack_width = current_tile_size * RACK_SIZE + (RACK_SIZE + 1) * tile_spacing;
        float rack_x_start = -rack_width / 2;
        float rack_y_start = 450 - (current_tile_size + 20) - 20;
        
        x = rack_x_start + tile_spacing + rack_index * (current_tile_size + tile_spacing) + current_tile_size / 2;
        y = rack_y_start + tile_spacing + current_tile_size / 2;
    }

    bool is_on_board(int r, int c)
    {
        return r >= 0 && r < BOARD_SIZE && c >= 0 && c < BOARD_SIZE;
    }

    void draw(float subframe)
    {
        // Shake logic at the beginning of draw
        float shake_x = 0;
        float shake_y = 0;
        if (shake_timer > 0)
        {
            // Use an easing function to make the shake fade out
            float current_intensity = shake_intensity_initial * ease_out_cubic(shake_timer / shake_duration_initial);
            shake_x = (frand() * 2 - 1) * current_intensity;
            shake_y = (frand() * 2 - 1) * current_intensity;
            shake_timer -= DT;
            if(shake_timer < 0) shake_timer = 0;
        }

        // Center the board and displace it upwards, applying shake
        board_x = self.x() - board_size_px / 2.0 + shake_x;
        board_y = self.y() - board_size_px / 2.0 - 75 + shake_y;

        // Draw the board background
        g.draw_rectangle_world(self.layer(), 0, board_x, board_y, board_x + board_size_px, board_y + board_size_px, 0, 0xFFD2B48C);

        // Draw the special tiles
        for (int r = 0; r < BOARD_SIZE; r++)
        {
            for (int c = 0; c < BOARD_SIZE; c++)
            {
                uint tile_colour;
                switch(board_layout[r][c])
                {
                    case TripleWord:    tile_colour = 0xFFDC143C; break;
                    case DoubleWord:    tile_colour = 0xFFFF69B4; break;
                    case TripleLetter:  tile_colour = 0xFF1E90FF; break;
                    case DoubleLetter:  tile_colour = 0xFFB0E0E6; break;
                    case Start:         tile_colour = 0xFFFFD700; break;
                    default:            tile_colour = 0xFFF5DEB3; break;
                }
                g.draw_rectangle_world(self.layer(), 1, 
                    board_x + c * tile_size, board_y + r * tile_size, 
                    board_x + (c + 1) * tile_size, board_y + (r + 1) * tile_size, 
                    0, tile_colour);
            }
        }

        // Draw the placed tiles
        for (int r = 0; r < BOARD_SIZE; r++)
        {
            for (int c = 0; c < BOARD_SIZE; c++)
            {
                Tile@ tile = board_tiles[r][c];
                if (@tile != null)
                {
                    draw_tile_on_board(tile, r, c);
                }
            }
        }

        // Draw grid lines
        const float line_width = 1.5; 
        const uint line_colour = 0xFF000000;
        for (int r = 0; r <= BOARD_SIZE; r++)
        {
            g.draw_line_world(self.layer(), 2, board_x, board_y + r * tile_size, board_x + board_size_px, board_y + r * tile_size, line_width, line_colour);
        }
        for (int c = 0; c <= BOARD_SIZE; c++)
        {
            g.draw_line_world(self.layer(), 2, board_x + c * tile_size, board_y, board_x + c * tile_size, board_y + board_size_px, line_width, line_colour);
        }
        
        // Draw the player's tile rack HUD
        draw_hud(subframe);
        
        // Draw the selected tile at the world mouse cursor position
        if (@selected_tile != null)
        {
            float world_mouse_x, world_mouse_y;
            g.mouse_world(self.player_index() >= 0 ? self.player_index() : 0, self.layer(), 10, world_mouse_x, world_mouse_y);
            draw_tile_at_world_pos(selected_tile, world_mouse_x, world_mouse_y);
        }
    }

    void draw_tile_on_board(Tile@ tile, int r, int c)
    {
        float tile_world_x = board_x + c * tile_size;
        float tile_world_y = board_y + r * tile_size;
        draw_tile_at_world_pos(tile, tile_world_x + tile_size / 2, tile_world_y + tile_size / 2);
    }
    
    void draw_tile_at_world_pos(Tile@ tile, float x, float y)
    {
        float tile_world_x = x - tile_size / 2;
        float tile_world_y = y - tile_size / 2;

        g.draw_rectangle_world(self.layer(), 22, tile_world_x, tile_world_y, tile_world_x + tile_size, tile_world_y + tile_size, 0, 0xFFDEB887); // BurlyWood
        
        // Use an approved font and size for the letter
        ui_text.set_font("sans_bold", 26);
        ui_text.text(tile.tileChar);
        ui_text.colour(0xFF000000);
        ui_text.align_horizontal(0);
        ui_text.align_vertical(0);
        ui_text.draw_world(self.layer(), 23, tile_world_x + tile_size / 2, tile_world_y + tile_size / 2 - 5, 1, 1, 0);

        // Use an approved font and size for the score, and scale it down
        ui_text.set_font("sans_bold", 20);
        ui_text.text("" + tile.tileScore);
        ui_text.align_horizontal(1);
        ui_text.align_vertical(1);
        ui_text.draw_world(self.layer(), 23, tile_world_x + tile_size - 4, tile_world_y + tile_size - 4, 0.7, 0.7, 0);
        
        // Draw outline
        g.draw_line_world(self.layer(), 24, tile_world_x, tile_world_y, tile_world_x + tile_size, tile_world_y, 2.0, 0xFF8B4513);
        g.draw_line_world(self.layer(), 24, tile_world_x, tile_world_y, tile_world_x, tile_world_y + tile_size, 2.0, 0xFF8B4513);
        g.draw_line_world(self.layer(), 24, tile_world_x + tile_size, tile_world_y, tile_world_x + tile_size, tile_world_y + tile_size, 2.0, 0xFF8B4513);
        g.draw_line_world(self.layer(), 24, tile_world_x, tile_world_y + tile_size, tile_world_x + tile_size, tile_world_y + tile_size, 2.0, 0xFF8B4513);
    }

    void draw_hud(float subframe)
    {
        // Player 0 (Human) Rack
        draw_player_rack(subframe);
        
        // Draw HUD buttons
        if (is_swapping)
        {
            // Draw "Confirm" button
            float confirm_button_y = -(button_height * 2 + button_spacing) / 2;
            draw_button("Confirm", button_x, confirm_button_y, button_width, button_height, 0xFF4CAF50); // Green
        }
        else if (!is_selecting_blank)
        {
            float current_y = -(button_height * 2 + button_spacing) / 2;
            for(uint i = 0; i < button_labels.length(); i++)
            {
                draw_button(button_labels[i], button_x, current_y, button_width, button_height, button_colours[i]);
                current_y += button_height + button_spacing;
            }
        }
        
        // Draw animations
        for (int i = 0; i < int(active_animations.length()); i++)
        {
            TileAnimation@ anim = active_animations[i];

            if (anim.delay > 0) continue;

            float t = ease_in_out_cubic(anim.progress / anim.duration);
            float x = lerp(anim.from_x, anim.to_x, t);
            float y = lerp(anim.from_y, anim.to_y, t);
            
            draw_tile_on_hud(anim.tile, x - tile_size / 2, y - tile_size / 2, tile_size);
        }

        if (is_selecting_blank)
        {
            draw_blank_selection_ui();
        }
        
        // Draw Score
        ui_text.set_font("sans_bold", 36);
        ui_text.text("Score: " + player.score);
        ui_text.colour(0xFFFFFFFF);
        ui_text.align_horizontal(1);
        ui_text.align_vertical(-1);
        hud_canvas.draw_text(ui_text, 800 - 20, -450 + 20, 1, 1, 0);
    }

    void draw_blank_selection_ui()
    {
        // Draw a semi-transparent background overlay
        hud_canvas.draw_rectangle(-800, -450, 800, 450, 0, 0xAA000000);

        const int letters_per_row = 7;
        const float button_size = 60;
        const float button_spacing = 10;
        const float grid_width = letters_per_row * (button_size + button_spacing) - button_spacing;
        const float grid_height = 4 * (button_size + button_spacing) - button_spacing;
        const float start_x = -grid_width / 2;
        const float start_y = -grid_height / 2;

        for (int i = 0; i < 26; i++)
        {
            int r = i / letters_per_row;
            int c = i % letters_per_row;
            float x = start_x + c * (button_size + button_spacing);
            float y = start_y + r * (button_size + button_spacing);
            string letter = string::chr(int(65 + i));

            bool hovered = mouse.x >= x && mouse.x <= x + button_size && mouse.y >= y && mouse.y <= y + button_size;
            uint colour = hovered ? 0xFF666666 : 0xFF444444;

            hud_canvas.draw_rectangle(x, y, x + button_size, y + button_size, 0, colour);
            
            ui_text.set_font("sans_bold", 36);
            ui_text.text(letter);
            ui_text.colour(0xFFFFFFFF);
            ui_text.align_horizontal(0);
            ui_text.align_vertical(0);
            hud_canvas.draw_text(ui_text, x + button_size / 2, y + button_size / 2, 1, 1, 0);
        }
    }
    
    void handle_blank_selection()
    {
        if (!mouse.left_press) return;

        const int letters_per_row = 7;
        const float button_size = 60;
        const float button_spacing = 10;
        const float grid_width = letters_per_row * (button_size + button_spacing) - button_spacing;
        const float grid_height = 4 * (button_size + button_spacing) - button_spacing;
        const float start_x = -grid_width / 2;
        const float start_y = -grid_height / 2;

        for (int i = 0; i < 26; i++)
        {
            int r = i / letters_per_row;
            int c = i % letters_per_row;
            float x = start_x + c * (button_size + button_spacing);
            float y = start_y + r * (button_size + button_spacing);

            if (mouse.x >= x && mouse.x <= x + button_size && mouse.y >= y && mouse.y <= y + button_size)
            {
                string chosen_letter = string::chr(int(65 + i));
                blank_tile_to_set.tileChar = chosen_letter;
                
                @board_tiles[blank_tile_board_r][blank_tile_board_c] = blank_tile_to_set;
                blank_tile_to_set.placed_this_turn = true;

                // Reset state
                is_selecting_blank = false;
                @blank_tile_to_set = null;
                blank_tile_board_r = -1;
                blank_tile_board_c = -1;
                return;
            }
        }
    }

    void draw_button(string label, float x, float y, float w, float h, uint base_colour)
    {
        bool hovered = mouse.x >= x && mouse.x <= x + w && mouse.y >= y && mouse.y <= y + h;
        uint colour = base_colour;
        if(hovered)
        {
            uint r = (colour >> 16) & 0xFF;
            uint g = (colour >> 8) & 0xFF;
            uint b = colour & 0xFF;
            r = min(r + 40, uint(255));
            g = min(g + 40, uint(255));
            b = min(b + 40, uint(255));
            colour = (colour & 0xFF000000) | (r << 16) | (g << 8) | b;
        }

        hud_canvas.draw_rectangle(x, y, x + w, y + h, 0, colour);
        const float thickness = 2.0;
        hud_canvas.draw_line(x, y, x + w, y, thickness, 0xFFFFFFFF);
        hud_canvas.draw_line(x + w, y, x + w, y + h, thickness, 0xFFFFFFFF);
        hud_canvas.draw_line(x + w, y + h, x, y + h, thickness, 0xFFFFFFFF);
        hud_canvas.draw_line(x, y + h, x, y, thickness, 0xFFFFFFFF);
        
        ui_text.set_font("sans_bold", 26);
        ui_text.text(label);
        ui_text.colour(0xFFFFFFFF);
        ui_text.align_horizontal(0);
        ui_text.align_vertical(0);
        hud_canvas.draw_text(ui_text, x + w / 2, y + h / 2, 1, 1, 0);
    }

    void draw_player_rack(float subframe)
    {
		if (@player == null) return;

		// -- Calculate rack and tile dimensions in HUD pixels --
		float current_tile_size = tile_size;
		float tile_spacing = 10;
		float rack_width_hud = current_tile_size * RACK_SIZE + (RACK_SIZE + 1) * tile_spacing;
		float rack_height_hud = current_tile_size + 2 * tile_spacing;
		float rack_x_hud = -rack_width_hud / 2;
		float rack_y_hud = 450 - rack_height_hud - 20;

		// -- Get camera view for converting HUD coordinates to World coordinates --
		camera@ cam = get_camera(self.player_index() >= 0 ? self.player_index() : 0);
		float screen_x, screen_y, screen_w, screen_h;
		cam.get_layer_draw_rect(subframe, self.layer(), screen_x, screen_y, screen_w, screen_h);

		// -- Convert rack's HUD coordinates to World coordinates --
		float world_rack_x1 = screen_x + ((rack_x_hud + 800) / 1600.0) * screen_w;
		float world_rack_y1 = screen_y + ((rack_y_hud + 450) / 900.0) * screen_h;
		float world_rack_x2 = screen_x + (((rack_x_hud + rack_width_hud) + 800) / 1600.0) * screen_w;
		float world_rack_y2 = screen_y + (((rack_y_hud + rack_height_hud) + 450) / 900.0) * screen_h;
		
		// -- Draw the rack background in world space on a low sublayer (e.g., 0) --
		g.draw_rectangle_world(self.layer(), 0, world_rack_x1, world_rack_y1, world_rack_x2, world_rack_y2, 0, 0xCC5C4837);



        float rack_width = current_tile_size * RACK_SIZE + (RACK_SIZE + 1) * tile_spacing;
        float rack_height = current_tile_size + 2 * tile_spacing;
        float rack_x = -rack_width / 2;
        float rack_y = 450 - rack_height - 20;



        // hud_canvas.draw_rectangle(rack_x, rack_y, rack_x + rack_width, rack_y + rack_height, 0, 0xCC5C4837);

        for (int i = 0; i < RACK_SIZE; i++)
        {
            Tile@ tile = player.PlayingTiles[i];
            if (@tile == null) continue;

            bool is_this_tile_animating = false;
            for(int j = 0; j < int(active_animations.length()); j++)
            {
                if(@active_animations[j].tile == @tile)
                {
                    is_this_tile_animating = true;
                    break;
                }
            }
            if(is_this_tile_animating) continue;

            float tile_x = rack_x + tile_spacing + i * (current_tile_size + tile_spacing);
            float tile_y = rack_y + tile_spacing;
            draw_tile_on_hud(tile, tile_x, tile_y, current_tile_size);
        }
    }

    void draw_tile_on_hud(Tile@ tile, float x, float y, float size)
    {
        uint base_colour = tile.is_selected_for_swap ? 0xFFFFA500 : 0xFFDEB887; // Orange if selected, BurlyWood otherwise
        hud_canvas.draw_rectangle(x, y, x + size, y + size, 0, base_colour);
        hud_canvas.draw_line(x, y, x + size, y, 1.0, 0xFF8B4513);
        hud_canvas.draw_line(x, y, x, y + size, 1.0, 0xFF8B4513);
        hud_canvas.draw_line(x + size, y, x + size, y + size, 1.0, 0xFF8B4513);
        hud_canvas.draw_line(x, y + size, x + size, y + size, 1.0, 0xFF8B4513);

        // Use an approved font and size for the letter
        ui_text.set_font("sans_bold", int(26 * (size / tile_size)));
        ui_text.text(tile.tileChar);
        ui_text.colour(0xFF000000);
        ui_text.align_horizontal(0);
        ui_text.align_vertical(0);
        hud_canvas.draw_text(ui_text, x + size / 2, y + size / 2 - 5 * (size / tile_size), 1, 1, 0);

        // Use an approved font and size for the score, and scale it down
        ui_text.set_font("sans_bold", int(20 * (size / tile_size)));
        ui_text.text("" + tile.tileScore);
        ui_text.align_horizontal(1);
        ui_text.align_vertical(1);
        hud_canvas.draw_text(ui_text, x + size - 4, y + size - 4, 0.7, 0.7, 0);
    }
}

// Use a script-level constant to embed the text file.
const string EMBED_wordlist = "words.txt";

// --- Main Script Class ---
class script
{
    // Dictionary to store the word list
    dictionary word_list;
    replay_rand rrnd;
    ScrabbleGame@ game_instance;
    bool game_initialized = false;
    
    scene@ g;

    // --- Pause Detection & Shake ---
    bool is_paused = false;
    bool unpause_shake_pending = false;
    int has_stepped = 2;
    int frames = 0;
	bool level_ended = false;

    script()
    {
        @g = get_scene();
    }

    void on_level_start()
    {
        frames = 0;
        is_paused = false;
        unpause_shake_pending = false;
        has_stepped = 2;
        level_ended = false;
        
        // Load the raw text from the embedded file
        string all_words_raw = get_embed_value('wordlist');
        
        // Skip header (find the start of the third line)
        int start_index = 0;
        int newlines_found = 0;
        const uint len = all_words_raw.length();
        for (uint i = 0; i < len; ++i)
        {
            if (all_words_raw[i] == 10) // '\n'
            {
                newlines_found++;
                if (newlines_found == 2)
                {
                    start_index = i + 1;
                    break;
                }
            }
        }

        if (start_index == 0 && len > 0) {
            // Fallback if not enough newlines are found (e.g., small test file)
            puts("Warning: Could not find two header lines in wordlist.");
        }
        
        int word_start = -1;

        for (uint i = start_index; i < len; ++i)
        {
            uint chr = all_words_raw[i];
            
            // A word character is an uppercase letter
            bool is_word_char = (chr >= 65 && chr <= 90);

            if (is_word_char && word_start == -1)
            {
                // Start of a new word
                word_start = i;
            }
            else if (!is_word_char && word_start != -1)
            {
                // End of the current word
                string word = all_words_raw.substr(word_start, i - word_start);
                if (word.length() > 1)
                {
                    word_list.set(word, true);
                }
                word_start = -1;
            }
        }
        
        // Add the last word if the file doesn't end with a delimiter
        if (word_start != -1)
        {
            string word = all_words_raw.substr(word_start, len - word_start);
            if (word.length() > 1)
            {
                word_list.set(word, true);
            }
        }

        puts("Loaded " + word_list.getSize() + " words.");
    }
    
    void on_level_end() {
		level_ended = true;
	}

    void step(int)
    {
        has_stepped = 2;
        
        if (is_paused) {
            int current_breaks = g.combo_break_count();
            int new_breaks;
            if (current_breaks == 0) new_breaks = 1;
            else if (current_breaks == 1) new_breaks = 2;
            else if (current_breaks <= 3) new_breaks = 4;
            else if (current_breaks <= 5) new_breaks = 6;
            else new_breaks = current_breaks + 1;
            g.combo_break_count(new_breaks);

            is_paused = false;
        }

        if (unpause_shake_pending) {
            controllable@ p_controllable = controller_controllable(0);
            if (@p_controllable != null) {
                scriptenemy@ game_entity = p_controllable.as_scriptenemy();
                if (@game_entity != null) {
                    ScrabbleGame@ scrabble_instance = cast<ScrabbleGame@>(game_entity.get_object());
                    if (@scrabble_instance != null) {
                        scrabble_instance.start_shake(0.4, 4.0);
                    }
                }
            }
            unpause_shake_pending = false;
        }

        rrnd.step();
        if(!game_initialized && rrnd.seed_set())
        {
            if(@game_instance != null)
            {
                game_instance.initialize_game_state();
                game_initialized = true;
            }
        }
        frames++;
    }

    void step_fixed()
    {
        if (is_paused) return;
        if (frames <= 54 || level_ended) return;

        if (has_stepped <= 0) {
            is_paused = true;
            unpause_shake_pending = true;
        }
        has_stepped--;
    }

    void spawn_player(message@ msg)
    {
        // Pass the now-populated word_list to the game logic, for a single player.
        @game_instance = ScrabbleGame(@word_list);
        scriptenemy@ game_entity = create_scriptenemy(game_instance);
        game_entity.x(msg.get_float("x"));
        game_entity.y(msg.get_float("y"));
        msg.set_entity("player", @game_entity.as_entity());
    }
}
