// from https://github.com/EfeDursun125/CS-EBOT/blob/main/source/waypoint.cpp
// higher version waypoints will be ignored due to updated waypoint system
const int WaypointVersion = 2;
const int MaxWaypoints = 4096;
const int MaxPathIndex = 8;

int nearestIndex = -1;
int savedIndex = -1;
bool showWaypoints = false;
float waypointDrawTime = 0.0;
float waypointUpdate = 0.0;
float nextDrawTime = 0.1;

bool m_hasHealthWaypoints = false;
bool m_hasAmmoWaypoints = false;
bool m_hasSniperWaypoints = false;
bool m_hasStickyWaypoints = false;
bool m_hasRouteWaypoints = false;
bool m_hasSentryWaypoints = false;
bool m_hasTeleporterEnterWaypoints = false;
bool m_hasTeleporterExitWaypoints = false;
bool m_hasRocketJumpWaypoints = false;

int m_lastFailedWaypoint[TFMaxPlayers];

char m_aboutTheWaypoint[192] = "";

enum
{
    WAYPOINT_JUMP = 1,
    WAYPOINT_SNIPER = 2,
    WAYPOINT_DEFEND = 3,
    WAYPOINT_FALLDAMAGE = 4,
    WAYPOINT_CROUCH = 5,
    WAYPOINT_AMMO = 6,
    WAYPOINT_HEALTH = 7,
    WAYPOINT_RESUPPLY = 8,
    WAYPOINT_DOUBLEJUMP = 9,
    WAYPOINT_ROCKETJUMP = 10,
    WAYPOINT_DEMOMANCAMP = 11,
    WAYPOINT_SENTRY = 12,
    WAYPOINT_ROUTE = 13,
    WAYPOINT_TELEPORTERENTER = 14,
    WAYPOINT_TELEPORTEREXIT = 15,
    WAYPOINT_CAPTUREPOINT = 16,
    WAYPOINT_NOSTRAFE = 17,
    WAYPOINT_NOSPY = 18,
    WAYPOINT_DEMOCHARGE = 19
};

enum AStarState_t {OPEN, CLOSED, NEW};
enum struct AStar_t
{
    float g;
    float f;
    int parent;
    AStarState_t state;
}

enum struct WaypointHeader
{
    char author[32];
	int fileVersion;
	int pointNumber;
}

enum struct Path
{
    int index;
    int activeArea;

	int flags;
	float radius;
    int team;

    float distance[8];
    int pathIndex[8];

    float origin[3];
	float campStart[3];
	float campEnd[3];
}

enum struct PathOLD
{
    int index;
    int activeArea;

	int flags;
	int radius;
    int team;

    int distance[8];
    int pathIndex[8];

    int origin[3];
	int campStart[3];
	int campEnd[3];
}

enum struct SearchNode
{
    float cost;
    int index;
}

bool m_hasWaypoints;
int m_waypointNumber;
Path m_paths[MaxWaypoints];

stock void InitializeWaypoints()
{
    for (int i = 0; i < m_waypointNumber; i++)
        WaypointClear(i);

    waypointDrawTime = 0.0;
    waypointUpdate = 0.0;
    nextDrawTime = 0.5;
    m_hasWaypoints = false;
    m_waypointNumber = 0;

    WaypointLoad();
    ResetDangerMap();
}

int deathsWPT[MaxWaypoints + 1];
float damageWPT[MaxWaypoints + 1];

stock void ResetDangerMap()
{
    for (int i = 0; i < m_waypointNumber; i++)
    {
        deathsWPT[i] = 0;
        damageWPT[i] = 0.0;
    }
}

stock void IncreaseDamage(int waypoint, float newDamage)
{
    damageWPT[waypoint] += newDamage;
}

stock float GetDamage(int waypoint)
{
    return damageWPT[waypoint];
}

stock void IncreaseDeaths(int waypoint)
{
    deathsWPT[waypoint] += 1;
}

stock int GetDeaths(int waypoint)
{
    return deathsWPT[waypoint] + 1;
}

stock void UpdateWaypoints()
{
    if (waypointUpdate > GetGameTime())
        return;
    
    for (int i = 0; i < m_waypointNumber; i++)
    {
        damageWPT[i] -= 12.0;
        if (damageWPT[i] < 0.0)
            damageWPT[i] = 0.0;
    }
    
    waypointUpdate = GetGameTime() + 10.0;
}

stock void DrawWaypoints()
{
    if (!showWaypoints)
        return;
    
    if (!IsValidClient(m_hostEntity))
        FindHostEntity();

    if (waypointDrawTime > GetGameTime())
        return;
    
    m_lastFailedWaypoint[m_hostEntity] = -1;

    // reset the minimal distance changed before
    float nearestDistance = 999999999.0;
    float range = 768.0;

    for (int i = 0; i < m_waypointNumber; i++)
    {
        float distance = GetVectorDistance(m_paths[i].origin, GetOrigin(m_hostEntity), true);
        if (distance > Squaredf(range))
            continue;
        
        if (distance > Squaredf(75.0) && !ClientViewsOrigin(m_hostEntity, m_paths[i].origin, 0.6))
            continue;

        // check the distance
        if (distance < nearestDistance)
        {
            nearestIndex = i;
            nearestDistance = distance;
        }

        // r, g, b, alpha
        int color[4] = {0, 255, 0, 255};
        float height = 72.0;

        if (m_paths[i].flags == WAYPOINT_CROUCH)
            height = 36.0;
        
        if (m_paths[i].flags == WAYPOINT_RESUPPLY)
        {
            color[0] = 188;
            color[1] = 52;
            color[2] = 123;
        }
        else if (m_paths[i].flags == WAYPOINT_SNIPER)
            color[2] = 255;
        else if (m_paths[i].flags == WAYPOINT_DEFEND)
        {
            color[0] = 255;
            color[1] = 125;
        }
        else if (m_paths[i].flags == WAYPOINT_FALLDAMAGE)
        {
            color[0] = 128;
            color[1] = 128;
            color[2] = 128;
        }
        else if (m_paths[i].flags == WAYPOINT_JUMP)
        {
            color[0] = 255;
            color[2] = 255;
        }
        else if (m_paths[i].flags == WAYPOINT_HEALTH)
        {
            color[0] = 255;
            color[2] = 255;
        }
        else if (m_paths[i].flags == WAYPOINT_AMMO)
        {
            color[0] = 55;
            color[1] = 64;
            color[2] = 68;
        }
        else if (m_paths[i].flags == WAYPOINT_CAPTUREPOINT)
            color[0] = 255;
        else if (m_paths[i].flags == WAYPOINT_DOUBLEJUMP)
        {
            color[0] = 160;
            color[1] = 240;
            color[2] = 20;
        }
        else if (m_paths[i].flags == WAYPOINT_NOSTRAFE)
        {
            color[0] = 147;
            color[1] = 128;
            color[2] = 147;
        }
        else if (m_paths[i].flags == WAYPOINT_ROCKETJUMP)
        {
            color[0] = 128;
            color[1] = 6;
            color[2] = 150;
        }
        else if (m_paths[i].flags == WAYPOINT_NOSPY)
        {
            color[0] = 160;
            color[1] = 124;
            color[2] = 122;
        }
        else if (m_paths[i].flags == WAYPOINT_DEMOCHARGE)
        {
            color[0] = 139;
            color[1] = 189;
            color[2] = 204;
        }
        else if (m_paths[i].flags == WAYPOINT_ROUTE)
        {
            color[0] = 70;
            color[1] = 70;
            color[2] = 70;
        }
        else if (m_paths[i].flags == WAYPOINT_SENTRY)
        {
            color[0] = 163;
            color[1] = 155;
            color[2] = 198;
        }
        else if (m_paths[i].flags == WAYPOINT_TELEPORTERENTER)
        {
            color[0] = 127;
            color[1] = 131;
            color[2] = 168;
        }
        else if (m_paths[i].flags == WAYPOINT_TELEPORTEREXIT)
        {
            color[0] = 131;
            color[1] = 127;
            color[2] = 168;
        }
        else if (m_paths[i].flags == WAYPOINT_DEMOMANCAMP)
        {
            color[0] = 125;
            color[1] = 47;
            color[2] = 142;
        }
        else if (m_paths[i].team == 2)
        {
            color[0] = 255;
            color[1] = 0;
        }
        else if (m_paths[i].team == 3)
        {
            color[1] = 0;
            color[2] = 255;
        }
        else if (m_paths[i].team == 4)
            color[1] = 255;
        else if (m_paths[i].team == 5)
            color[0] = 255;

        // convert int to float
        float indexOrigin[3];
        indexOrigin = m_paths[i].origin;
        indexOrigin[2] += height;

        TE_SetupBeamPoints(m_paths[i].origin, indexOrigin, m_beamIndex, m_beamIndex, 0, 30, nextDrawTime + GetGameFrameTime(), 1.75, 1.75, 5, 0.0, color, 20);
        TE_SendToClient(m_hostEntity);
    }

    // draw a paths, camplines and danger directions for nearest waypoint
    if (nearestIndex != -1 && GetVectorDistance(GetEyePosition(m_hostEntity), m_paths[nearestIndex].origin, true) <= Squaredf(range) && IsVisibleWaypoint(GetEyePosition(m_hostEntity), m_paths[nearestIndex].origin))
    {
        // draw the camplines
        if (m_paths[nearestIndex].flags == WAYPOINT_SNIPER || m_paths[nearestIndex].flags == WAYPOINT_DEFEND || m_paths[nearestIndex].flags == WAYPOINT_SENTRY || m_paths[nearestIndex].flags == WAYPOINT_DEMOMANCAMP || m_paths[nearestIndex].flags == WAYPOINT_TELEPORTERENTER || m_paths[nearestIndex].flags == WAYPOINT_TELEPORTEREXIT)
        {
            float height = 72.0;
            if (m_paths[nearestIndex].flags == WAYPOINT_CROUCH)
                height = 36.0;

            // convert int to float
            float indexOrigin[3];
            indexOrigin = m_paths[nearestIndex].origin;
            indexOrigin[2] += height;

            TE_SetupBeamPoints(indexOrigin, m_paths[nearestIndex].campStart, m_laserIndex, m_laserIndex, 0, 30, nextDrawTime + GetGameFrameTime(), 1.0, 1.0, 5, 0.0, {255, 0, 0, 255}, 20);
            TE_SendToClient(m_hostEntity);
            TE_SetupBeamPoints(indexOrigin, m_paths[nearestIndex].campEnd, m_laserIndex, m_laserIndex, 0, 30, nextDrawTime + GetGameFrameTime(), 1.0, 1.0, 5, 0.0, {255, 0, 0, 255}, 20);
            TE_SendToClient(m_hostEntity);
        }

        // draw the connections
        for (int i = 0; i < MaxPathIndex; i++)
        {
            if (m_paths[nearestIndex].pathIndex[i] == -1)
                continue;

            float height = 36.0;
            if (m_paths[nearestIndex].flags == WAYPOINT_CROUCH)
                height = 18.0;
            
            float height2 = 36.0;
            if (m_paths[m_paths[nearestIndex].pathIndex[i]].flags == WAYPOINT_CROUCH)
                height2 = 18.0;
            
            float indexOrigin[3];
            indexOrigin = m_paths[nearestIndex].origin;
            indexOrigin[2] = m_paths[nearestIndex].origin[2] + height;

            float indexOrigin2[3];
            indexOrigin2 = m_paths[m_paths[nearestIndex].pathIndex[i]].origin;
            indexOrigin2[2] = m_paths[m_paths[nearestIndex].pathIndex[i]].origin[2] + height2;

            if (IsConnected(m_paths[nearestIndex].pathIndex[i], nearestIndex)) // two way connection
            {
                TE_SetupBeamPoints(indexOrigin, indexOrigin2, m_laserIndex, m_laserIndex, 0, 30, nextDrawTime + GetGameFrameTime(), 1.0, 1.0, 5, 0.0, {255, 255, 0, 255}, 20);
                TE_SendToClient(m_hostEntity);
            }
            else // one way connection
            {
                TE_SetupBeamPoints(indexOrigin, indexOrigin2, m_laserIndex, m_laserIndex, 0, 30, nextDrawTime + GetGameFrameTime(), 1.0, 1.0, 5, 0.0, {255, 255, 255, 255}, 20);
                TE_SendToClient(m_hostEntity);
            }
        }

        // now look for oneway incoming connections
        for (int i = 0; i < m_waypointNumber; i++)
        {
            if (IsConnected(m_paths[i].index, m_paths[nearestIndex].index) && !IsConnected(m_paths[nearestIndex].index, m_paths[i].index))
            {
                float height = 36.0;
                if (m_paths[nearestIndex].flags == WAYPOINT_CROUCH)
                    height = 18.0;
                
                float height2 = 36.0;
                if (m_paths[i].flags == WAYPOINT_CROUCH)
                    height2 = 18.0;

                float indexOrigin[3];
                indexOrigin = m_paths[nearestIndex].origin;
                indexOrigin[2] = m_paths[nearestIndex].origin[2] + height;

                float indexOrigin2[3];
                indexOrigin2 = m_paths[i].origin;
                indexOrigin2[2] = m_paths[i].origin[2] + height2;

                TE_SetupBeamPoints(indexOrigin, indexOrigin2, m_laserIndex, m_laserIndex, 0, 30, nextDrawTime + GetGameFrameTime(), 1.0, 1.0, 5, 0.0, {0, 192, 96, 255}, 20);
                TE_SendToClient(m_hostEntity);
            }
        }

        // if radius is nonzero, draw a square
        int color[4] = {0, 0, 255, 255};
        float origin[3];
        origin = m_paths[nearestIndex].origin;
        float height = 18.0;

        if (m_paths[nearestIndex].flags == WAYPOINT_CROUCH)
            height = 9.0;
        
        origin[2] += height;

        if (m_paths[nearestIndex].team == 2)
        {
            color[0] = 255;
            color[1] = 0;
            color[2] = 0;
        }
        else if (m_paths[nearestIndex].team == 3)
        {
            color[0] = 0;
            color[1] = 0;
            color[2] = 255;
        }
        else if (m_paths[nearestIndex].team == 4)
        {
            color[0] = 0;
            color[1] = 255;
            color[2] = 0;
        }
        else if (m_paths[nearestIndex].team == 5)
        {
            color[0] = 255;
            color[1] = 255;
            color[2] = 0;
        }
        else
        {
            color[0] = 255;
            color[1] = 255;
            color[2] = 255;
        }

        if (m_paths[nearestIndex].radius > 0.0)
        {
            float root = m_paths[nearestIndex].radius;
            TE_SetupBeamPoints(VectorWithRoot(origin, root, root), VectorWithRoot(origin, -root, root), m_laserIndex, m_laserIndex, 0, 30, nextDrawTime + GetGameFrameTime(), 1.0, 1.0, 5, 0.0, color, 20);
            TE_SendToClient(m_hostEntity);
            TE_SetupBeamPoints(VectorWithRoot(origin, root, root), VectorWithRoot(origin, root, -root), m_laserIndex, m_laserIndex, 0, 30, nextDrawTime + GetGameFrameTime(), 1.0, 1.0, 5, 0.0, color, 20);
            TE_SendToClient(m_hostEntity);
            TE_SetupBeamPoints(VectorWithRoot(origin, -root, -root), VectorWithRoot(origin, root, -root), m_laserIndex, m_laserIndex, 0, 30, nextDrawTime + GetGameFrameTime(), 1.0, 1.0, 5, 0.0, color, 20);
            TE_SendToClient(m_hostEntity);
            TE_SetupBeamPoints(VectorWithRoot(origin, -root, -root), VectorWithRoot(origin, -root, root), m_laserIndex, m_laserIndex, 0, 30, nextDrawTime + GetGameFrameTime(), 1.0, 1.0, 5, 0.0, color, 20);
            TE_SendToClient(m_hostEntity);
        }
        else
        {
            float root = 8.0;
            TE_SetupBeamPoints(VectorWithRoot(origin, root, -root), VectorWithRoot(origin, -root, root), m_laserIndex, m_laserIndex, 0, 30, nextDrawTime + GetGameFrameTime(), 1.0, 1.0, 5, 0.0, color, 20);
            TE_SendToClient(m_hostEntity);
            TE_SetupBeamPoints(VectorWithRoot(origin, -root, -root), VectorWithRoot(origin, root, root), m_laserIndex, m_laserIndex, 0, 30, nextDrawTime + GetGameFrameTime(), 1.0, 1.0, 5, 0.0, color, 20);
            TE_SendToClient(m_hostEntity);
        }

        int i = nearestIndex;
        char wpteam[128] = "All";

        if (m_paths[i].team == 2)
            wpteam = "Red Only";
        else if (m_paths[i].team == 3)
            wpteam = "Blu Only";
        else if (m_paths[i].team == 4)
            wpteam = "Green Only";
        else if (m_paths[i].team == 5)
            wpteam = "Yellow Only";
        
        char wpflags[128] = "None";
        if (m_paths[i].flags == WAYPOINT_AMMO)
            wpflags = "Ammo";
        else if (m_paths[i].flags == WAYPOINT_CAPTUREPOINT)
            wpflags = "Capture";
        else if (m_paths[i].flags == WAYPOINT_CROUCH)
            wpflags = "Crouch";
        else if (m_paths[i].flags == WAYPOINT_DEMOMANCAMP)
            wpflags = "Demoman Sticky Trap";
        else if (m_paths[i].flags == WAYPOINT_ROUTE)
            wpflags = "Route";
        else if (m_paths[i].flags == WAYPOINT_DOUBLEJUMP)
            wpflags = "Double Jump";
        else if (m_paths[i].flags == WAYPOINT_JUMP)
            wpflags = "Jump";
        else if (m_paths[i].flags == WAYPOINT_HEALTH)
            wpflags = "Health";
        else if (m_paths[i].flags == WAYPOINT_DEFEND)
            wpflags = "Defend";
        else if (m_paths[i].flags == WAYPOINT_FALLDAMAGE)
            wpflags = "Fall Damage";
        else if (m_paths[i].flags == WAYPOINT_RESUPPLY)
            wpflags = "Resupply";
        else if (m_paths[i].flags == WAYPOINT_ROCKETJUMP)
            wpflags = "Rocket Jump";
        else if (m_paths[i].flags == WAYPOINT_SENTRY)
            wpflags = "Sentry";
        else if (m_paths[i].flags == WAYPOINT_SNIPER)
            wpflags = "Sniper";
        else if (m_paths[i].flags == WAYPOINT_TELEPORTERENTER)
            wpflags = "Teleporter Enter";
        else if (m_paths[i].flags == WAYPOINT_TELEPORTEREXIT)
            wpflags = "Teleporter Exit";
        else if (m_paths[i].flags == WAYPOINT_NOSTRAFE)
            wpflags = "No Strafe (Safe Movement)";
        else if (m_paths[i].flags == WAYPOINT_NOSPY)
            wpflags = "No Spy";
        else if (m_paths[i].flags == WAYPOINT_DEMOCHARGE)
            wpflags = "Demo Charge";
        
        SetHudTextParams(0.0, 0.0, nextDrawTime + GetGameFrameTime(), 255, 255, 255, 255, 2, 1.0, 0.0, 0.0);
        ShowHudText(m_hostEntity, -1, "Waypoint Index: %d (%d/%d)\nTeam: %s\nActive Area: %d\nFlags: %s\nRadius: %d\nOrigin: %d %d %d\nAim Start Origin: %d %d %d\nAim End Origin: %d %d %d\nDamage: %d\nDeaths: %d", m_paths[i].index, m_waypointNumber, MaxWaypoints, wpteam, m_paths[i].activeArea, wpflags, RoundFloat(m_paths[i].radius), RoundFloat(m_paths[i].origin[0]), RoundFloat(m_paths[i].origin[1]), RoundFloat(m_paths[i].origin[2]), RoundFloat(m_paths[i].campStart[0]), RoundFloat(m_paths[i].campStart[1]), RoundFloat(m_paths[i].campStart[2]), RoundFloat(m_paths[i].campEnd[0]), RoundFloat(m_paths[i].campEnd[1]), RoundFloat(m_paths[i].campEnd[2]), RoundFloat(GetDamage(i)), (GetDeaths(i) - 1));
    }

    waypointDrawTime = GetGameTime() + nextDrawTime;
}

stock bool IsVisibleWaypointIndex(int start, int end, float startheight = 18.0, float endheight = 18.0)
{
	float fstart[3];
	float fend[3];

	fstart = m_paths[start].origin;
	fstart[2] += startheight;

	fstart = m_paths[end].origin;
	fend[2] += endheight;

	Handle trace = TR_TraceRayFilterEx(fstart, fend, MASK_CUSTOM, RayType_EndPoint, TraceEntityFilterStuff);
	float flFraction = TR_GetFraction(trace);
	delete trace;

	if (flFraction >= 1.0) 
		return !(flFraction == 0.0);
    
	return false;
}

// check if waypoint A has a connection to waypoint B
stock bool IsConnected(int pointA, int pointB)
{
    for (int i = 0; i < MaxPathIndex; i++)
    {
        if (m_paths[pointA].pathIndex[i] == pointB)
            return true;
    }
    return false;
}

stock bool IsWaypointReachable(float start[3], float goal[3])
{
    // cant connect to itself...
    if (start[0] == goal[0] && start[1] == goal[1] && start[2] == goal[2])
        return false;
    
    float src[3];
    src = start;
    src[2] += 32.0;
    float destination[3];
    destination = goal;
    destination[2] += 32.0;

    // unable to reach with walking
    if ((destination[2] - src[2]) > 72.0)
        return false;

    float distance = GetVectorDistance(destination, src);

    // is the destination not close enough?
    if (distance > 333)
        return false;

    // if waypoint is visible from current position
    if (IsVisibleWaypoint(src, destination))
    {
        // is dest node higher than src? (45 is max jump height)
        if (destination[2] > src[2] + 44.0)
        {
            float sourceNew[3];
            sourceNew = destination;
            float destinationNew[3];
            destinationNew = destination;
            destinationNew[2] = destinationNew[2] - 50.0; // straight down 50 units

            Handle tr = TR_TraceRayFilterEx(sourceNew, destinationNew, MASK_PLAYERSOLID, RayType_EndPoint, WaypointFilter);
            float fr = TR_GetFraction(tr);
            CloseHandle(tr);

            // check if we didn't hit anything, if not then it's in mid-air
            if (fr >= 1.0)
                return false; // can't reach this one
        }

        // check if distance to ground drops more than step height at points between source and destination...
        float sub[3];
        SubtractVectors(destination, src, sub);
        float direction[3];
        NormalizeVector(sub, direction); // 1 unit long
        float check[3];
        check = src;
        float down[3];
        down = src;

        down[2] = down[2] - 1000.0; // straight down 1000 units

        Handle tr = TR_TraceRayFilterEx(check, down, MASK_PLAYERSOLID, RayType_EndPoint, WaypointFilter);

        float lastHeight = TR_GetFraction(tr) * 1000.0; // height from ground
        distance = GetVectorDistance(destination, check); // distance from goal

        CloseHandle(tr);

        while (distance > 10.0)
        {
            // move 10 units closer to the goal...
            check[0] = check[0] + (direction[0] * 10.0);
            check[1] = check[1] + (direction[1] * 10.0);
            check[2] = check[2] + (direction[2] * 10.0);

            down = check;
            down[2] = down[2] - 1000.0; // straight down 1000 units

            Handle tr2 = TR_TraceRayFilterEx(check, down, MASK_PLAYERSOLID, RayType_EndPoint, WaypointFilter);

            float height = TR_GetFraction(tr2) * 1000.0; // height from ground
            
            CloseHandle(tr2);

            // is the current height greater than the step height?
            if (height < lastHeight - 18.0)
                return false; // can't get there without jumping...

            lastHeight = height;
            distance = GetVectorDistance(destination, check); // distance from goal
        }

        return true;
    }

    return false;
}

stock void AutoWaypoint()
{
    if (GetConVarInt(EBotAutoWaypoint) != 1)
        return;

    if (!IsValidClient(m_hostEntity))
    {
        FindHostEntity();
        return;
    }

    if (!(GetEntityFlags(m_hostEntity) & FL_ONGROUND))
        return;

    if (!IsMoving(m_hostEntity))
        return;

    int index = -1;
    float maxDistance = 99999999.0;
    for (int i = 0; i < m_waypointNumber; i++)
    {
        float distance = GetVectorDistance(m_paths[i].origin, GetOrigin(m_hostEntity), true);
        if (distance < maxDistance)
        {
            index = i;
            maxDistance = distance;
        }
    }

    if (index != -1 && GetVectorDistance(GetOrigin(m_hostEntity), m_paths[index].origin) > GetConVarFloat(EBotDistance))
        WaypointAdd(GetOrigin(m_hostEntity));
}

stock void WaypointAdd(float origin[3], bool cdebug = true)
{
    if (m_waypointNumber >= MaxWaypoints)
    {
        if (cdebug)
            PrintHintTextToAll("Maximum number of waypoints reached (%d/%d)", MaxWaypoints, MaxWaypoints);

        return;
    }

    int index = m_waypointNumber;
    m_waypointNumber++;
    
    m_paths[index].index = index;
    m_paths[index].activeArea = 0;
    m_paths[index].flags = 0;
    m_paths[index].radius = GetConVarFloat(EBotRadius);
    m_paths[index].team = 0;

    m_paths[index].origin = origin;

    m_paths[index].campStart = {0.0, 0.0, 0.0};
    m_paths[index].campEnd = {0.0, 0.0, 0.0};

    for (int i = 0; i < MaxPathIndex; i++)
    {
        m_paths[index].pathIndex[i] = -1;
        m_paths[index].distance[i] = 0.0;
    }

    // calculate all the paths to this new waypoint
    for (int i = 0; i < m_waypointNumber; i++)
    {
        // skip the waypoint that was just added
        if (index == i)
            continue; 

        // check if the waypoint is reachable from the new one (one-way)
        if (IsWaypointReachable(m_paths[index].origin, m_paths[i].origin))
            AddPath(index, i, GetVectorDistance(m_paths[index].origin, m_paths[i].origin));

        // check if the new one is reachable from the waypoint (other way)
        if (IsWaypointReachable(m_paths[i].origin, m_paths[index].origin))
            AddPath(i, index, GetVectorDistance(m_paths[i].origin, m_paths[index].origin));
    }

    //CalculateWayzone(index);
    if (cdebug)
        PrintHintTextToAll("Waypoint added to %d %d %d", m_paths[index].origin[0], m_paths[index].origin[1], m_paths[index].origin[2]);
}

/*stock void CalculateWayzone(int index)
{
    float start[3];
    float direction[3];
    TraceResult tr;
    bool wayBlocked = false;
    for (int i = 0; i < MaxPathIndex; i++)
    {
        if (m_paths[index].pathIndex[i] != -1)
        {
            m_paths[index].radius = 0.0;
            return;
        }
    }
    for (float scanDistance = 16.0; scanDistance < 144.0; scanDistance += 16.0)
    {
        start = VectorAsFloat(m_paths[index].origin);
        MakeVectors(nullvec);
        direction = g_pGlobals->v_forward * scanDistance;
        direction = direction.ToAngles();
        m_paths[index].radius = scanDistance;
        for (float circleRadius = 0.0f; circleRadius < 180.0f; circleRadius += 5.0f)
        {
            MakeVectors(direction);
            Vector radiusStart = start - g_pGlobals->v_forward * scanDistance;
            Vector radiusEnd = start + g_pGlobals->v_forward * scanDistance;
            TraceHull(radiusStart, radiusEnd, true, head_hull, null, &tr);
            Vector dropStart = start + (g_pGlobals->v_forward * scanDistance);
            Vector dropEnd = dropStart - Vector(0.0f, 0.0f, scanDistance + 60.0f);
            TraceHull(dropStart, dropEnd, true, head_hull, null, &tr);
            if (tr.flFraction >= 1.0f)
            {
                wayBlocked = true;
                path->radius -= 16.0f;
                break;
            }
            dropStart = start - (g_pGlobals->v_forward * scanDistance);
            dropEnd = dropStart - Vector(0.0f, 0.0f, scanDistance + 60.0f);
            TraceHull(dropStart, dropEnd, true, head_hull, null, &tr);
            if (tr.flFraction >= 1.0f)
            {
                wayBlocked = true;
                path->radius -= 16.0f;
                break;
            }
            radiusEnd.z += 34.0f;
            TraceHull(radiusStart, radiusEnd, true, head_hull, null, &tr);
            if (tr.flFraction < 1.0f)
            {
                wayBlocked = true;
                path->radius -= 16.0f;
                break;
            }
            direction.y = AngleNormalize(direction.y + circleRadius);
        }
        if (wayBlocked)
            break;
    }
    path->radius -= 16.0f;
    if (path->radius < 0.0f)
        path->radius = 0.0f;
}*/

stock void AddPath(int srcIndex, int destIndex, float distance)
{
    if (srcIndex == destIndex)
    {
        PrintHintTextToAll("Waypoint cannot be connected to itself");
        return;
    }

    // don't allow paths get connected twice
    for (int i = 0; i < MaxPathIndex; i++)
    {
        if (m_paths[srcIndex].pathIndex[i] == destIndex)
        {
            PrintHintTextToAll("Denied path creation from %d to %d (path already exists)", srcIndex, destIndex);
            return;
        }
    }

    // check for free space in the connection indices
    for (int i = 0; i < MaxPathIndex; i++)
    {
        if (m_paths[srcIndex].pathIndex[i] == -1)
        {
            m_paths[srcIndex].pathIndex[i] = destIndex;
            m_paths[srcIndex].distance[i] = distance;
            PrintHintTextToAll("Path added from %d to %d", srcIndex, destIndex);
            return;
        }
    }

    // there wasn't any free space. try exchanging it with a long-distance path
    float maxDistance = -99999.0;
    int slotID = -1;

    for (int i = 0; i < MaxPathIndex; i++)
    {
        if (m_paths[srcIndex].distance[i] > maxDistance)
        {
            maxDistance = m_paths[srcIndex].distance[i];
            slotID = i;
        }
    }

    if (slotID != -1)
    {
        PrintHintTextToAll("Reached to path limit (%d)\nPath added from %d to %d", MaxPathIndex, srcIndex, destIndex);
        m_paths[srcIndex].pathIndex[slotID] = destIndex;
        m_paths[srcIndex].distance[slotID] = distance;
    }
}

// this function allow player to manually remove a path from one waypoint to another
stock void DeletePath(int nodeFrom, int nodeTo)
{
    if (nodeFrom == nodeTo)
        PrintHintTextToAll("They are same waypoint...");

    int index = 0;
    for (index = 0; index < MaxPathIndex; index++)
    {
        if (m_paths[nodeFrom].pathIndex[index] == nodeTo)
        {
            m_paths[nodeFrom].pathIndex[index] = -1; // unassign this path
            m_paths[nodeFrom].distance[index] = 0.0;
            PrintHintTextToAll("Path deleted from %d to %d", nodeFrom, nodeTo);
            return;
        }
    }

    // not found this way ? check for incoming connections then
    index = nodeFrom;
    nodeFrom = nodeTo;
    nodeTo = index;

    for (index = 0; index < MaxPathIndex; index++)
    {
        if (m_paths[nodeFrom].pathIndex[index] == nodeTo)
        {
            m_paths[nodeFrom].pathIndex[index] = -1; // unassign this path
            m_paths[nodeFrom].distance[index] = 0.0;
            PrintHintTextToAll("Path deleted from %d to %d", nodeFrom, nodeTo);
            return;
        }
    }

    PrintHintTextToAll("There is already no path on this waypoint");
}

stock void DeleteWaypointIndex(int index)
{
    if (m_waypointNumber < 0)
    {
        PrintHintTextToAll("No waypoints found");
        return;
    }

    if (index == -1)
    {
        PrintHintTextToAll("Waypoint is invalid");
        return;
    }

    int i, j;
    for (i = 0; i < m_waypointNumber; i++) // delete all references to Node
    {
        for (j = 0; j < MaxPathIndex; j++)
        {
            if (m_paths[i].pathIndex[j] == index)
            {
                m_paths[i].pathIndex[j] = -1;  // unassign this path
                m_paths[i].distance[j] = 0.0;
            }
        }
    }

    for (i = 0; i < m_waypointNumber; i++)
    {
        if (m_paths[i].index > index) // if index bigger than deleted node...
            m_paths[i].index--;

        for (j = 0; j < MaxPathIndex; j++)
        {
            if (m_paths[i].pathIndex[j] > index)
                m_paths[i].pathIndex[j]--;
        }
    }

    // free deleted node
    WaypointClear(index);

    // Rotate Path Array down
    for (i = index; i < m_waypointNumber - 1; i++)
        m_paths[i] = m_paths[i + 1];

    m_waypointNumber--;
    PrintHintTextToAll("Waypoint %d is deleted", index);
}

stock float HeuristicNormal(int start, int goal)
{
    if (m_paths[start].flags == WAYPOINT_DOUBLEJUMP)
        return 0.0;

    return GetWaypointDistance(start, goal);
}

stock float CostNormal(int current, int parent, int client)
{
    if (parent == -1)
        return 1.0;
    
    if (m_paths[current].flags == WAYPOINT_DOUBLEJUMP)
        return 1.0;
    
    // low health or don't want take any damage
    if (m_paths[current].flags == WAYPOINT_FALLDAMAGE && (m_lowHealth[client] || GetRandomInt(1, 3) == 1 || TF2_GetPlayerClass(client) == TFClass_Spy))
        return 65355.0;
    
    // they don't let us jump and heavy can't jump
    if (m_paths[current].flags == WAYPOINT_JUMP && (m_hasEnemiesNear[client] || TF2_GetPlayerClass(client) == TFClass_Heavy))
        return 65355.0;
    
    if (m_paths[current].flags == WAYPOINT_NOSPY && TF2_GetPlayerClass(client) == TFClass_Spy)
        return 65355.0;
    
    float cl = float(client) + 1.0;
	return GetWaypointDistance(current, parent) + float(GetDeaths(current) * GetDeaths(parent)) + GetDamage(current) + GetDamage(parent) + GetRandomFloat(-cl, cl);
}

// A* Pathfinding
stock void AStarFindPath(int srcIndex, int destIndex, int client, float targetposition[3] = NULL_VECTOR)
{
    if (!IsValidClient(client))
        return;

	if (destIndex == -1)
    {
        if (GetConVarInt(EBotDebug) == 1)
        {
            char buffer[128];
            GetClientName(client, buffer, sizeof(buffer));
            PrintHintTextToAll("Destination is invalid\nBot: %s", buffer);
        }

        return;
    }

	if (srcIndex <= 0 || !IsVisible(m_paths[srcIndex].origin, GetEyePosition(client)))
		srcIndex = FindNearestWaypoint(GetOrigin(client), 999999.0, client);
    
    if (srcIndex == destIndex)
		return;

    AStar_t astar[MaxWaypoints];
    for (int i = 0; i < m_waypointNumber; i++)
    {
        astar[i].g = 0.0;
        astar[i].f = 0.0;
        astar[i].parent = -1;
        astar[i].state = NEW;
    }

    astar[srcIndex].g = CostNormal(srcIndex, -1, client);
    astar[srcIndex].f = astar[srcIndex].g + HeuristicNormal(srcIndex, destIndex);
    astar[srcIndex].state = OPEN;
    
    if (m_hidingSpots[client] == null)
    {
        delete m_hidingSpots[client];
        m_hidingSpots[client] = new ArrayList();
    }
    
    if (m_hidingSpots[client] != null)
        m_hidingSpots[client].Clear();

    SearchNode start;
    start.index = srcIndex;
    start.cost = astar[srcIndex].f;

    ArrayList openList = new ArrayList(2);
    openList.PushArray(start, 2);
    while (openList.Length > 0)
    {
        // set current node to the node with the lowest f value
        SearchNode currentIndex;
        openList.Sort(Sort_Ascending, Sort_Float);
        openList.GetArray(0, currentIndex, 2);
        openList.Erase(0);

        // is the current node the goal node?
        if (currentIndex.index == destIndex)
        {
            DeletePathNodes(client);
            
            if (!IsNullVector(targetposition))
            {
                m_positions[client].PushArray(targetposition, 3);
                m_pathIndex[client].Push(destIndex);
                m_positions[client].PushArray(targetposition, 3);
                m_pathIndex[client].Push(destIndex);
            }

            // build the complete path
            while (currentIndex.index != -1)
            {
				AddToPath(currentIndex.index, client);
                currentIndex.index = astar[currentIndex.index].parent;
			}

            delete openList;
            return;
        }

        if (astar[currentIndex.index].state != OPEN)
            continue;

        // put current node into CLOSED list
        astar[currentIndex.index].state = CLOSED;

        // now expand the current node
        for (int i = 0; i < MaxPathIndex; i++)
        {
            int self = m_paths[currentIndex.index].pathIndex[i];
            if (self == -1)
                continue;
            
            // blocked waypoint
            if (m_paths[self].flags == 0 && m_paths[self].activeArea != 0 && m_paths[self].activeArea != currentActiveArea)
                continue;
            
            if (m_paths[self].flags == WAYPOINT_DOUBLEJUMP && TF2_GetPlayerClass(client) != TFClass_Scout)
                continue;

            if (m_paths[self].flags == WAYPOINT_ROCKETJUMP && (TF2_GetPlayerClass(client) != TFClass_Soldier || m_lowHealth[client]))
                continue;

            if (m_paths[self].flags == WAYPOINT_DEMOCHARGE && TF2_GetPlayerClass(client) != TFClass_DemoMan)
                continue;
            else
            {
                int wearable = MaxClients + 1;
	            while ((wearable = FindEntityByClassname(wearable, "tf_wearable_demoshield*")) > MaxClients)
	            {
    	        	if (GetEntPropEnt(wearable, Prop_Send, "m_hOwnerEntity") != client)
                        continue;
                }
            }

            if (self != destIndex)
            {
                if (IsValidClient(m_nearestEnemy[client]))
                {
                    if (ClientViewsOrigin(m_nearestEnemy[client], m_paths[self].origin, 0.55))
                    {
                        float vector[3];
                        vector = m_paths[self].origin;
                        float enemyHeight = GetHeight(m_nearestEnemy[client]);
                        vector[2] += enemyHeight;
                        if (IsVisible(vector, GetEyePosition(m_nearestEnemy[client])))
                        {
                            if (GetVectorDistance(GetOrigin(m_nearestEnemy[client]), vector, true) - GetVectorDistance(GetOrigin(client), vector, true) <= 0.0)
                                continue;
                        }
                    }
                    else
                        m_hidingSpots[client].Push(self);
                }
            }

            // calculate the F value as F = G + H
            float g = astar[currentIndex.index].g + CostNormal(self, currentIndex.index, client);
            float h = HeuristicNormal(self, destIndex);
            float f = g + h;

            if (astar[self].state == NEW || astar[self].f > f)
            {
                // put the current child into open list
                astar[self].parent = currentIndex.index;
                astar[self].state = OPEN;
                astar[self].g = g;
                astar[self].f = f;
                SearchNode next;
                next.index = self;
                next.cost = astar[self].f;
                openList.PushArray(next, 2);
            }
        }
    }

    delete openList;

    m_lastFailedWaypoint[client] = destIndex;
    AStarFindShortestPath(srcIndex, destIndex, client, targetposition);
}

// A* for second try
stock void AStarFindShortestPath(int srcIndex, int destIndex, int client, float targetposition[3] = NULL_VECTOR)
{
	if (destIndex == -1)
    {
        if (GetConVarInt(EBotDebug) == 1)
        {
            char buffer[128];
            GetClientName(client, buffer, sizeof(buffer));
            PrintHintTextToAll("Destination is invalid\nBot: %s", buffer);
        }

        return;
    }
    
	if (srcIndex <= 0 || !IsVisible(m_paths[srcIndex].origin, GetEyePosition(client)))
		srcIndex = FindNearestWaypoint(GetOrigin(client), 999999.0, client);
    
    if (srcIndex == destIndex)
		return;
    
    AStar_t astar[MaxWaypoints];
    for (int i = 0; i < m_waypointNumber; i++)
    {
        astar[i].g = 0.0;
        astar[i].f = 0.0;
        astar[i].parent = -1;
        astar[i].state = NEW;
    }

    astar[srcIndex].f = GetWaypointDistance(srcIndex, destIndex);
    astar[srcIndex].state = OPEN;
    
    SearchNode start;
    start.index = srcIndex;
    start.cost = astar[srcIndex].f;

    ArrayList openList = new ArrayList(2);
    openList.PushArray(start, 2);
    while (openList.Length > 0)
    {
        // set current node to the node with the lowest f value
        openList.Sort(Sort_Ascending, Sort_Float);
        SearchNode currentIndex;
        openList.GetArray(0, currentIndex, 2);
        openList.Erase(0);
		
        // is the current node the goal node?
        if (currentIndex.index == destIndex)
        {
            DeletePathNodes(client);

            if (!IsNullVector(targetposition))
            {
                m_positions[client].PushArray(targetposition, 3);
                m_pathIndex[client].Push(destIndex);
                m_positions[client].PushArray(targetposition, 3);
                m_pathIndex[client].Push(destIndex);
            }

            // build the complete path
            while (currentIndex.index != -1)
            {
				AddToPath(currentIndex.index, client);
                currentIndex.index = astar[currentIndex.index].parent;
			}

            delete openList;
            return;
        }

        if (astar[currentIndex.index].state != OPEN)
            continue;

        // put current node into CLOSED list
        astar[currentIndex.index].state = CLOSED;

        // now expand the current node
        for (int i = 0; i < MaxPathIndex; i++)
        {
            int self = m_paths[currentIndex.index].pathIndex[i];
            if (self == -1)
                continue;
            
            if (m_paths[self].flags == WAYPOINT_DOUBLEJUMP && TF2_GetPlayerClass(client) != TFClass_Scout)
                continue;

            if (m_paths[self].flags == WAYPOINT_ROCKETJUMP && (TF2_GetPlayerClass(client) != TFClass_Soldier || m_lowHealth[client]))
                continue;

            float f = GetWaypointDistance(self, destIndex);
            if (astar[self].state == NEW || astar[self].f > f)
            {
                // put the current child into open list
                astar[self].parent = currentIndex.index;
                astar[self].state = OPEN;
                astar[self].f = f;
                SearchNode next;
                next.index = self;
                next.cost = astar[self].f;
                openList.PushArray(next, 2);
            }
        }
    }

    // if still failing try to go random point
    delete openList;
    destIndex = GetRandomInt(1, m_waypointNumber);
    m_lastFailedWaypoint[client] = -1;
}

stock int FindCoverWaypoint(int client = -1, float maxDistance = 99999.0)
{
    maxDistance *= maxDistance;

    if (m_hidingSpots[client] != null && m_hidingSpots[client].Length > 0)
    {
        int index = -1;
        for (int spot = m_hidingSpots[client].Length - 1; spot > 0; spot--)
        {
            float normalDist = maxDistance;

            int hs = m_hidingSpots[client].Get(spot);
            float distance = GetVectorDistance(m_paths[hs].origin, GetOrigin(client), true) + (GetDamage(hs) * GetDamage(hs)) + (GetDeaths(hs) * GetDeaths(hs));
            if (distance < normalDist)
            {
                index = hs;
                normalDist = distance;
            }  
	    }

        if (index != -1)
            return index;
    }

    // do not move to a position near to the enemy
    /*float distance = GetVectorDistance(GetOrigin(client), m_lastEnemyOrigin[client], true);
    if (maxDistance > distance)
        maxDistance = distance;

    float minDist = 384.0 * 384.0;
    if (maxDistance < minDist)
        maxDistance = minDist;

    int srcIndex = m_currentIndex[client];
    int enemyIndex = FindNearestWaypoint(m_lastEnemyOrigin[client], 999999.0, m_lastEnemy[client]);
    if (enemyIndex == -1)
        return -1;
        
    float height = GetHeight(client);
    float enemyheight;
    
    if (IsValidClient(m_lastEnemy[client]))
        enemyheight = GetHeight(m_lastEnemy[client]) * 2;
    else
        enemyheight = height * 2;

    ArrayList enemyIndices = new ArrayList();

    int waypointIndex[MaxPathIndex];
    float minDistance[MaxPathIndex];

    for (int i = 0; i < MaxPathIndex; i++)
    {
        waypointIndex[i] = -1;
        minDistance[i] = maxDistance;
    }

    // now get enemies neigbouring points
    for (int i = 0; i < MaxPathIndex; i++)
    {
        if (m_paths[enemyIndex].pathIndex[i] != -1)
            enemyIndices.Push(m_paths[enemyIndex].pathIndex[i]);
    }

    // find the best waypoint now
    for (int i = 0; i < m_waypointNumber; i++)
    {
        // exclude current waypoints and waypoints seen by the enemy
        if (i == srcIndex || IsVisibleWaypointIndex(enemyIndex, i, height, enemyheight) || IsVisibleWaypointIndex(srcIndex, i, height, enemyheight))
            continue;

        bool neighbourVisible = false;  // now check neighbour waypoints for visibility

        for (int j = 0; j != enemyIndices.Length; j++)
        {
            if (IsVisibleWaypointIndex(enemyIndices.Get(j), i, enemyheight, height) || IsVisibleWaypointIndex(enemyIndices.Get(j), srcIndex, enemyheight, height))
            {
                neighbourVisible = true;
                break;
            }
        }

        // skip visible points
        if (neighbourVisible)
            continue;

        float distances = GetWaypointDistance(srcIndex, i);
        float enemyDistance = GetWaypointDistance(enemyIndex, i);

        if (distances >= enemyDistance)
            continue;

        for (int j = 0; j < MaxPathIndex; j++)
        {
            if (distances < minDistance[j])
            {
                waypointIndex[j] = i;
                minDistance[j] = distances;
                break;
            }
        }
    }

    // use statistic if we have them
    for (int i = 0; i < MaxPathIndex; i++) // not now
    {
        if (waypointIndex[i] != -1)
        {
            int deaths = GetDeaths(waypointIndex[i]);
            minDistance[i] += GetDamage(waypointIndex[i]) + (deaths * deaths);
        }
    }

    // take the first one which isn't spotted by the enemy and not in blacklist
    for (int i = 0; i < MaxPathIndex; i++)
    {
        if (waypointIndex[i] != -1 && m_lastFailedWaypoint[client] != waypointIndex[i])
        {
            float origin[3];
            origin = m_lastEnemyOrigin[client];
            origin[2] += enemyheight;
            float origin2[3];
            origin2 = m_paths[waypointIndex[i]].origin;
            origin2[2] += height;
            
            // expand the search
            for (int j = 0; j < MaxPathIndex; j++)
            {
                int expandedIndex = m_paths[waypointIndex[i]].pathIndex[j];
                if (expandedIndex != -1 && expandedIndex != waypointIndex[i] && m_lastFailedWaypoint[client] != expandedIndex)
                {
                    float origin3[3];
                    origin3 = m_lastEnemyOrigin[client];
                    origin3[2] += enemyheight;
                    float origin4[3];
                    origin4 = m_paths[expandedIndex].origin;
                    origin4[2] += height;
                    if (!IsVisible(origin3, origin4))
                        return expandedIndex;
                }
            }
        }
    }*/

    return -1;
}

stock bool IsVisibleByEnemy(int client, int waypointIndex, float height)
{
    if (waypointIndex == -1)
        return false;

    float origin[3];
    origin = m_paths[waypointIndex].origin;
    origin[2] += height;

    for (int i = 1; i <= MaxClients; i++)
    {
        if (!IsValidClient(i))
			continue;
		
		if (!IsPlayerAlive(i))
			continue;
		
		if (GetClientTeam(i) == GetClientTeam(client))
			continue;
        
        if (IsVisible(origin, GetEyePosition(i)))
            return true;
    }

    return false;
}

stock void AddToPath(int currentIndex, int client)
{
    m_pathIndex[client].Push(currentIndex);
    float radius = m_paths[currentIndex].radius;

    if (radius > 0.0)
    {
        float origin2[3];
        origin2[0] = m_paths[currentIndex].origin[0] + GetRandomFloat(-radius, radius);
        origin2[1] = m_paths[currentIndex].origin[1] + GetRandomFloat(-radius, radius);
        origin2[2] = m_paths[currentIndex].origin[2];
        m_positions[client].PushArray(origin2, 3);
    }
    else
        m_positions[client].PushArray(m_paths[currentIndex].origin, 3);

	for (int i = m_positions[client].Length - 1; i > 0; i--)
	{
		float flFromPos[3], flToPos[3];
		m_positions[client].GetArray(i, flFromPos, 3);
		m_positions[client].GetArray(i - 1, flToPos, 3);
		
		if (GetConVarInt(EBotDebug) == 1)
		{
			TE_SetupBeamPoints(flFromPos, flToPos, m_laserIndex, m_laserIndex, 0, 30, 1.0, 1.0, 1.0, 5, 0.0, {0, 255, 0, 255}, 30);
			TE_SendToClient(m_hostEntity);
		}
	}

	for (int i = m_pathIndex[client].Length - 1; i > 0; i--)
	{
		float flFromPos[3], flToPos[3];
		m_pathIndex[client].GetArray(i, flFromPos);
		m_pathIndex[client].GetArray(i - 1, flToPos);
		
		if (GetConVarInt(EBotDebug) == 1)
		{
			TE_SetupBeamPoints(flFromPos, flToPos, m_laserIndex, m_laserIndex, 0, 30, 1.0, 1.0, 1.0, 5, 0.0, {0, 255, 0, 255}, 30);
			TE_SendToClient(m_hostEntity);
		}
	}

	m_targetNode[client] = m_positions[client].Length - 2;
	m_currentWaypointIndex[client] = m_pathIndex[client].Length - 2;
}

// find the nearest node to that origin, and return the index to this node
stock int FindNearestWaypoint(float origin[3], float maxDistance = 999999.0, int client = -1)
{
    int index = -1;
    float normalDist = maxDistance * maxDistance;

    for (int i = 0; i < m_waypointNumber; i++)
    {
        // blocked waypoint
   		if (m_paths[i].activeArea != 0 && m_paths[i].activeArea != currentActiveArea)
			continue;

        if (client > 0 && client <= MaxClients)
        {
            if (m_paths[i].team == 2 && GetClientTeam(client) == 3)
                continue;
            else if (m_paths[i].team == 3 && GetClientTeam(client) == 2)
                continue;
        }
        
        float distance = GetVectorDistance(m_paths[i].origin, origin, true);
        if (distance < normalDist)
        {
            index = i;
            normalDist = distance;
        }
    }
    
    return index;
}

stock void DeletePathNodes(int client)
{
	if (m_positions[client] == null)
	{
		delete m_positions[client];
		m_positions[client] = new ArrayList(3);
	}

	if (m_positions[client] != null)
		m_positions[client].Clear();
	
	if (m_pathIndex[client] == null)
	{
		delete m_pathIndex[client];
		m_pathIndex[client] = new ArrayList();
	}

	if (m_pathIndex[client] != null)
		m_pathIndex[client].Clear();
}

stock void WaypointClear(int index)
{
    m_paths[index].index = index;
    m_paths[index].activeArea = 0;
    m_paths[index].flags = 0;
    m_paths[index].team = 0;
    m_paths[index].radius = 0.0;
    m_paths[index].origin = {0.0, 0.0, 0.0};
    m_paths[index].campStart = {0.0, 0.0, 0.0};
    m_paths[index].campEnd = {0.0, 0.0, 0.0};
    for (int i = 0; i < MaxPathIndex; i++)
    {
        m_paths[index].pathIndex[i] = -1;
        m_paths[index].distance[i] = 0.0;
    }
}

stock float GetWaypointDistance(int srcIndex, int destIndex)
{
    if (srcIndex == -1 || destIndex == -1)
        return 99999.0;

    if (srcIndex == destIndex)
        return 0.0;

    for (int i = 0; i < MaxPathIndex; i++)
    {
        if (m_paths[srcIndex].pathIndex[i] == destIndex)
            return m_paths[srcIndex].distance[i];
    }

    return GetFastDistance(m_paths[srcIndex].origin, m_paths[destIndex].origin);
}

stock bool IsWaypointUsed(int index)
{
    for (int i = 0; i < MaxClients; i++)
    {
        if (!IsValidClient(i))
            continue;
        
        if (!IsPlayerAlive(i))
            continue;

        if (m_goalIndex[i] == index || GetVectorDistance(GetOrigin(i), m_paths[index].origin, true) <= Squared(64))
            return true;
    }

    return false;
}

stock void SetGoalIndex(int client, int index)
{
    if (index != -1)
        m_goalIndex[client] = index;
}

stock void InitTypes()
{
    m_hasHealthWaypoints = false;
    m_hasAmmoWaypoints = false;
    m_hasSniperWaypoints = false;
    m_hasStickyWaypoints = false;
    m_hasRouteWaypoints = false;
    m_hasSentryWaypoints = false;
    m_hasTeleporterEnterWaypoints = false;
    m_hasTeleporterExitWaypoints = false;
    m_hasRocketJumpWaypoints = false;

    for (int i = 0; i < m_waypointNumber; i++)
    {
        if (m_paths[i].flags == WAYPOINT_RESUPPLY)
        {
            m_hasHealthWaypoints = true;
            m_hasAmmoWaypoints = true;
        }
        else if (m_paths[i].flags == WAYPOINT_HEALTH)
            m_hasHealthWaypoints = true;
        else if (m_paths[i].flags == WAYPOINT_AMMO)
            m_hasAmmoWaypoints = true;
        else if (m_paths[i].flags == WAYPOINT_SNIPER)
            m_hasSniperWaypoints = true;
        else if (m_paths[i].flags == WAYPOINT_DEMOMANCAMP)
            m_hasStickyWaypoints = true;
        else if (m_paths[i].flags == WAYPOINT_ROUTE)
            m_hasRouteWaypoints = true;
        else if (m_paths[i].flags == WAYPOINT_SENTRY)
            m_hasSentryWaypoints = true;
        else if (m_paths[i].flags == WAYPOINT_TELEPORTERENTER)
            m_hasTeleporterEnterWaypoints = true;
        else if (m_paths[i].flags == WAYPOINT_TELEPORTEREXIT)
            m_hasTeleporterExitWaypoints = true;
        else if (m_paths[i].flags == WAYPOINT_ROCKETJUMP)
            m_hasRocketJumpWaypoints = true;
    }
}

stock void WaypointLoad()
{
    char filepath[PLATFORM_MAX_PATH];
    BuildPath(Path_SM, filepath, sizeof(filepath), "ebot/waypoints/%s.ewp", currentMap);
    File fp = OpenFile(filepath, "rb");

    CreateTimer(15.0, PrintHudTextOnStart);

    if (fp == null)
    {
        if (NavMesh_Exists())
            m_aboutTheWaypoint = "No waypoint file found\nNavmesh file will be used";
        else
            m_aboutTheWaypoint = "No waypoint file found\nNo navmesh file found\nBots will not move";
        return;
    }

    // read about the file and author
    WaypointHeader header;
    fp.Read(header, sizeof(header), 4);
    m_waypointNumber = header.pointNumber;

    if (m_waypointNumber <= 0)
    {
        m_aboutTheWaypoint = "Broken waypoint file\nThis waypoint file will be ignored";
        fp.Close();
        return;
    }

    else if (header.fileVersion > WaypointVersion)
    {
        Format(m_aboutTheWaypoint, sizeof(m_aboutTheWaypoint), "Waypoint version is too high\nPlease update your ebot!\nThis waypoint file will be ignored %s", header.fileVersion);
        fp.Close();
        return;
    }

    Format(m_aboutTheWaypoint, sizeof(m_aboutTheWaypoint), "Waypoint file by %s\n%d waypoints loaded", header.author, m_waypointNumber);

    if (header.fileVersion == 1)
    {
        PathOLD m_pathsOLD[MaxWaypoints];
        for (int i = 0; i < m_waypointNumber; i++)
            fp.Read(m_pathsOLD[i], sizeof(PathOLD), 4);

        for (int i = 0; i < m_waypointNumber; i++)
        {
            m_paths[i].index = m_pathsOLD[i].index;
            m_paths[i].pathIndex = m_pathsOLD[i].pathIndex;
            m_paths[i].activeArea = m_pathsOLD[i].activeArea;
            m_paths[i].team = m_pathsOLD[i].team;
            m_paths[i].flags = m_pathsOLD[i].flags;
            m_paths[i].origin = VectorAsFloat(m_pathsOLD[i].origin);
            m_paths[i].campStart = VectorAsFloat(m_pathsOLD[i].campStart);
            m_paths[i].campEnd = VectorAsFloat(m_pathsOLD[i].campEnd);
            m_paths[i].radius = float(m_pathsOLD[i].radius);
            m_paths[i].distance[0] = float(m_pathsOLD[i].distance[0]);
            m_paths[i].distance[1] = float(m_pathsOLD[i].distance[1]);
            m_paths[i].distance[2] = float(m_pathsOLD[i].distance[2]);
            m_paths[i].distance[3] = float(m_pathsOLD[i].distance[3]);
            m_paths[i].distance[4] = float(m_pathsOLD[i].distance[4]);
            m_paths[i].distance[5] = float(m_pathsOLD[i].distance[5]);
            m_paths[i].distance[6] = float(m_pathsOLD[i].distance[6]);
            m_paths[i].distance[7] = float(m_pathsOLD[i].distance[7]);
        }
    }
    else
    {
        // read the all waypoint data
        for (int i = 0; i < m_waypointNumber; i++)
            fp.Read(m_paths[i], sizeof(Path), 4);
    }
    
    fp.Close();
    m_hasWaypoints = true;
    InitTypes();
    return;
}

stock void WaypointSave()
{
    if (m_waypointNumber <= 0)
    {
        PrintHintTextToAll("No waypoints to save");
        return;
    }

    char waypointAuthor[32] = "unknown";
    if (IsValidClient(m_hostEntity))
        GetClientName(m_hostEntity, waypointAuthor, sizeof(waypointAuthor));

    WaypointHeader header;
    header.author = waypointAuthor;

    char filepath[PLATFORM_MAX_PATH];
    BuildPath(Path_SM, filepath, sizeof(filepath), "ebot/waypoints/%s.ewp", currentMap);
    
    // remember the original waypoint author
    File rf = OpenFile(filepath, "rb");
    if (rf != null)
    {
        rf.Read(header, sizeof(header), 4);
        rf.Close();
    }
    
    header.fileVersion = WaypointVersion;
    header.pointNumber = m_waypointNumber;
    
    File fp = OpenFile(filepath, "wb");

    // write the all waypoint data
    fp.Write(header, sizeof(header), 4);

    // save the waypoint paths...
    for (int i = 0; i < m_waypointNumber; i++)
        fp.Write(m_paths[i], sizeof(Path), 4);
    
    fp.Close();
    PrintHintTextToAll("%d waypoints saved sucsessfully!", m_waypointNumber);
}

/*stock void Analyze()
{
    if (m_analyzeWaypoints == false)
        return;
    if (m_waypointNumber > 0)
    {
        for (int i = 0; i < m_waypointNumber; i++)
        {
            if (i != -1)
            {
                Vector WayVec = g_waypoint->GetPath(i)->origin;
                Vector Start;
                float ran = sypbm_analyze_distance.GetFloat();
                Start.x = (WayVec.x + engine->RandomFloat(((-ran) - 5.0f), (ran + 5.0f)));
                Start.y = (WayVec.y + engine->RandomFloat(((-ran) - 5.0f), (ran + 5.0f)));
                Start.z = (WayVec.z + engine->RandomFloat(1, ran));
                TraceResult tr;
                TraceResult tr2;
                TraceLine(WayVec, Start, true, g_hostEntity, &tr);
                TraceLine(tr.vecEndPos, Vector(tr.vecEndPos.x, tr.vecEndPos.y, -9999.0f), true, false, g_hostEntity, &tr2);
                float lastwaypointaddtime = 0.0f;
                if (tr.flFraction == 1.0f)
                {
                    int startindex = g_waypoint->FindNearest(tr.vecEndPos, ran);
                    if (!IsValidWaypoint(startindex))
                    {
                        if (tr2.flFraction != 1.0f)
                        {
                            int endindex = g_waypoint->FindNearest(tr2.vecEndPos, ran);
                            if (!IsValidWaypoint(endindex))
                            {
                                TraceResult tr3;
                                TraceResult tr4;
                                TraceResult tr5;
                                TraceResult tr6;
                                Vector TargetPosition = Vector(tr2.vecEndPos.x, tr2.vecEndPos.y, (tr2.vecEndPos.z + 36.0f));
                                TraceLine(TargetPosition, (TargetPosition + (g_pGlobals->v_forward * sypbm_analyze_wall_check_distance.GetFloat())), true, false, g_hostEntity, &tr3); // forward
                                TraceLine(TargetPosition, (TargetPosition - (g_pGlobals->v_forward * sypbm_analyze_wall_check_distance.GetFloat())), true, false, g_hostEntity, &tr4); // backward
                                TraceLine(TargetPosition, (TargetPosition + (g_pGlobals->v_right * sypbm_analyze_wall_check_distance.GetFloat())), true, false, g_hostEntity, &tr5); // right
                                TraceLine(TargetPosition, (TargetPosition - (g_pGlobals->v_right * sypbm_analyze_wall_check_distance.GetFloat())), true, false, g_hostEntity, &tr6); // left
                                // if we dont hit wall, put waypoint here
                                if (tr3.flFraction == 1.0f && tr4.flFraction == 1.0f && tr5.flFraction == 1.0f && tr6.flFraction == 1.0f)
                                {
                                    edict_t* ent = null;
                                    while (!FNullEnt(ent = FIND_ENTITY_BY_CLASSNAME(ent, "hostage_entity")))
                                    {
                                        // if already saved || moving skip it
                                        if ((ent->v.effects & EF_NODRAW) && (ent->v.speed > 0))
                                            continue;
                                        TraceResult vis;
                                        TraceLine(TargetPosition, GetEntityOrigin(ent), true, false, g_hostEntity, &vis);
                                        if (vis.flFraction == 1.0f && GetDistanceSquared(TargetPosition, GetEntityOrigin(ent)) <= Squared(sypbm_analyze_goal_check_distance.GetFloat()))
                                        {
                                            lastwaypointaddtime = engine->GetTime();
                                            Add(100, TargetPosition);
                                        }
                                    }
                                    while (!FNullEnt(ent = FIND_ENTITY_BY_CLASSNAME(ent, "func_bomb_target")))
                                    {
                                        TraceResult vis;
                                        TraceLine(TargetPosition, GetEntityOrigin(ent), true, false, g_hostEntity, &vis);
                                        if (vis.flFraction == 1.0f && GetDistanceSquared(TargetPosition, GetEntityOrigin(ent)) <= Squared(sypbm_analyze_goal_check_distance.GetFloat()))
                                        {
                                            lastwaypointaddtime = engine->GetTime();
                                            Add(100, TargetPosition);
                                        }
                                    }
                                    while (!FNullEnt(ent = FIND_ENTITY_BY_CLASSNAME(ent, "info_bomb_target")))
                                    {
                                        TraceResult vis;
                                        TraceLine(TargetPosition, GetEntityOrigin(ent), true, false, g_hostEntity, &vis);
                                        if (vis.flFraction == 1.0f && GetDistanceSquared(TargetPosition, GetEntityOrigin(ent)) <= Squared(sypbm_analyze_goal_check_distance.GetFloat()))
                                        {
                                            lastwaypointaddtime = engine->GetTime();
                                            Add(100, TargetPosition);
                                        }
                                    }
                                    int doublecheckindex = g_waypoint->FindNearest(TargetPosition, ran);
                                    if (!IsValidWaypoint(doublecheckindex))
                                    {
                                        TraceResult trf;
                                        TraceResult trb;
                                        TraceResult trr;
                                        TraceResult trl;
                                        TraceLine(TargetPosition, (TargetPosition + (g_pGlobals->v_forward * sypbm_analyze_wall_check_distance.GetFloat())), true, false, g_hostEntity, &trf); // forward
                                        TraceLine(TargetPosition, (TargetPosition - (g_pGlobals->v_forward * sypbm_analyze_wall_check_distance.GetFloat())), true, false, g_hostEntity, &trb); // backward
                                        TraceLine(TargetPosition, (TargetPosition + (g_pGlobals->v_right * sypbm_analyze_wall_check_distance.GetFloat())), true, false, g_hostEntity, &trr); // right
                                        TraceLine(TargetPosition, (TargetPosition - (g_pGlobals->v_right * sypbm_analyze_wall_check_distance.GetFloat())), true, false, g_hostEntity, &trl); // left
                                        // double check
                                        if (trf.flFraction == 1.0f && trb.flFraction == 1.0f && trr.flFraction == 1.0f && trl.flFraction == 1.0f)
                                        {
                                            g_analyzeputrequirescrouch = false;
                                            TraceResult upcheck;
                                            Vector TargetPosition2 = Vector(tr2.vecEndPos.x, tr2.vecEndPos.y, (tr2.vecEndPos.z + 69.0f));
                                            TraceLine(TargetPosition, TargetPosition2, true, false, g_hostEntity, &upcheck);
                                            if (upcheck.flFraction != 1.0f)
                                                g_analyzeputrequirescrouch = true;
                                            lastwaypointaddtime = engine->GetTime();
                                            Add(-1, g_analyzeputrequirescrouch ? Vector(tr2.vecEndPos.x, tr2.vecEndPos.y, (tr2.vecEndPos.z + 18.0f)) : TargetPosition);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}*/
