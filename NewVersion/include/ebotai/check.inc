float m_checkTimer[TFMaxPlayers];

public void CheckSlowThink(int client)
{
	if (m_checkTimer[client] < GetGameTime())
	{
		m_isSlowThink[client] = true;

		m_isSniper[client] = false;
		m_isMedic[client] = false;
		m_hasCrossbow[client] = false;
		m_hasWrench[client] = false;
		m_hasSandwich[client] = false;

		int primaryWeapon = GetPlayerWeaponSlot(client, 0);
		if (IsValidEntity(primaryWeapon))
		{
			m_primaryID[client] = TF2_GetWeaponID(primaryWeapon);
			m_primaryClassName[client] = TF2_GetWeaponClassName(primaryWeapon);
			
			if (StrEqual(m_primaryClassName[client], "tf_weapon_sniperrifle"))
				m_isSniper[client] = true;
			else if (StrEqual(m_primaryClassName[client], "tf_weapon_sniperrifle_decap"))
				m_isSniper[client] = true;
			else if (StrEqual(m_primaryClassName[client], "tf_weapon_crossbow"))
				m_hasCrossbow[client] = true;
		}

		int secondaryWeapon = GetPlayerWeaponSlot(client, 1);
		if (IsValidEntity(secondaryWeapon))
		{
			m_secondaryID[client] = TF2_GetWeaponID(secondaryWeapon);
			m_secondaryClassName[client] = TF2_GetWeaponClassName(secondaryWeapon);
			
			if (StrEqual(m_secondaryClassName[client], "tf_weapon_medigun"))
				m_isMedic[client] = true;
			else if (StrEqual(m_secondaryClassName[client], "tf_weapon_lunchbox"))
				m_hasSandwich[client] = true;
		}

		int meleeWeapon = GetPlayerWeaponSlot(client, 2);
		if (IsValidEntity(meleeWeapon))
		{
			m_meleeID[client] = TF2_GetWeaponID(meleeWeapon);
			m_meleeClassName[client] = TF2_GetWeaponClassName(meleeWeapon);

			if (StrEqual(m_meleeClassName[client], "tf_weapon_wrench"))
				m_hasWrench[client] = true;
		}

		if (m_meleeID[client] == 656)
		{
			if (m_hasEnemiesNear[client] && TF2_IsPlayerInCondition(client, TFCond_Taunting))
				FakeClientCommandThrottled(client, "taunt");
		}

		if (GameRules_GetProp("m_bPlayingMedieval"))
		{
			if (!IsMoving(client))
			{
				if (m_hasEnemiesNear[client] && TF2_GetPlayerClass(client) != TFClass_Sniper)
					AStarFindPath(-1, FindNearestWaypoint(GetOrigin(m_nearestEnemy[client]), 999999.0, client), client, GetOrigin(m_nearestEnemy[client]));
				else if (m_goalIndex[client] != -1 && m_paths[m_goalIndex[client]].activeArea != currentActiveArea)
					SelectObjective(client);
			}
			else if (m_hasEnemiesNear[client] && TF2_GetPlayerClass(client) == TFClass_DemoMan)
				m_buttons[client] |= IN_ATTACK2;
		}

		if (!m_hasWaypoints)
			CheckWalls(client);

		if (m_eBotSenseChance[client] <= 0)
		{
			if (GetConVarInt(EBotSenseMin) < 10)
				SetConVarInt(EBotSenseMin, 10);
			
			if (GetConVarInt(EBotSenseMax) > 90)
				SetConVarInt(EBotSenseMax, 90);
			
			if (GetConVarInt(EBotSenseMin) > GetConVarInt(EBotSenseMax))
				SetConVarInt(EBotSenseMin, GetConVarInt(EBotSenseMax));

			m_eBotSenseChance[client] = GetRandomInt(GetConVarInt(EBotSenseMin), GetConVarInt(EBotSenseMax));
		}

		if (m_difficulty[client] < 0 || m_difficulty[client] > 4)
		{
			if (GetConVarInt(EBotDifficulty) < 0 || GetConVarInt(EBotDifficulty) > 4)
				m_difficulty[client] = GetRandomInt(0, 4);
			else
				m_difficulty[client] = GetConVarInt(EBotDifficulty);
		}

		if (m_eBotDodgeRange[client] <= 0)
		{
			if (GetConVarInt(m_eBotDodgeRangeMin) > GetConVarInt(m_eBotDodgeRangeMax))
				SetConVarInt(m_eBotDodgeRangeMin, GetConVarInt(m_eBotDodgeRangeMax));
			m_eBotDodgeRange[client] = GetRandomInt(GetConVarInt(m_eBotDodgeRangeMin), GetConVarInt(m_eBotDodgeRangeMax));
		}

		if (IsSniper(client) && !m_hasEnemiesNear[client] && !m_hasEntitiesNear[client])
		{
			if (TF2_IsPlayerInCondition(client, TFCond_Zoomed) && IsMoving(client))
				m_buttons[client] |= IN_ATTACK2;
		}
		
		if (m_hasEnemiesNear[client] && TF2_GetPlayerClass(client) == TFClass_Sniper && (IsWeaponSlotActive(client, 1) && m_secondaryID[client] == 751))
			m_buttons[client] |= IN_ATTACK2;
		
		if (TF2_GetPlayerClass(client) == TFClass_Sniper && (IsWeaponSlotActive(client, 0) && m_primaryID[client] == 752))
			m_buttons[client] |= IN_RELOAD;
		
		if (m_hasEnemiesNear[client] && TF2_GetPlayerClass(client) == TFClass_Scout && (IsWeaponSlotActive(client, 2) && m_meleeID[client] == 44 || m_meleeID[client] == 648))
			m_attack2Timer[client] = GetGameTime() + 2.0;
		
		if (m_hasEnemiesNear[client] && TF2_GetPlayerClass(client) == TFClass_Pyro && (IsWeaponSlotActive(client, 0) && m_primaryID[client] == 594))
			m_buttons[client] |= IN_ATTACK2;
		
		if (TF2_GetPlayerClass(client) == TFClass_Scout && (IsWeaponSlotActive(client, 0) && m_primaryID[client] == 448))
			m_buttons[client] |= IN_ATTACK2;
		
		if (!m_hasEnemiesNear[client] && TF2_GetPlayerClass(client) == TFClass_Scout && m_secondaryID[client] == 46 || m_secondaryID[client] == 1145 || m_secondaryID[client] == 163)
		{
			EquipWeaponSlot(client, 1);
			if (IsWeaponSlotActive(client, 1))
				m_buttons[client] |= IN_ATTACK;
		}
		
		if (!m_hasEnemiesNear[client] && TF2_GetPlayerClass(client) == TFClass_Soldier && m_secondaryID[client] == 129 || m_secondaryID[client] == 1001 || m_secondaryID[client] == 226 || m_secondaryID[client] == 354)
		{
			if (100.0 == GetEntPropFloat(client, Prop_Send, "m_flRageMeter"))
			{
				EquipWeaponSlot(client, 1);
				if (IsWeaponSlotActive(client, 1))
					m_buttons[client] |= IN_ATTACK;
			}
		}
		
		if (!m_hasEnemiesNear[client] && TF2_GetPlayerClass(client) == TFClass_Heavy && m_secondaryID[client] == 42 || m_secondaryID[client] == 863 || m_secondaryID[client] == 1002 || m_secondaryID[client] == 1190 || m_secondaryID[client] == 311 || m_secondaryID[client] == 159 || m_secondaryID[client] == 433)
		{
			if (GetClientHealth(client) < (GetMaxHealth(client) / 1.4))
			{
				EquipWeaponSlot(client, 1);
				if (IsWeaponSlotActive(client, 1) && (GetClientHealth(client) < (GetMaxHealth(client) / 1.4)))
					m_buttons[client] |= IN_ATTACK;
			}
		}

		if (TF2_GetPlayerClass(client) == TFClass_Pyro && m_primaryID[client] != 594)
		{
			int iProjectile = -1;
			while ((iProjectile = FindEntityByClassname(iProjectile, "tf_projectile_*")) != INVALID_ENT_REFERENCE)
			{
				if (IsValidEntity(iProjectile) && GetClientTeam(client) != GetTeamNumber(iProjectile))
				{
					if (GetVectorDistance(GetOrigin(iProjectile), GetOrigin(client), true) <= Squared(256))
					{
						EquipWeaponSlot(client, 0);
						m_buttons[client] |= IN_ATTACK2;
						m_lookAt[client] = GetOrigin(iProjectile);
						m_pauseTime[client] = GetGameTime() + 2.0;
					}
				}
			}
		}
		
		if (TF2_GetPlayerClass(client) == TFClass_Engineer && m_secondaryID[client] == 528)
		{
			int iProjectile = -1;
			while ((iProjectile = FindEntityByClassname(iProjectile, "tf_projectile_*")) != INVALID_ENT_REFERENCE)
			{
				if (IsValidEntity(iProjectile) && GetClientTeam(client) != GetTeamNumber(iProjectile))
				{
					if (GetVectorDistance(GetOrigin(iProjectile), GetOrigin(client), true) <= Squared(256))
					{
						EquipWeaponSlot(client, 1);
						m_lookAt[client] = GetOrigin(iProjectile);
						if (GetMetal(client) > 65.0)
							m_buttons[client] |= IN_ATTACK2;
						else
							m_buttons[client] |= IN_ATTACK;
					}
				}
			}
		}

		if (TF2_GetPlayerClass(client) == TFClass_DemoMan)
		{
			m_stickyCount[client] = GetPlayerStickyCount(client);
			
			if (m_stickyCount[client] > 0)
			{
				int iSticky = -1;
				while ((iSticky = FindEntityByClassname(iSticky, "tf_projectile_pipe_remote")) != INVALID_ENT_REFERENCE)
				{
					if (!IsValidEntity(iSticky))
						continue;

					if (GetEntityThrower(iSticky) != client)
						continue;

					for (int search = 1; search <= MaxClients; search++)
					{
						if (search == client)
							continue;

						if (!IsValidClient(search))
							continue;

						if (!IsPlayerAlive(search))
							continue;

						if (TF2_IsPlayerInCondition(search, TFCond_Ubercharged))
							continue;

						if (GetClientTeam(client) == GetClientTeam(search))
							continue;
						
						if (GetVectorDistance(GetOrigin(iSticky), GetOrigin(search), true) > Squared(120))
							continue;
						
						m_buttons[client] |= IN_ATTACK2;
					}
				}
			}
		}

		if (ChanceOf(m_eBotSenseChance[client])) // any sus?
		{
			for (int search = 1; search <= MaxClients; search++)
			{
				if (search == client)
					continue;
				
				if (!IsValidClient(search))
					continue;
				
				if (!IsPlayerAlive(search))
					continue;
				
				// only pyro ebots can suspect ebot spies
				if (IsEBot(search) && TF2_GetPlayerClass(client) == TFClass_Pyro)
					continue;
				
				if (!IsVisible(GetEyePosition(client), GetEyePosition(search)))
					continue;
				
				if (!IsClientAimingToMe(client, search))
					continue;
				
				if (!ClientViews(client, search, 0.7))
					continue;
				
				m_knownSpy[client] = search;
			}
		}

		if (m_hasEnemiesNear[client] && IsValidClient(m_nearestEnemy[client]) && TF2_GetPlayerClass(m_nearestEnemy[client]) == TFClass_Spy)
			m_knownSpy[client] = m_nearestEnemy[client];
		
		if (TF2_GetPlayerClass(client) == TFClass_Spy)
		{
			if (!TF2_IsPlayerInCondition(client, TFCond_Disguised) && !TF2_IsPlayerInCondition(client, TFCond_Disguising) && !TF2_HasTheFlag(client))
				DisguiseAsEnemy(client);
			
			if (m_hasEntitiesNear[client] && IsValidEntity(m_nearestEntity[client]) && HasEntProp(m_nearestEntity[client], Prop_Send, "m_bHasSapper")) // remember sappable things
				m_knownSentry[client] = m_nearestEntity[client];
		}

		UseVoiceCommand(client);
		
		m_checkTimer[client] = GetGameTime() + GetRandomFloat(0.95, 1.05);
	}
	else
		m_isSlowThink[client] = false;
}

// automaticly triggers the hiding process
public void CheckHideFromDanger(int client)
{
	if (!m_hasWaypoints)
		return;

	if (m_hasEntitiesNear[client])
	{
		SetProcess(client, PRO_HIDE, 60.0, "| process started -> hiding from danger", false, false);
		return;
	}

	if (!m_isSlowThink[client])
		return;
	
	if (!m_hasEnemiesNear[client])
		return;
	
	bool musthide = false;

	if (!GameRules_GetProp("m_bPlayingMedieval") && m_enemiesNearCount[client] > m_friendsNearCount[client] + 2)
		musthide = true;
	
	if (TF2_GetPlayerClass(client) == TFClass_Medic && m_friendsNearCount[client] <= 0)
		musthide = true;
		
	if (TF2_GetPlayerClass(client) == TFClass_Spy && m_meleeID[client] == 356)
	{
		if (GetClientHealth(client) <= 55.0)
			musthide = true;
	}
	else if (GetClientHealth(client) <= (GetMaxHealth(client) / 1.4))
		musthide = true;
	else if (IsValidClient(m_nearestEnemy[client]) && TF2_GetPlayerClass(m_nearestEnemy[client]) == TFClass_Heavy && GetVectorDistance(GetOrigin(client), GetOrigin(m_nearestEnemy[client]), true) <= Squaredf(768.0))
		musthide = true;
	
	if (musthide)
		SetProcess(client, PRO_HIDE, 60.0, "| process started -> hiding from danger", false, false);
}

// automaticly triggers the collect health pack process
public void CheckHealth(int client)
{
	if (!m_isSlowThink[client])
		return;
	
	if (m_hasWaypoints && m_hasEnemiesNear[client])
		return;
	
	if (TF2_GetPlayerClass(client) == TFClass_Spy && m_meleeID[client] == 356)
	{
		if (GetClientHealth(client) < 55.0)
			m_lowHealth[client] = true;
		else
			m_lowHealth[client] = false;
	}
	else if (GetClientHealth(client) < (GetMaxHealth(client) / 1.4))
		m_lowHealth[client] = true;
	else
		m_lowHealth[client] = false;
	
	if (m_lowHealth[client])
		SetProcess(client, PRO_GETHEALTH, 90.0, "| process started -> need health pack");
}

// automaticly triggers the collect health pack process
public void CheckAmmo(int client)
{
	if (!m_isSlowThink[client])
		return;
		
	if (GetAmmo(client, 0) <= 3 && TF2_GetPlayerClass(client) != TFClass_Spy && TF2_GetPlayerClass(client) != TFClass_Medic)
		m_lowAmmo[client] = true;
	else
		m_lowAmmo[client] = false;
	
	if (m_lowAmmo[client])
		SetProcess(client, PRO_GETAMMO, 90.0, "| process started -> need ammo pack");
}

public void CheckWalls(int client)
{
	if (!m_hasEnemiesNear[client] && CheckWall(client))
		m_buttons[client] |= IN_JUMP;
}
