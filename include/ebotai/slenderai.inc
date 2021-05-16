bool EnableLook[MAXPLAYERS+1];
bool HasMonstersNear[MAXPLAYERS+1];
int NearestMonster[MAXPLAYERS+1];

stock void SlenderMovement(int client)
{
	if(HasMonstersNear[client])
	{
		FakeClientCommandThrottled(client, "+sprint");
	}
	else
	{
		FakeClientCommandThrottled(client, "-sprint");
	}
	
	int page = FindNearestPage(client);
	
	if(page != -1)
	{
		if(GetVectorDistance(GetEyePosition(client), GetOrigin(page), true) < 300.0)
		{
			TF2_FindPath(client, GetOrigin(page));
		}
		else
		{
			FindNearestPagePack(client);
			GoToNearestPage(client);
		}
		
		return;
	}
	
	int theexit = FindExit(client)
	
	if(theexit != -1)
	{
		TF2_FindPath(client, GetOrigin(theexit));
		
		return;
	}
}

stock void SlenderLook(int client)
{
	int page = FindNearestPage(client);
	
	if(page != -1 && GetVectorDistance(GetOrigin(client), GetOrigin(page), true) < 300.0)
	{
		TF2_LookAtPos(client, GetOrigin(page), EBotAimSpeed[client]);
		EnableLook[client] = false;
		
		return;
	}
	
	EnableLook[client] = true;
}

stock void SlenderAttack(int client)
{
	int page = FindNearestPage(client);
	
	if(page != -1 && GetVectorDistance(GetOrigin(client), GetOrigin(page), true) < 150.0)
	{
		AttackTimer[client] = GetGameTime() + 0.2;
		
		return;
	}
}

public int FindMonsters(int client)
{
	float distance = -1.0;
	int MonstersNearCount = 0;
	
	int iMonster = -1;
	while ((iMonster = FindEntityByClassname(iMonster, "monster*")) != -1)
	{
		if (IsValidEntity(iMonster) && !IsValidEdict(iMonster))
		{
			if(IsPointVisible(GetEyePosition(client), WorldSpaceCenter(iMonster)))
			{
				MonstersNearCount = MonstersNearCount + 1
				
				float edict_distance = GetVectorDistance(GetEyePosition(client), GetOrigin(iMonster), true);
				if((edict_distance < distance) || (distance == -1.0))
				{
					distance = edict_distance;
					NearestMonster[client] = iMonster;
				}
			}
		}
	}
	
	HasMonstersNear[client] = MonstersNearCount > 0;
}