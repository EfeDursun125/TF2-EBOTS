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

public bool EngineerIdleRequirements(const int client)
{
	return true;
}

public void EngineerIdleStart(const int client)
{
	EquipWeaponSlot(client, 0);
}

public void EngineerIdleUpdate(const int client)
{
	if (TF2_HasTheFlag(client))
	{
		ObjectiveMovement(client);
		if (MustFindObjective(client, m_goalIndex[client], m_currentIndex[client]))
		{
			SelectObjective(client);
			FollowPath(client, m_goalPosition[client], -1, m_goalIndex[client]);
		}
		
		return;
	}

	m_eBotDodgeRange[client] = 512;

	CheckHealth(client);

	if (m_lowAmmo[client])
		SetProcess(client, PRO_GETAMMO, 90.0, "| process started -> need ammo pack", true, false);
	
	if (IsMetalLow(client))
	{
		m_lowAmmo[client] = true;
		SetProcess(client, PRO_GETAMMO, 90.0, "| process started -> need ammo pack", true, false);
	}
	else
		m_lowAmmo[client] = false;
	
	CheckHideFromDanger(client);
	FindFriendsAndEnemiens(client);
	FindEnemyEntities(client);
	CheckEngineerBuildings(client);
	LookUpdate(client);

	static float eyePosition[3];
	static float targetOrigin[3];
	eyePosition = GetEyePosition(client);

	if (m_useTeleporter[client] && IsValidEntity(m_teleporterEntity[client]))
	{
		DeletePathNodes(client);
		m_nextStuckCheck[client] = GetGameTime() + 5.0;
		targetOrigin = GetOrigin(m_teleporterEntity[client]);

		if (GetVectorDistance(GetOrigin(client), targetOrigin, true) < Squaredf(24.0))
		{
			m_moveVel[client][0] = 0.0;
			m_moveVel[client][1] = 0.0;
			m_moveVel[client][2] = 0.0;
		}
		else
			MoveToWaypoint(client, targetOrigin);
		
		return;
	}
	
	if (!IsValidEntity(SentryGun[client]))
	{
		if (WantsBuildTeleporterEnter[client] && GetRandomInt(1, 3) == 1)
			SetProcess(client, PRO_BUILDTELEPORTERENTER, 20.0, "| process started -> trying to build teleporter enter", false, false);

		if (WantsBuildTeleporterExit[client] && IsValidEntity(TeleporterEnter[client]))
			SetProcess(client, PRO_BUILDTELEPORTEREXIT, 80.0, "| process started -> trying to build teleporter exit", false, false);

		if (WantsBuildSentryGun[client])
			SetProcess(client, PRO_BUILDSENTRY, 40.0, "| process started -> trying to build sentry gun", false, false);
	}

	if (WantsBuildDispenser[client] && !IsValidEntity(Dispenser[client]) && IsValidEntity(SentryGun[client]) && IsVisible(eyePosition, GetCenter(SentryGun[client])))
		SetProcess(client, PRO_BUILDDISPENSER, 60.0, "| process started -> trying to build dispenser", false, false);

	if (WantsBuildTeleporterExit[client] && !IsValidEntity(TeleporterExit[client]))
		SetProcess(client, PRO_BUILDTELEPORTEREXIT, 80.0, "| process started -> trying to build teleporter exit", false, false);

	if (WantsBuildTeleporterEnter[client] && !IsValidEntity(TeleporterEnter[client]))
		SetProcess(client, PRO_BUILDTELEPORTERENTER, 20.0, "| process started -> trying to build teleporter enter", false, false);

	if (IsValidEntity(SentryGun[client]) && (!SentryIsMaxLevel[client] || !SentryHealthIsFull[client]))
	{
		targetOrigin = GetCenter(SentryGun[client]);

		if (GetVectorDistance(eyePosition, targetOrigin, true) < Squaredf(200.0) && IsVisible(eyePosition, targetOrigin))
		{
			if (IsWeaponSlotActive(client, 2))
			{
				MoveTo(client, targetOrigin);
				m_lookAt[client] = targetOrigin;
				m_attackTimer[client] = GetGameTime() + 0.5;
				m_duckTimer[client] = GetGameTime() + 0.5;
				m_ignoreEnemies[client] = GetGameTime() + 1.0;
				m_nextStuckCheck[client] = GetGameTime() + 5.0;
			}
			else
				EquipWeaponSlot(client, 2);
		}
		else
			FollowPath(client, targetOrigin);
		
		return;
	}
	
	if (WantsMoveSentryGun[client] && IsValidEntity(SentryGun[client]))
	{
		targetOrigin = GetCenter(SentryGun[client]);

		if (GetVectorDistance(eyePosition, targetOrigin, true) < Squaredf(200.0) && IsVisible(eyePosition, targetOrigin))
		{
			if (WantsMoveSentryGun[client])
			{
				MoveTo(client, targetOrigin);
				m_lookAt[client] = targetOrigin;
				m_buttons[client] |= IN_ATTACK2;
				m_ignoreEnemies[client] = GetGameTime() + 1.0;
				m_nextStuckCheck[client] = GetGameTime() + 5.0;

				if (GetEntProp(client, Prop_Send, "m_bCarryingObject") && IsValidEntity(GetEntPropEnt(client, Prop_Send, "m_hCarriedObject")))
				{
					SetProcess(client, PRO_BUILDSENTRY, 60.0, "| process started -> wants to move sentry gun", false, false);
					return;
				}
			}
			else
			{
				if (IsWeaponSlotActive(client, 2))
				{
					MoveTo(client, targetOrigin);
					m_lookAt[client] = targetOrigin;
					m_attackTimer[client] = GetGameTime() + 0.5;
					m_duckTimer[client] = GetGameTime() + 0.5;
					m_ignoreEnemies[client] = GetGameTime() + 1.0;
					m_nextStuckCheck[client] = GetGameTime() + 5.0;
				}
				else
					EquipWeaponSlot(client, 2);
			}
		}
		else
			FollowPath(client, targetOrigin);
		
		return;
	}

	if (IsValidEntity(Dispenser[client]) && (!DispenserIsMaxLevel[client] || !DispenserHealthIsFull[client]))
	{
		targetOrigin = GetCenter(Dispenser[client]);

		if (GetVectorDistance(eyePosition, targetOrigin, true) < Squaredf(200.0) && IsVisible(eyePosition, targetOrigin))
		{
			if (IsWeaponSlotActive(client, 2))
			{
				MoveTo(client, targetOrigin);
				m_lookAt[client] = targetOrigin;
				m_attackTimer[client] = GetGameTime() + 0.5;
				m_duckTimer[client] = GetGameTime() + 0.5;
				m_ignoreEnemies[client] = GetGameTime() + 1.0;
				m_nextStuckCheck[client] = GetGameTime() + 5.0;
			}
			else
				EquipWeaponSlot(client, 2);
		}
		else
			FollowPath(client, targetOrigin);
		
		return;
	}
	
	if (WantsMoveDispenser[client] && IsValidEntity(Dispenser[client]))
	{
		targetOrigin = GetCenter(Dispenser[client]);

		if (GetVectorDistance(eyePosition, targetOrigin, true) < Squaredf(200.0) && IsVisible(eyePosition, targetOrigin))
		{
			MoveTo(client, targetOrigin);
			m_lookAt[client] = targetOrigin;
			m_buttons[client] |= IN_ATTACK2;
			m_ignoreEnemies[client] = GetGameTime() + 1.0;
			m_nextStuckCheck[client] = GetGameTime() + 5.0;

			if (GetEntProp(client, Prop_Send, "m_bCarryingObject") && IsValidEntity(GetEntPropEnt(client, Prop_Send, "m_hCarriedObject")))
			{
				SetProcess(client, PRO_BUILDDISPENSER, 60.0, "| process started -> wants to move dispenser", false, false);
				return;
			}
		}
		else
			FollowPath(client, targetOrigin);
		
		return;
	}
	
	if (IsValidEntity(TeleporterEnter[client]) && (!TeleporterEnterHealthIsFull[client] || !TeleporterEnterIsMaxLevel[client]))
	{
		targetOrigin = GetOrigin(TeleporterEnter[client]);

		if (GetVectorDistance(GetOrigin(client), targetOrigin, true) < Squaredf(200.0) && IsVisible(GetEyePosition(client), targetOrigin))
		{
			if (IsWeaponSlotActive(client, 2))
			{
				if (GetVectorDistance(GetOrigin(client), targetOrigin, true) < Squaredf(48.0))
					MoveOut(client, targetOrigin);
				else if (GetVectorDistance(GetOrigin(client), targetOrigin, true) > Squaredf(56.0))
					MoveTo(client, targetOrigin);
				
				m_lookAt[client] = GetCenter(TeleporterEnter[client]);
				m_attackTimer[client] = GetGameTime() + 0.5;
				m_duckTimer[client] = GetGameTime() + 0.5;
				m_ignoreEnemies[client] = GetGameTime() + 1.0;
				m_nextStuckCheck[client] = GetGameTime() + 5.0;
			}
			else
				EquipWeaponSlot(client, 2);
		}
		else
			FollowPath(client, targetOrigin);
		
		return;
	}

	if (GetMetal(client) < 0.0)
		SetProcess(client, PRO_GETAMMO, 90.0, "| process started -> need ammo pack", false, false);
	else if (IsValidEntity(SentryGun[client]))
	{
		targetOrigin = GetCenter(SentryGun[client]);

		if (GetVectorDistance(eyePosition, targetOrigin, true) < Squaredf(200.0) && IsVisible(eyePosition, targetOrigin))
		{
			if (IsWeaponSlotActive(client, 2))
			{
				MoveTo(client, targetOrigin);
				m_lookAt[client] = targetOrigin;
				m_attackTimer[client] = GetGameTime() + 0.5;
				m_duckTimer[client] = GetGameTime() + 0.5;
				m_ignoreEnemies[client] = GetGameTime() + 1.0;
				m_nextStuckCheck[client] = GetGameTime() + 5.0;
			}
			else
				EquipWeaponSlot(client, 2);
		}
		else
			FollowPath(client, targetOrigin);
	}
	else if (GetMetal(client) < 130.0)
		SetProcess(client, PRO_GETAMMO, 90.0, "| process started -> need ammo pack", false, false);
	else
		SetProcess(client, PRO_BUILDSENTRY, 80.0, "| process started -> trying to build sentry gun", false, false);
}

public void EngineerIdleEnd(const int client)
{
	// reset battle range
	m_eBotDodgeRange[client] = -1;
}

public void CheckEngineerBuildings(const int client)
{
    if (!m_isSlowThink[client])
        return;

    WantsBuildSentryGun[client] = true;
    WantsBuildDispenser[client] = true;
    WantsBuildTeleporterEnter[client] = true;
    WantsBuildTeleporterExit[client] = true;
    SentryIsMaxLevel[client] = false;
    SentryHealthIsFull[client] = false;
    DispenserIsMaxLevel[client] = false;
    DispenserHealthIsFull[client] = false;
    TeleporterEnterIsMaxLevel[client] = false;
    TeleporterEnterHealthIsFull[client] = false;
    TeleporterExitIsMaxLevel[client] = false;
    TeleporterExitHealthIsFull[client] = false;

    SentryGun[client] = TF2_GetObject(client, TFObject_Sentry, TFObjectMode_None);
    Dispenser[client] = TF2_GetObject(client, TFObject_Dispenser, TFObjectMode_None);
    TeleporterEnter[client] = TF2_GetObject(client, TFObject_Teleporter, TFObjectMode_Entrance);
    TeleporterExit[client] = TF2_GetObject(client, TFObject_Teleporter, TFObjectMode_Exit);

    if (IsValidEntity(SentryGun[client]))
    {
        WantsBuildSentryGun[client] = false;

        if (TF2_GetPlayerWeaponID(client, 2) == 142 || !hasWrench(client))
            SentryIsMaxLevel[client] = true;
        else
            SentryIsMaxLevel[client] = (GetEntProp(SentryGun[client], Prop_Send, "m_iUpgradeLevel") >= 3);
                
        SentryHealthIsFull[client] = (GetEntProp(SentryGun[client], Prop_Send, "m_iHealth") >= GetEntProp(SentryGun[client], Prop_Send, "m_iMaxHealth") && !GetEntProp(SentryGun[client], Prop_Send, "m_bHasSapper"));
        
        if (EngineerMoveSentryTimer[client] < GetGameTime())
            WantsMoveSentryGun[client] = true;
		else if ((EngineerMoveSentryTimer[client] + 90.0) < GetGameTime()) // unable to reach
			FakeClientCommandThrottled(client, "destroy 2 0");
    }
    else
    {
        WantsBuildSentryGun[client] = true;
        if (IsOnDefanse(client))
            EngineerMoveSentryTimer[client] = GetGameTime() + 99999999.0;
        else
            EngineerMoveSentryTimer[client] = GetGameTime() + GetRandomFloat(90.0, 160.0);
        WantsMoveSentryGun[client] = false;
    }
    
    if (IsValidEntity(Dispenser[client]))
    {
        WantsBuildDispenser[client] = false;
        DispenserIsMaxLevel[client] = (GetEntProp(Dispenser[client], Prop_Send, "m_iUpgradeLevel") >= 3);
        DispenserHealthIsFull[client] = (GetEntProp(Dispenser[client], Prop_Send, "m_iHealth") >= GetEntProp(Dispenser[client], Prop_Send, "m_iMaxHealth") && !GetEntProp(Dispenser[client], Prop_Send, "m_bHasSapper"));
		if ((EngineerMoveDispenserTimer[client] + 60.0) < GetGameTime()) // unable to reach
			FakeClientCommandThrottled(client, "destroy 0 0");
    }
    else
    {
        if (!WantsBuildSentryGun[client])
            WantsBuildDispenser[client] = true;
        else
        {
            WantsBuildDispenser[client] = false;
            if (IsOnDefanse(client))
                EngineerMoveDispenserTimer[client] = GetGameTime() + 99999999.0;
            else
                EngineerMoveDispenserTimer[client] = GetGameTime() + GetRandomFloat(120.0, 180.0);
            WantsMoveDispenser[client] = false;
        }
    }
            
    if (IsValidEntity(TeleporterEnter[client]))
    {
        WantsBuildTeleporterEnter[client] = false;
        TeleporterEnterIsMaxLevel[client] = (GetEntProp(TeleporterEnter[client], Prop_Send, "m_iUpgradeLevel") >= 3);
        TeleporterEnterHealthIsFull[client] = (GetEntProp(TeleporterEnter[client], Prop_Send, "m_iHealth") >= GetEntProp(TeleporterEnter[client], Prop_Send, "m_iMaxHealth") && !GetEntProp(TeleporterEnter[client], Prop_Send, "m_bHasSapper"));
    }
    else if (m_hasTeleporterEnterWaypoints && m_hasTeleporterExitWaypoints)
        WantsBuildTeleporterEnter[client] = true;
    else
        WantsBuildTeleporterEnter[client] = false;
    
    if (IsValidEntity(TeleporterExit[client]))
    {
        WantsBuildTeleporterExit[client] = false;
        TeleporterExitIsMaxLevel[client] = (GetEntProp(TeleporterExit[client], Prop_Send, "m_iUpgradeLevel") >= 3);
        TeleporterExitHealthIsFull[client] = (GetEntProp(TeleporterExit[client], Prop_Send, "m_iHealth") >= GetEntProp(TeleporterExit[client], Prop_Send, "m_iMaxHealth") && !GetEntProp(TeleporterExit[client], Prop_Send, "m_bHasSapper"));
    }
    else if (m_hasTeleporterEnterWaypoints && m_hasTeleporterExitWaypoints && !WantsBuildSentryGun[client] && !WantsBuildDispenser[client] && !WantsBuildTeleporterEnter[client])
        WantsBuildTeleporterExit[client] = true;
    else
        WantsBuildTeleporterExit[client] = false;
}