class PH_PlayerTracker
{
    private ref map<string, vector> m_LastPositions;
    private ref map<string, float> m_LastPositionTime;
    private ref map<string, float> m_LastInventorySnapshot;
    private ref map<string, float> m_LastStateSnapshot;

    void PH_PlayerTracker()
    {
        m_LastPositions = new map<string, vector>();
        m_LastPositionTime = new map<string, float>();
        m_LastInventorySnapshot = new map<string, float>();
        m_LastStateSnapshot = new map<string, float>();
    }

    void OnTick(array<PlayerBase> players)
    {
        PH_ConfigData cfg = PH_Config.GetInstance().Get();
        float now = GetGame().GetTickTime();

        foreach (PlayerBase player : players)
        {
            if (!player || !player.GetIdentity() || !player.IsAlive())
                continue;

            string steamId = player.GetIdentity().GetPlainId();
            CheckPositionUpdate(player, steamId, now, cfg);
            CheckStateSnapshot(player, steamId, now, cfg);
            CheckInventorySnapshot(player, steamId, now, cfg);
        }
    }

    void ForcePositionLog(PlayerBase player, string reason)
    {
        if (!player || !player.GetIdentity())
            return;

        PH_ConfigData cfg = PH_Config.GetInstance().Get();
        string steamId = player.GetIdentity().GetPlainId();
        float now = GetGame().GetTickTime();
        float minInterval = cfg.forcedPositionMinIntervalSeconds;
        if (minInterval <= 0)
            minInterval = 2.0;

        bool critical = IsCriticalPositionReason(reason);

        if (!critical)
        {
            float lastTime;
            if (m_LastPositionTime.Find(steamId, lastTime) && (now - lastTime) < minInterval)
                return;
        }

        LogPosition(player, steamId, reason);
        m_LastPositions.Set(steamId, player.GetPosition());
        m_LastPositionTime.Set(steamId, now);
    }

    private void CheckPositionUpdate(PlayerBase player, string steamId, float now, PH_ConfigData cfg)
    {
        if (!cfg.logPosition)
            return;

        vector pos = player.GetPosition();
        vector lastPos;
        float lastTime;

        bool hasLast = m_LastPositions.Find(steamId, lastPos);
        m_LastPositionTime.Find(steamId, lastTime);

        bool shouldLog = false;
        string reason = PH_Constants.EVT_POSITION_UPDATE;

        if (!hasLast)
        {
            shouldLog = true;
        }
        else
        {
            float dist = vector.Distance(pos, lastPos);
            if (dist >= cfg.positionMinDistance)
                shouldLog = true;

            if (hasLast && (now - lastTime) >= cfg.positionIntervalSeconds)
                shouldLog = true;
        }

        if (shouldLog)
        {
            LogPosition(player, steamId, reason);
            m_LastPositions.Set(steamId, pos);
            m_LastPositionTime.Set(steamId, now);
        }
    }

    private void LogPosition(PlayerBase player, string steamId, string reason)
    {
        map<string, string> meta = new map<string, string>();
        if (reason != "")
            meta.Set("reason", reason);

        meta.Set("speed", player.GetSpeed().ToString());

        PH_Service.GetInstance().LogEvent(player, PH_Constants.CAT_POSITION, PH_Constants.EVT_POSITION_UPDATE, meta);
    }

    private void CheckStateSnapshot(PlayerBase player, string steamId, float now, PH_ConfigData cfg)
    {
        if (!cfg.logPlayerState)
            return;

        float lastTime;
        m_LastStateSnapshot.Find(steamId, lastTime);

        if ((now - lastTime) < cfg.playerStateInterval)
            return;

        PH_Service.GetInstance().LogPlayerState(player);
        m_LastStateSnapshot.Set(steamId, now);
    }

    private void CheckInventorySnapshot(PlayerBase player, string steamId, float now, PH_ConfigData cfg)
    {
        if (!cfg.logInventory)
            return;

        float lastTime;
        m_LastInventorySnapshot.Find(steamId, lastTime);

        if ((now - lastTime) < cfg.inventorySnapshotInterval)
            return;

        PH_Service.GetInstance().LogInventorySnapshot(player);
        m_LastInventorySnapshot.Set(steamId, now);
    }

    void RemovePlayer(string steamId)
    {
        m_LastPositions.Remove(steamId);
        m_LastPositionTime.Remove(steamId);
        m_LastInventorySnapshot.Remove(steamId);
        m_LastStateSnapshot.Remove(steamId);
    }

    private bool IsCriticalPositionReason(string reason)
    {
        if (reason == PH_Constants.EVT_JOIN)
            return true;
        if (reason == PH_Constants.EVT_RECONNECT)
            return true;
        if (reason == PH_Constants.EVT_RESPAWN)
            return true;
        if (reason == PH_Constants.EVT_PLAYER_DEATH)
            return true;
        if (reason == PH_Constants.EVT_DISCONNECT)
            return true;

        return false;
    }
};
