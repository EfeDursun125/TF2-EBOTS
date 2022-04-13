// from https://github.com/EfeDursun125/CS-EBOT/blob/main/source/waypoint.cpp
// higher version waypoints will be ignored due to updated waypoint system
const int WaypointVersion = 1;
const int MaxWaypoints = 1024;
const int MaxPathIndex = 8;

int nearestIndex = -1;
int savedIndex = -1;
bool showWaypoints = false;
float waypointDrawTime = 0.0;
float nextDrawTime = 0.5;

enum WaypointFlag
{
    WAYPOINT_REDONLY = (1 << 1),
    WAYPOINT_BLUONLY = (1 << 2),
	WAYPOINT_JUMP = (1 << 3),
    WAYPOINT_SNIPER = (1 << 4),
    WAYPOINT_HIDE = (1 << 5),
    WAYPOINT_HIDE_UNTIL_FIRE = (1 << 6),
    WAYPOINT_DANGER = (1 << 7),
    WAYPOINT_CROUCH = (1 << 8),
    WAYPOINT_AMMO = (1 << 9),
    WAYPOINT_HEALTH = (1 << 10),
    WAYPOINT_RESUPPLY = (1 << 11),
    WAYPOINT_DOUBLEJUMP = (1 << 12),
    WAYPOINT_ROCKETJUMP = (1 << 13),
    WAYPOINT_DEMOMANDEFEND = (1 << 14),
    WAYPOINT_SENTRY = (1 << 15),
    WAYPOINT_DISPENSER = (1 << 16),
    WAYPOINT_TELEPORTERENTER = (1 << 17),
    WAYPOINT_TELEPORTEREXIT = (1 << 18),
    WAYPOINT_CAPTUREPOINT = (1 << 19)
};

enum AStarState_t {OPEN, CLOSED, NEW};
enum struct AStar_t
{
    int g;
    int f;
    int parent;
    AStarState_t state;
}
AStar_t astar[MaxWaypoints];

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
	int radius;

    int distance[MaxPathIndex];
    int pathIndex[MaxPathIndex];

    int origin[3];
	int campStart[3];
	int campEnd[3];
}

bool m_hasWaypoints;
int m_waypointNumber;
Path m_paths[MaxWaypoints];

stock void InitializeWaypoints()
{
    for (int i = 0; i < m_waypointNumber; i++)
        WaypointClear(i);
    waypointDrawTime = 0.0;
    nextDrawTime = 0.5;
    m_hasWaypoints = false;
    m_waypointNumber = 0;
    WaypointLoad();
}

stock void DrawWaypoints()
{
    if (!showWaypoints)
        return;
    
    if (!IsValidClient(m_hostEntity))
        FindHostEntity();

    if (waypointDrawTime > GetGameTime())
        return;

    // reset the minimal distance changed before
    float nearestDistance = 999999999.0;
    for (int i = 0; i < m_waypointNumber; i++)
    {
        float distance = GetVectorDistance(VectorAsFloat(m_paths[i].origin), GetOrigin(m_hostEntity), true);
        if (distance > Squared(896))
            continue;

        // check the distance
        if (distance < nearestDistance)
        {
            nearestIndex = i;
            nearestDistance = distance;
        }

        // r, g, b, alpha
        int color[4] = {0, 255, 0, 255};
        int height = 72;

        if (m_paths[i].flags & _:WAYPOINT_CROUCH)
            height = 36;
            
        if (m_paths[i].flags & _:WAYPOINT_REDONLY)
            color[0] = 255;
        else if (m_paths[i].flags & _:WAYPOINT_BLUONLY)
            color[2] = 255;
        else if (m_paths[i].flags & _:WAYPOINT_SNIPER)
        {
            color[1] = 255;
            color[2] = 255;
        }
        else if (m_paths[i].flags & _:WAYPOINT_HIDE)
        {
            color[0] = 255;
            color[1] = 125;
        }
        else if (m_paths[i].flags & _:WAYPOINT_HIDE_UNTIL_FIRE)
        {
            color[0] = 255;
            color[1] = 255;
        }
        else if (m_paths[i].flags & _:WAYPOINT_DANGER)
            color[0] = 255;
        else if (m_paths[i].flags & _:WAYPOINT_JUMP)
        {
            color[0] = 255;
            color[1] = 255;
            color[2] = 255;
        }
        else if (m_paths[i].flags & _:WAYPOINT_HEALTH)
        {
            color[0] = 255;
            color[1] = 255;
            color[2] = 255;
        }

        // convert int to float
        float indexOrigin[3];
        indexOrigin = VectorAsFloat(m_paths[i].origin);
        indexOrigin[2] += float(height);

        TE_SetupBeamPoints(VectorAsFloat(m_paths[i].origin), indexOrigin, m_beamIndex, m_beamIndex, 0, 30, nextDrawTime, 1.75, 1.75, 5, 0.0, color, 20);
        TE_SendToClient(m_hostEntity);
    }

    // draw a paths, camplines and danger directions for nearest waypoint
    if (nearestIndex != -1 && IsVisibleWaypoint(VectorAsInt(GetEyePosition(m_hostEntity)), m_paths[nearestIndex].origin))
    {
        // draw the camplines
        if (m_paths[nearestIndex].flags & _:WAYPOINT_SNIPER)
        {
            int height = 72;
            if (m_paths[nearestIndex].flags & _:WAYPOINT_CROUCH)
                height = 36;

            // convert int to float
            float indexOrigin[3];
            indexOrigin = VectorAsFloat(m_paths[nearestIndex].origin);
            indexOrigin[2] = float(height);

            TE_SetupBeamPoints(indexOrigin, VectorAsFloat(m_paths[nearestIndex].campStart), m_laserIndex, m_laserIndex, 0, 30, nextDrawTime, 1.0, 1.0, 5, 0.0, {255, 0, 0, 255}, 20);
            TE_SendToClient(m_hostEntity);
            TE_SetupBeamPoints(indexOrigin, VectorAsFloat(m_paths[nearestIndex].campEnd), m_laserIndex, m_laserIndex, 0, 30, nextDrawTime, 1.0, 1.0, 5, 0.0, {255, 0, 0, 255}, 20);
            TE_SendToClient(m_hostEntity);
        }

        // draw the connections
        for (int i = 0; i < MaxPathIndex; i++)
        {
            if (m_paths[nearestIndex].pathIndex[i] == -1)
                continue;

            int height = 36;
            if (m_paths[nearestIndex].flags & _:WAYPOINT_CROUCH)
                height = 18;

            float indexOrigin[3];
            indexOrigin = VectorAsFloat(m_paths[nearestIndex].origin);
            indexOrigin[2] = float(m_paths[nearestIndex].origin[2] + height);

            float indexOrigin2[3];
            indexOrigin2 = VectorAsFloat(m_paths[m_paths[nearestIndex].pathIndex[i]].origin);
            indexOrigin2[2] = float(m_paths[m_paths[nearestIndex].pathIndex[i]].origin[2] + height);

            if (IsConnected(m_paths[nearestIndex].pathIndex[i], nearestIndex)) // two way connection
            {
                TE_SetupBeamPoints(indexOrigin, indexOrigin2, m_laserIndex, m_laserIndex, 0, 30, nextDrawTime, 1.0, 1.0, 5, 0.0, {255, 255, 0, 255}, 20);
                TE_SendToClient(m_hostEntity);
            }
            else // one way connection
            {
                TE_SetupBeamPoints(indexOrigin, indexOrigin2, m_laserIndex, m_laserIndex, 0, 30, nextDrawTime, 1.0, 1.0, 5, 0.0, {255, 255, 255, 255}, 20);
                TE_SendToClient(m_hostEntity);
            }
        }

        // now look for oneway incoming connections
        for (int i = 0; i < m_waypointNumber; i++)
        {
            if (IsConnected(m_paths[i].index, m_paths[nearestIndex].index) && !IsConnected(m_paths[nearestIndex].index, m_paths[i].index))
            {
                int height = 36;
                if (m_paths[nearestIndex].flags & _:WAYPOINT_CROUCH)
                    height = 18;

                float indexOrigin[3];
                indexOrigin = VectorAsFloat(m_paths[nearestIndex].origin);
                indexOrigin[2] = float(m_paths[nearestIndex].origin[2] + height);

                float indexOrigin2[3];
                indexOrigin2 = VectorAsFloat(m_paths[i].origin);
                indexOrigin2[2] = float(m_paths[i].origin[2] + height);

                TE_SetupBeamPoints(indexOrigin, indexOrigin2, m_laserIndex, m_laserIndex, 0, 30, nextDrawTime, 1.0, 1.0, 5, 0.0, {0, 192, 96, 255}, 20);
                TE_SendToClient(m_hostEntity);
            }
        }

        // if radius is nonzero, draw a square
        int color[4] = {0, 0, 255, 255};
        int origin[3];
        origin = m_paths[nearestIndex].origin;
        int height = 18;
        if (m_paths[nearestIndex].flags & _:WAYPOINT_CROUCH)
            height = 9;
        origin[2] += height;
        if (m_paths[nearestIndex].radius > 0)
        {
            int root = m_paths[nearestIndex].radius;
            TE_SetupBeamPoints(VectorWithRoot(origin, root, root), VectorWithRoot(origin, -root, root), m_laserIndex, m_laserIndex, 0, 30, nextDrawTime, 1.0, 1.0, 5, 0.0, color, 20);
            TE_SendToClient(m_hostEntity);
            TE_SetupBeamPoints(VectorWithRoot(origin, root, root), VectorWithRoot(origin, root, -root), m_laserIndex, m_laserIndex, 0, 30, nextDrawTime, 1.0, 1.0, 5, 0.0, color, 20);
            TE_SendToClient(m_hostEntity);
            TE_SetupBeamPoints(VectorWithRoot(origin, -root, -root), VectorWithRoot(origin, root, -root), m_laserIndex, m_laserIndex, 0, 30, nextDrawTime, 1.0, 1.0, 5, 0.0, color, 20);
            TE_SendToClient(m_hostEntity);
            TE_SetupBeamPoints(VectorWithRoot(origin, -root, -root), VectorWithRoot(origin, -root, root), m_laserIndex, m_laserIndex, 0, 30, nextDrawTime, 1.0, 1.0, 5, 0.0, color, 20);
            TE_SendToClient(m_hostEntity);
        }
        else
        {
            int root = 8;
            TE_SetupBeamPoints(VectorWithRoot(origin, root, -root), VectorWithRoot(origin, -root, root), m_laserIndex, m_laserIndex, 0, 30, nextDrawTime, 1.0, 1.0, 5, 0.0, color, 20);
            TE_SendToClient(m_hostEntity);
            TE_SetupBeamPoints(VectorWithRoot(origin, -root, -root), VectorWithRoot(origin, root, root), m_laserIndex, m_laserIndex, 0, 30, nextDrawTime, 1.0, 1.0, 5, 0.0, color, 20);
            TE_SendToClient(m_hostEntity);
        }

        int i = nearestIndex;
        char wpflags[128] = "";
        if (m_paths[i].flags & _:WAYPOINT_REDONLY)
            wpflags = " RED_ONLY";
        
        SetHudTextParams(0.0, 0.0, nextDrawTime + 0.02, 255, 255, 255, 255, 2, 1.0, 0.0, 0.0);
        ShowHudText(m_hostEntity, -1, "Nearest Waypoint Data\nIndex: %d (%d/%d)\nActive Area: %d\nFlags:%s\nRadius: %d\nOrigin: %d %d %d\nAim Start Origin: %d %d %d\nAim End Origin: %d %d %d", m_paths[i].index, m_waypointNumber, MaxWaypoints, m_paths[i].activeArea, wpflags, m_paths[i].radius, m_paths[i].origin[0], m_paths[i].origin[1], m_paths[i].origin[2], m_paths[i].campStart[0], m_paths[i].campStart[1], m_paths[i].campStart[2], m_paths[i].campEnd[0], m_paths[i].campEnd[1], m_paths[i].campEnd[2]);
    }

    waypointDrawTime = GetGameTime() + nextDrawTime;
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

stock bool IsWaypointReachable(int src[3], int destination[3])
{
    // cant connect to itself...
    if (src[0] == destination[0] && src[1] == destination[1] && src[2] == destination[2])
        return false;

    // unable to reach with walking
    if ((destination[2] - src[2]) > 72)
        return false;
    
    // is the destination not close enough?
    if (GetVectorDistance(VectorAsFloat(src), VectorAsFloat(destination)) > 400)
        return false;
    
    return IsVisibleWaypoint(src, destination);
}

stock void WaypointAdd(int origin[3], int flags = 0)
{
    if (m_waypointNumber >= MaxWaypoints)
    {
        PrintHintTextToAll("Maximum number of waypoints reached (%d)", MaxWaypoints);
        return;
    }

    int index = m_waypointNumber;
    m_waypointNumber++;
    
    m_paths[index].index = index;
    m_paths[index].activeArea = 0;
    m_paths[index].flags = 0;
    m_paths[index].radius = 0;

    m_paths[index].origin = origin;

    m_paths[index].campStart = {0, 0, 0};
    m_paths[index].campEnd = {0, 0, 0};

    for (int i = 0; i < MaxPathIndex; i++)
    {
        m_paths[index].pathIndex[i] = -1;
        m_paths[index].distance[i] = 0;
    }

    switch(flags)
    {
        case 1:
            m_paths[index].flags = _:WAYPOINT_REDONLY;
        case 2:
            m_paths[index].flags = _:WAYPOINT_BLUONLY;
        case 3:
            m_paths[index].flags = _:WAYPOINT_JUMP;
        case 4:
            m_paths[index].flags = _:WAYPOINT_SNIPER;
        case 5:
            m_paths[index].flags = _:WAYPOINT_HIDE;
        case 6:
            m_paths[index].flags = _:WAYPOINT_HIDE_UNTIL_FIRE;
        case 7:
            m_paths[index].flags = _:WAYPOINT_DANGER;
        case 8:
            m_paths[index].flags = _:WAYPOINT_CROUCH;
    }

    // calculate all the paths to this new waypoint
    for (int i = 0; i < m_waypointNumber; i++)
    {
        // skip the waypoint that was just added
        if (index == i)
            continue; 

        // check if the waypoint is reachable from the new one (one-way)
        if (IsWaypointReachable(m_paths[index].origin, m_paths[i].origin))
            AddPath(index, i, GetVectorDistance(VectorAsFloat(m_paths[index].origin), VectorAsFloat(m_paths[i].origin)));

        // check if the new one is reachable from the waypoint (other way)
        if (IsWaypointReachable(m_paths[i].origin, m_paths[index].origin))
            AddPath(i, index, GetVectorDistance(VectorAsFloat(m_paths[i].origin), VectorAsFloat(m_paths[index].origin)));
    }

    //CalculateWayzone(index); // calculate the wayzone of this waypoint for set the radius
    PrintHintTextToAll("Waypoint added to %d %d %d", m_paths[index].origin[0], m_paths[index].origin[1], m_paths[index].origin[2]);
}

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
            m_paths[srcIndex].distance[i] = RoundFloat(distance);
            PrintHintTextToAll("Path added from %d to %d", srcIndex, destIndex);
            return;
        }
    }

    // there wasn't any free space. try exchanging it with a long-distance path
    int maxDistance = -99999;
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
        m_paths[srcIndex].distance[slotID] = RoundFloat(distance);
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
            m_paths[nodeFrom].distance[index] = 0;
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
            m_paths[nodeFrom].distance[index] = 0;
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
                m_paths[i].distance[j] = 0;
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

stock int HeuristicNormal(int start, int goal)
{
	return GetWaypointDistance(start, goal);
}

stock int CostNormal(int current, int parent, int client)
{
    /*int team = GetClientTeam(client);
    if (m_paths[current].flags & _:WAYPOINT_REDONLY && team == 3)
        return 65355;
    else if (m_paths[current].flags & _:WAYPOINT_BLUONLY)
        return 65355;
    
    if (m_paths[current].flags & _:WAYPOINT_DOUBLEJUMP && TF2_GetPlayerClass(client) != TFClass_Scout)
        return 65355;*/
	return HeuristicNormal(current, parent);
}

// A* Pathfinding
stock void AStarFindPath(int srcIndex, int destIndex, int client)
{
	if (srcIndex == destIndex)
		return;

	if (destIndex == -1)
		return;
	
	if (srcIndex <= 0)
		srcIndex = FindNearestWaypoint(GetOrigin(client));
	
    for (int i = 0; i < m_waypointNumber; i++)
    {
        astar[i].g = 0;
        astar[i].f = 0;
        astar[i].parent = -1;
        astar[i].state = NEW;
    }

    astar[srcIndex].g = CostNormal(srcIndex, -1, client);
    astar[srcIndex].f = astar[srcIndex].g + HeuristicNormal(srcIndex, destIndex);
    astar[srcIndex].state = OPEN;

    ArrayStack openList = new ArrayStack();
    openList.Push(srcIndex);
    while (!openList.Empty)
    {
        // remove the first node from the open list
        int currentIndex = openList.Pop(0);
		
        // is the current node the goal node?
        if (currentIndex == destIndex)
        {
            // build the complete path
            while (currentIndex != -1)
            {
				AddToPath(currentIndex, client);
                currentIndex = astar[currentIndex].parent;
			}
            break;
        }

        if (astar[currentIndex].state != OPEN)
            continue;

        // put current node into CLOSED list
        astar[currentIndex].state = CLOSED;

        // now expand the current node
        for (int i = 0; i < MaxPathIndex; i++)
        {
            int self = m_paths[currentIndex].pathIndex[i];
            if (self == -1)
                continue;

            // calculate the F value as F = G + H
            int g = astar[currentIndex].g + CostNormal(self, currentIndex, client);
            int h = HeuristicNormal(self, destIndex);
            int f = g + h;

            if (astar[self].state == NEW || astar[self].f > f)
            {
                // put the current child into open list
                astar[self].parent = currentIndex;
                astar[self].state = OPEN;
                astar[self].g = g;
                astar[self].f = f;
                openList.Push(self);
            }
        }
    }
}
        
stock void AddToPath(int currentIndex, int client)
{
	m_positions[client].PushArray(VectorAsFloat(m_paths[currentIndex].origin), 3);
	m_pathIndex[client].Push(currentIndex);
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

/*stock void CalculateWayzone(int index)
{
    for (int scanDistance = 8; scanDistance < 144; scanDistance += 8)
    {
        
    }

    if (path->radius < 0)
        path->radius = 0;
}*/

// find the nearest node to that origin, and return the index to this node
stock int FindNearestWaypoint(float origin[3], float maxDistance = 999999.0, int client = -1)
{
    int index = -1;
    maxDistance = maxDistance * maxDistance;
    for (int i = 0; i < m_waypointNumber; i++)
    {
        float distance = GetVectorDistance(VectorAsFloat(m_paths[i].origin), origin, true);
        if (distance < maxDistance)
        {
            index = i;
            maxDistance = distance;
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
    m_paths[index].radius = 0;
    m_paths[index].origin = {0, 0, 0};
    m_paths[index].campStart = {0, 0, 0};
    m_paths[index].campEnd = {0, 0, 0};
    for (int i = 0; i < MaxPathIndex; i++)
    {
        m_paths[index].pathIndex[i] = -1;
        m_paths[index].distance[i] = 0;
    }
}

stock int GetWaypointDistance(int srcIndex, int destIndex)
{
    if (srcIndex == destIndex)
        return 0;
    for (int i = 0; i < MaxPathIndex; i++)
    {
        if (m_paths[srcIndex].pathIndex[i] == destIndex)
            return m_paths[srcIndex].distance[i];
    }
    return RoundToFloor(GetVectorDistance(VectorAsFloat(m_paths[srcIndex].origin), VectorAsFloat(m_paths[destIndex].origin)));
}

stock void WaypointLoad()
{
    char filepath[PLATFORM_MAX_PATH];
    BuildPath(Path_SM, filepath, sizeof(filepath), "ebotWaypoints/%s.ewp", currentMap);
    File fp = OpenFile(filepath, "rb");

    if (fp == null)
    {
        if (NavMesh_Exists())
            PrintHintTextToAll("No waypoint file found\nNavmesh file will be used instead");
        else
            PrintHintTextToAll("No waypoint file found\nNo navmesh file found\nBots will not move");
        return;
    }

    // read about the file and author
    WaypointHeader header;
    fp.Read(header, sizeof(header), 4);
    m_waypointNumber = header.pointNumber;

    if (m_waypointNumber <= 0)
    {
        PrintHintTextToAll("Broken waypoint file\nThis waypoint file will be ignored");
        fp.Close();
        return;
    }

    if (header.fileVersion > WaypointVersion)
    {
        PrintHintTextToAll("Waypoint version is too high\nPlease update your ebot!\nThis waypoint file will be ignored %s", header.fileVersion);
        fp.Close();
        return;
    }

    PrintHintTextToAll("Waypoint file by %s\n%d waypoints loaded", header.author, m_waypointNumber);
    
    // read the all waypoint data
    for (int i = 0; i < m_waypointNumber; i++)
        fp.Read(m_paths[i], sizeof(Path), 4);
    
    fp.Close();
    m_hasWaypoints = true;
    return;
}

stock void WaypointSave()
{
    char waypointAuthor[32] = "unknown";
    if (IsValidClient(m_hostEntity))
        GetClientName(m_hostEntity, waypointAuthor, sizeof(waypointAuthor));

    WaypointHeader header;
    header.fileVersion = WaypointVersion;
    header.pointNumber = m_waypointNumber;
    header.author = waypointAuthor;

    char filepath[PLATFORM_MAX_PATH];
    BuildPath(Path_SM, filepath, sizeof(filepath), "ebotWaypoints/%s.ewp", currentMap);
    File fp = OpenFile(filepath, "wb");

    // write the all waypoint data
    fp.Write(header, sizeof(header), 4);

    // save the waypoint paths...
    for (int i = 0; i < m_waypointNumber; i++)
        fp.Write(m_paths[i], sizeof(Path), 4);
    
    fp.Close();
    PrintHintTextToAll("%d waypoints saved sucsessfully!", m_waypointNumber);
}