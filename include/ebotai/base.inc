stock void BaseAI(int client) // this is base of the ai
{
	Look(client);
	
	Check(client);
	
	if(IsSlowThink[client])
	{
		FindEnemiens(client);
		
		Attack(client);
		
		if(!HasEnemiesNear[client]) // for optimize
			GetNearestEnemyBuilding(client);
		
		Movement(client);
		
		CheckStuck(client);
		
		//GetNearestEyeBallBoss(client);
		
		if(TF2_GetPlayerClass(client) == TFClass_Medic)
		{
			FindFriendsForHeal(client);
		}
		else if(ProtectTeammateTime[client] > GetGameTime())
		{
			FindFriendsWithLowHealth(client);
		}
		else
		{
			FindFriends(client);
		}
		
		if(GetConVarInt(EBotUseVoiceline) == 1)
		{
			UseVoiceCommand(client);
		}
		
		if(UseTeleporter[client])
		{
			GetNearestTeleporter(client);
		}
	}
}