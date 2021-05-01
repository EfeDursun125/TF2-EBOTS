stock void SlenderBaseAI(int client) // currently its buggy, bots can't see bosses or bots tries go to already picked pages. (maybe its a search bug? cuz bots never tries attack to already picked pages)
{
	SlenderMovement(client);
	
	SlenderLook(client);
	
	Check(client);
	
	SlenderAttack(client);
	
	CheckStuck(client);
	
	if(IsSlowThink[client])
	{
		FindFriends(client);
		
		FindMonsters(client);
	}
}