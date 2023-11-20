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

	if (StrContains(currentMap, "arena_" , false) != -1)
	{
		isArena = true;
		isKOTH = true;
	}
	else if (StrContains(currentMap, "koth_" , false) != -1)
		isKOTH = true;
	else if (StrContains(currentMap, "pl_" , false) != -1)
		isPL = true;
	else if (StrContains(currentMap, "plr_" , false) != -1)
		isPLR = true;
	else if (StrContains(currentMap, "ctf_" , false) != -1)
		isCTF = true;
	else if (StrContains(currentMap, "mvm_" , false) != -1)
		isMVM = true;
	else if (StrContains(currentMap, "cp_" , false) != -1 || StrContains(currentMap, "tc_" , false) != -1)
	{
		if (GetBluControlPointCount() > 0)
			isCP = true;
		else
			isAD = true;
	}
	else if (StrContains(currentMap, "vsh_" , false) != -1)
		isVSH = true;
	else
		isDM = true;
}

stock bool IsOnDefanse(const int client)
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