class PH_Service
{
    private static ref PH_Service instance;
    private ref PH_EventWriter m_Writer;
    private ref PH_SessionManager m_Sessions;
    private ref PH_PlayerTracker m_Tracker;
    private ref PH_ServerSnapshot m_ServerSnapshot;
    private ref PH_AdminCommands m_AdminCommands;
    private ref array<PlayerBase> m_OnlinePlayers;
    private ref map<string, ref PH_PlayerActionState> m_LastActions;
    private bool m_Initialized;
    private bool m_TickScheduled;
    private int m_TickCounter;
    private int m_ServerFrameTicks;
    private float m_ServerFpsWindowStart;
    private float m_ServerFps;
    private float m_LastSnapshotWriteTime;

    static PH_Service GetInstance()
    {
        if (!instance)
            instance = new PH_Service();
        return instance;
    }

    void PH_Service()
    {
        m_Writer = new PH_EventWriter();
        m_Sessions = new PH_SessionManager();
        m_Tracker = new PH_PlayerTracker();
        m_ServerSnapshot = new PH_ServerSnapshot();
        m_AdminCommands = new PH_AdminCommands();
        m_OnlinePlayers = new array<PlayerBase>();
        m_LastActions = new map<string, ref PH_PlayerActionState>();
        m_TickCounter = 0;
    }

    void Init()
    {
        if (!GetGame().IsServer())
            return;

        if (m_Initialized)
            return;

        m_Initialized = true;
        PH_Config.GetInstance();

        string root = "$profile:" + PH_Constants.ROOT_FOLDER;
        if (!FileExist(root))
            MakeDirectory(root);

        PH_Log.Info("Mod initialized. Log root: " + root);
        m_Writer.ScheduleFlush();
        ScheduleTick();
    }

    PH_SessionManager GetSessions()
    {
        return m_Sessions;
    }

    PH_PlayerTracker GetTracker()
    {
        return m_Tracker;
    }

    void RegisterPlayer(PlayerBase player)
    {
        if (!player || !player.GetIdentity())
            return;

        if (m_OnlinePlayers.Find(player) == -1)
            m_OnlinePlayers.Insert(player);
    }

    void UnregisterPlayer(PlayerBase player)
    {
        if (!player)
            return;

        int idx = m_OnlinePlayers.Find(player);
        if (idx != -1)
            m_OnlinePlayers.Remove(idx);

        if (player.GetIdentity())
            m_Tracker.RemovePlayer(player.GetIdentity().GetPlainId());
    }

    string GetPlayerFolder(string steamId)
    {
        return "$profile:" + PH_Constants.ROOT_FOLDER + "\\" + steamId;
    }

    string GetPlayerEventsPath(string steamId)
    {
        return GetPlayerFolder(steamId) + "\\" + PH_Constants.EVENTS_FOLDER;
    }

    void EnsurePlayerFolder(string steamId)
    {
        string folder = GetPlayerFolder(steamId);
        if (!FileExist(folder))
            MakeDirectory(folder);

        string events = folder + "\\" + PH_Constants.EVENTS_FOLDER;
        if (!FileExist(events))
            MakeDirectory(events);

        string snapshots = folder + "\\" + PH_Constants.SNAPSHOTS_FOLDER;
        if (!FileExist(snapshots))
            MakeDirectory(snapshots);

        string stats = folder + "\\" + PH_Constants.STATISTICS_FOLDER;
        if (!FileExist(stats))
            MakeDirectory(stats);
    }

    void UpdateProfile(PlayerBase player)
    {
        if (!player || !player.GetIdentity())
            return;

        string steamId = player.GetIdentity().GetPlainId();
        string ipAddress = PH_PlayerHelper.GetPlayerIp(player.GetIdentity());
        EnsurePlayerFolder(steamId);
        string folder = GetPlayerFolder(steamId);
        PH_ProfileStore.UpdateProfile(steamId, player.GetIdentity().GetName(), folder, ipAddress);
    }

    void SetPlayerOffline(PlayerBase player)
    {
        if (!player || !player.GetIdentity())
            return;

        string steamId = player.GetIdentity().GetPlainId();
        EnsurePlayerFolder(steamId);
        PH_ProfileStore.SetOffline(steamId, player.GetIdentity().GetName(), GetPlayerFolder(steamId));
    }

    void SaveSessionFile(string steamId, PH_SessionData session)
    {
        EnsurePlayerFolder(steamId);
        PH_ProfileStore.SaveSession(GetPlayerFolder(steamId), session);
    }

    void LogEvent(PlayerBase player, string category, string eventType, map<string, string> metadata)
    {
        if (!GetGame().IsServer())
            return;

        if (!PH_Config.GetInstance().IsCategoryEnabled(category))
            return;

        if (!player || !player.GetIdentity())
            return;

        string steamId = player.GetIdentity().GetPlainId();
        vector pos = player.GetPosition();
        vector orient = player.GetOrientation();
        string sessionId = m_Sessions.GetSessionId(player);

        string json = BuildEventJson(steamId, player.GetIdentity().GetName(), sessionId, eventType, category, pos, orient[1], metadata);

        m_Writer.WriteEvent(steamId, json);
        RecordLastAction(steamId, eventType, category, metadata);

        if (category == PH_Constants.CAT_SESSION)
            return;

        PH_ConfigData cfg = PH_Config.GetInstance().Get();
        if (cfg.positionOnAction != 0 && category != PH_Constants.CAT_POSITION)
            m_Tracker.ForcePositionLog(player, eventType);
    }

    void LogEventForSteamId(string steamId, string playerName, string category, string eventType, vector pos, float orientation, map<string, string> metadata)
    {
        if (!GetGame().IsServer())
            return;

        if (!PH_Config.GetInstance().IsCategoryEnabled(category))
            return;

        string sessionId = m_Sessions.GetSessionIdBySteamId(steamId);
        string json = BuildEventJson(steamId, playerName, sessionId, eventType, category, pos, orientation, metadata);
        m_Writer.WriteEvent(steamId, json);
    }

    string BuildEventJson(string steamId, string playerName, string sessionId, string eventType, string category, vector pos, float orientation, map<string, string> metadata)
    {
        string json = "{";
        json += PH_JsonBuilder.StringField("timestamp", PH_Timestamp.NowISO()) + ",";
        json += PH_JsonBuilder.StringField("steamid", steamId) + ",";
        json += PH_JsonBuilder.StringField("playerName", playerName) + ",";
        json += PH_JsonBuilder.StringField("sessionId", sessionId) + ",";
        json += PH_JsonBuilder.StringField("event", eventType) + ",";
        json += PH_JsonBuilder.StringField("category", category) + ",";
        json += PH_JsonBuilder.VectorField("position", pos) + ",";
        json += PH_JsonBuilder.NumberField("orientation", orientation);

        if (metadata.Count() > 0)
        {
            json += ",\"metadata\":" + PH_JsonBuilder.MetadataObject(metadata);
        }

        json += "}";
        return json;
    }

    void LogPlayerState(PlayerBase player)
    {
        if (!player || !player.GetIdentity())
            return;

        map<string, string> meta = new map<string, string>();
        meta.Set("health", player.GetHealth("", "").ToString());
        meta.Set("blood", player.GetHealth("GlobalHealth", "Blood").ToString());
        meta.Set("shock", player.GetHealth("GlobalHealth", "Shock").ToString());
        meta.Set("energy", player.GetStatEnergy().Get().ToString());
        meta.Set("water", player.GetStatWater().Get().ToString());
        meta.Set("stamina", player.GetStatStamina().Get().ToString());
        meta.Set("wetness", player.GetStatWet().Get().ToString());
        meta.Set("temperature", player.GetStatHeatComfort().Get().ToString());
        meta.Set("alive", player.IsAlive().ToString());
        meta.Set("unconscious", player.IsUnconscious().ToString());
        meta.Set("restrained", player.IsRestrained().ToString());

        LogEvent(player, PH_Constants.CAT_PLAYER_STATE, "PlayerStateSnapshot", meta);
    }

    void LogInventorySnapshot(PlayerBase player)
    {
        if (!player || !player.GetIdentity())
            return;

        PH_ConfigData cfg = PH_Config.GetInstance().Get();
        if (!cfg.logInventorySnapshots)
            return;

        string snapshotId = PH_DeathInventoryStore.CaptureInventorySnapshot(player);

        map<string, string> meta = new map<string, string>();
        array<EntityAI> items = new array<EntityAI>();
        player.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, items);

        string itemList = "[";
        bool first = true;
        for (int i = 0; i < items.Count(); i++)
        {
            EntityAI item = items.Get(i);
            if (!item || item == player)
                continue;

            if (!first)
                itemList += ",";
            first = false;
            itemList += PH_ItemSerializer.SerializeItem(item);
        }
        itemList += "]";

        meta.Set("items", itemList);
        meta.Set("itemCount", items.Count().ToString());

        if (snapshotId != "")
            meta.Set("inventorySnapshotId", snapshotId);

        LogEvent(player, PH_Constants.CAT_INVENTORY, PH_Constants.EVT_INVENTORY_SNAPSHOT, meta);
    }

    void FlushAll()
    {
        m_Writer.FlushAll();
    }

    void RecordServerFrame()
    {
        if (!GetGame().IsServer())
            return;

        m_ServerFrameTicks++;
        float now = GetGame().GetTickTime();

        if (m_ServerFpsWindowStart <= 0)
            m_ServerFpsWindowStart = now;

        float elapsed = now - m_ServerFpsWindowStart;
        if (elapsed >= 1.0)
        {
            m_ServerFps = m_ServerFrameTicks / elapsed;
            m_ServerFrameTicks = 0;
            m_ServerFpsWindowStart = now;
        }
    }

    float GetServerFps()
    {
        if (m_ServerFps > 0)
            return m_ServerFps;

        float avgFps = GetGame().GetAvgFPS(64);
        if (avgFps > 0)
            return avgFps;

        return 0;
    }

    private void ScheduleTick()
    {
        if (m_TickScheduled)
            return;

        m_TickScheduled = true;
        GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(OnTick, 1000, true);
    }

    private void RecordLastAction(string steamId, string eventType, string category, map<string, string> metadata)
    {
        if (category == PH_Constants.CAT_POSITION)
            return;

        PH_PlayerActionState state = new PH_PlayerActionState();
        state.actionTime = PH_Timestamp.NowISO();
        state.actionCategory = category;
        state.actionLabel = BuildActionLabel(eventType, category, metadata);
        state.actionItemPid = "";
        state.actionItemName = "";

        if (metadata)
        {
            string itemPid;
            if (metadata.Find("itemPid", itemPid))
                state.actionItemPid = itemPid;

            string itemName;
            if (metadata.Find("itemDisplayName", itemName))
                state.actionItemName = itemName;
        }

        m_LastActions.Set(steamId, state);
    }

    private string BuildActionLabel(string eventType, string category, map<string, string> metadata)
    {
        string itemName = "";

        if (metadata)
        {
            metadata.Find("itemDisplayName", itemName);

            if (itemName != "")
            {
                if (eventType == PH_Constants.EVT_ITEM_PICKUP)
                    return "Picked up " + itemName;
                if (eventType == PH_Constants.EVT_ITEM_DROP)
                    return "Dropped " + itemName;
                if (eventType == PH_Constants.EVT_ITEM_USE)
                    return "Used " + itemName;
                if (eventType == PH_Constants.EVT_ITEM_ATTACH)
                    return "Attached " + itemName;
                if (eventType == PH_Constants.EVT_ITEM_DETACH)
                    return "Detached " + itemName;
                if (eventType == PH_Constants.EVT_ITEM_MOVE)
                    return itemName + " moved";
            }
        }

        string label = eventType;

        if (metadata)
        {
            string actionClass;
            if (metadata.Find("actionClass", actionClass) && actionClass != "")
                label = actionClass;
            else
            {
                string action;
                if (metadata.Find("action", action) && action != "")
                    label = action;
            }
        }

        return label;
    }

    private void OnTick()
    {
        m_Tracker.OnTick(m_OnlinePlayers);
        m_TickCounter++;

        PH_ConfigData cfg = PH_Config.GetInstance().Get();
        float now = GetGame().GetTickTime();
        float snapshotInterval = cfg.serverSnapshotIntervalSeconds;
        if (snapshotInterval <= 0)
            snapshotInterval = 10.0;

        if (m_LastSnapshotWriteTime <= 0 || (now - m_LastSnapshotWriteTime) >= snapshotInterval)
        {
            m_ServerSnapshot.WriteSnapshot(m_OnlinePlayers, m_LastActions);
            m_LastSnapshotWriteTime = now;
        }

        if (m_TickCounter % 5 == 0)
            m_AdminCommands.ProcessPending(m_OnlinePlayers);
    }
};
