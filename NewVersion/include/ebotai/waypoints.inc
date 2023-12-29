// from https://github.com/EfeDursun125/CS-EBOT/blob/main/source/waypoint.cpp
// higher version waypoints will be ignored due to updated waypoint system
const int WaypointVersion = 3;
const int MaxWaypoints = 2048;
const int MaxPathIndex = 8;

int nearestIndex = -1;
int savedIndex = -1;
bool showWaypoints = false;
float waypointDrawTime = 0.0;
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
bool m_hasCaptureWaypoints = false;

int m_lastFailedWaypoint[TFMaxPlayers];
char m_aboutTheWaypoint[192] = "";

enum
{
    WAYPOINT_JUMP = (1 << 1),
    WAYPOINT_SNIPER = (1 << 2),
    WAYPOINT_DEFEND = (1 << 3),
    WAYPOINT_FALLDAMAGE = (1 << 4),
    WAYPOINT_CROUCH = (1 << 5),
    WAYPOINT_AMMO = (1 << 6),
    WAYPOINT_HEALTH = (1 << 7),
    WAYPOINT_RESUPPLY = (1 << 8),
    WAYPOINT_DOUBLEJUMP = (1 << 9),
    WAYPOINT_ROCKETJUMP = (1 << 10),
    WAYPOINT_DEMOMANCAMP = (1 << 11),
    WAYPOINT_SENTRY = (1 << 12),
    WAYPOINT_ROUTE = (1 << 13),
    WAYPOINT_TELEPORTERENTER = (1 << 14),
    WAYPOINT_TELEPORTEREXIT = (1 << 15),
    WAYPOINT_CAPTUREPOINT = (1 << 16),
    WAYPOINT_NOSTRAFE = (1 << 17),
    WAYPOINT_NOSPY = (1 << 18),
    WAYPOINT_DEMOCHARGE = (1 << 19),
    WAYPOINT_DONTHIDE = (1 << 20)
};

stock char[] GetWaypointName(int waypoint)
{
	char string[512] = "None";
    FormatEx(string, sizeof(string), "%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s", waypoint == 0 ? "None" : "", 
        waypoint & WAYPOINT_JUMP ? "Jump " : "", 
        waypoint & WAYPOINT_SNIPER ? "Sniper " : "", 
        waypoint & WAYPOINT_DEFEND ? "Defend " : "", 
        waypoint & WAYPOINT_FALLDAMAGE ? "FallDamage " : "", 
        waypoint & WAYPOINT_CROUCH ? "Crouch " : "", 
        waypoint & WAYPOINT_AMMO ? "Ammo " : "", 
        waypoint & WAYPOINT_HEALTH ? "Health " : "", 
        waypoint & WAYPOINT_RESUPPLY ? "Resupply " : "", 
        waypoint & WAYPOINT_DOUBLEJUMP ? "DoubleJump " : "", 
        waypoint & WAYPOINT_ROCKETJUMP ? "RocketJump " : "", 
        waypoint & WAYPOINT_DEMOMANCAMP ? "StickyTrap " : "", 
        waypoint & WAYPOINT_SENTRY ? "Sentry " : "", 
        waypoint & WAYPOINT_ROUTE ? "Route " : "", 
        waypoint & WAYPOINT_TELEPORTERENTER ? "TeleEnter " : "", 
        waypoint & WAYPOINT_TELEPORTEREXIT ? "TeleExit " : "", 
        waypoint & WAYPOINT_CAPTUREPOINT ? "CapturePoint " : "", 
        waypoint & WAYPOINT_NOSTRAFE ? "NoStrafe " : "", 
        waypoint & WAYPOINT_NOSPY ? "NoSpy " : "", 
        waypoint & WAYPOINT_DONTHIDE ? "DontHide " : "", 
        waypoint & WAYPOINT_DEMOCHARGE ? "DemoCharge " : "");
	return string;
}

enum
{
    AREA1 = (1 << 1),
    AREA2 = (1 << 2),
    AREA3 = (1 << 3),
    AREA4 = (1 << 4),
    AREA5 = (1 << 5),
    AREA6 = (1 << 6),
    AREA7 = (1 << 7),
    AREA8 = (1 << 8),
    AREA9 = (1 << 9)
};

stock char[] GetAreaName(const int area)
{
	char string[256] = "All Time";
    FormatEx(string, sizeof(string), "%s%s%s%s%s%s%s%s%s%s", area == 0 ? "All Time" : "", 
        area & AREA1 ? "Area1 " : "", 
        area & AREA2 ? "Area2 " : "", 
        area & AREA3 ? "Area3 " : "", 
        area & AREA4 ? "Area4 " : "", 
        area & AREA5 ? "Area5 " : "", 
        area & AREA6 ? "Area6 " : "", 
        area & AREA7 ? "Area7 " : "", 
        area & AREA8 ? "Area8 " : "", 
        area & AREA9 ? "Area9 " : "");
	return string;
}

enum struct AStar
{
    float g;
    float f;
    int parent;
    bool is_closed;
}

enum struct HeapNode
{
    int id;
    float priority;
}

AStar astar[MaxWaypoints];
HeapNode heap[MaxWaypoints];

enum struct PriorityQueue
{
    int size;
    void NewQueue()
    {
        this.size = 0;
    }

    bool IsEmpty()
    {
        return !this.size;
    }

    int Size()
    {
        return this.size;
    }

    void InsertLowest(const int value, const float priority)
    {
        heap[this.size].priority = priority;
        heap[this.size].id = value;
        this.size++;

        static HeapNode temp;
        static int child;
        static int parent;

        child = this.size - 1;
        while (child)
        {
            parent = (child - 1) / 2;
            if (heap[parent].priority < heap[child].priority)
                break;

            temp = heap[child];
            heap[child] = heap[parent];
            heap[parent] = temp;
            child = parent;
        }
    }

    void InsertHighest(const int value, const float priority)
    {
        heap[this.size].priority = priority;
        heap[this.size].id = value;
        this.size++;

        static HeapNode temp;
        static int child;
        static int parent;

        child = this.size - 1;
        while (child)
        {
            parent = (child - 1) / 2;
            if (heap[parent].priority > heap[child].priority)
                break;

            temp = heap[child];
            heap[child] = heap[parent];
            heap[parent] = temp;
            child = parent;
        }
    }

    int RemoveLowest()
    {
        static int retID;
        retID = heap[0].id;
        this.size--;
        heap[0] = heap[this.size];

        static int parent;
        static int child;
        static int rightChild;
        parent = 0;
        child = (2 * parent) + 1;

        static HeapNode ref;
        ref = heap[parent];
        while (child < this.size)
        {
            rightChild = (2 * parent) + 2;
            if (rightChild < this.size)
            {
                if (heap[rightChild].priority < heap[child].priority)
                    child = rightChild;
            }

            if (ref.priority < heap[child].priority)
                break;

            heap[parent] = heap[child];
            parent = child;
            child = (2 * parent) + 1;
        }

        heap[parent] = ref;
        return retID;
    }

    int RemoveHighest()
    {
        static int retID;
        retID = heap[0].id;
        this.size--;
        heap[0] = heap[this.size];

        static int parent;
        static int child;
        static int rightChild;
        parent = 0;
        child = (2 * parent) + 1;

        static HeapNode ref;
        ref = heap[parent];
        while (child < this.size)
        {
            rightChild = (2 * parent) + 2;
            if (rightChild < this.size)
            {
                if (heap[rightChild].priority < heap[child].priority)
                    child = rightChild;
            }

            if (ref.priority > heap[child].priority)
                break;

            heap[parent] = heap[child];
            parent = child;
            child = (2 * parent) + 1;
        }

        heap[parent] = ref;
        return retID;
    }
}

enum struct WaypointHeader
{
    char author[32];
	int fileVersion;
	int pointNumber;
}

enum struct WPath
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

bool m_hasWaypoints;
bool m_hasNavpoints;

int m_waypointNumber;
WPath m_paths[MaxWaypoints];

stock void InitializeWaypoints()
{
    int i;
    for (i = 0; i < m_waypointNumber; i++)
        WaypointClear(i);

    waypointDrawTime = 0.0;
    nextDrawTime = 0.5;
    m_hasWaypoints = false;
    m_hasNavpoints = false;
    m_waypointNumber = 0;

    WaypointLoad();
    ResetDangerMap();
    DangerMapLoad();
}

float m_maxDamageRed[TFMaxPlayers];
float m_maxDamageBlu[TFMaxPlayers];
float damageGLOBALred[7][MaxWaypoints];
float damageGLOBALblu[7][MaxWaypoints];
float damageWPTred[7][TFMaxPlayers][MaxWaypoints];
float damageWPTblu[7][TFMaxPlayers][MaxWaypoints];

stock void ResetDangerMap(const bool global = false)
{
    int C, i, j;
    for (j = 0; j < 7; j++)
    {
        for (i = 0; i < m_waypointNumber; i++)
        {
            if (global)
            {
                damageGLOBALred[j][i] = 0.0;
                damageGLOBALblu[j][i] = 0.0;
            }

            for (C = 0; C < TFMaxPlayers; C++)
            {
                damageWPTred[j][C][i] = 0.0;
                damageWPTblu[j][C][i] = 0.0;
            }
        }
    }
}

stock float GetDamageTotalRED(const int waypoint)
{
    return damageGLOBALred[currentActiveAreaInt][waypoint];
}

stock float GetDamageTotalBLU(const int waypoint)
{
    return damageGLOBALblu[currentActiveAreaInt][waypoint];
}

stock void IncreaseDamage(const int client, const int waypoint, const float newDamage)
{
    int i;
    float half;
    if (m_team[client] == 2)
    {
        damageWPTred[currentActiveAreaInt][client][waypoint] += newDamage;
        damageGLOBALred[currentActiveAreaInt][waypoint] += newDamage;
        half = newDamage * 0.2;
        for (i = 0; i < MaxPathIndex; i++)
        {
            if (m_paths[waypoint].pathIndex[i] == -1)
                continue;
            
            if ((m_paths[m_paths[waypoint].pathIndex[i]].radius + half) > newDamage)
            {
                damageWPTred[currentActiveAreaInt][client][m_paths[waypoint].pathIndex[i]] += newDamage;
                damageGLOBALred[currentActiveAreaInt][m_paths[waypoint].pathIndex[i]] += newDamage;
            }
            else
            {
                damageWPTred[currentActiveAreaInt][client][m_paths[waypoint].pathIndex[i]] += half;
                damageGLOBALred[currentActiveAreaInt][m_paths[waypoint].pathIndex[i]] += half;
            }
        }
    }
    else
    {
        damageWPTblu[currentActiveAreaInt][client][waypoint] += newDamage;
        damageGLOBALblu[currentActiveAreaInt][waypoint] += newDamage;
        half = newDamage * 0.2;
        for (i = 0; i < MaxPathIndex; i++)
        {
            if (m_paths[waypoint].pathIndex[i] == -1)
                continue;
            
            if ((m_paths[m_paths[waypoint].pathIndex[i]].radius + half) > newDamage)
            {
                damageWPTblu[currentActiveAreaInt][client][m_paths[waypoint].pathIndex[i]] += newDamage;
                damageGLOBALblu[currentActiveAreaInt][m_paths[waypoint].pathIndex[i]] += newDamage;
            }
            else
            {
                damageWPTblu[currentActiveAreaInt][client][m_paths[waypoint].pathIndex[i]] += half;
                damageGLOBALblu[currentActiveAreaInt][m_paths[waypoint].pathIndex[i]] += half;
            }
        }
    }
}

stock float GetDamage(const int client, const int waypoint)
{
    if (m_lastEnemySeen[client] + 30.0 < GetGameTime())
        return 0.0;

    if (m_team[client] == 2)
        return damageWPTred[currentActiveAreaInt][client][waypoint];
    
    return damageWPTblu[currentActiveAreaInt][client][waypoint];
}

stock float GetMaxDamage(const int client)
{
    if (m_team[client] == 2)
        return m_maxDamageRed[client];
    
    return m_maxDamageBlu[client];
}

stock void DrawWaypoints()
{
    if (!showWaypoints)
        return;
    
    if (!IsValidClient(m_hostEntity))
    {
        FindHostEntity();
        return;
    }

    if (waypointDrawTime > GetGameTime())
        return;
    
    float frame = 0.05;
    
    m_lastFailedWaypoint[m_hostEntity] = -1;

    // reset the minimal distance changed before
    float nearestDistance = 999999999.0;
    float range = 768.0;

    if (m_waypointNumber > 1024)
        range = 512.0;
    
    // draw a paths, camplines and danger directions for nearest waypoint
    if (nearestIndex != -1 && GetVectorDistance(GetEyePosition(m_hostEntity), m_paths[nearestIndex].origin, true) < Squaredf(range) && IsVisibleWaypointDebug(GetEyePosition(m_hostEntity), m_paths[nearestIndex].origin))
    {
        // draw the camplines
        if (m_paths[nearestIndex].flags & WAYPOINT_SNIPER || m_paths[nearestIndex].flags & WAYPOINT_DEFEND || m_paths[nearestIndex].flags & WAYPOINT_SENTRY || m_paths[nearestIndex].flags & WAYPOINT_DEMOMANCAMP || m_paths[nearestIndex].flags & WAYPOINT_TELEPORTERENTER || m_paths[nearestIndex].flags & WAYPOINT_TELEPORTEREXIT)
        {
            float height = 72.0;
            if (m_paths[nearestIndex].flags & WAYPOINT_CROUCH)
                height = 36.0;

            // convert int to float
            float indexOrigin[3];
            indexOrigin = m_paths[nearestIndex].origin;
            indexOrigin[2] += height;

            TE_SetupBeamPoints(indexOrigin, m_paths[nearestIndex].campStart, m_laserIndex, m_laserIndex, 0, 30, nextDrawTime + frame, 1.0, 1.0, 5, 0.0, {255, 0, 0, 255}, 20);
            TE_SendToClient(m_hostEntity);
            TE_SetupBeamPoints(indexOrigin, m_paths[nearestIndex].campEnd, m_laserIndex, m_laserIndex, 0, 30, nextDrawTime + frame, 1.0, 1.0, 5, 0.0, {255, 0, 0, 255}, 20);
            TE_SendToClient(m_hostEntity);
        }

        // draw the connections
        for (int i = 0; i < MaxPathIndex; i++)
        {
            if (m_paths[nearestIndex].pathIndex[i] == -1)
                continue;

            float height = 36.0;
            if (m_paths[nearestIndex].flags & WAYPOINT_CROUCH)
                height = 18.0;
            
            float height2 = 36.0;
            if (m_paths[m_paths[nearestIndex].pathIndex[i]].flags & WAYPOINT_CROUCH)
                height2 = 18.0;
            
            float indexOrigin[3];
            indexOrigin = m_paths[nearestIndex].origin;
            indexOrigin[2] = m_paths[nearestIndex].origin[2] + height;

            float indexOrigin2[3];
            indexOrigin2 = m_paths[m_paths[nearestIndex].pathIndex[i]].origin;
            indexOrigin2[2] = m_paths[m_paths[nearestIndex].pathIndex[i]].origin[2] + height2;

            if (IsConnected(m_paths[nearestIndex].pathIndex[i], nearestIndex)) // two way connection
            {
                TE_SetupBeamPoints(indexOrigin, indexOrigin2, m_laserIndex, m_laserIndex, 0, 30, nextDrawTime + frame, 1.0, 1.0, 5, 0.0, {255, 255, 0, 255}, 20);
                TE_SendToClient(m_hostEntity);
            }
            else // one way connection
            {
                TE_SetupBeamPoints(indexOrigin, indexOrigin2, m_laserIndex, m_laserIndex, 0, 30, nextDrawTime + frame, 1.0, 1.0, 5, 0.0, {255, 255, 255, 255}, 20);
                TE_SendToClient(m_hostEntity);
            }
        }

        // now look for oneway incoming connections
        for (int i = 0; i < m_waypointNumber; i++)
        {
            if (IsConnected(m_paths[i].index, m_paths[nearestIndex].index) && !IsConnected(m_paths[nearestIndex].index, m_paths[i].index))
            {
                float height = 36.0;
                if (m_paths[nearestIndex].flags & WAYPOINT_CROUCH)
                    height = 18.0;
                
                float height2 = 36.0;
                if (m_paths[i].flags & WAYPOINT_CROUCH)
                    height2 = 18.0;

                float indexOrigin[3];
                indexOrigin = m_paths[nearestIndex].origin;
                indexOrigin[2] = m_paths[nearestIndex].origin[2] + height;

                float indexOrigin2[3];
                indexOrigin2 = m_paths[i].origin;
                indexOrigin2[2] = m_paths[i].origin[2] + height2;

                TE_SetupBeamPoints(indexOrigin, indexOrigin2, m_laserIndex, m_laserIndex, 0, 30, nextDrawTime + frame, 1.0, 1.0, 5, 0.0, {0, 192, 96, 255}, 20);
                TE_SendToClient(m_hostEntity);
            }
        }

        // if radius is nonzero, draw a square
        int color[4] = {0, 0, 255, 255};
        float origin[3];
        origin = m_paths[nearestIndex].origin;
        float height = 18.0;

        if (m_paths[nearestIndex].flags & WAYPOINT_CROUCH)
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
        
        SetHudTextParams(0.0, -1.0, nextDrawTime + GetGameFrameTime(), 255, 255, 255, 255, 2, 1.0, 0.0, 0.0);
        ShowHudText(m_hostEntity, -1, "Index: %d (%d/%d)\nTeam: %s\nAreas: %s\nFlags: %s\nRadius: %d\nOrigin: %d %d %d\nAim Start: %d %d %d\nAim End: %d %d %d\nTotal DMG: RED - %d | BLU - %d", m_paths[i].index, m_waypointNumber, MaxWaypoints, wpteam, GetAreaName(m_paths[i].activeArea), GetWaypointName(m_paths[i].flags), RoundFloat(m_paths[i].radius), RoundFloat(m_paths[i].origin[0]), RoundFloat(m_paths[i].origin[1]), RoundFloat(m_paths[i].origin[2]), RoundFloat(m_paths[i].campStart[0]), RoundFloat(m_paths[i].campStart[1]), RoundFloat(m_paths[i].campStart[2]), RoundFloat(m_paths[i].campEnd[0]), RoundFloat(m_paths[i].campEnd[1]), RoundFloat(m_paths[i].campEnd[2]), RoundFloat(GetDamageTotalRED(m_paths[i].index)), RoundFloat(GetDamageTotalBLU(m_paths[i].index)));
    }

    float distance;
    int i;
    for (i = 0; i < m_waypointNumber; i++)
    {
        distance = GetVectorDistance(m_paths[i].origin, GetOrigin(m_hostEntity));
        if (distance > range)
            continue;
        
        if (distance > 64.0 && !ClientViewsOrigin(m_hostEntity, m_paths[i].origin, 0.6))
            continue;

        if (!IsVisibleToWaypoint(GetEyePosition(m_hostEntity), i, GetHeight(m_hostEntity)))
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

        if (m_paths[i].flags & WAYPOINT_CROUCH)
            height = 36.0;
        
        if (m_paths[i].flags & WAYPOINT_RESUPPLY)
        {
            color[0] = 188;
            color[1] = 52;
            color[2] = 123;
        }
        else if (m_paths[i].flags & WAYPOINT_SNIPER)
            color[2] = 255;
        else if (m_paths[i].flags & WAYPOINT_DEFEND)
        {
            color[0] = 255;
            color[1] = 125;
        }
        else if (m_paths[i].flags & WAYPOINT_FALLDAMAGE)
        {
            color[0] = 128;
            color[1] = 128;
            color[2] = 128;
        }
        else if (m_paths[i].flags & WAYPOINT_JUMP)
        {
            color[0] = 255;
            color[2] = 255;
        }
        else if (m_paths[i].flags & WAYPOINT_HEALTH)
        {
            color[0] = 255;
            color[2] = 255;
        }
        else if (m_paths[i].flags & WAYPOINT_AMMO)
        {
            color[0] = 55;
            color[1] = 64;
            color[2] = 68;
        }
        else if (m_paths[i].flags & WAYPOINT_CAPTUREPOINT)
            color[0] = 255;
        else if (m_paths[i].flags & WAYPOINT_DOUBLEJUMP)
        {
            color[0] = 160;
            color[1] = 240;
            color[2] = 20;
        }
        else if (m_paths[i].flags & WAYPOINT_NOSTRAFE)
        {
            color[0] = 147;
            color[1] = 128;
            color[2] = 147;
        }
        else if (m_paths[i].flags & WAYPOINT_ROCKETJUMP)
        {
            color[0] = 128;
            color[1] = 6;
            color[2] = 150;
        }
        else if (m_paths[i].flags & WAYPOINT_NOSPY)
        {
            color[0] = 160;
            color[1] = 124;
            color[2] = 122;
        }
        else if (m_paths[i].flags & WAYPOINT_DEMOCHARGE)
        {
            color[0] = 139;
            color[1] = 189;
            color[2] = 204;
        }
        else if (m_paths[i].flags & WAYPOINT_ROUTE)
        {
            color[0] = 70;
            color[1] = 70;
            color[2] = 70;
        }
        else if (m_paths[i].flags & WAYPOINT_SENTRY)
        {
            color[0] = 163;
            color[1] = 155;
            color[2] = 198;
        }
        else if (m_paths[i].flags & WAYPOINT_TELEPORTERENTER)
        {
            color[0] = 127;
            color[1] = 131;
            color[2] = 168;
        }
        else if (m_paths[i].flags & WAYPOINT_TELEPORTEREXIT)
        {
            color[0] = 131;
            color[1] = 127;
            color[2] = 168;
        }
        else if (m_paths[i].flags & WAYPOINT_DEMOMANCAMP)
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
        else if (m_paths[i].flags & WAYPOINT_DONTHIDE)
        {
            color[0] = 160;
            color[1] = 124;
            color[2] = 122;
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

    waypointDrawTime = GetGameTime() + nextDrawTime;
}

stock bool IsVisibleWaypointIndex(int start, int end, float startheight = 18.0, float endheight = 18.0)
{
	float fstart[3];
	float fend[3];

	fstart = m_paths[start].origin;
	fstart[2] += startheight;

	fend = m_paths[end].origin;
	fend[2] += endheight;

	Handle trace = TR_TraceRayFilterEx(fstart, fend, MASK_PLAYERSOLID, RayType_EndPoint, TraceEntityFilterStuff);
	float flFraction = TR_GetFraction(trace);
	delete trace;

	if (flFraction >= 1.0) 
		return !(flFraction == 0.0);
    
	return false;
}

stock bool IsVisibleToWaypoint(float origin[3], int waypoint, float height = 64.0)
{
	float end[3];
	end = m_paths[waypoint].origin;
	end[2] += height;

	Handle trace = TR_TraceRayFilterEx(origin, end, MASK_PLAYERSOLID, RayType_EndPoint, TraceEntityFilterStuff);
	float flFraction = TR_GetFraction(trace);
	delete trace;

	if (flFraction >= 1.0) 
		return !(flFraction == 0.0);
    
	return false;
}

// check if waypoint A has a connection to waypoint B
stock bool IsConnected(int pointA, int pointB)
{
    int i;
    for (i = 0; i < MaxPathIndex; i++)
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
    if ((destination[2] - src[2]) > 63.0)
        return false;

    float distance = GetVectorDistance(destination, src, true);

    // is the destination not close enough?
    if (distance > Squaredf(400.0))
        return false;

    // if waypoint is visible from current position
    if (IsVisibleWaypoint(src, destination))
    {
        // is dest waypoint higher than src? (45 is max jump height)
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

        while (distance > Squaredf(10.0))
        {
            // move 10 units closer to the goal...
            check[0] = check[0] + (direction[0] * 10.0);
            check[1] = check[1] + (direction[1] * 10.0);
            check[2] = check[2] + (direction[2] * 10.0);

            down = check;
            down[2] = down[2] - 1000.0; // straight down 1000 units

            Handle tr2 = TR_TraceRayFilterEx(check, down, MASK_PLAYERSOLID, RayType_EndPoint, WaypointFilter)

            float height = TR_GetFraction(tr2) * 1000.0; // height from ground
            
            CloseHandle(tr2);

            // is the current height greater than the step height?
            if (height < lastHeight - 18.0)
                return false; // can't get there without jumping...

            lastHeight = height;
            distance = GetVectorDistance(destination, check, true); // distance from goal
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

stock void WaypointAdd(float origin[3], bool cdebug = true, int flags = 0, bool autoPath = true)
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
    m_paths[index].flags = flags;
    m_paths[index].radius = GetConVarFloat(EBotRadius);
    m_paths[index].team = 0;

    m_paths[index].origin = origin;

    m_paths[index].campStart = NULL_VECTOR;
    m_paths[index].campEnd = NULL_VECTOR;

    int i;
    for (i = 0; i < MaxPathIndex; i++)
    {
        m_paths[index].pathIndex[i] = -1;
        m_paths[index].distance[i] = 0.0;
    }

     // calculate all the paths to this new waypoint
    if (autoPath)
    {
        for (i = 0; i < m_waypointNumber; i++)
        {
            // skip the waypoint that was just added
            if (index == i)
                continue; 

            // check if the waypoint is reachable from the new one (one-way)
            if (IsWaypointReachable(m_paths[index].origin, m_paths[i].origin))
                AddPath(index, i, GetVectorDistance2D(m_paths[index].origin, m_paths[i].origin));

            // check if the new one is reachable from the waypoint (other way)
            if (IsWaypointReachable(m_paths[i].origin, m_paths[index].origin))
                AddPath(i, index, GetVectorDistance2D(m_paths[i].origin, m_paths[index].origin));
        }
    }

    //CalculateWayzone(index);

    if (cdebug)
        PrintHintTextToAll("Waypoint added to %d %d %d", RoundFloat(m_paths[index].origin[0]), RoundFloat(m_paths[index].origin[1]), RoundFloat(m_paths[index].origin[2]));
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
        start = m_paths[index].origin;
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

    int i;

    // don't allow paths get connected twice
    for (i = 0; i < MaxPathIndex; i++)
    {
        if (m_paths[srcIndex].pathIndex[i] == destIndex)
        {
            PrintHintTextToAll("Denied path creation from %d to %d (path already exists)", srcIndex, destIndex);
            return;
        }
    }

    // check for free space in the connection indices
    for (i = 0; i < MaxPathIndex; i++)
    {
        if (m_paths[srcIndex].pathIndex[i] == -1)
        {
            m_paths[srcIndex].pathIndex[i] = destIndex;
            m_paths[srcIndex].distance[i] = distance;
            PrintHintTextToAll("Path added from %d to %d", srcIndex, destIndex);
            return;
        }
    }

    // there wasn't any free space try exchanging it with a long-distance path
    float maxDistance = -99999.0;
    int slotID = -1;

    for (i = 0; i < MaxPathIndex; i++)
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

stock void DeleteWaypointIndex(const int index)
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
    for (i = 0; i < m_waypointNumber; i++) // delete all references to waypoint
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
        if (m_paths[i].index > index) // if index bigger than deleted waypoint...
            m_paths[i].index--;

        for (j = 0; j < MaxPathIndex; j++)
        {
            if (m_paths[i].pathIndex[j] > index)
                m_paths[i].pathIndex[j]--;
        }
    }

    // free deleted waypoint
    WaypointClear(index);

    // Rotate Path Array down
    for (i = index; i < m_waypointNumber - 1; i++)
    {
        m_paths[i] = m_paths[i + 1];
        for (j = 0; j < 7; j++)
        {
            damageGLOBALred[j][i] = damageGLOBALred[j][i + 1];
            damageGLOBALblu[j][i] = damageGLOBALblu[j][i + 1];
        }
    }

    m_waypointNumber--;
    PrintHintTextToAll("Waypoint %d is deleted", index);
}

stock float HeuristicNormal(const int start, const int goal)
{
    return GetVectorDistance(m_paths[start].origin, m_paths[goal].origin, true);
}

stock float CostNormal(const int current, const int parent, const int client)
{
	return GetVectorDistance(m_paths[current].origin, m_paths[parent].origin, true) + GetDamage(client, parent);
}

stock float CostDistance(const int current, const int parent)
{
    return GetVectorDistance(m_paths[current].origin, m_paths[parent].origin, true);
}

// A* Pathfinding
stock bool AStarFindPath(int srcIndex, int destIndex, const int client, float targetposition[3] = NULL_VECTOR)
{
    if (!DeactiveCloak(client) || (!isVSH && CurrentProcess[client] == PRO_HIDE))
        return AStarFindHidingSpot(srcIndex, client, targetposition);

    if (CurrentProcess[client] == PRO_GETHEALTH)
        return AStarFindHealthPack(srcIndex, client);
    
    if (CurrentProcess[client] == PRO_GETAMMO)
        return AStarFindAmmoPack(srcIndex, client);

    if (isVSH)
        return AStarVSH(srcIndex, client);

    if (m_waypointNumber > 2048)
        return AStarFindShortestPath(srcIndex, destIndex, client, targetposition);

	if (destIndex == -1)
    {
        if (GetConVarInt(EBotDebug) == 1)
        {
            char buffer[128];
            GetClientName(client, buffer, sizeof(buffer));
            PrintHintTextToAll("Destination is invalid\nBot: %s", buffer);
        }

        return false;
    }

	if (srcIndex < 1 || !IsVisibleToWaypoint(GetEyePosition(client), srcIndex, GetHeight(client)))
		srcIndex = FindWaypoint(GetOrigin(client), client);
    
    if (srcIndex == destIndex)
		return false;
    
    int i;
    for (i = 0; i < m_waypointNumber; i++)
    {
        astar[i].f = 0.0;
        astar[i].g = 0.0;
        astar[i].parent = -1;
        astar[i].is_closed = false;
    }

    astar[srcIndex].g = CostNormal(srcIndex, destIndex, client);
    astar[srcIndex].f = astar[srcIndex].g + HeuristicNormal(srcIndex, destIndex);
    
    if (m_hidingSpots[client] == null)
    {
        delete m_hidingSpots[client];
        m_hidingSpots[client] = new ArrayList();
    }
    
    if (m_hidingSpots[client] != null)
        m_hidingSpots[client].Clear();
    
    int currentIndex;
    int self;
    int wearable;
    int limit = (m_waypointNumber / 2) + 2;

    float g;
    float f;
    float vector[3];

    PriorityQueue openList;
    openList.NewQueue();
    openList.InsertLowest(srcIndex, astar[srcIndex].f);
    while (!openList.IsEmpty())
    {
        currentIndex = openList.RemoveLowest();

        // is the current waypoint the goal waypoint?
        if (currentIndex == destIndex || openList.Size() > limit)
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
            while (currentIndex != -1)
            {
				AddToPath(currentIndex, client);
                currentIndex = astar[currentIndex].parent;
			}

            ReversePath(client);
            return true;
        }

        if (astar[currentIndex].is_closed)
            continue;

        // set current waypoint as CLOSED
        astar[currentIndex].is_closed = true;

        // now expand the current waypoint
        for (i = 0; i < MaxPathIndex; i++)
        {
            self = m_paths[currentIndex].pathIndex[i];
            if (self == -1)
                continue;
            
            if (self == m_lastFailedWaypoint[client])
                continue;
            
            // blocked waypoint
            if (self != destIndex && m_paths[self].activeArea != 0 && !(m_paths[self].activeArea & currentActiveArea))
                continue;
            
            if (m_paths[self].flags & WAYPOINT_DOUBLEJUMP && m_class[client] != TFClass_Scout)
                continue;

            if (m_paths[self].flags & WAYPOINT_ROCKETJUMP && (m_class[client] != TFClass_Soldier || m_lowHealth[client]))
                continue;

            if (m_paths[self].flags & WAYPOINT_DEMOCHARGE && m_class[client] != TFClass_DemoMan)
                continue;
            else
            {
                wearable = MaxClients + 1;
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
                        vector = m_paths[self].origin;
                        vector[2] += GetHeight(m_nearestEnemy[client]);
                        if (IsVisible(vector, GetEyePosition(m_nearestEnemy[client])))
                        {
                            if (GetVectorDistance(GetOrigin(m_nearestEnemy[client]), vector, true) - GetVectorDistance(GetOrigin(client), vector, true) < 1.0)
                                continue;
                        }
                    }
                    else
                        m_hidingSpots[client].Push(self);
                }
            }

            // calculate the F value as F = G + H
            g = astar[currentIndex].g + CostNormal(self, currentIndex, client);
            f = g + HeuristicNormal(self, destIndex);

            if (!astar[self].is_closed || astar[self].f > f)
            {
                astar[self].parent = currentIndex;
                astar[self].g = g;
                astar[self].f = f;
                openList.InsertLowest(self, f);
            }
        }
    }

    m_lastFailedWaypoint[client] = destIndex;

    ArrayList PossiblePath = new ArrayList();
    for (i = 0; i < m_waypointNumber; i++)
    {
        if (astar[i].is_closed)
        {
            if (isPL)
            {
                if (!(m_paths[i].activeArea & currentActiveArea))
                    continue;
                
                PossiblePath.Push(i);
            }
            else if (isCP || isAD)
            {
                if (m_paths[i].flags & WAYPOINT_CAPTUREPOINT)
                    continue;

                if (m_paths[i].activeArea != 0 && !(m_paths[i].activeArea & currentActiveArea))
                    continue;
                
                PossiblePath.Push(i);
            }
            else
                PossiblePath.Push(i);
        }
    }

    // just roam around :(
    if (PossiblePath.Length > 0)
    {
        i = PossiblePath.Get(crandomint(0, PossiblePath.Length - 1));
        delete PossiblePath;
        return AStarFindShortestPath(srcIndex, i, client, NULL_VECTOR);
    }

    delete PossiblePath;
    return AStarFindShortestPath(srcIndex, destIndex, client, targetposition);
}

// A* for second try
stock bool AStarFindShortestPath(int srcIndex, int destIndex, const int client, float targetposition[3] = NULL_VECTOR)
{
	if (destIndex < 1)
    {
        if (GetConVarInt(EBotDebug) == 1)
        {
            char buffer[128];
            GetClientName(client, buffer, sizeof(buffer));
            PrintHintTextToAll("Destination is invalid\nBot: %s", buffer);
        }

        return false;
    }
    
	if (srcIndex < 1 || !IsVisibleToWaypoint(GetEyePosition(client), srcIndex, GetHeight(client)))
		srcIndex = FindWaypoint(GetOrigin(client), client);
    
    if (srcIndex == destIndex)
		return false;
    
    int i;
    for (i = 0; i < m_waypointNumber; i++)
    {
        astar[i].f = 0.0;
        astar[i].g = 0.0;
        astar[i].parent = -1;
        astar[i].is_closed = false;
    }

    astar[srcIndex].g = CostDistance(srcIndex, destIndex);
    astar[srcIndex].f = astar[srcIndex].g + CostDistance(srcIndex, destIndex);

    int currentIndex;
    int self;
    int wearable;
    int limit = (m_waypointNumber / 2) + 2;

    float g;
    float f;
    
    PriorityQueue openList;
    openList.NewQueue();
    openList.InsertLowest(srcIndex, astar[srcIndex].f);
    while (!openList.IsEmpty())
    {
        currentIndex = openList.RemoveLowest();
		
        // is the current waypoint the goal waypoint?
        if (currentIndex == destIndex || openList.Size() > limit)
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
            while (currentIndex != -1)
            {
				AddToPath(currentIndex, client);
                currentIndex = astar[currentIndex].parent;
			}

            ReversePath(client);
            return true;
        }

        if (astar[currentIndex].is_closed)
            continue;

        // set current waypoint as CLOSED
        astar[currentIndex].is_closed = true;

        // now expand the current waypoint
        for (i = 0; i < MaxPathIndex; i++)
        {
            self = m_paths[currentIndex].pathIndex[i];
            if (self == -1)
                continue;

            if (self == m_lastFailedWaypoint[client])
                continue;
            
            // blocked waypoint
            if (self != destIndex && m_paths[self].activeArea != 0 && !(m_paths[self].activeArea & currentActiveArea))
                continue;
            
            if (m_paths[self].flags & WAYPOINT_DOUBLEJUMP && m_class[client] != TFClass_Scout)
                continue;

            if (m_paths[self].flags & WAYPOINT_ROCKETJUMP && (m_class[client] != TFClass_Soldier || m_lowHealth[client]))
                continue;
            
            if (m_paths[self].flags & WAYPOINT_DEMOCHARGE && m_class[client] != TFClass_DemoMan)
                continue;
            else
            {
                wearable = MaxClients + 1;
	            while ((wearable = FindEntityByClassname(wearable, "tf_wearable_demoshield*")) > MaxClients)
	            {
    	        	if (GetEntPropEnt(wearable, Prop_Send, "m_hOwnerEntity") != client)
                        continue;
                }
            }

            // calculate the F value
            g = astar[currentIndex].g + CostDistance(self, currentIndex);
            f = g + CostDistance(self, destIndex);

            if (!astar[self].is_closed || astar[self].f > f)
            {
                astar[self].parent = currentIndex;
                astar[self].g = g;
                astar[self].f = f;
                openList.InsertLowest(self, f);
            }
        }
    }

    ArrayList PossiblePath = new ArrayList();
    for (i = 0; i < m_waypointNumber; i++)
    {
        if (astar[i].is_closed)
            PossiblePath.Push(i);
    }

    // just roam around :(
    if (PossiblePath.Length > 0)
    {
        i = PossiblePath.Get(crandomint(0, PossiblePath.Length - 1));
        delete PossiblePath;
        return AStarFindShortestPath(srcIndex, i, client, NULL_VECTOR);
    }

    delete PossiblePath;
    return false;
}

stock bool AStarFindHidingSpot(int srcIndex, const int client, float dangerOrigin[3])
{
	if (srcIndex < 1 || !IsVisibleToWaypoint(GetEyePosition(client), srcIndex, GetHeight(client)))
		srcIndex = FindWaypoint(GetOrigin(client), client);
    
    int i;
    for (i = 0; i < m_waypointNumber; i++)
    {
        astar[i].f = 0.0;
        astar[i].parent = -1;
        astar[i].is_closed = false;
    }

    astar[srcIndex].f = GetVectorDistance(m_paths[srcIndex].origin, dangerOrigin, true);
    int currentIndex;
    int self;
    int wearable;
    float f;
    float height;
    height = GetHeight(client);

    PriorityQueue openList;
    openList.NewQueue();
    openList.InsertHighest(srcIndex, astar[srcIndex].f);
    while (!openList.IsEmpty())
    {
        currentIndex = openList.RemoveHighest();

        // is the current waypoint the goal waypoint?
        if (!(m_paths[currentIndex].flags & WAYPOINT_DONTHIDE) && !IsVisibleToWaypoint(dangerOrigin, currentIndex, height) && !IsDangerous(client, m_paths[currentIndex].origin, height))
        {
            DeletePathNodes(client);

            // build the complete path
            while (currentIndex != -1)
            {
				AddToPath(currentIndex, client);
                currentIndex = astar[currentIndex].parent;
                wearable = currentIndex;
			}

            if (wearable >= 0 && wearable < MaxWaypoints)
            {
                // are there health waypoint ??? stand on it, might we can get health pack
                for (i = 0; i < MaxPathIndex; i++)
                {
                    self = m_paths[wearable].pathIndex[i];
                    if (self == -1)
                        continue;
                
                    // blocked waypoint
                    if (m_paths[self].activeArea != 0 && !(m_paths[self].activeArea & currentActiveArea))
                        continue;
            
                    if (m_paths[self].team == 2 && m_team[client] == 3)
                        continue;
                    else if (m_paths[self].team == 3 && m_team[client] == 2)
                        continue;
                
                    if (m_paths[self].flags & WAYPOINT_HEALTH && !IsVisibleToWaypoint(dangerOrigin, wearable, height) && !IsDangerous(client, m_paths[wearable].origin, height))
                    {
                        AddToPath(self, client);
                        AddToPath(self, client);
                        ReversePath(client);
                        return true;
                    }
                }

                // if no health waypoints near, at least get some ammo...
                for (i = 0; i < MaxPathIndex; i++)
                {
                    self = m_paths[wearable].pathIndex[i];
                    if (self == -1)
                        continue;
                
                    // blocked waypoint
                    if (m_paths[self].activeArea != 0 && !(m_paths[self].activeArea & currentActiveArea))
                        continue;

                    if (m_paths[self].team == 2 && m_team[client] == 3)
                        continue;
                    else if (m_paths[self].team == 3 && m_team[client] == 2)
                        continue;
                
                    if (m_paths[self].flags & WAYPOINT_AMMO && !IsVisibleToWaypoint(dangerOrigin, wearable, height) && !IsDangerous(client, m_paths[wearable].origin, height))
                    {
                        AddToPath(self, client);
                        AddToPath(self, client);
                        ReversePath(client);
                        return true;
                    }
                }

                // AT LEAST TAKE THE MINIMUM DAMAGE
                int val = -1;
                float dmg = 0.0;
                float maxDmg = GetDamage(client, currentIndex);
                for (i = 0; i < MaxPathIndex; i++)
                {
                    self = m_paths[wearable].pathIndex[i];
                    if (self == -1)
                        continue;
                
                    // blocked waypoint
                    if (m_paths[self].activeArea != 0 && !(m_paths[self].activeArea & currentActiveArea))
                        continue;
                    
                    if (m_paths[self].team == 2 && m_team[client] == 3)
                        continue;
                    else if (m_paths[self].team == 3 && m_team[client] == 2)
                        continue;
                    
                    dmg = GetDamage(client, self);
                    if (dmg < maxDmg && !IsVisibleToWaypoint(dangerOrigin, wearable, height) && !IsDangerous(client, m_paths[wearable].origin, height))
                    {
                        val = self;
                        maxDmg = dmg;
                    }
                }

                if (val > -1)
                {
                    AddToPath(val, client);
                    AddToPath(val, client);
                    ReversePath(client);
                    return true;
                }
            }

            ReversePath(client);
            return true;
        }

        if (astar[currentIndex].is_closed)
            continue;

        // set current waypoint as CLOSED
        astar[currentIndex].is_closed = true;

        // now expand the current waypoint
        for (i = 0; i < MaxPathIndex; i++)
        {
            self = m_paths[currentIndex].pathIndex[i];
            if (self == -1)
                continue;

            if (self == m_lastFailedWaypoint[client])
                continue;
            
            // blocked waypoint
            if (m_paths[self].activeArea != 0 && !(m_paths[self].activeArea & currentActiveArea))
                continue;
            
            if (m_paths[self].team == 2 && m_team[client] == 3)
                continue;
            else if (m_paths[self].team == 3 && m_team[client] == 2)
                continue;
            
            if (m_paths[self].flags & WAYPOINT_DOUBLEJUMP && m_class[client]!= TFClass_Scout)
                continue;

            if (m_paths[self].flags & WAYPOINT_ROCKETJUMP && (m_class[client] != TFClass_Soldier || m_lowHealth[client]))
                continue;
            
            if (m_paths[self].flags & WAYPOINT_DEMOCHARGE && m_class[client] != TFClass_DemoMan)
                continue;
            else
            {
                wearable = MaxClients + 1;
	            while ((wearable = FindEntityByClassname(wearable, "tf_wearable_demoshield*")) > MaxClients)
	            {
    	        	if (GetEntPropEnt(wearable, Prop_Send, "m_hOwnerEntity") != client)
                        continue;
                }
            }

            f = GetVectorDistance(m_paths[self].origin, dangerOrigin, true);
            if (!astar[self].is_closed || astar[self].f > f)
            {
                astar[self].parent = currentIndex;
                astar[self].f = f;
                openList.InsertHighest(self, f);
            }
        }
    }

    ArrayList PossiblePath = new ArrayList();
    for (i = 0; i < m_waypointNumber; i++)
    {
        if (astar[i].is_closed)
            PossiblePath.Push(i);
    }

    // just roam around :(
    if (PossiblePath.Length > 0)
    {
        i = PossiblePath.Get(crandomint(0, PossiblePath.Length - 1));
        delete PossiblePath;
        return AStarFindShortestPath(srcIndex, i, client, NULL_VECTOR);
    }

    delete PossiblePath;
    return false;
}

stock bool AStarVSH(int srcIndex, const int client)
{
	if (srcIndex < 1 || !IsVisibleToWaypoint(GetEyePosition(client), srcIndex, GetHeight(client)))
		srcIndex = FindWaypoint(GetOrigin(client), client);

    for (int i = 0; i < m_waypointNumber; i++)
    {
        astar[i].f = 0.0;
        astar[i].parent = -1;
        astar[i].is_closed = false;
    }

    float dangerOrigin[3];
    dangerOrigin = GetOrigin(client);

    int limit = (m_waypointNumber / 2) + 2;

    astar[srcIndex].f = GetVectorDistance(m_paths[srcIndex].origin, dangerOrigin, true);

    PriorityQueue openList;
    openList.NewQueue();
    openList.InsertHighest(srcIndex, astar[srcIndex].f);
    while (!openList.IsEmpty())
    {
        int currentIndex = openList.RemoveHighest();
        float height = GetHeight(client);

        // is the current waypoint the goal waypoint?
        if (limit > openList.Size() || (m_team[client] == 3 && IsEnemyInDistance(client, currentIndex, height, 256.0)) || (m_team[client] != 3 && !IsEnemyInDistance(client, currentIndex, height, 600.0) && IsVisibleByEnemy(client, currentIndex, height)))
        {
            DeletePathNodes(client);

            // build the complete path
            while (currentIndex != -1)
            {
				AddToPath(currentIndex, client);
                currentIndex = astar[currentIndex].parent;
			}

            ReversePath(client);
            return true;
        }

        if (astar[currentIndex].is_closed)
            continue;

        // set current waypoint as CLOSED
        astar[currentIndex].is_closed = true;

        // now expand the current waypoint
        for (int i = 0; i < MaxPathIndex; i++)
        {
            int self = m_paths[currentIndex].pathIndex[i];
            if (self == -1)
                continue;

            if (self == m_lastFailedWaypoint[client])
                continue;
            
            // blocked waypoint
            if (m_paths[self].activeArea != 0 && !(m_paths[self].activeArea & currentActiveArea))
                continue;
            
            if (m_paths[self].team == 2 && m_team[client] == 3)
                continue;
            else if (m_paths[self].team == 3 && m_team[client] == 2)
                continue;
            
            if (m_paths[self].flags & WAYPOINT_DOUBLEJUMP && m_class[client]!= TFClass_Scout)
                continue;

            if (m_paths[self].flags & WAYPOINT_ROCKETJUMP && (m_class[client] != TFClass_Soldier || m_lowHealth[client]))
                continue;
            
            if (m_paths[self].flags & WAYPOINT_DEMOCHARGE && m_class[client] != TFClass_DemoMan)
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

            float f = GetVectorDistance(m_paths[self].origin, dangerOrigin, true);
            if (!astar[self].is_closed || astar[self].f > f)
            {
                astar[self].parent = currentIndex;
                astar[self].f = f;
                openList.InsertHighest(self, f);
            }
        }
    }

    ArrayList PossiblePath = new ArrayList();
    for (int i = 0; i < m_waypointNumber; i++)
    {
        if (astar[i].is_closed)
            PossiblePath.Push(i);
    }

    // just roam around :(
    if (PossiblePath.Length > 0)
    {
        int i = PossiblePath.Get(crandomint(0, PossiblePath.Length - 1));
        delete PossiblePath;
        return AStarFindShortestPath(srcIndex, i, client, NULL_VECTOR);
    }

    delete PossiblePath;
    return false;
}

stock bool AStarFindHealthPack(int srcIndex, const int client)
{
	if (srcIndex < 1 || !IsVisibleToWaypoint(GetEyePosition(client), srcIndex, GetHeight(client)))
		srcIndex = FindWaypoint(GetOrigin(client), client);
    
    int i;
    for (i = 0; i < m_waypointNumber; i++)
    {
        astar[i].f = 0.0;
        astar[i].parent = -1;
        astar[i].is_closed = false;
    }

    float dangerOrigin[3];
    dangerOrigin = GetOrigin(client);
    astar[srcIndex].f = GetVectorDistance(m_paths[srcIndex].origin, dangerOrigin, true);

    int currentIndex;
    int self;
    int wearable;
    float f;

    PriorityQueue openList;
    openList.NewQueue();
    openList.InsertLowest(srcIndex, astar[srcIndex].f);
    while (!openList.IsEmpty())
    {
        currentIndex = openList.RemoveLowest();

        // is the current waypoint the goal waypoint?
        if (m_paths[currentIndex].flags & WAYPOINT_RESUPPLY || (m_paths[currentIndex].flags & WAYPOINT_HEALTH && FindNearestHealthPack(m_paths[currentIndex].origin, 75.0) != -1 && !IsDangerous(client, m_paths[currentIndex].origin, GetHeight(client))))
        {
            DeletePathNodes(client);

            // build the complete path
            while (currentIndex != -1)
            {
				AddToPath(currentIndex, client);
                currentIndex = astar[currentIndex].parent;
			}

            ReversePath(client);
            return true;
        }

        if (astar[currentIndex].is_closed)
            continue;

        // set current waypoint as CLOSED
        astar[currentIndex].is_closed = true;

        // now expand the current waypoint
        for (i = 0; i < MaxPathIndex; i++)
        {
            self = m_paths[currentIndex].pathIndex[i];
            if (self == -1)
                continue;

            if (self == m_lastFailedWaypoint[client])
                continue;
            
            // blocked waypoint
            if (m_paths[self].activeArea != 0 && !(m_paths[self].activeArea & currentActiveArea))
                continue;
            
            if (m_paths[self].team == 2 && m_team[client] == 3)
                continue;
            else if (m_paths[self].team == 3 && m_team[client] == 2)
                continue;
            
            if (m_paths[self].flags & WAYPOINT_DOUBLEJUMP && m_class[client] != TFClass_Scout)
                continue;

            if (m_paths[self].flags & WAYPOINT_ROCKETJUMP && (m_class[client] != TFClass_Soldier || m_lowHealth[client]))
                continue;
            
            if (m_paths[self].flags & WAYPOINT_DEMOCHARGE && m_class[client] != TFClass_DemoMan)
                continue;
            else
            {
                wearable = MaxClients + 1;
	            while ((wearable = FindEntityByClassname(wearable, "tf_wearable_demoshield*")) > MaxClients)
	            {
    	        	if (GetEntPropEnt(wearable, Prop_Send, "m_hOwnerEntity") != client)
                        continue;
                }
            }

            f = GetVectorDistance(m_paths[self].origin, dangerOrigin, true);
            if (!astar[self].is_closed || astar[self].f > f)
            {
                astar[self].parent = currentIndex;
                astar[self].f = f;
                openList.InsertLowest(self, f);
            }
        }
    }

    ArrayList PossiblePath = new ArrayList();
    for (i = 0; i < m_waypointNumber; i++)
    {
        if (astar[i].is_closed)
            PossiblePath.Push(i);
    }

    // just roam around :(
    if (PossiblePath.Length > 0)
    {
        i = PossiblePath.Get(crandomint(0, PossiblePath.Length - 1));
        delete PossiblePath;
        return AStarFindShortestPath(srcIndex, i, client, NULL_VECTOR);
    }

    delete PossiblePath;
    return false;
}

stock int FindNearestHealthPack(const float origin[3], const float maxRange)
{
	float distance = 999999999999.0;
    float edict_distance;
	int entity = -1;
    int x;
	for (x = 0; x < GetMaxEntities(); x++)
	{
		if (!IsValidHealthPack(x))
			continue;
		
		edict_distance = GetFastDistance(origin, GetOrigin(x));
		if (edict_distance < distance)
		{
			distance = edict_distance;
			entity = x;
		}
	}

    if (IsValidEntity(entity) && GetVectorDistance(origin, GetOrigin(entity), true) < Squaredf(maxRange))
        return entity;

	return -1;
}

stock bool IsValidHealthPack(const int entity)
{
	if (!IsValidEntity(entity))
		return false;

	if (!HasEntProp(entity, Prop_Send, "m_fEffects"))
		return false;

	if (GetEntProp(entity, Prop_Send, "m_fEffects") != 0)
		return false;

	char class[64];
	GetEntityClassname(entity, class, sizeof(class));
	
	if (StrContains(class, "item_health", false) == -1 && StrContains(class, "obj_dispenser", false) == -1)
		return false;
	
	return true;
}

stock bool AStarFindAmmoPack(int srcIndex, const int client)
{
	if (srcIndex < 1 || !IsVisibleToWaypoint(GetEyePosition(client), srcIndex, GetHeight(client)))
		srcIndex = FindWaypoint(GetOrigin(client), client);
    
    int i;
    for (i = 0; i < m_waypointNumber; i++)
    {
        astar[i].f = 0.0;
        astar[i].parent = -1;
        astar[i].is_closed = false;
    }

    float dangerOrigin[3];
    dangerOrigin = GetOrigin(client);

    astar[srcIndex].f = GetVectorDistance(m_paths[srcIndex].origin, dangerOrigin, true);

    int currentIndex;
    int self;
    int wearable;
    float f;

    PriorityQueue openList;
    openList.NewQueue();
    openList.InsertLowest(srcIndex, astar[srcIndex].f);
    while (!openList.IsEmpty())
    {
        currentIndex = openList.RemoveLowest();

        // is the current waypoint the goal waypoint?
        if (m_paths[currentIndex].flags & WAYPOINT_RESUPPLY || (m_paths[currentIndex].flags & WAYPOINT_AMMO && FindNearestAmmoPack(m_paths[currentIndex].origin, 75.0) != -1 && !IsDangerous(client, m_paths[currentIndex].origin, GetHeight(client))))
        {
            DeletePathNodes(client);

            // build the complete path
            while (currentIndex != -1)
            {
				AddToPath(currentIndex, client);
                currentIndex = astar[currentIndex].parent;
			}

            ReversePath(client);
            return true;
        }

        if (astar[currentIndex].is_closed)
            continue;

        // set current waypoint as CLOSED
        astar[currentIndex].is_closed = true;

        // now expand the current waypoint
        for (i = 0; i < MaxPathIndex; i++)
        {
            self = m_paths[currentIndex].pathIndex[i];
            if (self == -1)
                continue;

            if (self == m_lastFailedWaypoint[client])
                continue;
            
            // blocked waypoint
            if (m_paths[self].activeArea != 0 && !(m_paths[self].activeArea & currentActiveArea))
                continue;
            
            if (m_paths[self].team == 2 && m_team[client] == 3)
                continue;
            else if (m_paths[self].team == 3 && m_team[client] == 2)
                continue;
            
            if (m_paths[self].flags & WAYPOINT_DOUBLEJUMP && m_class[client] != TFClass_Scout)
                continue;

            if (m_paths[self].flags & WAYPOINT_ROCKETJUMP && (m_class[client] != TFClass_Soldier || m_lowHealth[client]))
                continue;
            
            if (m_paths[self].flags & WAYPOINT_DEMOCHARGE && m_class[client] != TFClass_DemoMan)
                continue;
            else
            {
                wearable = MaxClients + 1;
	            while ((wearable = FindEntityByClassname(wearable, "tf_wearable_demoshield*")) > MaxClients)
	            {
    	        	if (GetEntPropEnt(wearable, Prop_Send, "m_hOwnerEntity") != client)
                        continue;
                }
            }

            f = GetVectorDistance(m_paths[self].origin, dangerOrigin, true);
            if (!astar[self].is_closed || astar[self].f > f)
            {
                astar[self].parent = currentIndex;
                astar[self].f = f;
                openList.InsertLowest(self, f);
            }
        }
    }

    ArrayList PossiblePath = new ArrayList();
    for (i = 0; i < m_waypointNumber; i++)
    {
        if (astar[i].is_closed)
            PossiblePath.Push(i);
    }

    // just roam around :(
    if (PossiblePath.Length > 0)
    {
        i = PossiblePath.Get(crandomint(0, PossiblePath.Length - 1));
        delete PossiblePath;
        return AStarFindShortestPath(srcIndex, i, client, NULL_VECTOR);
    }

    delete PossiblePath;
    return false;
}

stock int FindNearestAmmoPack(const float origin[3], const float maxRange)
{
	float distance = 999999999999.0;
    float edict_distance;
	int entity = -1;
    int x;
	for (x = 0; x < GetMaxEntities(); x++)
	{
		if (!IsValidAmmoPack(x))
			continue;
		
		edict_distance = GetFastDistance(origin, GetOrigin(x));
		if (edict_distance < distance)
		{
			distance = edict_distance;
			entity = x;
		}
	}

    if (IsValidEntity(entity) && GetVectorDistance(origin, GetOrigin(entity), true) < Squaredf(maxRange))
        return entity;

	return -1;
}

stock bool IsValidAmmoPack(const int entity)
{
	if (!IsValidEntity(entity))
		return false;

	if (!HasEntProp(entity, Prop_Send, "m_fEffects"))
		return false;

	if (GetEntProp(entity, Prop_Send, "m_fEffects") != 0)
		return false;

	char class[64]; 
	GetEntityClassname(entity, class, sizeof(class));
	
	if (StrContains(class, "tf_ammo_pack", false) == -1 && StrContains(class, "item_ammo", false) == -1 && StrContains(class, "tf_ammo_pack", false) == -1)
		return false;
	
	return true;
}

stock bool IsPathPossibleWaypoint(int srcIndex, int destIndex)
{
	if (destIndex == -1)
        return false;
    
	if (srcIndex == -1)
		return true;
    
    if (srcIndex == destIndex)
		return true;
    
    int i, self;
    float f;
    for (i = 0; i < m_waypointNumber; i++)
    {
        astar[i].f = 0.0;
        astar[i].parent = -1;
        astar[i].is_closed = false;
    }

    astar[srcIndex].f = GetWaypointDistance(srcIndex, destIndex);
    
    int currentIndex;

    PriorityQueue openList;
    openList.NewQueue();
    openList.InsertLowest(srcIndex, astar[srcIndex].f);
    while (!openList.IsEmpty())
    {
        currentIndex = openList.RemoveLowest();
		
        // is the current waypoint the goal waypoint?
        if (currentIndex == destIndex)
            return true;

        if (astar[currentIndex].is_closed)
            continue;

        // set current waypoint as CLOSED
        astar[currentIndex].is_closed = true;

        // now expand the current waypoint
        for (i = 0; i < MaxPathIndex; i++)
        {
            self = m_paths[currentIndex].pathIndex[i];
            if (self == -1)
                continue;

            f = GetWaypointDistance(self, destIndex);
            if (!astar[self].is_closed || astar[self].f > f)
            {
                astar[self].parent = currentIndex;
                astar[self].f = f;
                openList.InsertLowest(self, f);
            }
        }
    }

    return false;
}

stock bool IsVisibleByEnemy(const int client, const int waypointIndex, const float height)
{
    float origin[3];
    origin = m_paths[waypointIndex].origin;
    origin[2] += height;

    int i;
    for (i = 1; i <= MaxClients; i++)
    {
        if (!IsValidClient(i))
			continue;
        
        // count deads too, because (almost) every teammate knows where is his friend is died, or died teammate can say where is enemy, etc. so basically even if he's dead this area is not safe
		//if (!m_isAlive[i])
		//	continue;
		
		if (m_team[i] == m_team[client])
			continue;
        
        if (IsVisible(origin, GetEyePosition(i)))
            return true;
    }

    return false;
}

stock bool IsEnemyInDistance(const int client, const int waypointIndex, const float height, const float distance)
{
    float origin[3];
    origin = m_paths[waypointIndex].origin;
    origin[2] += height;

    int i;
    for (i = 1; i <= MaxClients; i++)
    {
        if (!IsValidClient(i))
			continue;
        
        // count deads too, because (almost) every teammate knows where is his friend is died, or died teammate can say where is enemy, etc. so basically even if he's dead this area is not safe
		//if (!m_isAlive[i])
		//	continue;
		
		if (m_team[i] == m_team[client])
			continue;
        
        if (GetVectorDistance(origin, GetEyePosition(i), true) < Squaredf(distance))
            return true;
    }

    return false;
}

stock void AddToPath(const int currentIndex, const int client)
{
    m_pathIndex[client].Push(currentIndex);
    float radius;
    radius = m_paths[currentIndex].radius;

    if (radius > 0.0)
    {
        float origin[3];
        origin[0] = m_paths[currentIndex].origin[0] + crandomfloat(-radius, radius);
        origin[1] = m_paths[currentIndex].origin[1] + crandomfloat(-radius, radius);
        origin[2] = m_paths[currentIndex].origin[2];
        m_positions[client].PushArray(origin, 3);
    }
    else
        m_positions[client].PushArray(m_paths[currentIndex].origin, 3);
}

stock void ReversePath(const int client)
{
    float flFromPos[3];
    float flToPos[3];

    int i;
	for (i = m_positions[client].Length - 1; i > 0; i--)
	{
		m_positions[client].GetArray(i, flFromPos, 3);
		m_positions[client].GetArray(i - 1, flToPos, 3);
	}

	for (i = m_pathIndex[client].Length - 1; i > 0; i--)
	{
		m_pathIndex[client].GetArray(i, flFromPos);
		m_pathIndex[client].GetArray(i - 1, flToPos);
	}

	m_targetNode[client] = m_positions[client].Length - 1;
	m_currentWaypointIndex[client] = m_pathIndex[client].Length - 1;
}

// find the nearest waypoint to that origin, and return the index to this waypoint
stock int FindNearestWaypoint(const float origin[3], const float maxDistance = 9999999.0, const int client = -1)
{
    int index = -1;
    float distance;
    float normalDist = maxDistance * maxDistance;
    bool isValid;
    
    if (IsValidClient(client))
        isValid = true;
    else
        isValid = false;
    
    int i;
    for (i = 0; i < m_waypointNumber; i++)
    {
        // blocked waypoint
   		if (m_paths[i].activeArea != 0 && !(m_paths[i].activeArea & currentActiveArea))
			continue;

        if (isValid)
        {
            if (m_paths[i].team == 2 && m_team[client] == 3)
                continue;
            else if (m_paths[i].team == 3 && m_team[client] == 2)
                continue;
        }
        
        distance = GetVectorDistance(m_paths[i].origin, origin, true);
        if (distance < normalDist)
        {
            index = i;
            normalDist = distance;
        }
    }
    
    return index;
}

stock int FindWaypoint(const float origin[3], const int client = -1)
{
    int index = -1;
    float normalDist = 999999999999999.0;
    float eyePosition[3];
    eyePosition = GetEyePosition(client);
    float height = GetHeight(client);
    float distance;
    int i;
    for (i = 0; i < m_waypointNumber; i++)
    {
        // blocked waypoint
   		if (m_paths[i].activeArea != 0 && !(m_paths[i].activeArea & currentActiveArea))
			continue;

        if (!IsVisibleToWaypoint(eyePosition, i, height))
            continue;
        
        distance = GetFastDistance(m_paths[i].origin, origin);
        if (distance < normalDist)
        {
            index = i;
            normalDist = distance;
        }
    }

    if (index == -1)
        return FindWaypointFast(origin, client);
    
    return index;
}

stock int FindWaypointFast(const float origin[3], const int client = -1)
{
    int index = -1;
    float normalDist = 999999999999999.0;
    bool isValid;
    float distance;
    
    if (IsValidClient(client))
        isValid = true;
    else
        isValid = false;
    
    int i;
    for (i = 0; i < m_waypointNumber; i++)
    {
        // blocked waypoint
   		if (m_paths[i].activeArea != 0 && !(m_paths[i].activeArea & currentActiveArea))
			continue;

        if (isValid)
        {
            if (m_paths[i].team == 2 && m_team[client] == 3)
                continue;
            else if (m_paths[i].team == 3 && m_team[client] == 2)
                continue;
        }
        
        distance = GetFastDistance(m_paths[i].origin, origin);
        if (distance < normalDist)
        {
            index = i;
            normalDist = distance;
        }
    }
    
    return index;
}

stock int FindWaypointFastest(float origin[3])
{
    int index = -1;
    float normalDist = 999999999999999.0;
    float distance;
    int i;
    for (i = 0; i < m_waypointNumber; i++)
    {
        // blocked waypoint
   		if (m_paths[i].activeArea != 0 && !(m_paths[i].activeArea & currentActiveArea))
			continue;

        distance = GetFastDistance(m_paths[i].origin, origin);
        if (distance < normalDist)
        {
            index = i;
            normalDist = distance;
        }
    }

    return index;
}

stock void DeletePathNodes(const int client)
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

stock void WaypointClear(const int index)
{
    m_paths[index].index = index;
    m_paths[index].activeArea = 0;
    m_paths[index].flags = 0;
    m_paths[index].team = 0;
    m_paths[index].radius = 0.0;
    m_paths[index].origin = {0.0, 0.0, 0.0};
    m_paths[index].campStart = {0.0, 0.0, 0.0};
    m_paths[index].campEnd = {0.0, 0.0, 0.0};
    int i;
    for (i = 0; i < MaxPathIndex; i++)
    {
        m_paths[index].pathIndex[i] = -1;
        m_paths[index].distance[i] = 0.0;
    }
}

stock float GetWaypointDistance(const int srcIndex, const int destIndex)
{
    int i;
    for (i = 0; i < MaxPathIndex; i++)
    {
        if (m_paths[srcIndex].pathIndex[i] == destIndex)
            return m_paths[srcIndex].distance[i];
    }

    return GetFastDistance2D(m_paths[srcIndex].origin, m_paths[destIndex].origin);
}

stock bool IsWaypointUsed(const int index)
{
    int i;
    for (i = 0; i < MaxClients; i++)
    {
        if (!IsValidClient(i))
            continue;
        
        if (!m_isAlive[i])
            continue;

        if (m_goalIndex[i] == index || m_currentIndex[i] == index || GetVectorDistance(GetOrigin(i), m_paths[index].origin, true) < Squaredf(64.0))
            return true;
    }

    return false;
}

stock void SetGoalIndex(const int client, const int index)
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
    m_hasCaptureWaypoints = false;

    int i;
    for (i = 0; i < m_waypointNumber; i++)
    {
        if (m_paths[i].flags & WAYPOINT_RESUPPLY)
        {
            m_hasHealthWaypoints = true;
            m_hasAmmoWaypoints = true;
        }

        if (m_paths[i].flags & WAYPOINT_HEALTH)
            m_hasHealthWaypoints = true;

        if (m_paths[i].flags & WAYPOINT_AMMO)
            m_hasAmmoWaypoints = true;

        if (m_paths[i].flags & WAYPOINT_SNIPER)
            m_hasSniperWaypoints = true;

        if (m_paths[i].flags & WAYPOINT_DEMOMANCAMP)
            m_hasStickyWaypoints = true;

        if (m_paths[i].flags & WAYPOINT_ROUTE)
            m_hasRouteWaypoints = true;

        if (m_paths[i].flags & WAYPOINT_SENTRY)
            m_hasSentryWaypoints = true;

        if (m_paths[i].flags & WAYPOINT_TELEPORTERENTER)
            m_hasTeleporterEnterWaypoints = true;

        if (m_paths[i].flags & WAYPOINT_TELEPORTEREXIT)
            m_hasTeleporterExitWaypoints = true;

        if (m_paths[i].flags & WAYPOINT_ROCKETJUMP)
            m_hasRocketJumpWaypoints = true;
        
        if (m_paths[i].flags & WAYPOINT_CAPTUREPOINT)
            m_hasCaptureWaypoints = true;
    }
}

stock void WaypointLoad()
{
    bool isNav = false;
    char filepath[PLATFORM_MAX_PATH];
    BuildPath(Path_SM, filepath, sizeof(filepath), "ebot/waypoints/%s.ewp", currentMap);
    File fp = OpenFile(filepath, "rb");
    if (fp == null)
        return;

    CreateTimer(15.0, PrintHudTextOnStart);
    
    m_aboutTheWaypoint = "No waypoint file found\nBots will be idle";

    if (fp == null && NavMesh.IsLoaded())
    {
        BuildPath(Path_SM, filepath, sizeof(filepath), "ebot/waypoints/nav/%s.ewp", currentMap);
        fp = OpenFile(filepath, "rb");

        if (fp != null)
        {
            isNav = true;
            m_aboutTheWaypoint = "No waypoint file found\nNavMesh will be used for pathing";
        }
        else
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
        FormatEx(m_aboutTheWaypoint, sizeof(m_aboutTheWaypoint), "Waypoint version is too high\nPlease update your ebot!\nThis waypoint file will be ignored %s", header.fileVersion);
        fp.Close();
        return;
    }

    FormatEx(m_aboutTheWaypoint, sizeof(m_aboutTheWaypoint), "Waypoint file by %s\n%d waypoints loaded", header.author, m_waypointNumber);

    if (header.fileVersion == 1)
    {
        int i;
        PathOLD m_pathsOLD[MaxWaypoints];

        for (i = 0; i < m_waypointNumber; i++)
            fp.Read(m_pathsOLD[i], sizeof(PathOLD), 4);

        for (i = 0; i < m_waypointNumber; i++)
        {
            m_paths[i].index = m_pathsOLD[i].index;
            m_paths[i].pathIndex = m_pathsOLD[i].pathIndex;
            if (m_pathsOLD[i].activeArea == 1)
                m_paths[i].activeArea = AREA1;
            else if (m_pathsOLD[i].activeArea == 2)
                m_paths[i].activeArea = AREA2;
            else if (m_pathsOLD[i].activeArea == 3)
                m_paths[i].activeArea = AREA3;
            else if (m_pathsOLD[i].activeArea == 4)
                m_paths[i].activeArea = AREA4;
            else if (m_pathsOLD[i].activeArea == 5)
                m_paths[i].activeArea = AREA5;
            else if (m_pathsOLD[i].activeArea == 6)
                m_paths[i].activeArea = AREA6;
            else if (m_pathsOLD[i].activeArea == 7)
                m_paths[i].activeArea = AREA7;
            else
                m_paths[i].activeArea = 0;
            m_paths[i].team = m_pathsOLD[i].team;
            if (m_pathsOLD[i].flags == 1)
                m_paths[i].flags = WAYPOINT_JUMP;
            else if (m_pathsOLD[i].flags == 2)
                m_paths[i].flags = WAYPOINT_SNIPER;
            else if (m_pathsOLD[i].flags == 3)
                m_paths[i].flags = WAYPOINT_DEFEND;
            else if (m_pathsOLD[i].flags == 4)
                m_paths[i].flags = WAYPOINT_FALLDAMAGE;
            else if (m_pathsOLD[i].flags == 5)
                m_paths[i].flags = WAYPOINT_CROUCH;
            else if (m_pathsOLD[i].flags == 6)
                m_paths[i].flags = WAYPOINT_AMMO;
            else if (m_pathsOLD[i].flags == 7)
                m_paths[i].flags = WAYPOINT_HEALTH;
            else if (m_pathsOLD[i].flags == 8)
                m_paths[i].flags = WAYPOINT_RESUPPLY;
            else if (m_pathsOLD[i].flags == 9)
                m_paths[i].flags = WAYPOINT_DOUBLEJUMP;
            else if (m_pathsOLD[i].flags == 10)
                m_paths[i].flags = WAYPOINT_ROCKETJUMP;
            else if (m_pathsOLD[i].flags == 11)
                m_paths[i].flags = WAYPOINT_DEMOMANCAMP;
            else if (m_pathsOLD[i].flags == 12)
                m_paths[i].flags = WAYPOINT_SENTRY;
            else if (m_pathsOLD[i].flags == 13)
                m_paths[i].flags = WAYPOINT_ROUTE;
            else if (m_pathsOLD[i].flags == 14)
                m_paths[i].flags = WAYPOINT_TELEPORTERENTER;
            else if (m_pathsOLD[i].flags == 15)
                m_paths[i].flags = WAYPOINT_TELEPORTEREXIT;
            else if (m_pathsOLD[i].flags == 16)
                m_paths[i].flags = WAYPOINT_CAPTUREPOINT;
            else if (m_pathsOLD[i].flags == 17)
                m_paths[i].flags = WAYPOINT_NOSTRAFE;
            else if (m_pathsOLD[i].flags == 18)
                m_paths[i].flags = WAYPOINT_NOSPY;
            else if (m_pathsOLD[i].flags == 19)
                m_paths[i].flags = WAYPOINT_DEMOCHARGE;
            else
                m_paths[i].flags = 0;
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
    else if (header.fileVersion == 2)
    {
        int i;
        WPath m_pathsOLD[MaxWaypoints];
        for (i = 0; i < m_waypointNumber; i++)
            fp.Read(m_pathsOLD[i], sizeof(WPath), 4);

        for (i = 0; i < m_waypointNumber; i++)
        {
            m_paths[i].index = m_pathsOLD[i].index;
            m_paths[i].pathIndex = m_pathsOLD[i].pathIndex;
            if (m_pathsOLD[i].activeArea == 1)
                m_paths[i].activeArea = AREA1;
            else if (m_pathsOLD[i].activeArea == 2)
                m_paths[i].activeArea = AREA2;
            else if (m_pathsOLD[i].activeArea == 3)
                m_paths[i].activeArea = AREA3;
            else if (m_pathsOLD[i].activeArea == 4)
                m_paths[i].activeArea = AREA4;
            else if (m_pathsOLD[i].activeArea == 5)
                m_paths[i].activeArea = AREA5;
            else if (m_pathsOLD[i].activeArea == 6)
                m_paths[i].activeArea = AREA6;
            else if (m_pathsOLD[i].activeArea == 7)
                m_paths[i].activeArea = AREA7;
            else
                m_paths[i].activeArea = 0;
            m_paths[i].team = m_pathsOLD[i].team;
            if (m_pathsOLD[i].flags == 1)
                m_paths[i].flags = WAYPOINT_JUMP;
            else if (m_pathsOLD[i].flags == 2)
                m_paths[i].flags = WAYPOINT_SNIPER;
            else if (m_pathsOLD[i].flags == 3)
                m_paths[i].flags = WAYPOINT_DEFEND;
            else if (m_pathsOLD[i].flags == 4)
                m_paths[i].flags = WAYPOINT_FALLDAMAGE;
            else if (m_pathsOLD[i].flags == 5)
                m_paths[i].flags = WAYPOINT_CROUCH;
            else if (m_pathsOLD[i].flags == 6)
                m_paths[i].flags = WAYPOINT_AMMO;
            else if (m_pathsOLD[i].flags == 7)
                m_paths[i].flags = WAYPOINT_HEALTH;
            else if (m_pathsOLD[i].flags == 8)
                m_paths[i].flags = WAYPOINT_RESUPPLY;
            else if (m_pathsOLD[i].flags == 9)
                m_paths[i].flags = WAYPOINT_DOUBLEJUMP;
            else if (m_pathsOLD[i].flags == 10)
                m_paths[i].flags = WAYPOINT_ROCKETJUMP;
            else if (m_pathsOLD[i].flags == 11)
                m_paths[i].flags = WAYPOINT_DEMOMANCAMP;
            else if (m_pathsOLD[i].flags == 12)
                m_paths[i].flags = WAYPOINT_SENTRY;
            else if (m_pathsOLD[i].flags == 13)
                m_paths[i].flags = WAYPOINT_ROUTE;
            else if (m_pathsOLD[i].flags == 14)
                m_paths[i].flags = WAYPOINT_TELEPORTERENTER;
            else if (m_pathsOLD[i].flags == 15)
                m_paths[i].flags = WAYPOINT_TELEPORTEREXIT;
            else if (m_pathsOLD[i].flags == 16)
                m_paths[i].flags = WAYPOINT_CAPTUREPOINT;
            else if (m_pathsOLD[i].flags == 17)
                m_paths[i].flags = WAYPOINT_NOSTRAFE;
            else if (m_pathsOLD[i].flags == 18)
                m_paths[i].flags = WAYPOINT_NOSPY;
            else if (m_pathsOLD[i].flags == 19)
                m_paths[i].flags = WAYPOINT_DEMOCHARGE;
            else
                m_paths[i].flags = 0;
            m_paths[i].origin = m_pathsOLD[i].origin;
            m_paths[i].campStart = m_pathsOLD[i].campStart;
            m_paths[i].campEnd = m_pathsOLD[i].campEnd;
            m_paths[i].radius = m_pathsOLD[i].radius;
            m_paths[i].distance[0] = m_pathsOLD[i].distance[0];
            m_paths[i].distance[1] = m_pathsOLD[i].distance[1];
            m_paths[i].distance[2] = m_pathsOLD[i].distance[2];
            m_paths[i].distance[3] = m_pathsOLD[i].distance[3];
            m_paths[i].distance[4] = m_pathsOLD[i].distance[4];
            m_paths[i].distance[5] = m_pathsOLD[i].distance[5];
            m_paths[i].distance[6] = m_pathsOLD[i].distance[6];
            m_paths[i].distance[7] = m_pathsOLD[i].distance[7];
        }
    }
    else
    {
        // read the all waypoint data
        int i;
        for (i = 0; i < m_waypointNumber; i++)
            fp.Read(m_paths[i], sizeof(WPath), 4);
    }
    
    fp.Close();
    
    if (isNav)
        m_hasNavpoints = true;
    else
        m_hasWaypoints = true;
    
    InitTypes();

    return;
}

stock void WaypointSave(const bool nav = false)
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
    BuildPath(Path_SM, filepath, sizeof(filepath), nav ? "ebot/waypoints/nav/%s.ewp" : "ebot/waypoints/%s.ewp", currentMap);
    
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
    if (fp == null)
        return;
    
    // write the all waypoint data
    fp.Write(header, sizeof(header), 4);

    // save the waypoint paths...
    int i;
    for (i = 0; i < m_waypointNumber; i++)
        fp.Write(m_paths[i], sizeof(WPath), 4);
    
    fp.Close();
    PrintHintTextToAll("%d waypoints saved sucsessfully!", m_waypointNumber);
}

stock void DangerMapSave()
{
    if (m_waypointNumber <= 0)
        return;
    
    char filepath[PLATFORM_MAX_PATH];
    BuildPath(Path_SM, filepath, sizeof(filepath), "ebot/waypoints/data/%s.dm", currentMap);
    File fp = OpenFile(filepath, "wb");
    if (fp == null)
        return;

    int i, j;
    for (j = 0; j < 7; j++)
    {
        for (i = 0; i < m_waypointNumber; i++)
        {
            fp.Write(damageGLOBALred[j][i], 4, 4);
            fp.Write(damageGLOBALblu[j][i], 4, 4);
        }
    }
    
    fp.Close();
}

stock void DangerMapLoad()
{
    if (m_waypointNumber <= 0)
        return;
    
    char filepath[PLATFORM_MAX_PATH];
    BuildPath(Path_SM, filepath, sizeof(filepath), "ebot/waypoints/data/%s.dm", currentMap);
    File fp = OpenFile(filepath, "rb");
    if (fp == null)
    {
        ResetDangerMap(true);
        return;
    }
    
    int i, j;
    for (j = 0; j < 7; j++)
    {
        for (i = 0; i < m_waypointNumber; i++)
        {
            fp.Read(damageGLOBALred[j][i], 4, 4);
            fp.Read(damageGLOBALblu[j][i], 4, 4);
        }
    }

    fp.Close();
}