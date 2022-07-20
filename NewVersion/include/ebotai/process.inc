int FailedProcess[MAXPLAYERS + 1];
int RememberedProcess[MAXPLAYERS + 1];
float CurrentProcessTime[MAXPLAYERS + 1];
float RememberedProcessTime[MAXPLAYERS + 1];

stock void SetProcess(int client, int process, float time = 9999.0, const char[] debugnote = "clear", bool ignoredebug = false, bool remember = true)
{
	if (FailedProcess[client] == process)
		return;
	
	if (IsReadyForTheProcess(client, process) && CurrentProcess[client] != process)
	{
		if (!ignoredebug)
			PrintToServer("%N has got a new process from %s to %s %s", client, GetProcessName(CurrentProcess[client]), GetProcessName(process), debugnote);

		if (remember && CurrentProcess[client] != PRO_HIDE)
		{
			RememberedProcess[client] = CurrentProcess[client];
			RememberedProcessTime[client] = CurrentProcessTime[client];
		}

		EndProcess(client, CurrentProcess[client]);
		CurrentProcess[client] = process;
		StartProcess(client, process);
		CurrentProcessTime[client] = GetGameTime() + time;
	}
}

public void StartProcess(int client, int process)
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
	}
}

public void EndProcess(int client, int process)
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
	}
}

public void UpdateProcess(int client)
{
	if (CurrentProcess[client] != PRO_DEFAULT && CurrentProcessTime[client] < GetGameTime() && CurrentProcess[client] != PRO_SPYLURK && CurrentProcess[client] != PRO_SPYHUNT)
	{
		PrintToServer("%N is cancelled %s process -> timed out.", client, GetProcessName(CurrentProcess[client]));
		EndProcess(client, CurrentProcess[client]);
		CurrentProcess[client] = PRO_DEFAULT;
		StartProcess(client, PRO_DEFAULT);
	}
	
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
	}

	if (m_hasEnemiesNear[client] && TF2_IsPlayerInCondition(client, TFCond_Cloaked))
		SetProcess(client, PRO_HIDE, 60.0, "", true);
}

stock void FinishCurrentProcess(int client, bool nodbg = false, const char[] debugnote = "finished by the system")
{
	if (CurrentProcess[client] != PRO_DEFAULT)
	{
		if (nodbg && CurrentProcess[client] != PRO_ATTACK)
			PrintToServer("%N is cancelled %s process -> %s", client, GetProcessName(CurrentProcess[client]), debugnote);

		EndProcess(client, CurrentProcess[client]);

		if (RememberedProcess[client] != CurrentProcess[client] && RememberedProcessTime[client] > GetGameTime())
		{
			StartProcess(client, RememberedProcess[client]);
			CurrentProcess[client] = RememberedProcess[client];
			return;
		}

		StartProcess(client, PRO_DEFAULT);
		CurrentProcess[client] = PRO_DEFAULT;
	}
}

stock void SetFailedProcess(int client, bool nodbg = false, const char[] debugnote = "finished by the system")
{
	if (CurrentProcess[client] != PRO_DEFAULT)
	{
		FailedProcess[client] = CurrentProcess[client];
		FinishCurrentProcess(client, nodbg, debugnote);
	}
}

public bool IsReadyForTheProcess(int client, int process)
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
	}

	return true;
}

stock char[] GetProcessName(int process)
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
			string = "DEFEND";
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
	}
	return string;
}
