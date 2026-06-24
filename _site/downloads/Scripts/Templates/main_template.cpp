#include '../lib/std.cpp';
#include 'Player.cpp';

/*
 * A basic template with a Player class
 */
class script
{
	
	int num_players;
	array<Player> players;
	
	script()
	{
		num_players = num_cameras();
		players.resize(num_players);
		
		for(int player_index = 0; player_index < num_players; player_index++)
		{
			players[player_index].player_index = player_index;
		}
	}
	
	void checkpoint_load()
	{
		for(int player_index = 0; player_index < num_players; player_index++)
		{
			players[player_index].checkpoint_load();
		}
	}
	
	void step(int num_entities)
	{
		for(int player_index = 0; player_index < num_players; player_index++)
		{
			players[player_index].step(num_entities);
		}
	}
	
}