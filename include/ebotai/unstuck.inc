int StuckWarnings[MAXPLAYERS + 1];
float StuckTimer[MAXPLAYERS + 1];
float StuckArea[MAXPLAYERS + 1][3];

stock void CheckStuck(int client)
{
	if(HasEnemiesNear[client] || HasEnemyBuildingsNear[client])
		return;
	
	if (GetVectorDistance(GetOrigin(client), StuckArea[client]) < 75.0)
	{
		StuckWarnings[client] += 1
		
		if (StuckWarnings[client] > 10)
		{
			TryUnStuck[client] = true;
		}
	}
	else
	{
		if (StuckWarnings[client] > 0)
		{
			StuckWarnings[client] -= 1
		}
		
		if (StuckWarnings[client] < 5)
		{
			TryUnStuck[client] = false;
		}
	}
	
	if (TryUnStuck[client])
	{
		g_bJump[client] = true;
		
		float RandomPosition[3];
		float g_flSelectedMoveArea[3];
		RandomPosition = GetOrigin(client);
		
		if(IsSlowThink[client])
		{
			RandomPosition[0] += GetRandomFloat(-600.0, 600.0);
			RandomPosition[1] += GetRandomFloat(-600.0, 600.0);
		}
		
		NavArea movearea = TheNavMesh.GetNearestNavArea_Vec(GetOrigin(client), true, 25000.0, false, false, GetClientTeam(client));
		if(movearea != NavArea_Null)
		{
			movearea.GetCenter(g_flSelectedMoveArea);
			
			TF2_FindPath(client, g_flSelectedMoveArea);
		}
		else
		{
			TF2_FindPath(client, RandomPosition);
		}
	}
	
	if (StuckTimer[client] < GetGameTime())
	{
		StuckArea[client] = GetOrigin(client);
		
		StuckTimer[client] = GetGameTime() + 2.0;
	}
}