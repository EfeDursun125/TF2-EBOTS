public void ThinkAI(const int client)
{
	UpdateProcess(client);
}

// automatic look update
stock void LookUpdate(const int client, const bool lookaround = true)
{
	if (m_class[client] == TFClass_Pyro && m_primaryID[client] != 594 && m_hasFriendsNear[client] && m_friendDistance[client] < Squaredf(384.0) && TF2_IsPlayerInCondition(m_nearestFriend[client], TFCond_OnFire))
	{
		if (IsWeaponSlotActive(client, 0))
		{
			m_lookAt[client] = GetEyePosition(m_nearestFriend[client]);
			if (m_friendDistance[client] <= Squaredf(192.0))
				m_attack2Timer[client] = GetGameTime() + 0.2;
			else
				MoveTo(client, GetOrigin(m_nearestFriend[client]));
			return;
		}
		else
			EquipWeaponSlot(client, 0);
	}
	else if (m_class[client] == TFClass_Pyro && m_secondaryID[client] == 595 && m_hasFriendsNear[client] && m_friendDistance[client] < Squaredf(384.0) && TF2_IsPlayerInCondition(m_nearestFriend[client], TFCond_OnFire))
	{
		if (IsWeaponSlotActive(client, 1))
		{
			m_lookAt[client] = GetEyePosition(m_nearestFriend[client]);
			if (m_friendDistance[client] <= Squaredf(192.0))
				m_buttons[client] |= IN_ATTACK2;
			else
				MoveTo(client, GetOrigin(m_nearestFriend[client]));
			return;
		}
		else
			EquipWeaponSlot(client, 1);
	}

	if (m_hasEnemiesNear[client] || m_hasEntitiesNear[client])
	{
		if (m_class[client] == TFClass_Engineer || m_class[client] == TFClass_Medic || m_class[client] == TFClass_Spy)
		{
			if (m_hasEnemiesNear[client] && m_hasEntitiesNear[client] && IsValidEntity(m_nearestEntity[client]))
			{
				float center[3];
				center = GetCenter(m_nearestEntity[client]);

				if (m_enemyDistance[client] < GetVectorDistance(GetOrigin(client), center, true))
					LookAtEnemiens(client);
				else
					m_lookAt[client] = center;

				if (CurrentProcess[client] != PRO_SPYSAP)
				{
					AutoAttack(client);
					SelectBestCombatWeapon(client);
				}

				m_pauseTime[client] = GetGameTime() + crandomfloat(1.5, 2.5);
			}
			else if (m_hasEnemiesNear[client])
			{
				LookAtEnemiens(client);
				AutoAttack(client);
				SelectBestCombatWeapon(client);
				m_pauseTime[client] = GetGameTime() + crandomfloat(1.5, 2.5);
			}
			else if (m_hasEntitiesNear[client] && IsValidEntity(m_nearestEntity[client]))
			{
				m_lookAt[client] = GetCenter(m_nearestEntity[client]);

				if (CurrentProcess[client] != PRO_SPYSAP)
				{
					AutoAttack(client);
					SelectBestCombatWeapon(client);
				}

				m_pauseTime[client] = GetGameTime() + crandomfloat(1.5, 2.5);
			}
		}

		return;
	}

	if (lookaround)
		LookAround(client);
}
