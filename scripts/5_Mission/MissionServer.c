modded class MissionServer
{
    override void OnInit()
    {
        super.OnInit();
        PH_Service.GetInstance().Init();
    }

    override void OnEvent(EventType eventTypeId, Param params)
    {
        if (GetGame().IsServer() && eventTypeId == ChatMessageEventTypeID)
        {
            PH_ChatHandler.HandleChatMessage(params);
        }

        if (eventTypeId == ClientNewEventTypeID)
        {
            ClientNewEventParams newParams;
            if (Class.CastTo(newParams, params) && newParams.param1)
            {
                string steamId = newParams.param1.GetPlainId();
                if (PH_BanStore.IsBanned(steamId))
                {
                    string reason = PH_BanStore.GetBanReason(steamId);
                    PH_Log.Info("Blocked banned player join: " + steamId + " reason=" + reason);
                    GetGame().DisconnectPlayer(newParams.param1, newParams.param1.GetId());
                    return;
                }
            }
        }

        super.OnEvent(eventTypeId, params);
    }

    override void OnClientReadyEvent(PlayerIdentity identity, PlayerBase player)
    {
        super.OnClientReadyEvent(identity, player);

        if (!GetGame().IsServer() || !player || !identity)
            return;

        PH_Service.GetInstance().Init();
        PH_Service.GetInstance().RegisterPlayer(player);

        if (PH_Service.GetInstance().GetSessions().GetSessionId(player) == "")
        {
            string sessionId = PH_Service.GetInstance().GetSessions().StartSession(player, PH_Constants.EVT_JOIN);

            map<string, string> meta = new map<string, string>();
            meta.Set("sessionId", sessionId);
            meta.Set("characterName", identity.GetName());
            meta.Set("steamId", identity.GetPlainId());
            meta.Set("source", "OnClientReadyEvent");

            PH_Service.GetInstance().LogEvent(player, PH_Constants.CAT_SESSION, PH_Constants.EVT_JOIN, meta);
            PH_Service.GetInstance().GetTracker().ForcePositionLog(player, PH_Constants.EVT_JOIN);
        }

        GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(PH_PendingDeletionsStore.ProcessForPlayerDelayed, 5000, false, player);
    }

    override PlayerBase OnClientNewEvent(PlayerIdentity identity, vector pos, ParamsReadContext ctx)
    {
        PlayerBase player = super.OnClientNewEvent(identity, pos, ctx);
        return player;
    }

    override void OnClientDisconnectedEvent(PlayerIdentity identity, PlayerBase player, int logoutTime, bool authFailed)
    {
        if (player && GetGame().IsServer())
        {
            string reason = PH_Constants.EVT_DISCONNECT;
            if (authFailed)
                reason = "AuthFailed";

            map<string, string> meta = new map<string, string>();
            meta.Set("reason", reason);
            meta.Set("logoutTime", logoutTime.ToString());
            PH_Service.GetInstance().LogEvent(player, PH_Constants.CAT_SESSION, PH_Constants.EVT_DISCONNECT, meta);
            PH_Service.GetInstance().GetSessions().EndSession(player, reason);
            PH_Service.GetInstance().SetPlayerOffline(player);
            PH_Service.GetInstance().UnregisterPlayer(player);
            PH_Service.GetInstance().FlushAll();
        }

        super.OnClientDisconnectedEvent(identity, player, logoutTime, authFailed);
    }

    override void OnClientRespawnEvent(PlayerIdentity identity, PlayerBase player)
    {
        super.OnClientRespawnEvent(identity, player);

        if (!GetGame().IsServer() || !player || !player.GetIdentity())
            return;

        PH_Service.GetInstance().RegisterPlayer(player);
        string sessionId = PH_Service.GetInstance().GetSessions().StartSession(player, PH_Constants.EVT_RESPAWN);

        map<string, string> meta = new map<string, string>();
        meta.Set("sessionId", sessionId);
        PH_Service.GetInstance().LogEvent(player, PH_Constants.CAT_SESSION, PH_Constants.EVT_RESPAWN, meta);
        PH_Service.GetInstance().GetTracker().ForcePositionLog(player, PH_Constants.EVT_RESPAWN);
    }

    override void OnMissionFinish()
    {
        if (GetGame().IsServer())
            PH_Service.GetInstance().FlushAll();

        super.OnMissionFinish();
    }

    override void OnUpdate(float timeslice)
    {
        super.OnUpdate(timeslice);

        if (GetGame().IsServer())
            PH_Service.GetInstance().RecordServerFrame();
    }
};
