stock int GameModeDeathmatch(int client)
{
	if (!IsSlowThink[client])
		return;
	
	int Enemy = GetNearestPossibleEnemy(client);
	
	if(Enemy != -1)
	{
		NavArea area = TheNavMesh.GetNearestNavArea_Vec(GetOrigin(Enemy), true, 50000.0, false, false, GetClientTeam(client));
		if(area != NavArea_Null)
		{
			float RandomPosition[3];
			
			area.GetRandomPoint(RandomPosition);
			
			TF2_FindPath(client, RandomPosition);
		}
		else
		{
			TF2_FindPath(client, GetOrigin(Enemy));
		}
	}
}
