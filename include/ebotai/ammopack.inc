float g_flFindNearestAmmoTimer[MAXPLAYERS + 1];

float g_flNearestAmmoOrigin[MAXPLAYERS + 1][3];

stock int FindNearestAmmoPack(int client)
{
	if(g_flFindNearestAmmoTimer[client] < GetGameTime())
	{
		int ammopack = FindNearestAmmo(client);
		
		if (ammopack != -1 && PF_Exists(client) && PF_IsPathToVectorPossible(client, GetOrigin(ammopack)))
		{
			g_flNearestAmmoOrigin[client] = GetOrigin(ammopack);
		}
		
		g_flFindNearestAmmoTimer[client] = GetGameTime() + 40.0;
	}
	else if(GetVectorDistance(GetOrigin(client), g_flNearestAmmoOrigin[client]) < 21.0)
	{
		int ammopack = FindNearestAmmo(client);
		
		if (ammopack != -1 && PF_Exists(client) && PF_IsPathToVectorPossible(client, GetOrigin(ammopack)))
		{
			g_flNearestAmmoOrigin[client] = GetOrigin(ammopack);
		}
		
		g_flFindNearestAmmoTimer[client] = GetGameTime() + 40.0;
	}
}

stock int GoToAmmoPack(int client)
{
	TF2_FindPath(client, g_flNearestAmmoOrigin[client]);
}

public int FindNearestAmmo(int client)
{
	char ClassName[32];
	float clientOrigin[3];
	float entityOrigin[3];
	float distance = -1.0;
	int nearestEntity = -1;
	for(int x = 0; x <= GetMaxEntities(); x++)
	{
		if(IsValidEdict(x) && IsValidEntity(x))
		{
			GetEdictClassname(x, ClassName, 32);
			
			if(StrContains(ClassName, "prop_dynamic", false) == -1 && !HasEntProp(x, Prop_Send, "m_fEffects"))
				continue;
			
			if(StrContains(ClassName, "prop_dynamic", false) == -1 && GetEntProp(x, Prop_Send, "m_fEffects") != 0)
				continue;
			
			if(StrContains(ClassName, "item_ammo", false) != -1 || StrContains(ClassName, "tf_ammo_pack", false) != -1 || StrContains(ClassName, "obj_dispenser", false) != -1 || StrContains(ClassName, "func_regen", false) != -1 || StrContains(ClassName, "pd_dispenser", false) != -1 || StrContains(ClassName, "rd_robot_dispenser", false) != -1 || StrContains(ClassName, "item_healthammokit", false) != -1)
			{
				entityOrigin = GetOrigin(x);
				GetClientEyePosition(client, clientOrigin);
				
				float edict_distance = GetVectorDistance(clientOrigin, entityOrigin);
				if((edict_distance < distance) || (distance == -1.0))
				{
					distance = edict_distance;
					nearestEntity = x;
				}
			}
		}
	}
	
	return nearestEntity;
}
