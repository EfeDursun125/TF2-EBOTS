float VoiceTimer[MAXPLAYERS + 1];

stock void UseVoiceCommand(int client)
{
	if(VoiceTimer[client] < GetGameTime())
	{
		if(HasEnemiesNear[client] && IsValidClient(NearestEnemy[client]) && TF2_GetPlayerClass(NearestEnemy[client]) == TFClass_Spy && ChanceOf(EBotSenseChance[client]))
		{
			FakeClientCommandThrottled(client, "voicemenu 1 1");
			
			return;
		}
		
		if(TF2_GetNumberOfHealers(client) >= 1 && ChanceOf(EBotSenseChance[client]))
		{
			FakeClientCommandThrottled(client, "voicemenu 0 1");
			
			return;
		}
		
		if(HasEnemiesNear[client] && g_bAmmoIsLow[client] && ChanceOf(EBotSenseChance[client]))
		{
			FakeClientCommandThrottled(client, "voicemenu 2 0");
			
			return;
		}
		
		if(g_bHealthIsLow[client] && ChanceOf(EBotSenseChance[client]))
		{
			FakeClientCommandThrottled(client, "voicemenu 0 0");
			
			return;
		}
		
		VoiceTimer[client] = GetGameTime() + GetRandomFloat(10.0, 30.0);
	}
}