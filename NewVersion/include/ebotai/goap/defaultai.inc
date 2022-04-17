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
	if (m_hasEnemiesNear[client] && TF2_IsPlayerInCondition(client, TFCond_CritOnWin))
	{
		LookUpdate(client);
		FollowPath(client, GetOrigin(m_nearestEnemy[client]));
		FindFriendsAndEnemiens(client);
		return;
	}

	FindFriendsAndEnemiens(client);

	if (m_hasEnemiesNear[client])
		SetProcess(client, PRO_ATTACK, false, 180.0, "", true);
	
	if (TF2_GetPlayerClass(client) == TFClass_Medic)
	{
		if (m_hasEnemiesNear[client] && m_hasFriendsNear[client] && m_friendDistance[client] <= m_enemyDistance[client])
			SetProcess(client, PRO_HEAL, true, 6.0, "", true);
		else if (!m_hasEnemiesNear[client] && m_hasFriendsNear[client])
			SetProcess(client, PRO_HEAL, true, 6.0, "", true);
	}

	CheckHealth(client);
	CheckAmmo(client);

	if (m_hasWaypoints)
	{
		ObjectiveMovement(client);

		int index = m_goalIndex[client];
		if (index == -1 || (m_paths[index].activeArea != 0 && m_paths[index].activeArea != currentActiveArea))
			SelectObjective(client);

		if (index != -1 && (m_paths[index].flags == _:WAYPOINT_SNIPER || m_paths[index].flags == _:WAYPOINT_DEFEND) && GetVectorDistance(GetOrigin(client), VectorAsFloat(m_paths[index].origin), true) <= Squared(128))
		{
			if (m_pauseTime[client] < GetGameTime())
			{
				if (GetRandomInt(1, 2) == 1)
					m_lookAt[client] = VectorAsFloat(m_paths[index].campStart);
				else
					m_lookAt[client] = VectorAsFloat(m_paths[index].campEnd);
				m_pauseTime[client] = GetGameTime() + GetRandomFloat(2.0, 7.0);
				if (m_hasEnemiesNear[client])
					m_lookAt[client] = GetEyePosition(m_nearestEnemy[client]);
			}
			if (TF2_GetPlayerClass(client) == TFClass_Sniper)
			{
				if (!TF2_IsPlayerInCondition(client, TFCond_Zoomed) && !IsMoving(client))
					m_buttons[client] |= IN_ATTACK2;
			}
			return;
		}
	}
	else
	{
		if (IsValidEntity(m_goalEntity[client]))
			ObjectiveMovement(client);
		else
			SelectObjective(client);
	}

	LookUpdate(client);
}

stock void DefaultEnd(int client)
{
	if (TF2_GetPlayerClass(client) == TFClass_Spy)
		return;
	m_goalEntity[client] = -1;
}

stock void SelectObjective(int client)
{
	if (m_hasWaypoints && TF2_GetPlayerClass(client) == TFClass_Sniper && m_hasSniperWaypoints)
	{
		int index = -1;
		ArrayList GoodSniperPoints = new ArrayList();
		ArrayList AllSniperPoints = new ArrayList();
		for (int i = 0; i < m_waypointNumber; i++)
		{
			// blocked waypoint
   			if (m_paths[i].activeArea != 0 && m_paths[i].activeArea != currentActiveArea)
				continue;

			if (m_paths[i].flags != _:WAYPOINT_SNIPER)
				continue;

    		// not for our team
			int cteam = GetClientTeam(client);
 			if (cteam == 3 && m_paths[i].team == 2)
    			continue;
				
   			if (cteam == 2 && m_paths[i].team == 3)
        		continue;

			if (m_hasEnemiesNear[client] && GetVectorDistance(VectorAsFloat(m_paths[i].origin), GetOrigin(m_nearestEnemy[client]), true) > Squared(768))
				GoodSniperPoints.Push(i);
			else
				AllSniperPoints.Push(i);
		}

		if (index == -1)
		{
			if (GoodSniperPoints.Length > 0)
				index = GoodSniperPoints.Get(GetRandomInt(0, GoodSniperPoints.Length - 1));
			else if (AllSniperPoints.Length > 0)
				index = AllSniperPoints.Get(GetRandomInt(0, AllSniperPoints.Length - 1));
		}

		delete GoodSniperPoints;
		delete AllSniperPoints;

		if (index != -1)
		{
			m_goalIndex[client] = index;
			if (m_paths[index].radius > 0)
			{
				m_goalPosition[client][0] = float(m_paths[index].origin[0] + GetRandomInt(-m_paths[index].radius, m_paths[index].radius));
				m_goalPosition[client][1] = float(m_paths[index].origin[1] + GetRandomInt(-m_paths[index].radius, m_paths[index].radius));
				m_goalPosition[client][2] = float(m_paths[index].origin[2]);
			}
			else
				m_goalPosition[client] = VectorAsFloat(m_paths[index].origin);
			return;
		}
	}

	if (StrContains(currentMap, "koth_" , false) != -1)
	{
		if (m_hasWaypoints)
		{
			int index = -1;
			for (int i = 0; i < m_waypointNumber; i++)
			{
				// blocked waypoint
   				if (m_paths[i].activeArea != 0 && m_paths[i].activeArea != currentActiveArea)
					continue;

    			// not for our team
    			int cteam = GetClientTeam(client);
 			   	if (cteam == 3 && m_paths[i].team == 2)
    			    continue;
				
   				if (cteam == 2 && m_paths[i].team == 3)
        			continue;
				
				if (m_paths[i].flags == _:WAYPOINT_CAPTUREPOINT)
				{
					index = i;
					break;
				}
			}

			if (index == -1)
				index = GetRandomInt(1, m_waypointNumber - 1);
			
			m_goalIndex[client] = index;
			if (m_paths[index].radius > 0)
			{
				m_goalPosition[client][0] = float(m_paths[index].origin[0] + GetRandomInt(-m_paths[index].radius, m_paths[index].radius));
				m_goalPosition[client][1] = float(m_paths[index].origin[1] + GetRandomInt(-m_paths[index].radius, m_paths[index].radius));
				m_goalPosition[client][2] = float(m_paths[index].origin[2]);
			}
			else
				m_goalPosition[client] = VectorAsFloat(m_paths[index].origin);
		}
		else
		{
			int capturepoint;
			if ((capturepoint = FindEntityByClassname(capturepoint, "team_control_point")) != INVALID_ENT_REFERENCE)
			{
				if (IsValidEntity(capturepoint))
					m_goalEntity[client] = capturepoint;
			}
		}
	}
	else if (StrContains(currentMap, "ctf_" , false) != -1)
	{
		// flag required for check status
		int flag = -1;
		while ((flag = FindEntityByClassname(flag, "item_teamflag")) != INVALID_ENT_REFERENCE)
		{
			if (IsValidEntity(flag) && HasEntProp(flag, Prop_Send, "m_nFlagStatus") && GetClientTeam(client) != GetTeamNumber(flag))
				m_goalEntity[client] = flag;
		}
		m_goalIndex[client] = GetRandomInt(1, m_waypointNumber - 1);
	}
	else if (StrContains(currentMap, "cp_" , false) != -1 || StrContains(currentMap, "tc_" , false) != -1 || StrContains(currentMap, "mvm_" , false) != -1)
	{
		if (m_hasWaypoints)
		{
			int index = -1;
			ArrayList ControlPoints = new ArrayList();
			ArrayList GoodDefendPoints = new ArrayList();
			ArrayList AllDefendPoints = new ArrayList();
			for (int i = 0; i < m_waypointNumber; i++)
			{
				// blocked waypoint
   				if (m_paths[i].activeArea != 0 && m_paths[i].activeArea != currentActiveArea)
					continue;

    			// not for our team
 			   	int cteam = GetClientTeam(client);
 			   	if (cteam == 3 && m_paths[i].team == 2)
    			    continue;
				
   				if (cteam == 2 && m_paths[i].team == 3)
        			continue;
				
				if (m_paths[i].flags == _:WAYPOINT_CAPTUREPOINT)
					ControlPoints.Push(i);
				
				if (m_paths[i].flags != _:WAYPOINT_DEFEND)
					continue;

				if (m_hasEnemiesNear[client] && GetVectorDistance(VectorAsFloat(m_paths[i].origin), GetOrigin(m_nearestEnemy[client]), true) > Squared(768))
					GoodDefendPoints.Push(i);
				else
					AllDefendPoints.Push(i);
			}

			if (index == -1)
			{
				if (ControlPoints.Length > 0)
					index = ControlPoints.Get(GetRandomInt(0, ControlPoints.Length - 1));
				else if (GoodDefendPoints.Length > 0)
					index = GoodDefendPoints.Get(GetRandomInt(0, GoodDefendPoints.Length - 1));
				else if (AllDefendPoints.Length > 0)
					index = AllDefendPoints.Get(GetRandomInt(0, AllDefendPoints.Length - 1));
			}

			delete ControlPoints;
			delete GoodDefendPoints;
			delete AllDefendPoints;
			
			if (index != -1)
			{
				m_goalIndex[client] = index;
				if (m_paths[index].radius > 0)
				{
					m_goalPosition[client][0] = float(m_paths[index].origin[0] + GetRandomInt(-m_paths[index].radius, m_paths[index].radius));
					m_goalPosition[client][1] = float(m_paths[index].origin[1] + GetRandomInt(-m_paths[index].radius, m_paths[index].radius));
					m_goalPosition[client][2] = float(m_paths[index].origin[2]);
				}
				else
					m_goalPosition[client] = VectorAsFloat(m_paths[index].origin);
			}
		}
	}
	else if (m_hasWaypoints)
		m_goalPosition[client] = VectorAsFloat(m_paths[GetRandomInt(1, m_waypointNumber - 1)].origin);
	else
		m_goalEntity[client] = FindNearestEnemy(client);
}

stock void ObjectiveMovement(int client)
{
	if (IsReloading(client))
		return;
	
	if (m_hasWaypoints && TF2_GetPlayerClass(client) == TFClass_Sniper && m_hasSniperWaypoints)
	{
		int index = m_goalIndex[client];
		if (index != -1 && m_paths[index].flags == _:WAYPOINT_SNIPER && (m_paths[index].activeArea == 0 || m_paths[index].activeArea == currentActiveArea))
		{
			if (GetVectorDistance(GetOrigin(client), m_goalPosition[client], true) <= Squared(4))
			{
				m_moveVel[client][0] = 0.0;
				m_moveVel[client][1] = 0.0;
				m_moveVel[client][2] = 0.0;
				m_nextStuckCheck[client] = GetGameTime() + 5.0;
				m_nextPathUpdate[client] = GetGameTime() + 5.0;
			}
			else
				FollowPath(client, m_goalPosition[client]);
		}
		else
			SelectObjective(client);
		
	}
	else if (StrContains(currentMap, "koth_" , false) != -1)
	{
		if (m_hasWaypoints)
		{
			if (GetVectorDistance(GetOrigin(client), m_goalPosition[client], true) <= Squared(4))
			{
				m_moveVel[client][0] = 0.0;
				m_moveVel[client][1] = 0.0;
				m_moveVel[client][2] = 0.0;
			}
			else
				FollowPath(client, m_goalPosition[client]);
		}
		else
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
	}
	else if (StrContains(currentMap, "ctf_" , false) != -1)
	{
		if (!IsValidEntity(m_goalEntity[client]) || !HasEntProp(m_goalEntity[client], Prop_Send, "m_nFlagStatus"))
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
	else if (StrContains(currentMap, "cp_" , false) != -1)
	{
		if (m_hasWaypoints)
		{
			if (GetVectorDistance(GetOrigin(client), m_goalPosition[client], true) <= Squared(4))
			{
				m_moveVel[client][0] = 0.0;
				m_moveVel[client][1] = 0.0;
				m_moveVel[client][2] = 0.0;
			}
			else
				FollowPath(client, m_goalPosition[client]);
		}
	}
	else if (m_hasWaypoints)
		FollowPath(client, m_goalPosition[client]);
	else
	{
		m_goalPosition[client] = GetOrigin(m_goalEntity[client]);
		FollowPath(client, m_goalPosition[client]);
	}
}