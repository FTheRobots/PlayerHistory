class PH_SessionManager
{
    private ref map<string, ref PH_SessionData> m_ActiveSessions;
    private ref map<PlayerBase, string> m_PlayerToSession;

    void PH_SessionManager()
    {
        m_ActiveSessions = new map<string, ref PH_SessionData>();
        m_PlayerToSession = new map<PlayerBase, string>();
    }

    string StartSession(PlayerBase player, string reason)
    {
        if (!player || !player.GetIdentity())
            return "";

        string steamId = player.GetIdentity().GetPlainId();
        string sessionId = PH_Timestamp.GenerateSessionId();

        PH_SessionData session = new PH_SessionData();
        session.sessionId = sessionId;
        session.steamId = steamId;
        session.characterName = player.GetIdentity().GetName();
        session.joinTime = PH_Timestamp.NowISO();
        session.active = true;
        session.disconnectReason = reason;
        session.ipAddress = PH_PlayerHelper.GetPlayerIp(player.GetIdentity());

        m_ActiveSessions.Set(steamId, session);
        m_PlayerToSession.Set(player, sessionId);

        PH_Service.GetInstance().EnsurePlayerFolder(steamId);
        PH_Service.GetInstance().UpdateProfile(player);
        PH_Service.GetInstance().SaveSessionFile(steamId, session);

        return sessionId;
    }

    void EndSession(PlayerBase player, string reason)
    {
        if (!player || !player.GetIdentity())
            return;

        string steamId = player.GetIdentity().GetPlainId();
        PH_SessionData session;

        if (m_ActiveSessions.Find(steamId, session) && session)
        {
            session.disconnectTime = PH_Timestamp.NowISO();
            session.disconnectReason = reason;
            session.active = false;
            PH_Service.GetInstance().SaveSessionFile(steamId, session);
            m_ActiveSessions.Remove(steamId);
        }

        m_PlayerToSession.Remove(player);
    }

    string GetSessionId(PlayerBase player)
    {
        if (!player)
            return "";

        string sessionId;
        if (m_PlayerToSession.Find(player, sessionId))
            return sessionId;
        return "";
    }

    string GetSessionIdBySteamId(string steamId)
    {
        PH_SessionData session;
        if (m_ActiveSessions.Find(steamId, session) && session)
            return session.sessionId;
        return "";
    }

    PH_SessionData GetActiveSession(string steamId)
    {
        PH_SessionData session;
        m_ActiveSessions.Find(steamId, session);
        return session;
    }
};
