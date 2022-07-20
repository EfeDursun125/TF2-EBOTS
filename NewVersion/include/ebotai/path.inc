float pathdelayer;

stock void FollowPath(int client, float targetposition[3], int entity = -1, int index = -1)
{
	if (m_positions[client] != null)
	{
		if (m_targetNode[client] > 0 && m_targetNode[client] <= m_positions[client].Length)
		{
			if (m_hasWaypoints)
				m_currentIndex[client] = m_pathIndex[client].Get(m_currentWaypointIndex[client]);

			float flGoPos[3];
			m_positions[client].GetArray(m_targetNode[client], flGoPos);
			
			float flPos[3]
			GetClientAbsOrigin(client, flPos);
			
			float flToPos[3];
			flToPos[0] = flGoPos[0];
			flToPos[1] = flGoPos[1];
			flToPos[2] = flGoPos[2];
			flToPos[2] += GetHeight(client);
			
			if (GetConVarInt(EBotDebug) == 1)
			{
				TE_SetupBeamPoints(flGoPos, flToPos, m_laserIndex, m_laserIndex, 0, 30, 0.1, 1.0, 1.0, 5, 0.0, {255, 0, 0, 255}, 30);
				TE_SendToClient(m_hostEntity);
				if (m_hasWaypoints && m_currentIndex[client] != -1)
				{
					float Vector[3];
					float Vector2[3];
					Vector = VectorAsFloat(m_paths[m_currentIndex[client]].origin);
					Vector2 = Vector;
					Vector2[2] += GetHeight(client);
					TE_SetupBeamPoints(Vector, Vector2, m_laserIndex, m_laserIndex, 0, 30, 0.1, 1.0, 1.0, 5, 0.0, {0, 255, 0, 255}, 30);
					TE_SendToClient(m_hostEntity);
				}
			}

			if (m_nextStuckCheck[client] <= GetGameTime())
			{
				MoveTo(client, flGoPos);
				CheckWalls(client);
				if (m_nextStuckCheck[client] + 3.0 <= GetGameTime())
				{
					FindPath(client, targetposition, entity, index);
					m_nextStuckCheck[client] = GetGameTime() + 5.0;
				}
			}
			else if (m_hasWaypoints && m_currentIndex[client] == m_goalIndex[client] && (m_paths[m_goalIndex[client]].flags == _:WAYPOINT_SNIPER || m_paths[m_goalIndex[client]].flags == _:WAYPOINT_DEFEND))
			{
				m_nextStuckCheck[client] = GetGameTime() + 5.0;
				MoveToWaypoint(client, flGoPos, targetposition);
			}
			else
				MoveTo(client, flGoPos, m_hasWaypoints ? m_hasEntitiesNear[client] : true);
			
			flGoPos[2] = flPos[2];

			float speed = GetEntPropFloat(client, Prop_Data, "m_flMaxspeed");
			if (m_hasWaypoints)
			{
				bool isCharging = false;
				float value = isCharging ? 192.0 : 64.0;
				if (m_paths[m_currentIndex[client]].flags == _:WAYPOINT_SNIPER || m_paths[m_currentIndex[client]].flags == _:WAYPOINT_DEFEND || m_paths[m_currentIndex[client]].flags == _:WAYPOINT_SENTRY)
				{
					if (m_currentIndex[client] != -1 && m_paths[m_currentIndex[client]].radius > 0.0)
						value += float(m_paths[m_currentIndex[client]].radius);
					else
						value = 48.0;
				}
				else
				{
					if (m_currentIndex[client] != -1 && m_paths[m_currentIndex[client]].radius > 0.0)
						value = float(m_paths[m_currentIndex[client]].radius);
					value += (GetConVarFloat(EBotFPS) * speed);
				}

				float flNodeDist = GetVectorDistance(VectorAsFloat(m_paths[m_currentIndex[client]].origin), flPos, true);
				if (flNodeDist <= Squaredf(value) || GetVectorDistance(flGoPos, flPos, true) <= Squaredf(isCharging ? 162.0 : 54.0))
				{
					if (m_paths[m_currentIndex[client]].flags == _:WAYPOINT_ROCKETJUMP)
					{
						m_pauseTime[client] = GetGameTime() + 1.0;
						m_buttons[client] |= IN_JUMP;
						m_lookAt[client] = VectorAsFloat(m_paths[m_currentIndex[client]].origin);
						m_buttons[client] |= IN_ATTACK;
					}
					m_nextStuckCheck[client] = GetGameTime() + 5.0;
					m_currentWaypointIndex[client]--;
					m_targetNode[client]--;
					if (m_paths[m_currentIndex[client]].flags == _:WAYPOINT_JUMP || m_paths[m_currentIndex[client]].flags == _:WAYPOINT_DOUBLEJUMP)
					{
						m_positions[client].GetArray(m_targetNode[client], flGoPos);
						MoveTo(client, flGoPos, !m_hasWaypoints);
						m_buttons[client] |= IN_JUMP;
						if (TF2_GetPlayerClass(client) == TFClass_Scout)
						{
							DJTime[client] = GetGameTime() + 0.5;
							NoDodge[client] = GetGameTime() + 6.5;
						}
						else
							DJTime[client] = GetGameTime() + 0.05;
					}
				}
			}
			else
			{
				float flNodeDist = GetVectorDistance(flGoPos, flPos, true);
				float value = 54.0 + (GetConVarFloat(EBotFPS) * speed);
				if (flNodeDist <= Squaredf(value))
				{
					m_nextStuckCheck[client] = GetGameTime() + 5.0;
					float jumppos[3];
					CNavArea jump = NavMesh_GetArea(jumppos);
					if (jump != INVALID_NAV_AREA && jump.Attributes & NAV_MESH_JUMP)
						m_buttons[client] |= IN_JUMP;
					m_targetNode[client]--;
				}
			}
			m_pathAhead[client] = flToPos;
		}
		else
			FindPath(client, targetposition, entity, index);
	}
	else
		FindPath(client, targetposition, entity, index);
	
	if (m_isSlowThink[client] && !IsMoving(client))
		FindPath(client, targetposition, entity, index);
}

stock void FindPath(int client, float targetposition2[3], int entity = -1, int index = -1)
{
	if (m_hasWaypoints)
	{
		if (StrContains(currentMap, "plr_" , false) != -1 || (StrContains(currentMap, "pl_" , false) != -1 && GetClientTeam(client) == 3))
			SelectObjective(client);
		if (index == -1)
			index = FindNearestWaypoint(targetposition2, 999999.0, client);
		AStarFindPath(-1, index, client, targetposition2);
		return;
	}

	if (pathdelayer > GetGameTime())
		return;
	
	DeletePathNodes(client);
	
	float flPos[3];
	GetClientAbsOrigin(client, flPos);
	
	flPos[2] += 15.0;
	targetposition2[2] += 15.0;
	
	CNavArea startArea = NavMesh_GetNearestArea(flPos, false, 768.0, false, false);
	CNavArea goalArea = NavMesh_GetNearestArea(targetposition2, false, 768.0, false, false);
	
	if (startArea == INVALID_NAV_AREA || goalArea == INVALID_NAV_AREA)
		return;
	
	if (startArea == goalArea)
		return;
	
	float flMaxPathLength = 0.0;
	float flMaxStepSize = 48.0;
	CNavArea closestArea = INVALID_NAV_AREA;
	
	bool bBuiltPath = NavMesh_BuildPath(startArea, goalArea, targetposition2, NavMeshRandomizedPathCost, _, closestArea, flMaxPathLength, flMaxStepSize);
	if (bBuiltPath)
	{
		pathdelayer = GetGameTime() + 0.16;
		CNavArea tempArea = closestArea;
		CNavArea parentArea = tempArea.Parent;
		int iNavDirection;
		float flHalfWidth;
		
		float flCenterPortal[3], flClosestPoint[3];

		m_positions[client].PushArray(targetposition2, 3);
		while (parentArea != INVALID_NAV_AREA)
		{
			float flTempAreaCenter[3], flParentAreaCenter[3];
			tempArea.GetRandomPoint(flTempAreaCenter);
			parentArea.GetRandomPoint(flParentAreaCenter);
			
			iNavDirection = tempArea.ComputeDirection(flParentAreaCenter);
			tempArea.ComputePortal(parentArea, iNavDirection, flCenterPortal, flHalfWidth);
			tempArea.ComputeClosestPointInPortal(parentArea, iNavDirection, flCenterPortal, flClosestPoint);
			
			flClosestPoint[2] = tempArea.GetZ(flClosestPoint);
			m_positions[client].PushArray(flClosestPoint, 3);
			
			tempArea = parentArea;
			parentArea = tempArea.Parent;
		}
		
		for (int i = m_positions[client].Length - 1; i > 0; i--)
		{
			float flFromPos[3], flToPos[3];
			m_positions[client].GetArray(i, flFromPos, 3);
			m_positions[client].GetArray(i - 1, flToPos, 3);
			
			if (GetConVarInt(EBotDebug) == 1)
			{
				TE_SetupBeamPoints(flFromPos, flToPos, m_laserIndex, m_laserIndex, 0, 30, 5.0, 1.0, 1.0, 5, 0.0, {0, 255, 0, 255}, 30);
				TE_SendToClient(m_hostEntity);
			}
		}
		
		m_targetNode[client] = m_positions[client].Length - 2;
	}
	else
		m_lastFailedEntity[client] = entity;
}

public int NavMeshFastestPathCost(CNavArea area, CNavArea from, CNavLadder ladder, any data)
{
	if (from == INVALID_NAV_AREA)
		return 0;
	
	int iDist = 0;
	if (ladder != INVALID_NAV_LADDER)
		iDist = RoundToFloor(ladder.Length);
	else
	{
		float flAreaCenter[3]; float flFromAreaCenter[3];
		area.GetCenter(flAreaCenter);
		from.GetCenter(flFromAreaCenter);
		iDist = RoundToFloor(GetFastDistance(flAreaCenter, flFromAreaCenter));
	}

	return iDist + from.CostSoFar;
}

public int NavMeshRandomizedPathCost(CNavArea area, CNavArea from, CNavLadder ladder, any data)
{
	if (from == INVALID_NAV_AREA)
		return 0;

	int iDist = 0;
	if (ladder != INVALID_NAV_LADDER)
		iDist = RoundToFloor(ladder.Length);
	else
	{
		float flAreaCenter[3]; float flFromAreaCenter[3];
		area.GetRandomPoint(flAreaCenter);
		from.GetRandomPoint(flFromAreaCenter);
		
		iDist = RoundToFloor(GetFastDistance(flAreaCenter, flFromAreaCenter));
	}
	
	int iCost = iDist + from.CostSoFar;
	int iAreaFlags = area.Attributes;

	if (iAreaFlags & TF_NAV_SPAWN_ROOM_RED || iAreaFlags & TF_NAV_SPAWN_ROOM_BLUE || iAreaFlags & TF_NAV_SPAWN_ROOM_EXIT || iAreaFlags & TF_NAV_BLUE_SENTRY_DANGER || iAreaFlags & TF_NAV_RED_SENTRY_DANGER)
		iCost += (iDist * iDist);
	else
	{
		if (iAreaFlags & NAV_MESH_CROUCH)
			iCost += 20;
	
		if (iAreaFlags & NAV_MESH_JUMP)
			iCost += (5 * iDist);

		// rarely use danger areas
		if (iAreaFlags & NAV_MESH_AVOID && GetRandomInt(1, 3) != 1)
			iCost += (iDist * iDist);
	}
	
	return iCost;
}