float sentrybuildarea[TFMaxPlayers][3];

public bool BuildSentryRequirements(const int client)
{
	if (GetMetal(client) < 130.0)
		return false;
	
	if (IsValidEntity(SentryGun[client]))
		return false;
	
	if (!m_hasWaypoints)
	{
		PickSentrySpot(client, sentrybuildarea[client]);
		if (!IsPathPossible(client, GetOrigin(client), sentrybuildarea[client]))
			return false;
	}
	
	return true;
}

public void BuildSentryStart(const int client)
{
	DeletePathNodes(client); // NEVER EVER DELETE THIS LINE
	if (m_hasWaypoints || m_hasNavpoints)
		PickSentrySpot(client, sentrybuildarea[client]);
}

public void BuildSentryUpdate(const int client)
{
	CheckHealth(client);
	FindFriendsAndEnemiens(client);
	FindEnemyEntities(client);
	
	float range = GetVectorDistance(GetOrigin(client), sentrybuildarea[client], true);
	if (range < Squaredf(300.0))
	{
		m_ignoreEnemies[client] = GetGameTime() + 1.0;
		if (!IsWeaponSlotActive(client, 5) && !IsMoving(client))
			FakeClientCommandThrottled(client, "build 2 0");

		if (m_goalIndex[client] != -1 && (m_hasWaypoints || m_hasNavpoints))
		{
			m_lookAt[client] = m_paths[m_goalIndex[client]].campStart;
			LookAtPosition(client, m_lookAt[client], GetEyeAngles(client));
		}
		else
			LookUpdate(client);
		
		if (IsWeaponSlotActive(client, 5) && !IsMoving(client) && range < Squaredf(128.0) && IsClientAimingToPosition(client, m_lookAt[client]))
		{
			m_buttons[client] |= IN_ATTACK;
		
			int objBeingBuilt = GetEntPropEnt(GetActiveWeapon(client), Prop_Send, "m_hObjectBeingBuilt");
			if (!IsValidEntity(objBeingBuilt))
				return;
		
			bool placementOK = !!GetEntData(objBeingBuilt, FindSendPropInfo("CObjectSentrygun", "m_iKills") - 4);
			if (!placementOK && m_isSlowThink[client] && crandomint(1, 5) == 1)
			{
				PickSentrySpot(client, sentrybuildarea[client]);
				EquipWeaponSlot(client, 0);
			}

			return;
		}
	}
	else
		LookUpdate(client);

	if (range > Squaredf(70.0))
		FollowPath(client, sentrybuildarea[client]);
	else
		MoveToWaypoint(client, sentrybuildarea[client]);
	
	if (IsValidEntity(TF2_GetObject(client, TFObject_Sentry, TFObjectMode_None)))
	{
		EngineerMoveSentryTimer[client] = GetGameTime() + GetRandomFloat(90.0, 160.0);
		WantsBuildSentryGun[client] = false;
		WantsMoveSentryGun[client] = false;
		FinishCurrentProcess(client);
		FakeClientCommand(client, "tournament_player_readystate 1");
	}
}

public void BuildSentryEnd(const int client)
{
	EquipWeaponSlot(client, 0);
}

public void PickSentrySpot(const int client, float area[3])
{
	static int i;
	if ((m_hasWaypoints || m_hasNavpoints) && m_hasSentryWaypoints)
	{
		ArrayList BestSentrySpots = new ArrayList();
		ArrayList GoodSentrySpots = new ArrayList();
		ArrayList AllSentrySpots = new ArrayList();
		for (i = 0; i < m_waypointNumber; i++)
		{
			if (!(m_paths[i].flags & WAYPOINT_SENTRY))
				continue;
			
			// blocked waypoint
   			if (m_paths[i].activeArea != 0 && !(m_paths[i].activeArea & currentActiveArea))
				continue;
			
			if (m_lastFailedWaypoint[client] == i)
				continue;
			
    		// not for our team
 			if (m_team[client] == 3 && m_paths[i].team == 2)
    			continue;

   			if (m_team[client] == 2 && m_paths[i].team == 3)
        		continue;

			if (m_paths[i].radius > 32.0 || !IsWaypointUsed(i))
			{
				if (GetDamage(client, i) < GetMaxDamage(client))
					BestSentrySpots.Push(i);
				else
					GoodSentrySpots.Push(i);
			}
			else
				AllSentrySpots.Push(i);
		}

		if (BestSentrySpots.Length > 0)
			i = BestSentrySpots.Get(crandomint(0, BestSentrySpots.Length - 1));
		else if (GoodSentrySpots.Length > 0)
			i = GoodSentrySpots.Get(crandomint(0, GoodSentrySpots.Length - 1));
		else if (AllSentrySpots.Length > 0)
			i = AllSentrySpots.Get(crandomint(0, AllSentrySpots.Length - 1));
		else
			i = -1;
		
		delete BestSentrySpots;
		delete GoodSentrySpots;
		delete AllSentrySpots;

		if (i != -1)
		{
			m_goalIndex[client] = i;
			if (m_paths[i].radius > 0.0)
			{
				area[0] = m_paths[i].origin[0] + GetRandomFloat(-m_paths[i].radius, m_paths[i].radius);
				area[1] = m_paths[i].origin[1] + GetRandomFloat(-m_paths[i].radius, m_paths[i].radius);
				area[2] = m_paths[i].origin[2];
			}
			else
				area = m_paths[i].origin;
			
			return;
		}
	}
	else if (isPL)
	{
		if (IsValidEntity(m_bluPayload))
		{
			if (PickSentrySpotNAV(client, GetOrigin(m_bluPayload), area))
				return;
		}
		else
		{
			i = -1;
			while (IsValidEntity((i = FindEntityByClassname(i, "mapobj_cart_dispenser"))))
			{
				if (PickSentrySpotNAV(client, GetOrigin(i), area))
					return;
			}
		}
	}
	else if (isPLR)
	{
		if (m_team[client] == 2)
		{
			if (IsValidEntity(m_bluPayload))
			{
				if (PickSentrySpotNAV(client, GetOrigin(m_bluPayload), area))
					return;
			}
			else
			{
				i = -1;
				while (IsValidEntity((i = FindEntityByClassname(i, "mapobj_cart_dispenser"))))
				{
					if (GetTeamNumber(i) != 3)
						continue;

					if (PickSentrySpotNAV(client, GetOrigin(i), area))
						return;
				}
			}
		}
		else
		{
			if (IsValidEntity(m_redPayload))
			{
				if (PickSentrySpotNAV(client, GetOrigin(m_redPayload), area))
					return;
			}
			else
			{
				i = -1;
				while (IsValidEntity((i = FindEntityByClassname(i, "mapobj_cart_dispenser"))))
				{
					if (GetTeamNumber(i) != 2)
						continue;

					if (PickSentrySpotNAV(client, GetOrigin(i), area))
						return;
				}
			}
		}
	}
	else if (isCTF)
	{
		i = -1;
		while (IsValidEntity((i = FindEntityByClassname(i, "item_teamflag"))))
		{
			if (HasEntProp(i, Prop_Send, "m_nFlagStatus") && m_team[client] == GetTeamNumber(i))
			{
				if (PickSentrySpotNAV(client, GetOrigin(i), area))
					return;
			}
		}
	}
	else if (isKOTH || isArena)
	{
		if (IsValidEntity(m_capturePoint))
		{
			if (PickSentrySpotNAV(client, GetOrigin(m_capturePoint), area))
				return;
		}
		else
		{
			i = -1;
			while (IsValidEntity((i = FindEntityByClassname(i, "team_control_point"))))
			{
				if (PickSentrySpotNAV(client, GetOrigin(i), area))
					return;
			}
		}
	}

	i = crandomint(1, MaxClients + 1);
	if (IsValidClient(i) && GetClientTeam(i) != GetClientTeam(i))
		area = GetOrigin(i);
}

stock bool PickSentrySpotNAV(const int client, const float goal[3], float position[3], const float maxRange = 1300.0)
{
	CNavArea area = NavMesh.GetNearestNavArea(goal, false, 768.0, false, false, m_team[client]);
	if (area == NULL_AREA)
		return false;
	
	static AreasCollector areas;
	areas = NavMesh.CollectSurroundingAreas(area, maxRange);
	
	int i;
	float height;
	float point[3];
	height = GetHeight(client);
	int count = areas.Count();
	for (i = 0; i < count; i++)
	{
		area = areas.Get(i);
		GetRandomPoint(area, point);

		if (IsDangerous(client, point, height))
			continue;
		
		position = point;
		return true;
	}

	return false;
}