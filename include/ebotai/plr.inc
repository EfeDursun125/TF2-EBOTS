int PLRAction[MAXPLAYERS + 1];

float PLRActionChangeTimer[MAXPLAYERS + 1];

stock int GameModePayloadRace(int client)
{
	if (!IsSlowThink[client])
		return;
	
	if(PLRActionChangeTimer[client] < GetGameTime())
	{
		PLRAction[client] = GetRandomInt(1,2);
		
		PLRActionChangeTimer[client] = GetGameTime() + GetRandomFloat(30.0, 60.0);
	}
	
	char currentMap[PLATFORM_MAX_PATH];
	GetCurrentMap(currentMap, sizeof(currentMap));
	
	if(StrContains(currentMap, "plr_" , false) != -1)
	{
		if(PLRAction[client] == 1) // push payload
		{
			int payload;
			
			while((payload = FindEntityByClassname(payload, "mapobj_cart_dispenser")) != INVALID_ENT_REFERENCE)
			{
				int iTeamNumCart = GetEntProp(payload, Prop_Send, "m_iTeamNum");
				if (iTeamNumCart == GetClientTeam(client))
				{
					TF2_FindPath(client, GetOrigin(payload));
				}
			}
		}
		else if(PLRAction[client] == 2) // attack to nearest possible enemy
		{
			int Enemy = Client_GetClosest(client);
			
			if(Enemy != -1)
			{
				NavArea area = TheNavMesh.GetNearestNavArea_Vec(GetOrigin(Enemy), true, 50000.0, false, false, GetClientTeam(client));
				if(area != NavArea_Null)
				{
					float RandomPosition[3];
					
					area.GetRandomPoint(RandomPosition);
					
					if(PF_Exists(client) && !PF_IsPathToVectorPossible(client, RandomPosition)) // path is not possible? change action to push payload.
					{
						PLRAction[client] = 1;
						
						return;
					}
					
					TF2_FindPath(client, RandomPosition);
				}
				else // change action to push payload.
				{
					if(PF_Exists(client) && !PF_IsPathToVectorPossible(client, GetOrigin(Enemy)))
					{
						PLRAction[client] = 1;
						
						return;
					}
					
					TF2_FindPath(client, GetOrigin(Enemy));
				}
			}
		}
	}
}
