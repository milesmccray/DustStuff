class Player
{
	
	int player_index;
	controllable@ player;
	
	void checkpoint_load()
	{
		@player = null;
	}
	
	void step(int num_entities)
	{
		if(@player == null)
		{
			entity@ e = controller_entity(player_index);
			@player = (@e != null ? e.as_controllable() : null);
			
			/*dustman@ dm = (@e != null ? e.as_dustman() : null);
			
			if(@dm != null)
			{
			}*/
		}
		
		if(@player == null)
			return;
		
		// Do stuff
	}
	
}