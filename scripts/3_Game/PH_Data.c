class PH_PlayerProfile
{
    string steamId;
    string characterName;
    string characterId;
    string firstSeen;
    string lastSeen;
    int totalSessions;
    int totalPlaytimeSeconds;
    int totalDeaths;
    int totalKills;
    bool isOnline;
    string ipAddress;
    string sessionJoinTime;
};

class PH_SessionData
{
    string sessionId;
    string steamId;
    string characterName;
    string joinTime;
    string disconnectTime;
    string disconnectReason;
    int durationSeconds;
    bool active;
    string ipAddress;
};

class PH_OnlinePlayerSnapshot
{
    string steamId;
    string characterName;
    string ipAddress;
    string sessionJoinTime;
    float posX;
    float posY;
    float posZ;
    string lastAction;
    string lastActionTime;
    string lastActionCategory;
    string lastActionItemPid;
    string lastActionItemName;
};

class PH_ServerSnapshotData
{
    string timestamp;
    float serverFps;
    int playerCount;
    int zombieCount;
    int animalCount;
    int aiCount;
    string worldName;
    float worldSize;
    ref array<ref PH_OnlinePlayerSnapshot> onlinePlayers;
};

class PH_CommandEntry
{
    string id;
    string type;
    string steamId;
    string classname;
    string deathEntryId;
    string itemPid;
    string message;
    int banDurationMinutes;
    float x;
    float y;
    float z;
    int quantity;
    int forceRestore;
};

class PH_CommandQueue
{
    ref array<ref PH_CommandEntry> commands;

    void PH_CommandQueue()
    {
        commands = new array<ref PH_CommandEntry>();
    }
};

class PH_PendingDeletionEntry
{
    string itemPid;
    string queuedAt;

    void PH_PendingDeletionEntry()
    {
        itemPid = "";
        queuedAt = "";
    }
};

class PH_PendingDeletionsFile
{
    ref array<ref PH_PendingDeletionEntry> deletions;

    void PH_PendingDeletionsFile()
    {
        deletions = new array<ref PH_PendingDeletionEntry>();
    }
};
