stock int GameModePayloadRace(int client)
{
	if (!IsSlowThink[client])
		return;
	
	int payload = GetNearestPayload(client, GetClientTeam(client));
	
	if(payload != -1)
	{
		TF2_FindPath(client, GetOrigin(payload));
	}
}
