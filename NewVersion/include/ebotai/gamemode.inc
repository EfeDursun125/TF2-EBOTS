char currentMap[PLATFORM_MAX_PATH];

bool isCTF;
bool isPL;
bool isPLR;
bool isCP;
bool isAD;
bool isKOTH;
bool isArena;
bool isMVM;
bool isDM;
bool isVSH;

stock void AutoLoadGamemode()
{
	isCTF = false;
	isPL = false;
	isPLR = false;
	isCP = false;
	isAD = false;
	isKOTH = false;
	isArena = false;
	isMVM = false;
	isDM = false;
	isVSH = false;

	if (GetConVarInt(FindConVar("tf_gamemode_mvm")) == 1)
		isMVM = true;
	else if (GetConVarInt(FindConVar("tf_gamemode_arena")) == 1)
	{
		if (StrContains(currentMap, "vsh_" , false) != -1)
			isVSH = true;
		else
		{
			isArena = true;
			isKOTH = true;
		}
	}
	else if (GetConVarInt(FindConVar("tf_gamemode_ctf")) == 1)
		isCTF = true;
	else if (GetConVarInt(FindConVar("tf_gamemode_payload")) == 1)
	{
		if (StrContains(currentMap, "plr_" , false) != -1)
			isPLR = true;
		else
			isPL = true;
	}
	else if (GetConVarInt(FindConVar("tf_gamemode_cp")) == 1 || GetConVarInt(FindConVar("tf_gamemode_tc")) == 1)
	{
		if (StrContains(currentMap, "koth_" , false) != -1)
			isKOTH = true;
		else if (GetBluControlPointCount() > 0)
			isCP = true;
		else
			isAD = true;
	}
	else
		isDM = true;
}

stock bool IsOnDefense(const int client)
{
	if (m_team[client] == 2)
	{
		if (isPL)
			return true;
		
		if (isMVM)
			return true;
		
		if (isAD)
			return true;
	}

	return false;
}

stock bool IsOnOffanse(const int client)
{
	if (m_team[client] == 3)
	{
		if (isPL)
			return true;
		
		if (isMVM)
			return true;
		
		if (isAD)
			return true;
	}

	return false;
}