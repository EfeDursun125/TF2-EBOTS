int FailedProcess[TFMaxPlayers];
int RememberedProcess[TFMaxPlayers];
float CurrentProcessTime[TFMaxPlayers];
float RememberedProcessTime[TFMaxPlayers];

stock bool SetProcess(const int client, const int process, const float time = 9999.0, const char[] debugnote = "clear", const bool ignoredebug = false, const bool remember = true)
{
	if (FailedProcess[client] == process)
		return false;

	if (CurrentProcess[client] != process && IsReadyForTheProcess(client, process))
	{
		if (!ignoredebug && GetConVarInt(EBotDebug))
			PrintToServer("%N has got a new process from %s to %s %s", client, GetProcessName(CurrentProcess[client], client), GetProcessName(process, client), debugnote);

		if (remember && CurrentProcess[client] != PRO_HIDE && CurrentProcess[client] != PRO_ENGINEERIDLE && CurrentProcess[client] != PRO_SPYLURK)
		{
			RememberedProcess[client] = CurrentProcess[client];
			RememberedProcessTime[client] = CurrentProcessTime[client];
		}

		EndProcess(client, CurrentProcess[client]);
		CurrentProcess[client] = process;
		StartProcess(client, process);
		CurrentProcessTime[client] = GetGameTime() + time;
		return true;
	}

	return false;
}

public void StartProcess(const int client, const int process)
{
	switch (process)
	{
		case PRO_DEFAULT:
			DefaultStart(client);
		case PRO_GETHEALTH:
			GetHealthStart(client);
		case PRO_ATTACK:
			AttackStart(client);
		case PRO_GETAMMO:
			GetAmmoStart(client);
		case PRO_IDLE:
			IdleStart(client);
		case PRO_SPYLURK:
			SpyLurkStart(client);
		case PRO_SPYHUNT:
			SpyHuntStart(client);
		case PRO_SPYSAP:
			SpySapStart(client);
		case PRO_DEFEND:
			DefendStart(client);
		case PRO_HEAL:
			HealStart(client);
		case PRO_ENGINEERIDLE:
			EngineerIdleStart(client);
		case PRO_BUILDSENTRY:
			BuildSentryStart(client);
		case PRO_BUILDDISPENSER:
			BuildDispenserStart(client);
		case PRO_BUILDTELEPORTERENTER:
			BuildTeleEnterStart(client);
		case PRO_BUILDTELEPORTEREXIT:
			BuildTeleExitStart(client);
		case PRO_HIDE:
			HideStart(client);
		case PRO_HUNTENEMY:
			HuntStart(client);
		case PRO_FOLLOWPATH:
			FollowPathStart(client);
	}
}

public void EndProcess(const int client, const int process)
{
	CurrentProcessTime[client] = GetGameTime();

	switch (process)
	{
		case PRO_DEFAULT:
			DefaultEnd(client);
		case PRO_GETHEALTH:
			GetHealthEnd(client);
		case PRO_ATTACK:
			AttackEnd(client);
		case PRO_GETAMMO:
			GetAmmoEnd(client);
		case PRO_IDLE:
			IdleEnd(client);
		case PRO_SPYLURK:
			SpyLurkEnd(client);
		case PRO_SPYHUNT:
			SpyHuntEnd(client);
		case PRO_SPYSAP:
			SpySapEnd(client);
		case PRO_DEFEND:
			DefendEnd(client);
		case PRO_HEAL:
			HealEnd(client);
		case PRO_ENGINEERIDLE:
			EngineerIdleEnd(client);
		case PRO_BUILDSENTRY:
			BuildSentryEnd(client);
		case PRO_BUILDDISPENSER:
			BuildDispenserEnd(client);
		case PRO_BUILDTELEPORTERENTER:
			BuildTeleEnterEnd(client);
		case PRO_BUILDTELEPORTEREXIT:
			BuildTeleExitEnd(client);
		case PRO_HIDE:
			HideEnd(client);
		case PRO_HUNTENEMY:
			HuntEnd(client);
		case PRO_FOLLOWPATH:
			FollowPathEnd(client);
	}
}

public void UpdateProcess(const int client)
{
	switch (CurrentProcess[client])
	{
		case PRO_DEFAULT:
			DefaultUpdate(client);
		case PRO_GETHEALTH:
			GetHealthUpdate(client);
		case PRO_ATTACK:
			AttackUpdate(client);
		case PRO_GETAMMO:
			GetAmmoUpdate(client);
		case PRO_IDLE:
			IdleUpdate(client);
		case PRO_SPYLURK:
			SpyLurkUpdate(client);
		case PRO_SPYHUNT:
			SpyHuntUpdate(client);
		case PRO_SPYSAP:
			SpySapUpdate(client);
		case PRO_DEFEND:
			DefendUpdate(client);
		case PRO_HEAL:
			HealUpdate(client);
		case PRO_ENGINEERIDLE:
			EngineerIdleUpdate(client);
		case PRO_BUILDSENTRY:
			BuildSentryUpdate(client);
		case PRO_BUILDDISPENSER:
			BuildDispenserUpdate(client);
		case PRO_BUILDTELEPORTERENTER:
			BuildTeleEnterUpdate(client);
		case PRO_BUILDTELEPORTEREXIT:
			BuildTeleExitUpdate(client);
		case PRO_HIDE:
			HideUpdate(client);
		case PRO_HUNTENEMY:
			HuntUpdate(client);
		case PRO_FOLLOWPATH:
			FollowPathUpdate(client);
	}

	/*if (m_hasEnemiesNear[client] && TF2_IsPlayerInCondition(client, TFCond_Cloaked)) // useful for randomizer
	{
		SetProcess(client, PRO_HIDE, 60.0, "", true);
		return;
	}*/

	if (CurrentProcess[client] != PRO_DEFAULT && CurrentProcess[client] != PRO_ENGINEERIDLE && CurrentProcessTime[client] < GetGameTime() && CurrentProcess[client] != PRO_SPYLURK && CurrentProcess[client] != PRO_SPYHUNT)
	{
		if (GetConVarInt(EBotDebug))
			PrintToServer("%N is cancelled %s process -> timed out.", client, GetProcessName(CurrentProcess[client], client));
		EndProcess(client, CurrentProcess[client]);

		if (CurrentProcess[client] != PRO_DEFAULT && CurrentProcess[client] != PRO_ENGINEERIDLE && RememberedProcess[client] != CurrentProcess[client] && RememberedProcessTime[client] > GetGameTime() && IsReadyForTheProcess(client, RememberedProcess[client]))
		{
			StartProcess(client, RememberedProcess[client]);
			if (GetConVarInt(EBotDebug))
				PrintToServer("%N is remembered %s process -> from %s", client, GetProcessName(RememberedProcess[client], client), GetProcessName(CurrentProcess[client], client));
			CurrentProcess[client] = RememberedProcess[client];
			CurrentProcessTime[client] = RememberedProcessTime[client];
			RememberedProcess[client] = PRO_DEFAULT;
			RememberedProcessTime[client] = 0.0;
			return;
		}

		CurrentProcess[client] = PRO_DEFAULT;
		StartProcess(client, PRO_DEFAULT);
	}
}

stock void FinishCurrentProcess(const int client, const bool nodbg = false, const char[] debugnote = "finished by the system")
{
	if (CurrentProcess[client] != PRO_DEFAULT)
	{
		if (nodbg && CurrentProcess[client] != PRO_ATTACK && GetConVarInt(EBotDebug))
			PrintToServer("%N is cancelled %s process -> %s", client, GetProcessName(CurrentProcess[client], client), debugnote);

		EndProcess(client, CurrentProcess[client]);

		if (CurrentProcess[client] != PRO_DEFAULT && CurrentProcess[client] != PRO_ENGINEERIDLE && RememberedProcess[client] != CurrentProcess[client] && RememberedProcessTime[client] > GetGameTime() && IsReadyForTheProcess(client, RememberedProcess[client]))
		{
			StartProcess(client, RememberedProcess[client]);
			if (GetConVarInt(EBotDebug))
				PrintToServer("%N is remembered %s process -> from %s", client, GetProcessName(RememberedProcess[client], client), GetProcessName(CurrentProcess[client], client));
			CurrentProcess[client] = RememberedProcess[client];
			CurrentProcessTime[client] = RememberedProcessTime[client];
			RememberedProcess[client] = PRO_DEFAULT;
			RememberedProcessTime[client] = 0.0;
			return;
		}

		StartProcess(client, PRO_DEFAULT);
		CurrentProcess[client] = PRO_DEFAULT;
	}
}

stock void SetFailedProcess(const int client, const bool nodbg = false, const char[] debugnote = "finished by the system")
{
	if (CurrentProcess[client] != PRO_DEFAULT)
	{
		FailedProcess[client] = CurrentProcess[client];
		FinishCurrentProcess(client, nodbg, debugnote);
	}
}

public bool IsReadyForTheProcess(const int client, const int process)
{
	switch (process)
	{
		case PRO_DEFAULT:
			return DefaultRequirements(client);
		case PRO_GETHEALTH:
			return GetHealthRequirements(client);
		case PRO_ATTACK:
			return AttackRequirements(client);
		case PRO_GETAMMO:
			return GetAmmoRequirements(client);
		case PRO_IDLE:
			return IdleRequirements(client);
		case PRO_SPYLURK:
			return SpyLurkRequirements(client);
		case PRO_SPYHUNT:
			return SpyHuntRequirements(client);
		case PRO_SPYSAP:
			return SpySapRequirements(client);
		case PRO_DEFEND:
			return DefendRequirements(client);
		case PRO_HEAL:
			return HealRequirements(client);
		case PRO_ENGINEERIDLE:
			return EngineerIdleRequirements(client);
		case PRO_BUILDSENTRY:
			return BuildSentryRequirements(client);
		case PRO_BUILDDISPENSER:
			return BuildDispenserRequirements(client);
		case PRO_BUILDTELEPORTERENTER:
			return BuildTeleEnterRequirements(client);
		case PRO_BUILDTELEPORTEREXIT:
			return BuildTeleExitRequirements(client);
		case PRO_HIDE:
			return HideRequirements(client);
		case PRO_HUNTENEMY:
			return HuntRequirements(client);
		case PRO_FOLLOWPATH:
			return FollowPathRequirements(client);
	}

	return true;
}

stock char[] GetProcessName(const int process, const int client)
{
	char string[64] = "UNKNOWN";
	switch (process)
	{
		case PRO_DEFAULT:
			string = "DEFAULT";
		case PRO_GETHEALTH:
			string = "GET_HEALTH";
		case PRO_ATTACK:
			string = "ATTACK";
		case PRO_GETAMMO:
			string = "GET_AMMO";
		case PRO_IDLE:
			string = "IDLE";
		case PRO_SPYLURK:
			string = "SPY_LURK";
		case PRO_SPYHUNT:
			string = "SPY_HUNT";
		case PRO_SPYSAP:
			string = "SPY_SAP";
		case PRO_DEFEND:
		{
			if (IsSniper(client))
				string = "SNIPING";
			else
				string = "DEFEND";
		}
		case PRO_HEAL:
			string = "HEAL";
		case PRO_ENGINEERIDLE:
			string = "ENGINEER_IDLE";
		case PRO_BUILDSENTRY:
			string = "BUILD_SENTRY";
		case PRO_BUILDDISPENSER:
			string = "BUILD_DISPENSER";
		case PRO_BUILDTELEPORTERENTER:
			string = "BUILD_TELE_ENTER";
		case PRO_BUILDTELEPORTEREXIT:
			string = "BUILD_TELE_EXIT";
		case PRO_HIDE:
			string = "HIDE";
		case PRO_HUNTENEMY:
			string = "HUNT ENEMY";
		case PRO_FOLLOWPATH:
			string = "FOLLOW PATH";
	}

	return string;
}
