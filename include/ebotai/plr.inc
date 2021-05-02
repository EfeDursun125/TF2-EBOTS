stock int GameModePayloadRace(int client)
{
	if (!IsSlowThink[client])
		return;
	
	int payload = GetNearestPayload(client, GetClientTeam(client));
	
	if(payload != -1)
	{
		float payloadposition[3];
		payloadposition = GetOrigin(payload);
		
		payloadposition[0] += GetRandomFloat(-30.0, 30.0);
		payloadposition[1] += GetRandomFloat(-30.0, 30.0);
		
		TF2_FindPath(client, payloadposition);
	}
}
