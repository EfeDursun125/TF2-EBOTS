int g_iCPAction[MAXPLAYERS+1];

stock int GameModeControlPoint(int client)
{
	if(IsSlowThink[client] && IsAttackDefendMap)
	{
		if(changeroletimer[client] < GetGameTime())
		{
			g_iCPAction[client] = GetRandomInt(1,3); // 1 = capture the point, 2 = defend, 3 = push.
			
			changeroletimer[client] = GetGameTime() + GetRandomFloat(10.0, 40.0);
		}
		
		char currentMap[PLATFORM_MAX_PATH];
		GetCurrentMap(currentMap, sizeof(currentMap));
		if(StrContains(currentMap, "cp_" , false) != -1 || StrContains(currentMap, "tc_" , false) != -1)
		{
			if((GetMyControlPointCount(client) + 1) >= GetEnemyControlPointCount(client)) // we must focus to control points for easy win.
			{
				int capturepoint = GetNearestEnemyControlPoint(client);
				if(capturepoint != -1)
				{
					float cappointpos[3];
					cappointpos = GetOrigin(capturepoint);
					
					float RandomPosition[3];
					
					RandomPosition[0] = cappointpos[0] + GetRandomFloat(-200.0, 200.0);
					RandomPosition[1] = cappointpos[1] + GetRandomFloat(-200.0, 200.0);
					RandomPosition[2] = cappointpos[2] + 50.0;
					
					TF2_FindPath(client, RandomPosition);
				}
				else // enemy team don't have a capturable any point, is that attack/defend map? let's defend!
				{
					float cappointpos[3];
					cappointpos = GetOrigin(GetNearestMyControlPoint(client));
					
					float g_flRandomDefendArea[3];
					
					if(g_flDefendPosChangeTimer[client] < GetGameTime())
					{
						g_bFindNewDefendSpot[client] = true;
						
						g_flDefendPosChangeTimer[client] = GetGameTime() + GetRandomFloat(5.0, 15.0);
					}
					
					if(g_bFindNewDefendSpot[client])
					{
						g_flRandomDefendArea[0] = cappointpos[0] + GetRandomFloat(-750.0, 750.0);
						g_flRandomDefendArea[1] = cappointpos[1] + GetRandomFloat(-750.0, 750.0);
						g_flRandomDefendArea[2] = cappointpos[2];
					}
					else
					{
						TF2_FindPath(client, g_flSelectedDefendArea[client]);
						
						return;
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
			}
			else if(GetMyControlPointCount(client) <= 0) // we are in attacking team?
			{
				int capturepoint = GetNearestEnemyControlPoint(client);
				if(capturepoint != -1)
				{
					float cappointpos[3];
					cappointpos = GetOrigin(capturepoint);
					
					float RandomPosition[3];
					
					RandomPosition[0] = cappointpos[0] + GetRandomFloat(-200.0, 200.0);
					RandomPosition[1] = cappointpos[1] + GetRandomFloat(-200.0, 200.0);
					RandomPosition[2] = cappointpos[2] + 50.0;
					
					TF2_FindPath(client, RandomPosition);
				}
			}
			else
			{
				if(g_iCPAction[client] == 1) // attack!
				{
					int capturepoint = GetNearestEnemyControlPoint(client);
					if(capturepoint != -1)
					{
						float cappointpos[3];
						cappointpos = GetOrigin(capturepoint);
						
						float RandomPosition[3];
						
						RandomPosition[0] = cappointpos[0] + GetRandomFloat(-200.0, 200.0);
						RandomPosition[1] = cappointpos[1] + GetRandomFloat(-200.0, 200.0);
						RandomPosition[2] = cappointpos[2] + 50.0;
						
						TF2_FindPath(client, RandomPosition);
					}
				}
				else if(g_iCPAction[client] == 2) // let's defend.
				{
					float cappointpos[3];
					cappointpos = GetOrigin(GetNearestMyControlPoint(client));
					
					float g_flRandomDefendArea[3];
					
					if(g_flDefendPosChangeTimer[client] < GetGameTime())
					{
						g_bFindNewDefendSpot[client] = true;
						
						g_flDefendPosChangeTimer[client] = GetGameTime() + GetRandomFloat(5.0, 15.0);
					}
					
					if(g_bFindNewDefendSpot[client])
					{
						g_flRandomDefendArea[0] = cappointpos[0] + GetRandomFloat(-750.0, 750.0);
						g_flRandomDefendArea[1] = cappointpos[1] + GetRandomFloat(-750.0, 750.0);
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
					
					TF2_FindPath(client, g_flSelectedDefendArea[client]);
				}
				else // let's push!
				{
					int Enemy = Client_GetClosest(client);
					
					if(Enemy != -1)
					{
						NavArea area = TheNavMesh.GetNearestNavArea_Vec(GetOrigin(Enemy), true, 50000.0, false, false, GetClientTeam(client));
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
		else
		{
			int capturepoint = GetNearestEnemyControlPoint(client);
			if(capturepoint != -1)
			{
				float cappointpos[3];
				cappointpos = GetOrigin(capturepoint);
				
				float RandomPosition[3];
				
				RandomPosition[0] = cappointpos[0] + GetRandomFloat(-200.0, 200.0);
				RandomPosition[1] = cappointpos[1] + GetRandomFloat(-200.0, 200.0);
				RandomPosition[2] = cappointpos[2] + 50.0;
				
				TF2_FindPath(client, RandomPosition);
			}
			else // arena logic.
			{
				int capturepoint2;
				if((capturepoint2 = FindEntityByClassname(capturepoint2, "team_control_point")) != INVALID_ENT_REFERENCE)
				{
					if(IsValidEntity(capturepoint2))// && GetEntProp(capturepoint2, Prop_Data, "m_bLocked") == 0) // lets capture.
					{
						float cappointpos2[3];
						cappointpos2 = GetOrigin(capturepoint2);
						
						float RandomPosition2[3];
						
						RandomPosition2[0] = cappointpos2[0] + GetRandomFloat(-200.0, 200.0);
						RandomPosition2[1] = cappointpos2[1] + GetRandomFloat(-200.0, 200.0);
						RandomPosition2[2] = cappointpos2[2] + 50.0;
						
						TF2_FindPath(client, RandomPosition2);
					}
					else
					{
						GameModeDeathmatch(client);
					}
				}
			}
		}
	}
	else if(IsSlowThink[client])
	{
		if(GetClientTeam(client) == 3)
		{
			int capturepoint = GetNearestEnemyControlPoint(client);
			if(capturepoint != -1)
			{
				float cappointpos[3];
				cappointpos = GetOrigin(capturepoint);
				
				float RandomPosition[3];
				
				RandomPosition[0] = cappointpos[0] + GetRandomFloat(-200.0, 200.0);
				RandomPosition[1] = cappointpos[1] + GetRandomFloat(-200.0, 200.0);
				RandomPosition[2] = cappointpos[2] + 50.0;
				
				TF2_FindPath(client, RandomPosition);
			}
		}
		else
		{
			int capturepoint = GetNearestDefendableControlPoint(client);
			if(capturepoint != -1)
			{
				float cappointpos[3];
				cappointpos = GetOrigin(capturepoint);
				
				float RandomPosition[3];
				
				RandomPosition[0] = cappointpos[0] + GetRandomFloat(-750.0, 750.0);
				RandomPosition[1] = cappointpos[1] + GetRandomFloat(-750.0, 750.0);
				RandomPosition[2] = cappointpos[2] + 50.0;
				
				TF2_FindPath(client, RandomPosition);
			}
		}
	}
}