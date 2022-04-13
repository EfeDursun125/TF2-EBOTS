float pathdelayer;

stock bool IsPathPossible(float start[3], float goal[3], float maxrange = 0.0, float maxstepsize = 48.0)
{
	// no checks for now, just return true because we don't have a pathfinder yet, using it too much makes game run slow
	if (pathdelayer > GetGameTime())
		return true;
	
	CNavArea startArea = NavMesh_GetNearestArea(start, false, 256.0, false, false);
	CNavArea goalArea = NavMesh_GetNearestArea(goal, false, 256.0, false, false);
	if (startArea == INVALID_NAV_AREA || goalArea == INVALID_NAV_AREA)
		return false;

	CNavArea closestArea = INVALID_NAV_AREA;
	return NavMesh_BuildPath(startArea, goalArea, goal, NavMeshFastestPathCost, _, closestArea, maxrange, maxstepsize);
}

stock void FollowPath(int client, float targetposition[3])
{
	if (m_positions[client] != null)
	{
		if (m_targetNode[client] >= 0 && m_targetNode[client] < m_positions[client].Length)
		{
			float flGoPos[3];
			m_positions[client].GetArray(m_targetNode[client], flGoPos);
			
			if (m_nextStuckCheck[client] <= GetGameTime() && pathdelayer < GetGameTime())
			{
				m_buttons[client] |= IN_JUMP;
				m_nextStuckCheck[client] = GetGameTime() + 5.0;
				m_nextPathUpdate[client] = GetGameTime() + 5.5;
				FindPath(client, targetposition);
			}
			
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
			}
			
			flGoPos[2] = flPos[2];
			float flNodeDist = GetVectorDistance(flGoPos, flPos, true);
			MoveTo(client, flGoPos);

			float value = 32.0 + (GetConVarFloat(EBotFPS) * 280);
			if (flNodeDist <= Squaredf(value))
			{
				m_nextStuckCheck[client] = GetGameTime() + 5.0;
				if (m_hasWaypoints)
				{
					m_currentWaypointIndex[client]--;
				}
				else
				{
					float jumppos[3];
					CNavArea jump = NavMesh_GetArea(jumppos);
					if (jump != INVALID_NAV_AREA && jump.Attributes & NAV_MESH_JUMP)
						m_buttons[client] |= IN_JUMP;
				}
				m_targetNode[client]--;
				
			}

			m_pathAhead[client] = flToPos;
		}
		else if (m_nextPathUpdate[client] <= GetGameTime())
			FindPath(client, targetposition);
	}
	else if (m_nextPathUpdate[client] <= GetGameTime())
		FindPath(client, targetposition);
	
	return;
}

stock void FindPath(int client, float targetposition2[3])
{
	if (pathdelayer > GetGameTime())
		return;

	DeletePathNodes(client);

	if (m_hasWaypoints)
	{
		int destIndex = FindNearestWaypoint(targetposition2);
		AStarFindPath(-1, destIndex, client);
		return;
	}
	
	float flPos[3];
	GetClientAbsOrigin(client, flPos);
	
	flPos[2] += 15.0;
	targetposition2[2] += 15.0;
	
	CNavArea startArea = NavMesh_GetNearestArea(flPos, false, 768.0, false, false);
	CNavArea goalArea = NavMesh_GetNearestArea(targetposition2, false, 768.0, false, false);
	
	if (startArea == INVALID_NAV_AREA || goalArea == INVALID_NAV_AREA)
		return;
	
	float flMaxPathLength = 0.0;
	float flMaxStepSize = 48.0;
	CNavArea closestArea = INVALID_NAV_AREA;
	
	bool bBuiltPath = NavMesh_BuildPath(startArea, goalArea, targetposition2, NavMeshRandomizedPathCost, _, closestArea, flMaxPathLength, flMaxStepSize);
	if (bBuiltPath)
	{
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
				TE_SetupBeamPoints(flFromPos, flToPos, m_laserIndex, m_laserIndex, 0, 30, 1.0, 1.0, 1.0, 5, 0.0, {0, 255, 0, 255}, 30);
				TE_SendToClient(m_hostEntity);
			}
		}
		
		m_targetNode[client] = m_positions[client].Length - 2;
	}
	
	m_nextPathUpdate[client] = GetGameTime() + 1.0;
	pathdelayer = GetGameTime() + 0.2;
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
		
		iDist = RoundToFloor(GetVectorDistance(flAreaCenter, flFromAreaCenter));
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