bool m_teleEnterTriggerBlock[TFMaxPlayers];

public bool BuildTeleEnterRequirements(const int client)
{
	if (m_teleEnterTriggerBlock[client])
		return false;
	
	if (m_enterFail[client])
		return false;
	
	if (GetMetal(client) < 50.0)
		return false;
	
	return true;
}

float teleenterbuildarea[TFMaxPlayers][3];
public void BuildTeleEnterStart(const int client)
{
	PickTeleEnterSpot(client, teleenterbuildarea[client]);
	if (!IsNullVector(teleenterbuildarea[client]))
		DeletePathNodes(client);
	else
	{
		m_teleEnterTriggerBlock[client] = true;
		FinishCurrentProcess(client);
	}
}

public void BuildTeleEnterUpdate(const int client)
{
	m_goalEntity[client] = -1;
	CheckHealth(client);
	FindFriendsAndEnemiens(client);
	FindEnemyEntities(client);
	
	float range = GetVectorDistance(GetOrigin(client), teleenterbuildarea[client], true);
	if (range < Squaredf(300.0))
	{
		m_ignoreEnemies[client] = GetGameTime() + 1.0;

		if (!IsWeaponSlotActive(client, 5) && !IsMoving(client))
		{
			FakeClientCommandThrottled(client, "build 1 0");
			m_buttons[client] |= IN_ATTACK;
		}

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
				PickTeleEnterSpot(client, teleenterbuildarea[client]);
			
			return;
		}
	}
	else
		LookUpdate(client);

	if (range > Squaredf(70.0))
		FollowPath(client, teleenterbuildarea[client]);
	else
		MoveToWaypoint(client, teleenterbuildarea[client]);
	
	if (IsValidEntity(TF2_GetObject(client, TFObject_Teleporter, TFObjectMode_Entrance)))
	{
		FinishCurrentProcess(client);
		WantsBuildTeleporterEnter[client] = false;
	}
}

public void BuildTeleEnterEnd(const int client)
{
	m_enterFail[client] = true;
	EquipWeaponSlot(client, 0);
}

stock void PickTeleEnterSpot(const int client, float area[3] = NULL_VECTOR)
{
	if ((m_hasWaypoints || m_hasNavpoints) && m_hasTeleporterEnterWaypoints)
	{
		ArrayList BestTeleEnterSpots = new ArrayList();
		if (!BestTeleEnterSpots)
			return;

		ArrayList GoodTeleEnterSpots = new ArrayList();
		if (!GoodTeleEnterSpots)
			return;

		ArrayList AllTeleEnterSpots = new ArrayList();
		if (!AllTeleEnterSpots)
			return;

		int i;
		for (i = 0; i < m_waypointNumber; i++)
		{
			if (!(m_paths[i].flags & WAYPOINT_TELEPORTERENTER))
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
					BestTeleEnterSpots.Push(i);
				else
					GoodTeleEnterSpots.Push(i);
			}
			else
				AllTeleEnterSpots.Push(i);
		}

		if (BestTeleEnterSpots.Length)
			i = BestTeleEnterSpots.Get(crandomint(0, BestTeleEnterSpots.Length - 1));
		else if (GoodTeleEnterSpots.Length)
			i = GoodTeleEnterSpots.Get(crandomint(0, GoodTeleEnterSpots.Length - 1));
		else if (AllTeleEnterSpots.Length)
			i = AllTeleEnterSpots.Get(crandomint(0, AllTeleEnterSpots.Length - 1));
		else
			i = -1;

		delete BestTeleEnterSpots;
		delete GoodTeleEnterSpots;
		delete AllTeleEnterSpots;

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