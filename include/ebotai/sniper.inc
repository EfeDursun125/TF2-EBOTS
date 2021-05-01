int SelectedSniperHint[MAXPLAYERS + 1];

float FindRandomSniperHint[MAXPLAYERS + 1];

stock void SniperLogic(int client)
{	
	if(!HasEnemiesNear[client])
	{
		if(!ClientIsMoving(client))
		{
			if(IsWeaponSlotActive(client, 0) && !TF2_IsPlayerInCondition(client, TFCond_Zoomed))
			{
				ForcePressButton[client] |= IN_ATTACK2;
			}
			else
			{
				EquipWeaponSlot(client, 0);
			}
		}
		else
		{
			if(TF2_IsPlayerInCondition(client, TFCond_Zoomed))
			{
				ForcePressButton[client] |= IN_ATTACK2;
			}
		}
	}
}

stock void SniperMoveLogic(int client)
{
	if(GetUsableSniperHintsCount(client) > 0)
	{
		if (FindRandomSniperHint[client] < GetGameTime())
		{
			SelectedSniperHint[client] = FindRandomSniperSpot(client);
			
			FindRandomSniperHint[client] = GetGameTime() + GetRandomFloat(10.0, 20.0);
		}
		
		if(SelectedSniperHint[client] != -1 && IsValidEntity(SelectedSniperHint[client]) && IsValidEdict(SelectedSniperHint[client]))
		{
			TF2_FindPath(client, GetOrigin(SelectedSniperHint[client]));
			
			return;
		}
		else if(IsSlowThink[client])
		{
			SelectedSniperHint[client] = FindRandomSniperSpot(client);
			
			FindRandomSniperHint[client] = GetGameTime() + GetRandomFloat(10.0, 20.0);
		}
	}
	
	ObjectiveMovement(client);
}

stock int FindRandomSniperSpot(int client)
{
	int iEntity = -1;
	
	int iSelectedHint = -1;
	
	ArrayList SniperHints = new ArrayList();
	
	while((iEntity = FindEntityByClassname(iEntity, "func_tfbot_hint")) != -1)
	{
		if (!IsValidEntity(iEntity))
			continue;
		
		if (!IsValidEdict(iEntity))
			continue;
		
		if (GetEntProp(iEntity, Prop_Data, "m_hint") != 0)
			continue;
		
		if(GetEntProp(iEntity, Prop_Data, "m_isDisabled") == 1)
			continue;
			
		if(GetEnemyTeam(client) == GetTeamNumber(iEntity))
			continue;
		
		SniperHints.Push(iEntity);
	}
	
	if(SniperHints.Length > 0)
	{
		iSelectedHint = SniperHints.Get(GetRandomInt(0, SniperHints.Length - 1));
	}
	
	delete SniperHints;
	
	return iSelectedHint;
}