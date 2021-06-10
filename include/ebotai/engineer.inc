float g_flRandomSentrySpotPos[MAXPLAYERS + 1][3];

float SentryBuildPosition[MAXPLAYERS + 1][3];

float DispenserBuildPosition[MAXPLAYERS + 1][3];

float TeleporterEnterBuildPosition[MAXPLAYERS + 1][3];

int SentryHint[MAXPLAYERS + 1];

int TeleporterExitHint[MAXPLAYERS + 1];

bool HasABuildPosition[MAXPLAYERS + 1];

stock void EngineerMoveLogic(int client)
{
	if(WantsBuildSentryGun[client])
	{
		if(GetVectorDistance(GetOrigin(client), SentryBuildPosition[client]) <= 50.0 || GetVectorDistance(GetOrigin(client), GetOrigin(SentryHint[client])) <= 25.0)
		{
			if(IsWeaponSlotActive(client, 5))
			{
				ForcePressButton[client] |= IN_ATTACK;
			}
			else
			{
				FakeClientCommandThrottled(client, "build 2 0");
			}
		}
		
		if(GetUsableSentryHintsCount(client) > 0)
		{
			if (EngineerChangeSentrySpotTimer[client] < GetGameTime())
			{
				SentryHint[client] = FindRandomSentrySpot(client);
				
				EngineerChangeSentrySpotTimer[client] = GetGameTime() + GetRandomFloat(10.0, 20.0);
			}
			
			if(SentryHint[client] != -1 && IsValidEntity(SentryHint[client]) && IsValidEdict(SentryHint[client]))
			{
				TF2_FindPath(client, GetOrigin(SentryHint[client]));
				
				SetEntPropEnt(SentryHint[client], Prop_Send, "m_hOwnerEntity", client);
				
				return;
			}
			else if(IsSlowThink[client])
			{
				SentryHint[client] = FindRandomSentrySpot(client);
				
				EngineerChangeSentrySpotTimer[client] = GetGameTime() + GetRandomFloat(10.0, 20.0);
			}
		}
		else
		{
			TF2_FindSentrySpot(client);
		}
		
		if(PF_Exists(client) && PF_IsPathToVectorPossible(client, SentryBuildPosition[client]))
		{
			TF2_FindPath(client, SentryBuildPosition[client]);
		}
		else
		{
			ObjectiveMovement(client);
		}
		
		return;
	}
	
	if(WantsMoveSentryGun[client] && SentryGun[client] != -1 && IsValidEntity(SentryGun[client]))
	{
		if(GetEntProp(client, Prop_Send, "m_bCarryingObject") && IsValidEntity(GetEntPropEnt(client, Prop_Send, "m_hCarriedObject")))
		{
			if(GetVectorDistance(GetOrigin(client), SentryBuildPosition[client]) < 100.0)
			{
				if(IsWeaponSlotActive(client, 5))
				{
					EngineerMoveSentryTimer[client] = GetGameTime() + GetRandomFloat(90.0, 160.0);
					
					WantsMoveSentryGun[client] = false;
					
					g_bPickRandomSentrySpot[client] = true;
				}
				else
				{
					ForcePressButton[client] |= IN_ATTACK2;
				}
			}
			else
			{
				TF2_FindPath(client, SentryBuildPosition[client]);
			}
		}
		else
		{
			TF2_FindPath(client, GetOrigin(SentryGun[client]));
			
			if(GetVectorDistance(GetOrigin(client), GetOrigin(SentryGun[client])) < 100.0)
			{
				ForcePressButton[client] |= IN_ATTACK2;
			}
		}
		
		if(GetUsableSentryHintsCount(client) > 0)
		{
			if (EngineerChangeSentrySpotTimer[client] < GetGameTime())
			{
				SentryHint[client] = FindRandomSentrySpot(client);
				
				EngineerChangeSentrySpotTimer[client] = GetGameTime() + GetRandomFloat(10.0, 20.0);
			}
			
			if(SentryHint[client] != -1 && IsValidEntity(SentryHint[client]) && IsValidEdict(SentryHint[client]))
			{
				TF2_FindPath(client, GetOrigin(SentryHint[client]));
				
				SetEntPropEnt(SentryHint[client], Prop_Send, "m_hOwnerEntity", client);
				
				return;
			}
			else if(IsSlowThink[client])
			{
				SentryHint[client] = FindRandomSentrySpot(client);
				
				EngineerChangeSentrySpotTimer[client] = GetGameTime() + GetRandomFloat(10.0, 20.0);
			}
		}
		else
		{
			TF2_FindSentrySpot(client);
		}
		
		return;
	}
	
	if(WantsBuildDispenser[client] && SentryGun[client] != -1 && IsValidEntity(SentryGun[client]))
	{
		float SelectedArea[3];
		
		SelectedArea = GetOrigin(SentryGun[client]);
		
		SelectedArea[0] += GetRandomFloat(-200.0, 200.0);
		SelectedArea[1] += GetRandomFloat(-200.0, 200.0);
		
		NavArea buildposition = TheNavMesh.GetNearestNavArea_Vec(SelectedArea, true, 5000.0, false, false, GetClientTeam(client));
		if(buildposition != NavArea_Null)
		{
			buildposition.GetRandomPoint(DispenserBuildPosition[client]);
			
			TF2_FindPath(client, DispenserBuildPosition[client]);
		}
		else
		{
			TF2_FindPath(client, GetOrigin(SentryGun[client]));
		}
		
		if(IsWeaponSlotActive(client, 5))
		{
			ForcePressButton[client] |= IN_ATTACK;
		}
		else
		{
			FakeClientCommandThrottled(client, "build 0 0");
		}
		
		return;
	}
	
	if(WantsMoveDispenser[client] && Dispenser[client] != -1 && IsValidEntity(Dispenser[client]))
	{
		if(GetEntProp(client, Prop_Send, "m_bCarryingObject") && IsValidEntity(GetEntPropEnt(client, Prop_Send, "m_hCarriedObject")))
		{
			float SelectedArea[3];
			
			SelectedArea = GetOrigin(SentryGun[client]);
			
			SelectedArea[0] += GetRandomFloat(-200.0, 200.0);
			SelectedArea[1] += GetRandomFloat(-200.0, 200.0);
			
			NavArea buildposition = TheNavMesh.GetNearestNavArea_Vec(SelectedArea, true, 5000.0, false, false, GetClientTeam(client));
			if(buildposition != NavArea_Null)
			{
				buildposition.GetRandomPoint(DispenserBuildPosition[client]);
				
				TF2_FindPath(client, DispenserBuildPosition[client]);
			}
			else
			{
				TF2_FindPath(client, GetOrigin(SentryGun[client]));
			}
			
			if(GetVectorDistance(GetOrigin(client), DispenserBuildPosition[client]) < 75.0)
			{
				ForcePressButton[client] |= IN_ATTACK2;
				
				EngineerMoveDispenserTimer[client] = GetGameTime() + GetRandomFloat(120.0, 180.0);
				
				WantsMoveDispenser[client] = false;
			}
		}
		else
		{
			TF2_FindPath(client, GetOrigin(Dispenser[client]));
			
			if(GetVectorDistance(GetOrigin(client), GetOrigin(Dispenser[client])) < 100.0)
			{
				ForcePressButton[client] |= IN_ATTACK2;
			}
		}
		
		return;
	}
	
	if(WantsBuildTeleporterEnter[client])
	{
		if(IsWeaponSlotActive(client, 5))
		{
			ForcePressButton[client] |= IN_ATTACK;
		}
		else
		{
			FakeClientCommandThrottled(client, "build 1 0");
		}
		
		if(!HasABuildPosition[client])
		{
			float RandomPosition[3];
			
			RandomPosition[0] = PlayerSpawn[client][0] + GetRandomFloat(300.0, -300.0);
			RandomPosition[1] = PlayerSpawn[client][1] + GetRandomFloat(300.0, -300.0);
			RandomPosition[2] = PlayerSpawn[client][2];
			
			NavArea buildposition = TheNavMesh.GetNearestNavArea_Vec(RandomPosition, true, 5000.0, false, false, GetClientTeam(client));
			if(buildposition != NavArea_Null)
			{
				if(HasTFAttributes(buildposition, BLOCKED) || HasTFAttributes(buildposition, BLUE_SPAWN_ROOM) || HasTFAttributes(buildposition, RED_SPAWN_ROOM))
					return;
				
				buildposition.GetRandomPoint(TeleporterEnterBuildPosition[client]);
				
				if(!PF_IsPathToVectorPossible(client, TeleporterEnterBuildPosition[client]))
					return;
				
				TF2_FindPath(client, TeleporterEnterBuildPosition[client]);
			}
			else
			{
				WantsBuildTeleporterEnter[client] = false
			}
		}
		else
		{
			TF2_FindPath(client, TeleporterEnterBuildPosition[client]);
		}
		
		return;
	}
	
	if(WantsBuildTeleporterExit[client])
	{
		if(IsWeaponSlotActive(client, 5))
			ForcePressButton[client] |= IN_ATTACK;
		else
			FakeClientCommandThrottled(client, "build 1 1");
		
		if(GetEntProp(client, Prop_Send, "m_bCarryingObject") && IsValidEntity(GetEntPropEnt(client, Prop_Send, "m_hCarriedObject")))
			TeleporterExitHint[client] = FindRandomTeleporterExitSpot(client);
		
		if(IsValidEntity(TeleporterExitHint[client]))
			TF2_FindPath(client, GetOrigin(TeleporterExitHint[client]));
		else
			TeleporterExitHint[client] = FindRandomTeleporterExitSpot(client);
		
		return;
	}
	
	if(SentryGun[client] != -1 && IsValidEntity(SentryGun[client]) && IsValidEdict(SentryGun[client]) && (!SentryIsMaxLevel[client] || !SentryHealthIsFull[client]))
	{
		TF2_FindPath(client, GetOrigin(SentryGun[client]));
		
		return;
	}
	
	if(Dispenser[client] != -1 && IsValidEntity(Dispenser[client]) && IsValidEdict(Dispenser[client]) && (!DispenserIsMaxLevel[client] || !DispenserHealthIsFull[client]))
	{
		TF2_FindPath(client, GetOrigin(Dispenser[client]));
		
		return;
	}
	
	if(TeleporterEnter[client] != -1 && IsValidEntity(TeleporterEnter[client]) && IsValidEdict(TeleporterEnter[client]) && (!TeleporterEnterHealthIsFull[client] || !TeleporterEnterIsMaxLevel[client]))
	{
		TF2_FindPath(client, GetOrigin(TeleporterEnter[client]));
		
		return;
	}
	
	if(TeleporterExit[client] != -1 && IsValidEntity(TeleporterExit[client]) && IsValidEdict(TeleporterExit[client]) && (!TeleporterExitHealthIsFull[client] || !TeleporterExitIsMaxLevel[client]))
	{
		TF2_FindPath(client, GetOrigin(TeleporterExit[client]));
		
		return;
	}
	
	ObjectiveMovement(client);
}

stock int FindRandomSentrySpot(int client)
{
	int iEntity = -1;
	
	int iSelectedHint = -1;
	
	ArrayList SentryHints = new ArrayList();
	
	while((iEntity = FindEntityByClassname(iEntity, "bot_hint_sentrygun")) != -1)
	{
		if (!IsValidEntity(iEntity))
			continue;
		
		if (!IsValidEdict(iEntity))
			continue;
		
		//if (GetEntProp(iEntity, Prop_Data, "m_hOwnerEntity") != null)
		//	continue;
		
		if(GetEntProp(iEntity, Prop_Data, "m_isDisabled") == 1)
			continue;
			
		if(GetEnemyTeam(client) == GetTeamNumber(iEntity))
			continue;
		
		SentryHints.Push(iEntity);
	}
	
	if(SentryHints.Length > 0)
	{
		iSelectedHint = SentryHints.Get(GetRandomInt(0, SentryHints.Length - 1));
	}
	
	delete SentryHints;
	
	return iSelectedHint;
}

stock int FindRandomTeleporterExitSpot(int client)
{
	int iEntity = -1;
	
	int iSelectedHint = -1;
	
	ArrayList TeleporterHints = new ArrayList();
	
	while((iEntity = FindEntityByClassname(iEntity, "bot_hint_teleporter_exit")) != -1)
	{
		if (!IsValidEntity(iEntity))
			continue;
		
		if (!IsValidEdict(iEntity))
			continue;
		
		if(GetEntProp(iEntity, Prop_Data, "m_isDisabled") == 1)
			continue;
			
		if(GetEnemyTeam(client) == GetTeamNumber(iEntity))
			continue;
		
		TeleporterHints.Push(iEntity);
	}
	
	if(TeleporterHints.Length > 0)
	{
		iSelectedHint = TeleporterHints.Get(GetRandomInt(0, TeleporterHints.Length - 1));
	}
	
	delete TeleporterHints;
	
	return iSelectedHint;
}

stock void EngineerAimLogic(int client)
{
	if(WantsBuildDispenser[client] && IsWeaponSlotActive(client, 5))
	{
		TF2_LookAround(client);
		
		return;
	}
	
	if(SentryGun[client] != -1 && IsValidEntity(SentryGun[client]) && IsValidEdict(SentryGun[client]) && !IsMetalLow(client) && GetVectorDistance(GetEyePosition(client), GetOrigin(SentryGun[client])) < 150.0 && (!SentryIsMaxLevel[client] || !SentryHealthIsFull[client]))
	{
		if(IsPointVisibleTank(GetEyePosition(client), GetOrigin(SentryGun[client])))
		{
			TF2_LookAtPos(client, GetOrigin(SentryGun[client]), EBotAimSpeed[client]);
			
			if(IsWeaponSlotActive(client, 2))
			{
				AttackTimer[client] = GetGameTime() + 1.2;
				
				if(!TF2_IsNextToWall(client))
				{
					CrouchTime[client] = GetGameTime() + 1.2;
				}
			}
			else
			{
				EquipWeaponSlot(client, 2);
			}
			
			return;
		}
	}
	
	if(WantsMoveSentryGun[client] && SentryGun[client] != -1 && IsValidEntity(SentryGun[client]) && IsValidEdict(SentryGun[client]) && GetVectorDistance(GetOrigin(client), GetOrigin(SentryGun[client])) < 175.0)
	{
		if(!GetEntProp(client, Prop_Send, "m_bCarryingObject") && IsWeaponSlotActive(client, 5))
		{
			TF2_LookAtPos(client, GetOrigin(SentryGun[client]), EBotAimSpeed[client]);
			
			return;
		}
		else
		{
			ForcePressButton[client] |= IN_ATTACK;
		}
	}
	
	if(Dispenser[client] != -1 && IsValidEntity(Dispenser[client]) && IsValidEdict(Dispenser[client]) && !IsMetalLow(client) && GetVectorDistance(GetEyePosition(client), GetOrigin(Dispenser[client])) < 150.0 && (!DispenserIsMaxLevel[client] || !DispenserHealthIsFull[client]))
	{
		if(IsPointVisibleTank(GetEyePosition(client), GetOrigin(Dispenser[client])))
		{
			TF2_LookAtPos(client, GetOrigin(Dispenser[client]), EBotAimSpeed[client]);
			
			if(IsWeaponSlotActive(client, 2))
			{
				AttackTimer[client] = GetGameTime() + 1.2;
				
				CrouchTime[client] = GetGameTime() + 1.2;
			}
			else
			{
				EquipWeaponSlot(client, 2);
			}
			
			return;
		}
	}
	
	if(TeleporterEnter[client] != -1 && !IsMetalLow(client) && IsValidEntity(TeleporterEnter[client]) && IsValidEdict(TeleporterEnter[client]) && GetVectorDistance(GetEyePosition(client), GetOrigin(TeleporterEnter[client])) < 150.0 && (!TeleporterEnterIsMaxLevel[client] || !TeleporterEnterHealthIsFull[client]))
	{
		if(IsPointVisibleTank(GetEyePosition(client), GetOrigin(TeleporterEnter[client])))
		{
			TF2_LookAtPos(client, GetOrigin(TeleporterEnter[client]), EBotAimSpeed[client]);
			
			if(IsWeaponSlotActive(client, 2))
			{
				AttackTimer[client] = GetGameTime() + 1.2;
				
				CrouchTime[client] = GetGameTime() + 1.2;
			}
			else
			{
				EquipWeaponSlot(client, 2);
			}
			
			return;
		}
	}
	
	if(HasEnemiesNear[client])
	{
		LookAtEnemiens(client);
		SelectBestCombatWeapon(client);
		
		return;
	}
	
	if(SentryGun[client] != -1 && IsValidEntity(SentryGun[client]) && IsValidEdict(SentryGun[client]) && GetVectorDistance(GetEyePosition(client), GetOrigin(SentryGun[client])) < 150.0)
	{
		if(IsPointVisibleTank(GetEyePosition(client), GetOrigin(SentryGun[client])))
		{
			TF2_LookAtPos(client, GetOrigin(SentryGun[client]), EBotAimSpeed[client]);
			
			if(IsWeaponSlotActive(client, 2))
			{
				AttackTimer[client] = GetGameTime() + 1.2;
				
				if(!TF2_IsNextToWall(client))
				{
					CrouchTime[client] = GetGameTime() + 1.2;
				}
			}
			else
			{
				EquipWeaponSlot(client, 2);
			}
			
			return;
		}
	}
	
	TF2_LookAround(client);
}

stock void TF2_FindSentrySpot(int client)
{
	char currentMap[PLATFORM_MAX_PATH];
	GetCurrentMap(currentMap, sizeof(currentMap));
	float clientEyes[3];
	GetClientEyePosition(client, clientEyes);
	float bestspot[3];
	if(TF2_GetPlayerClass(client) == TFClass_Engineer)
	{
		if(StrContains(currentMap, "koth_" , false) != -1)
		{
			if(GetConVarInt(EBotNoArea) != 1)
			{
				int AreaCount = LoadFromAddress(navarea_count, NumberType_Int32);
				
				if (AreaCount <= 0)
					return;
				
				ArrayList RandomNavArea  = new ArrayList();
				
				if(g_bPickRandomSentrySpot[client] && WantsBuildSentryGun[client])
				{
					for(int i = 0; i < AreaCount; i++)
					{
						NavArea area = view_as<NavArea>(LoadFromAddress(TheNavAreas + view_as<Address>(4 * i), NumberType_Int32));
						if(area == NavArea_Null)
							continue;
						
						int capturepoint;
						if((capturepoint = FindEntityByClassname(capturepoint, "team_control_point")) != INVALID_ENT_REFERENCE)
						{
							if(HasTFAttributes(area, BLUE_SPAWN_ROOM))
								continue;
							
							if(HasTFAttributes(area, RED_SPAWN_ROOM))
								continue;
							
							if(HasTFAttributes(area, BLOCKED))
								continue;
							
							if(area.HasAttributes(NAV_MESH_NO_JUMP)) // we can't jump don't build here.
								continue;
							
							float areaCenter[3]; area.GetCenter(areaCenter);
							areaCenter[2] += 50.0;
							
							if(GetVectorDistance(GetOrigin(capturepoint), areaCenter, true) > 1000.0)
								continue;
							
							RandomNavArea.Push(area);
						}
					}
				}
				
				NavArea randomArea = NavArea_Null;
			
				if(RandomNavArea.Length > 0)
				{
					randomArea = RandomNavArea.Get(GetRandomInt(0, RandomNavArea.Length - 1));
					randomArea.GetRandomPoint(SentryBuildPosition[client]);
					if(PF_Exists(client) && PF_IsPathToVectorPossible(client, SentryBuildPosition[client]))
					{
						g_bPickRandomSentrySpot[client] = false;
					}
					else
					{
						g_bPickRandomSentrySpot[client] = true;
					}
				}
				else
				{
					randomArea = NavArea_Null;
					g_bPickRandomSentrySpot[client] = true;
				}
			}
			else
			{
				int capturepoint;
				if((capturepoint = FindEntityByClassname(capturepoint, "team_control_point")) != INVALID_ENT_REFERENCE)
				{
					float capturepointpos[3];
					float capturepointpos2[3];
					GetEntPropVector(capturepoint, Prop_Send, "m_vecOrigin", capturepointpos);
					GetEntPropVector(capturepoint, Prop_Send, "m_vecOrigin", capturepointpos2);
					
					capturepointpos[2] += GetRandomFloat(200.0, 400.0);
					
					if(g_bPickRandomSentrySpot[client])
					{
						g_flRandomSentrySpotPos[client][0] = capturepointpos[0] + GetRandomFloat(-2000.0, 2000.0);
						g_flRandomSentrySpotPos[client][1] = capturepointpos[1] + GetRandomFloat(-2000.0, 2000.0);
						g_flRandomSentrySpotPos[client][2] = capturepointpos2[2] + 50.0;
						
						bestspot[0] = g_flRandomSentrySpotPos[client][0];
						bestspot[1] = g_flRandomSentrySpotPos[client][1];
						bestspot[2] = clientEyes[2];
						
						NavArea area = TheNavMesh.GetNearestNavArea_Vec(g_flRandomSentrySpotPos[client], true, 500.0, false, false, GetClientTeam(client));
						if(area != NavArea_Null)
						{
							if(!HasTFAttributes(area, BLOCKED) && !HasTFAttributes(area, BLUE_SPAWN_ROOM) && !HasTFAttributes(area, RED_SPAWN_ROOM) && !HasTFAttributes(area, BLUE_SENTRY) && !HasTFAttributes(area, RED_SENTRY))
							{
								if(IsPointVisible(bestspot, capturepointpos) && GetVectorDistance(bestspot, capturepointpos, true) < 1300.0)
								{
									area.GetRandomPoint(SentryBuildPosition[client]);
									
									//if(PF_Exists(client) && !PF_IsPathToVectorPossible(client, SentryBuildPosition[client])) // laggy
									//	return;
									
									g_bPickRandomSentrySpot[client] = false;
								}
								else
								{
									g_bPickRandomSentrySpot[client] = true;
								}
							}
							else
							{
								g_bPickRandomSentrySpot[client] = true;
							}
						}
						else
						{
							g_bPickRandomSentrySpot[client] = true;
						}
					}
				}
			}
		}
		else if(StrContains(currentMap, "cp_" , false) != -1)
		{
			if(WantsMoveSentryGun[client])
			{
				int capturepoint = GetNearestEnemyControlPoint(client);
				if(capturepoint != -1)
				{
					NavArea area = TheNavMesh.GetNearestNavArea_Vec(GetOrigin(capturepoint), true, 500.0, false, false, GetClientTeam(client));
					if(area != NavArea_Null)
					{
						area.GetRandomPoint(SentryBuildPosition[client]);
					}
					else
					{
						SentryBuildPosition[client] = GetOrigin(capturepoint);
					}
				}
				
				return;
			}
			
			int capturepoint = GetNearestMyControlPoint(client);
			if(capturepoint != -1)
			{
				float capturepointpos[3];
				capturepointpos = GetOrigin(capturepoint);
				
				capturepointpos[2] += GetRandomFloat(100.0, 200.0);
				
				if(g_bPickRandomSentrySpot[client])
				{
					g_flRandomSentrySpotPos[client][0] = capturepointpos[0] + GetRandomFloat(-1500.0, 1500.0);
					g_flRandomSentrySpotPos[client][1] = capturepointpos[1] + GetRandomFloat(-1500.0, 1500.0);
					g_flRandomSentrySpotPos[client][2] = capturepointpos[2] + GetRandomFloat(-500.0, 500.0);
					
					NavArea area = TheNavMesh.GetNearestNavArea_Vec(g_flRandomSentrySpotPos[client], true, 500.0, false, false, GetClientTeam(client));
					if(area != NavArea_Null)
					{
						area.GetRandomPoint(SentryBuildPosition[client]);
						
						if(HasTFAttributes(area, BLUE_SPAWN_ROOM))
							return;
						
						if(HasTFAttributes(area, RED_SPAWN_ROOM))
							return;
						
						if(HasTFAttributes(area, BLOCKED)) // lag?
							return;
						
						if(area.HasAttributes(NAV_MESH_NO_JUMP)) // if we can't jump don't build here!
							return;
						
						if(!IsPointVisible(SentryBuildPosition[client], capturepointpos))
							return;
						
						if(GetVectorDistance(SentryBuildPosition[client], capturepointpos, true) > 1300.0)
							return;
						
						if(PF_Exists(client) && !PF_IsPathToVectorPossible(client, SentryBuildPosition[client])) // sometimes laggy
							return;
						
						g_bPickRandomSentrySpot[client] = false;
					}
					else
					{
						g_bPickRandomSentrySpot[client] = true;
					}
				}
			}
			else
			{
				float capturepointpos[3];
				capturepointpos = GetOrigin(GetNearestEnemyControlPoint(client));
				
				capturepointpos[2] += GetRandomFloat(100.0, 200.0);
				
				if(g_bPickRandomSentrySpot[client])
				{
					g_flRandomSentrySpotPos[client][0] = capturepointpos[0] + GetRandomFloat(-1500.0, 1500.0);
					g_flRandomSentrySpotPos[client][1] = capturepointpos[1] + GetRandomFloat(-1500.0, 1500.0);
					g_flRandomSentrySpotPos[client][2] = capturepointpos[2] + GetRandomFloat(-500.0, 500.0);
					
					NavArea area = TheNavMesh.GetNearestNavArea_Vec(g_flRandomSentrySpotPos[client], true, 500.0, false, false, GetClientTeam(client));
					if(area != NavArea_Null)
					{
						area.GetRandomPoint(SentryBuildPosition[client]);
						
						if(HasTFAttributes(area, BLUE_SPAWN_ROOM))
							return;
						
						if(HasTFAttributes(area, RED_SPAWN_ROOM))
							return;
						
						if(HasTFAttributes(area, BLOCKED)) // lag?
							return;
						
						if(area.HasAttributes(NAV_MESH_NO_JUMP)) // if we can't jump don't build here!
							return;
						
						if(!IsPointVisible(SentryBuildPosition[client], capturepointpos))
							return;
						
						if(GetVectorDistance(SentryBuildPosition[client], capturepointpos, true) > 1300.0)
							return;
						
						if(!PF_IsPathToVectorPossible(client, SentryBuildPosition[client])) // sometimes laggy
							return;
						
						g_bPickRandomSentrySpot[client] = false;
					}
					else
					{
						g_bPickRandomSentrySpot[client] = true;
					}
				}
			}
		}
		else if(StrContains(currentMap, "pl_" , false) != -1)
		{
			int payload;
			if((payload = FindEntityByClassname(payload, "mapobj_cart_dispenser")) != INVALID_ENT_REFERENCE)
			{
				float payloadposition[3];
				payloadposition = GetOrigin(payload);
				
				payloadposition[2] += 200;
				
				if(g_bPickRandomSentrySpot[client])
				{
					g_flRandomSentrySpotPos[client][0] = payloadposition[0] + GetRandomFloat(-1500.0, 1500.0);
					g_flRandomSentrySpotPos[client][1] = payloadposition[1] + GetRandomFloat(-1500.0, 1500.0);
					g_flRandomSentrySpotPos[client][2] = payloadposition[2] + GetRandomFloat(-500.0, 500.0);
					
					NavArea area = TheNavMesh.GetNearestNavArea_Vec(g_flRandomSentrySpotPos[client], true, 500.0, false, false, GetClientTeam(client));
					if(area != NavArea_Null)
					{
						area.GetRandomPoint(SentryBuildPosition[client]);
						
						if(HasTFAttributes(area, BLUE_SPAWN_ROOM))
							return;
						
						if(HasTFAttributes(area, RED_SPAWN_ROOM))
							return;
						
						if(HasTFAttributes(area, RED_SENTRY))
							return;
						
						if(HasTFAttributes(area, BLUE_SENTRY))
							return;
						
						if(HasTFAttributes(area, BLOCKED))
							return;
						
						if(area.HasAttributes(NAV_MESH_NO_JUMP)) // if we can't jump don't build here!
							return;
						
						if(IsPointVisible(SentryBuildPosition[client], payloadposition))
							return;
						
						if(GetVectorDistance(SentryBuildPosition[client], payloadposition, true) > 1100.0)
							return;
						
						if(!PF_IsPathToVectorPossible(client, SentryBuildPosition[client])) // sometimes laggy
							return;
						
						g_bPickRandomSentrySpot[client] = false;
					}
					else
					{
						g_bPickRandomSentrySpot[client] = true;
					}
				}
			}
		}
		else if(StrContains(currentMap, "ctf_" , false) != -1)
		{
			if(GetConVarInt(EBotNoArea) != 1)
			{
				int AreaCount = LoadFromAddress(navarea_count, NumberType_Int32);
				
				if (AreaCount <= 0)
					return;
				
				ArrayList RandomNavArea  = new ArrayList();
				
				if(g_bPickRandomSentrySpot[client] && WantsBuildSentryGun[client])
				{
					for(int i = 0; i < AreaCount; i++)
					{
						NavArea area = view_as<NavArea>(LoadFromAddress(TheNavAreas + view_as<Address>(4 * i), NumberType_Int32));
						if(area == NavArea_Null)
							continue;
						
						int flag;
						if((flag = FindEntityByClassname(flag, "item_teamflag")) != INVALID_ENT_REFERENCE)
						{
							int iTeamNumObj = GetEntProp(flag, Prop_Send, "m_iTeamNum");
							if(IsValidEntity(flag) && GetClientTeam(client) == iTeamNumObj)
							{
								if(HasTFAttributes(area, BLUE_SPAWN_ROOM))
									continue;
								
								if(HasTFAttributes(area, RED_SPAWN_ROOM))
									continue;
								
								if(HasTFAttributes(area, BLOCKED))
									continue;
								
								if(area.HasAttributes(NAV_MESH_NO_JUMP)) // we can't jump don't build here.
									continue;
								
								float areaCenter[3]; area.GetCenter(areaCenter);
								areaCenter[2] += 50.0;
								
								if(GetVectorDistance(GetOrigin(flag), areaCenter, true) > 1000.0)
									continue;
								
								RandomNavArea.Push(area);
							}
						}
					}
				}
				
				NavArea randomArea = NavArea_Null;
			
				if(RandomNavArea.Length > 0)
				{
					randomArea = RandomNavArea.Get(GetRandomInt(0, RandomNavArea.Length - 1));
					randomArea.GetRandomPoint(SentryBuildPosition[client]);
					if(PF_Exists(client) && PF_IsPathToVectorPossible(client, SentryBuildPosition[client]))
					{
						g_bPickRandomSentrySpot[client] = false;
					}
					else
					{
						g_bPickRandomSentrySpot[client] = true;
					}
				}
				else
				{
					randomArea = NavArea_Null;
					g_bPickRandomSentrySpot[client] = true;
				}
			}
			else
			{
				int flag;
				while((flag = FindEntityByClassname(flag, "item_teamflag")) != INVALID_ENT_REFERENCE)
				{
					int iTeamNumObj = GetEntProp(flag, Prop_Send, "m_iTeamNum");
					if(IsValidEntity(flag) && GetClientTeam(client) == iTeamNumObj)
					{
						float flagposition[3];
						GetEntPropVector(flag, Prop_Send, "m_vecOrigin", flagposition);
						
						int FlagStatus = GetEntProp(flag, Prop_Send, "m_nFlagStatus");
						
						flagposition[2] += GetRandomFloat(200.0, 400.0);
						
						if(g_bPickRandomSentrySpot[client])
						{
							if(FlagStatus == 0 || FlagStatus == 2)
							{
								g_flRandomSentrySpotPos[client][0] = PlayerSpawn[client][0] + GetRandomFloat(-2000.0, 2000.0);
								g_flRandomSentrySpotPos[client][1] = PlayerSpawn[client][1] + GetRandomFloat(-2000.0, 2000.0);
								g_flRandomSentrySpotPos[client][2] = clientEyes[2];
								
								bestspot[0] = g_flRandomSentrySpotPos[client][0];
								bestspot[1] = g_flRandomSentrySpotPos[client][1];
								bestspot[2] = clientEyes[2];
								
								NavArea area = TheNavMesh.GetNearestNavArea_Vec(g_flRandomSentrySpotPos[client], true, 500.0, false, false, GetClientTeam(client));
								if(area != NavArea_Null)
								{
									if(!HasTFAttributes(area, BLOCKED) && !HasTFAttributes(area, BLUE_SPAWN_ROOM) && !HasTFAttributes(area, RED_SPAWN_ROOM) && !HasTFAttributes(area, BLUE_SENTRY) && !HasTFAttributes(area, RED_SENTRY))
									{
										if(GetVectorDistance(bestspot, PlayerSpawn[client], true) > 1100.0)
										{
											area.GetRandomPoint(SentryBuildPosition[client]);
											
											if(PF_Exists(client) && !PF_IsPathToVectorPossible(client, SentryBuildPosition[client]))
												return;
											
											if(area.HasAttributes(NAV_MESH_NO_JUMP)) // if we can't jump don't build here!
												return;
											
											g_bPickRandomSentrySpot[client] = false;
										}
										else
										{
											g_bPickRandomSentrySpot[client] = true;
										}
									}
									else
									{
										g_bPickRandomSentrySpot[client] = true;
									}
								}
								else
								{
									g_bPickRandomSentrySpot[client] = true;
								}
							}
							
							if(FlagStatus == 1)
							{
								g_flRandomSentrySpotPos[client][0] = flagposition[0] + GetRandomFloat(-1500.0, 1500.0);
								g_flRandomSentrySpotPos[client][1] = flagposition[1] + GetRandomFloat(-1500.0, 1500.0);
								g_flRandomSentrySpotPos[client][2] = flagposition[2] + 50.0;
								
								bestspot[0] = g_flRandomSentrySpotPos[client][0];
								bestspot[1] = g_flRandomSentrySpotPos[client][1];
								bestspot[2] = clientEyes[2];
								
								NavArea area = TheNavMesh.GetNearestNavArea_Vec(g_flRandomSentrySpotPos[client], true, 500.0, false, false, GetClientTeam(client));
								if(area != NavArea_Null)
								{
									if(!HasTFAttributes(area, BLOCKED) && !HasTFAttributes(area, BLUE_SPAWN_ROOM) && !HasTFAttributes(area, RED_SPAWN_ROOM))
									{
										if(IsPointVisible(bestspot, flagposition) && GetVectorDistance(bestspot, flagposition, true) < 1300.0)
										{
											area.GetRandomPoint(SentryBuildPosition[client]);
											
											if(PF_Exists(client) && !PF_IsPathToVectorPossible(client, SentryBuildPosition[client]))
												return;
											
											if(area.HasAttributes(NAV_MESH_NO_JUMP)) // if we can't jump don't build here!
												return;
											
											g_bPickRandomSentrySpot[client] = false;
										}
										else
										{
											g_bPickRandomSentrySpot[client] = true;
										}
									}
									else
									{
										g_bPickRandomSentrySpot[client] = true;
									}
								}
								else
								{
									g_bPickRandomSentrySpot[client] = true;
								}
							}
						}
					}
				}
			}
		}
		else if(StrContains(currentMap, "plr_" , false) != -1)
		{
			if(GetConVarInt(EBotNoArea) != 1)
			{
				int AreaCount = LoadFromAddress(navarea_count, NumberType_Int32);
				
				if (AreaCount <= 0)
					return;
				
				ArrayList RandomNavArea  = new ArrayList();
				
				if(g_bPickRandomSentrySpot[client] && WantsBuildSentryGun[client])
				{
					for(int i = 0; i < AreaCount; i++)
					{
						NavArea area = view_as<NavArea>(LoadFromAddress(TheNavAreas + view_as<Address>(4 * i), NumberType_Int32));
						if(area == NavArea_Null)
							continue;
						
						int payload = GetNearestPayload(client, GetClientTeam(client));
						if(payload != -1)
						{
							if(HasTFAttributes(area, BLUE_SPAWN_ROOM))
								continue;
							
							if(HasTFAttributes(area, RED_SPAWN_ROOM))
								continue;
							
							if(HasTFAttributes(area, BLOCKED))
								continue;
							
							if(area.HasAttributes(NAV_MESH_NO_JUMP)) // we can't jump don't build here.
								continue;
							
							float areaCenter[3]; area.GetCenter(areaCenter);
							areaCenter[2] += 50.0;
							
							if(GetVectorDistance(GetOrigin(payload), areaCenter, true) > 1000.0)
								continue;
							
							RandomNavArea.Push(area);
						}
					}
				}
				
				NavArea randomArea = NavArea_Null;
			
				if(RandomNavArea.Length > 0)
				{
					randomArea = RandomNavArea.Get(GetRandomInt(0, RandomNavArea.Length - 1));
					randomArea.GetRandomPoint(SentryBuildPosition[client]);
					if(PF_Exists(client) && PF_IsPathToVectorPossible(client, SentryBuildPosition[client]))
					{
						g_bPickRandomSentrySpot[client] = false;
					}
					else
					{
						g_bPickRandomSentrySpot[client] = true;
					}
				}
				else
				{
					randomArea = NavArea_Null;
					g_bPickRandomSentrySpot[client] = true;
				}
			}
			else
			{
				int payload = GetNearestPayload(client, GetClientTeam(client));
				if(payload != -1)
				{
					float payloadposition[3];
					payloadposition = GetOrigin(payload);
					
					if(g_bPickRandomSentrySpot[client])
					{
						g_flRandomSentrySpotPos[client][0] = payloadposition[0] + GetRandomFloat(-1250.0, 1250.0);
						g_flRandomSentrySpotPos[client][1] = payloadposition[1] + GetRandomFloat(-1250.0, 1250.0);
						g_flRandomSentrySpotPos[client][2] = payloadposition[2];
						
						bestspot[0] = g_flRandomSentrySpotPos[client][0];
						bestspot[1] = g_flRandomSentrySpotPos[client][1];
						bestspot[2] = clientEyes[2];
						
						NavArea area = TheNavMesh.GetNearestNavArea_Vec(g_flRandomSentrySpotPos[client], true, 500.0, false, false, GetClientTeam(client));
						if(area != NavArea_Null)
						{
							if(!HasTFAttributes(area, BLOCKED) && !HasTFAttributes(area, BLUE_SPAWN_ROOM) && !HasTFAttributes(area, RED_SPAWN_ROOM) && !HasTFAttributes(area, BLUE_SENTRY) && !HasTFAttributes(area, RED_SENTRY))
							{
								area.GetRandomPoint(SentryBuildPosition[client]);
								
								if(!IsPointVisibleTank(g_flRandomSentrySpotPos[client], payloadposition))
									return;
								
								if(PF_Exists(client) && !PF_IsPathToVectorPossible(client, SentryBuildPosition[client]))
									return;
								
								g_bPickRandomSentrySpot[client] = false;
							}
							else
							{
								g_bPickRandomSentrySpot[client] = true;
							}
						}
						else
						{
							g_bPickRandomSentrySpot[client] = true;
						}
					}
				}
			}
		}
	}
}