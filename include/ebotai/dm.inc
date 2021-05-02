stock int GameModeDeathmatch(int client)
{
	if (!IsSlowThink[client])
		return;
	
	int Enemy = GetNearestPossibleEnemy(client);
	
	if(Enemy != -1)
	{
		TF2_FindPath(client, GetOrigin(Enemy));
	}
}
