int StuckWarnings[MAXPLAYERS + 1];
float StuckTimer[MAXPLAYERS + 1];
float StuckArea[MAXPLAYERS + 1][3];

stock void CheckStuck(int client)
{
	if(HasEnemiesNear[client] || HasEnemyBuildingsNear[client])
		return;
	
	if (GetVectorDistance(GetOrigin(client), StuckArea[client]) < 25.0)
	{
		StuckWarnings[client] += 1;
		
		if (StuckWarnings[client] > 10)
		{
			TryUnStuck[client] = true;
		}
	}
	else
	{
		if (StuckWarnings[client] > 0)
		{
			StuckWarnings[client] -= 1;
		}
		
		if (StuckWarnings[client] < 5)
		{
			TryUnStuck[client] = false;
		}
	}
	
	if (StuckTimer[client] < GetGameTime())
	{
		StuckArea[client] = GetOrigin(client);
		
		StuckTimer[client] = GetGameTime() + 2.0;
	}
}