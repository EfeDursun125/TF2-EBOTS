int PLAction[MAXPLAYERS + 1];

float PLActionChangeTimer[MAXPLAYERS + 1];

stock int GameModePayload(int client)
{
	if (!IsSlowThink[client])
		return;
	
	float g_flRandomDefendArea[3];
	
	if(PLActionChangeTimer[client] < GetGameTime())
	{
		PLAction[client] = GetRandomInt(1,2);
		
		PLActionChangeTimer[client] = GetGameTime() + GetRandomFloat(30.0, 60.0);
	}
	
	char currentMap[PLATFORM_MAX_PATH];
	GetCurrentMap(currentMap, sizeof(currentMap));
	
	if(StrContains(currentMap, "pl" , false) != -1)
	{
		if(GetClientTeam(client) == 3 && PLAction[client] == 1) // push payload
		{
			int payload;
			while((payload = FindEntityByClassname(payload, "mapobj_cart_dispenser")) != INVALID_ENT_REFERENCE)
			{
				if(IsValidEntity(payload))
				{
					TF2_FindPath(client, GetOrigin(payload));
				}
			}
		}
		else if(GetClientTeam(client) == 3 && PLAction[client] == 2) // attack to nearest possible enemy
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
						PLAction[client] = 1;
						
						return;
					}
					
					TF2_FindPath(client, RandomPosition);
				}
				else // change action to push payload.
				{
					if(PF_Exists(client) && !PF_IsPathToVectorPossible(client, GetOrigin(Enemy)))
					{
						PLAction[client] = 1;
						
						return;
					}
					
					TF2_FindPath(client, GetOrigin(Enemy));
				}
			}
		}
		else if(GetClientTeam(client) == 2)
		{
			int payload;
			while((payload = FindEntityByClassname(payload, "mapobj_cart_dispenser")) != INVALID_ENT_REFERENCE)
			{
				if(IsValidEntity(payload))
				{
					float payloadpos[3];
					payloadpos = GetOrigin(payload);
					
					if(g_flDefendPosChangeTimer[client] < GetGameTime())
					{
						g_bFindNewDefendSpot[client] = true;
						
						g_flDefendPosChangeTimer[client] = GetGameTime() + GetRandomFloat(5.0, 15.0);
					}
					
					if(g_bFindNewDefendSpot[client])
					{
						if(IsSlowThink[client])
						{
							g_flRandomDefendArea[0] = payloadpos[0] + GetRandomFloat(-750.0, 750.0);
							g_flRandomDefendArea[1] = payloadpos[1] + GetRandomFloat(-750.0, 750.0);
							g_flRandomDefendArea[2] = payloadpos[2];
						}
						
						NavArea area = TheNavMesh.GetNearestNavArea_Vec(g_flRandomDefendArea, true, 5000.0, false, false, GetClientTeam(client));
						if(area != NavArea_Null)
						{
							area.GetRandomPoint(g_flSelectedDefendArea[client]);
							
							if(PF_Exists(client) && !PF_IsPathToVectorPossible(client, g_flSelectedDefendArea[client]))
							{
								g_bFindNewDefendSpot[client] = true;
								
								return;
							}
							
							g_bFindNewDefendSpot[client] = false;
						}
						else
						{
							g_bFindNewDefendSpot[client] = true;
							
							TF2_FindPath(client, GetOrigin(payload)); // at least go to payload
							
							return;
						}
					}
					
					TF2_FindPath(client, g_flSelectedDefendArea[client]);
				}
			}
		}
	}
}
