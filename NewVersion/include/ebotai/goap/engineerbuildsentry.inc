public bool BuildSentryRequirements(int client)
{
	if (GetMetal(client) < 130.0)
		return false;
	return true;
}

float sentrybuildarea[3];
public void BuildSentryStart(int client)
{
	DeletePathNodes(client); // NEVER EVER EVER DELETE THIS LINE
	PickSentrySpot(client, sentrybuildarea);
}

public void BuildSentryUpdate(int client)
{
	bool lookupdate = true;

	CheckHealth(client);
	FindFriendsAndEnemiens(client);
	FindEnemyEntities(client);
	
	float range = GetVectorDistance(GetOrigin(client), sentrybuildarea, true);
	if (range <= Squared(300))
	{
		if (!IsWeaponSlotActive(client, 5) && !IsMoving(client))
			FakeClientCommandThrottled(client, "build 2");
		lookupdate = false;
		int index = m_goalIndex[client];
		if (m_hasWaypoints && index != -1)
		{
			if (GetRandomInt(1, 2) == 1)
				m_lookAt[client] = VectorAsFloat(m_paths[index].campStart);
			else
				m_lookAt[client] = VectorAsFloat(m_paths[index].campEnd);
		}
		else
			m_lookAt[client] = sentrybuildarea;
	}

	if (range > Squared(70))
		FollowPath(client, sentrybuildarea);
	else
		MoveToWaypoint(client, sentrybuildarea, sentrybuildarea);
	
	if (IsWeaponSlotActive(client, 5))
	{
		int objBeingBuilt = GetEntPropEnt(GetActiveWeapon(client), Prop_Send, "m_hObjectBeingBuilt");
		if (!IsValidEntity(objBeingBuilt))
			return;
		
		bool m_bPlacementOK = !!GetEntData(objBeingBuilt, FindSendPropInfo("CObjectSentrygun", "m_iKills") - 4);
		m_buttons[client] |= IN_ATTACK;
		
		if (!m_bPlacementOK && m_isSlowThink[client] && GetRandomInt(1, 2) == 1)
			PickSentrySpot(client, sentrybuildarea);

		return;
	}
	
	int sentry = TF2_GetObject(client, TFObject_Sentry, TFObjectMode_None);
	if (sentry != -1)
	{
		FinishCurrentProcess(client);
		EngineerMoveSentryTimer[client] = GetGameTime() + GetRandomFloat(90.0, 160.0);
		WantsBuildSentryGun[client] = false;
		WantsMoveSentryGun[client] = false;
	}
	
	if (lookupdate)
		LookUpdate(client);
}

public void BuildSentryEnd(int client)
{
	// :(
}

public void PickSentrySpot(int client, float area[3])
{
	if (m_hasWaypoints && m_hasSentryWaypoints)
	{
		int index = -1;
		ArrayList GoodSentrySpots = new ArrayList();
		ArrayList AllSentrySpots = new ArrayList();
		for (int i = 0; i < m_waypointNumber; i++)
		{
			// blocked waypoint
   			if (m_paths[i].activeArea != 0 && m_paths[i].activeArea != currentActiveArea)
				continue;
			
			if (m_lastFailedWaypoint[client] == i)
				continue;

			if (m_paths[i].flags != _:WAYPOINT_SENTRY)
				continue;

    		// not for our team
			int cteam = GetClientTeam(client);
 			if (cteam == 3 && m_paths[i].team == 2)
    			continue;
				
   			if (cteam == 2 && m_paths[i].team == 3)
        		continue;

			if (!IsWaypointUsed(i))
				GoodSentrySpots.Push(i);
			else
				AllSentrySpots.Push(i);
		}

		if (index == -1)
		{
			if (GoodSentrySpots.Length > 0)
				index = GoodSentrySpots.Get(GetRandomInt(0, GoodSentrySpots.Length - 1));
			else if (AllSentrySpots.Length > 0)
				index = AllSentrySpots.Get(GetRandomInt(0, AllSentrySpots.Length - 1));
		}

		delete GoodSentrySpots;
		delete AllSentrySpots;

		if (index != -1)
		{
			m_goalIndex[client] = index;
			if (m_paths[index].radius > 0)
			{
				area[0] = float(m_paths[index].origin[0] + GetRandomInt(-m_paths[index].radius, m_paths[index].radius));
				area[1] = float(m_paths[index].origin[1] + GetRandomInt(-m_paths[index].radius, m_paths[index].radius));
				area[2] = float(m_paths[index].origin[2]);
			}
			else
				area = VectorAsFloat(m_paths[index].origin);
			return;
		}
	}
	else // do navmesh version
	{
		// damn, no waypoints, let's try to find a good spot
	}
}