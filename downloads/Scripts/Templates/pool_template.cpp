class ==OBJECTPool==
{
	
	private uint pool_index = 0;
	private uint pool_size = 8;
	private array<OBJECT@> pool(pool_size);
	
	OBJECT@ get()
	{
		if(pool_index > 0)
			return pool[--pool_index];
		
		return OBJECT();
	}
	
	void release(OBJECT@ obj)
	{
		if(pool_index == pool_size)
		{
			pool_size += 8;
			pool.resize(pool_size);
		}
		
		@pool[pool_index++] = obj;
	}
	
}