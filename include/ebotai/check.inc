bool g_bJump[MAXPLAYERS + 1];
bool g_bCrouch[MAXPLAYERS + 1];

float g_flCTimer[MAXPLAYERS + 1];

float g_flSpyForgotTime[MAXPLAYERS + 1];

float EngineerChangeSentrySpotTimer[MAXPLAYERS + 1];

int SentryGun[MAXPLAYERS + 1];

int Dispenser[MAXPLAYERS + 1];

int TeleporterEnter[MAXPLAYERS + 1];

int TeleporterExit[MAXPLAYERS + 1];

bool SentryHealthIsFull[MAXPLAYERS + 1];
bool SentryIsMaxLevel[MAXPLAYERS + 1];

bool DispenserHealthIsFull[MAXPLAYERS + 1];
bool DispenserIsMaxLevel[MAXPLAYERS + 1];

bool TeleporterEnterHealthIsFull[MAXPLAYERS + 1];
bool TeleporterEnterIsMaxLevel[MAXPLAYERS + 1];

bool TeleporterExitHealthIsFull[MAXPLAYERS + 1];
bool TeleporterExitIsMaxLevel[MAXPLAYERS + 1];

bool SapTeleporter[MAXPLAYERS + 1];

bool SapBuildings[MAXPLAYERS + 1];

int TargetTeleporter[MAXPLAYERS + 1];

int TargetBuilding[MAXPLAYERS + 1];

float EngineerMoveSentryTimer[MAXPLAYERS + 1];

bool WantsMoveSentryGun[MAXPLAYERS + 1];

float EngineerMoveDispenserTimer[MAXPLAYERS + 1];

bool WantsMoveDispenser[MAXPLAYERS + 1];

stock void Check(int client)
{
	if (g_flCTimer[client] < GetGameTime())
	{
		IsSlowThink[client] = true;
		
		if(g_flSpyForgotTime[client] < GetGameTime())
		{
			if(IsValidClient(TargetSpyClient[client]) && !IsPointVisible(GetEyePosition(client), GetEyePosition(TargetSpyClient[client])))
			{
				TargetSpyClient[client] = -1;
			}
			
			g_flSpyForgotTime[client] = GetGameTime() + GetRandomFloat(4.0, 12.0);
		}
		else if(IsPointVisible(GetEyePosition(client), GetEyePosition(TargetSpyClient[client])))
		{
			g_flSpyForgotTime[client] = GetGameTime() + GetRandomFloat(4.0, 12.0);
		}
		
		if(HasEnemiesNear[client] && IsValidClient(NearestEnemy[client]))
		{
			if(IsPlayerAlive(NearestEnemy[client]))
			{
				LastKnownEnemyPosition[client] = GetEyePosition(NearestEnemy[client]);
			}
			
			if(!HasFriendsNear[client] && ChanceOf(EBotSenseChance[client])) // no friends here. is area safe?
			{
				StopTime[client] = GetGameTime() + GetRandomFloat(1.0, 5.0);
			}
			else if(HasFriendsNear[client] && ChanceOf(EBotSenseChance[client]) && ProtectTeammateTime[client] < GetGameTime() && GetClientHealth(NearestFriend[client]) < (GetEntProp(NearestFriend[client], Prop_Data, "m_iMaxHealth") / 1.4))
			{
				ProtectTeammateTime[client] = GetGameTime() + GetRandomFloat(8.0, 16.0);
				
				if(ChanceOf(EBotSenseChance[client]))
				{
					if(GetRandomInt(1, 2) == 1)
					{
						FakeClientCommandThrottled(client, "voicemenu 1 0");
					}
					else if(GetRandomInt(1, 2) == 1)
					{
						FakeClientCommandThrottled(client, "voicemenu 2 0");
					}
					else
					{
						FakeClientCommandThrottled(client, "voicemenu 0 2");
					}
				}
			}
			
			IsLastKnownEnemyPositionClear[client] = false;
		}
		else if(!IsPointVisible(GetEyePosition(client), LastKnownEnemyPosition[3]))
		{
			IsLastKnownEnemyPositionClear[client] = false;
		}
		else if(HasEnemiesNear[client] && IsPointVisible(GetEyePosition(client), LastKnownEnemyPosition[3]))
		{
			IsLastKnownEnemyPositionClear[client] = true;
		}
		
		if(IsValidEntity(GetPlayerWeaponSlot(client, 0)))
		{
			PrimaryID[client] = GetEntProp(GetPlayerWeaponSlot(client, 0), Prop_Send, "m_iItemDefinitionIndex");
		}
		
		if(IsValidEntity(GetPlayerWeaponSlot(client, 1)))
		{
			SecondaryID[client] = GetEntProp(GetPlayerWeaponSlot(client, 1), Prop_Send, "m_iItemDefinitionIndex");
		}
		
		if(IsValidEntity(GetPlayerWeaponSlot(client, 2)))
		{
			MeleeID[client] = GetEntProp(GetPlayerWeaponSlot(client, 2), Prop_Send, "m_iItemDefinitionIndex");
		}
		
		if(TF2_GetPlayerClass(client) == TFClass_Medic && HasEnemiesNear[client])
		{
			if(GetClientHealth(client) < 50.0)
			{
				ForcePressButton[client] |= IN_ATTACK2;
			}
			else if(HasFriendsNear[client] && IsValidClient(NearestFriend[client]) && GetClientHealth(NearestFriend[client]) < 75.0)
			{
				ForcePressButton[client] |= IN_ATTACK2;
			}
		}
		
		if (TF2_GetPlayerClass(client) == TFClass_Spy && MeleeID[client] == 356)
		{
			if(GetClientHealth(client) < 50.0)
			{
				g_bHealthIsLow[client] = true;
			}
			else
			{
				g_bHealthIsLow[client] = false;
			}
		}
		else if (GetClientHealth(client) < (GetEntProp(client, Prop_Data, "m_iMaxHealth") / 1.4))
  		{
			g_bHealthIsLow[client] = true;
		}
  		else
		{
			g_bHealthIsLow[client] = false;
		}
		
		if (IsAmmoLow(client) || IsMetalLow(client))
  		{
			g_bAmmoIsLow[client] = true;
		}
  		else
		{
			g_bAmmoIsLow[client] = false;
		}
		
		if(GetConVarInt(EBotPerformance) != 1)
		{
			if(GetEntityFlags(client) & FL_ONGROUND)
			{
				NavArea jumparea = TheNavMesh.GetNearestNavArea_Vec(GetOrigin(client), false, 250.0, false, false, GetClientTeam(client));
				if(jumparea != NavArea_Null)
				{
					if(jumparea.HasAttributes(NAV_MESH_JUMP) && !jumparea.HasAttributes(NAV_MESH_NO_JUMP))
					{
						g_bJump[client] = true;
					}
				}
			}
			
			if(!g_bCrouch[client] && GetEntityFlags(client) & FL_ONGROUND)
			{
				NavArea croucharea = TheNavMesh.GetNearestNavArea_Vec(GetOrigin(client), false, 250.0, false, false, GetClientTeam(client));
				if(croucharea != NavArea_Null)
				{
					if(croucharea.HasAttributes(NAV_MESH_CROUCH))
					{
						g_bCrouch[client] = true;
					}
				}
			}
			
			if(g_bCrouch[client])
			{
				NavArea croucharea = TheNavMesh.GetNearestNavArea_Vec(GetOrigin(client), false, 250.0, false, false, GetClientTeam(client));
				if(croucharea != NavArea_Null)
				{
					if(!croucharea.HasAttributes(NAV_MESH_CROUCH))
					{
						g_bCrouch[client] = false;
					}
				}
			}
		}
		
		if(TF2_GetPlayerClass(client) == TFClass_Engineer)
		{
			if(EngineerChangeSentrySpotTimer[client] < GetGameTime())
			{
				g_bPickRandomSentrySpot[client] = true;
				
				EngineerChangeSentrySpotTimer[client] = GetGameTime() + GetRandomFloat(10.0, 20.0);
			}
			
			if(!ClientIsMoving(client) && IsWeaponSlotActive(client, 5))
			{
				ForcePressButton[client] |= IN_ATTACK;
			}
			
			SentryGun[client] = TF2_GetObject(client, TFObject_Sentry, TFObjectMode_None);
			Dispenser[client] = TF2_GetObject(client, TFObject_Dispenser, TFObjectMode_None);
			TeleporterEnter[client] = TF2_GetObject(client, TFObject_Teleporter, TFObjectMode_Entrance);
			TeleporterExit[client] = TF2_GetObject(client, TFObject_Teleporter, TFObjectMode_Exit);
			
			if(SentryGun[client] != -1 && IsValidEntity(SentryGun[client]) && IsValidEdict(SentryGun[client]))
			{
				WantsBuildSentryGun[client] = false;
				
				int SentryLevel = GetEntProp(SentryGun[client], Prop_Send, "m_iUpgradeLevel");
				int SentryHealth = GetEntProp(SentryGun[client], Prop_Send, "m_iHealth");
				int SentryMaxHealth = GetEntProp(SentryGun[client], Prop_Send, "m_iMaxHealth");
				
				if(MeleeID[client] == 142)
				{
					SentryIsMaxLevel[client] = true;
				}
				else
				{
					if(SentryLevel < 3)
					{
						SentryIsMaxLevel[client] = false;
					}
					else
					{
						SentryIsMaxLevel[client] = true;
					}
				}
				
				if(SentryHealth < SentryMaxHealth)
				{
					SentryHealthIsFull[client] = false;
				}
				else
				{
					SentryHealthIsFull[client] = true;
				}
				
				if(EngineerMoveSentryTimer[client] < GetGameTime())
				{
					WantsMoveSentryGun[client] = true;
				}
			}
			else
			{
				WantsBuildSentryGun[client] = true;
				
				EngineerMoveSentryTimer[client] = GetGameTime() + GetRandomFloat(90.0, 160.0);
				
				WantsMoveSentryGun[client] = false;
			}
			
			if(Dispenser[client] != -1 && IsValidEntity(Dispenser[client]) && IsValidEdict(Dispenser[client]))
			{
				WantsBuildDispenser[client] = false;
				
				int DispenserLevel = GetEntProp(Dispenser[client], Prop_Send, "m_iUpgradeLevel");
				int DispenserHealth = GetEntProp(Dispenser[client], Prop_Send, "m_iHealth");
				int DispenserMaxHealth = GetEntProp(Dispenser[client], Prop_Send, "m_iMaxHealth");
				
				if(DispenserLevel < 3)
				{
					DispenserIsMaxLevel[client] = false;
				}
				else
				{
					DispenserIsMaxLevel[client] = true;
				}
				
				if(DispenserHealth < DispenserMaxHealth)
				{
					DispenserHealthIsFull[client] = false;
				}
				else
				{
					DispenserHealthIsFull[client] = true;
				}
			}
			else
			{
				if(!WantsBuildSentryGun[client] && SentryHealthIsFull[client] && SentryIsMaxLevel[client])
				{
					WantsBuildDispenser[client] = true;
				}
				else
				{
					WantsBuildDispenser[client] = false;
				}
				
				EngineerMoveDispenserTimer[client] = GetGameTime() + GetRandomFloat(120.0, 180.0);
				
				WantsMoveDispenser[client] = false;
			}
			
			if(TeleporterEnter[client] != -1 && IsValidEntity(TeleporterEnter[client]) && IsValidEdict(TeleporterEnter[client]))
			{
				WantsBuildTeleporterEnter[client] = false;
				
				int TeleporterLevel = GetEntProp(TeleporterEnter[client], Prop_Send, "m_iUpgradeLevel");
				int TeleporterHealth = GetEntProp(TeleporterEnter[client], Prop_Send, "m_iHealth");
				int TeleporterMaxHealth = GetEntProp(TeleporterEnter[client], Prop_Send, "m_iMaxHealth");
				
				if(TeleporterLevel < 3)
				{
					TeleporterEnterIsMaxLevel[client] = false;
				}
				else
				{
					TeleporterEnterIsMaxLevel[client] = true;
				}
				
				if(TeleporterHealth < TeleporterMaxHealth)
				{
					TeleporterEnterHealthIsFull[client] = false;
				}
				else
				{
					TeleporterEnterHealthIsFull[client] = true;
				}
			}
			else if(!WantsBuildSentryGun[client] && SentryHealthIsFull[client] && SentryIsMaxLevel[client] && !WantsBuildDispenser[client] && DispenserHealthIsFull[client] && DispenserIsMaxLevel[client])
			{
				WantsBuildTeleporterEnter[client] = false; // don't forget this...
			}
			else
			{
				WantsBuildTeleporterEnter[client] = false;
			}
			
			if(TeleporterExit[client] != -1 && IsValidEntity(TeleporterExit[client]) && IsValidEdict(TeleporterExit[client]))
			{
				WantsBuildTeleporterExit[client] = false;
				
				int TeleporterLevel = GetEntProp(TeleporterExit[client], Prop_Send, "m_iUpgradeLevel");
				int TeleporterHealth = GetEntProp(TeleporterExit[client], Prop_Send, "m_iHealth");
				int TeleporterMaxHealth = GetEntProp(TeleporterExit[client], Prop_Send, "m_iMaxHealth");
				
				if(TeleporterLevel < 3)
				{
					TeleporterExitIsMaxLevel[client] = false;
				}
				else
				{
					TeleporterExitIsMaxLevel[client] = true;
				}
				
				if(TeleporterHealth < TeleporterMaxHealth)
				{
					TeleporterExitHealthIsFull[client] = false;
				}
				else
				{
					TeleporterExitHealthIsFull[client] = true;
				}
			}
			else if(!WantsBuildSentryGun[client] && SentryHealthIsFull[client] && SentryIsMaxLevel[client] && !WantsBuildDispenser[client] && DispenserHealthIsFull[client] && DispenserIsMaxLevel[client])
			{
				WantsBuildTeleporterExit[client] = true;
			}
			else
			{
				WantsBuildTeleporterExit[client] = false;
			}
		}
		
		if(TF2_GetPlayerClass(client) == TFClass_Sniper)
		{
			if(HasEnemiesNear[client] && GetVectorDistance(GetOrigin(client), SniperSpotOrigin[client]) > 500.0)
			{
				StopTime[client] = GetGameTime() + 2.5;
			}
			
			SniperLogic(client);
		}
		
		if(TF2_GetPlayerClass(client) == TFClass_Spy)
		{
			if(!IsCapturingControlPoint(client) && !TF2_IsPlayerInCondition(client, TFCond_Disguised) && !TF2_IsPlayerInCondition(client, TFCond_Disguising))
			{
				Disguise(client);
			}
			else if(TF2_IsPlayerInCondition(client, TFCond_Disguised) && !IsCapturingControlPoint(client))
			{
				FakeClientCommandThrottled(client, "lastdisguise");
			}
			
			if(IsCapturingControlPoint(client) && TF2_IsPlayerInCondition(client, TFCond_Disguised))
			{
				ForcePressButton[client] |= IN_ATTACK;
			}
			
			if(HasEnemiesNear[client])
			{
				if(TF2_IsPlayerInCondition(NearestEnemy[client], TFCond_TeleportedGlow))
				{
					SapTeleporter[client] = false;
					
					if (TargetTeleporter[client] == -1 || !IsValidEntity(TargetTeleporter[client]))
					{
						//TargetTeleporter[client] = GetNearestEntity(client, "obj_teleporter"); // its buggy
					}
				}
			}
			
			if(HasEnemyBuildingsNear[client])
			{
				if(HasEnemyBuildingsNear[client] && NearestEnemyBuilding[client] != -1 && IsValidEntity(NearestEnemyBuilding[client]) && IsValidEdict(NearestEnemyBuilding[client]) && HasEntProp(NearestEnemyBuilding[client], Prop_Send, "m_bHasSapper") && GetEntProp(NearestEnemyBuilding[client], Prop_Send, "m_bHasSapper") == 0)
				{
					TargetBuilding[client] = NearestEnemyBuilding[client];
					
					SapBuildings[client] = true;
				}
			}
		}
		
		if(TF2_GetPlayerClass(client) == TFClass_Pyro)
		{
			/*int iFlare = -1;
			
			while((iFlare = FindEntityByClassname(iFlare, "tf_weapon_flaregun_revenge")) != INVALID_ENT_REFERENCE)
			{
				if (IsValidEntity(iFlare))
				{
					for (int search = 1; search <= MaxClients; search++)
					{
						if (IsClientInGame(search) && IsPlayerAlive(search) && search != client && (GetClientTeam(client) != GetClientTeam(search)))
						{
							if(GetVectorDistance(GetOrigin(iFlare), GetOrigin(search)) < 75.0 && !TF2_IsPlayerInCondition(search, TFCond_Ubercharged))
							{
								if (GetOw(iFlare) == client)
								{
									ForcePressButton[client] |= IN_ATTACK2;
								}
							}
						}
					}
				}
			}*/
			
			int iProjectile = -1;
			while((iProjectile = FindEntityByClassname(iProjectile, "tf_projectile_*")) != INVALID_ENT_REFERENCE)
			{
				if(IsValidEntity(iProjectile))
				{
					if(GetVectorDistance(GetOrigin(iProjectile), GetOrigin(client)) < 350.0)
					{
						ForcePressButton[client] |= IN_ATTACK2;
						
						g_flLookPos[client] = GetOrigin(iProjectile);
						
						g_flLookTimer[client] = GetGameTime() + 2.0;
					}
				}
			}
		}
		
		if(TF2_GetPlayerClass(client) == TFClass_DemoMan)
		{
			if(GetPlayerStickyCount(client) < 8)
			{
				DefendFlag[client] = true;
			}
			else
			{
				DefendFlag[client] = false;
				
				int flag;
				while((flag = FindEntityByClassname(flag, "item_teamflag")) != INVALID_ENT_REFERENCE)
				{
					if(IsValidEntity(flag) && GetClientTeam(client) == GetEntProp(flag, Prop_Send, "m_iTeamNum"))
					{
						if(GetEntProp(flag, Prop_Send, "m_nFlagStatus") == 1)
						{
							if(GetPlayerStickyCount(client) >= 8)
							{
								Attack2Timer[client] = GetGameTime() + 0.2;
							}
						}
					}
				}
			}
			
			int iSticky = -1;
			
			while((iSticky = FindEntityByClassname(iSticky, "tf_projectile_pipe_remote")) != INVALID_ENT_REFERENCE)
			{
				if(IsValidEntity(iSticky))
				{
					for(int search = 1; search <= MaxClients; search++)
					{
						if(IsClientInGame(search) && IsPlayerAlive(search) && search != client && (GetClientTeam(client) != GetClientTeam(search)))
						{
							if(GetVectorDistance(GetOrigin(iSticky), GetOrigin(search)) < 75.0 && !TF2_IsPlayerInCondition(search, TFCond_Ubercharged))
							{
								if (GetEntityThrower(iSticky) == client)
								{
									ForcePressButton[client] |= IN_ATTACK2;
								}
							}
						}
					}
				}
			}
		}
		
		if(UseTeleporter[client] && TF2_IsPlayerInCondition(client, TFCond_TeleportedGlow))
		{
			UseTeleporter[client] = false;
		}
		
		if(TF2_IsNextToWall(client) && (ClientIsMoving(client) || TF2_GetPlayerClass(client) == TFClass_Engineer))
		{
			int nOldButtons = GetEntProp(client, Prop_Data, "m_nOldButtons");
			SetEntProp(client, Prop_Data, "m_nOldButtons", (nOldButtons &= ~(IN_JUMP|IN_DUCK)));
			
			NavArea jumparea = TheNavMesh.GetNearestNavArea_Vec(GetOrigin(client), false, 250.0, false, false, GetClientTeam(client));
			if(jumparea != NavArea_Null)
			{
				if(!jumparea.HasAttributes(NAV_MESH_NO_JUMP))
				{
					g_bJump[client] = true;
				}
			}
		}
		
		if(ChanceOf(EBotSenseChance[client])) // any sus?
		{
			for (int search = 1; search <= MaxClients; search++)
			{
				if(!IsValidClient(search))
					continue;
				
				if(!IsPlayerAlive(search))
					continue;
				
				//if(GetClientTeam(client) == GetClientTeam(search))
				//	continue;
				
				if(search == client)
					continue;
				
				if(!IsPointVisible(GetEyePosition(client), GetEyePosition(search)))
					continue;
				
				if(!IsClientAimingToMe(client, search))
					continue;
				
				TargetSpyClient[client] = search;
			}
		}
		
		g_flCTimer[client] = GetGameTime() + GetRandomFloat(0.9, 1.1);
	}
	else
	{
		IsSlowThink[client] = false;
		
		if(HasEnemiesNear[client] && IsValidClient(NearestEnemy[client]) && !IsPlayerAlive(NearestEnemy[client])) // dead enemy
		{
			if(TF2_IsPlayerInCondition(client, TFCond_CritOnWin) && ChanceOf(GetConVarInt(EBotTauntChance)))
			{
				FakeClientCommandThrottled(client, "taunt");
			}
			
			FindEnemiens(client); // recalculate alive enemies
		}
		
		if(HasEnemiesNear[client])
		{
			if(IsValidClient(NearestEnemy[client]) && TF2_GetPlayerClass(NearestEnemy[client]) == TFClass_Spy)
			{
				TargetSpyClient[client] = NearestEnemy[client];
			}
		}
		
		if(TF2_GetPlayerClass(client) == TFClass_Spy)
		{
			SpyReactChecker(client);
		}
	}
}

stock void Disguise(int client)
{
	if(!TF2_IsPlayerInCondition(client, TFCond_Disguised) && !TF2_IsPlayerInCondition(client, TFCond_Disguising) && !TF2_HasTheFlag(client))
	{
		int randomclass = GetRandomInt(1,9);
		switch(randomclass)
		{
			case 1:
			{
				TF2_DisguisePlayer(client, GetEnemyTFTeam(client), TFClass_Scout);
			}
			case 2:
			{
				TF2_DisguisePlayer(client, GetEnemyTFTeam(client), TFClass_Soldier);
			}
			case 3:
			{
				TF2_DisguisePlayer(client, GetEnemyTFTeam(client), TFClass_Pyro);
			}
			case 4:
			{
				TF2_DisguisePlayer(client, GetEnemyTFTeam(client), TFClass_Engineer);
			}
			case 5:
			{
				TF2_DisguisePlayer(client, GetEnemyTFTeam(client), TFClass_Heavy);
			}
			case 6:
			{
				TF2_DisguisePlayer(client, GetEnemyTFTeam(client), TFClass_DemoMan);
			}
			case 7:
			{
				TF2_DisguisePlayer(client, GetEnemyTFTeam(client), TFClass_Medic);
			}
			case 8:
			{
				TF2_DisguisePlayer(client, GetEnemyTFTeam(client), TFClass_Sniper);
			}
			case 9:
			{
				TF2_DisguisePlayer(client, GetEnemyTFTeam(client), TFClass_Spy);
			}
		}
	}
}