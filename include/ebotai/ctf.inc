float g_flRedFlagCapPoint[3];
float g_flBluFlagCapPoint[3];

stock int GameModeCTF(int client)
{
	char currentMap[PLATFORM_MAX_PATH];
	GetCurrentMap(currentMap, sizeof(currentMap));
	if(StrContains(currentMap, "ctf_" , false) != -1)
	{
		if (IsSlowThink[client])
		{
			int flag;
			while((flag = FindEntityByClassname(flag, "item_teamflag")) != INVALID_ENT_REFERENCE)
			{
				if(IsValidEntity(flag) && GetClientTeam(client) != GetTeamNumber(flag))
				{
					int FlagStatus = GetEntProp(flag, Prop_Send, "m_nFlagStatus");
					
					if (FlagStatus == 1)
					{
						if (!TF2_HasTheFlag(client))
						{
							int flag2;
							while((flag2 = FindEntityByClassname(flag2, "item_teamflag")) != INVALID_ENT_REFERENCE)
							{
								if(IsValidEntity(flag2) && GetClientTeam(client) == GetTeamNumber(flag2))
								{
									int FlagStatus2 = GetEntProp(flag2, Prop_Send, "m_nFlagStatus");
									
									if (FlagStatus2 == 1) // did someone is picked our flag? chase that guy.
									{
										TF2_FindPath(client, GetOrigin(flag2));
									}
									else // go to nearest enemy
									{
										int Enemy = GetNearestPossibleEnemy(client);
										
										if(Enemy != -1)
										{
											TF2_FindPath(client, GetOrigin(Enemy));
										}
										else if(GetVectorDistance(GetOrigin(client), GetOrigin(flag), true) > 300.0)
										{
											TF2_FindPath(client, GetOrigin(flag));
										}
									}
								}
							}
						}
						else // go to flag capture area
						{
							if(GetClientTeam(client) == 2)
							{
								TF2_FindPath(client, g_flRedFlagCapPoint);
							}
							else
							{
								TF2_FindPath(client, g_flBluFlagCapPoint);
							}
						}
					}
					else
					{
						TF2_FindPath(client, GetOrigin(flag));
					}
				}
			}
		}
	}
}
