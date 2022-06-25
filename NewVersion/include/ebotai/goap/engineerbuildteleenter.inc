bool m_teleEnterTriggerBlock[MAXPLAYERS + 1] = false;
public bool BuildTeleEnterRequirements(int client)
{
	if (m_teleEnterTriggerBlock[client])
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
	LookUpdate(client);
	
	float range = GetVectorDistance(GetOrigin(client), teleenterbuildarea, true);
	if (range <= Squared(300))
	{
		if (!IsWeaponSlotActive(client, 5) && !IsMoving(client))
		{
			FakeClientCommandThrottled(client, "build 1 0");
			m_buttons[client] |= IN_ATTACK;
		}
		
		float position[3];
		position = NULL_VECTOR;

		if (m_hasWaypoints)
		{
			int index = m_goalIndex[client];
			if (index != -1)
			{
				if (GetRandomInt(1, 2) == 1)
					position = VectorAsFloat(m_paths[index].campStart);
				else
					position = VectorAsFloat(m_paths[index].campEnd);
			}
		}
		else
			position = teleenterbuildarea;
		
		if (!IsNullVector(position))
			m_lookAt[client] = position;
	}

	if (range > Squared(70))
		FollowPath(client, teleenterbuildarea);
	else
		MoveToWaypoint(client, teleenterbuildarea, teleenterbuildarea);
	
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
	
	int teleenter = TF2_GetObject(client, TFObject_Teleporter, TFObjectMode_Entrance);
	if (teleenter != -1)
	{
		FinishCurrentProcess(client);
		WantsBuildTeleporterEnter[client] = false;
	}
}

public void BuildTeleEnterEnd(int client)
{
	// :(
}

stock void PickTeleEnterSpot(int client, float area[3] = NULL_VECTOR)
{
	if (m_hasWaypoints && m_hasTeleporterEnterWaypoints)
	{
		int index = -1;
		ArrayList GoodTeleEnterSpots = new ArrayList();
		ArrayList AllTeleEnterSpots = new ArrayList();
		for (int i = 0; i < m_waypointNumber; i++)
		{
			if (m_paths[i].flags != _:WAYPOINT_TELEPORTERENTER)
				continue;
			
			// blocked waypoint
   			if (m_paths[i].activeArea != 0 && m_paths[i].activeArea != currentActiveArea)
				continue;
			
			if (m_lastFailedWaypoint[client] == i)
				continue;
			
    		// not for our team
			int cteam = GetClientTeam(client);
 			if (cteam == 3 && m_paths[i].team == 2)
    			continue;
				
   			if (cteam == 2 && m_paths[i].team == 3)
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
}