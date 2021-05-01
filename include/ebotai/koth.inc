int g_iKothAction[MAXPLAYERS+1];

float changeroletimer[MAXPLAYERS+1];

bool g_bFindNewDefendSpot[MAXPLAYERS + 1];
float g_flDefendPosChangeTimer[MAXPLAYERS + 1];
float g_flSelectedDefendArea[MAXPLAYERS + 1][3];

stock int GameModeKoth(int client)
{
	char currentMap[PLATFORM_MAX_PATH];
	GetCurrentMap(currentMap, sizeof(currentMap));
	//g_iKothAction[client] = 1; // 1 = defend point, 2 = attack
	
	if(changeroletimer[client] < GetGameTime())
	{
		g_iKothAction[client] = GetRandomInt(1,2);
		
		changeroletimer[client] = GetGameTime() + GetRandomFloat(30.0, 90.0);
	}
	
	if(StrContains(currentMap, "koth_" , false) != -1 && IsSlowThink[client])
	{
		float g_flRandomDefendArea[3];
		
		int capturepoint;
		if((capturepoint = FindEntityByClassname(capturepoint, "team_control_point")) != INVALID_ENT_REFERENCE)
		{
			if(IsValidEntity(capturepoint) && GetClientTeam(client) != GetTeamNumber(capturepoint)) // lets capture.
			{
				float cappointpos[3];
				GetEntPropVector(capturepoint, Prop_Send, "m_vecOrigin", cappointpos);
				
				if(IsSlowThink[client])
				{
					g_flRandomDefendArea[0] = cappointpos[0] + GetRandomFloat(-200.0, 200.0);
					g_flRandomDefendArea[1] = cappointpos[1] + GetRandomFloat(-200.0, 200.0);
					g_flRandomDefendArea[2] = cappointpos[2] + 50.0;
				}
				
				TF2_FindPath(client, g_flRandomDefendArea);
			}
			else if(IsValidEntity(capturepoint) && GetClientTeam(client) == GetTeamNumber(capturepoint) && g_iKothAction[client] == 1) // now... defend time!
			{
				float cappointpos[3];
				cappointpos = GetOrigin(capturepoint);
				
				if(g_flDefendPosChangeTimer[client] < GetGameTime())
				{
					g_bFindNewDefendSpot[client] = true;
					
					g_flDefendPosChangeTimer[client] = GetGameTime() + GetRandomFloat(5.0, 15.0);
				}
				
				if(g_bFindNewDefendSpot[client])
				{
					if(IsSlowThink[client])
					{
						g_flRandomDefendArea[0] = cappointpos[0] + GetRandomFloat(-1000.0, 1000.0);
						g_flRandomDefendArea[1] = cappointpos[1] + GetRandomFloat(-1000.0, 1000.0);
						g_flRandomDefendArea[2] = cappointpos[2];
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
					}
				}
				
				TF2_FindPath(client, g_flSelectedDefendArea[client]);
			}
			else if(IsValidEntity(capturepoint) && GetClientTeam(client) == GetTeamNumber(capturepoint) && g_iKothAction[client] == 2) // stop defending, push!
			{
				if(IsSlowThink[client])
				{
					int Enemy = Client_GetClosest(client);
					
					if(Enemy != -1)
					{
						NavArea area = TheNavMesh.GetNearestNavArea_Vec(g_flRandomDefendArea, true, 5000.0, false, false, GetClientTeam(client));
						if(area != NavArea_Null)
						{
							float SelectedArea[3];
							
							area.GetRandomPoint(SelectedArea);
							
							if(PF_Exists(client) && !PF_IsPathToVectorPossible(client, SelectedArea))
							{
								g_iKothAction[client] = 1;
								return;
							}
							
							TF2_FindPath(client, SelectedArea);
						}
						else
						{
							if(PF_Exists(client) && !PF_IsPathToVectorPossible(client, GetOrigin(Enemy)))
							{
								g_iKothAction[client] = 1;
								return;
							}
							
							TF2_FindPath(client, GetOrigin(Enemy));
						}
					}
					else
					{
						g_iKothAction[client] = 1;
					}
				}
			}
		}
	}
}
