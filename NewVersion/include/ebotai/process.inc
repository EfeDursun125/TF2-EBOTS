enum
{
    PRO_DEFAULT = 0,
	PRO_IDLE,
	PRO_ATTACK,
	PRO_HUNTENEMY,
	PRO_HEAL,
	PRO_SPYLURK,
	PRO_SPYHUNT,
	PRO_GETAMMO,
	PRO_GETHEALTH,
	PRO_FLEE,
	PRO_DEFEND,
	PRO_HIDE,
	PRO_BUILDDISPENSER,
	PRO_BUILDTELEPORTER,
	PRO_BUILDSENTRY
}

int CurrentProcess[MAXPLAYERS + 1] = PRO_DEFAULT;
int RememberedProcess[MAXPLAYERS + 1] = PRO_DEFAULT;
float CurrentProcessTime[MAXPLAYERS + 1] = 9999.0;
float RememberedProcessTime[MAXPLAYERS + 1] = 0.0;

stock void SetProcess(int client, int process, float time = 9999.0, const char[] debugnote = "clear", bool ignoredebug = false)
{
	if (IsReadyForTheProcess(client, process) && CurrentProcess[client] != process)
	{
		if (!ignoredebug)
			PrintToServer("%N has got a new process from %s to %s %s", client, GetProcessName(CurrentProcess[client]), GetProcessName(process), debugnote);
		RememberedProcess[client] = CurrentProcess[client];
		RememberedProcessTime[client] = CurrentProcessTime[client];
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
		case PRO_DEFEND:
			DefendStart(client);
		case PRO_HEAL:
			HealStart(client);
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
		case PRO_DEFEND:
			DefendEnd(client);
		case PRO_HEAL:
			HealEnd(client);
	}
}

public void UpdateProcess(int client)
{
	if (CurrentProcess[client] != PRO_DEFAULT && CurrentProcessTime[client] < GetGameTime())
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
		case PRO_DEFEND:
			DefendUpdate(client);
		case PRO_HEAL:
			HealUpdate(client);
	}
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
		case PRO_DEFEND:
			return DefendRequirements(client);
		case PRO_HEAL:
			return HealRequirements(client);
	}

	return true;
}

stock char GetProcessName(int process)
{
	char string[PLATFORM_MAX_PATH] = "UNKNOWN";
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
		case PRO_DEFEND:
			string = "DEFEND";
		case PRO_HEAL:
			string = "HEAL";
	}
	return string;
}
