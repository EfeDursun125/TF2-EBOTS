#include <sourcemod>
#include <tf2_stocks>

#pragma semicolon 1
#pragma newdecls required
#define PLUGIN_VERSION "1.21"

#define TFMaxPlayers 34 // set this value more than your server's max player number, more = higher ram usage
int randomf2phat;

int g_iResourceEntity;
bool g_bTouched[TFMaxPlayers];
bool g_bSuddenDeathMode;
bool g_bMVM;
bool g_bMedi;
bool g_bLateLoad;
ConVar g_hCVTimer;
ConVar g_hCVEnabled;
ConVar g_hCVTeam;
ConVar g_hCVMVMSupport;
Handle g_hWWeaponEquip;

int scoutPrimary[TFMaxPlayers];
int scoutSecondary[TFMaxPlayers];
int scoutMelee[TFMaxPlayers];
int soldierPrimary[TFMaxPlayers];
int soldierSecondary[TFMaxPlayers];
int soldierMelee[TFMaxPlayers];
int pyroPrimary[TFMaxPlayers];
int pyroSecondary[TFMaxPlayers];
int pyroMelee[TFMaxPlayers];
int engineerPrimary[TFMaxPlayers];
int engineerSecondary[TFMaxPlayers];
int engineerMelee[TFMaxPlayers];
int heavyPrimary[TFMaxPlayers];
int heavySecondary[TFMaxPlayers];
int heavyMelee[TFMaxPlayers];
int demomanPrimary[TFMaxPlayers];
int demomanSecondary[TFMaxPlayers];
int demomanMelee[TFMaxPlayers];
int medicPrimary[TFMaxPlayers];
int medicSecondary[TFMaxPlayers];
int medicMelee[TFMaxPlayers];
int sniperPrimary[TFMaxPlayers];
int sniperSecondary[TFMaxPlayers];
int sniperMelee[TFMaxPlayers];
int spyPrimary[TFMaxPlayers];
int spyMelee[TFMaxPlayers];
//int spySapper[TFMaxPlayers]; // ERROR!
int spyWatch[TFMaxPlayers];

int scoutLoadout[TFMaxPlayers];
int soldierLoadout[TFMaxPlayers];
int pyroLoadout[TFMaxPlayers];
int engineerLoadout[TFMaxPlayers];
int heavyLoadout[TFMaxPlayers];
int demomanLoadout[TFMaxPlayers];
int medicLoadout[TFMaxPlayers];
int sniperLoadout[TFMaxPlayers];
int spyLoadout[TFMaxPlayers];

public Plugin myinfo = 
{
	name = "Give Bots Loadouts",
	author = "luki1412, EfeDursun125",
	description = "Gives TF2 bots non-stock loadouts",
	version = PLUGIN_VERSION,
	url = "https://forums.alliedmods.net/member.php?u=43109"
}

public APLRes AskPluginLoad2(Handle myself, bool late, char[] error, int err_max) 
{
	if (GetEngineVersion() != Engine_TF2) 
	{
		Format(error, err_max, "This plugin only works for Team Fortress 2.");
		return APLRes_Failure;
	}

	g_bLateLoad = late;
	return APLRes_Success;
}

public void OnPluginStart() 
{
	ConVar hCVversioncvar = CreateConVar("sm_gbl_version", PLUGIN_VERSION, "Give Bots Weapons version cvar", FCVAR_NOTIFY|FCVAR_DONTRECORD); 
	g_hCVEnabled = CreateConVar("sm_gbl_enabled", "1", "Enables/disables this plugin", FCVAR_NONE, true, 0.0, true, 1.0);
	g_hCVTimer = CreateConVar("sm_gbl_delay", "0.111", "Delay for giving weapons to bots", FCVAR_NONE, true, 0.1, true, 30.0);
	g_hCVTeam = CreateConVar("sm_gbl_team", "1", "Team to give weapons to: 1-both, 2-red, 3-blu", FCVAR_NONE, true, 1.0, true, 3.0);
	g_hCVMVMSupport = CreateConVar("sm_gbw_mvm", "1", "Enables/disables giving bots weapons when MVM mode is enabled", FCVAR_NONE, true, 0.0, true, 1.0);
	
	HookEvent("post_inventory_application", player_inv);
	HookEvent("teamplay_round_stalemate", EventSuddenDeath, EventHookMode_PostNoCopy);
	HookEvent("teamplay_round_start", EventRoundReset, EventHookMode_PostNoCopy);
	HookConVarChange(g_hCVEnabled, OnEnabledChanged);
	
	SetConVarString(hCVversioncvar, PLUGIN_VERSION);
	AutoExecConfig(true, "Give_Bots_Weapons");

	if (g_bLateLoad)
		OnMapStart();
	
	GameData hGameConfig = LoadGameConfigFile("sm-tf2.games");
	if (!hGameConfig)
		SetFailState("Failed to find sm-tf2.games.txt gamedata! Can't continue.");

	StartPrepSDKCall(SDKCall_Player);
	PrepSDKCall_SetVirtual(hGameConfig.GetOffset("RemoveWearable") - 1); // EquipWearable offset is always behind RemoveWearable, subtract its value by 1
	PrepSDKCall_AddParameter(SDKType_CBaseEntity, SDKPass_Pointer);
	g_hWWeaponEquip = EndPrepSDKCall();
	if (!g_hWWeaponEquip)
		SetFailState("Failed to prepare the SDKCall for giving weapons. Try updating gamedata or restarting your server.");

	if (hGameConfig)
		delete hGameConfig;

	// for the plugin reload
	int i;
	for (i = 0; i <= MaxClients; i++)
	{
		if (!IsPlayerHere(i))
			continue;

		if (!IsFakeClient(i))
			continue;

		SetupLoadouts(i);
	}
}

public void OnEnabledChanged(ConVar convar, const char[] oldValue, const char[] newValue)
{
	if (GetConVarBool(g_hCVEnabled))
	{
		HookEvent("post_inventory_application", player_inv);
		HookEvent("teamplay_round_stalemate", EventSuddenDeath, EventHookMode_PostNoCopy);
		HookEvent("teamplay_round_start", EventRoundReset, EventHookMode_PostNoCopy);
	}
	else
	{
		UnhookEvent("post_inventory_application", player_inv);
		UnhookEvent("teamplay_round_stalemate", EventSuddenDeath, EventHookMode_PostNoCopy);
		UnhookEvent("teamplay_round_start", EventRoundReset, EventHookMode_PostNoCopy);
	}
}

int g_seed;
public void OnMapStart()
{
	if (GameRules_GetProp("m_bPlayingMannVsMachine"))
		g_bMVM = true;
	else
		g_bMVM = false;

	if (GameRules_GetProp("m_bPlayingMedieval"))
		g_bMedi = true;
	else
		g_bMedi = false;

	g_iResourceEntity = GetPlayerResourceEntity();
	g_seed += GetTime();
	randomf2phat = crandomint(1, 7);
}

stock int frand()
{
	g_seed = (214013 * g_seed + 2531011);
	return (g_seed >> 16) & 32767;
}

stock int crandomint(const int min, const int max)
{
	return frand() % (max - min + 1) + min;
}

public void OnClientPutInServer(int client)
{
	SetupLoadouts(client);
}

stock void SetupLoadouts(const int client)
{
	scoutLoadout[client] = crandomint(1, 33);
	soldierLoadout[client] = crandomint(1, 43);
	pyroLoadout[client] = crandomint(1, 35);
	engineerLoadout[client] = crandomint(1, 24);
	heavyLoadout[client] = crandomint(1, 18);
	demomanLoadout[client] = crandomint(1, 17);
	medicLoadout[client] = crandomint(1, 23);
	sniperLoadout[client] = crandomint(1, 34);
	spyLoadout[client] = crandomint(1, 19);
	SetupWeapons(client);
}

public void OnClientDisconnect(int client)
{
	g_bTouched[client] = false;
}

public void player_inv(Handle event, const char[] name, bool dontBroadcast) 
{
	if (!GetConVarBool(g_hCVEnabled))
		return;
	
	if (!g_bSuddenDeathMode && (!g_bMVM || GetConVarBool(g_hCVMVMSupport)))
	{
		int client = GetClientOfUserId(GetEventInt(event, "userid"));
		if (!IsPlayerHere(client) || g_bTouched[client])
			return;
		
		g_bTouched[client] = true;
		switch (GetConVarInt(g_hCVTeam))
		{
			case 1:
				CreateTimer(GetConVarFloat(g_hCVTimer), Timer_GiveWeapons, client, TIMER_FLAG_NO_MAPCHANGE);
			case 2:
			{
				if (GetClientTeam(client) == 2)
					CreateTimer(GetConVarFloat(g_hCVTimer), Timer_GiveWeapons, client, TIMER_FLAG_NO_MAPCHANGE);
			}
			case 3:
			{
				if (GetClientTeam(client) == 3)
					CreateTimer(GetConVarFloat(g_hCVTimer), Timer_GiveWeapons, client, TIMER_FLAG_NO_MAPCHANGE);
			}
		}
	}
}

public Action Timer_GiveWeapons(Handle timer, any data)
{
	int client = data;
	if (!IsPlayerHere(client))
		return Plugin_Continue;

	g_bTouched[client] = false;
	switch (GetConVarInt(g_hCVTeam))
	{
		case 2:
		{
			if (GetClientTeam(client) != 2)
				return Plugin_Continue;
		}
		case 3:
		{
			if (GetClientTeam(client) != 3)
				return Plugin_Continue;
		}
	}

	if (!g_bSuddenDeathMode && (!g_bMVM || GetConVarBool(g_hCVMVMSupport)))
	{
		if (!g_bMedi)
		{
			switch (TF2_GetPlayerClass(client))
			{
				case TFClass_Scout:
				{
					switch (scoutPrimary[client])
					{
						case 220:
							CreateWeapon(client, "tf_weapon_handgun_scout_primary", 0, scoutPrimary[client]);
						case 448:
							CreateWeapon(client, "tf_weapon_soda_popper", 0, scoutPrimary[client]);
						case 772:
							CreateWeapon(client, "tf_weapon_pep_brawler_blaster", 0, scoutPrimary[client]);
						default:
							CreateWeapon(client, "tf_weapon_scattergun", 0, scoutPrimary[client]);
					}

					switch (scoutSecondary[client])
					{
						case 46:
							CreateWeapon(client, "tf_weapon_lunchbox_drink", 1, scoutSecondary[client]);
						case 163:
							CreateWeapon(client, "tf_weapon_lunchbox_drink", 1, scoutSecondary[client]);
						case 222:
							CreateWeapon(client, "tf_weapon_jar_milk", 1, scoutSecondary[client]);
						case 449:
							CreateWeapon(client, "tf_weapon_handgun_scout_secondary", 1, scoutSecondary[client]);
						case 773:
							CreateWeapon(client, "tf_weapon_handgun_scout_secondary", 1, scoutSecondary[client]);
						case 812:
							CreateWeapon(client, "tf_weapon_cleaver", 1, scoutSecondary[client]);
						case 1121:
							CreateWeapon(client, "tf_weapon_jar_milk", 1, scoutSecondary[client]);
						case 1145:
							CreateWeapon(client, "tf_weapon_lunchbox_drink", 1, scoutSecondary[client]);
						default:
							CreateWeapon(client, "tf_weapon_pistol", 1, scoutSecondary[client]);
					}

					switch (scoutMelee[client])
					{
						case 44:
							CreateWeapon(client, "tf_weapon_bat_wood", 2, scoutMelee[client]);
						case 221:
							CreateWeapon(client, "tf_weapon_bat_fish", 2, scoutMelee[client]);
						case 572:
							CreateWeapon(client, "tf_weapon_bat_fish", 2, scoutMelee[client]);
						case 648:
							CreateWeapon(client, "tf_weapon_bat_giftwrap", 2, scoutMelee[client]);
						case 999:
							CreateWeapon(client, "tf_weapon_bat_fish", 2, scoutMelee[client]);
						default:
							CreateWeapon(client, "tf_weapon_bat", 2, scoutMelee[client]);
					}

					switch (scoutLoadout[client])
					{
						case 1:
						{
							CreateHat(client, 30428); // Pomade Prince
							CreateHat(client, 30376); // Ticket Boy
							CreateHat(client, 30427); // Argyle Ace
						}
						case 2:
						{
							CreateHat(client, 30540); // Brooklyn Booties
							CreateHat(client, 30428); // Pomade Prince
							CreateHat(client, 30552); // Thermal Tracker
						}
						case 3:
						{
							CreateHat(client, 451); // Bonk Boy
							CreateHat(client, 106); // Bonk Helm
							CreateHat(client, 30083); // Caffeine Cooler
						}
						case 4:
						{
							CreateHat(client, 150); // Troublemaker's Tossle Cap
							CreateHat(client, 30552); // Thermal Tracker
							CreateHat(client, 1016); // Buck Turner All-Stars
						}
						case 5:
						{
							CreateHat(client, 780); // Fed-Fightin' Fedora
							CreateHat(client, 781); // Dillinger's Duffel
							CreateHat(client, 343); // Professor Speks
						}
						case 6:
						{
							CreateHat(client, 617); // Backwards Ballcap
							CreateHat(client, 30076); // Bigg Mann on Campus
							CreateHat(client, 1016); // Buck Turner All-Stars
						}
						case 7:
						{
							CreateHat(client, 30636); // Fortunate Son
							CreateHat(client, 30637); // Flak Jack
							CreateHat(client, 30561); // Bootenkhamuns
						}
						case 8:
						{
							CreateHat(client, 30479); // Thirst Blood
							CreateHat(client, 814); // Triad Trinket
							CreateHat(client, 815); // Champ Stamp
						}
						case 9:
						{
							CreateHat(client, 984); // Tough Stuff Muffs
							CreateHat(client, 30134); // Delinquent's Down Vest
							CreateHat(client, 987); // Merc's Muffler
						}
						case 10:
						{
							CreateHat(client, 780); // Fed-Fightin' Fedora
							CreateHat(client, 486); // Summer Shades
							CreateHat(client, 490); // Flip-Flops
						}
						case 11:
						{
							CreateHat(client, 30479); // Thirst Blood
							CreateHat(client, 30060); // Cheet Sheet
							CreateHat(client, 30306); // Dictator
						}
						case 12:
						{
							CreateHat(client, 126); // Bill's Hat
							CreateHat(client, 30397); // Bruiser's Bandana
							CreateHat(client, 30426); // Paisley Pro
						}
						case 13:
						{
							CreateHat(client, 30573); // Mountebank's Masque
							CreateHat(client, 30574); // Courtier's Collar
							CreateHat(client, 30575); // Harlequin's Hooves
						}
						case 14:
						{
							CreateHat(client, 652); // Big Elfin Deal
							CreateHat(client, 653); // Bootie Time
							CreateHat(client, 1075); // Sack Fulla Smissmas
						}
						case 15:
						{
							CreateHat(client, 150); // Troublemaker's Tossle Cap
							CreateHat(client, 722); // Fast Learner
							CreateHat(client, 143); // Earbuds
						}
						case 16:
						{
							CreateHat(client, 30428); // Pomade Prince
							CreateHat(client, 30552); // Thermal Tracker
							CreateHat(client, 30540); // Brooklyn Booties
						}
						case 17:
						{
							CreateHat(client, 30428); // Pomade Prince
							CreateHat(client, 30189); // Frenchman's Formals
							CreateHat(client, 30427); // Argyle Ace
						}
						case 18:
						{
							CreateHat(client, 451); // Bonk boy
							CreateHat(client, 30185); // Flapjack
							CreateHat(client, 30540); // Brooklyn Booties
						}
						case 19:
						{
							CreateHat(client, 30119); // Federal Casemaker
							CreateHat(client, 30077); // Cool Cat Cardigan
							CreateHat(client, 30427); // Argyle Ace
						}
						case 20:
						{
							CreateHat(client, 150); // Troublemaker's Tossle Cap
							CreateHat(client, 30178); // Weight Room Warmer
							CreateHat(client, 30540); // Brooklyn Booties
						}
						case 21:
						{
							CreateHat(client, 30471); // Alien Cranium
							CreateHat(client, 30470); // Biomech Backpack
							CreateHat(client, 30472); // Xeno Suit
						}
						case 22:
						{
							CreateHat(client, 107); // Ye Olde Baker Boy
							CreateHat(client, 343); // Professor Speks
							CreateHat(client, 30309); // Dead of Night
						}
						case 23:
						{
							CreateHat(client, 453); // Hero's Tail
							CreateHat(client, 30661); // Cadet Visor
							CreateHat(client, 30552); // Thermal Tracker
						}
						case 24:
						{
							CreateHat(client, 30394); // Frickin' Sweet Ninja Hood
							CreateHat(client, 30395); // Southie Shinobi
							CreateHat(client, 30396); // Red Socks
						}
						case 25:
						{
							CreateHat(client, 143); // Earbuds
							CreateHat(client, 30479); // Thirst Blood
							CreateHat(client, 30552); // Thermal Tracker
						}
						case 26:
						{
							CreateHat(client, 30325); // Little Drummer Mann
							CreateHat(client, 30326); // Scout Shako
							CreateHat(client, 5617); // Voodoo-Cursed Scout Soul
						}
						case 27:
						{
							CreateHat(client, 30496); // Crazy Legs
							CreateHat(client, 30495); // Claws and Infect
							CreateHat(client, 30494); // Head Hunter
						}
						case 28:
						{
							CreateHat(client, 30077); // Cool Cat Cardigan
							CreateHat(client, 781); // Dillinger's Duffel
							CreateHat(client, 780); // Fed-Fightin' Fedora
						}
						case 29:
						{
							CreateHat(client, 617); // Backwards Ballcap
							CreateHat(client, 30027); // Bolt Boy
							CreateHat(client, 722); // Fast Learner
						}
						case 30:
						{
							CreateHat(client, 30357); // Dark Falkirk Helm
							CreateHat(client, 30358); // Sole Saviors
							CreateHat(client, 30189); // Frenchman's Formals
						}
						case 31:
						{
							CreateHat(client, 30185); // Flapjack
							CreateHat(client, 30177); // Hong Kong Cone
							CreateHat(client, 983); // Digit Divulger
						}
						case 32:
						{
							CreateHat(client, 539); // El Jefe
							CreateHat(client, 30309); // Dead of Night
							CreateHat(client, 30551); // Flashdance Footies
						}
						default:
							RandomF2Phat(client);
					}
				}
				case TFClass_Sniper:
				{
					switch (sniperPrimary[client])
					{
						case 56:
							CreateWeapon(client, "tf_weapon_compound_bow", 0, sniperPrimary[client]);
						case 402:
							CreateWeapon(client, "tf_weapon_sniperrifle_decap", 0, sniperPrimary[client]);
						case 1005:
							CreateWeapon(client, "tf_weapon_compound_bow", 0, sniperPrimary[client]);
						case 1092:
							CreateWeapon(client, "tf_weapon_compound_bow", 0, sniperPrimary[client]);
						case 1098:
							CreateWeapon(client, "tf_weapon_sniperrifle_classic", 0, sniperPrimary[client]);
						default:
							CreateWeapon(client, "tf_weapon_sniperrifle", 0, sniperPrimary[client]);
					}

					switch (sniperSecondary[client])
					{
						case 57:
							CreateWeapon(client, "tf_wearable_razorback", 1, sniperSecondary[client], true);
						case 58:
							CreateWeapon(client, "tf_weapon_jar", 1, sniperSecondary[client]);
						case 231:
							CreateWeapon(client, "tf_wearable", 1, sniperSecondary[client], true);
						case 642:
							CreateWeapon(client, "tf_wearable", 1, sniperSecondary[client], true);
						case 751:
							CreateWeapon(client, "tf_weapon_charged_smg", 1, sniperSecondary[client]);
						case 1083:
							CreateWeapon(client, "tf_weapon_jar", 1, sniperSecondary[client]);
						case 1105:
							CreateWeapon(client, "tf_weapon_jar", 1, sniperSecondary[client]);
						default:
							CreateWeapon(client, "tf_weapon_smg", 1, sniperSecondary[client]);
					}

					CreateWeapon(client, "tf_weapon_club", 2, sniperMelee[client]);

					switch (sniperLoadout[client])
					{
						case 1:
						{
							CreateHat(client, 921); // Executioner
							CreateHat(client, 393); // Villain's Veil
							CreateHat(client, 343); // Professor Speks
						}
						case 2:
						{
							CreateHat(client, 981); // Cold Killer
							CreateHat(client, 30328); // Extra Layer
							CreateHat(client, 30551); // Flashdance Footies
						}
						case 3:
						{
							CreateHat(client, 30135); // Wet Works
							CreateHat(client, 30317); // Five-Month Shadow
							CreateHat(client, 30649); // Final Fontiersman
						}
						case 4:
						{
							CreateHat(client, 600); // Your Worst Nightmare
							CreateHat(client, 30550); // Snow Sleeves
							CreateHat(client, 646); // Itsy Bitsy Spyer
						}
						case 5:
						{
							CreateHat(client, 766); // The Doublecross-Comm
							CreateHat(client, 30546); // Boxcar Bomber
							CreateHat(client, 30310); // Snow Scoper
						}
						case 6:
						{
							CreateHat(client, 762); // Flamingo Kid
							CreateHat(client, 766); // Doublecross-Comm
							CreateHat(client, 30650); // Starduster
						}
						case 7:
						{
							CreateHat(client, 626); // Swagman's Swatter
							CreateHat(client, 30650); // Starduster
							CreateHat(client, 30284); // Sir Shootsalot
						}
						case 8:
						{
							CreateHat(client, 30373); // Toowoomba Tunic
							CreateHat(client, 314); // Larrikin Robin
							CreateHat(client, 1094); // Criminal Cloak
						}
						case 9:
						{
							CreateHat(client, 492); // Summer Hat
							CreateHat(client, 486); // Summer Shades
							CreateHat(client, 814); // Triad Trinket
						}
						case 10:
						{
							CreateHat(client, 626); // Swagman's Swatter
							CreateHat(client, 30317); // Five-Month Shadow
							CreateHat(client, 30424); // Triggerman's Tacticals
						}
						case 11:
						{
							CreateHat(client, 671); // Brown Bomber
							CreateHat(client, 30306); // Dictator
							CreateHat(client, 500); // ETF2L Highlander 2nd
						}
						case 12:
						{
							CreateHat(client, 30085); // Macho Mann
							CreateHat(client, 30324); // Golden Garment
							CreateHat(client, 819); // Lone Star
						}
						case 13:
						{
							CreateHat(client, 762); // Salty Dog
							CreateHat(client, 30317); // Five-Month Shadow
							CreateHat(client, 645); // Outback Intellectual
						}
						case 14:
						{
							CreateHat(client, 30413); // Merc's Mohawk
							CreateHat(client, 30597); // Bushman's Bristles
							CreateHat(client, 30309); // Dead of Night
						}
						case 15:
						{
							CreateHat(client, 30373); // Toowoomba Tunic
							CreateHat(client, 30650); // Starduster
							CreateHat(client, 626); // Swagman's Swatter
						}
						case 16:
						{
							CreateHat(client, 518); // Anger
							CreateHat(client, 30599); // Marksman's Mohair
							CreateHat(client, 30309); // Dead of Night
						}
						case 17:
						{
							CreateHat(client, 263); // Ellis' Hat
							CreateHat(client, 5625); // Voodoo-Cursed Sniper Soul
							CreateHat(client, 30414); // Eye-Catcher
						}
						case 18:
						{
							CreateHat(client, 600); // Your Worst Nightmare
							CreateHat(client, 30100); // Birdman of Australiacatraz
							CreateHat(client, 30068); // Breakneck Baggies
						}
						case 19:
						{
							CreateHat(client, 30397); // Bruiser's Bandana
							CreateHat(client, 263); // Ellis' Hat
							CreateHat(client, 30170); // Chronomancer
						}
						case 20:
						{
							CreateHat(client, 626); // Swagman's Swatter
							CreateHat(client, 30284); // Sir Shootsalot
							CreateHat(client, 30170); // Chronomancer
						}
						case 21:
						{
							CreateHat(client, 30324); // Golden Garment
							CreateHat(client, 30316); // Toy Soldier
							CreateHat(client, 30306); // Dictator
						}
						case 22:
						{
							CreateHat(client, 30005); // Shooter's Tin Topi
							CreateHat(client, 30478); // Poacher's Safari Jacket
							CreateHat(client, 30424); // Triggerman's Tacticals
						}
						case 23:
						{
							CreateHat(client, 30598); // Professional's Ushanka
							CreateHat(client, 766); // Doublecross-Comm
							CreateHat(client, 30649); // Final Fontiersman
						}
						case 24:
						{
							CreateHat(client, 1076); // Smissmas Caribou
							CreateHat(client, 783); // HazMat Headcase
							CreateHat(client, 30649); // Final Fontiersman
						}
						case 25:
						{
							CreateHat(client, 30623); // Rotation Sensation
							CreateHat(client, 30309); // Dead of Night
							CreateHat(client, 647); // All-Father
						}
						case 26:
						{
							CreateHat(client, 30135); // Wet Works
							CreateHat(client, 814); // Triad Trinket
							CreateHat(client, 646); // Itsy Bitsy Spyer
						}
						case 27:
						{
							CreateHat(client, 877); // Stovepipe Sniper Shako
							CreateHat(client, 30324); // Golden Garment
							CreateHat(client, 855); // Vigilant Pin
						}
						case 28:
						{
							CreateHat(client, 631); // Hat With No Name
							CreateHat(client, 30100); // Birdman of Australiacatraz
							CreateHat(client, 30629); // Support Spurs
						}
						case 29:
						{
							CreateHat(client, 819); // Lone Star
							CreateHat(client, 30100); // Birdman of Australiacatraz
							CreateHat(client, 734); // Teufort Tooth Kicker
						}
						case 30:
						{
							CreateHat(client, 564); // Holy Hunter
							CreateHat(client, 5625); // Voodoo-Cursed Sniper Soul
							CreateHat(client, 30317); // Five-Month Shadow
						}
						case 31:
						{
							CreateHat(client, 117); // Ritzy Rick's Hair Fixative
							CreateHat(client, 30309); // Dead of Night
							CreateHat(client, 30306); // Dictator
						}
						case 32:
						{
							CreateHat(client, 762); // Flamingo Kid
							CreateHat(client, 30170); // Chronomancer
							CreateHat(client, 618); // Crocodile Smile
						}
						case 33:
						{
							CreateHat(client, 30549); // Winter Woodsman
							CreateHat(client, 30550); // Snow Sleeves
							CreateHat(client, 1011); // Tux
						}
						default:
							RandomF2Phat(client);
					}
				}
				case TFClass_Soldier:
				{
					switch (soldierPrimary[client])
					{
						case 127:
							CreateWeapon(client, "tf_weapon_rocketlauncher_directhit", 0, soldierPrimary[client]);
						case 441:
							CreateWeapon(client, "tf_weapon_particle_cannon", 0, soldierPrimary[client]);
						case 1104:
							CreateWeapon(client, "tf_weapon_rocketlauncher_airstrike", 0, soldierPrimary[client]);
						default:
							CreateWeapon(client, "tf_weapon_rocketlauncher", 0, soldierPrimary[client]);
					}

					switch (soldierSecondary[client])
					{
						case 129:
							CreateWeapon(client, "tf_weapon_buff_item", 1, soldierSecondary[client]);
						case 133:
							CreateWeapon(client, "tf_wearable", 1, soldierSecondary[client], true);
						case 226:
							CreateWeapon(client, "tf_weapon_buff_item", 1, soldierSecondary[client]);
						case 354:
							CreateWeapon(client, "tf_weapon_buff_item", 1, soldierSecondary[client]);
						case 442:
							CreateWeapon(client, "tf_weapon_raygun", 1, soldierSecondary[client]);
						case 444:
							CreateWeapon(client, "tf_wearable", 1, soldierSecondary[client], true);
						case 1001:
							CreateWeapon(client, "tf_weapon_buff_item", 1, soldierSecondary[client]);
						case 1101:
							CreateWeapon(client, "tf_weapon_parachute", 1, soldierSecondary[client]);
						default:
							CreateWeapon(client, "tf_weapon_shotgun_soldier", 1, soldierSecondary[client]);
					}

					switch (soldierMelee[client])
					{
						case 357:
							CreateWeapon(client, "tf_weapon_katana", 2, soldierMelee[client]);
						default:
							CreateWeapon(client, "tf_weapon_shovel", 2, soldierMelee[client]);
					}

					switch (soldierLoadout[client])
					{
						case 1:
						{
							CreateHat(client, 125); // Cheater's Lament
							CreateHat(client, 936); // Exorcizor
							CreateHat(client, 30397); // Bruiser's Bandana
						}
						case 2:
						{
							CreateHat(client, 30316); // Toy Soldier
							CreateHat(client, 30306); // Dictator
							CreateHat(client, 30129); // Hornblower
						}
						case 3:
						{
							CreateHat(client, 30069); // Powdered Practitioner
							CreateHat(client, 30142); // Founding Father
							CreateHat(client, 30117); // Colonial Clogs
						}
						case 4:
						{
							CreateHat(client, 852); // Soldier's Stogie
							CreateHat(client, 54); // Soldier's Stash
							CreateHat(client, 701); // Lucky Shot
						}
						case 5:
						{
							CreateHat(client, 340); // Defiant Spartan
							CreateHat(client, 30126); // Shotgun's Shoulder Guard
							CreateHat(client, 1025); // Fortune Hunter
						}
						case 6:
						{
							CreateHat(client, 1899); // World Traveler
							CreateHat(client, 30129); // Hornblower
							CreateHat(client, 30397); // Bruiser's Bandana
						}
						case 7:
						{
							CreateHat(client, 445); // Armored Authority
							CreateHat(client, 446); // Fancy Dress Uniform
							CreateHat(client, 30388); // Classified Coif
						}
						case 8:
						{
							CreateHat(client, 378); // Team Captain
							CreateHat(client, 30388); // Classified Coif
							CreateHat(client, 30115); // Compatriot
						}
						case 9:
						{
							CreateHat(client, 30251); // Cadaver's Capper
							CreateHat(client, 30281); // Shaolin Sash
							CreateHat(client, 5618); // Voodoo-Cursed Soldier Soul
						}
						case 10:
						{
							CreateHat(client, 152); // Killer's Kabuto
							CreateHat(client, 875); // Menpo
							CreateHat(client, 30126); // Shotgun's Shoulder Guard
						}
						case 12:
						{
							CreateHat(client, 183); // Sergeant's Drill Hat
							CreateHat(client, 446); // Fancy Dress Uniform
							CreateHat(client, 30335); // Marshall's Mutton Chops
						}
						case 13:
						{
							CreateHat(client, 152); // Killer's Kabuto
							CreateHat(client, 30126); // Shotgun's Shoulder Guard
							CreateHat(client, 1025); // Fortune Hunter
						}
						case 14:
						{
							CreateHat(client, 516); // Stahlhelm
							CreateHat(client, 446); // Fancy Dress Uniform
							CreateHat(client, 30339); // Killer's Kit
						}
						case 15:
						{
							CreateHat(client, 30306); // Dictator
							CreateHat(client, 439); // Lord Cockswain's Pith Helmet
							CreateHat(client, 30388); // Classified Coif
						}
						case 16:
						{
							CreateHat(client, 631); // Hat With No Name
							CreateHat(client, 30388); // Classified Coif
							CreateHat(client, 30554); // Mistaken Movember
						}
						case 17:
						{
							CreateHat(client, 701); // Lucky Shot
							CreateHat(client, 852); // Soldier's Stogie
							CreateHat(client, 30477); // Lone Survivor
						}
						case 18:
						{
							CreateHat(client, 30601); // Cold Snap Coat
							CreateHat(client, 984); // Tough Stuff Muffs
							CreateHat(client, 30558); // Coldfront Curbstompers
						}
						case 19:
						{
							CreateHat(client, 944); // That '70s Chapeau
							CreateHat(client, 30309); // Dead of Night
							CreateHat(client, 852); // Soldier's Stogie
						}
						case 20:
						{
							CreateHat(client, 30338); // Ground Control
							CreateHat(client, 30388); // Classified Coif
							CreateHat(client, 30339); // Killer's Kit
						}
						case 21:
						{
							CreateHat(client, 378); // Team Captain
							CreateHat(client, 143); // Earbuds
							CreateHat(client, 30388); // Classified Coif
						}
						case 22:
						{
							CreateHat(client, 391); // Spook Specs
							CreateHat(client, 143); // Earbuds
							CreateHat(client, 30388); // Classified Coif
						}
						case 23:
						{
							CreateHat(client, 289); // Voodoo Juju
							CreateHat(client, 647); // All-Father
							CreateHat(client, 30129); // Hornblower
						}
						case 24:
						{
							CreateHat(client, 30085); // Macho Mann
							CreateHat(client, 183); // Sergeant's Drill Hat
							CreateHat(client, 446); // Fancy Dress Uniform
						}
						case 25:
						{
							CreateHat(client, 631); // Hat With No Name
							CreateHat(client, 30558); // Coldfront Curbstompers
							CreateHat(client, 30554); // Mistaken Movember
						}
						case 26:
						{
							CreateHat(client, 30120); // Rebel Rouser
							CreateHat(client, 30129); // Hornblower
							CreateHat(client, 30569); // Tomb Readers
						}
						case 27:
						{
							CreateHat(client, 30554); // Mistaken Movember
							CreateHat(client, 829); // War Pig
							CreateHat(client, 30601); // Cold Snap Coat
						}
						case 28:
						{
							CreateHat(client, 378); // Team Captain
							CreateHat(client, 650); // Kringle Collection
							CreateHat(client, 446); // Fancy Dress Uniform
						}
						case 29:
						{
							CreateHat(client, 666); // B.M.O.C.
							CreateHat(client, 647); // All-Father
							CreateHat(client, 650); // Kringle Collection
						}
						case 30:
						{
							CreateHat(client, 162); // Max's Severed Head
							CreateHat(client, 30115); // Compatriot
							CreateHat(client, 30130); // Lieutenant Bites
						}
						case 31:
						{
							CreateHat(client, 250); // Chieftain's Challenge
							CreateHat(client, 30140); // Virtual Viewfinder
							CreateHat(client, 647); // All-Father
						}
						case 32:
						{
							CreateHat(client, 634); // Point and Shoot
							CreateHat(client, 30126); // Shotgun's Shoulder Guard
							CreateHat(client, 339); // Exquisite Rack
						}
						case 33:
						{
							CreateHat(client, 30390); // Spook Specs
							CreateHat(client, 647); // All-Father
							CreateHat(client, 30129); // Hornblower
						}
						case 34:
						{
							CreateHat(client, 30623); // Rotation Sensation
							CreateHat(client, 486); // Summer Shades
							CreateHat(client, 30554); // Mistaken Movember
						}
						case 35:
						{
							CreateHat(client, 30335); // Marshall's Mutton Chops
							CreateHat(client, 1093); // Gilded Guard
							CreateHat(client, 30129); // Hornblower
						}
						case 36:
						{
							CreateHat(client, 986); // Mutton Mann
							CreateHat(client, 611); // Salty Dog
							CreateHat(client, 30414); // Eye-Catcher
						}
						case 37:
						{
							CreateHat(client, 30120); // Rebel Rouser
							CreateHat(client, 30129); // Hornblower
							CreateHat(client, 30554); // Mistaken Movember
						}
						case 38:
						{
							CreateHat(client, 240); // Lumbricus Lid
							CreateHat(client, 54); // Soldier's Stash
							CreateHat(client, 701); // Lucky Shot
						}
						case 39:
						{
							CreateHat(client, 440); // Lord Cockswain's Novelty Mutton Chops and Pipe
							CreateHat(client, 378); // Team Captain
							CreateHat(client, 30339); // Killer's Kit
						}
						case 40:
						{
							CreateHat(client, 30116); // Caribbean Conqueror
							CreateHat(client, 30554); // Mistaken Movember
							CreateHat(client, 30131); // Brawling Buccaneer
						}
						case 41:
						{
							CreateHat(client, 391); // Honcho's Headgear
							CreateHat(client, 30388); // Classified Coif
							CreateHat(client, 30339); // Killer's Kit
						}
						case 42:
						{
							CreateHat(client, 1021); // Doe-Boy
							CreateHat(client, 440); // Lord Cockswain's Novelty Mutton Chops and Pipe
							CreateHat(client, 446); // Fancy Dress Uniform
						}
						default:
							RandomF2Phat(client);
					}
				}
				case TFClass_DemoMan:
				{
					switch (demomanPrimary[client])
					{
						case 405:
							CreateWeapon(client, "tf_wearable", 0, demomanPrimary[client], true);
						case 608:
							CreateWeapon(client, "tf_wearable", 0, demomanPrimary[client], true);
						case 996:
							CreateWeapon(client, "tf_weapon_cannon", 0, demomanPrimary[client]);
						case 1101:
							CreateWeapon(client, "tf_weapon_parachute", 0, demomanPrimary[client]);
						default:
							CreateWeapon(client, "tf_weapon_grenadelauncher", 0, demomanPrimary[client]);
					}

					switch (demomanSecondary[client])
					{
						case 131:
							CreateWeapon(client, "tf_wearable_demoshield", 1, demomanSecondary[client], true);
						case 406:
							CreateWeapon(client, "tf_wearable_demoshield", 1, demomanSecondary[client], true);
						case 1099:
							CreateWeapon(client, "tf_wearable_demoshield", 1, demomanSecondary[client], true);
						case 1144:
							CreateWeapon(client, "tf_wearable_demoshield", 1, demomanSecondary[client], true);
						default:
							CreateWeapon(client, "tf_weapon_pipebomblauncher", 1, demomanSecondary[client]);
					}

					switch (demomanMelee[client])
					{
						case 132:
							CreateWeapon(client, "tf_weapon_sword", 2, demomanMelee[client]);
						case 154:
							CreateWeapon(client, "tf_weapon_shovel", 2, demomanMelee[client]);
						case 172:
							CreateWeapon(client, "tf_weapon_sword", 2, demomanMelee[client]);
						case 266:
							CreateWeapon(client, "tf_weapon_sword", 2, demomanMelee[client]);
						case 307:
							CreateWeapon(client, "tf_weapon_stickbomb", 2, demomanMelee[client]);
						case 327:
							CreateWeapon(client, "tf_weapon_sword", 2, demomanMelee[client]);
						case 357:
							CreateWeapon(client, "tf_weapon_katana", 2, demomanMelee[client]);
						case 482:
							CreateWeapon(client, "tf_weapon_sword", 2, demomanMelee[client]);
						case 1082:
							CreateWeapon(client, "tf_weapon_sword", 2, demomanMelee[client]);
						default:
							CreateWeapon(client, "tf_weapon_bottle", 2, demomanPrimary[client]);
					}

					switch (demomanLoadout[client])
					{
						case 1:
						{
							CreateHat(client, 261);
						}
						case 2:
						{
							CreateHat(client, 116);
							CreateHat(client, 743);
							CreateHat(client, 583);
						}
						case 3:
						{
							CreateHat(client, 940);
							CreateHat(client, 743);
							CreateHat(client, 583);
						}
						case 4:
						{
							CreateHat(client, 261);
							CreateHat(client, 343);
							CreateHat(client, 165);
						}
						case 5:
						{
							CreateHat(client, 776);
							CreateHat(client, 390);
							CreateHat(client, 845);
						}
						case 6:
						{
							CreateHat(client, 359);
						}
						case 7:
						{
						}
						case 8:
						{
							CreateHat(client, 30429);
							CreateHat(client, 30430);
							CreateHat(client, 30431);
						}
						case 9:
						{
							CreateHat(client, 342);
							CreateHat(client, 874);
						}
						case 10:
						{
							CreateHat(client, 306);
						}
						case 11:
						{
							CreateHat(client, 165);
						}
						case 12:
						{
							CreateHat(client, 30180);
							CreateHat(client, 30110);
							CreateHat(client, 610);
						}
						case 13:
						{
							CreateHat(client, 166);
						}
						case 14:
						{
							CreateHat(client, 845);
							CreateHat(client, 830);
						}
						case 15:
						{

						}
						case 16:
						{
							CreateHat(client, 30357);
						}
						default:
							RandomF2Phat(client);
					}
				}
				case TFClass_Medic:
				{
					switch (medicPrimary[client])
					{
						case 305:
							CreateWeapon(client, "tf_weapon_crossbow", 0, medicPrimary[client]);
						case 1079:
							CreateWeapon(client, "tf_weapon_crossbow", 0, medicPrimary[client]);
						default:
							CreateWeapon(client, "tf_weapon_syringegun_medic", 0, medicPrimary[client]);
					}

					CreateWeapon(client, "tf_weapon_medigun", 1, medicSecondary[client]);
					CreateWeapon(client, "tf_weapon_bonesaw", 2, medicMelee[client]);

					switch (medicLoadout[client])
					{
						case 1:
						{
							CreateHat(client, 261);
						}
						case 2:
						{
							CreateHat(client, 756);
							CreateHat(client, 743);
							CreateHat(client, 583);
						}
						case 3:
						{
							CreateHat(client, 261);
							CreateHat(client, 343);
							CreateHat(client, 165);
						}
						case 4:
						{
							CreateHat(client, 30311);
							CreateHat(client, 30312);
							CreateHat(client, 30410);
						}
						case 5:
						{
							CreateHat(client, 828);
							CreateHat(client, 378);
						}
						case 6:
						{
						}
						case 7:
						{
							CreateHat(client, 388);
							CreateHat(client, 657);
						}
						case 8:
						{
							CreateHat(client, 143);
							CreateHat(client, 101);
							CreateHat(client, 30361);
						}
						case 9:
						{
							CreateHat(client, 30490);
							CreateHat(client, 303);
						}
						case 10:
						{
							CreateHat(client, 165);
						}
						case 11:
						{
							CreateHat(client, 5622);
						}
						case 12:
						{
							CreateHat(client, 432);
							CreateHat(client, 343);
							CreateHat(client, 538);
						}
						case 13:
						{
							CreateHat(client, 166);
						}
						case 14:
						{
							CreateHat(client, 165);
							CreateHat(client, 30119);
							CreateHat(client, 987);
						}
						case 15:
						{
							CreateHat(client, 30066);
							CreateHat(client, 143);
							CreateHat(client, 738);
						}
						case 16:
						{
							CreateHat(client, 378);
							CreateHat(client, 878);
							CreateHat(client, 522);
						}
						case 17:
						{
							CreateHat(client, 261);
							CreateHat(client, 583);
							CreateHat(client, 744);
						}
						case 18:
						{
							CreateHat(client, 778);
							CreateHat(client, 30419);
							CreateHat(client, 978);
						}
						case 19:
						{
							CreateHat(client, 867);
							CreateHat(client, 868);
						}
						case 20:
						{

						}
						case 21:
						{
							CreateHat(client, 101);
							CreateHat(client, 143);
						}
						case 22:
						{
							CreateHat(client, 30357);
						}
						default:
							RandomF2Phat(client);
					}
				}
				case TFClass_Heavy:
				{
					CreateWeapon(client, "tf_weapon_minigun", 0, heavyPrimary[client]);

					switch (heavySecondary[client])
					{
						case 42:
							CreateWeapon(client, "tf_weapon_lunchbox", 1, heavySecondary[client]);
						case 159:
							CreateWeapon(client, "tf_weapon_lunchbox", 1, heavySecondary[client]);
						case 311:
							CreateWeapon(client, "tf_weapon_lunchbox", 1, heavySecondary[client]);
						case 433:
							CreateWeapon(client, "tf_weapon_lunchbox", 1, heavySecondary[client]);
						case 863:
							CreateWeapon(client, "tf_weapon_lunchbox", 1, heavySecondary[client]);
						case 1002:
							CreateWeapon(client, "tf_weapon_lunchbox", 1, heavySecondary[client]);
						case 1190:
							CreateWeapon(client, "tf_weapon_lunchbox", 1, heavySecondary[client]);
						default:
							CreateWeapon(client, "tf_weapon_shotgun_hwg", 1, heavySecondary[client]);
					}

					CreateWeapon(client, "tf_weapon_fists", 2, heavyMelee[client]);

					switch (heavyLoadout[client])
					{
						case 1:
						{
							CreateHat(client, 261);
						}
						case 2:
						{
							CreateHat(client, 940);
							CreateHat(client, 743);
							CreateHat(client, 583);
						}
						case 3:
						{
							CreateHat(client, 261);
							CreateHat(client, 343);
							CreateHat(client, 165);
						}
						case 4:
						{
							CreateHat(client, 145);
							CreateHat(client, 30319);
							CreateHat(client, 777);
						}
						case 5:
						{
							CreateHat(client, 30545);
						}
						case 6:
						{
						}
						case 7:
						{
							CreateHat(client, 427);
							CreateHat(client, 30085);
						}
						case 8:
						{
							CreateHat(client, 96);
							CreateHat(client, 30342);
							CreateHat(client, 30343);
						}
						case 9:
						{
							CreateHat(client, 309);
						}
						case 10:
						{
							CreateHat(client, 165);
						}
						case 11:
						{
							CreateHat(client, 162);
							CreateHat(client, 143);
							CreateHat(client, 30302);
						}
						case 12:
						{
							CreateHat(client, 166);
						}
						case 13:
						{
							CreateHat(client, 97);
							CreateHat(client, 30302);
						}
						case 14:
						{
							CreateHat(client, 30085);
							CreateHat(client, 601);
							CreateHat(client, 30178);
						}
						case 15:
						{
						}
						case 16:
						{
							CreateHat(client, 246);
							CreateHat(client, 757);
						}
						case 17:
						{
							CreateHat(client, 30357);
						}
						default:
							RandomF2Phat(client);
					}
				}
				case TFClass_Pyro:
				{
					switch (pyroPrimary[client])
					{
						case 1178:
							CreateWeapon(client, "tf_weapon_rocketlauncher_fireball", 0, pyroPrimary[client]);
						default:
							CreateWeapon(client, "tf_weapon_flamethrower", 0, pyroPrimary[client]);
					}

					switch (pyroSecondary[client])
					{
						case 39:
							CreateWeapon(client, "tf_weapon_flaregun", 1, pyroSecondary[client]);
						case 351:
							CreateWeapon(client, "tf_weapon_flaregun", 1, pyroSecondary[client]);
						case 595:
							CreateWeapon(client, "tf_weapon_flaregun_revenge", 1, pyroSecondary[client]);
						case 740:
							CreateWeapon(client, "tf_weapon_flaregun", 1, pyroSecondary[client]);
						case 1081:
							CreateWeapon(client, "tf_weapon_flaregun", 1, pyroSecondary[client]);
						case 1179:
							CreateWeapon(client, "tf_weapon_rocketpack", 1, pyroSecondary[client]);
						case 1180:
							CreateWeapon(client, "tf_weapon_jar_gas", 1, pyroSecondary[client]);
						default:
							CreateWeapon(client, "tf_weapon_shotgun_pyro", 1, pyroSecondary[client]);
					}

					switch (pyroMelee[client])
					{
						case 813:
							CreateWeapon(client, "tf_weapon_breakable_sign", 2, pyroMelee[client]);
						case 1181:
							CreateWeapon(client, "tf_weapon_slap", 2, pyroMelee[client]);
						default:
							CreateWeapon(client, "tf_weapon_fireaxe", 2, pyroMelee[client]);
					}

					switch (pyroLoadout[client])
					{
						case 1:
						{
							CreateHat(client, 30162); // Bone Dome
							CreateHat(client, 30163); // Air Raider
							CreateHat(client, 30664); // Space Diver
						}
						case 2:
						{
							CreateHat(client, 335); // Foster's Facade
							CreateHat(client, 471); // Proof of Purchase
							CreateHat(client, 30168); // Special Eyes
						}
						case 3:
						{
							CreateHat(client, 702); // Warsworn Helmet
							CreateHat(client, 316); // Pyromancer's Mask
							CreateHat(client, 787); // Tribal Bones
						}
						case 4:
						{
							CreateHat(client, 105); // Brigade Helm
							CreateHat(client, 387); // Sight for Sore Eyes
							CreateHat(client, 632); // Cremator's Conscience
						}
						case 5:
						{
							CreateHat(client, 394); // Connoisseur's Cap
							CreateHat(client, 30367); // Cute Suit
							CreateHat(client, 30068); // Breakneck Baggies
						}
						case 6:
						{
							CreateHat(client, 644); // Head Warmer
							CreateHat(client, 30305); // Sub Zero Suit
							CreateHat(client, 651); // Jingle Belt
						}
						case 7:
						{
							CreateHat(client, 597); // Bubble Pipe
							CreateHat(client, 596); // Moonman Backpack
							CreateHat(client, 30032); // Rusty Reaper
						}
						case 8:
						{
							CreateHat(client, 435); // Dead Cone
							CreateHat(client, 632); // Cremator's Conscience
							CreateHat(client, 30169); // Trickster's Turnout Gear
						}
						case 9:
						{
							CreateHat(client, 105); // Brigade Helm
							CreateHat(client, 632); // Cremator's Conscience
							CreateHat(client, 30169); // Trickster's Turnout Gear
						}
						case 10:
						{
							CreateHat(client, 30580); // Pyromancer's Hood
							CreateHat(client, 30581); // Pyromancer's Raiments
							CreateHat(client, 316); // Pyromancer's Mask
						}
						case 11:
						{
							CreateHat(client, 213); // Attendant
							CreateHat(client, 30400); // Lunatic's Leathers
							CreateHat(client, 570); // Last Breath
						}
						case 12:
						{
							CreateHat(client, 570); // Last Breath
							CreateHat(client, 30400); // Lunatic's Leathers
							CreateHat(client, 30413); // Merc's Mohawk
						}
						case 13:
						{
							CreateHat(client, 570); // Last Breath
							CreateHat(client, 937); // Wraith Wrap
							CreateHat(client, 30309); // Dead of Night
						}
						case 14:
						{
							CreateHat(client, 570); // Last Breath
							CreateHat(client, 30305); // Sub Zero Suit
							CreateHat(client, 30168); // Special Eyes
						}
						case 15:
						{
							CreateHat(client, 30089); // El Muchacho
							CreateHat(client, 570); // Last Breath
							CreateHat(client, 627); // Flamboyant Flamenco
						}
						case 16:
						{
							CreateHat(client, 105); // Brigade Helm
							CreateHat(client, 30169); // Trickster's Turnout Gear
							CreateHat(client, 387); // Sight for Sore Eyes
						}
						case 17:
						{
							CreateHat(client, 316); // Pyromancer's Mask
							CreateHat(client, 247); // Old Guadalajara
							CreateHat(client, 30089); // El Muchacho
						}
						case 18:
						{
							CreateHat(client, 420); // Aperture Labs Hard Hat
							CreateHat(client, 30367); // Cute Suit
							CreateHat(client, 522); // Deus Specs
						}
						case 19:
						{
							CreateHat(client, 316); // Pyromancer's Mask
							CreateHat(client, 976); // Winter Wonderland Wrap
							CreateHat(client, 30176); // Pop-eyes
						}
						case 20:
						{
							CreateHat(client, 30075); // Mair Mask
							CreateHat(client, 30309); // Dead of Night
							CreateHat(client, 30168); // Special Eyes
						}
						case 21:
						{
							CreateHat(client, 854); // Area 451
							CreateHat(client, 30168); // Special Eyes
							CreateHat(client, 316); // Pyromancer's Mask
						}
						case 22:
						{
							CreateHat(client, 126); // Bill's Hat
							CreateHat(client, 30168); // Special Eyes
							CreateHat(client, 30092); // Soot Suit
						}
						case 23:
						{
							CreateHat(client, 335); // Foster's Facade
							CreateHat(client, 471); // Proof of Purchase
							CreateHat(client, 641); // Ornament Armament
						}
						case 24:
						{
							CreateHat(client, 30192); // Hard-Headed Hardware
							CreateHat(client, 287); // Spine-Chilling Skull
							CreateHat(client, 1126); // Duck Badge
						}
						case 25:
						{
							CreateHat(client, 30036); // Filamental
							CreateHat(client, 30309); // Dead of Night
							CreateHat(client, 316); // Pyromancer's Mask
						}
						case 26:
						{
							CreateHat(client, 30544); // North Polar Fleece
							CreateHat(client, 571); // Apparition's Aspect
							CreateHat(client, 182); // Vintage Merryweather
						}
						case 27:
						{
							CreateHat(client, 30362); // Law
							CreateHat(client, 30305); // Sub Zero Suit
							CreateHat(client, 976); // Winter Wonderland Wrap
						}
						case 28:
						{
							CreateHat(client, 394); // Connoisseur's Cap
							CreateHat(client, 30417); // Frymaster
							CreateHat(client, 30309); // Dead of Night
						}
						case 29:
						{
							CreateHat(client, 102); // Respectless Rubber Glove
							CreateHat(client, 316); // Pyromancer's Mask
							CreateHat(client, 30305); // Sub Zero Suit
						}
						case 30:
						{
							CreateHat(client, 976); // Winter Wonderland Wrap
							CreateHat(client, 182); // Vintage Merryweather
							CreateHat(client, 30664); // Space Diver
						}
						case 31:
						{
							CreateHat(client, 30305); // Sub Zero Suit
							CreateHat(client, 1020); // Person in the Iron Mask
							CreateHat(client, 30176); // Pop-eyes
						}
						case 32:
						{
							CreateHat(client, 30063); // Centurion
							CreateHat(client, 30075); // Mair Mask
							CreateHat(client, 30169); // Trickster's Turnout Gear
						}
						case 33:
						{
							CreateHat(client, 1014); // Brutal Bouffant
							CreateHat(client, 30367); // Cute Suit
							CreateHat(client, 387); // Sight for Sore Eyes
						}
						case 34:
						{
							CreateHat(client, 162); // Max's Severed Head
							CreateHat(client, 30036); // Filamental
							CreateHat(client, 597); // Bubble Pipe
						}
						default:
							RandomF2Phat(client);
					}
				}
				case TFClass_Spy:
				{
					CreateWeapon(client, "tf_weapon_revolver", 0, spyPrimary[client]);
					CreateWeapon(client, "tf_weapon_knife", 2, spyMelee[client]);
					CreateWeapon(client, "tf_weapon_invis", 4, spyWatch[client]);

					switch (spyLoadout[client])
					{
						case 1:
						{
							CreateHat(client, 261);
						}
						case 2:
						{
							CreateHat(client, 668);
							CreateHat(client, 743);
							CreateHat(client, 583);
						}
						case 3:
						{
							CreateHat(client, 261);
							CreateHat(client, 343);
							CreateHat(client, 165);
						}
						case 4:
						{
							CreateHat(client, 437);
							CreateHat(client, 879);
						}
						case 5:
						{
							CreateHat(client, 459);
							CreateHat(client, 30353);
						}
						case 6:
						{
						}
						case 7:
						{
							CreateHat(client, 223);
						}
						case 8:
						{
							CreateHat(client, 30405);
							CreateHat(client, 30404);
							CreateHat(client, 30467);
						}
						case 9:
						{
							CreateHat(client, 459);
							CreateHat(client, 462);
						}
						case 10:
						{
							CreateHat(client, 165);
						}
						case 11:
						{
							CreateHat(client, 30397);
							CreateHat(client, 30414);
							CreateHat(client, 30546);
						}
						case 12:
						{
							CreateHat(client, 166);
						}
						case 13:
						{
							CreateHat(client, 180);
						}
						case 14:
						{
							CreateHat(client, 30177);
							CreateHat(client, 30389);
							CreateHat(client, 30397);
						}
						case 15:
						{
							CreateHat(client, 397);
							CreateHat(client, 337);
							CreateHat(client, 30189);
						}
						case 16:
						{
						}
						case 17:
						{
							CreateHat(client, 30357);
						}
						case 18:
						{
						}
						default:
							RandomF2Phat(client);
					}
				}
				case TFClass_Engineer:
				{
					switch (engineerPrimary[client])
					{
						case 141:
							CreateWeapon(client, "tf_weapon_sentry_revenge", 0, engineerPrimary[client]);
						case 588:
							CreateWeapon(client, "tf_weapon_drg_pomson", 0, engineerPrimary[client]);
						case 997:
							CreateWeapon(client, "tf_weapon_shotgun_building_rescue", 0, engineerPrimary[client]);
						case 1004:
							CreateWeapon(client, "tf_weapon_sentry_revenge", 0, engineerPrimary[client]);
						default:
							CreateWeapon(client, "tf_weapon_shotgun", 0, engineerPrimary[client]);
					}

					switch (engineerSecondary[client])
					{
						case 140:
							CreateWeapon(client, "tf_weapon_laser_pointer", 1, engineerSecondary[client]);
						case 528:
							CreateWeapon(client, "tf_weapon_mechanical_arm", 1, engineerSecondary[client]);
						case 1086:
							CreateWeapon(client, "tf_weapon_laser_pointer", 1, engineerSecondary[client]);
						case 30668:
							CreateWeapon(client, "tf_weapon_laser_pointer", 1, engineerSecondary[client]);
						default:
							CreateWeapon(client, "tf_weapon_shotgun", 1, engineerSecondary[client]);
					}

					switch (engineerMelee[client])
					{
						case 142:
							CreateWeapon(client, "tf_weapon_robot_arm", 2, engineerMelee[client]);
						default:
							CreateWeapon(client, "tf_weapon_wrench", 2, engineerMelee[client]);
					}

					switch (engineerLoadout[client])
					{
						case 1:
						{
							CreateHat(client, 261);
						}
						case 2:
						{
							CreateHat(client, 941);
							CreateHat(client, 743);
							CreateHat(client, 583);
						}
						case 3:
						{
							CreateHat(client, 261);
							CreateHat(client, 343);
							CreateHat(client, 165);
						}
						case 4:
						{
							CreateHat(client, 420);
							CreateHat(client, 496);
							CreateHat(client, 30306);
						}
						case 5:
						{
							CreateHat(client, 48);
						}
						case 6:
						{
						}
						case 7:
						{
							CreateHat(client, 30402);
							CreateHat(client, 30403);
						}
						case 8:
						{
							CreateHat(client, 162);
							CreateHat(client, 143);
							CreateHat(client, 30414);
						}
						case 9:
						{
							CreateHat(client, 590);
							CreateHat(client, 591);
						}
						case 10:
						{
							CreateHat(client, 165);
						}
						case 11:
						{
							CreateHat(client, 30510);
						}
						case 12:
						{
							CreateHat(client, 116);
						}
						case 13:
						{
							CreateHat(client, 166);
						}
						case 14:
						{
							CreateHat(client, 165);
							CreateHat(client, 30119);
							CreateHat(client, 987);
						}
						case 15:
						{
							CreateHat(client, 30066);
							CreateHat(client, 143);
							CreateHat(client, 733);
						}
						case 16:
						{
							CreateHat(client, 94);
							CreateHat(client, 5621);
							CreateHat(client, 755);
						}
						case 17:
						{
							CreateHat(client, 261);
							CreateHat(client, 583);
							CreateHat(client, 744);
						}
						case 18:
						{
							CreateHat(client, 755);
							CreateHat(client, 94);
							CreateHat(client, 30412);
						}
						case 19:
						{
							CreateHat(client, 30408);
							CreateHat(client, 30341);
							CreateHat(client, 30407);
						}
						case 20:
						{
						}
						case 21:
						{
							CreateHat(client, 162);
						}
						case 22:
						{
							CreateHat(client, 30357);
						}
						case 23:
						{
						}
						default:
							RandomF2Phat(client);
					}
				}
			}

			EquipWeaponSlot(client, 0);
		}
		else
		{
			switch (TF2_GetPlayerClass(client))
			{
				case TFClass_Scout:
				{
					int rnd = crandomint(1, 12);
					switch (rnd)
					{
						case 1:
							CreateWeapon(client, "tf_weapon_bat_wood", 2, 44);
						case 2:
							CreateWeapon(client, "tf_weapon_bat_fish", 2, 572);
						case 3:
							CreateWeapon(client, "tf_weapon_bat", 2, 317);
						case 4:
							CreateWeapon(client, "tf_weapon_bat", 2, 349);
						case 5:
							CreateWeapon(client, "tf_weapon_bat", 2, 355);
						case 6:
							CreateWeapon(client, "tf_weapon_bat_giftwrap", 2, 648);
						case 7:
							CreateWeapon(client, "tf_weapon_bat", 2, 450);
						case 8:
							CreateWeapon(client, "tf_weapon_bat_fish", 2, 221);
						case 9:
							CreateWeapon(client, "tf_weapon_bat", 2, 1013);
						case 10:
							CreateWeapon(client, "tf_weapon_bat", 2, 939);
						case 11:
							CreateWeapon(client, "tf_weapon_bat", 2, 880);
						case 12:
							CreateWeapon(client, "tf_weapon_bat", 2, 474);
					}
				}
				case TFClass_Sniper:
				{
					int rnd = crandomint(1, 3);
					switch (rnd)
					{
						case 1:
							CreateWeapon(client, "tf_wearable", 1, 57, true);
						case 2:
							CreateWeapon(client, "tf_wearable", 1, 231, true);
						case 3:
							CreateWeapon(client, "tf_wearable", 1, 642, true);
					}

					rnd = crandomint(1, 7);
					switch (rnd)
					{
						case 1:
							CreateWeapon(client, "tf_weapon_club", 2, 171);
						case 2:
							CreateWeapon(client, "tf_weapon_club", 2, 232);
						case 3:
							CreateWeapon(client, "tf_weapon_club", 2, 401);
						case 4:
							CreateWeapon(client, "tf_weapon_club", 2, 1013);
						case 5:
							CreateWeapon(client, "tf_weapon_club", 2, 939);
						case 6:
							CreateWeapon(client, "tf_weapon_club", 2, 880);
						case 7:
							CreateWeapon(client, "tf_weapon_club", 2, 474);
					}
				}
				case TFClass_Soldier:
				{
					int rnd = crandomint(1, 2);
					switch (rnd)
					{
						case 1:
							CreateWeapon(client, "tf_wearable", 1, 133, true);
						case 2:
							CreateWeapon(client, "tf_wearable", 1, 444, true);
					}

					rnd = crandomint(1, 9);
					switch (rnd)
					{
						case 1:
							CreateWeapon(client, "tf_weapon_shovel", 2, 128);
						case 2:
							CreateWeapon(client, "tf_weapon_shovel", 2, 154);
						case 3:
							CreateWeapon(client, "tf_weapon_shovel", 2, 447);
						case 4:
							CreateWeapon(client, "tf_weapon_shovel", 2, 775);
						case 5:
							CreateWeapon(client, "tf_weapon_katana", 2, 357);
						case 6:
							CreateWeapon(client, "tf_weapon_shovel", 2, 1013);
						case 7:
							CreateWeapon(client, "tf_weapon_shovel", 2, 939);
						case 8:
							CreateWeapon(client, "tf_weapon_shovel", 2, 880);
						case 9:
							CreateWeapon(client, "tf_weapon_shovel", 2, 474);
					}
				}
				case TFClass_DemoMan:
				{
					int rnd = crandomint(1, 2);
					switch (rnd)
					{
						case 1:
							CreateWeapon(client, "tf_wearable", 0, 405, true);
						case 2:
							CreateWeapon(client, "tf_wearable", 0, 608, true);
					}

					rnd = crandomint(1, 3);
					switch (rnd)
					{
						case 1:
							CreateWeapon(client, "tf_wearable_demoshield", 1, 131, true);
						case 2:
							CreateWeapon(client, "tf_wearable_demoshield", 1, 406, true);
						case 3:
							CreateWeapon(client, "tf_wearable_demoshield", 1, 1099, true);
					}
				
					rnd = crandomint(1, 13);
					switch (rnd)
					{
						case 1:
							CreateWeapon(client, "tf_weapon_sword", 2, 132);
						case 2:
							CreateWeapon(client, "tf_weapon_shovel", 2, 154);
						case 3:
							CreateWeapon(client, "tf_weapon_sword", 2, 172);
						case 4:
							CreateWeapon(client, "tf_weapon_stickbomb", 2, 307);
						case 5:
							CreateWeapon(client, "tf_weapon_sword", 2, 327);
						case 6:
							CreateWeapon(client, "tf_weapon_katana", 2, 357);
						case 7:
							CreateWeapon(client, "tf_weapon_sword", 2, 482);
						case 8:
							CreateWeapon(client, "tf_weapon_bottle", 2, 609);
						case 9:
							CreateWeapon(client, "tf_weapon_bottle", 2, 1013);
						case 10:
							CreateWeapon(client, "tf_weapon_bottle", 2, 939);
						case 11:
							CreateWeapon(client, "tf_weapon_bottle", 2, 880);
						case 12:
							CreateWeapon(client, "tf_weapon_bottle", 2, 474);
						case 13:
							CreateWeapon(client, "tf_weapon_sword", 2, 404);
					}
				}
				case TFClass_Medic:
				{
					int rnd = crandomint(1, 8);
					switch (rnd)
					{
						case 1:
							CreateWeapon(client, "tf_weapon_bonesaw", 2, 37);
						case 2:
							CreateWeapon(client, "tf_weapon_bonesaw", 2, 173);
						case 3:
							CreateWeapon(client, "tf_weapon_bonesaw", 2, 304);
						case 4:
							CreateWeapon(client, "tf_weapon_bonesaw", 2, 413);
						case 5:
							CreateWeapon(client, "tf_weapon_bonesaw", 2, 1013);
						case 6:
							CreateWeapon(client, "tf_weapon_bonesaw", 2, 939);
						case 7:
							CreateWeapon(client, "tf_weapon_bonesaw", 2, 880);
						case 8:
							CreateWeapon(client, "tf_weapon_bonesaw", 2, 474);
					}
				}
				case TFClass_Heavy:
				{
					int rnd = crandomint(1, 11);
					switch (rnd)
					{
						case 1:
							CreateWeapon(client, "tf_weapon_fists", 2, 43);
						case 2:
							CreateWeapon(client, "tf_weapon_fists", 2, 239);
						case 3:
							CreateWeapon(client, "tf_weapon_fists", 2, 310);
						case 4:
							CreateWeapon(client, "tf_weapon_fists", 2, 331);
						case 5:
							CreateWeapon(client, "tf_weapon_fists", 2, 426);
						case 6:
							CreateWeapon(client, "tf_weapon_fists", 2, 656);
						case 7:
							CreateWeapon(client, "tf_weapon_fists", 2, 587);
						case 8:
							CreateWeapon(client, "tf_weapon_fireaxe", 2, 1013);
						case 9:
							CreateWeapon(client, "tf_weapon_fireaxe", 2, 939);
						case 10:
							CreateWeapon(client, "tf_weapon_fireaxe", 2, 880);
						case 11:
							CreateWeapon(client, "tf_weapon_fireaxe", 2, 474);
					}
				}
				case TFClass_Pyro:
				{
					int rnd = crandomint(1, 15);
					switch (rnd)
					{
						case 1:
							CreateWeapon(client, "tf_weapon_fireaxe", 2, 38);
						case 2:
							CreateWeapon(client, "tf_weapon_fireaxe", 2, 153);
						case 3:
							CreateWeapon(client, "tf_weapon_fireaxe", 2, 214);
						case 4:
							CreateWeapon(client, "tf_weapon_fireaxe", 2, 326);
						case 5:
							CreateWeapon(client, "tf_weapon_fireaxe", 2, 348);
						case 6:
							CreateWeapon(client, "tf_weapon_fireaxe", 2, 593);
						case 7:
							CreateWeapon(client, "tf_weapon_breakable_sign", 2, 813);
						case 8:
							CreateWeapon(client, "tf_weapon_fireaxe", 2, 457);
						case 9:
							CreateWeapon(client, "tf_weapon_fireaxe", 2, 739);
						case 10:
							CreateWeapon(client, "tf_weapon_slap", 2, 1181);
						case 11:
							CreateWeapon(client, "tf_weapon_fireaxe", 2, 466);
						case 12:
							CreateWeapon(client, "tf_weapon_fireaxe", 2, 1013);
						case 13:
							CreateWeapon(client, "tf_weapon_fireaxe", 2, 939);
						case 14:
							CreateWeapon(client, "tf_weapon_fireaxe", 2, 880);
						case 15:
							CreateWeapon(client, "tf_weapon_fireaxe", 2, 474);
					}
				}
				case TFClass_Spy:
				{
					int rnd = crandomint(1, 6);
					switch (rnd)
					{
						case 1:
							CreateWeapon(client, "tf_weapon_knife", 2, 356);
						case 2:
							CreateWeapon(client, "tf_weapon_knife", 2, 461);
						case 3:
							CreateWeapon(client, "tf_weapon_knife", 2, 649);
						case 4:
							CreateWeapon(client, "tf_weapon_knife", 2, 638);
						case 5:
							CreateWeapon(client, "tf_weapon_knife", 2, 225);
						case 6:
							CreateWeapon(client, "tf_weapon_knife", 2, 574);
					}

					rnd = 1;
					switch (rnd)
					{
						case 1:
							CreateWeapon(client, "tf_weapon_invis", 4, 947);
					}
				}
				case TFClass_Engineer:
				{
					int rnd = crandomint(1, 4);
					switch (rnd)
					{
						case 1:
							CreateWeapon(client, "tf_weapon_wrench", 2, 155);
						case 2:
							CreateWeapon(client, "tf_weapon_wrench", 2, 329);
						case 3:
							CreateWeapon(client, "tf_weapon_wrench", 2, 589);
						case 4:
							CreateWeapon(client, "tf_weapon_robot_arm", 2, 142);
					}
				}
			}
		}
	}

	return Plugin_Continue;
}

public void EventSuddenDeath(Handle event, const char[] name, bool dontBroadcast)
{
	g_bSuddenDeathMode = true;
}

public void EventRoundReset(Handle event, const char[] name, bool dontBroadcast)
{
	g_bSuddenDeathMode = false;
}

stock bool CreateWeapon(const int client, const char[] classname, const int slot, const int itemindex, const bool wearable = false)
{
	int weapon = CreateEntityByName(classname);
	if (!IsValidEntity(weapon))
	{
		LogError("Failed to create a valid entity with class name [%s]! Skipping.", classname);
		return false;
	}
	
	char entclass[64];
	GetEntityNetClass(weapon, entclass, sizeof(entclass));
	SetEntProp(weapon, Prop_Send, "m_iItemDefinitionIndex", itemindex);
	SetEntData(weapon, FindSendPropInfo(entclass, "m_iEntityQuality"), 6);
	SetEntData(weapon, FindSendPropInfo(entclass, "m_hOwnerEntity"), client);
	SetEntProp(weapon, Prop_Send, "m_iEntityLevel", crandomint(1, 99));
	SetEntProp(weapon, Prop_Send, "m_bInitialized", 1);
	if (!DispatchSpawn(weapon)) 
	{
		LogError("The created weapon entity [Class name: %s, Item index: %i, Index: %i], failed to spawn! Skipping.", classname, itemindex, weapon);
		AcceptEntityInput(weapon, "Kill");
		return false;
	}

	switch (itemindex)
	{
		case 810:
		{
			SetEntProp(weapon, Prop_Send, "m_iObjectType", 3);
			SetEntProp(weapon, Prop_Data, "m_iSubType", 3);
			SetEntProp(weapon, Prop_Send, "m_aBuildableObjectTypes", 0, _, 0);
			SetEntProp(weapon, Prop_Send, "m_aBuildableObjectTypes", 0, _, 1);
			SetEntProp(weapon, Prop_Send, "m_aBuildableObjectTypes", 0, _, 2);
			SetEntProp(weapon, Prop_Send, "m_aBuildableObjectTypes", 1, _, 3);
		}
		case 998:
			SetEntProp(weapon, Prop_Send, "m_nChargeResistType", crandomint(0, 2));
		case 1178:
			SetEntData(client, FindSendPropInfo("CTFPlayer", "m_iAmmo") + (GetEntProp(weapon, Prop_Send, "m_iPrimaryAmmoType", 1) * 4), 40, 4);
		case 39, 351, 740:
			SetEntData(client, FindSendPropInfo("CTFPlayer", "m_iAmmo") + (GetEntProp(weapon, Prop_Send, "m_iPrimaryAmmoType", 1) * 4), 16, 4);
	}

	if (slot > -1)
		TF2_RemoveWeaponSlot(client, slot);

	if (!wearable) 
		EquipPlayerWeapon(client, weapon);
	else 
		SDKCall(g_hWWeaponEquip, client, weapon);

	if (slot > -1 && !wearable && GetPlayerWeaponSlot(client, slot) != weapon)
	{
		LogError("The created weapon entity [Class name: %s, Item index: %i, Index: %i], failed to equip! This is probably caused by invalid gamedata.", classname, itemindex, weapon);
		AcceptEntityInput(weapon, "Kill");
		return false;
	}

	return true;
}

stock bool CreateHat(const int client, const int itemindex, const int quality = 6)
{
	int hat = CreateEntityByName("tf_wearable");
	if (!IsValidEntity(hat))
	{
		LogError("Failed to create a valid entity with class name [tf_wearable]! Skipping.");
		return false;
	}
	
	char entclass[64];
	GetEntityNetClass(hat, entclass, sizeof(entclass));
	SetEntProp(hat, Prop_Send, "m_iItemDefinitionIndex", itemindex);
	SetEntData(hat, FindSendPropInfo(entclass, "m_hOwnerEntity"), client);
	SetEntData(hat, FindSendPropInfo(entclass, "m_iEntityQuality"), quality);
	SetEntProp(hat, Prop_Send, "m_iEntityLevel", crandomint(1, 100));
	SetEntProp(hat, Prop_Send, "m_bInitialized", 1);
	if (!DispatchSpawn(hat)) 
	{
		LogError("The created cosmetic entity [Class name: tf_wearable, Item index: %i, Index: %i], failed to spawn! Skipping.", itemindex, hat);
		AcceptEntityInput(hat, "Kill");
		return false;
	}

	SDKCall(g_hWWeaponEquip, client, hat);
	return true;
}

public Action TimerHealth(Handle timer, any client)
{
	int hp = GetPlayerMaxHp(client);
	if (hp > 0)
		SetEntityHealth(client, hp);
	
	return Plugin_Continue;
}

stock void EquipWeaponSlot(const int client, const int slot)
{
	int iWeapon = GetPlayerWeaponSlot(client, slot);
	if (IsValidEntity(iWeapon))
		EquipWeapon(client, iWeapon);
}

stock void EquipWeapon(int client, int weapon)
{
	char class[80];
	GetEntityClassname(weapon, class, sizeof(class));
	Format(class, sizeof(class), "use %s", class);
	FakeClientCommandThrottled(client, class);
}

float g_flNextCommand[MAXPLAYERS + 1];
stock bool FakeClientCommandThrottled(const int client, const char[] command)
{
	if (g_flNextCommand[client] > GetGameTime())
		return false;
	
	FakeClientCommand(client, command);
	g_flNextCommand[client] = GetGameTime() + 0.4;
	return true;
}

stock int GetPlayerMaxHp(const int client)
{
	if (!IsClientInGame(client) || g_iResourceEntity == -1)
		return -1;

	return GetEntProp(g_iResourceEntity, Prop_Send, "m_iMaxHealth", _, client);
}

stock bool IsPlayerHere(const int client)
{
	return (client && IsClientInGame(client) && IsFakeClient(client) && !IsClientReplay(client) && !IsClientSourceTV(client));
}

stock void SetupWeapons(const int client)
{
	switch (crandomint(1, 6))
	{
		case 1:
		{
			switch (crandomint(1, 24))
			{
				case 1:
					scoutPrimary[client] = 669;
				case 2:
					scoutPrimary[client] = 799;
				case 3:
					scoutPrimary[client] = 808;
				case 4:
					scoutPrimary[client] = 888;
				case 5:
					scoutPrimary[client] = 897;
				case 6:
					scoutPrimary[client] = 906;
				case 7:
					scoutPrimary[client] = 915;
				case 8:
					scoutPrimary[client] = 964;
				case 9:
					scoutPrimary[client] = 973;
				case 10:
					scoutPrimary[client] = 1078;
				case 11:
					scoutPrimary[client] = 15002;
				case 12:
					scoutPrimary[client] = 15015;
				case 13:
					scoutPrimary[client] = 15021;
				case 14:
					scoutPrimary[client] = 15029;
				case 15:
					scoutPrimary[client] = 15036;
				case 16:
					scoutPrimary[client] = 15053;
				case 17:
					scoutPrimary[client] = 15065;
				case 18:
					scoutPrimary[client] = 15069;
				case 19:
					scoutPrimary[client] = 15106;
				case 20:
					scoutPrimary[client] = 15107;
				case 21:
					scoutPrimary[client] = 15108;
				case 22:
					scoutPrimary[client] = 15131;
				case 23:
					scoutPrimary[client] = 15151;
				case 24:
					scoutPrimary[client] = 15157;
			}
		}
		case 2:
			scoutPrimary[client] = 45;
		case 3:
			scoutPrimary[client] = 220;
		case 4:
			scoutPrimary[client] = 448;
		case 5:
			scoutPrimary[client] = 772;
		case 6:
			scoutPrimary[client] = 1103;
	}

	switch (crandomint(1, 8))
	{
		case 1:
		{
			switch (crandomint(1, 16))
			{
				case 1:
					scoutSecondary[client] = 1121;
				case 2:
					scoutSecondary[client] = 1145;
				case 3:
					scoutSecondary[client] = 15013;
				case 4:
					scoutSecondary[client] = 15018;
				case 5:
					scoutSecondary[client] = 15035;
				case 6:
					scoutSecondary[client] = 15041;
				case 7:
					scoutSecondary[client] = 15046;
				case 8:
					scoutSecondary[client] = 15056;
				case 9:
					scoutSecondary[client] = 15060;
				case 10:
					scoutSecondary[client] = 15061;
				case 11:
					scoutSecondary[client] = 15100;
				case 12:
					scoutSecondary[client] = 15101;
				case 13:
					scoutSecondary[client] = 15102;
				case 14:
					scoutSecondary[client] = 15126;
				case 15:
					scoutSecondary[client] = 15148;
				case 16:
					scoutSecondary[client] = 30666;
			}
		}
		case 2:
			scoutSecondary[client] = 46;
		case 3:
			scoutSecondary[client] = 163;
		case 4:
			scoutSecondary[client] = 222;
		case 5:
			scoutSecondary[client] = 294;
		case 6:
			scoutSecondary[client] = 449;
		case 7:
			scoutSecondary[client] = 773;
		case 8:
			scoutSecondary[client] = 812;
	}

	switch (crandomint(1, 13))
	{
		case 1:
			scoutMelee[client] = 44;
		case 2:
			scoutMelee[client] = 221;
		case 3:
			scoutMelee[client] = 317;
		case 4:
			scoutMelee[client] = 325;
		case 5:
			scoutMelee[client] = 349;
		case 6:
			scoutMelee[client] = 355;
		case 7:
			scoutMelee[client] = 450;
		case 8:
			scoutMelee[client] = 452;
		case 9:
			scoutMelee[client] = 572;
		case 10:
			scoutMelee[client] = 648;
		case 11:
			scoutMelee[client] = 660;
		case 12:
			scoutMelee[client] = 999;
		case 13:
			scoutMelee[client] = 30667;
	}

	switch (crandomint(1, 8))
	{
		case 1:
		{
			switch (crandomint(1, 22))
			{
				case 1:
					soldierPrimary[client] = 658;
				case 2:
					soldierPrimary[client] = 800;
				case 3:
					soldierPrimary[client] = 809;
				case 4:
					soldierPrimary[client] = 889;
				case 5:
					soldierPrimary[client] = 898;
				case 6:
					soldierPrimary[client] = 907;
				case 7:
					soldierPrimary[client] = 916;
				case 8:
					soldierPrimary[client] = 965;
				case 9:
					soldierPrimary[client] = 974;
				case 10:
					soldierPrimary[client] = 1085;
				case 11:
					soldierPrimary[client] = 15006;
				case 12:
					soldierPrimary[client] = 15014;
				case 13:
					soldierPrimary[client] = 15028;
				case 14:
					soldierPrimary[client] = 15043;
				case 15:
					soldierPrimary[client] = 15052;
				case 16:
					soldierPrimary[client] = 15057;
				case 17:
					soldierPrimary[client] = 15081;
				case 18:
					soldierPrimary[client] = 15104;
				case 19:
					soldierPrimary[client] = 15105;
				case 20:
					soldierPrimary[client] = 15129;
				case 21:
					soldierPrimary[client] = 15130;
				case 22:
					soldierPrimary[client] = 15150;
			}
		}
		case 2:
			soldierPrimary[client] = 127;
		case 3:
			soldierPrimary[client] = 228;
		case 4:
			soldierPrimary[client] = 414;
		case 5:
			soldierPrimary[client] = 441;
		case 6:
			soldierPrimary[client] = 513;
		case 7:
			soldierPrimary[client] = 730;
		case 8:
			soldierPrimary[client] = 1104;
	}

	switch (crandomint(1, 6))
	{
		case 1:
		{
			switch (crandomint(1, 14))
			{
				case 1:
					soldierSecondary[client] = 133; // useless
				case 2:
					soldierSecondary[client] = 444; // useless
				case 3:
					soldierSecondary[client] = 1001;
				case 4:
					soldierSecondary[client] = 1101; // useless
				case 5:
					soldierSecondary[client] = 1141;
				case 6:
					soldierSecondary[client] = 15003;
				case 7:
					soldierSecondary[client] = 15016;
				case 8:
					soldierSecondary[client] = 15044;
				case 9:
					soldierSecondary[client] = 15047;
				case 10:
					soldierSecondary[client] = 15085;
				case 11:
					soldierSecondary[client] = 15109;
				case 12:
					soldierSecondary[client] = 15132;
				case 13:
					soldierSecondary[client] = 15133;
				case 14:
					soldierSecondary[client] = 15152;
			}
		}
		case 2:
			soldierSecondary[client] = 129;
		case 3:
			soldierSecondary[client] = 226;
		case 4:
			soldierSecondary[client] = 354;
		case 5:
			soldierSecondary[client] = 415;
		case 6:
			soldierSecondary[client] = 442;
		case 7:
			soldierSecondary[client] = 1153;
	}

	switch (crandomint(1, 7))
	{
		case 1:
			soldierMelee[client] = 128;
		case 2:
			soldierMelee[client] = 154;
		case 3:
			soldierMelee[client] = 264;
		case 4:
			soldierMelee[client] = 357;
		case 5:
			soldierMelee[client] = 416;
		case 6:
			soldierMelee[client] = 447;
		case 7:
			soldierMelee[client] = 775;
	}

	switch (crandomint(1, 5))
	{
		case 1:
		{
			switch (crandomint(1, 25))
			{
				case 1:
					pyroPrimary[client] = 659;
				case 2:
					pyroPrimary[client] = 741; // only in pyroland
				case 3:
					pyroPrimary[client] = 798;
				case 4:
					pyroPrimary[client] = 807;
				case 5:
					pyroPrimary[client] = 887;
				case 6:
					pyroPrimary[client] = 896;
				case 7:
					pyroPrimary[client] = 905;
				case 8:
					pyroPrimary[client] = 914;
				case 9:
					pyroPrimary[client] = 963;
				case 10:
					pyroPrimary[client] = 972;
				case 11:
					pyroPrimary[client] = 1146;
				case 12:
					pyroPrimary[client] = 15005;
				case 13:
					pyroPrimary[client] = 15017;
				case 14:
					pyroPrimary[client] = 15030;
				case 15:
					pyroPrimary[client] = 15034;
				case 16:
					pyroPrimary[client] = 15049;
				case 17:
					pyroPrimary[client] = 15054;
				case 18:
					pyroPrimary[client] = 15066;
				case 19:
					pyroPrimary[client] = 15067;
				case 20:
					pyroPrimary[client] = 15068;
				case 21:
					pyroPrimary[client] = 15089;
				case 22:
					pyroPrimary[client] = 15090;
				case 23:
					pyroPrimary[client] = 15115;
				case 24:
					pyroPrimary[client] = 15141;
				case 25:
					pyroPrimary[client] = 30474;
			}
		}
		case 2:
			pyroPrimary[client] = 40;
		case 3:
			pyroPrimary[client] = 215;
		case 4:
			pyroPrimary[client] = 594;
		case 5:
			pyroPrimary[client] = 1178;
	}

	switch (crandomint(1, 8))
	{
		case 1:
		{
			switch (crandomint(1, 12))
			{
				case 1:
					pyroSecondary[client] = 1081;
				case 2:
					pyroSecondary[client] = 1141;
				case 3:
					pyroSecondary[client] = 1179; // useless
				case 4:
					pyroSecondary[client] = 15003;
				case 5:
					pyroSecondary[client] = 15016;
				case 6:
					pyroSecondary[client] = 15044;
				case 7:
					pyroSecondary[client] = 15047;
				case 8:
					pyroSecondary[client] = 15085;
				case 9:
					pyroSecondary[client] = 15109;
				case 10:
					pyroSecondary[client] = 15132;
				case 11:
					pyroSecondary[client] = 15133;
				case 12:
					pyroSecondary[client] = 15152;
			}
		}
		case 2:
			pyroSecondary[client] = 39;
		case 3:
			pyroSecondary[client] = 351;
		case 4:
			pyroSecondary[client] = 415;
		case 5:
			pyroSecondary[client] = 595;
		case 6:
			pyroSecondary[client] = 740;
		case 7:
			pyroSecondary[client] = 1153;
		case 8:
			pyroSecondary[client] = 1180;
	}

	switch (crandomint(1, 12))
	{
		case 1:
			pyroMelee[client] = 38;
		case 2:
			pyroMelee[client] = 153;
		case 3:
			pyroMelee[client] = 214;
		case 4:
			pyroMelee[client] = 326;
		case 5:
			pyroMelee[client] = 348;
		case 6:
			pyroMelee[client] = 457;
		case 7:
			pyroMelee[client] = 466;
		case 8:
			pyroMelee[client] = 593;
		case 9:
			pyroMelee[client] = 739;
		case 10:
			pyroMelee[client] = 813;
		case 11:
			pyroMelee[client] = 1000;
		case 12:
			pyroMelee[client] = 1181;
	}

	switch (crandomint(1, 6))
	{
		case 1:
		{
			switch (crandomint(1, 9))
			{
				case 1:
					demomanPrimary[client] = 1101; // useless
				case 2:
					demomanPrimary[client] = 15077;
				case 3:
					demomanPrimary[client] = 15079;
				case 4:
					demomanPrimary[client] = 15091;
				case 5:
					demomanPrimary[client] = 15092;
				case 6:
					demomanPrimary[client] = 15116;
				case 7:
					demomanPrimary[client] = 15117;
				case 8:
					demomanPrimary[client] = 15142;
				case 9:
					demomanPrimary[client] = 15158;
			}
		}
		case 2:
			demomanPrimary[client] = 308;
		case 3:
			demomanPrimary[client] = 405;
		case 4:
			demomanPrimary[client] = 608;
		case 5:
			demomanPrimary[client] = 996;
		case 6:
			demomanPrimary[client] = 1151;
	}

	switch (crandomint(1, 6))
	{
		case 1:
		{
			switch (crandomint(1, 23))
			{
				case 1:
					demomanSecondary[client] = 661;
				case 2:
					demomanSecondary[client] = 797;
				case 3:
					demomanSecondary[client] = 806;
				case 4:
					demomanSecondary[client] = 886;
				case 5:
					demomanSecondary[client] = 895;
				case 6:
					demomanSecondary[client] = 904;
				case 7:
					demomanSecondary[client] = 913;
				case 8:
					demomanSecondary[client] = 962;
				case 9:
					demomanSecondary[client] = 971;
				case 10:
					demomanSecondary[client] = 1144;
				case 11:
					demomanSecondary[client] = 15009;
				case 12:
					demomanSecondary[client] = 15012;
				case 13:
					demomanSecondary[client] = 15024;
				case 14:
					demomanSecondary[client] = 15038;
				case 15:
					demomanSecondary[client] = 15045;
				case 16:
					demomanSecondary[client] = 15048;
				case 17:
					demomanSecondary[client] = 15082;
				case 18:
					demomanSecondary[client] = 15083;
				case 19:
					demomanSecondary[client] = 15084;
				case 20:
					demomanSecondary[client] = 15113;
				case 21:
					demomanSecondary[client] = 15137;
				case 22:
					demomanSecondary[client] = 15138;
				case 23:
					demomanSecondary[client] = 15155;
			}
		}
		case 2:
			demomanSecondary[client] = 130;
		case 3:
			demomanSecondary[client] = 131;
		case 4:
			demomanSecondary[client] = 406;
		case 5:
			demomanSecondary[client] = 1099;
		case 6:
			demomanSecondary[client] = 1150;
	}

	switch (crandomint(1, 11))
	{
		case 1:
			demomanMelee[client] = 132;
		case 2:
			demomanMelee[client] = 154;
		case 3:
			demomanMelee[client] = 172;
		case 4:
			demomanMelee[client] = 266;
		case 5:
			demomanMelee[client] = 307;
		case 6:
			demomanMelee[client] = 327;
		case 7:
			demomanMelee[client] = 357;
		case 8:
			demomanMelee[client] = 404;
		case 9:
			demomanMelee[client] = 482;
		case 10:
			demomanMelee[client] = 609;
		case 11:
			demomanMelee[client] = 1082;
	}

	switch (crandomint(1, 5))
	{
		case 1:
		{
			switch (crandomint(1, 25))
			{
				case 1:
					heavyPrimary[client] = 298;
				case 2:
					heavyPrimary[client] = 654;
				case 3:
					heavyPrimary[client] = 793;
				case 4:
					heavyPrimary[client] = 802;
				case 5:
					heavyPrimary[client] = 882;
				case 6:
					heavyPrimary[client] = 891;
				case 7:
					heavyPrimary[client] = 900;
				case 8:
					heavyPrimary[client] = 909;
				case 9:
					heavyPrimary[client] = 958;
				case 10:
					heavyPrimary[client] = 967;
				case 11:
					heavyPrimary[client] = 15004;
				case 12:
					heavyPrimary[client] = 15020;
				case 13:
					heavyPrimary[client] = 15026;
				case 14:
					heavyPrimary[client] = 15031;
				case 15:
					heavyPrimary[client] = 15040;
				case 16:
					heavyPrimary[client] = 15055;
				case 17:
					heavyPrimary[client] = 15086;
				case 18:
					heavyPrimary[client] = 15087;
				case 19:
					heavyPrimary[client] = 15088;
				case 20:
					heavyPrimary[client] = 15098;
				case 21:
					heavyPrimary[client] = 15099;
				case 22:
					heavyPrimary[client] = 15123;
				case 23:
					heavyPrimary[client] = 15124;
				case 24:
					heavyPrimary[client] = 15125;
				case 25:
					heavyPrimary[client] = 15147;
			}
		}
		case 2:
			heavyPrimary[client] = 41;
		case 3:
			heavyPrimary[client] = 312;
		case 4:
			heavyPrimary[client] = 424;
		case 5:
			heavyPrimary[client] = 811;
	}

	switch (crandomint(1, 8))
	{
		case 1:
		{
			switch (crandomint(1, 12))
			{
				case 1:
					heavySecondary[client] = 863;
				case 2:
					heavySecondary[client] = 1002;
				case 3:
					heavySecondary[client] = 1141;
				case 4:
					heavySecondary[client] = 15003;
				case 5:
					heavySecondary[client] = 15016;
				case 6:
					heavySecondary[client] = 15044;
				case 7:
					heavySecondary[client] = 15047;
				case 8:
					heavySecondary[client] = 15085;
				case 9:
					heavySecondary[client] = 15109;
				case 10:
					heavySecondary[client] = 15132;
				case 11:
					heavySecondary[client] = 15133;
				case 12:
					heavySecondary[client] = 15152;
			}
		}
		case 2:
			heavySecondary[client] = 42;
		case 3:
			heavySecondary[client] = 159;
		case 4:
			heavySecondary[client] = 311;
		case 5:
			heavySecondary[client] = 425;
		case 6:
			heavySecondary[client] = 433;
		case 7:
			heavySecondary[client] = 1153;
		case 8:
			heavySecondary[client] = 1190;
	}

	switch (crandomint(1, 9))
	{
		case 1:
			heavyMelee[client] = 43;
		case 2:
			heavyMelee[client] = 239;
		case 3:
			heavyMelee[client] = 310;
		case 4:
			heavyMelee[client] = 331;
		case 5:
			heavyMelee[client] = 426;
		case 6:
			heavyMelee[client] = 587;
		case 7:
			heavyMelee[client] = 656;
		case 8:
			heavyMelee[client] = 1084;
		case 9:
			heavyMelee[client] = 1100;
	}

	switch (crandomint(1, 6))
	{
		case 1:
		{
			switch (crandomint(1, 11))
			{
				case 1:
					engineerPrimary[client] = 1004;
				case 2:
					engineerPrimary[client] = 1141;
				case 3:
					engineerPrimary[client] = 15003;
				case 4:
					engineerPrimary[client] = 15016;
				case 5:
					engineerPrimary[client] = 15044;
				case 6:
					engineerPrimary[client] = 15047;
				case 7:
					engineerPrimary[client] = 15085;
				case 8:
					engineerPrimary[client] = 15109;
				case 9:
					engineerPrimary[client] = 15132;
				case 10:
					engineerPrimary[client] = 15133;
				case 11:
					engineerPrimary[client] = 15152;
			}
		}
		case 2:
			engineerPrimary[client] = 141;
		case 3:
			engineerPrimary[client] = 527;
		case 4:
			engineerPrimary[client] = 588;
		case 5:
			engineerPrimary[client] = 997;
		case 6:
			engineerPrimary[client] = 1153;
	}

	switch (crandomint(1, 19))
	{
		case 1:
			engineerSecondary[client] = 140;
		case 2:
			engineerSecondary[client] = 294;
		case 3:
			engineerSecondary[client] = 528;
		case 4:
			engineerSecondary[client] = 1086;
		case 5:
			engineerSecondary[client] = 15013;
		case 6:
			engineerSecondary[client] = 15018;
		case 7:
			engineerSecondary[client] = 15035;
		case 8:
			engineerSecondary[client] = 15041;
		case 9:
			engineerSecondary[client] = 15046;
		case 10:
			engineerSecondary[client] = 15056;
		case 11:
			engineerSecondary[client] = 15060;
		case 12:
			engineerSecondary[client] = 15061;
		case 13:
			engineerSecondary[client] = 15100;
		case 14:
			engineerSecondary[client] = 15101;
		case 15:
			engineerSecondary[client] = 15102;
		case 16:
			engineerSecondary[client] = 15126;
		case 17:
			engineerSecondary[client] = 15148;
		case 18:
			engineerSecondary[client] = 30666;
		case 19:
			engineerSecondary[client] = 30668;
	}

	switch (crandomint(1, 21))
	{
		case 1:
			engineerMelee[client] = 142;
		case 2:
			engineerMelee[client] = 155;
		case 3:
			engineerMelee[client] = 169;
		case 4:
			engineerMelee[client] = 329;
		case 5:
			engineerMelee[client] = 589;
		case 6:
			engineerMelee[client] = 662;
		case 7:
			engineerMelee[client] = 795;
		case 8:
			engineerMelee[client] = 804;
		case 9:
			engineerMelee[client] = 884;
		case 10:
			engineerMelee[client] = 893;
		case 11:
			engineerMelee[client] = 902;
		case 12:
			engineerMelee[client] = 911;
		case 13:
			engineerMelee[client] = 960;
		case 14:
			engineerMelee[client] = 969;
		case 15:
			engineerMelee[client] = 15073;
		case 16:
			engineerMelee[client] = 15074;
		case 17:
			engineerMelee[client] = 15075;
		case 18:
			engineerMelee[client] = 15139;
		case 19:
			engineerMelee[client] = 15140;
		case 20:
			engineerMelee[client] = 15114;
		case 21:
			engineerMelee[client] = 15156;
	}

	switch (crandomint(1, 4))
	{
		case 1:
			medicPrimary[client] = 36;
		case 2:
			medicPrimary[client] = 305;
		case 3:
			medicPrimary[client] = 412;
		case 4:
			medicPrimary[client] = 1079;
	}

	switch (crandomint(1, 4))
	{
		case 1:
		{
			switch (crandomint(1, 21))
			{
				case 1:
					medicSecondary[client] = 663;
				case 2:
					medicSecondary[client] = 796;
				case 3:
					medicSecondary[client] = 805;
				case 4:
					medicSecondary[client] = 885;
				case 5:
					medicSecondary[client] = 894;
				case 6:
					medicSecondary[client] = 903;
				case 7:
					medicSecondary[client] = 912;
				case 8:
					medicSecondary[client] = 961;
				case 9:
					medicSecondary[client] = 970;
				case 10:
					medicSecondary[client] = 15008;
				case 11:
					medicSecondary[client] = 15010;
				case 12:
					medicSecondary[client] = 15025;
				case 13:
					medicSecondary[client] = 15039;
				case 14:
					medicSecondary[client] = 15050;
				case 15:
					medicSecondary[client] = 15078;
				case 16:
					medicSecondary[client] = 15097;
				case 17:
					medicSecondary[client] = 15121;
				case 18:
					medicSecondary[client] = 15122;
				case 19:
					medicSecondary[client] = 15123;
				case 20:
					medicSecondary[client] = 15145;
				case 21:
					medicSecondary[client] = 15146;
			}
		}
		case 2:
			medicSecondary[client] = 35;
		case 3:
			medicSecondary[client] = 411;
		case 4:
			medicSecondary[client] = 998;
	}

	switch (crandomint(1, 6))
	{
		case 1:
			medicMelee[client] = 37;
		case 2:
			medicMelee[client] = 173;
		case 3:
			medicMelee[client] = 304;
		case 4:
			medicMelee[client] = 413;
		case 5:
			medicMelee[client] = 1003;
		case 6:
			medicMelee[client] = 1143;
	}

	switch (crandomint(1, 9))
	{
		case 1:
		{
			switch (crandomint(1, 25))
			{
				case 1:
					sniperPrimary[client] = 664;
				case 2:
					sniperPrimary[client] = 792;
				case 3:
					sniperPrimary[client] = 801;
				case 4:
					sniperPrimary[client] = 881;
				case 5:
					sniperPrimary[client] = 890;
				case 6:
					sniperPrimary[client] = 899;
				case 7:
					sniperPrimary[client] = 908;
				case 8:
					sniperPrimary[client] = 957;
				case 9:
					sniperPrimary[client] = 966;
				case 10:
					sniperPrimary[client] = 1005;
				case 11:
					sniperPrimary[client] = 15000;
				case 12:
					sniperPrimary[client] = 15007;
				case 13:
					sniperPrimary[client] = 15019;
				case 14:
					sniperPrimary[client] = 15023;
				case 15:
					sniperPrimary[client] = 15033;
				case 16:
					sniperPrimary[client] = 15059;
				case 17:
					sniperPrimary[client] = 15070;
				case 18:
					sniperPrimary[client] = 15071;
				case 19:
					sniperPrimary[client] = 15072;
				case 20:
					sniperPrimary[client] = 15111;
				case 21:
					sniperPrimary[client] = 15112;
				case 22:
					sniperPrimary[client] = 15135;
				case 23:
					sniperPrimary[client] = 15136;
				case 24:
					sniperPrimary[client] = 15154;
				case 25:
					sniperPrimary[client] = 30665;
			}
		}
		case 2:
			sniperPrimary[client] = 56;
		case 3:
			sniperPrimary[client] = 230;
		case 4:
			sniperPrimary[client] = 402;
		case 5:
			sniperPrimary[client] = 526;
		case 6:
			sniperPrimary[client] = 752;
		case 7:
			sniperPrimary[client] = 851;
		case 8:
			sniperPrimary[client] = 1092;
		case 9:
			sniperPrimary[client] = 1098;
	}

	switch (crandomint(1, 6))
	{
		case 1:
		{
			switch (crandomint(1, 12))
			{
				case 1:
					sniperSecondary[client] = 1083;
				case 2:
					sniperSecondary[client] = 1105;
				case 3:
					sniperSecondary[client] = 1149;
				case 4:
					sniperSecondary[client] = 15001;
				case 5:
					sniperSecondary[client] = 15022;
				case 6:
					sniperSecondary[client] = 15032;
				case 7:
					sniperSecondary[client] = 15037;
				case 8:
					sniperSecondary[client] = 15058;
				case 9:
					sniperSecondary[client] = 15076;
				case 10:
					sniperSecondary[client] = 15110;
				case 11:
					sniperSecondary[client] = 15134;
				case 12:
					sniperSecondary[client] = 15153;
			}
		}
		case 2:
			sniperSecondary[client] = 57;
		case 3:
			sniperSecondary[client] = 58;
		case 4:
			sniperSecondary[client] = 231;
		case 5:
			sniperSecondary[client] = 642;
		case 6:
			sniperSecondary[client] = 751;
	}

	switch (crandomint(1, 3))
	{
		case 1:
			sniperMelee[client] = 171;
		case 2:
			sniperMelee[client] = 232;
		case 3:
			sniperMelee[client] = 401;
	}

	switch (crandomint(1, 5))
	{
		case 1:
		{
			switch (crandomint(1, 14))
			{
				case 1:
					spyPrimary[client] = 161;
				case 2:
					spyPrimary[client] = 1006;
				case 3:
					spyPrimary[client] = 1142;
				case 4:
					spyPrimary[client] = 15011;
				case 5:
					spyPrimary[client] = 15027;
				case 6:
					spyPrimary[client] = 15042;
				case 7:
					spyPrimary[client] = 15051;
				case 8:
					spyPrimary[client] = 15062;
				case 9:
					spyPrimary[client] = 15063;
				case 10:
					spyPrimary[client] = 15064;
				case 11:
					spyPrimary[client] = 15103;
				case 12:
					spyPrimary[client] = 15128;
				case 13:
					spyPrimary[client] = 15127;
				case 14:
					spyPrimary[client] = 15149;
			}
		}
		case 2:
			spyPrimary[client] = 61;
		case 3:
			spyPrimary[client] = 224;
		case 4:
			spyPrimary[client] = 460;
		case 5:
			spyPrimary[client] = 525;
	}

	switch (crandomint(1, 5))
	{
		case 1:
		{
			switch (crandomint(1, 21))
			{
				case 1:
					spyMelee[client] = 574;
				case 2:
					spyMelee[client] = 638;
				case 3:
					spyMelee[client] = 665;
				case 4:
					spyMelee[client] = 727;
				case 5:
					spyMelee[client] = 794;
				case 6:
					spyMelee[client] = 803;
				case 7:
					spyMelee[client] = 883;
				case 8:
					spyMelee[client] = 892;
				case 9:
					spyMelee[client] = 901;
				case 10:
					spyMelee[client] = 910;
				case 11:
					spyMelee[client] = 959;
				case 12:
					spyMelee[client] = 968;
				case 13:
					spyMelee[client] = 15062;
				case 14:
					spyMelee[client] = 15094;
				case 15:
					spyMelee[client] = 15095;
				case 16:
					spyMelee[client] = 15096;
				case 17:
					spyMelee[client] = 15118;
				case 18:
					spyMelee[client] = 15119;
				case 19:
					spyMelee[client] = 15143;
				case 20:
					spyMelee[client] = 15144;
				case 21:
					spyMelee[client] = 30758;
			}
		}
		case 2:
			spyMelee[client] = 225;
		case 3:
			spyMelee[client] = 356;
		case 4:
			spyMelee[client] = 461;
		case 5:
			spyMelee[client] = 649;
	}

	switch (crandomint(1, 4))
	{
		case 1:
			spyWatch[client] = 59;
		case 2:
			spyWatch[client] = 60;
		case 3:
			spyWatch[client] = 297;
		case 4:
			spyWatch[client] = 947;
	}
}

stock void RandomF2Phat(const int client)
{
	switch (randomf2phat)
	{
		case 1: // alien swarm parasite
			CreateHat(client, 189);
		case 2: // bill's hat
			CreateHat(client, 126);
		case 3: // ellis' hat
			CreateHat(client, 263);
		case 4: // ghastlier gibus
			CreateHat(client, 279);
		case 5: // ghastlierest gibus
			CreateHat(client, 584);
		case 6: // the galvanized gibus
			CreateHat(client, 30003);
		default: // most popular gibus
			CreateHat(client, 116);
	}
}