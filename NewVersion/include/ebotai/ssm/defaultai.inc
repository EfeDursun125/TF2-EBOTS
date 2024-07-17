public bool DefaultRequirements(const int client)
{
	return true;
}

public void DefaultStart(const int client)
{
    switch (m_class[client])
    {
        case TFClass_Engineer:
        {
            if (!GameRules_GetProp("m_bPlayingMedieval"))
            {
                RememberedProcess[client] = PRO_ENGINEERIDLE;
                CurrentProcess[client] = PRO_ENGINEERIDLE;
                EngineerIdleStart(client);
                return;
            }
        }
        case TFClass_Spy:
        {
            RememberedProcess[client] = PRO_SPYLURK;
            CurrentProcess[client] = PRO_SPYLURK;
            SpyLurkStart(client);
            return;
        }
    }

    m_enemiesNearCount[client] = 0;
	m_friendsNearCount[client] = 0;
	m_hasEnemiesNear[client] = false;
	m_hasFriendsNear[client] = false;
	DeletePathNodes(client);
}

public void DefaultUpdate(const int client)
{
    switch (m_class[client])
    {
        case TFClass_Medic:
        {
            if (m_hasEnemiesNear[client] && m_hasFriendsNear[client] && m_friendDistance[client] < m_enemyDistance[client])
            {
				if (SetProcess(client, PRO_HEAL, (m_difficulty[client] + 8.0), "", true))
				    return;
			}
			else if (!m_hasEnemiesNear[client] && m_hasFriendsNear[client])
			{
				if (SetProcess(client, PRO_HEAL, (m_difficulty[client] + 8.0), "", true))
				    return;
			}
        }
        case TFClass_Spy:
        {
            RememberedProcess[client] = PRO_SPYLURK;
            CurrentProcess[client] = PRO_SPYLURK;
            SpyLurkUpdate(client);
            return;
        }
        case TFClass_Engineer:
        {
            RememberedProcess[client] = PRO_ENGINEERIDLE;
            CurrentProcess[client] = PRO_ENGINEERIDLE;
            EngineerIdleUpdate(client);
            return;
        }
    }

    if (m_isSlowThink[client])
    {
        SelectObjective(client);
		FindFriendsAndEnemiens(client);
		FindEnemyEntities(client);
	}
	else
	{
		LookUpdate(client);

		// follow our friend if we cannot find a path...
		if (m_hasFriendsNear[client] && IsValidClient(m_nearestFriend[client]) && GetVectorDistance(GetOrigin(client), GetOrigin(m_nearestFriend[client]), true) < Squaredf(200.0))
			MoveTo(client, GetOrigin(m_nearestFriend[client]));

		// be smart
		if (m_hasEnemiesNear[client] && IsValidClient(m_nearestEnemy[client]) && (!m_hasFriendsNear[client] || TF2_IsPlayerInCondition(m_nearestEnemy[client], TFCond_Zoomed)))
		{
			float duckduck[3], height[3];
			duckduck = GetOrigin(client);
			height = duckduck;
			duckduck[2] += 36.0;
			height[2] += 72.0;
			if (!IsVisible(duckduck, GetEyePosition(m_nearestEnemy[client])) && IsVisible(height, GetEyePosition(m_nearestEnemy[client])))
				m_duckTimer[client] = GetGameTime() + 1.25;
			else
			    m_duckTimer[client] = 0.0;
		}
	}

	CheckSlowThink(client);
}

public void DefaultEnd(const int client)
{

}

public void SelectObjective(const int client)
{
	int i;
	int index;
	if ((m_hasWaypoints || m_hasNavpoints) && m_hasSniperWaypoints && IsSniper(client) && !TF2_HasTheFlag(client) && !isOT)
	{
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

		if (BestSniperPoints.Length > 0)
			index = BestSniperPoints.Get(crandomint(0, BestSniperPoints.Length - 1));
		else if (GoodSniperPoints.Length > 0)
			index = GoodSniperPoints.Get(crandomint(0, GoodSniperPoints.Length - 1));
		else if (AllSniperPoints.Length > 0)
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

			FindPath(client, m_goalPosition[client], -1, m_goalIndex[client]);
	        if (SetProcess(client, PRO_FOLLOWPATH, 90.0, "", true, true))
				return;
		}
	}

	if ((m_hasWaypoints || m_hasNavpoints) && m_hasStickyWaypoints && m_class[client] == TFClass_DemoMan && !TF2_HasTheFlag(client) && !isOT)
	{
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

		if (BestDemoSpots.Length > 0)
			index = BestDemoSpots.Get(crandomint(0, BestDemoSpots.Length - 1));
		else if (GoodDemoSpots.Length > 0)
			index = GoodDemoSpots.Get(crandomint(0, GoodDemoSpots.Length - 1));
		else if (AllDemoSpots.Length > 0)
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

			FindPath(client, m_goalPosition[client], -1, m_goalIndex[client]);
	        if (SetProcess(client, PRO_FOLLOWPATH, 90.0, "", true, true))
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

		FindPath(client, m_goalPosition[client], m_goalEntity[client], m_goalIndex[client]);
		if (SetProcess(client, PRO_FOLLOWPATH, 90.0, "", true, true))
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

		if (m_hasCaptureWaypoints && (m_hasWaypoints || m_hasNavpoints) && !isOT)
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

			if (BestDefendPoints.Length > 0)
				index = BestDefendPoints.Get(crandomint(0, BestDefendPoints.Length - 1));
			else if (GoodDefendPoints.Length > 0)
				index = GoodDefendPoints.Get(crandomint(0, GoodDefendPoints.Length - 1));
			else if (AllDefendPoints.Length > 0)
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

				FindPath(client, m_goalPosition[client], m_goalEntity[client], m_goalIndex[client]);
				if (SetProcess(client, PRO_FOLLOWPATH, 90.0, "", true, true))
					return;
			}
		}
		else if (IsValidEntity(m_capturePoint))
		{
			m_goalEntity[client] = m_capturePoint;
			FindPath(client, GetOrigin(m_goalEntity[client]), m_goalEntity[client], -1);
			if (SetProcess(client, PRO_FOLLOWPATH, 90.0, "", true, true))
				return;
		}
	}
	else if (isPL)
	{
		if (!IsValidEntity(m_bluPayload))
		{
			FindPayload();
			return;
		}

		if (m_hasWaypoints || m_hasNavpoints)
		{
			if (m_team[client] == 2 && !isOT)
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
					if (!(m_paths[i].flags & WAYPOINT_DEFEND))
						continue;

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

				if (BestDefendPoints.Length > 0)
					index = BestDefendPoints.Get(crandomint(0, BestDefendPoints.Length - 1));
				else if (GoodDefendPoints.Length > 0)
					index = GoodDefendPoints.Get(crandomint(0, GoodDefendPoints.Length - 1));
				else if (AllDefendPoints.Length > 0)
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

					FindPath(client, m_goalPosition[client], m_goalEntity[client], m_goalIndex[client]);
					if (SetProcess(client, PRO_FOLLOWPATH, 90.0, "", true, true))
						return;
				}
				else if (IsValidEntity(m_bluPayload))
				{
					m_goalEntity[client] = m_bluPayload;
					m_goalPosition[client] = GetOrigin(m_bluPayload);
					m_goalPosition[client][0] += crandomfloat(-768.0, 768.0);
					m_goalPosition[client][1] += crandomfloat(-768.0, 768.0);
					m_goalPosition[client][2] += crandomfloat(-256.0, 512.0);
					FindPath(client, m_goalPosition[client], m_goalEntity[client], m_goalIndex[client]);
					if (SetProcess(client, PRO_FOLLOWPATH, 90.0, "", true, true))
						return;
				}
			}
			else if (IsValidEntity(m_bluPayload))
			{
				m_goalEntity[client] = m_bluPayload;
				m_goalPosition[client] = GetOrigin(m_bluPayload);
				FindPath(client, m_goalPosition[client], m_goalEntity[client], m_goalIndex[client]);
				if (SetProcess(client, PRO_FOLLOWPATH, 90.0, "", true, true))
					return;
			}
		}
		else if (IsValidEntity(m_bluPayload))
		{
			m_goalEntity[client] = m_bluPayload;
			if (m_team[client] == 2 && !isOT)
			{
				m_goalPosition[client] = GetOrigin(m_bluPayload);
				m_goalPosition[client][0] += crandomfloat(-768.0, 768.0);
				m_goalPosition[client][1] += crandomfloat(-768.0, 768.0);
				m_goalPosition[client][2] += crandomfloat(-256.0, 512.0);
			}
			else
				m_goalPosition[client] = GetOrigin(m_bluPayload);

			FindPath(client, m_goalPosition[client], m_goalEntity[client], m_goalIndex[client]);
			if (SetProcess(client, PRO_FOLLOWPATH, 90.0, "", true, true))
				return;
		}
	}
	else if (isPLR)
	{
		if (!IsValidEntity(m_bluPayload) || !IsValidEntity(m_redPayload))
		{
			FindPayload();
			return;
		}

		if (m_team[client] == 2 && IsValidEntity(m_redPayload))
		{
			m_goalPosition[client] = GetOrigin(m_redPayload);
			m_goalPosition[client][0] += crandomfloat(-32.0, 32.0);
			m_goalPosition[client][1] += crandomfloat(-32.0, 32.0);
			m_goalPosition[client][2] += crandomfloat(-32.0, 32.0);
			m_goalIndex[client] = FindWaypointFast(m_goalPosition[client], client);
			m_goalEntity[client] = m_redPayload;
		}
		else if (IsValidEntity(m_bluPayload))
		{
			m_goalPosition[client] = GetOrigin(m_bluPayload);
			m_goalPosition[client][0] += crandomfloat(-32.0, 32.0);
			m_goalPosition[client][1] += crandomfloat(-32.0, 32.0);
			m_goalPosition[client][2] += crandomfloat(-32.0, 32.0);
			m_goalIndex[client] = FindWaypointFast(m_goalPosition[client], client);
			m_goalEntity[client] = m_bluPayload;
		}

		if (m_goalIndex[client] > 0)
		{
			FindPath(client, m_goalPosition[client], m_goalEntity[client], m_goalIndex[client]);
			if (SetProcess(client, PRO_FOLLOWPATH, 90.0, "", true, true))
				return;
		}
	}
	else if (isCTF)
	{
	    if (TF2_HasTheFlag(client))
		{
			if (m_team[client] == 2)
				m_goalPosition[client] = m_redFlagCapPoint;
			else
				m_goalPosition[client] = m_bluFlagCapPoint;

			FindPath(client, m_goalPosition[client], -1, -1);
			if (SetProcess(client, PRO_FOLLOWPATH, 90.0, "", true, true))
				return;
		}
		else
		{
			if (!IsValidEntity(m_goalEntity[client]) || !HasEntProp(m_goalEntity[client], Prop_Send, "m_nFlagStatus"))
				m_goalEntity[client] = GetEnemyFlag(client);
			else
			{
				if (GetEntProp(m_goalEntity[client], Prop_Send, "m_nFlagStatus") == 1) // someone got the flag
				{
					index = GetOurFlag(client);
					if (GetEntProp(index, Prop_Send, "m_nFlagStatus") == 1) // did someone is just picked our flag? chase him!
					{
						FindPath(client, GetOrigin(index), index, -1);
						if (SetProcess(client, PRO_FOLLOWPATH, 90.0, "", true, true))
							return;
					}

					// or go to nearest enemy
					i = FindNearestEnemy(client);
					if (IsValidClient(i))
					{
						FindPath(client, GetOrigin(i), i, -1);
						if (SetProcess(client, PRO_FOLLOWPATH, 90.0, "", true, true))
							return;
					}
					else if (GetVectorDistance(GetOrigin(client), GetOrigin(index), true) > Squaredf(300.0)) // or... cover our flag
					{
						FindPath(client, GetOrigin(index), index, -1);
						if (SetProcess(client, PRO_FOLLOWPATH, 90.0, "", true, true))
							return;
					}
				}
				else
				{
					if (m_team[client] == 2)
						m_goalPosition[client] = m_bluFlagCapPoint;
					else
						m_goalPosition[client] = m_redFlagCapPoint;

					FindPath(client, m_goalPosition[client], m_goalEntity[client], -1);
					if (SetProcess(client, PRO_FOLLOWPATH, 90.0, "", true, true))
						return;
				}
			}
		}
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

			if (BestControlPoints.Length > 0)
				index = BestControlPoints.Get(crandomint(0, BestControlPoints.Length - 1));
			else if (ControlPoints.Length> 0)
				index = ControlPoints.Get(crandomint(0, ControlPoints.Length - 1));
			else if (BestDefendPoints.Length > 0)
				index = BestDefendPoints.Get(crandomint(0, BestDefendPoints.Length - 1));
			else if (GoodDefendPoints.Length > 0)
				index = GoodDefendPoints.Get(crandomint(0, GoodDefendPoints.Length - 1));
			else if (AllDefendPoints.Length > 0)
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

				FindPath(client, m_goalPosition[client], -1, m_goalIndex[client]);
				if (SetProcess(client, PRO_FOLLOWPATH, 90.0, "", true, true))
					return;
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

			FindPath(client, m_goalPosition[client], -1, m_goalIndex[client]);
			if (SetProcess(client, PRO_FOLLOWPATH, 90.0, "", true, true))
				return;
		}
		else
		{
			m_goalEntity[client] = GetNearestEnemyControlPoint(client);
			FindPath(client, GetOrigin(m_goalEntity[client]), m_goalEntity[client], -1);
			if (SetProcess(client, PRO_FOLLOWPATH, 90.0, "", true, true))
				return;
		}
	}

    if (!isDM || crandomint(1, 2) == 1)
    {
		index = FindNearestEnemy(client);
		if (IsValidClient(index))
		{
			m_goalEntity[client] = index;
			FindPath(client, GetOrigin(m_goalEntity[client]), m_goalEntity[client], -1);
			if (SetProcess(client, PRO_FOLLOWPATH, 90.0, "", true, true))
				return;
		}
	}

	if (m_hasWaypoints || m_hasNavpoints)
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

		FindPath(client, m_goalPosition[client], -1, m_goalIndex[client]);
		if (SetProcess(client, PRO_FOLLOWPATH, 90.0, "", true, true))
			return;
	}
}

stock void NavMeshControlPoint(const int client)
{
	if (!m_isSlowThink[client])
		return;

	m_goalEntity[client] = GetNearestEnemyControlPoint(client);
	m_goalPosition[client] = GetOrigin(m_goalEntity[client]);
}
