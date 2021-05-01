
float g_flFindNearestPageTimer[MAXPLAYERS + 1];

float g_flNearestPageOrigin[MAXPLAYERS + 1][3];

stock int FindNearestPagePack(int client)
{
	if(g_flFindNearestPageTimer[client] < GetGameTime())
	{
		int page = FindRandomPage(client);
		
		if (page != -1)
		{
			GetEntPropVector(page, Prop_Send, "m_vecOrigin", g_flNearestPageOrigin[client]);
			
			g_flFindNearestPageTimer[client] = GetGameTime() + 40.0;
		}
	}
	else if(GetVectorDistance(GetOrigin(client), g_flNearestPageOrigin[client]) < 100.0)
	{
		int page = FindRandomPage(client);
		
		if (page != -1)
		{
			GetEntPropVector(page, Prop_Send, "m_vecOrigin", g_flNearestPageOrigin[client]);
			
			g_flFindNearestPageTimer[client] = GetGameTime() + 20.0;
		}
	}
}

stock int GoToNearestPage(int client)
{
	TF2_FindPath(client, g_flNearestPageOrigin[client]);
}

stock int FindRandomPage(int client)
{
	int iSelectedPage = -1;
	ArrayList AllPages = new ArrayList();
	char strName[64];
	char ClassName[64];
	
	for(new x = 0; x <= GetMaxEntities(); x++)
	{
		if (!IsValidEntity(x))
			continue;
		
		if (!IsValidEdict(x))
			continue;
		
		GetEdictClassname(x, ClassName, 64);
		GetEntPropString(x, Prop_Data, "m_iName", strName, sizeof(strName));
		
		if(StrContains(ClassName, "prop_dynamic", false) == -1 && StrContains(strName, "sf2_page", false) == -1 && strcmp(strName, "sf2_page_model", false) == 0)
			continue;
		
		AllPages.Push(x);
	}
	
	if(AllPages.Length > 0)
	{
		iSelectedPage = AllPages.Get(GetRandomInt(0, AllPages.Length - 1));
	}
	
	delete AllPages;
	
	return iSelectedPage;
}

public int FindNearestPage(int client)
{
	char strName[64];
	char ClassName[64];
	float distance = -1.0;
	int nearestEntity = -1;
	
	for(int x = 0; x <= GetMaxEntities(); x++)
	{
		if(IsValidEdict(x) && IsValidEntity(x))
		{
			GetEdictClassname(x, ClassName, 64);
			GetEntPropString(x, Prop_Data, "m_iName", strName, sizeof(strName));
			
			if(StrContains(ClassName, "prop_dynamic", false) != -1 && StrContains(strName, "sf2_page", false) != -1 && strcmp(strName, "sf2_page_model", false) != 0)
			{
				float edict_distance = GetVectorDistance(GetOrigin(client), GetOrigin(x));
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

public int FindExit(int client)
{
	char strName[64];
	char ClassName[64];
	float distance = -1.0;
	int nearestEntity = -1;
	
	for(int x = 0; x <= GetMaxEntities(); x++)
	{
		if(IsValidEdict(x) && IsValidEntity(x))
		{
			GetEdictClassname(x, ClassName, 64);
			GetEntPropString(x, Prop_Data, "m_iName", strName, sizeof(strName));
			
			if(StrContains(ClassName, "trigger_multiple", false) != -1 && StrContains(strName, "sf2_escape_trigger", false) != -1)
			{
				float edict_distance = GetVectorDistance(GetOrigin(client), GetOrigin(x));
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
