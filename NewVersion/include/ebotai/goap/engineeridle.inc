int SentryGun[MAXPLAYERS + 1] = -1;
int Dispenser[MAXPLAYERS + 1] = -1;
int TeleporterEnter[MAXPLAYERS + 1] = -1;
int TeleporterExit[MAXPLAYERS + 1] = -1;

bool SentryHealthIsFull[MAXPLAYERS + 1];
bool SentryIsMaxLevel[MAXPLAYERS + 1];

bool DispenserHealthIsFull[MAXPLAYERS + 1];
bool DispenserIsMaxLevel[MAXPLAYERS + 1];

bool TeleporterEnterHealthIsFull[MAXPLAYERS + 1];
bool TeleporterEnterIsMaxLevel[MAXPLAYERS + 1];

bool TeleporterExitHealthIsFull[MAXPLAYERS + 1];
bool TeleporterExitIsMaxLevel[MAXPLAYERS + 1];

float EngineerMoveSentryTimer[MAXPLAYERS + 1] = 0.0;
bool WantsMoveSentryGun[MAXPLAYERS + 1];
float EngineerMoveDispenserTimer[MAXPLAYERS + 1] = 0.0;
bool WantsMoveDispenser[MAXPLAYERS + 1];

public bool EngineerIdleRequirements(int client)
{
	if (TF2_HasTheFlag(client))
		return false;
	return true;
}

public void EngineerIdleStart(int client)
{
	// :(
}

public void EngineerIdleUpdate(int client)
{
	if (WantsBuildSentryGun[client])
	{
		SetProcess(client, PRO_BUILDSENTRY, 60.0, "| process started -> trying to build sentry gun");
		return;
	}

	if (WantsBuildDispenser[client] && IsVisible(GetEyePosition(client), GetCenter(SentryGun[client])))
	{
		SetProcess(client, PRO_BUILDDISPENSER, 60.0, "| process started -> trying to build dispenser");
		return;
	}

	/*if (WantsBuildTeleporter[client])
	{
		SetProcess(client, PRO_BUILDTELEPORTERENTER, 60.0, "| process started -> trying to build teleporter enter");
		return;
	}

	if (WantsBuildTeleporterExit[client])
	{
		SetProcess(client, PRO_BUILDTELEPORTEREXIT, 60.0, "| process started -> trying to build teleporter exit");
		return;
	}*/

	FindFriendsAndEnemiens(client);
	FindEnemyEntities(client);
	CheckEngineerBuildings(client);
	
	bool lookupdate = true;

	if (SentryGun[client] != -1 && (!SentryIsMaxLevel[client] || !SentryHealthIsFull[client]))
	{
		if (GetVectorDistance(GetOrigin(client), GetOrigin(SentryGun[client]), true) <= Squared(128) && IsVisible(GetEyePosition(client), GetCenter(SentryGun[client])))
		{
			if (IsWeaponSlotActive(client, 2))
			{
				MoveTo(client, GetOrigin(SentryGun[client]));
				m_lookAt[client] = GetCenter(SentryGun[client]);
				m_attackTimer[client] = GetGameTime() + 0.4;
				lookupdate = false;
			}
			else
				EquipWeaponSlot(client, 2);
		}
		else
			FollowPath(client, GetOrigin(SentryGun[client]));
	}
	else if (Dispenser[client] != -1 && (!DispenserIsMaxLevel[client] || !DispenserHealthIsFull[client]))
	{
		if (GetVectorDistance(GetOrigin(client), GetOrigin(Dispenser[client]), true) <= Squared(128) && IsVisible(GetEyePosition(client), GetCenter(Dispenser[client])))
		{
			if (IsWeaponSlotActive(client, 2))
			{
				MoveTo(client, GetOrigin(Dispenser[client]));
				m_lookAt[client] = GetCenter(Dispenser[client]);
				m_attackTimer[client] = GetGameTime() + 0.4;
				lookupdate = false;
			}
			else
				EquipWeaponSlot(client, 2);
		}
		else
			FollowPath(client, GetOrigin(Dispenser[client]));
	}
	else if (TeleporterEnter[client] != -1 && (!TeleporterEnterHealthIsFull[client] || !TeleporterEnterIsMaxLevel[client]))
	{
		if (GetVectorDistance(GetOrigin(client), GetOrigin(TeleporterEnter[client]), true) <= Squared(128) && IsVisible(GetEyePosition(client), GetCenter(TeleporterEnter[client])))
		{
			if (IsWeaponSlotActive(client, 2))
			{
				MoveTo(client, GetOrigin(TeleporterEnter[client]));
				m_lookAt[client] = GetCenter(TeleporterEnter[client]);
				m_attackTimer[client] = GetGameTime() + 0.4;
				lookupdate = false;
			}
			else
				EquipWeaponSlot(client, 2);
		}
		else
			FollowPath(client, GetOrigin(TeleporterEnter[client]));
	}
	else if (WantsMoveDispenser[client] && Dispenser[client] != -1)
	{
		if (GetVectorDistance(GetOrigin(client), GetOrigin(Dispenser[client]), true) <= Squared(128) && IsVisible(GetEyePosition(client), GetCenter(Dispenser[client])))
		{
			MoveTo(client, GetOrigin(Dispenser[client]));
			m_lookAt[client] = GetCenter(Dispenser[client]);
			m_buttons[client] |= IN_ATTACK2;
			if (GetEntProp(client, Prop_Send, "m_bCarryingObject") && IsValidEntity(GetEntPropEnt(client, Prop_Send, "m_hCarriedObject")))
			{
				SetProcess(client, PRO_BUILDDISPENSER, 60.0, "| process started -> wants to move dispenser");
				return;
			}
		}
		else
			FollowPath(client, GetOrigin(Dispenser[client]));
	}
	else if (SentryGun[client] != -1)
	{
		if (GetVectorDistance(GetOrigin(client), GetOrigin(SentryGun[client]), true) <= Squared(128) && IsVisible(GetEyePosition(client), GetCenter(SentryGun[client])))
		{
			if (WantsMoveSentryGun[client])
			{
				MoveTo(client, GetOrigin(SentryGun[client]));
				m_lookAt[client] = GetCenter(SentryGun[client]);
				m_buttons[client] |= IN_ATTACK2;
				if (GetEntProp(client, Prop_Send, "m_bCarryingObject") && IsValidEntity(GetEntPropEnt(client, Prop_Send, "m_hCarriedObject")))
				{
					SetProcess(client, PRO_BUILDSENTRY, 60.0, "| process started -> wants to move sentry gun");
					return;
				}
			}
			else
			{
				if (IsWeaponSlotActive(client, 2))
				{
					MoveTo(client, GetOrigin(SentryGun[client]));
					m_lookAt[client] = GetCenter(SentryGun[client]);
					m_attackTimer[client] = GetGameTime() + 0.4;
					lookupdate = false;
				}
				else
					EquipWeaponSlot(client, 2);
			}
		}
		else
			FollowPath(client, GetOrigin(SentryGun[client]));
	}

	CheckHealth(client);

	if (IsMetalLow(client))
		m_lowAmmo[client] = true;
	else
		m_lowAmmo[client] = false;
	
	if (m_lowAmmo[client])
	{
		RememberedProcess[client] = PRO_ENGINEERIDLE;
		SetProcess(client, PRO_GETAMMO, 90.0, "| process started -> need ammo pack.");
	}
	
	if (lookupdate)
		LookUpdate(client);
}

public void EngineerIdleEnd(int client)
{
	// :(
}

public void CheckEngineerBuildings(int client)
{
	if (m_isSlowThink[client])
		return;
	
	if (!IsMoving(client) && IsWeaponSlotActive(client, 5)) // if we're stuck, sadly put building here
		m_buttons[client] |= IN_ATTACK;
	else if (!IsMoving(client) && WantsBuildSentryGun[client])
		FakeClientCommandThrottled(client, "build 2 0");
	else if (!IsMoving(client) && WantsBuildDispenser[client])
		FakeClientCommandThrottled(client, "build 0 0");
	
	SentryGun[client] = TF2_GetObject(client, TFObject_Sentry, TFObjectMode_None);
	Dispenser[client] = TF2_GetObject(client, TFObject_Dispenser, TFObjectMode_None);
	TeleporterEnter[client] = TF2_GetObject(client, TFObject_Teleporter, TFObjectMode_Entrance);
	TeleporterExit[client] = TF2_GetObject(client, TFObject_Teleporter, TFObjectMode_Exit);
	
	if (SentryGun[client] != -1)
	{
		WantsBuildSentryGun[client] = false;
		int SentryLevel = GetEntProp(SentryGun[client], Prop_Send, "m_iUpgradeLevel");
		int SentryHealth = GetEntProp(SentryGun[client], Prop_Send, "m_iHealth");
		int SentryMaxHealth = GetEntProp(SentryGun[client], Prop_Send, "m_iMaxHealth");

		if (TF2_GetPlayerWeaponID(client, 2) == 142)
			SentryIsMaxLevel[client] = true;
		else
		{
			if (SentryLevel < 3)
				SentryIsMaxLevel[client] = false;
			else
				SentryIsMaxLevel[client] = true;
		}
				
		if (SentryHealth < SentryMaxHealth)
			SentryHealthIsFull[client] = false;
		else
			SentryHealthIsFull[client] = true;
		
		if (EngineerMoveSentryTimer[client] < GetGameTime())
			WantsMoveSentryGun[client] = true;
	}
	else
	{
		WantsBuildSentryGun[client] = true;
		EngineerMoveSentryTimer[client] = GetGameTime() + GetRandomFloat(90.0, 160.0);
		WantsMoveSentryGun[client] = false;
	}
	
	if (Dispenser[client] != -1)
	{
		WantsBuildDispenser[client] = false;
		int DispenserLevel = GetEntProp(Dispenser[client], Prop_Send, "m_iUpgradeLevel");
		int DispenserHealth = GetEntProp(Dispenser[client], Prop_Send, "m_iHealth");
		int DispenserMaxHealth = GetEntProp(Dispenser[client], Prop_Send, "m_iMaxHealth");

		if (DispenserLevel < 3)
			DispenserIsMaxLevel[client] = false;
		else
			DispenserIsMaxLevel[client] = true;
		
		if (DispenserHealth < DispenserMaxHealth)
			DispenserHealthIsFull[client] = false;
		else
			DispenserHealthIsFull[client] = true;
	}
	else
	{
		if (!WantsBuildSentryGun[client] && SentryHealthIsFull[client] && SentryIsMaxLevel[client])
			WantsBuildDispenser[client] = true;
		else
		{
			WantsBuildDispenser[client] = false;
			EngineerMoveDispenserTimer[client] = GetGameTime() + GetRandomFloat(120.0, 180.0);
			WantsMoveDispenser[client] = false;
		}
	}
			
	if (TeleporterEnter[client] != -1)
	{
		WantsBuildTeleporterEnter[client] = false;
		int TeleporterLevel = GetEntProp(TeleporterEnter[client], Prop_Send, "m_iUpgradeLevel");
		int TeleporterHealth = GetEntProp(TeleporterEnter[client], Prop_Send, "m_iHealth");
		int TeleporterMaxHealth = GetEntProp(TeleporterEnter[client], Prop_Send, "m_iMaxHealth");

		if (TeleporterLevel < 3)
			TeleporterEnterIsMaxLevel[client] = false;
		else
			TeleporterEnterIsMaxLevel[client] = true;
				
		if (TeleporterHealth < TeleporterMaxHealth)
			TeleporterEnterHealthIsFull[client] = false;
		else
			TeleporterEnterHealthIsFull[client] = true;
	}
	else if (m_hasTeleporterEnterWaypoints && m_hasTeleporterExitWaypoints && !WantsBuildSentryGun[client] && SentryHealthIsFull[client] && SentryIsMaxLevel[client] && !WantsBuildDispenser[client] && DispenserHealthIsFull[client] && DispenserIsMaxLevel[client])
		WantsBuildTeleporterEnter[client] = true;
	else
		WantsBuildTeleporterEnter[client] = false;
	
	if (TeleporterExit[client] != -1)
	{
		WantsBuildTeleporterExit[client] = false;
		int TeleporterLevel = GetEntProp(TeleporterExit[client], Prop_Send, "m_iUpgradeLevel");
		int TeleporterHealth = GetEntProp(TeleporterExit[client], Prop_Send, "m_iHealth");
		int TeleporterMaxHealth = GetEntProp(TeleporterExit[client], Prop_Send, "m_iMaxHealth");

		if (TeleporterLevel < 3)
			TeleporterExitIsMaxLevel[client] = false;
		else
			TeleporterExitIsMaxLevel[client] = true;
				
		if (TeleporterHealth < TeleporterMaxHealth)
			TeleporterExitHealthIsFull[client] = false;
		else
			TeleporterExitHealthIsFull[client] = true;
	}
	else if (m_hasTeleporterEnterWaypoints && m_hasTeleporterExitWaypoints && !WantsBuildSentryGun[client] && SentryHealthIsFull[client] && SentryIsMaxLevel[client] && !WantsBuildDispenser[client] && DispenserHealthIsFull[client] && DispenserIsMaxLevel[client] && !WantsBuildTeleporterEnter[client] && TeleporterEnterHealthIsFull[client])
		WantsBuildTeleporterExit[client] = true;
	else
		WantsBuildTeleporterExit[client] = false;
}