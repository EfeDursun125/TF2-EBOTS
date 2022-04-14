stock bool DefaultRequirements(int client)
{
	return true;
}

stock void DefaultStart(int client)
{
	if (TF2_GetPlayerClass(client) == TFClass_Spy)
		SetProcess(client, PRO_SPYLURK, true, 9999.0, "", true);
	SelectObjective(client);
}

stock void DefaultUpdate(int client)
{
	LookUpdate(client);

	if (TF2_IsPlayerInCondition(client, TFCond_CritOnWin) && m_hasEnemiesNear[client])
	{
		FollowPath(client, GetOrigin(m_nearestEnemy[client]));
		FindFriendsAndEnemiens(client);
		return;
	}

	if (IsValidEntity(m_goalEntity[client]))
		ObjectiveMovement(client);
	else
		SelectObjective(client);

	if (m_hasEnemiesNear[client])
		SetProcess(client, PRO_ATTACK, false, 120.0, "", true);
	
	FindFriendsAndEnemiens(client);
	CheckHealth(client);
	CheckAmmo(client);
	
}

stock void DefaultEnd(int client)
{
	if (TF2_GetPlayerClass(client) == TFClass_Spy)
		return;
	m_goalEntity[client] = -1;
}

stock void SelectObjective(int client)
{
	if (StrContains(currentMap, "koth_" , false) != -1)
	{
		int capturepoint;
		if ((capturepoint = FindEntityByClassname(capturepoint, "team_control_point")) != INVALID_ENT_REFERENCE)
		{
			if (IsValidEntity(capturepoint))
				m_goalEntity[client] = capturepoint;
		}
	}
	else if (StrContains(currentMap, "ctf_" , false) != -1)
	{
		int flag = -1;
		while ((flag = FindEntityByClassname(flag, "item_teamflag")) != INVALID_ENT_REFERENCE)
		{
			if (IsValidEntity(flag) && HasEntProp(flag, Prop_Send, "m_nFlagStatus") && GetClientTeam(client) != GetTeamNumber(flag))
				m_goalEntity[client] = flag;
		}
	}
	else
		m_goalEntity[client] = FindNearestEnemy(client);
}

stock void ObjectiveMovement(int client)
{
	if (IsReloading(client))
		return;
	
	if (StrContains(currentMap, "koth_" , false) != -1)
	{
		float cappointpos[3];
		GetEntPropVector(m_goalEntity[client], Prop_Send, "m_vecOrigin", cappointpos);
		if (GetTeamNumber(m_goalEntity[client]) == GetClientTeam(client))
		{
			m_goalPosition[client][0] = cappointpos[0] + GetRandomFloat(-768.0, 768.0);
			m_goalPosition[client][1] = cappointpos[1] + GetRandomFloat(-768.0, 768.0);
			m_goalPosition[client][2] = cappointpos[2] + GetRandomFloat(-768.0, 768.0);
		}
		else
		{
			m_goalPosition[client][0] = cappointpos[0] + GetRandomFloat(-192.0, 192.0);
			m_goalPosition[client][1] = cappointpos[1] + GetRandomFloat(-192.0, 192.0);
			m_goalPosition[client][2] = cappointpos[2] + 18;
		}
		FollowPath(client, m_goalPosition[client]);
	}
	else if (StrContains(currentMap, "ctf_" , false) != -1)
	{
		if (!HasEntProp(m_goalEntity[client], Prop_Send, "m_nFlagStatus"))
			SelectObjective(client);
		else
		{
			int flag;
			while ((flag = FindEntityByClassname(flag, "item_teamflag")) != INVALID_ENT_REFERENCE)
			{
				if (IsValidEntity(flag) && GetClientTeam(client) != GetTeamNumber(flag))
				{
					if (GetEntProp(flag, Prop_Send, "m_nFlagStatus") == 1)
					{
						if (!TF2_HasTheFlag(client))
						{
							int flag2;
							while ((flag2 = FindEntityByClassname(flag2, "item_teamflag")) != INVALID_ENT_REFERENCE)
							{
								if (IsValidEntity(flag2) && GetClientTeam(client) == GetTeamNumber(flag2))
								{
									if (GetEntProp(flag2, Prop_Send, "m_nFlagStatus") == 1) // did someone is picked our flag? chase that guy.
										FollowPath(client, GetOrigin(flag2));
									else // go to nearest enemy
									{
										int enemy = FindNearestEnemy(client);
										if (IsValidClient(enemy))
											FollowPath(client, GetOrigin(enemy));
										else if (GetVectorDistance(GetOrigin(client), GetOrigin(flag), true) > Squared(300)) // cover our flag
											FollowPath(client, GetOrigin(flag));
									}
								}
							}
						}
						else // go to flag capture area
							FollowPath(client, GetClientTeam(client) == 2 ? m_redFlagCapPoint : m_bluFlagCapPoint);
					}
					else
					{
						m_goalPosition[client] = GetClientTeam(client) == 2 ? m_redFlagCapPoint : m_bluFlagCapPoint;
						FollowPath(client, GetOrigin(flag));
					}
				}
			}
		}
	}
	else
	{
		m_goalPosition[client] = GetOrigin(m_goalEntity[client]);
		FollowPath(client, m_goalPosition[client]);
	}
}