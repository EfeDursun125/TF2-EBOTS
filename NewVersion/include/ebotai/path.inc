stock bool IsPathPossible(const int client, const float startOrigin[3], const float endOrigin[3], const float flMaxPathLength = 0.0)
{
	CNavArea startArea = NavMesh.GetNavArea(startOrigin, 256.0);
	CNavArea goalArea = NavMesh.GetNavArea(endOrigin, 256.0);
	
	if (!startArea || !goalArea)
		return false;
	
	if (startArea == goalArea)
		return true;
	
	CNavArea closestArea = NULL_AREA;
	return NavMesh.BuildPath(startArea, goalArea, endOrigin, PathCostFastest, closestArea, flMaxPathLength, m_team[client], false);
}

stock void FollowPath(const int client, float targetposition[3], int entity = -1, int index = -1)
{
	if (m_spawnTime[client] + 1.0 > GetGameTime())
	{
		SelectObjective(client);
		return;
	}

	if (IsNullVector(targetposition))
	{
		DeletePathNodes(client);
		SelectObjective(client);
		return;
	}
	
	if (m_stopTime[client] > GetGameTime() && !TF2_IsPlayerInCondition(client, TFCond_OnFire))
	{
		if (TF2_IsPlayerInCondition(client, TFCond_Zoomed) && (m_hasEnemiesNear[client] || m_hasEntitiesNear[client]))
			m_stopTime[client] = GetGameTime() + 10.0;
		return;
	}

	if (!m_hasWaypoints && !m_hasNavpoints && GetVectorDistance(GetOrigin(client), targetposition, true) <= Squaredf(128.0))
	{
		DeletePathNodes(client);
		MoveTo(client, targetposition, true);
		return;
	}

	if (m_positions[client] != null)
	{
		if (m_targetNode[client] >= 0 && m_targetNode[client] < m_positions[client].Length)
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
					float Vectorz[3];
					float Vector2[3];
					Vectorz = m_paths[m_currentIndex[client]].origin;
					Vector2 = Vectorz;
					Vector2[2] += GetHeight(client);
					TE_SetupBeamPoints(Vectorz, Vector2, m_laserIndex, m_laserIndex, 0, 30, 0.1, 1.0, 1.0, 5, 0.0, {0, 255, 0, 255}, 30);
					TE_SendToClient(m_hostEntity);
				}
			}

			if (m_nextStuckCheck[client] <= GetGameTime())
			{
				MoveTo(client, flGoPos, !m_hasWaypoints);
				CheckWalls(client);
				m_lastFailedWaypoint[client] = m_currentIndex[client];
				if (m_nextStuckCheck[client] + 3.0 <= GetGameTime())
				{
					FindPath(client, targetposition, entity, index);
					m_nextStuckCheck[client] = GetGameTime() + 5.0;
				}
			}
			else if (m_hasWaypoints && m_goalIndex[client] != -1 && m_currentIndex[client] == m_goalIndex[client] && (m_paths[m_goalIndex[client]].flags & WAYPOINT_SNIPER || m_paths[m_goalIndex[client]].flags & WAYPOINT_DEMOMANCAMP || m_paths[m_goalIndex[client]].flags & WAYPOINT_DEFEND))
			{
				m_nextStuckCheck[client] = GetGameTime() + 5.0;
				MoveToWaypoint(client, flGoPos);
			}
			else
			{
				if (!m_hasWaypoints || (m_goalIndex[client] != -1 && !(m_paths[m_goalIndex[client]].flags & WAYPOINT_SNIPER)))
				{
					if (TF2_IsPlayerInCondition(client, TFCond_Zoomed) && (m_hasEnemiesNear[client] || m_hasEntitiesNear[client]))
						m_stopTime[client] = GetGameTime() + 10.0;
				}

				MoveTo(client, flGoPos, !m_hasWaypoints);
			}
			
			flGoPos[2] = flPos[2];

			float speed = GetEntPropFloat(client, Prop_Data, "m_flMaxspeed");
			if (m_hasWaypoints)
			{
				float flNodeDist = GetVectorDistance(m_paths[m_currentIndex[client]].origin, flPos, true);

				float value = 32.0;
				if (m_paths[m_currentIndex[client]].flags & WAYPOINT_SNIPER || m_paths[m_currentIndex[client]].flags & WAYPOINT_DEFEND || m_paths[m_currentIndex[client]].flags & WAYPOINT_SENTRY)
				{
					if (m_currentIndex[client] != -1 && m_paths[m_currentIndex[client]].radius > 0.0)
						value += m_paths[m_currentIndex[client]].radius;
					else
						value = 32.0;
				}
				else
				{ 
					if (m_currentIndex[client] != -1 && m_paths[m_currentIndex[client]].radius > 0.0)
						value = m_paths[m_currentIndex[client]].radius;
					value += (speed * GetConVarFloat(EBotFPS));
				}
				
				if (flNodeDist <= Squaredf(value) || GetVectorDistance(flGoPos, flPos, true) <= Squaredf(32.0))
				{
					if (m_paths[m_currentIndex[client]].flags & WAYPOINT_ROCKETJUMP)
					{
						float vec[3];
						float camangle[3];
						float wayorigin[3];
						wayorigin = m_paths[m_currentIndex[client]].origin;
						m_nextStuckCheck[client] = GetGameTime() + 5.0;
						m_currentWaypointIndex[client]--;
						m_targetNode[client]--;
						m_currentIndex[client] = m_pathIndex[client].Get(m_currentWaypointIndex[client]);
						wayorigin[0] += ((wayorigin[0] - m_paths[m_currentIndex[client]].origin[0]) / 3.0);
						wayorigin[1] += ((wayorigin[1] - m_paths[m_currentIndex[client]].origin[1]) / 3.0);
						wayorigin[2] -= GetHeight(client);
						MakeVectorFromPoints(wayorigin, GetEyePosition(client), vec);
						GetVectorAngles(vec, camangle);
						camangle[0] *= -1.0;
						camangle[1] += 180.0;
						ClampAngle(camangle);
						m_lookAt[client] = m_paths[m_currentIndex[client]].origin;
						m_pauseTime[client] = GetGameTime() + 1.0;
						TeleportEntity(client, NULL_VECTOR, camangle, vec);
						m_buttons[client] |= IN_DUCK;
						m_buttons[client] |= IN_JUMP;
						m_buttons[client] |= IN_ATTACK;
					}
					else
					{
						m_nextStuckCheck[client] = GetGameTime() + 5.0;
						m_currentWaypointIndex[client]--;
						m_targetNode[client]--;
					}

					if (m_paths[m_currentIndex[client]].flags & WAYPOINT_JUMP || m_paths[m_currentIndex[client]].flags & WAYPOINT_DOUBLEJUMP)
					{
						m_positions[client].GetArray(m_targetNode[client], flGoPos);
						MoveTo(client, flGoPos, !m_hasWaypoints);
						m_buttons[client] |= IN_JUMP;

						if (m_class[client] == TFClass_Scout)
						{
							DJTime[client] = GetGameTime() + 0.5;
							NoDodge[client] = GetGameTime() + 6.5;
						}
						else
							DJTime[client] = GetGameTime() + 0.05;
					}
					else if (m_paths[m_currentIndex[client]].flags & WAYPOINT_CROUCH)
						CrouchTime[client] = GetGameTime() + 1.0;
				}
			}
			else
			{
				float flNodeDist = GetVectorDistance(flGoPos, flPos, true);
				float value = 32.0 + (GetConVarFloat(EBotFPS) * speed);
				if (flNodeDist <= Squaredf(value))
				{
					m_nextStuckCheck[client] = GetGameTime() + 5.0;
					float jumppos[3];
					CNavArea jump = NavMesh.GetNavArea(jumppos);

					if (jump && jump.HasAttributes(NAV_MESH_JUMP))
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
}

stock void FindPath(int client, float targetposition2[3], int entity = -1, int index = -1)
{
	if (m_spawnTime[client] + 1.0 > GetGameTime())
	{
		SelectObjective(client);
		return;
	}
		
	if (m_hasWaypoints && !m_hasNavpoints)
	{
		if (isPLR || (isPL && m_team[client] == 3) || isCP)
			SelectObjective(client);

		if (index == -1)
			index = FindWaypointFast(targetposition2, client);
			
		AStarFindPath(m_currentIndex[client], index, client, targetposition2);
		return;
	}
	
	float flPos[3];
	GetClientAbsOrigin(client, flPos);
	
	flPos[2] += 15.0;
	targetposition2[2] += 15.0;
	
	CNavArea startArea = NavMesh.GetNearestNavArea(flPos, false, 768.0, false, false, m_team[client]);
	CNavArea goalArea = NavMesh.GetNearestNavArea(targetposition2, false, 768.0, false, false, m_team[client]);
	
	if (!startArea || !goalArea)
		return;
	
	if (startArea == goalArea)
		return;
	
	const float flMaxPathLength = 0.0;
	CNavArea closestArea = NULL_AREA;
	
	bool bBuiltPath = NavMesh.BuildPath(startArea, goalArea, targetposition2, TFPathCost, closestArea, flMaxPathLength, m_team[client], false);
	if (bBuiltPath)
	{
		DeletePathNodes(client);

		CNavArea tempArea = closestArea;
		CNavArea parentArea = tempArea.GetParent();
		NavDirType iNavDirection;
		
		float flCenterPortal[3], flClosestPoint[3];

		m_positions[client].PushArray(targetposition2, 3);
		while (parentArea)
		{
			float flTempAreaCenter[3], flParentAreaCenter[3];
			GetRandomPoint(tempArea, flTempAreaCenter);
			GetRandomPoint(parentArea, flParentAreaCenter);
			
			iNavDirection = ComputeDirection(tempArea, flParentAreaCenter);
			tempArea.ComputePortal(parentArea, iNavDirection, flCenterPortal);
			tempArea.ComputeClosestPointInPortal(parentArea, iNavDirection, flCenterPortal, flClosestPoint);

			AddDirectionVector(flClosestPoint, iNavDirection, GetRandomFloat(6.0, 12.0));
			
			flClosestPoint[2] = tempArea.GetZVector(flClosestPoint);
			m_positions[client].PushArray(flClosestPoint, 3);
			
			tempArea = parentArea;
			parentArea = tempArea.GetParent();
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

stock void AddDirectionVector(float vector[3], const NavDirType dir, const float amount)
{
	switch (dir)
	{
	case NORTH:
		vector[1] -= amount;
	case SOUTH:
		vector[1] += amount;
	case EAST:
		vector[0] += amount;
	case WEST:
		vector[0] -= amount;
	}
}

stock NavDirType ComputeDirection(CNavArea area, const float flPos[3])
{
	float flExtentLow[3]; float flExtentHigh[3];
	area.GetExtent(flExtentLow, flExtentHigh);
	
	if (flPos[0] >= flExtentLow[0] && flPos[0] <= flExtentHigh[0])
	{
		if (flPos[1] < flExtentLow[1])
			return NORTH;
		else if (flPos[1] > flExtentHigh[1])
			return SOUTH;
	}
	else if (flPos[1] >= flExtentLow[1] && flPos[1] <= flExtentHigh[1])
	{
		if (flPos[0] < flExtentLow[0])
			return WEST;
		else if (flPos[0] > flExtentHigh[0])
			return EAST;
	}
	
	float flCenter[3];
	area.GetCenter(flCenter);
	
	float flTo[3];
	SubtractVectors(flPos, flCenter, flTo);
	
	if (FloatAbs(flTo[0]) > FloatAbs(flTo[1]))
	{
		if (flTo[0] > 0.0)
			return EAST;
		
		return WEST;
	}
	else
	{
		if (flTo[1] > 0.0)
			return SOUTH;
		
		return NORTH;
	}
}

public float PathCostFastest(CNavArea area, CNavArea fromArea, CNavLadder ladder, int elevator, float length)
{
	if (fromArea == NULL_AREA)
		return 0.0;
	
	float center[3];
	area.GetCenter(center);

	float fromCenter[3];
	fromArea.GetCenter(fromCenter);

	return GetFastDistance2D(center, fromCenter) + fromArea.GetCostSoFar();
}

public float TFPathCost(CNavArea area, CNavArea fromArea, CNavLadder ladder, int elevator, float length)
{
	if (fromArea == NULL_AREA)
		return 0.0;
	
	float center[3];
	GetRandomPoint(area, center);

	float fromCenter[3];
	GetRandomPoint(fromArea, fromCenter);

	float dist = GetVectorDistance(center, fromCenter, true);
	
	float height = fromArea.ComputeAdjacentConnectionHeightChange(area);
	if (height >= 18.0)
		dist *= (height * 0.18);
	
	int seed = RoundToFloor(GetGameTime() * 0.1) + 1;
	seed *= area.GetID();

	float multiplier = 1.0 + (Cosine(float(seed)) + 1.0) * 10.0;
	float cost = dist * multiplier;

	return cost + fromArea.GetCostSoFar();
}