class PH_ServerSnapshot
{
    private int m_ZombieCount;
    private int m_AnimalCount;
    private int m_HumanAiCount;
    private float m_LastEntityScanTime;
    private const float ENTITY_SCAN_INTERVAL = 30.0;

    void PH_ServerSnapshot()
    {
        m_ZombieCount = 0;
        m_AnimalCount = 0;
        m_HumanAiCount = 0;
        m_LastEntityScanTime = 0;
    }

    void WriteSnapshot(array<PlayerBase> players, map<string, ref PH_PlayerActionState> lastActions)
    {
        if (!GetGame().IsServer())
            return;

        float now = GetGame().GetTickTime();
        if (now - m_LastEntityScanTime >= ENTITY_SCAN_INTERVAL)
        {
            ScanEntityCounts();
            m_LastEntityScanTime = now;
        }

        PH_ServerSnapshotData snapshot = new PH_ServerSnapshotData();
        snapshot.timestamp = PH_Timestamp.NowISO();
        snapshot.serverFps = PH_Service.GetInstance().GetServerFps();
        snapshot.playerCount = players.Count();
        snapshot.zombieCount = m_ZombieCount;
        snapshot.animalCount = m_AnimalCount;
        snapshot.aiCount = m_HumanAiCount;
        snapshot.worldName = GetGame().GetWorldName();
        snapshot.worldSize = GetGame().GetWorld().GetWorldSize();
        PH_ConfigData cfg = PH_Config.GetInstance().Get();
        snapshot.snapshotIntervalSeconds = cfg.serverSnapshotIntervalSeconds;
        if (snapshot.snapshotIntervalSeconds <= 0)
            snapshot.snapshotIntervalSeconds = 10.0;
        snapshot.onlinePlayers = new array<ref PH_OnlinePlayerSnapshot>();

        foreach (PlayerBase player : players)
        {
            if (!player || !player.GetIdentity())
                continue;

            PH_OnlinePlayerSnapshot entry = BuildPlayerEntry(player, lastActions);
            if (entry)
                snapshot.onlinePlayers.Insert(entry);
        }

        SaveSnapshot(snapshot);
    }

    private PH_OnlinePlayerSnapshot BuildPlayerEntry(PlayerBase player, map<string, ref PH_PlayerActionState> lastActions)
    {
        PlayerIdentity identity = player.GetIdentity();
        if (!identity)
            return null;

        string steamId = identity.GetPlainId();
        PH_OnlinePlayerSnapshot entry = new PH_OnlinePlayerSnapshot();
        entry.steamId = steamId;
        entry.characterName = identity.GetName();

        PH_SessionData session = PH_Service.GetInstance().GetSessions().GetActiveSession(steamId);
        if (session)
        {
            entry.sessionJoinTime = session.joinTime;
            entry.ipAddress = session.ipAddress;
        }

        vector pos = player.GetPosition();
        entry.posX = pos[0];
        entry.posY = pos[1];
        entry.posZ = pos[2];

        PH_PlayerActionState actionState;
        if (lastActions && lastActions.Find(steamId, actionState) && actionState)
        {
            entry.lastAction = actionState.actionLabel;
            entry.lastActionTime = actionState.actionTime;
            entry.lastActionCategory = actionState.actionCategory;
            entry.lastActionItemPid = actionState.actionItemPid;
            entry.lastActionItemName = actionState.actionItemName;
        }

        return entry;
    }

    private void ScanEntityCounts()
    {
        PH_ConfigData cfg = PH_Config.GetInstance().Get();
        if (cfg.scanEntityCounts == 0)
            return;

        m_ZombieCount = 0;
        m_AnimalCount = 0;
        m_HumanAiCount = 0;

        array<EntityAI> entities = new array<EntityAI>();
        PH_EntityQuery.GetEntitiesInWorldBox(entities, QueryFlags.DYNAMIC);

        for (int i = 0; i < entities.Count(); i++)
        {
            EntityAI ent = entities.Get(i);
            if (!ent)
                continue;

            if (IsHumanAiEntity(ent))
            {
                m_HumanAiCount++;
                continue;
            }

            if (ent.IsInherited(ZombieBase))
                m_ZombieCount++;
            else if (ent.IsInherited(AnimalBase))
                m_AnimalCount++;
        }
    }

    private bool IsHumanAiEntity(EntityAI ent)
    {
        if (ent.IsKindOf("eAIBase"))
            return true;

        if (ent.IsKindOf("InfectedBanditBase"))
            return true;

        return false;
    }

    private void SaveSnapshot(PH_ServerSnapshotData snapshot)
    {
        string root = "$profile:" + PH_Constants.ROOT_FOLDER;
        if (!FileExist(root))
            MakeDirectory(root);

        string path = root + "\\" + PH_Constants.SERVER_SNAPSHOT_FILE;
        string err;
        JsonFileLoader<PH_ServerSnapshotData>.SaveFile(path, snapshot, err);
    }
};

class PH_PlayerActionState
{
    string actionLabel;
    string actionTime;
    string actionCategory;
    string actionItemPid;
    string actionItemName;
};
