stock void LookAtEnemiens(int client)
{
	if(HasEnemiesNear[client])
	{
		int i = NearestEnemy[client];
		
		if(!IsValidClient(i))
			return;
		
		float EnemyBodyOrigin[3];
		float BestAimPosition[3];
		float Distance;
		
		g_flLookPos[client] = GetEyePosition(NearestEnemy[client]);
		
		if(TF2_GetPlayerClass(client) == TFClass_Soldier && IsWeaponSlotActive(client, 0))
		{
			if(IsPointVisible(GetEyePosition(client), GetOrigin(NearestEnemy[client])))
			{
				if(PrimaryID[client] == 127)
				{
					float EntVec[3];
					GetEntPropVector(NearestEnemy[client], Prop_Data, "m_vecVelocity", EntVec);
					
					float AimPositon[3];
					
					AimPositon = GetOrigin(NearestEnemy[client]);
					
					AimPositon[2] += EntVec[2];
					AimPositon[1] += EntVec[1];
					AimPositon[0] += EntVec[0];
					
					TF2_LookAtPos(client, AimPositon, EBotAimSpeed[client]);
				}
				else
				{
					float EntVec[3];
					GetEntPropVector(NearestEnemy[client], Prop_Data, "m_vecVelocity", EntVec);
					
					float AimPositon[3];
					
					AimPositon = GetOrigin(NearestEnemy[client]);
					
					AimPositon[2] += (EntVec[2] / 2);
					AimPositon[1] += (EntVec[1] / 2);
					AimPositon[0] += (EntVec[0] / 2);
					
					TF2_LookAtPos(client, AimPositon, EBotAimSpeed[client]);
				}
			}
			else
			{
				TF2_LookAtPos(client, GetEyePosition(NearestEnemy[client]), EBotAimSpeed[client]);
			}
		}
		else if(TF2_GetPlayerClass(client) == TFClass_DemoMan && !IsWeaponSlotActive(client, 2))
		{
			GetClientEyePosition(NearestEnemy[client], BestAimPosition);
			
			if(PrimaryID[client] == 996)
			{
				Distance = GetVectorDistance(GetEyePosition(client), GetEyePosition(NearestEnemy[client]));
				
				BestAimPosition[2] += (Distance / 7) - 5;
				
				if(IsPointVisible(GetEyePosition(client), BestAimPosition))
				{
					TF2_LookAtPos(client, BestAimPosition, EBotAimSpeed[client]);
				}
				else
				{
					TF2_LookAtPos(client, GetEyePosition(NearestEnemy[client]), EBotAimSpeed[client]);
				}
			}
			else
			{
				if(GetVectorDistance(GetEyePosition(client), GetEyePosition(NearestEnemy[client])) < 500)
				{
					BestAimPosition[2] -= 15.0;
				}
				else if(GetVectorDistance(GetEyePosition(client), GetEyePosition(NearestEnemy[client])) < 750)
				{
					BestAimPosition[2] += 50.0;
				}
				else if(GetVectorDistance(GetEyePosition(client), GetEyePosition(NearestEnemy[client])) < 1000)
				{
					BestAimPosition[2] += 100.0;
				}
				else if(GetVectorDistance(GetEyePosition(client), GetEyePosition(NearestEnemy[client])) < 1250)
				{
					BestAimPosition[2] += 150.0;
				}
				else if(GetVectorDistance(GetEyePosition(client), GetEyePosition(NearestEnemy[client])) < 1500)
				{
					BestAimPosition[2] += 200.0;
				}
				else if(GetVectorDistance(GetEyePosition(client), GetEyePosition(NearestEnemy[client])) > 1500)
				{
					BestAimPosition[2] += 250.0;
				}
				
				if(IsPointVisible(GetEyePosition(client), BestAimPosition))
				{
					TF2_LookAtPos(client, BestAimPosition, EBotAimSpeed[client]);
				}
				else
				{
					TF2_LookAtPos(client, GetEyePosition(NearestEnemy[client]), EBotAimSpeed[client]);
				}
			}
		}
		else if(TF2_GetPlayerClass(client) == TFClass_Medic && IsWeaponSlotActive(client, 0))
		{
			if(PrimaryID[client] == 305 || PrimaryID[client] == 1079)
			{
				if(IsPointVisible(GetEyePosition(client), GetEyePosition(NearestEnemy[client])))
				{
					TF2_LookAtPos(client, GetEyePosition(NearestEnemy[client]), EBotAimSpeed[client]);
				}
				else
				{
					TF2_LookAtPos(client, GetOrigin(NearestEnemy[client]), EBotAimSpeed[client]);
				}
			}
			else
			{
				GetClientEyePosition(NearestEnemy[client], BestAimPosition);
				
				Distance = GetVectorDistance(GetEyePosition(client), GetEyePosition(NearestEnemy[client]));
				
				BestAimPosition[2] += (Distance / 17) - 2;
				
				if(IsPointVisible(GetEyePosition(client), BestAimPosition))
				{
					TF2_LookAtPos(client, BestAimPosition, EBotAimSpeed[client]);
				}
				else
				{
					TF2_LookAtPos(client, GetEyePosition(NearestEnemy[client]), EBotAimSpeed[client]);
				}
			}
		}
		else if(TF2_GetPlayerClass(client) == TFClass_Sniper && IsWeaponSlotActive(client, 0))
		{
			int iBone = LookupBone(NearestEnemy[client], "bip_head");
			if(iBone != -1)
			{
				float Nothing[3];
				float AimPositon[3];
				GetBonePosition(NearestEnemy[client], iBone, AimPositon, Nothing);
				AimPositon[2] += 3.0;
				
				if(PrimaryID[client] == 56 || PrimaryID[client] == 1005 || PrimaryID[client] == 1092)
				{
					float EntVec[3];
					GetEntPropVector(NearestEnemy[client], Prop_Data, "m_vecVelocity", EntVec);
					
					if(GetEntityFlags(NearestEnemy[client]) & FL_ONGROUND)
					{
						float AimPositon2[3];
						
						AimPositon2[2] = AimPositon[2] + (EntVec[2] / 2.5);
						AimPositon2[1] = AimPositon[1] + (EntVec[1] / 2.5);
						AimPositon2[0] = AimPositon[0] + (EntVec[0] / 2.5);
						
						if(IsPointVisible(GetEyePosition(client), AimPositon2))
						{
							TF2_LookAtPos(client, AimPositon2, EBotAimSpeed[client]);
						}
						else
						{
							TF2_LookAtPos(client, AimPositon, EBotAimSpeed[client]);
						}
					}
					else
					{
						float AimPositon2[3];
						
						AimPositon2[2] = AimPositon[2] + (EntVec[2] / 2);
						AimPositon2[1] = AimPositon[1] + (EntVec[1] / 2);
						AimPositon2[0] = AimPositon[0] + (EntVec[0] / 2);
						
						if(IsPointVisible(GetEyePosition(client), AimPositon2))
						{
							TF2_LookAtPos(client, AimPositon2, EBotAimSpeed[client]);
						}
						else
						{
							TF2_LookAtPos(client, AimPositon, EBotAimSpeed[client]);
						}
					}
				}
				else
				{
					TF2_LookAtPos(client, AimPositon, EBotAimSpeed[client]);
				}
			}
			else
			{
				TF2_LookAtPos(client, GetEyePosition(NearestEnemy[client]), EBotAimSpeed[client]);
			}
		}
		else if(TF2_GetPlayerClass(client) == TFClass_Spy && (PrimaryID[client] == 61 || PrimaryID[client] == 1006))
		{
			int iBone = LookupBone(NearestEnemy[client], "bip_head");
			if(iBone != -1)
			{
				float vNothing[3];
				float AimPositon[3];
				GetBonePosition(NearestEnemy[client], iBone, AimPositon, vNothing);
				AimPositon[2] += 3.0;
				
				TF2_LookAtPos(client, AimPositon, EBotAimSpeed[client]);
			}
			else
			{
				TF2_LookAtPos(client, GetEyePosition(NearestEnemy[client]), EBotAimSpeed[client]);
			}
		}
		else
		{
			if(IsWeaponSlotActive(client, 2))
			{
				TF2_LookAtPos(client, GetEyePosition(NearestEnemy[client]), EBotAimSpeed[client]);
			}
			else
			{
				GetClientEyePosition(NearestEnemy[client], EnemyBodyOrigin);
				EnemyBodyOrigin[2] -= 30.0;
				
				if(IsPointVisible(GetEyePosition(client), EnemyBodyOrigin))
				{
					TF2_LookAtPos(client, EnemyBodyOrigin, EBotAimSpeed[client]); // body
				}
				else if(IsPointVisible(GetEyePosition(client), EnemyBodyOrigin))
				{
					TF2_LookAtPos(client, GetEyePosition(NearestEnemy[client]), EBotAimSpeed[client]); // head
				}
				else
				{
					TF2_LookAtPos(client, GetOrigin(NearestEnemy[client]), EBotAimSpeed[client]); // foot
				}
			}
		}
	}
}

bool g_bLookAround[MAXPLAYERS+1];
float g_flLookTimer[MAXPLAYERS + 1];

float LookAreaTimer[MAXPLAYERS + 1];

stock void TF2_LookAround(int client)
{
	g_bLookAround[client] = true;
	
	if(IsSlowThink[client] && ChanceOf(EBotSenseChance[client]) && !HasFriendsNear[client]) // no any friends near, who's footsteps is this or fire sounds?
	{
		for (int search = 1; search <= MaxClients; search++)
		{
			if(!IsValidClient(search))
				continue;
			
			if(!IsPlayerAlive(search))
				continue;
			
			if(GetClientTeam(client) == GetClientTeam(search))
				continue;
			
			if(search == client)
				continue;
			
			if(!ClientIsMoving(search) && !IsAttacking(search))
				continue;
			
			if(GetVectorDistance(GetEyePosition(client), GetEyePosition(search), true) > 768.0)
				continue;
			
			float PlayerArea[3];
			
			NavArea area = TheNavMesh.GetNearestNavArea_Vec(GetEyePosition(search), true, 500.0, false, false, TEAM_ANY);
			if(area != NavArea_Null)
			{
				area.GetCenter(PlayerArea);
			}
			
			if(TF2_GetPlayerClass(client) == TFClass_Heavy && IsWeaponSlotActive(client, 0))
			{
				int Primary = GetPlayerWeaponSlot(client, TFWeaponSlot_Primary);
				
				if(IsValidEntity(Primary) && HasAmmo(Primary))
				{
					if(g_bHealthIsLow[client])
					{
						Attack2Timer[client] = GetGameTime() + 2.0;
					}
					else
					{
						Attack2Timer[client] = GetGameTime() + 4.0;
					}
				}
				else
				{
					Attack2Timer[client] = 0.0;
				}
			}
			
			g_flLookPos[client][0] = PlayerArea[0];
			g_flLookPos[client][1] = PlayerArea[1];
			g_flLookPos[client][2] = PlayerArea[2] + (g_flClientEyePos[client][2] - g_flClientOrigin[client][2]);
			
			g_flLookTimer[client] = GetGameTime() + GetRandomFloat(1.0, 2.0);
			
			return;
		}
	}
	
	if(g_bLookAround[client])
	{
		float BestLookPos[3];
		float SelectRandomNav[3];
		
		if(LookAreaTimer[client] < GetGameTime())
		{
			GetClientEyePosition(client, BestLookPos);
			
			BestLookPos[0] += GetRandomFloat(-1000.0, 1000.0);
			BestLookPos[1] += GetRandomFloat(-1000.0, 1000.0);
			
			NavArea area = TheNavMesh.GetNearestNavArea_Vec(BestLookPos, true, 500.0, false, false, TEAM_ANY);
			if(area != NavArea_Null)
			{
				area.GetCenter(SelectRandomNav);
			}
			
			LookAreaTimer[client] = GetGameTime() + 0.1;
		}
		
		if(IsPointVisible(GetEyePosition(client), SelectRandomNav) && GetVectorDistance(GetEyePosition(client), SelectRandomNav, true) > 300.0)
		{
			if(g_flLookTimer[client] < GetGameTime())
			{
				g_flLookPos[client][0] = SelectRandomNav[0];
				g_flLookPos[client][1] = SelectRandomNav[1];
				g_flLookPos[client][2] = SelectRandomNav[2] + (g_flClientEyePos[client][2] - g_flClientOrigin[client][2]);
				
				g_flLookTimer[client] = GetGameTime() + GetRandomFloat(1.5, 2.25);
			}
		}
		else if(ClientIsMoving(client) && IsSlowThink[client])
		{
			NavArea area = TheNavMesh.GetNearestNavArea_Vec(PathAhead[client], true, 500.0, false, false, TEAM_ANY);
			if(area != NavArea_Null)
			{
				area.GetCenter(g_flLookPos[client]);
				
				g_flLookPos[client][2] += (g_flClientEyePos[client][2] - g_flClientOrigin[client][2]);
			}
		}
		
		TF2_LookAtPos(client, g_flLookPos[client], EBotAimSpeed[client]);
	}
}
