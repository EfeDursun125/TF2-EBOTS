bool m_teleExitTriggerBlock[TFMaxPlayers];

public bool BuildTeleExitRequirements(const int client)
{
	if (m_teleExitTriggerBlock[client])
		return false;

	if (m_exitFail[client])
		return false;

	if (GetMetal(client) < 50.0)
		return false;

	return true;
}

float teleexitbuildarea[TFMaxPlayers][3];
public void BuildTeleExitStart(const int client)
{
	PickTeleExitSpot(client, teleexitbuildarea[client]);
	if (!IsNullVector(teleexitbuildarea[client]))
		DeletePathNodes(client);
	else
	{
		m_teleExitTriggerBlock[client] = true;
		FinishCurrentProcess(client);
	}
}

public void BuildTeleExitUpdate(const int client)
{
	m_goalEntity[client] = -1;
	CheckHealth(client);
	FindFriendsAndEnemiens(client);
	FindEnemyEntities(client);

	float range = GetVectorDistance(GetOrigin(client), teleexitbuildarea[client], true);
	if (range < Squaredf(300.0))
	{
		m_ignoreEnemies[client] = GetGameTime() + 1.0;

		if (!IsWeaponSlotActive(client, 5) && !IsMoving(client))
			FakeClientCommandThrottled(client, "build 1 1");

		if ((m_hasWaypoints || m_hasNavpoints) && m_isSlowThink[client])
		{
			if (m_goalIndex[client] != -1)
			{
				if (crandomint(1, 2) == 1)
					m_lookAt[client] = m_paths[m_goalIndex[client]].campStart;
				else
					m_lookAt[client] = m_paths[m_goalIndex[client]].campEnd;
			}
		}

		LookAtPosition(client, m_lookAt[client], GetEyeAngles(client));

		if (IsWeaponSlotActive(client, 5))
		{
			if (!IsMoving(client))
				m_buttons[client] |= IN_ATTACK;

			if (!IsValidEntity(GetEntPropEnt(GetActiveWeapon(client), Prop_Send, "m_hObjectBeingBuilt")))
				return;

			if (m_isSlowThink[client] && crandomint(1, 10) == 1)
				PickTeleExitSpot(client, teleexitbuildarea[client]);

			return;
		}
	}
	else
		LookUpdate(client);

	if (range > Squaredf(70.0))
		FollowPath(client, teleexitbuildarea[client]);
	else
		MoveToWaypoint(client, teleexitbuildarea[client]);

	if (IsValidEntity(TF2_GetObject(client, TFObject_Teleporter, TFObjectMode_Exit)))
	{
		FinishCurrentProcess(client);
		WantsBuildTeleporterExit[client] = false;
	}
}

public void BuildTeleExitEnd(const int client)
{
	m_exitFail[client] = true;
	EquipWeaponSlot(client, 0);
}

stock void PickTeleExitSpot(const int client, float area[3] = NULL_VECTOR)
{
	if ((m_hasWaypoints || m_hasNavpoints) && m_hasTeleporterExitWaypoints)
	{
		ArrayList BestTeleExitSpots = new ArrayList();
		if (!BestTeleExitSpots)
			return;

		ArrayList GoodTeleExitSpots = new ArrayList();
		if (!GoodTeleExitSpots)
			return;

		ArrayList AllTeleExitSpots = new ArrayList();
		if (!AllTeleExitSpots)
			return;

		int i;
		for (i = 0; i < m_waypointNumber; i++)
		{
			if (!(m_paths[i].flags & WAYPOINT_TELEPORTEREXIT))
				continue;

			// blocked waypoint
   			if (m_paths[i].activeArea != 0 && !(m_paths[i].activeArea & currentActiveArea))
				continue;

			if (m_lastFailedWaypoint[client] == i)
				continue;

    		// not for our team
 			if (m_team[client] == 3 && m_paths[i].team == 2)
    			continue;
   			else if (m_team[client] == 2 && m_paths[i].team == 3)
        		continue;

			if (m_paths[i].radius > 32.0 || !IsWaypointUsed(i))
			{
				if (GetDamage(client, i) < GetMaxDamage(client))
					BestTeleExitSpots.Push(i);
				else
					GoodTeleExitSpots.Push(i);
			}
			else
				AllTeleExitSpots.Push(i);
		}

		if (BestTeleExitSpots.Length > 0)
			i = BestTeleExitSpots.Get(crandomint(0, BestTeleExitSpots.Length - 1));
		else if (GoodTeleExitSpots.Length > 0)
			i = GoodTeleExitSpots.Get(crandomint(0, GoodTeleExitSpots.Length - 1));
		else if (AllTeleExitSpots.Length > 0)
			i = AllTeleExitSpots.Get(crandomint(0, AllTeleExitSpots.Length - 1));
		else
			i = -1;

		delete BestTeleExitSpots;
		delete GoodTeleExitSpots;
		delete AllTeleExitSpots;

		if (i != -1)
		{
			m_goalIndex[client] = i;
			if (m_paths[i].radius > 0.0)
			{
				area[0] = m_paths[i].origin[0] + crandomfloat(-m_paths[i].radius, m_paths[i].radius);
				area[1] = m_paths[i].origin[1] + crandomfloat(-m_paths[i].radius, m_paths[i].radius);
				area[2] = m_paths[i].origin[2];
			}
			else
				area = m_paths[i].origin;
		}
	}
}
