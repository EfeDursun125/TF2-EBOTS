int TeleporterEnter[TFMaxPlayers];
int TeleporterExit[TFMaxPlayers];

bool SentryHealthIsFull[TFMaxPlayers];
bool SentryIsMaxLevel[TFMaxPlayers];

bool DispenserHealthIsFull[TFMaxPlayers];
bool DispenserIsMaxLevel[TFMaxPlayers];

bool TeleporterEnterHealthIsFull[TFMaxPlayers];
bool TeleporterEnterIsMaxLevel[TFMaxPlayers];

bool TeleporterExitHealthIsFull[TFMaxPlayers];
bool TeleporterExitIsMaxLevel[TFMaxPlayers];

float EngineerMoveSentryTimer[TFMaxPlayers];
bool WantsMoveSentryGun[TFMaxPlayers];
float EngineerMoveDispenserTimer[TFMaxPlayers];
bool WantsMoveDispenser[TFMaxPlayers];

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
	m_eBotDodgeRange[client] = 512;
	
	if (TF2_HasTheFlag(client))
	{
		CurrentProcess[client] = PRO_DEFAULT;
		return;
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

	CheckHideFromDanger(client);
	FindFriendsAndEnemiens(client);
	FindEnemyEntities(client);
	CheckEngineerBuildings(client);
	LookUpdate(client);

	if (m_useTeleporter[client] && IsValidEntity(m_teleporterEntity[client]))
	{
		DeletePathNodes(client);
		m_nextStuckCheck[client] = GetGameTime() + 5.0;
		if (GetVectorDistance(GetOrigin(client), GetOrigin(m_teleporterEntity[client]), true) <= Squaredf(24.0))
		{
			m_moveVel[client][0] = 0.0;
			m_moveVel[client][1] = 0.0;
			m_moveVel[client][2] = 0.0;
		}
		else
			MoveToWaypoint(client, GetOrigin(m_teleporterEntity[client]), GetOrigin(m_teleporterEntity[client]));
		return;
	}
	
	if (!IsValidEntity(SentryGun[client]))
	{
		if (WantsBuildTeleporterEnter[client] && GetRandomInt(1, 3) == 1)
			SetProcess(client, PRO_BUILDTELEPORTERENTER, 20.0, "| process started -> trying to build teleporter enter");

		if (WantsBuildTeleporterExit[client] && IsValidEntity(TeleporterEnter[client]))
			SetProcess(client, PRO_BUILDTELEPORTEREXIT, 80.0, "| process started -> trying to build teleporter exit");
	}

	if (WantsBuildSentryGun[client])
		SetProcess(client, PRO_BUILDSENTRY, 40.0, "| process started -> trying to build sentry gun");

	if (WantsBuildDispenser[client])
		SetProcess(client, PRO_BUILDDISPENSER, 60.0, "| process started -> trying to build dispenser");

	if (WantsBuildTeleporterExit[client])
		SetProcess(client, PRO_BUILDTELEPORTEREXIT, 80.0, "| process started -> trying to build teleporter exit");

	if (WantsBuildTeleporterEnter[client])
		SetProcess(client, PRO_BUILDTELEPORTERENTER, 20.0, "| process started -> trying to build teleporter enter");

	if (IsValidEntity(SentryGun[client]) && (!SentryIsMaxLevel[client] || !SentryHealthIsFull[client]))
	{
		if (GetVectorDistance(GetOrigin(client), GetOrigin(SentryGun[client]), true) <= Squaredf(200.0) && IsVisible(GetEyePosition(client), GetCenter(SentryGun[client])))
		{
			if (IsWeaponSlotActive(client, 2))
			{
				MoveTo(client, GetOrigin(SentryGun[client]));
				m_lookAt[client] = GetCenter(SentryGun[client]);
				m_attackTimer[client] = GetGameTime() + 0.5;
				m_duckTimer[client] = GetGameTime() + 0.5;
				m_ignoreEnemies[client] = GetGameTime() + 1.0;
			}
			else
				EquipWeaponSlot(client, 2);
		}
		else
			FollowPath(client, GetOrigin(SentryGun[client]));
		
		return;
	}
	
	if (WantsMoveSentryGun[client] && IsValidEntity(SentryGun[client]))
	{
		if (GetVectorDistance(GetOrigin(client), GetOrigin(SentryGun[client]), true) <= Squaredf(200.0) && IsVisible(GetEyePosition(client), GetCenter(SentryGun[client])))
		{
			if (WantsMoveSentryGun[client])
			{
				MoveTo(client, GetOrigin(SentryGun[client]));
				m_lookAt[client] = GetCenter(SentryGun[client]);
				m_buttons[client] |= IN_ATTACK2;
				m_ignoreEnemies[client] = GetGameTime() + 1.0;

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
					m_attackTimer[client] = GetGameTime() + 0.5;
					m_duckTimer[client] = GetGameTime() + 0.5;
					m_ignoreEnemies[client] = GetGameTime() + 1.0;
				}
				else
					EquipWeaponSlot(client, 2);
			}
		}
		else
			FollowPath(client, GetOrigin(SentryGun[client]));
		
		return;
	}

	if (IsValidEntity(Dispenser[client]) && (!DispenserIsMaxLevel[client] || !DispenserHealthIsFull[client]))
	{
		if (GetVectorDistance(GetOrigin(client), GetOrigin(Dispenser[client]), true) <= Squaredf(200.0) && IsVisible(GetEyePosition(client), GetCenter(Dispenser[client])))
		{
			if (IsWeaponSlotActive(client, 2))
			{
				MoveTo(client, GetOrigin(Dispenser[client]));
				m_lookAt[client] = GetCenter(Dispenser[client]);
				m_attackTimer[client] = GetGameTime() + 0.5;
				m_duckTimer[client] = GetGameTime() + 0.5;
				m_ignoreEnemies[client] = GetGameTime() + 1.0;
			}
			else
				EquipWeaponSlot(client, 2);
		}
		else
			FollowPath(client, GetOrigin(Dispenser[client]));
		
		return;
	}
	
	if (WantsMoveDispenser[client] && IsValidEntity(Dispenser[client]))
	{
		if (GetVectorDistance(GetOrigin(client), GetOrigin(Dispenser[client]), true) <= Squaredf(200.0) && IsVisible(GetEyePosition(client), GetCenter(Dispenser[client])))
		{
			MoveTo(client, GetOrigin(Dispenser[client]));
			m_lookAt[client] = GetCenter(Dispenser[client]);
			m_buttons[client] |= IN_ATTACK2;
			m_ignoreEnemies[client] = GetGameTime() + 1.0;

			if (GetEntProp(client, Prop_Send, "m_bCarryingObject") && IsValidEntity(GetEntPropEnt(client, Prop_Send, "m_hCarriedObject")))
			{
				SetProcess(client, PRO_BUILDDISPENSER, 60.0, "| process started -> wants to move dispenser");
				return;
			}
		}
		else
			FollowPath(client, GetOrigin(Dispenser[client]));
		
		return;
	}
	
	if (IsValidEntity(TeleporterEnter[client]) && (!TeleporterEnterHealthIsFull[client] || !TeleporterEnterIsMaxLevel[client]))
	{
		if (GetVectorDistance(GetOrigin(client), GetOrigin(TeleporterEnter[client]), true) <= Squaredf(200.0) && IsVisible(GetEyePosition(client), GetCenter(TeleporterEnter[client])))
		{
			if (IsWeaponSlotActive(client, 2))
			{
				if (GetVectorDistance(GetOrigin(client), GetOrigin(TeleporterEnter[client]), true) < Squaredf(48.0))
					MoveOut(client, GetOrigin(TeleporterEnter[client]));
				else if (GetVectorDistance(GetOrigin(client), GetOrigin(TeleporterEnter[client]), true) > Squaredf(56.0))
					MoveTo(client, GetOrigin(TeleporterEnter[client]));
				
				m_lookAt[client] = GetCenter(TeleporterEnter[client]);
				m_attackTimer[client] = GetGameTime() + 0.5;
				m_duckTimer[client] = GetGameTime() + 0.5;
				m_ignoreEnemies[client] = GetGameTime() + 1.0;
			}
			else
				EquipWeaponSlot(client, 2);
		}
		else
			FollowPath(client, GetOrigin(TeleporterEnter[client]));
		
		return;
	}
	
	if (IsValidEntity(SentryGun[client]))
	{
		if (GetVectorDistance(GetOrigin(client), GetOrigin(SentryGun[client]), true) <= Squaredf(200.0) && IsVisible(GetEyePosition(client), GetCenter(SentryGun[client])))
		{
			if (IsWeaponSlotActive(client, 2))
			{
				MoveTo(client, GetOrigin(SentryGun[client]));
				m_lookAt[client] = GetCenter(SentryGun[client]);
				m_attackTimer[client] = GetGameTime() + 0.5;
				m_duckTimer[client] = GetGameTime() + 0.5;
				m_ignoreEnemies[client] = GetGameTime() + 1.0;
			}
			else
				EquipWeaponSlot(client, 2);
		}
		else
			FollowPath(client, GetOrigin(SentryGun[client]));
	}
}

public void EngineerIdleEnd(int client)
{
	// reset battle range
	m_eBotDodgeRange[client] = -1;
}

public void CheckEngineerBuildings(int client)
{
	if (m_isSlowThink[client])
		return;
	
	SentryGun[client] = TF2_GetObject(client, TFObject_Sentry, TFObjectMode_None);
	Dispenser[client] = TF2_GetObject(client, TFObject_Dispenser, TFObjectMode_None);
	TeleporterEnter[client] = TF2_GetObject(client, TFObject_Teleporter, TFObjectMode_Entrance);
	TeleporterExit[client] = TF2_GetObject(client, TFObject_Teleporter, TFObjectMode_Exit);

	bool isDefending = IsOnDefanse(client);
	
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
		if (isDefending)
			EngineerMoveSentryTimer[client] = GetGameTime() + 99999999.0;
		else
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
		if (!WantsBuildSentryGun[client])
			WantsBuildDispenser[client] = true;
		else
		{
			WantsBuildDispenser[client] = false;
			if (isDefending)
				EngineerMoveDispenserTimer[client] = GetGameTime() + 99999999.0;
			else
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
	else if (m_hasTeleporterEnterWaypoints && m_hasTeleporterExitWaypoints && !WantsBuildSentryGun[client] && !WantsBuildDispenser[client] && !WantsBuildTeleporterEnter[client])
		WantsBuildTeleporterExit[client] = true;
	else
		WantsBuildTeleporterExit[client] = false;
}
