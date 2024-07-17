stock bool FollowPathRequirements(const int client)
{
    if (!m_positions[client])
        return false;

    if (m_targetNode[client] < 0 || m_targetNode[client] > m_positions[client].Length)
        return false;

	return true;
}

stock void FollowPathStart(const int client)
{

}

stock void FollowPathUpdate(const int client)
{
	if (m_positions[client])
	{
		if (m_targetNode[client] >= 0 && m_targetNode[client] < m_positions[client].Length)
		{
			if (m_hasEnemiesNear[client])
			{
				if (TF2_IsPlayerInCondition(client, TFCond_Zoomed) && (m_hasEnemiesNear[client] || m_hasEntitiesNear[client]))
					m_stopTime[client] = GetGameTime() + 10.0;

				if (SetProcess(client, PRO_ATTACK, 180.0, "", true))
				    return;
			}
			else if (m_goalIndex[client] != -1 && !(m_paths[m_goalIndex[client]].activeArea & currentActiveArea))
			{
				FinishCurrentProcess(client, true);
				return;
			}

			switch (m_class[client])
            {
                case TFClass_Medic:
                {
                    if (m_hasEnemiesNear[client] && m_hasFriendsNear[client] && m_friendDistance[client] < m_enemyDistance[client])
                    {
				        if (SetProcess(client, PRO_HEAL, (m_difficulty[client] + 8.0), "", true))
				            return;
			        }
					else if (!m_hasEnemiesNear[client] && m_hasFriendsNear[client])
					{
						if (SetProcess(client, PRO_HEAL, (m_difficulty[client] + 8.0), "", true))
						    return;
					}
        		}
        		case TFClass_Spy:
        		{
            		RememberedProcess[client] = PRO_SPYLURK;
            		CurrentProcess[client] = PRO_SPYLURK;
            		SpyLurkUpdate(client);
            		return;
        		}
        		case TFClass_Engineer:
        		{
            		RememberedProcess[client] = PRO_ENGINEERIDLE;
            		CurrentProcess[client] = PRO_ENGINEERIDLE;
            		EngineerIdleUpdate(client);
            		return;
        		}
        		default:
        		{
        			if (TF2_IsPlayerInCondition(client, TFCond_Ubercharged))
        			{
						int enemy = FindNearestEnemy(client);
						if (IsValidClient(enemy) && IsPlayerAlive(enemy) && m_lastFailedEntity[client] != enemy)
						{
							m_goalIndex[client] = -1;
							m_goalEntity[client] = enemy;
							m_goalPosition[client] = GetOrigin(enemy);
							if (SetProcess(client, PRO_HUNTENEMY, 15.0, "", true))
								return;
						}
					}
				}
    		}

			FindFriendsAndEnemiens(client);
			CheckHealth(client);
			CheckAmmo(client);
			LookUpdate(client);

    		if (m_stopTime[client] > GetGameTime() && !TF2_IsPlayerInCondition(client, TFCond_OnFire))
				return;

    		if (m_useTeleporter[client] && IsValidEntity(m_teleporterEntity[client]))
			{
				if (GetEntProp(m_teleporterEntity[client], Prop_Send, "m_iState") != 2 && GetEntProp(m_teleporterEntity[client], Prop_Send, "m_iState") != 6)
				{
					m_useTeleporter[client] = false;
					return;
				}

				DeletePathNodes(client);
				m_nextStuckCheck[client] = GetGameTime() + 5.0;
				if (GetVectorDistance(GetOrigin(client), GetOrigin(m_teleporterEntity[client]), true) < Squaredf(24.0))
				{
					m_moveVel[client][0] = 0.0;
					m_moveVel[client][1] = 0.0;
					m_moveVel[client][2] = 0.0;
				}
				else
					MoveToWaypoint(client, GetOrigin(m_teleporterEntity[client]));

				return;
			}

			if (m_hasWaypoints)
				m_currentIndex[client] = m_pathIndex[client].Get(m_currentWaypointIndex[client]);

			static float flGoPos[3];
			m_positions[client].GetArray(m_targetNode[client], flGoPos);

			static float flPos[3]
			GetClientAbsOrigin(client, flPos);

			static float flToPos[3];
			flToPos = flGoPos;
			flToPos[2] += GetHeight(client);

			if (m_nextStuckCheck[client] < GetGameTime())
			{
				MoveTo(client, flGoPos);
				CheckWalls(client);
				m_lastFailedWaypoint[client] = m_currentIndex[client];
				if (m_nextStuckCheck[client] + 3.0 < GetGameTime())
				{
					DeletePathNodes(client);
					m_nextStuckCheck[client] = GetGameTime() + 6.0;
					FinishCurrentProcess(client, true);
				    return;
				}
			}
			else if (m_hasWaypoints && m_goalIndex[client] != -1 && m_currentIndex[client] == m_goalIndex[client] && (m_paths[m_goalIndex[client]].flags & WAYPOINT_SNIPER || m_paths[m_goalIndex[client]].flags & WAYPOINT_DEMOMANCAMP || m_paths[m_goalIndex[client]].flags & WAYPOINT_DEFEND))
			{
				m_nextStuckCheck[client] = GetGameTime() + 6.0;
				MoveToWaypoint(client, flGoPos);
			}
			else
			{
				if (!m_hasWaypoints || (m_goalIndex[client] != -1 && !(m_paths[m_goalIndex[client]].flags & WAYPOINT_SNIPER)))
				{
					if (TF2_IsPlayerInCondition(client, TFCond_Zoomed) && (m_hasEnemiesNear[client] || m_hasEntitiesNear[client]))
						m_stopTime[client] = GetGameTime() + 10.0;
				}

				MoveTo(client, flGoPos);
			}

			flGoPos[2] = flPos[2];

			if (m_hasWaypoints)
			{
				static float flNodeDist;
				flNodeDist = GetVectorDistance(m_paths[m_currentIndex[client]].origin, flPos, true);

				static float value;
				value = 32.0;
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
					value += (GetEntPropFloat(client, Prop_Data, "m_flMaxspeed") * GetConVarFloat(EBotFPS));
				}

				if (flNodeDist < Squaredf(value) || GetVectorDistance(flGoPos, flPos, true) < Squaredf(32.0))
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
						wayorigin[0] += ((wayorigin[0] - m_paths[m_currentIndex[client]].origin[0]) * 0.33);
						wayorigin[1] += ((wayorigin[1] - m_paths[m_currentIndex[client]].origin[1]) * 0.33);
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
						m_nextStuckCheck[client] = GetGameTime() + 6.0;
						m_currentWaypointIndex[client]--;
						m_targetNode[client]--;
					}

					if (m_paths[m_currentIndex[client]].flags & WAYPOINT_JUMP || m_paths[m_currentIndex[client]].flags & WAYPOINT_DOUBLEJUMP)
					{
						if (m_targetNode[client] != -1)
							m_positions[client].GetArray(m_targetNode[client], flGoPos);

						MoveTo(client, flGoPos);

						if (!(m_buttons[client] & IN_JUMP))
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
				static float flNodeDist;
				flNodeDist = GetVectorDistance(flGoPos, flPos, true);
				static float value;
				value = 32.0 + (GetConVarFloat(EBotFPS) * GetEntPropFloat(client, Prop_Data, "m_flMaxspeed"));
				if (flNodeDist < Squaredf(value))
				{
					m_nextStuckCheck[client] = GetGameTime() + 6.0;
					static float jumppos[3];
					static CNavArea jump;
					jump = NavMesh.GetNavArea(jumppos);

					if (jump && jump.HasAttributes(NAV_MESH_JUMP))
						m_buttons[client] |= IN_JUMP;

					m_targetNode[client]--;
				}
			}

			m_pathAhead[client] = flToPos;
		}
		else if (IsOnDefense(client) || IsSniper(client))
			SetProcess(client, PRO_DEFEND, 30.0, "", true, false);
		else
			FinishCurrentProcess(client, true);
	}
	else
		FinishCurrentProcess(client, true);
}

stock void FollowPathEnd(const int client)
{

}
