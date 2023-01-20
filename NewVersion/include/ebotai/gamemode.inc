char currentMap[PLATFORM_MAX_PATH];

bool isCTF;
bool isPL;
bool isPLR;
bool isCP;
bool is5CP;
bool isAD;
bool isKOTH;
bool isMVM;
bool isDM;

stock void AutoLoadGamemode()
{
	isCTF = false;
	isPL = false;
	isPLR = false;
	isCP = false;
	is5CP = false;
	isAD = false;
	isKOTH = false;
	isMVM = false;
	isDM = false;

	if (StrContains(currentMap, "koth_" , false) != -1 || StrContains(currentMap, "arena_" , false) != -1)
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
		isCP = true;
		isAD = true;
	}
}