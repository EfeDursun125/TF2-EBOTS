float SelectedStickyArea[MAXPLAYERS + 1][3];

float DemoAttackTimer[MAXPLAYERS + 1];

stock int DemomanMove(int client)
{
	char currentMap[PLATFORM_MAX_PATH];
	GetCurrentMap(currentMap, sizeof(currentMap));
	
	if(StrContains(currentMap, "ctf_" , false) != -1)
	{
		TF2_FindPath(client, SelectedStickyArea[client]);
		
		return;
	}
	else if(StrContains(currentMap, "pl_" , false) != -1)
	{
		TF2_FindPath(client, SelectedStickyArea[client]);
		
		return;
	}
	
	ObjectiveMovement(client);
}

stock int DemomanAim(int client)
{
	char currentMap[PLATFORM_MAX_PATH];
	GetCurrentMap(currentMap, sizeof(currentMap));
	
	if(StrContains(currentMap, "ctf_" , false) != -1)
	{
		DemomanAimToFlagLogic(client);
		
		return;
	}
	else if(StrContains(currentMap, "pl_" , false) != -1)
	{
		DemomanAimToPayloadLogic(client);
		
		return;
	}
	
	TF2_LookAround(client);
}

stock int DemomanAimToFlagLogic(int client)
{
	int flag;
	while((flag = FindEntityByClassname(flag, "item_teamflag")) != INVALID_ENT_REFERENCE)
	{
		int FlagStatus = GetEntProp(flag, Prop_Send, "m_nFlagStatus");
		if(IsValidEntity(flag) && GetClientTeam(client) == GetTeamNumber(flag))
		{
			if(!IsPointVisible(GetEyePosition(client), GetOrigin(flag)))
			{
				TF2_LookAround(client);
				
				SelectedStickyArea[client] = GetOrigin(flag);
				
				return;
			}
			else if(FlagStatus == 1)
			{
				if(IsWeaponSlotActive(client, 1) && GetPlayerStickyCount(client) < 8)
				{
					if(DemoAttackTimer[client] < GetGameTime())
					{
						ForcePressButton[client] |= IN_ATTACK;
						
						DemoAttackTimer[client] = GetGameTime() + 0.2;
					}
				}
				else if(GetPlayerStickyCount(client) < 8)
				{
					EquipWeaponSlot(client, 1);
				}
				
				TF2_LookAtPos(client, GetOrigin(flag), EBotAimSpeed[client]);
			}
			else if(GetPlayerStickyCount(client) < 8)
			{
				if(IsSlowThink[client])
				{
					NavArea lookarea = TheNavMesh.GetNearestNavArea_Vec(GetOrigin(flag), true, 25000.0, true, true, GetClientTeam(client));
					if(lookarea != NavArea_Null)
					{
						lookarea.GetRandomPoint(SelectedStickyArea[client]);
					}
					
					StopTime[client] = GetGameTime() + GetRandomFloat(2.0, 5.0);
				}
				
				if(IsWeaponSlotActive(client, 1))
				{
					if(DemoAttackTimer[client] < GetGameTime())
					{
						ForcePressButton[client] |= IN_ATTACK;
						
						DemoAttackTimer[client] = GetGameTime() + 0.2;
					}
				}
				else
				{
					EquipWeaponSlot(client, 1);
				}
				
				TF2_LookAtPos(client, SelectedStickyArea[client], EBotAimSpeed[client]);
			}
		}
		else
		{
			TF2_LookAround(client);
		}
	}
}

stock int DemomanAimToPayloadLogic(int client)
{
	int payload;
	if((payload = FindEntityByClassname(payload, "mapobj_cart_dispenser")) != INVALID_ENT_REFERENCE)
	{
		if(IsValidEntity(payload))
		{
			if(!IsPointVisible(GetEyePosition(client), GetOrigin(payload)))
			{
				TF2_LookAround(client);
				
				SelectedStickyArea[client] = GetOrigin(payload);
				
				return;
			}
			else if(GetPlayerStickyCount(client) < 8)
			{
				StopTime[client] = GetGameTime() + GetRandomFloat(2.0, 5.0);
				
				NavArea lookarea = TheNavMesh.GetNearestNavArea_Vec(GetOrigin(payload), true, 25000.0, true, true, GetClientTeam(client));
				if(lookarea != NavArea_Null)
				{
					lookarea.GetRandomPoint(SelectedStickyArea[client]);
				}
				
				if(IsWeaponSlotActive(client, 1))
				{
					if(DemoAttackTimer[client] < GetGameTime())
					{
						ForcePressButton[client] |= IN_ATTACK;
						
						DemoAttackTimer[client] = GetGameTime() + 0.2;
					}
				}
				else
				{
					EquipWeaponSlot(client, 1);
				}
				
				TF2_LookAtPos(client, SelectedStickyArea[client], EBotAimSpeed[client]);
			}
		}
		else
		{
			TF2_LookAround(client);
		}
	}
	else
	{
		TF2_LookAround(client);
	}
}