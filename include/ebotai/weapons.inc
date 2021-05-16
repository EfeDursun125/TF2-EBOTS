stock void SelectBestCombatWeapon(int client)
{
	if(IsSlowThink[client] && HasEnemiesNear[client])
	{
		if(!IsValidClient(NearestEnemy[client]))
			return;
		
		if(TF2_GetPlayerClass(client) == TFClass_Sniper)
		{
			if(GetVectorDistance(GetOrigin(client), GetOrigin(NearestEnemy[client]), true) < 128.0)
			{
				EquipWeaponSlot(client, 2);
			}
			else if(GetVectorDistance(GetOrigin(client), GetOrigin(NearestEnemy[client]), true) < 512.0)
			{
				EquipWeaponSlot(client, 1);
			}
			else if(!IsWeaponSlotActive(client, 0))
			{
				EquipWeaponSlot(client, 0);
			}
		}
		else if(TF2_GetPlayerClass(client) == TFClass_Heavy)
		{
			if(GetVectorDistance(GetOrigin(client), GetOrigin(NearestEnemy[client]), true) < 128.0)
			{
				EquipWeaponSlot(client, 2);
			}
			else if(GetVectorDistance(GetOrigin(client), GetOrigin(NearestEnemy[client]), true) < 256.0)
			{
				EquipWeaponSlot(client, 1);
			}
			else if(!IsWeaponSlotActive(client, 0))
			{
				EquipWeaponSlot(client, 0);
			}
		}
		else if(TF2_GetPlayerClass(client) == TFClass_Pyro)
		{
			if(GetVectorDistance(GetOrigin(client), GetOrigin(NearestEnemy[client]), true) < 64.0 && !g_bHealthIsLow[client])
			{
				EquipWeaponSlot(client, 2);
			}
			else if(GetVectorDistance(GetOrigin(client), GetOrigin(NearestEnemy[client]), true) < 400.0)
			{
				EquipWeaponSlot(client, 0);
			}
			else if(!IsWeaponSlotActive(client, 1))
			{
				EquipWeaponSlot(client, 1);
			}
		}
		else if(TF2_GetPlayerClass(client) == TFClass_Soldier)
		{
			if(GetVectorDistance(GetOrigin(client), GetOrigin(NearestEnemy[client]), true) < 128.0)
			{
				EquipWeaponSlot(client, 2);
			}
			else if(GetVectorDistance(GetOrigin(client), GetOrigin(NearestEnemy[client]), true) < 256.0)
			{
				EquipWeaponSlot(client, 1);
			}
			else if(GetVectorDistance(GetOrigin(client), GetOrigin(NearestEnemy[client]), true) < 512.0 && TF2_GetPlayerClass(NearestEnemy[client]) == TFClass_Pyro)
			{
				EquipWeaponSlot(client, 1);
			}
			else if(!IsWeaponSlotActive(client, 0))
			{
				EquipWeaponSlot(client, 0);
			}
		}
		else if(TF2_GetPlayerClass(client) == TFClass_Medic)
		{
			if(GetVectorDistance(GetOrigin(client), GetOrigin(NearestEnemy[client]), true) < 128.0)
			{
				EquipWeaponSlot(client, 2);
			}
			else if(!IsWeaponSlotActive(client, 0))
			{
				EquipWeaponSlot(client, 0);
			}
		}
		else if(TF2_GetPlayerClass(client) == TFClass_DemoMan)
		{
			if(GetVectorDistance(GetOrigin(client), GetOrigin(NearestEnemy[client]), true) < 128.0)
			{
				EquipWeaponSlot(client, 2);
			}
			else if(!IsWeaponSlotActive(client, 0))
			{
				EquipWeaponSlot(client, 0);
			}
		}
		else if(TF2_GetPlayerClass(client) == TFClass_Scout)
		{
			if(GetVectorDistance(GetOrigin(client), GetOrigin(NearestEnemy[client]), true) > 1024.0)
			{
				EquipWeaponSlot(client, 1);
			}
			else if(GetVectorDistance(GetOrigin(client), GetOrigin(NearestEnemy[client]), true) < 128.0)
			{
				EquipWeaponSlot(client, 2);
			}
			else
			{
				EquipWeaponSlot(client, 0);
			}
		}
		else if(TF2_GetPlayerClass(client) == TFClass_Engineer && IsClientAimingToMe(NearestEnemy[client], client))
		{
			if(GetVectorDistance(GetOrigin(client), GetOrigin(NearestEnemy[client]), true) > 1024.0)
			{
				EquipWeaponSlot(client, 1);
			}
			else if(GetVectorDistance(GetOrigin(client), GetOrigin(NearestEnemy[client]), true) < 128.0)
			{
				EquipWeaponSlot(client, 2);
			}
			else
			{
				EquipWeaponSlot(client, 0);
			}
		}
		else if(TF2_GetPlayerClass(client) == TFClass_Spy)
		{
			if(GetVectorDistance(GetOrigin(client), GetOrigin(NearestEnemy[client]), true) < 300.0)
			{
				EquipWeaponSlot(client, 2);
			}
			else
			{
				EquipWeaponSlot(client, 0);
			}
		}
	}
}

stock void SelectBestCombatWeaponForVector(int client, float vector[3])
{
	if(IsSlowThink[client])
	{
		if(TF2_GetPlayerClass(client) == TFClass_Sniper)
		{
			if(GetVectorDistance(GetOrigin(client), vector, true) > 1024.0 && !IsWeaponSlotActive(client, 0))
			{
				EquipWeaponSlot(client, 0);
			}
			else if(GetVectorDistance(GetOrigin(client), vector, true) < 256.0 && !IsWeaponSlotActive(client, 2))
			{
				EquipWeaponSlot(client, 2);
			}
			else if(GetVectorDistance(GetOrigin(client), vector, true) > 256.0 && !IsWeaponSlotActive(client, 1) && !TF2_IsPlayerInCondition(client, TFCond_Zoomed))
			{
				EquipWeaponSlot(client, 1);
			}
		}
		else if(TF2_GetPlayerClass(client) == TFClass_Heavy)
		{
			if(GetVectorDistance(GetOrigin(client), vector, true) < 128.0 && !IsWeaponSlotActive(client, 2))
			{
				EquipWeaponSlot(client, 2);
			}
			else if(GetVectorDistance(GetOrigin(client), vector, true) < 256.0 && !IsWeaponSlotActive(client, 1))
			{
				EquipWeaponSlot(client, 1);
			}
			else if(!IsWeaponSlotActive(client, 0))
			{
				EquipWeaponSlot(client, 0);
			}
		}
		else if(TF2_GetPlayerClass(client) == TFClass_Pyro)
		{
			if(GetVectorDistance(GetOrigin(client), vector, true) < 64.0 && !IsWeaponSlotActive(client, 2) && g_bHealthIsLow[client])
			{
				EquipWeaponSlot(client, 2);
			}
			else if(GetVectorDistance(GetOrigin(client), vector, true) < 400.0 && !IsWeaponSlotActive(client, 0))
			{
				EquipWeaponSlot(client, 0);
			}
			else if(!IsWeaponSlotActive(client, 1))
			{
				EquipWeaponSlot(client, 1);
			}
		}
		else if(TF2_GetPlayerClass(client) == TFClass_Soldier)
		{
			if(GetVectorDistance(GetOrigin(client), vector, true) < 128.0 && !IsWeaponSlotActive(client, 2))
			{
				EquipWeaponSlot(client, 2);
			}
			else if(GetVectorDistance(GetOrigin(client), vector, true) < 256.0 && !IsWeaponSlotActive(client, 1))
			{
				EquipWeaponSlot(client, 1);
			}
			else if(!IsWeaponSlotActive(client, 0))
			{
				EquipWeaponSlot(client, 0);
			}
		}
		else if(TF2_GetPlayerClass(client) == TFClass_Medic)
		{
			if(GetVectorDistance(GetOrigin(client), vector, true) < 128.0 && !IsWeaponSlotActive(client, 2))
			{
				EquipWeaponSlot(client, 2);
			}
			else if(!IsWeaponSlotActive(client, 0))
			{
				EquipWeaponSlot(client, 0);
			}
		}
		else if(TF2_GetPlayerClass(client) == TFClass_DemoMan)
		{
			if(GetVectorDistance(GetOrigin(client), vector, true) < 128.0 && !IsWeaponSlotActive(client, 2))
			{
				EquipWeaponSlot(client, 2);
			}
			else if(!IsWeaponSlotActive(client, 0))
			{
				EquipWeaponSlot(client, 0);
			}
		}
		else if(TF2_GetPlayerClass(client) == TFClass_Scout)
		{
			if(GetVectorDistance(GetOrigin(client), vector, true) > 1024.0 && !IsWeaponSlotActive(client, 1))
			{
				EquipWeaponSlot(client, 1);
			}
			else if(GetVectorDistance(GetOrigin(client), vector, true) < 128.0 && !IsWeaponSlotActive(client, 2))
			{
				EquipWeaponSlot(client, 2);
			}
			else if(GetVectorDistance(GetOrigin(client), vector, true) > 128.0 && !IsWeaponSlotActive(client, 0))
			{
				EquipWeaponSlot(client, 0);
			}
		}
		else if(TF2_GetPlayerClass(client) == TFClass_Engineer)
		{
			if(GetVectorDistance(GetOrigin(client), vector, true) > 1024.0 && !IsWeaponSlotActive(client, 1))
			{
				EquipWeaponSlot(client, 1);
			}
			else if(GetVectorDistance(GetOrigin(client), vector, true) < 128.0 && !IsWeaponSlotActive(client, 2))
			{
				EquipWeaponSlot(client, 2);
			}
			else if(GetVectorDistance(GetOrigin(client), vector, true) > 128.0 && !IsWeaponSlotActive(client, 0))
			{
				EquipWeaponSlot(client, 0);
			}
		}
		else if(TF2_GetPlayerClass(client) == TFClass_Spy)
		{
			if(GetVectorDistance(GetOrigin(client), vector, true) < 128.0 && !IsWeaponSlotActive(client, 2))
			{
				EquipWeaponSlot(client, 2);
			}
			else if(GetVectorDistance(GetOrigin(client), vector, true) < 512.0 && !IsWeaponSlotActive(client, 2) && TF2_IsPlayerInCondition(client, TFCond_Disguised))
			{
				EquipWeaponSlot(client, 2);
			}
			else if(GetVectorDistance(GetOrigin(client), vector, true) > 128.0 && !IsWeaponSlotActive(client, 0))
			{
				EquipWeaponSlot(client, 0);
			}
		}
	}
}