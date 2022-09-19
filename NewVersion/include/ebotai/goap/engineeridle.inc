int SentryGun[MAXPLAYERS + 1];
int Dispenser[MAXPLAYERS + 1];
int TeleporterEnter[MAXPLAYERS + 1];
int TeleporterExit[MAXPLAYERS + 1];

bool SentryHealthIsFull[MAXPLAYERS + 1];
bool SentryIsMaxLevel[MAXPLAYERS + 1];

bool DispenserHealthIsFull[MAXPLAYERS + 1];
bool DispenserIsMaxLevel[MAXPLAYERS + 1];

bool TeleporterEnterHealthIsFull[MAXPLAYERS + 1];
bool TeleporterEnterIsMaxLevel[MAXPLAYERS + 1];

bool TeleporterExitHealthIsFull[MAXPLAYERS + 1];
bool TeleporterExitIsMaxLevel[MAXPLAYERS + 1];

float EngineerMoveSentryTimer[MAXPLAYERS + 1];
bool WantsMoveSentryGun[MAXPLAYERS + 1];
float EngineerMoveDispenserTimer[MAXPLAYERS + 1];
bool WantsMoveDispenser[MAXPLAYERS + 1];

public bool EngineerIdleRequirements(int client)
{
	return true;
}

public void EngineerIdleStart(int client)
{
	// :(
}

public void EngineerIdleUpdate(int client)
{
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

	CheckHideFromDanger(client);
	FindFriendsAndEnemiens(client);
	FindEnemyEntities(client);
	CheckEngineerBuildings(client);
	LookUpdate(client);

	if (TF2_HasTheFlag(client))
	{
		CurrentProcess[client] = PRO_DEFAULT;
		return;
	}
	
	if (!IsValidEntity(SentryGun[client]))
	{
		if (!m_enterFail[client] && WantsBuildTeleporterEnter[client] && GetRandomInt(1, 3) == 1)
		{
			SetProcess(client, PRO_BUILDTELEPORTERENTER, 20.0, "| process started -> trying to build teleporter enter");
			return;
		}

		if (!m_exitFail[client] && WantsBuildTeleporterExit[client] && GetRandomInt(1, 3) == 1)
		{
			SetProcess(client, PRO_BUILDTELEPORTEREXIT, 80.0, "| process started -> trying to build teleporter exit");
			return;
		}
	}

	if (WantsBuildSentryGun[client])
	{
		SetProcess(client, PRO_BUILDSENTRY, 40.0, "| process started -> trying to build sentry gun");
		return;
	}

	if (WantsBuildDispenser[client] && IsValidEntity(SentryGun[client]) && IsVisible(GetEyePosition(client), GetCenter(SentryGun[client])))
	{
		SetProcess(client, PRO_BUILDDISPENSER, 60.0, "| process started -> trying to build dispenser");
		return;
	}

	if (!m_exitFail[client] && WantsBuildTeleporterExit[client])
	{
		SetProcess(client, PRO_BUILDTELEPORTEREXIT, 80.0, "| process started -> trying to build teleporter exit");
		return;
	}

	if (!m_enterFail[client] && WantsBuildTeleporterEnter[client])
	{
		SetProcess(client, PRO_BUILDTELEPORTERENTER, 20.0, "| process started -> trying to build teleporter enter");
		return;
	}

	if (IsWeaponSlotActive(client, 5))
		EquipWeaponSlot(client, GetRandomInt(0, 2));
	else if (IsValidEntity(SentryGun[client]) && (!SentryIsMaxLevel[client] || !SentryHealthIsFull[client]))
	{
		if (GetVectorDistance(GetOrigin(client), GetOrigin(SentryGun[client]), true) <= Squared(128) && IsVisible(GetEyePosition(client), GetCenter(SentryGun[client])))
		{
			if (IsWeaponSlotActive(client, 2))
			{
				MoveTo(client, GetOrigin(SentryGun[client]));
				m_lookAt[client] = GetCenter(SentryGun[client]);
				m_attackTimer[client] = GetGameTime() + 0.4;
			}
			else
				EquipWeaponSlot(client, 2);
		}
		else
			FollowPath(client, GetOrigin(SentryGun[client]));
	}
	else if (IsValidEntity(Dispenser[client]) && (!DispenserIsMaxLevel[client] || !DispenserHealthIsFull[client]))
	{
		if (GetVectorDistance(GetOrigin(client), GetOrigin(Dispenser[client]), true) <= Squared(128) && IsVisible(GetEyePosition(client), GetCenter(Dispenser[client])))
		{
			if (IsWeaponSlotActive(client, 2))
			{
				MoveTo(client, GetOrigin(Dispenser[client]));
				m_lookAt[client] = GetCenter(Dispenser[client]);
				m_attackTimer[client] = GetGameTime() + 0.4;
			}
			else
				EquipWeaponSlot(client, 2);
		}
		else
			FollowPath(client, GetOrigin(Dispenser[client]));
	}
	else if (IsValidEntity(TeleporterEnter[client]) && (!TeleporterEnterHealthIsFull[client] || !TeleporterEnterIsMaxLevel[client]))
	{
		if (GetVectorDistance(GetOrigin(client), GetOrigin(TeleporterEnter[client]), true) <= Squared(128) && IsVisible(GetEyePosition(client), GetCenter(TeleporterEnter[client])))
		{
			if (IsWeaponSlotActive(client, 2))
			{
				if (GetVectorDistance(GetOrigin(client), GetOrigin(TeleporterEnter[client]), true) < Squared(48))
					MoveOut(client, GetOrigin(TeleporterEnter[client]));
				else if (GetVectorDistance(GetOrigin(client), GetOrigin(TeleporterEnter[client]), true) > Squared(56))
					MoveTo(client, GetOrigin(TeleporterEnter[client]));
				
				m_lookAt[client] = GetCenter(TeleporterEnter[client]);
				m_attackTimer[client] = GetGameTime() + 0.4;
			}
			else
				EquipWeaponSlot(client, 2);
		}
		else
			FollowPath(client, GetOrigin(TeleporterEnter[client]));
	}
	else if (WantsMoveDispenser[client] && IsValidEntity(Dispenser[client]))
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
	else if (IsValidEntity(SentryGun[client]))
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
				}
				else
					EquipWeaponSlot(client, 2);
			}
		}
		else
			FollowPath(client, GetOrigin(SentryGun[client]));
	}
	else if (!IsValidEntity(SentryGun[client]))
		SetProcess(client, PRO_BUILDSENTRY, 40.0, "| process started -> trying to build sentry gun");
	else
		SetProcess(client, PRO_DEFAULT, 999999.0, "| process started -> default due we don't have wrench");
}

public void EngineerIdleEnd(int client)
{
	// :(
}

public void CheckEngineerBuildings(int client)
{
	if (!m_isSlowThink[client])
		return;
	
	SentryGun[client] = TF2_GetObject(client, TFObject_Sentry, TFObjectMode_None);
	Dispenser[client] = TF2_GetObject(client, TFObject_Dispenser, TFObjectMode_None);
	TeleporterEnter[client] = TF2_GetObject(client, TFObject_Teleporter, TFObjectMode_Entrance);
	TeleporterExit[client] = TF2_GetObject(client, TFObject_Teleporter, TFObjectMode_Exit);
	
	if (IsValidEntity(SentryGun[client]))
	{
		WantsBuildSentryGun[client] = false;
		int SentryLevel = GetEntProp(SentryGun[client], Prop_Send, "m_iUpgradeLevel");
		int SentryHealth = GetEntProp(SentryGun[client], Prop_Send, "m_iHealth");
		int SentryMaxHealth = GetEntProp(SentryGun[client], Prop_Send, "m_iMaxHealth");

		if (TF2_GetPlayerWeaponID(client, 2) == 142 || !hasWrench(client))
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
	
	if (IsValidEntity(Dispenser[client]))
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
		if (!WantsBuildSentryGun[client] && SentryIsMaxLevel[client] && SentryHealthIsFull[client])
			WantsBuildDispenser[client] = true;
		else
		{
			WantsBuildDispenser[client] = false;
			EngineerMoveDispenserTimer[client] = GetGameTime() + GetRandomFloat(120.0, 180.0);
			WantsMoveDispenser[client] = false;
		}
	}
			
	if (IsValidEntity(TeleporterEnter[client]))
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
	else if (m_hasTeleporterEnterWaypoints && m_hasTeleporterExitWaypoints)
		WantsBuildTeleporterEnter[client] = true;
	else
		WantsBuildTeleporterEnter[client] = false;
	
	if (IsValidEntity(TeleporterExit[client]))
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
	else if (m_hasTeleporterEnterWaypoints && m_hasTeleporterExitWaypoints && !WantsBuildSentryGun[client] && !WantsBuildDispenser[client] && !WantsBuildTeleporterEnter[client] && SentryIsMaxLevel[client] && DispenserIsMaxLevel[client] && DispenserHealthIsFull[client] && SentryHealthIsFull[client])
		WantsBuildTeleporterExit[client] = true;
	else
		WantsBuildTeleporterExit[client] = false;
}
