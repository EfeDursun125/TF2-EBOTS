float m_checkTimer[MAXPLAYERS + 1];

stock void CheckSlowThink(int client)
{
	if (m_checkTimer[client] < GetGameTime())
	{
		m_isSlowThink[client] = true;
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

		if (m_eBotDodgeRange[client] <= 0)
		{
			if (GetConVarInt(m_eBotDodgeRangeMin) > GetConVarInt(m_eBotDodgeRangeMax))
				SetConVarInt(m_eBotDodgeRangeMin, GetConVarInt(m_eBotDodgeRangeMax));
			m_eBotDodgeRange[client] = GetRandomInt(GetConVarInt(m_eBotDodgeRangeMin), GetConVarInt(m_eBotDodgeRangeMax));
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

			if (StrContains(currentMap, "ctf_" , false) != -1)
			{
				int flag;
				while ((flag = FindEntityByClassname(flag, "item_teamflag")) != INVALID_ENT_REFERENCE)
				{
					if (!IsValidEntity(flag))
						continue;
					
					if (GetClientTeam(client) != GetTeamNumber(flag))
						continue;
					
					if (GetEntProp(flag, Prop_Send, "m_nFlagStatus") != 1)
						continue;

					m_attack2Timer[client] = GetGameTime() + 0.2;
				}
			}
			else
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
						
						if (GetVectorDistance(GetOrigin(iSticky), GetOrigin(search), true) > Squared(80))
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
		
		if (TF2_GetPlayerClass(client) == TFClass_Spy && !TF2_IsPlayerInCondition(client, TFCond_Disguised) && !TF2_HasTheFlag(client))
			TF2_DisguisePlayer(client, (GetClientTeam(client) == 2 ? TFTeam_Blue : TFTeam_Red), view_as<TFClassType>(GetRandomInt(1, 9)));

		m_checkTimer[client] = GetGameTime() + GetRandomFloat(0.95, 1.05);
	}
	else
		m_isSlowThink[client] = false;
}

// automaticly triggers the collect health pack process
stock void CheckHealth(int client)
{
	if (!m_isSlowThink[client])
		return;
	
	if (TF2_GetPlayerClass(client) == TFClass_Spy && TF2_GetPlayerWeaponID(client, 2) == 356)
	{
		if(GetClientHealth(client) <= 50.0)
			m_lowHealth[client] = true;
		else
			m_lowHealth[client] = false;
	}
	else if (GetClientHealth(client) <= (GetMaxHealth(client) / 1.4))
		m_lowHealth[client] = true;
	else
		m_lowHealth[client] = false;
	
	if (m_lowHealth[client])
		SetProcess(client, PRO_GETHEALTH, false, 90.0, "| process started -> need health pack.");
}

// automaticly triggers the collect health pack process
stock void CheckAmmo(int client)
{
	if (!m_isSlowThink[client])
		return;
	
	if (GetAmmo(client, 0) <= 3 || (TF2_GetPlayerClass(client) != TFClass_Spy && TF2_GetPlayerClass(client) != TFClass_Medic && GetAmmo(client, 1) <= 6))
		m_lowAmmo[client] = true;
	else
		m_lowAmmo[client] = false;
	
	if (m_lowAmmo[client])
		SetProcess(client, PRO_GETAMMO, false, 90.0, "| process started -> need ammo pack.");
}

stock void CheckWalls(int client)
{
	if (!m_hasEnemiesNear[client] && CheckWall(client))
		m_buttons[client] |= IN_JUMP;
}