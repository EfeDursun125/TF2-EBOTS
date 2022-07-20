float m_checkTimer[MAXPLAYERS + 1];

public void CheckSlowThink(int client)
{
	if (m_checkTimer[client] < GetGameTime())
	{
		m_isSlowThink[client] = true;

		m_primaryID[client] = TF2_GetPlayerWeaponID(client, 0);
		m_secondaryID[client] = TF2_GetPlayerWeaponID(client, 1);
		m_meleeID[client] = TF2_GetPlayerWeaponID(client, 2);

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

		if (TF2_GetPlayerClass(client) == TFClass_Sniper && !m_hasEnemiesNear[client] && !m_hasEntitiesNear[client])
		{
			if (TF2_IsPlayerInCondition(client, TFCond_Zoomed) && IsMoving(client))
				m_buttons[client] |= IN_ATTACK2;
		}

		if (TF2_GetPlayerClass(client) == TFClass_Pyro)
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

		if (TF2_GetPlayerClass(client) == TFClass_DemoMan)
		{
			if (GetPlayerStickyCount(client) < 8)
				m_maxSticky[client] = true;
			else
				m_maxSticky[client] = false;
			
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
						
					if (GetVectorDistance(GetOrigin(iSticky), GetOrigin(search), true) > Squared(80))
						continue;
						
					m_buttons[client] |= IN_ATTACK2;
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

			if (TF2_IsPlayerInCondition(client, TFCond_Cloaked))
			{
				int cloak = FindSendPropInfo("CTFPlayer", "m_flCloakMeter");
				SetEntDataFloat(client, cloak, 100.0);

				if (CurrentProcess[client] != PRO_HIDE && !m_hasEnemiesNear[client] && !m_hasEntitiesNear[client])
					m_buttons[client] |= IN_ATTACK2;
			}
			
			if (m_hasEntitiesNear[client] && IsValidEntity(m_nearestEntity[client]) && HasEntProp(m_nearestEntity[client], Prop_Send, "m_bHasSapper")) // remember sappable things
				m_knownSentry[client] = m_nearestEntity[client];
		}

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
	
	if (!m_hasEnemiesNear[client])
		return;
	
	if (!m_isSlowThink[client])
		return;
	
	bool musthide = false;

	if (!GameRules_GetProp("m_bPlayingMedieval") && m_enemiesNearCount[client] > m_friendsNearCount[client] + 2)
		musthide = true;
	
	if (TF2_GetPlayerClass(client) == TFClass_Spy && m_meleeID[client] == 356)
	{
		if (GetClientHealth(client) <= 55.0)
			musthide = true;
	}
	else if (GetClientHealth(client) <= (GetMaxHealth(client) / 1.4))
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