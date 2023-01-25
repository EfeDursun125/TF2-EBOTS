public bool BuildDispenserRequirements(int client)
{
	if (!IsValidEntity(SentryGun[client]))
		return false;
		
	if (!SentryHealthIsFull[client])
		return false;

	if (GetMetal(client) < 100.0)
		return false;

	if (IsMoving(client))
		return false;

	return true;
}

public void BuildDispenserStart(int client)
{
	// :(
}

public void BuildDispenserUpdate(int client)
{
	CheckHealth(client);
	FindFriendsAndEnemiens(client);
	FindEnemyEntities(client);
	LookUpdate(client);

	if (!IsWeaponSlotActive(client, 5))
		FakeClientCommandThrottled(client, "build 0 0");
	else
		m_buttons[client] |= IN_ATTACK;
	
	int dispenser = TF2_GetObject(client, TFObject_Dispenser, TFObjectMode_None);
	if (dispenser != -1)
	{
		FinishCurrentProcess(client);
		EngineerMoveDispenserTimer[client] = GetGameTime() + GetRandomFloat(120.0, 180.0);
		WantsBuildDispenser[client] = false;
		WantsMoveDispenser[client] = false;
	}
}

public void BuildDispenserEnd(int client)
{
	// :(
}