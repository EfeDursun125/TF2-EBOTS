float m_voiceTimer[TFMaxPlayers];
char m_lastVoice[TFMaxPlayers][32];

stock void CallVoiceCommand(int client, const char[] voicecommand)
{
	if (StrEqual(m_lastVoice[client], voicecommand))
		return;
	
	FakeClientCommandThrottled(client, voicecommand);
	FormatEx(m_lastVoice[client], 32, voicecommand);
}

stock void UseVoiceCommand(int client)
{
	if (m_voiceTimer[client] < GetGameTime())
	{
		m_voiceTimer[client] = GetGameTime() + GetRandomFloat(10.0, 20.0);

		if (m_hasEnemiesNear[client] && IsValidClient(m_nearestEnemy[client]) && m_class[m_nearestEnemy[client]] == TFClass_Spy && ChanceOf(m_eBotSenseChance[client]))
		{
			CallVoiceCommand(client, "voicemenu 1 1");
			return;
		}
		
		if (TF2_GetNumberOfHealers(client) >= 1 && ChanceOf(m_eBotSenseChance[client]))
		{
			CallVoiceCommand(client, "voicemenu 0 1");
			return;
		}
		
		if (m_hasEnemiesNear[client] && (m_lowAmmo[client] || CurrentProcess[client] == PRO_HIDE) && ChanceOf(m_eBotSenseChance[client]))
		{
			CallVoiceCommand(client, "voicemenu 2 0");
			return;
		}
		
		if (m_lowHealth[client] && ChanceOf(m_eBotSenseChance[client]))
		{
			CallVoiceCommand(client, "voicemenu 0 0");
			return;
		}
	}
}