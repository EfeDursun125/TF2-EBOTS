public bool BuildSentryRequirements(const int client)
{
	if (GetMetal(client) < 130.0)
		return false;
	
	if (IsValidEntity(SentryGun[client]))
		return false;
		
	return true;
}

float sentrybuildarea[3];
public void BuildSentryStart(const int client)
{
	DeletePathNodes(client); // NEVER EVER EVER DELETE THIS LINE
	PickSentrySpot(client, sentrybuildarea);
}

public void BuildSentryUpdate(const int client)
{
	CheckHealth(client);
	FindFriendsAndEnemiens(client);
	FindEnemyEntities(client);
	
	float range = GetVectorDistance(GetOrigin(client), sentrybuildarea, true);
	if (range <= Squaredf(300.0))
	{
		m_ignoreEnemies[client] = GetGameTime() + 1.0;
		if (!IsWeaponSlotActive(client, 5) && !IsMoving(client))
			FakeClientCommandThrottled(client, "build 2 0");

		if (m_goalIndex[client] != -1 && (m_hasWaypoints || m_hasNavpoints))
		{
			int index = m_goalIndex[client];
			m_lookAt[client] = m_paths[index].campStart;
			LookAtPosition(client, m_lookAt[client], GetEyeAngles(client));
		}
		else
			LookUpdate(client);
		
		if (IsWeaponSlotActive(client, 5) && !IsMoving(client) && range <= Squaredf(128.0) && IsClientAimingToPosition(client, m_lookAt[client]))
		{
			m_buttons[client] |= IN_ATTACK;
		
			int objBeingBuilt = GetEntPropEnt(GetActiveWeapon(client), Prop_Send, "m_hObjectBeingBuilt");
			if (!IsValidEntity(objBeingBuilt))
				return;
		
			bool m_bPlacementOK = !!GetEntData(objBeingBuilt, FindSendPropInfo("CObjectSentrygun", "m_iKills") - 4);
			if (!m_bPlacementOK && m_isSlowThink[client] && GetRandomInt(1, 5) == 1)
			{
				PickSentrySpot(client, sentrybuildarea);
				EquipWeaponSlot(client, 0);
			}

			return;
		}
	}
	else
		LookUpdate(client);

	if (range > Squaredf(70.0))
		FollowPath(client, sentrybuildarea);
	else
		MoveToWaypoint(client, sentrybuildarea);
	
	int sentry = TF2_GetObject(client, TFObject_Sentry, TFObjectMode_None);
	if (IsValidEntity(sentry))
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
	if ((m_hasWaypoints || m_hasNavpoints) && m_hasSentryWaypoints)
	{
		int index = -1;
		ArrayList GoodSentrySpots = new ArrayList();
		ArrayList AllSentrySpots = new ArrayList();
		for (int i = 0; i < m_waypointNumber; i++)
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
			if (m_paths[index].radius > 0.0)
			{
				area[0] = m_paths[index].origin[0] + GetRandomFloat(-m_paths[index].radius, m_paths[index].radius);
				area[1] = m_paths[index].origin[1] + GetRandomFloat(-m_paths[index].radius, m_paths[index].radius);
				area[2] = m_paths[index].origin[2];
			}
			else
				area = m_paths[index].origin;
		}
	}
	else // do navmesh version
	{
		int search = GetRandomInt(1, MaxClients + 1);
		if (IsValidClient(search) && GetClientTeam(client) != GetClientTeam(search))
			area = GetOrigin(search);
	}
}