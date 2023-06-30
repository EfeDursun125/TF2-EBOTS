bool m_teleEnterTriggerBlock[TFMaxPlayers];

public bool BuildTeleEnterRequirements(int client)
{
	if (m_teleEnterTriggerBlock[client])
		return false;
	
	if (m_enterFail[client])
		return false;
	
	if (GetMetal(client) < 50.0)
		return false;
	
	return true;
}

float teleenterbuildarea[3];
public void BuildTeleEnterStart(int client)
{
	PickTeleEnterSpot(client, teleenterbuildarea);
	if (!IsNullVector(teleenterbuildarea))
		DeletePathNodes(client);
	else
	{
		m_teleEnterTriggerBlock[client] = true;
		FinishCurrentProcess(client);
	}
}

public void BuildTeleEnterUpdate(int client)
{
	CheckHealth(client);
	FindFriendsAndEnemiens(client);
	FindEnemyEntities(client);
	
	float range = GetVectorDistance(GetOrigin(client), teleenterbuildarea, true);
	if (range <= Squaredf(300.0))
	{
		m_ignoreEnemies[client] = GetGameTime() + 1.0;

		if (!IsWeaponSlotActive(client, 5) && !IsMoving(client))
		{
			FakeClientCommandThrottled(client, "build 1 0");
			m_buttons[client] |= IN_ATTACK;
		}

		if ((m_hasWaypoints || m_hasNavpoints) && m_isSlowThink[client])
		{
			int index = m_goalIndex[client];
			if (index != -1)
			{
				if (GetRandomInt(1, 2) == 1)
					m_lookAt[client] = m_paths[index].campStart;
				else
					m_lookAt[client] = m_paths[index].campEnd;
			}
		}

		LookAtPosition(client, m_lookAt[client], GetEyeAngles(client));

		if (IsWeaponSlotActive(client, 5))
		{
			if (!IsMoving(client))
				m_buttons[client] |= IN_ATTACK;

			int objBeingBuilt = GetEntPropEnt(GetActiveWeapon(client), Prop_Send, "m_hObjectBeingBuilt");
			if (!IsValidEntity(objBeingBuilt))
				return;
			
			if (m_isSlowThink[client] && GetRandomInt(1, 10) == 1)
				PickTeleEnterSpot(client, teleenterbuildarea);
			
			return;
		}
	}
	else
		LookUpdate(client);

	if (range > Squaredf(70.0))
		FollowPath(client, teleenterbuildarea);
	else
		MoveToWaypoint(client, teleenterbuildarea);

	int teleenter = TF2_GetObject(client, TFObject_Teleporter, TFObjectMode_Entrance);
	if (IsValidEntity(teleenter))
	{
		FinishCurrentProcess(client);
		WantsBuildTeleporterEnter[client] = false;
	}
}

public void BuildTeleEnterEnd(int client)
{
	m_enterFail[client] = true;
	EquipWeaponSlot(client, 0);
}

stock void PickTeleEnterSpot(int client, float area[3] = NULL_VECTOR)
{
	if ((m_hasWaypoints || m_hasNavpoints) && m_hasTeleporterEnterWaypoints)
	{
		int index = -1;
		ArrayList GoodTeleEnterSpots = new ArrayList();
		ArrayList AllTeleEnterSpots = new ArrayList();
		for (int i = 0; i < m_waypointNumber; i++)
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

			if (!IsWaypointUsed(i))
				GoodTeleEnterSpots.Push(i);
			else
				AllTeleEnterSpots.Push(i);
		}

		if (index == -1)
		{
			if (GoodTeleEnterSpots.Length > 0)
				index = GoodTeleEnterSpots.Get(GetRandomInt(0, GoodTeleEnterSpots.Length - 1));
			else if (AllTeleEnterSpots.Length > 0)
				index = AllTeleEnterSpots.Get(GetRandomInt(0, AllTeleEnterSpots.Length - 1));
		}

		delete GoodTeleEnterSpots;
		delete AllTeleEnterSpots;

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
}