class script
{
	
	[text] bool enabled;
	
	scene@ g;
	
	script()
	{
		@g = get_scene();
	}
	
	void editor_step()
	{
		if(!enabled)
			return;
	}
	
	void editor_draw(float sub_frame)
	{
		if(!enabled)
			return;
	}
	
}