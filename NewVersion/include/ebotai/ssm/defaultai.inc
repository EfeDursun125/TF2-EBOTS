public bool DefaultRequirements(const int client)
{
	return true;
}

public void DefaultStart(const int client)
{
	if (!GameRules_GetProp("m_bPlayingMedieval") && m_class[client] == TFClass_Engineer)
	{
		RememberedProcess[client] = PRO_ENGINEERIDLE;
		CurrentProcess[client] = PRO_ENGINEERIDLE;
		EngineerIdleStart(client);
		return;
	}

	if (m_class[client] == TFClass_Spy)
	{
		RememberedProcess[client] = PRO_SPYLURK;
		CurrentProcess[client] = PRO_SPYLURK;
		SpyLurkStart(client);
		return;
	}
}

stock bool MustFindObjective(const int client, const int index, const int current)
{
	if (isCTF)
		return true;

	if (current == -1)
		return true;

	if (index == -1 || ((m_primaryID[client] == 56 || m_primaryID[client] == 1005 || m_primaryID[client] == 1092) && m_paths[current].flags & WAYPOINT_SNIPER) || m_paths[index].flags & WAYPOINT_ROUTE || (m_paths[index].activeArea != 0 && !(m_paths[index].activeArea & currentActiveArea)) || (current != -1 && (m_paths[current].flags & WAYPOINT_ROUTE || m_paths[current].flags & WAYPOINT_RESUPPLY || m_paths[current].flags & WAYPOINT_HEALTH || m_paths[current].flags & WAYPOINT_AMMO || m_paths[current].flags & WAYPOINT_CAPTUREPOINT)))
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

	if (isDM)
	{
		if (!IsValidClient(m_goalEntity[client]) || m_team[client] == m_team[m_goalEntity[client]])
			return true;
	}
	
	return false;
}

public void DefaultUpdate(const int client)
{
	if (IsMedic(client) || HasCrossbow(client))
	{
		if (m_hasEnemiesNear[client] && m_hasFriendsNear[client] && m_friendDistance[client] < m_enemyDistance[client])
			SetProcess(client, PRO_HEAL, (m_difficulty[client] + 8.0), "", true);
		else if (!m_hasEnemiesNear[client] && m_hasFriendsNear[client])
			SetProcess(client, PRO_HEAL, (m_difficulty[client] + 8.0), "", true);
	}
	else if (m_class[client] == TFClass_Spy)
	{
		RememberedProcess[client] = PRO_SPYLURK;
		CurrentProcess[client] = PRO_SPYLURK;
		SpyLurkUpdate(client);
		return;
	}
	else if (m_class[client] == TFClass_Engineer)
	{
		RememberedProcess[client] = PRO_ENGINEERIDLE;
		CurrentProcess[client] = PRO_ENGINEERIDLE;
		EngineerIdleUpdate(client);
		return;
	}

	FindFriendsAndEnemiens(client);
	FindEnemyEntities(client);

	if (m_hasEnemiesNear[client])
		SetProcess(client, PRO_ATTACK, 180.0, "", true, false);
	
	if (m_useTeleporter[client] && IsValidEntity(m_teleporterEntity[client]))
	{
		if (GetEntProp(m_teleporterEntity[client], Prop_Send, "m_iState") != 2 && GetEntProp(m_teleporterEntity[client], Prop_Send, "m_iState") != 6)
		{
			m_useTeleporter[client] = false;
			return;
		}

		DeletePathNodes(client);
		m_nextStuckCheck[client] = GetGameTime() + 5.0;
		if (GetVectorDistance(GetOrigin(client), GetOrigin(m_teleporterEntity[client]), true) < Squaredf(24.0))
		{
			m_moveVel[client][0] = 0.0;
			m_moveVel[client][1] = 0.0;
			m_moveVel[client][2] = 0.0;
		}
		else
			MoveToWaypoint(client, GetOrigin(m_teleporterEntity[client]));
		
		return;
	}

	CheckHideFromDanger(client);
	CheckHealth(client);
	CheckAmmo(client);

	ObjectiveMovement(client);

	if (MustFindObjective(client, m_goalIndex[client], m_currentIndex[client]))
	{
		SelectObjective(client);
		FollowPath(client, m_goalPosition[client], -1, m_goalIndex[client]);
		return;
	}

	if (m_goalIndex[client] != -1 && (m_paths[m_goalIndex[client]].flags & WAYPOINT_DEMOMANCAMP || m_paths[m_goalIndex[client]].flags & WAYPOINT_SNIPER || m_paths[m_goalIndex[client]].flags & WAYPOINT_DEFEND) && GetVectorDistance(GetOrigin(client), m_paths[m_goalIndex[client]].origin, true) < Squaredf(128.0))
	{
		if (IsSniper(client))
		{
			if (m_primaryID[client] != 1098 && !TF2_IsPlayerInCondition(client, TFCond_Zoomed) && !IsMoving(client))
				m_buttons[client] |= IN_ATTACK2;
		}
		
		if (m_class[client] == TFClass_DemoMan && m_paths[m_goalIndex[client]].flags & WAYPOINT_DEMOMANCAMP)
		{
			if (m_stickyCount[client] < 4)
				m_lookAt[client] = m_paths[m_goalIndex[client]].campStart;
			else if (m_stickyCount[client] < 8)
				m_lookAt[client] = m_paths[m_goalIndex[client]].campEnd;

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
			if (crandomint(1, 2) == 1 && (!IsSniper(client) || IsVisible(GetEyePosition(client), m_paths[m_goalIndex[client]].campStart)))
				m_lookAt[client] = m_paths[m_goalIndex[client]].campStart;
			else if (!IsSniper(client) || IsVisible(GetEyePosition(client), m_paths[m_goalIndex[client]].campStart))
				m_lookAt[client] = m_paths[m_goalIndex[client]].campEnd;
			else
				m_goalIndex[client] = -1;
			
			m_pauseTime[client] = GetGameTime() + crandomfloat(2.0, 7.0);
		}

		return;
	}

	LookUpdate(client);
}

public void DefaultEnd(const int client)
{
	if (m_class[client] == TFClass_Spy)
		return;
	
	if (m_class[client] == TFClass_Engineer)
		return;

	m_goalEntity[client] = -1;
	m_goalIndex[client] = -1;
	m_goalPosition[client] = NULL_VECTOR;
}

public void SelectObjective(const int client)
{
	int i;
	int index;
	if ((m_hasWaypoints || m_hasNavpoints) && m_hasSniperWaypoints && IsSniper(client) && !TF2_HasTheFlag(client))
	{
		if (m_goalIndex[client] != -1 && m_paths[m_goalIndex[client]].flags & WAYPOINT_SNIPER && (!m_paths[m_goalIndex[client]].activeArea || m_paths[m_goalIndex[client]].activeArea & currentActiveArea))
			return;

		ArrayList BestSniperPoints = new ArrayList();
		if (!BestSniperPoints)
			return;

		ArrayList GoodSniperPoints = new ArrayList();
		if (!GoodSniperPoints)
			return;

		ArrayList AllSniperPoints = new ArrayList();
		if (!AllSniperPoints)
			return;

		index = -1;
		for (i = 0; i < m_waypointNumber; i++)
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
			
			if (m_paths[i].radius > 32.0 || !IsWaypointUsed(i))
			{
				if (GetDamage(client, i) < GetMaxDamage(client))
					BestSniperPoints.Push(i);
				else
					GoodSniperPoints.Push(i);
			}
			else
				AllSniperPoints.Push(i);
		}

		if (BestSniperPoints.Length)
			index = BestSniperPoints.Get(crandomint(0, BestSniperPoints.Length - 1));
		else if (GoodSniperPoints.Length)
			index = GoodSniperPoints.Get(crandomint(0, GoodSniperPoints.Length - 1));
		else if (AllSniperPoints.Length)
			index = AllSniperPoints.Get(crandomint(0, AllSniperPoints.Length - 1));
		
		delete BestSniperPoints;
		delete GoodSniperPoints;
		delete AllSniperPoints;

		if (index != -1)
		{
			m_goalIndex[client] = index;
			if (m_paths[index].radius > 0.0)
			{
				m_goalPosition[client][0] = m_paths[index].origin[0] + crandomfloat(-m_paths[index].radius, m_paths[index].radius);
				m_goalPosition[client][1] = m_paths[index].origin[1] + crandomfloat(-m_paths[index].radius, m_paths[index].radius);
				m_goalPosition[client][2] = m_paths[index].origin[2];
			}
			else
				m_goalPosition[client] = m_paths[index].origin;
			
			return;
		}
	}

	if ((m_hasWaypoints || m_hasNavpoints) && m_hasStickyWaypoints && m_class[client] == TFClass_DemoMan && !TF2_HasTheFlag(client))
	{
		if (m_goalIndex[client] != -1 && m_paths[m_goalIndex[client]].flags & WAYPOINT_DEMOMANCAMP && (!m_paths[m_goalIndex[client]].activeArea || m_paths[m_goalIndex[client]].activeArea & currentActiveArea))
			return;

		ArrayList BestDemoSpots = new ArrayList();
		if (!BestDemoSpots)
			return;

		ArrayList GoodDemoSpots = new ArrayList();
		if (!GoodDemoSpots)
			return;

		ArrayList AllDemoSpots = new ArrayList();
		if (!AllDemoSpots)
			return;

		index = -1;
		for (i = 0; i < m_waypointNumber; i++)
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

			if (m_paths[i].radius > 32.0 || !IsWaypointUsed(i))
			{
				if (GetDamage(client, i) < GetMaxDamage(client))
					BestDemoSpots.Push(i);
				else
					GoodDemoSpots.Push(i);
			}
			else
				AllDemoSpots.Push(i);
		}

		if (BestDemoSpots.Length)
			index = BestDemoSpots.Get(crandomint(0, BestDemoSpots.Length - 1));
		else if (GoodDemoSpots.Length)
			index = GoodDemoSpots.Get(crandomint(0, GoodDemoSpots.Length - 1));
		else if (AllDemoSpots.Length)
			index = AllDemoSpots.Get(crandomint(0, AllDemoSpots.Length - 1));

		delete BestDemoSpots;
		delete GoodDemoSpots;
		delete AllDemoSpots;

		if (index != -1)
		{
			m_goalIndex[client] = index;
			if (m_paths[index].radius > 0.0)
			{
				m_goalPosition[client][0] = m_paths[index].origin[0] + crandomfloat(-m_paths[index].radius, m_paths[index].radius);
				m_goalPosition[client][1] = m_paths[index].origin[1] + crandomfloat(-m_paths[index].radius, m_paths[index].radius);
				m_goalPosition[client][2] = m_paths[index].origin[2];
			}
			else
				m_goalPosition[client] = m_paths[index].origin;
			
			return;
		}
	}

	if (isDM || GetConVarBool(EBotForceHuntEnemy))
	{
		index = FindNearestEnemy(client);
		if (IsValidClient(index))
		{
			m_goalEntity[client] = index;
			m_goalPosition[client] = GetOrigin(m_goalEntity[client]);
		}
		else if (m_hasWaypoints || m_hasNavpoints)
		{
			m_goalIndex[client] = crandomint(1, m_waypointNumber - 1);
			if (m_paths[m_goalIndex[client]].radius > 0.0)
			{
				m_goalPosition[client][0] = m_paths[m_goalIndex[client]].origin[0] + crandomfloat(-m_paths[m_goalIndex[client]].radius, m_paths[m_goalIndex[client]].radius);
				m_goalPosition[client][1] = m_paths[m_goalIndex[client]].origin[1] + crandomfloat(-m_paths[m_goalIndex[client]].radius, m_paths[m_goalIndex[client]].radius);
				m_goalPosition[client][2] = m_paths[m_goalIndex[client]].origin[2];
			}
			else
				m_goalPosition[client] = m_paths[m_goalIndex[client]].origin;
		}

		return;
	}

	if (isKOTH || isArena)
	{
		if (!IsValidEntity(m_capturePoint) || !HasEntProp(m_capturePoint, Prop_Send, "m_iTeamNum"))
		{
			index = -1;
			while (IsValidEntity((index = FindEntityByClassname(index, "team_control_point"))))
			{
				m_capturePoint = index;
				break;
			}
		}

		if (m_hasCaptureWaypoints && (m_hasWaypoints || m_hasNavpoints))
		{
			ArrayList BestDefendPoints = new ArrayList();
			if (!BestDefendPoints)
				return;

			ArrayList GoodDefendPoints = new ArrayList();
			if (!GoodDefendPoints)
				return;

			ArrayList AllDefendPoints = new ArrayList();
			if (!AllDefendPoints)
				return;

			index = -1;
			for (i = 0; i < m_waypointNumber; i++)
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

				if (m_paths[i].radius > 32.0 || !IsWaypointUsed(i))
				{
					if (GetDamage(client, i) < GetMaxDamage(client))
						BestDefendPoints.Push(i);
					else
						GoodDefendPoints.Push(i);
				}
				else
					AllDefendPoints.Push(i);
			}

			if (BestDefendPoints.Length)
				index = BestDefendPoints.Get(crandomint(0, BestDefendPoints.Length - 1));
			else if (GoodDefendPoints.Length)
				index = GoodDefendPoints.Get(crandomint(0, GoodDefendPoints.Length - 1));
			else if (AllDefendPoints.Length)
				index = AllDefendPoints.Get(crandomint(0, AllDefendPoints.Length - 1));
			
			delete BestDefendPoints;
			delete GoodDefendPoints;
			delete AllDefendPoints;
			
			if (index != -1)
			{
				m_goalIndex[client] = index;
				if (m_paths[index].radius > 0.0)
				{
					m_goalPosition[client][0] = m_paths[index].origin[0] + crandomfloat(-m_paths[index].radius, m_paths[index].radius);
					m_goalPosition[client][1] = m_paths[index].origin[1] + crandomfloat(-m_paths[index].radius, m_paths[index].radius);
					m_goalPosition[client][2] = m_paths[index].origin[2];
				}
				else
					m_goalPosition[client] = m_paths[index].origin;
			}
			else
			{
				index = FindNearestEnemy(client);
				if (IsValidClient(index))
				{
					m_goalEntity[client] = index;
					m_goalPosition[client] = GetOrigin(m_goalEntity[client]);
					SetGoalIndex(client, FindNearestWaypoint(m_goalPosition[client], 999999.0, client));
				}
				else
					SetGoalIndex(client, crandomint(1, m_waypointNumber - 1));
			}
		}
		else if (IsValidEntity(m_capturePoint))
			m_goalEntity[client] = m_capturePoint;
		else
		{
			index = FindNearestEnemy(client);
			if (IsValidClient(index))
			{
				m_goalEntity[client] = index;
				m_goalPosition[client] = GetOrigin(m_goalEntity[client]);
			}
		}
	}
	else if (isPL)
	{
		if (!IsValidEntity(m_bluPayload))
			FindPayload();

		if (m_hasWaypoints || m_hasNavpoints)
		{
			if (m_team[client] == 2)
			{
				ArrayList BestDefendPoints = new ArrayList();
				if (!BestDefendPoints)
					return;

				ArrayList GoodDefendPoints = new ArrayList();
				if (!GoodDefendPoints)
					return;

				ArrayList AllDefendPoints = new ArrayList();
				if (!AllDefendPoints)
					return;

				index = -1;
				for (i = 0; i < m_waypointNumber; i++)
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

					if (m_paths[i].radius > 32.0 || !IsWaypointUsed(i))
					{
						if (GetDamage(client, i) < GetMaxDamage(client))
							BestDefendPoints.Push(i);
						else
							GoodDefendPoints.Push(i);
					}
					else
						AllDefendPoints.Push(i);
				}

				if (BestDefendPoints.Length)
					index = BestDefendPoints.Get(crandomint(0, BestDefendPoints.Length - 1));
				else if (GoodDefendPoints.Length)
					index = GoodDefendPoints.Get(crandomint(0, GoodDefendPoints.Length - 1));
				else if (AllDefendPoints.Length)
					index = AllDefendPoints.Get(crandomint(0, AllDefendPoints.Length - 1));
				
				delete BestDefendPoints;
				delete GoodDefendPoints;
				delete AllDefendPoints;
			
				if (index != -1)
				{
					m_goalIndex[client] = index;
					if (m_paths[index].radius > 0.0)
					{
						m_goalPosition[client][0] = m_paths[index].origin[0] + crandomfloat(-m_paths[index].radius, m_paths[index].radius);
						m_goalPosition[client][1] = m_paths[index].origin[1] + crandomfloat(-m_paths[index].radius, m_paths[index].radius);
						m_goalPosition[client][2] = m_paths[index].origin[2];
					}
					else
						m_goalPosition[client] = m_paths[index].origin;
					
					if (IsValidEntity(m_bluPayload))
						m_goalEntity[client] = m_bluPayload;
				}
				else if (IsValidEntity(m_bluPayload))
				{
					m_goalEntity[client] = m_bluPayload;
					m_goalPosition[client] = GetOrigin(m_bluPayload);
					m_goalPosition[client][0] += crandomfloat(-768.0, 768.0);
					m_goalPosition[client][1] += crandomfloat(-768.0, 768.0);
					m_goalPosition[client][2] += crandomfloat(0.0, 512.0);
					SetGoalIndex(client, FindNearestWaypoint(m_goalPosition[client], 999999.0, client));
				}
			}
			else if (IsValidEntity(m_bluPayload))
			{
				m_goalEntity[client] = m_bluPayload;
				m_goalPosition[client] = GetOrigin(m_bluPayload);
				m_goalPosition[client][0] += crandomfloat(-64.0, 64.0);
				m_goalPosition[client][1] += crandomfloat(-64.0, 64.0);
				m_goalPosition[client][2] += crandomfloat(-64.0, 64.0);
				SetGoalIndex(client, FindNearestWaypoint(m_goalPosition[client], 999999.0, client));
			}
		}
		else if (IsValidEntity(m_bluPayload))
		{
			m_goalEntity[client] = m_bluPayload;
			if (m_team[client] == 2)
			{
				m_goalPosition[client] = GetOrigin(m_bluPayload);
				m_goalPosition[client][0] += crandomfloat(-768.0, 768.0);
				m_goalPosition[client][1] += crandomfloat(-768.0, 768.0);
				m_goalPosition[client][2] += crandomfloat(0.0, 512.0);
			}
			else
			{
				m_goalPosition[client] = GetOrigin(m_bluPayload);
				m_goalPosition[client][0] += crandomfloat(-64.0, 64.0);
				m_goalPosition[client][1] += crandomfloat(-64.0, 64.0);
				m_goalPosition[client][2] += crandomfloat(-64.0, 64.0);
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
			m_goalPosition[client][0] += crandomfloat(-64.0, 64.0);
			m_goalPosition[client][1] += crandomfloat(-64.0, 64.0);
			m_goalPosition[client][2] += crandomfloat(-64.0, 64.0);
			SetGoalIndex(client, FindNearestWaypoint(m_goalPosition[client], 999999.0, client));
			m_goalEntity[client] = m_redPayload;
		}
		else if (IsValidEntity(m_bluPayload))
		{
			m_goalPosition[client] = GetOrigin(m_bluPayload);
			m_goalPosition[client][0] += crandomfloat(-64.0, 64.0);
			m_goalPosition[client][1] += crandomfloat(-64.0, 64.0);
			m_goalPosition[client][2] += crandomfloat(-64.0, 64.0);
			SetGoalIndex(client, FindNearestWaypoint(m_goalPosition[client], 999999.0, client));
			m_goalEntity[client] = m_bluPayload;
		}
	}
	else if (isCTF)
	{
		// flag required for check status
		while ((index = FindEntityByClassname(index, "item_teamflag")) != INVALID_ENT_REFERENCE)
		{
			if (IsValidEntity(index) && HasEntProp(index, Prop_Send, "m_nFlagStatus") && m_team[client] != GetTeamNumber(index))
				m_goalEntity[client] = index;
		}

		m_goalIndex[client] = crandomint(1, m_waypointNumber - 1);
	}
	else if (isMVM || isAD)
	{
		if (m_hasWaypoints || m_hasNavpoints)
		{
			ArrayList BestControlPoints = new ArrayList();
			if (!BestControlPoints)
				return;

			ArrayList ControlPoints = new ArrayList();
			if (!ControlPoints)
				return;

			ArrayList BestDefendPoints = new ArrayList();
			if (!BestDefendPoints)
				return;

			ArrayList GoodDefendPoints = new ArrayList();
			if (!GoodDefendPoints)
				return;

			ArrayList AllDefendPoints = new ArrayList();
			if (!AllDefendPoints)
				return;

			index = -1;
			for (i = 0; i < m_waypointNumber; i++)
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
				{
					if (GetDamage(client, i) < GetMaxDamage(client))
						BestControlPoints.Push(i);
					else
						ControlPoints.Push(i);
				}

				if (!(m_paths[i].flags & WAYPOINT_DEFEND))
					continue;

				if (m_paths[i].radius > 32.0 || !IsWaypointUsed(i))
				{
					if (GetDamage(client, i) < GetMaxDamage(client))
						BestDefendPoints.Push(i);
					else
						GoodDefendPoints.Push(i);
				}
				else
					AllDefendPoints.Push(i);
			}

			if (BestControlPoints.Length)
				index = BestControlPoints.Get(crandomint(0, BestControlPoints.Length - 1));
			else if (ControlPoints.Length)
				index = ControlPoints.Get(crandomint(0, ControlPoints.Length - 1));
			else if (BestDefendPoints.Length)
				index = BestDefendPoints.Get(crandomint(0, BestDefendPoints.Length - 1));
			else if (GoodDefendPoints.Length)
				index = GoodDefendPoints.Get(crandomint(0, GoodDefendPoints.Length - 1));
			else if (AllDefendPoints.Length)
				index = AllDefendPoints.Get(crandomint(0, AllDefendPoints.Length - 1));
			
			delete BestControlPoints;
			delete ControlPoints;
			delete BestDefendPoints;
			delete GoodDefendPoints;
			delete AllDefendPoints;
			
			if (index != -1)
			{
				m_goalIndex[client] = index;
				if (m_paths[index].radius > 0.0)
				{
					m_goalPosition[client][0] = m_paths[index].origin[0] + crandomfloat(-m_paths[index].radius, m_paths[index].radius);
					m_goalPosition[client][1] = m_paths[index].origin[1] + crandomfloat(-m_paths[index].radius, m_paths[index].radius);
					m_goalPosition[client][2] = m_paths[index].origin[2];
				}
				else
					m_goalPosition[client] = m_paths[index].origin;
			}
			else
			{
				m_goalIndex[client] = crandomint(1, m_waypointNumber - 1);
				if (m_paths[m_goalIndex[client]].radius > 0.0)
				{
					m_goalPosition[client][0] = m_paths[m_goalIndex[client]].origin[0] + crandomfloat(-m_paths[m_goalIndex[client]].radius, m_paths[m_goalIndex[client]].radius);
					m_goalPosition[client][1] = m_paths[m_goalIndex[client]].origin[1] + crandomfloat(-m_paths[m_goalIndex[client]].radius, m_paths[m_goalIndex[client]].radius);
					m_goalPosition[client][2] = m_paths[m_goalIndex[client]].origin[2];
				}
				else
					m_goalPosition[client] = m_paths[m_goalIndex[client]].origin;
			}
		}
		else
		{
			index = FindNearestEnemy(client);
			if (IsValidClient(index))
			{
				m_goalEntity[client] = index;
				if (isAD)
					m_goalPosition[client] = GetOrigin(m_goalEntity[client]);
			}
		}
	}
	else if (isCP)
	{
		if (IsValidEntity(m_goalEntity[client]) && m_team[client] != GetTeamNumber(m_goalEntity[client]))
		{
			m_goalIndex[client] = FindNearestWaypoint(GetOrigin(m_goalEntity[client]));
			if (m_paths[m_goalIndex[client]].radius > 0.0)
			{
				m_goalPosition[client][0] = m_paths[m_goalIndex[client]].origin[0] + crandomfloat(-m_paths[m_goalIndex[client]].radius, m_paths[m_goalIndex[client]].radius);
				m_goalPosition[client][1] = m_paths[m_goalIndex[client]].origin[1] + crandomfloat(-m_paths[m_goalIndex[client]].radius, m_paths[m_goalIndex[client]].radius);
				m_goalPosition[client][2] = m_paths[m_goalIndex[client]].origin[2];
			}
			else
				m_goalPosition[client] = m_paths[m_goalIndex[client]].origin;
		}
		else
			m_goalEntity[client] = GetNearestEnemyControlPoint(client);
				
	}
	else if (m_hasWaypoints || m_hasNavpoints)
	{
		m_goalIndex[client] = crandomint(1, m_waypointNumber - 1);
		if (m_paths[m_goalIndex[client]].radius > 0.0)
		{
			m_goalPosition[client][0] = m_paths[m_goalIndex[client]].origin[0] + crandomfloat(-m_paths[m_goalIndex[client]].radius, m_paths[m_goalIndex[client]].radius);
			m_goalPosition[client][1] = m_paths[m_goalIndex[client]].origin[1] + crandomfloat(-m_paths[m_goalIndex[client]].radius, m_paths[m_goalIndex[client]].radius);
			m_goalPosition[client][2] = m_paths[m_goalIndex[client]].origin[2];
		}
		else
			m_goalPosition[client] = m_paths[m_goalIndex[client]].origin;
	}
	else
	{
		index = FindNearestEnemy(client);
		if (IsValidClient(index))
			m_goalEntity[client] = index;
	}
}

stock void ObjectiveMovement(const int client)
{
	if (GetConVarBool(EBotForceHuntEnemy))
	{
		if (m_hasWaypoints || m_hasNavpoints)
			FollowPath(client, m_goalPosition[client], -1, m_goalIndex[client]);
		else
		{
			m_goalPosition[client] = GetOrigin(m_goalEntity[client]);
			FollowPath(client, m_goalPosition[client]);
		}

		return;
	}

	if ((m_hasWaypoints || m_hasNavpoints) && m_class[client] == TFClass_DemoMan && m_hasStickyWaypoints)
	{
		if (m_goalIndex[client] != -1 && (!m_paths[m_goalIndex[client]].activeArea || m_paths[m_goalIndex[client]].activeArea & currentActiveArea))
		{
			if (GetVectorDistance(GetOrigin(client), m_goalPosition[client], true) < Squaredf(4.0))
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
	else if ((m_hasWaypoints || m_hasNavpoints) && IsSniper(client) && m_hasSniperWaypoints)
	{
		if (m_goalIndex[client] != -1 && (!m_paths[m_goalIndex[client]].activeArea || m_paths[m_goalIndex[client]].activeArea & currentActiveArea))
		{
			if (GetVectorDistance(GetOrigin(client), m_goalPosition[client], true) < Squaredf(4.0))
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
	else if (isKOTH || isArena)
	{
		if (m_hasWaypoints || m_hasNavpoints)
			FollowPath(client, m_goalPosition[client]);
		else if (IsValidEntity(m_capturePoint) && HasEntProp(m_capturePoint, Prop_Send, "m_iTeamNum"))
		{
			float cappointpos[3];
			cappointpos = GetOrigin(m_capturePoint);
			if (GetTeamNumber(m_capturePoint) == m_team[client])
			{
				m_goalPosition[client][0] = cappointpos[0] + crandomfloat(-768.0, 768.0);
				m_goalPosition[client][1] = cappointpos[1] + crandomfloat(-768.0, 768.0);
				m_goalPosition[client][2] = cappointpos[2] + crandomfloat(-128.0, 128.0);
			}
			else
			{
				m_goalPosition[client][0] = cappointpos[0] + crandomfloat(-192.0, 192.0);
				m_goalPosition[client][1] = cappointpos[1] + crandomfloat(-192.0, 192.0);
				m_goalPosition[client][2] = cappointpos[2] + 18.0;
			}
			
			FollowPath(client, m_goalPosition[client]);
		}
		else
			FollowPath(client, m_goalPosition[client]);
	}
	else if (isPL)
	{
		if (m_hasWaypoints || m_hasNavpoints)
			FollowPath(client, m_goalPosition[client], -1, m_goalIndex[client]);
		else
			FollowPath(client, m_goalPosition[client]);
	}
	else if (isPLR)
	{
		if (m_hasWaypoints || m_hasNavpoints)
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
			int flag, flag2, enemy;
			while ((flag = FindEntityByClassname(flag, "item_teamflag")) != INVALID_ENT_REFERENCE)
			{
				if (IsValidEntity(flag) && m_team[client] != GetTeamNumber(flag))
				{
					if (GetEntProp(flag, Prop_Send, "m_nFlagStatus") == 1)
					{
						if (!TF2_HasTheFlag(client))
						{
							while ((flag2 = FindEntityByClassname(flag2, "item_teamflag")) != INVALID_ENT_REFERENCE)
							{
								if (IsValidEntity(flag2) && m_team[client] == GetTeamNumber(flag2))
								{
									if (GetEntProp(flag2, Prop_Send, "m_nFlagStatus") == 1) // did someone is picked our flag? chase that guy.
										FollowPath(client, GetOrigin(flag2));
									else // go to nearest enemy
									{
										enemy = FindNearestEnemy(client);
										if (IsValidClient(enemy))
											FollowPath(client, GetOrigin(enemy));
										else if (GetVectorDistance(GetOrigin(client), GetOrigin(flag), true) > Squared(300)) // cover our flag
											FollowPath(client, GetOrigin(flag));
									}
								}
							}
						}
						else
						{	
							if (m_team[client] == 2)
								m_goalPosition[client] = m_redFlagCapPoint;
							else if (m_team[client] == 3)
								m_goalPosition[client] = m_bluFlagCapPoint;
						}
					}
					else
					{
						if (m_team[client] == 2)
							m_goalPosition[client] = m_bluFlagCapPoint;
						else if (m_team[client] == 3)
							m_goalPosition[client] = m_redFlagCapPoint;
							
						FollowPath(client, GetOrigin(flag));
					}
				}
			}
		}
	}
	else if (isAD || isCP)
		FollowPath(client, m_goalPosition[client], -1, m_goalIndex[client]);
	else if (m_hasWaypoints || m_hasNavpoints)
	{
		FollowPath(client, m_goalPosition[client], -1, m_goalIndex[client]);
		if (m_isSlowThink[client])
			SelectObjective(client);
	}
	else if (IsValidEntity(m_goalEntity[client]))
	{
		m_goalPosition[client] = GetOrigin(m_goalEntity[client]);
		FollowPath(client, m_goalPosition[client]);
	}
}

stock void NavMeshControlPoint(const int client)
{
	if (!m_isSlowThink[client])
		return;
	
	m_goalEntity[client] = GetNearestEnemyControlPoint(client);
	m_goalPosition[client] = GetOrigin(m_goalEntity[client]);
}