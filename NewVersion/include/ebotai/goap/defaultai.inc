public bool DefaultRequirements(int client)
{
	return true;
}

public void DefaultStart(int client)
{
	if (!GameRules_GetProp("m_bPlayingMedieval") && m_class[client] == TFClass_Engineer)
		SetProcess(client, PRO_ENGINEERIDLE, 99999.0, "", true);
	
	if (m_class[client] == TFClass_Spy)
		SetProcess(client, PRO_SPYLURK, 99999.0, "", true);
}

stock bool MustFindObjective(int client, int index, int current)
{
	if (current <= 0)
		return true;
	
	if (index <= 0 || ((m_primaryID[client] == 56 || m_primaryID[client] == 1005 || m_primaryID[client] == 1092) && m_paths[current].flags & WAYPOINT_SNIPER) || m_paths[index].flags & WAYPOINT_ROUTE || (m_paths[index].activeArea != 0 && !(m_paths[index].activeArea & currentActiveArea)) || (current > 0 && (m_paths[current].flags & WAYPOINT_ROUTE || m_paths[current].flags & WAYPOINT_RESUPPLY || m_paths[current].flags & WAYPOINT_HEALTH || m_paths[current].flags & WAYPOINT_AMMO || m_paths[current].flags & WAYPOINT_CAPTUREPOINT)))
		return true;

	if (isCP)
	{
		if (IsValidEntity(m_goalEntity[client]))
		{
			if (m_team[client] == GetTeamNumber(m_goalEntity[client]))
				return true;
		}
		else
			return true;
	}

	if (isVSH || isDM)
	{
		if (!IsValidClient(m_goalEntity[client]) || m_team[client] == m_team[m_goalEntity[client]])
			return true;
	}
	
	return false;
}

public void DefaultUpdate(int client)
{
	if (IsMedic(client) || HasCrossbow(client))
	{
		if (m_hasEnemiesNear[client] && m_hasFriendsNear[client] && m_friendDistance[client] <= m_enemyDistance[client])
			SetProcess(client, PRO_HEAL, (m_difficulty[client] + 8.0), "", true);
		else if (!m_hasEnemiesNear[client] && m_hasFriendsNear[client])
			SetProcess(client, PRO_HEAL, (m_difficulty[client] + 8.0), "", true);
	}
	else if (m_class[client] == TFClass_Spy)
	{
		CurrentProcess[client] = PRO_SPYLURK;
		return;
	}
	else if (m_class[client] == TFClass_Engineer)
	{
		CurrentProcess[client] = PRO_ENGINEERIDLE;
		return;
	}

	FindFriendsAndEnemiens(client);
	FindEnemyEntities(client);

	if (m_hasEnemiesNear[client])
		SetProcess(client, PRO_ATTACK, 180.0, "", true);
	
	if (m_useTeleporter[client] && IsValidEntity(m_teleporterEntity[client]))
	{
		if (GetEntProp(m_teleporterEntity[client], Prop_Send, "m_iState") != 2 && GetEntProp(m_teleporterEntity[client], Prop_Send, "m_iState") != 6)
		{
			m_useTeleporter[client] = false;
			return;
		}

		DeletePathNodes(client);
		m_nextStuckCheck[client] = GetGameTime() + 5.0;
		if (GetVectorDistance(GetOrigin(client), GetOrigin(m_teleporterEntity[client]), true) <= Squaredf(24.0))
		{
			m_moveVel[client][0] = 0.0;
			m_moveVel[client][1] = 0.0;
			m_moveVel[client][2] = 0.0;
		}
		else
			MoveToWaypoint(client, GetOrigin(m_teleporterEntity[client]), GetOrigin(m_teleporterEntity[client]));
		return;
	}

	CheckHideFromDanger(client);
	CheckHealth(client);
	CheckAmmo(client);

	ObjectiveMovement(client);

	int index = m_goalIndex[client];
	int current = m_currentIndex[client];
	if (MustFindObjective(client, index, current))
	{
		SelectObjective(client);
		FollowPath(client, m_goalPosition[client], -1, index);
	}

	if (index != -1 && (m_paths[index].flags & WAYPOINT_DEMOMANCAMP || m_paths[index].flags & WAYPOINT_SNIPER || m_paths[index].flags & WAYPOINT_DEFEND) && GetVectorDistance(GetOrigin(client), m_paths[index].origin, true) <= Squaredf(128.0))
	{
		if (IsSniper(client))
		{
			if (m_primaryID[client] != 1098 && !TF2_IsPlayerInCondition(client, TFCond_Zoomed) && !IsMoving(client))
				m_buttons[client] |= IN_ATTACK2;
		}
		
		if (m_class[client] == TFClass_DemoMan && m_paths[index].flags & WAYPOINT_DEMOMANCAMP)
		{
			if (m_stickyCount[client] < 4)
				m_lookAt[client] = m_paths[index].campStart;
			else if (m_stickyCount[client] < 8)
				m_lookAt[client] = m_paths[index].campEnd;

			if (m_stickyCount[client] < 8)
			{
				if (IsWeaponSlotActive(client, 1))
				{
					if (m_attackTimer[client] + 0.125 < GetGameTime())
						m_attackTimer[client] = GetGameTime() + 0.125;
				}
				else
					EquipWeaponSlot(client, 1);

				return;
			}
			else
				EquipWeaponSlot(client, 0);
		}

		if (m_pauseTime[client] < GetGameTime())
		{
			if (GetRandomInt(1, 2) == 1)
				m_lookAt[client] = m_paths[index].campStart;
			else
				m_lookAt[client] = m_paths[index].campEnd;
			
			m_pauseTime[client] = GetGameTime() + GetRandomFloat(2.0, 7.0);
		}

		return;
	}

	LookUpdate(client);
}

public void DefaultEnd(int client)
{
	if (m_class[client] == TFClass_Spy)
		return;
	
	if (m_class[client] == TFClass_Engineer)
		return;

	m_goalEntity[client] = -1;
	m_goalIndex[client] = -1;
	m_goalPosition[client] = NULL_VECTOR;
}

public void SelectObjective(int client)
{
	if (m_hasWaypoints && m_hasSniperWaypoints && IsSniper(client) && !TF2_HasTheFlag(client))
	{
		int myindex = m_goalIndex[client];
		if (myindex != -1 && m_paths[myindex].flags & WAYPOINT_SNIPER && (m_paths[myindex].activeArea == 0 || m_paths[myindex].activeArea & currentActiveArea))
			return;
		
		int index = -1;
		ArrayList GoodSniperPoints = new ArrayList();
		ArrayList AllSniperPoints = new ArrayList();
		for (int i = 0; i < m_waypointNumber; i++)
		{
			if (!(m_paths[i].flags & WAYPOINT_SNIPER))
				continue;
			
			// blocked waypoint
   			if (m_paths[i].activeArea != 0 && !(m_paths[i].activeArea & currentActiveArea))
				continue;

    		// not for our team
 			if (m_paths[i].team == 2 && m_team[client] == 3)
                continue;
            else if (m_paths[i].team == 3 && m_team[client] == 2)
                continue;

			if (!IsWaypointUsed(i))
				GoodSniperPoints.Push(i);
			else if (m_paths[i].radius > 32.0)
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
			if (m_paths[index].radius > 0.0)
			{
				m_goalPosition[client][0] = m_paths[index].origin[0] + GetRandomFloat(-m_paths[index].radius, m_paths[index].radius);
				m_goalPosition[client][1] = m_paths[index].origin[1] + GetRandomFloat(-m_paths[index].radius, m_paths[index].radius);
				m_goalPosition[client][2] = m_paths[index].origin[2];
			}
			else
				m_goalPosition[client] = m_paths[index].origin;
			
			return;
		}
	}

	if (m_hasWaypoints && m_hasStickyWaypoints && m_class[client] == TFClass_DemoMan && !TF2_HasTheFlag(client))
	{
		int myindex = m_goalIndex[client];
		if (myindex != -1 && m_paths[myindex].flags & WAYPOINT_DEMOMANCAMP && (m_paths[myindex].activeArea == 0 || m_paths[myindex].activeArea & currentActiveArea))
			return;
		
		int index = -1;
		ArrayList GoodSniperPoints = new ArrayList();
		ArrayList AllSniperPoints = new ArrayList();
		for (int i = 0; i < m_waypointNumber; i++)
		{
			if (!(m_paths[i].flags & WAYPOINT_DEMOMANCAMP))
				continue;
			
			// blocked waypoint
   			if (m_paths[i].activeArea != 0 && !(m_paths[i].activeArea & currentActiveArea))
				continue;

    		// not for our team
 			if (m_paths[i].team == 2 && m_team[client] == 3)
                continue;
            else if (m_paths[i].team == 3 && m_team[client] == 2)
                continue;

			if (!IsWaypointUsed(i))
				GoodSniperPoints.Push(i);
			else if (m_paths[i].radius > 32.0)
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
			if (m_paths[index].radius > 0.0)
			{
				m_goalPosition[client][0] = m_paths[index].origin[0] + GetRandomFloat(-m_paths[index].radius, m_paths[index].radius);
				m_goalPosition[client][1] = m_paths[index].origin[1] + GetRandomFloat(-m_paths[index].radius, m_paths[index].radius);
				m_goalPosition[client][2] = m_paths[index].origin[2];
			}
			else
				m_goalPosition[client] = m_paths[index].origin;
			
			return;
		}
	}

	if (isVSH || isDM || GetConVarBool(EBotForceHuntEnemy))
	{
		m_goalEntity[client] = FindNearestEnemy(client);
		if (IsValidClient(m_goalEntity[client]))
			m_goalPosition[client] = GetOrigin(m_goalEntity[client]);
		else if (m_hasWaypoints)
		{
			m_goalIndex[client] = GetRandomInt(1, m_waypointNumber - 1);
			if (m_paths[m_goalIndex[client]].radius > 0.0)
			{
				m_goalPosition[client][0] = m_paths[m_goalIndex[client]].origin[0] + GetRandomFloat(-m_paths[m_goalIndex[client]].radius, m_paths[m_goalIndex[client]].radius);
				m_goalPosition[client][1] = m_paths[m_goalIndex[client]].origin[1] + GetRandomFloat(-m_paths[m_goalIndex[client]].radius, m_paths[m_goalIndex[client]].radius);
				m_goalPosition[client][2] = m_paths[m_goalIndex[client]].origin[2];
			}
			else
				m_goalPosition[client] = m_paths[m_goalIndex[client]].origin;
		}

		return;
	}

	if (isKOTH)
	{
		if (m_hasWaypoints)
		{
			int index = -1;
			ArrayList GoodDefendPoints = new ArrayList();
			
			for (int i = 0; i < m_waypointNumber; i++)
			{
				// blocked waypoint
   				if (m_paths[i].activeArea != 0 && !(m_paths[i].activeArea & currentActiveArea))
					continue;

    			// not for our team
 			   	if (m_paths[i].team == 2 && m_team[client] == 3)
             	   continue;
            	else if (m_paths[i].team == 3 && m_team[client] == 2)
                	continue;
				
				if (m_paths[i].flags & WAYPOINT_CAPTUREPOINT && m_team[client] != GetTeamNumber(m_capturePoint))
				{
					index = i
					break;
				}
				
				if (!(m_paths[i].flags & WAYPOINT_DEFEND))
					continue;

				if (!IsWaypointUsed(i))
					GoodDefendPoints.Push(i);
				else if (m_paths[i].radius > 32.0)
					GoodDefendPoints.Push(i);
			}

			if (index == -1)
			{
				if (GoodDefendPoints.Length > 0)
					index = GoodDefendPoints.Get(GetRandomInt(0, GoodDefendPoints.Length - 1));
			}

			delete GoodDefendPoints;
			
			if (index != -1)
			{
				m_goalIndex[client] = index;
				if (m_paths[index].radius > 0.0)
				{
					m_goalPosition[client][0] = m_paths[index].origin[0] + GetRandomFloat(-m_paths[index].radius, m_paths[index].radius);
					m_goalPosition[client][1] = m_paths[index].origin[1] + GetRandomFloat(-m_paths[index].radius, m_paths[index].radius);
					m_goalPosition[client][2] = m_paths[index].origin[2];
				}
				else
					m_goalPosition[client] = m_paths[index].origin;
			}
			else
			{
				m_goalEntity[client] = FindNearestEnemy(client);
				if (IsValidClient(m_goalEntity[client]))
				{
					m_goalPosition[client] = GetOrigin(m_goalEntity[client]);
					SetGoalIndex(client, FindNearestWaypoint(m_goalPosition[client], 999999.0, client));
				}
				else
					SetGoalIndex(client, GetRandomInt(1, m_waypointNumber));
			}
		}
		else
			m_goalEntity[client] = m_capturePoint;
	}
	else if (isPL)
	{
		if (!IsValidEntity(m_bluPayload))
			FindPayload();
		
		if (m_hasWaypoints)
		{
			if (m_team[client] == 2)
			{
				int index = -1;
				ArrayList GoodDefendPoints = new ArrayList();
				ArrayList AllDefendPoints = new ArrayList();
				for (int i = 0; i < m_waypointNumber; i++)
				{
					if (m_lastFailedWaypoint[client] == i)
						continue;

					// blocked waypoint
   					if (m_paths[i].activeArea != 0 && !(m_paths[i].activeArea & currentActiveArea))
						continue;

    				// not for our team
   					if (m_paths[i].team == 2 && m_team[client] == 3)
               		 	continue;
            		else if (m_paths[i].team == 3 && m_team[client] == 2)
             		   	continue;
				
					if (!(m_paths[i].flags & WAYPOINT_DEFEND))
						continue;

					if (!IsWaypointUsed(i))
						GoodDefendPoints.Push(i);
					else if (m_paths[i].radius > 32.0)
						GoodDefendPoints.Push(i);
					else
						AllDefendPoints.Push(i);
				}

				if (index == -1)
				{
					if (GoodDefendPoints.Length > 0)
						index = GoodDefendPoints.Get(GetRandomInt(0, GoodDefendPoints.Length - 1));
					else if (AllDefendPoints.Length > 0)
						index = AllDefendPoints.Get(GetRandomInt(0, AllDefendPoints.Length - 1));
				}
				
				delete GoodDefendPoints;
				delete AllDefendPoints;
			
				if (index != -1)
				{
					m_goalIndex[client] = index;
					if (m_paths[index].radius > 0.0)
					{
						m_goalPosition[client][0] = m_paths[index].origin[0] + GetRandomFloat(-m_paths[index].radius, m_paths[index].radius);
						m_goalPosition[client][1] = m_paths[index].origin[1] + GetRandomFloat(-m_paths[index].radius, m_paths[index].radius);
						m_goalPosition[client][2] = m_paths[index].origin[2];
					}
					else
						m_goalPosition[client] = m_paths[index].origin;
				}
				else if (IsValidEntity(m_bluPayload))
				{
					m_goalEntity[client] = m_bluPayload;
					m_goalPosition[client] = GetOrigin(m_bluPayload);
					m_goalPosition[client][0] += GetRandomFloat(-768.0, 768.0);
					m_goalPosition[client][1] += GetRandomFloat(-768.0, 768.0);
					m_goalPosition[client][2] += GetRandomFloat(0.0, 512.0);
					SetGoalIndex(client, FindNearestWaypoint(m_goalPosition[client], 999999.0, client));
				}
			}
			else if (IsValidEntity(m_bluPayload))
			{
				m_goalEntity[client] = m_bluPayload;
				m_goalPosition[client] = GetOrigin(m_bluPayload);
				m_goalPosition[client][0] += GetRandomFloat(-64.0, 64.0);
				m_goalPosition[client][1] += GetRandomFloat(-64.0, 64.0);
				m_goalPosition[client][2] += GetRandomFloat(-64.0, 64.0);
				SetGoalIndex(client, FindNearestWaypoint(m_goalPosition[client], 999999.0, client));
			}
		}
		else if (IsValidEntity(m_bluPayload))
		{
			m_goalEntity[client] = m_bluPayload;
			if (m_team[client] == 2)
			{
				m_goalPosition[client] = GetOrigin(m_bluPayload);
				m_goalPosition[client][0] += GetRandomFloat(-768.0, 768.0);
				m_goalPosition[client][1] += GetRandomFloat(-768.0, 768.0);
				m_goalPosition[client][2] += GetRandomFloat(0.0, 512.0);
			}
			else
			{
				m_goalPosition[client] = GetOrigin(m_bluPayload);
				m_goalPosition[client][0] += GetRandomFloat(-64.0, 64.0);
				m_goalPosition[client][1] += GetRandomFloat(-64.0, 64.0);
				m_goalPosition[client][2] += GetRandomFloat(-64.0, 64.0);
			}
		}
	}
	else if (isPLR)
	{
		if (!IsValidEntity(m_bluPayload) || !IsValidEntity(m_redPayload))
			FindPayload();
		
		if (m_team[client] == 2 && IsValidEntity(m_redPayload))
		{
			m_goalPosition[client] = GetOrigin(m_redPayload);
			m_goalPosition[client][0] += GetRandomFloat(-64.0, 64.0);
			m_goalPosition[client][1] += GetRandomFloat(-64.0, 64.0);
			m_goalPosition[client][2] += GetRandomFloat(-64.0, 64.0);
			SetGoalIndex(client, FindNearestWaypoint(m_goalPosition[client], 999999.0, client));
			m_goalEntity[client] = m_redPayload;
		}
		else if (IsValidEntity(m_bluPayload))
		{
			m_goalPosition[client] = GetOrigin(m_bluPayload);
			m_goalPosition[client][0] += GetRandomFloat(-64.0, 64.0);
			m_goalPosition[client][1] += GetRandomFloat(-64.0, 64.0);
			m_goalPosition[client][2] += GetRandomFloat(-64.0, 64.0);
			SetGoalIndex(client, FindNearestWaypoint(m_goalPosition[client], 999999.0, client));
			m_goalEntity[client] = m_bluPayload;
		}
	}
	else if (isCTF)
	{
		// flag required for check status
		int flag = -1;
		while ((flag = FindEntityByClassname(flag, "item_teamflag")) != INVALID_ENT_REFERENCE)
		{
			if (IsValidEntity(flag) && HasEntProp(flag, Prop_Send, "m_nFlagStatus") && m_team[client] != GetTeamNumber(flag))
				m_goalEntity[client] = flag;
		}
		m_goalIndex[client] = GetRandomInt(1, m_waypointNumber - 1);
	}
	else if (isMVM || isAD)
	{
		if (m_hasWaypoints)
		{
			int index = -1;
			ArrayList ControlPoints = new ArrayList();
			ArrayList GoodDefendPoints = new ArrayList();
			ArrayList AllDefendPoints = new ArrayList();
			for (int i = 0; i < m_waypointNumber; i++)
			{
				if (m_lastFailedWaypoint[client] == i)
					continue;
				
				// blocked waypoint
   				if (m_paths[i].activeArea != 0 && !(m_paths[i].activeArea & currentActiveArea))
					continue;

    			// not for our team
   				if (m_paths[i].team == 2 && m_team[client] == 3)
                	continue;
            	else if (m_paths[i].team == 3 && m_team[client] == 2)
             	   	continue;
				
				if (m_paths[i].flags & WAYPOINT_CAPTUREPOINT)
					ControlPoints.Push(i);
				
				if (!(m_paths[i].flags & WAYPOINT_DEFEND))
					continue;

				if (!IsWaypointUsed(i))
					GoodDefendPoints.Push(i);
				else if (m_paths[i].radius > 32.0)
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
				if (m_paths[index].radius > 0.0)
				{
					m_goalPosition[client][0] = m_paths[index].origin[0] + GetRandomFloat(-m_paths[index].radius, m_paths[index].radius);
					m_goalPosition[client][1] = m_paths[index].origin[1] + GetRandomFloat(-m_paths[index].radius, m_paths[index].radius);
					m_goalPosition[client][2] = m_paths[index].origin[2];
				}
				else
					m_goalPosition[client] = m_paths[index].origin;
			}
			else
			{
				m_goalIndex[client] = GetRandomInt(1, m_waypointNumber - 1);
				if (m_paths[m_goalIndex[client]].radius > 0.0)
				{
					m_goalPosition[client][0] = m_paths[m_goalIndex[client]].origin[0] + GetRandomFloat(-m_paths[m_goalIndex[client]].radius, m_paths[m_goalIndex[client]].radius);
					m_goalPosition[client][1] = m_paths[m_goalIndex[client]].origin[1] + GetRandomFloat(-m_paths[m_goalIndex[client]].radius, m_paths[m_goalIndex[client]].radius);
					m_goalPosition[client][2] = m_paths[m_goalIndex[client]].origin[2];
				}
				else
					m_goalPosition[client] = m_paths[m_goalIndex[client]].origin;
			}
		}
		else
		{
			m_goalEntity[client] = FindNearestEnemy(client);
			if (isAD)
				m_goalPosition[client] = GetOrigin(m_goalEntity[client]);
		}
	}
	else if (isCP)
	{
		if (IsValidEntity(m_goalEntity[client]) && m_team[client] != GetTeamNumber(m_goalEntity[client]))
		{
			m_goalIndex[client] = FindNearestWaypoint(GetOrigin(m_goalEntity[client]));
			if (m_paths[m_goalIndex[client]].radius > 0.0)
			{
				m_goalPosition[client][0] = m_paths[m_goalIndex[client]].origin[0] + GetRandomFloat(-m_paths[m_goalIndex[client]].radius, m_paths[m_goalIndex[client]].radius);
				m_goalPosition[client][1] = m_paths[m_goalIndex[client]].origin[1] + GetRandomFloat(-m_paths[m_goalIndex[client]].radius, m_paths[m_goalIndex[client]].radius);
				m_goalPosition[client][2] = m_paths[m_goalIndex[client]].origin[2];
			}
			else
				m_goalPosition[client] = m_paths[m_goalIndex[client]].origin;
		}
		else
			m_goalEntity[client] = GetNearestEnemyControlPoint(client);
				
	}
	else if (m_hasWaypoints)
	{
		m_goalIndex[client] = GetRandomInt(1, m_waypointNumber - 1);
		if (m_paths[m_goalIndex[client]].radius > 0.0)
		{
			m_goalPosition[client][0] = m_paths[m_goalIndex[client]].origin[0] + GetRandomFloat(-m_paths[m_goalIndex[client]].radius, m_paths[m_goalIndex[client]].radius);
			m_goalPosition[client][1] = m_paths[m_goalIndex[client]].origin[1] + GetRandomFloat(-m_paths[m_goalIndex[client]].radius, m_paths[m_goalIndex[client]].radius);
			m_goalPosition[client][2] = m_paths[m_goalIndex[client]].origin[2];
		}
		else
			m_goalPosition[client] = m_paths[m_goalIndex[client]].origin;
	}
	else
		m_goalEntity[client] = FindNearestEnemy(client);
}

stock void ObjectiveMovement(int client)
{
	if (GetConVarBool(EBotForceHuntEnemy))
	{
		if (m_hasWaypoints)
			FollowPath(client, m_goalPosition[client], -1, m_goalIndex[client]);
		else
		{
			m_goalPosition[client] = GetOrigin(m_goalEntity[client]);
			FollowPath(client, m_goalPosition[client]);
		}

		return;
	}

	if (m_hasWaypoints && m_class[client] == TFClass_DemoMan && m_hasStickyWaypoints)
	{
		int index = m_goalIndex[client];
		if (index != -1 && (m_paths[index].activeArea == 0 || m_paths[index].activeArea & currentActiveArea))
		{
			if (GetVectorDistance(GetOrigin(client), m_goalPosition[client], true) <= Squaredf(4.0))
			{
				m_moveVel[client][0] = 0.0;
				m_moveVel[client][1] = 0.0;
				m_moveVel[client][2] = 0.0;
				m_nextStuckCheck[client] = GetGameTime() + 5.0;
			}
			else
				FollowPath(client, m_goalPosition[client]);
		}
		else
			SelectObjective(client);
	}
	else if (m_hasWaypoints && IsSniper(client) && m_hasSniperWaypoints)
	{
		int index = m_goalIndex[client];
		if (index != -1 && (m_paths[index].activeArea == 0 || m_paths[index].activeArea & currentActiveArea))
		{
			if (GetVectorDistance(GetOrigin(client), m_goalPosition[client], true) <= Squaredf(4.0))
			{
				m_moveVel[client][0] = 0.0;
				m_moveVel[client][1] = 0.0;
				m_moveVel[client][2] = 0.0;
				m_nextStuckCheck[client] = GetGameTime() + 5.0;
			}
			else
				FollowPath(client, m_goalPosition[client]);
		}
		else
			SelectObjective(client);
	}
	else if (isKOTH)
	{
		if (m_hasWaypoints)
			FollowPath(client, m_goalPosition[client]);
		else if (IsValidEntity(m_goalEntity[client]))
		{
			float cappointpos[3];
			cappointpos = GetOrigin(m_goalEntity[client]);
			if (GetTeamNumber(m_goalEntity[client]) == m_team[client])
			{
				m_goalPosition[client][0] = cappointpos[0] + GetRandomFloat(-768.0, 768.0);
				m_goalPosition[client][1] = cappointpos[1] + GetRandomFloat(-768.0, 768.0);
				m_goalPosition[client][2] = cappointpos[2] + GetRandomFloat(-768.0, 768.0);
			}
			else
			{
				m_goalPosition[client][0] = cappointpos[0] + GetRandomFloat(-192.0, 192.0);
				m_goalPosition[client][1] = cappointpos[1] + GetRandomFloat(-192.0, 192.0);
				m_goalPosition[client][2] = cappointpos[2] + 18.0;
			}
			
			FollowPath(client, m_goalPosition[client]);
		}
	}
	else if (isPL)
	{
		if (m_hasWaypoints)
			FollowPath(client, m_goalPosition[client], -1, m_goalIndex[client]);
		else
			FollowPath(client, m_goalPosition[client]);
	}
	else if (isPLR)
	{
		if (m_hasWaypoints)
			FollowPath(client, m_goalPosition[client], -1, m_goalIndex[client]);
		else
			FollowPath(client, m_goalPosition[client]);
	}
	else if (isCTF)
	{
		if (!IsValidEntity(m_goalEntity[client]) || !HasEntProp(m_goalEntity[client], Prop_Send, "m_nFlagStatus"))
			SelectObjective(client);
		else
		{
			int flag;
			while ((flag = FindEntityByClassname(flag, "item_teamflag")) != INVALID_ENT_REFERENCE)
			{
				if (IsValidEntity(flag) && m_team[client] != GetTeamNumber(flag))
				{
					if (GetEntProp(flag, Prop_Send, "m_nFlagStatus") == 1)
					{
						if (!TF2_HasTheFlag(client))
						{
							int flag2;
							while ((flag2 = FindEntityByClassname(flag2, "item_teamflag")) != INVALID_ENT_REFERENCE)
							{
								if (IsValidEntity(flag2) && m_team[client] == GetTeamNumber(flag2))
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
							FollowPath(client, m_team[client] == 2 ? m_redFlagCapPoint : m_bluFlagCapPoint);
					}
					else
					{
						m_goalPosition[client] = m_team[client] == 2 ? m_redFlagCapPoint : m_bluFlagCapPoint;
						FollowPath(client, GetOrigin(flag));
					}
				}
			}
		}
	}
	else if (isAD || isCP)
		FollowPath(client, m_goalPosition[client], -1, m_goalIndex[client]);
	else if (m_hasWaypoints)
	{
		FollowPath(client, m_goalPosition[client], -1, m_goalIndex[client]);
		if (m_isSlowThink[client])
			SelectObjective(client);
	}
	else
	{
		m_goalPosition[client] = GetOrigin(m_goalEntity[client]);
		FollowPath(client, m_goalPosition[client]);
	}
}

stock void NavMeshControlPoint(int client)
{
	if (!m_isSlowThink[client])
		return;
	
	m_goalEntity[client] = GetNearestEnemyControlPoint(client);
	m_goalPosition[client] = GetOrigin(m_goalEntity[client]);
}