class PH_AdminPlayerActions
{
    static void SendPrivateMessage(PlayerBase player, string messageText)
    {
        if (!player || messageText == "")
            return;

        player.MessageImportant("[Admin] " + messageText);
    }

    static void KickPlayer(PlayerBase player, string reason)
    {
        if (!player)
            return;

        PlayerIdentity identity = player.GetIdentity();
        if (!identity)
            return;

        if (reason != "")
            SendPrivateMessage(player, reason);

        map<string, string> meta = new map<string, string>();
        meta.Set("reason", reason);
        PH_Service.GetInstance().LogEvent(player, PH_Constants.CAT_SESSION, PH_Constants.EVT_KICK, meta);

        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(DelayedDisconnect, 500, false, identity);
        PH_Log.Info("Admin kick queued: " + identity.GetPlainId());
    }

    static void BanPlayer(PlayerBase player, string reason, int durationMinutes)
    {
        if (!player)
            return;

        PlayerIdentity identity = player.GetIdentity();
        if (!identity)
            return;

        string steamId = identity.GetPlainId();
        PH_BanStore.SaveBan(steamId, reason, durationMinutes);

        map<string, string> meta = new map<string, string>();
        meta.Set("reason", reason);
        meta.Set("durationMinutes", durationMinutes.ToString());
        PH_Service.GetInstance().LogEvent(player, PH_Constants.CAT_SESSION, PH_Constants.EVT_BAN, meta);

        if (reason != "")
            SendPrivateMessage(player, "Banned: " + reason);
        else
            SendPrivateMessage(player, "You have been banned from this server.");

        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(DelayedDisconnect, 500, false, identity);
        PH_Log.Info("Admin ban queued: " + steamId);
    }

    static void DelayedDisconnect(PlayerIdentity identity)
    {
        if (!identity)
            return;

        GetGame().DisconnectPlayer(identity, identity.GetId());
    }
};
