class PH_ChatHandler
{
    private static string s_LastDedupeKey = "";
    private static float s_LastDedupeTime = 0;

    static void HandleVanillaChatRpc(PlayerBase player, int channel, string message)
    {
        if (!GetGame() || !GetGame().IsServer())
            return;

        if (!player || !player.GetIdentity())
            return;

        message.TrimInPlace();
        if (message == "" || message.IndexOf("#login ") == 0)
            return;

        string chatTab = ResolveChatTab(channel);
        if (chatTab == "")
            return;

        string steamId = player.GetIdentity().GetPlainId();
        string playerName = player.GetIdentity().GetName();

        if (ShouldSkipDuplicate(steamId, message, chatTab))
            return;

        LogChat(player, steamId, playerName, channel, chatTab, message, "vanilla");
    }

    static void HandleChatMessage(Param params)
    {
        if (!GetGame() || !GetGame().IsServer())
            return;

        ChatMessageEventParams chatParams;
        if (!Class.CastTo(chatParams, params))
            return;

        string message = chatParams.param3;
        message.TrimInPlace();
        if (message == "")
            return;

        if (message.IndexOf("#login ") == 0)
            return;

        int channel = chatParams.param1;
        string playerName = chatParams.param2;
        playerName.TrimInPlace();

        string chatTab = ResolveChatTab(channel);
        if (chatTab == "")
            return;

        string steamId = ResolveSteamId(chatParams);
        if (steamId == "")
            steamId = "unknown";

        if (ShouldSkipDuplicate(steamId, message, chatTab))
            return;

        string chatSource = "vanilla";
        #ifdef EXPANSIONMODCHAT
        if (IsExpansionChannel(channel))
            chatSource = "expansion";
        #endif

        PlayerBase player = FindOnlinePlayer(steamId, playerName, chatParams);
        if (player)
        {
            LogChat(player, player.GetIdentity().GetPlainId(), player.GetIdentity().GetName(), channel, chatTab, message, chatSource, chatParams);
            return;
        }

        LogChatForIdentity(steamId, playerName, channel, chatTab, message, chatSource, chatParams);
    }

    private static bool ShouldSkipDuplicate(string steamId, string message, string chatTab)
    {
        string key = steamId + "|" + chatTab + "|" + message;
        float now = GetGame().GetTickTime();
        if (key == s_LastDedupeKey && (now - s_LastDedupeTime) < 1.0)
            return true;

        s_LastDedupeKey = key;
        s_LastDedupeTime = now;
        return false;
    }

    private static void LogChat(PlayerBase player, string steamId, string playerName, int channel, string chatTab, string message, string chatSource, ChatMessageEventParams chatParams = null)
    {
        map<string, string> meta = BuildChatMetadata(channel, chatTab, message, chatSource, chatParams);
        PH_Service.GetInstance().LogEvent(player, PH_Constants.CAT_CHAT, PH_Constants.EVT_CHAT, meta);
    }

    private static void LogChatForIdentity(string steamId, string playerName, int channel, string chatTab, string message, string chatSource, ChatMessageEventParams chatParams = null)
    {
        map<string, string> meta = BuildChatMetadata(channel, chatTab, message, chatSource, chatParams);
        vector pos = "0 0 0";
        PH_Service.GetInstance().LogEventForSteamId(steamId, playerName, PH_Constants.CAT_CHAT, PH_Constants.EVT_CHAT, pos, 0, meta);
    }

    private static map<string, string> BuildChatMetadata(int channel, string chatTab, string message, string chatSource, ChatMessageEventParams chatParams)
    {
        map<string, string> meta = new map<string, string>();
        meta.Set("message", message);
        meta.Set("chatTab", chatTab);
        meta.Set("chatChannel", channel.ToString());
        meta.Set("chatSource", chatSource);

        #ifdef EXPANSIONMODCHAT
        if (chatParams)
        {
            ExpansionChatMessageEventParams expParams = ExpansionChatMessageEventParams.Cast(chatParams);
            if (expParams)
            {
                #ifdef EXPANSIONMODGROUPS
                if (expParams.param6 != "")
                    meta.Set("groupTag", expParams.param6);
                #endif
            }
        }
        #endif

        return meta;
    }

    static string ResolveChatTab(int channel)
    {
        if (channel & CCAdmin)
            return "admin";

        #ifdef EXPANSIONMODCHAT
        if (channel & ExpansionChatChannels.CCTransport)
            return "transport";
        if (channel & ExpansionChatChannels.CCTeam)
            return "team";
        if (channel & ExpansionChatChannels.CCGlobal)
            return "global";
        #endif

        if (channel & CCTransmitter || channel & CCPublicAddressSystem)
            return "transport";

        if (channel & CCDirect || channel == 0 || channel & CCMegaphone)
            return "global";

        if (channel & CCSystem || channel & CCBattlEye)
            return "admin";

        return "global";
    }

    #ifdef EXPANSIONMODCHAT
    static bool IsExpansionChannel(int channel)
    {
        if (channel & ExpansionChatChannels.CCTransport)
            return true;
        if (channel & ExpansionChatChannels.CCGlobal)
            return true;
        if (channel & ExpansionChatChannels.CCTeam)
            return true;
        return false;
    }
    #endif

    static string ResolveSteamId(ChatMessageEventParams chatParams)
    {
        if (chatParams.param2 != "")
        {
            PlayerIdentity identity = GetIdentityById(chatParams.param2);
            if (identity)
                return identity.GetPlainId();

            string byName = ResolveSteamIdByName(chatParams.param2);
            if (byName != "")
                return byName;
        }

        #ifdef EXPANSIONMODCHAT
        ExpansionChatMessageEventParams expParams = ExpansionChatMessageEventParams.Cast(chatParams);
        if (expParams && expParams.param5 != "")
        {
            PlayerIdentity expIdentity = GetIdentityById(expParams.param5);
            if (expIdentity)
                return expIdentity.GetPlainId();
        }
        #endif

        return "";
    }

    static PlayerIdentity GetIdentityById(string id)
    {
        if (id == "")
            return null;

        array<Man> players = new array<Man>();
        GetGame().GetPlayers(players);

        for (int i = 0; i < players.Count(); i++)
        {
            PlayerBase pb = PlayerBase.Cast(players.Get(i));
            if (!pb || !pb.GetIdentity())
                continue;

            if (pb.GetIdentity().GetId() == id)
                return pb.GetIdentity();
        }

        return null;
    }

    static string ResolveSteamIdByName(string playerName)
    {
        if (playerName == "")
            return "";

        string needle = playerName;
        needle.ToLower();

        array<Man> players = new array<Man>();
        GetGame().GetPlayers(players);

        for (int i = 0; i < players.Count(); i++)
        {
            PlayerBase pb = PlayerBase.Cast(players.Get(i));
            if (!pb || !pb.GetIdentity())
                continue;

            string candidate = pb.GetIdentity().GetName();
            candidate.ToLower();
            if (candidate == needle)
                return pb.GetIdentity().GetPlainId();
        }

        return "";
    }

    static PlayerBase FindOnlinePlayer(string steamId, string playerName, ChatMessageEventParams chatParams = null)
    {
        array<Man> players = new array<Man>();
        GetGame().GetPlayers(players);

        for (int i = 0; i < players.Count(); i++)
        {
            PlayerBase pb = PlayerBase.Cast(players.Get(i));
            if (!pb || !pb.GetIdentity())
                continue;

            if (steamId != "" && steamId != "unknown" && pb.GetIdentity().GetPlainId() == steamId)
                return pb;

            if (playerName != "")
            {
                if (pb.GetIdentity().GetId() == playerName)
                    return pb;

                string candidate = pb.GetIdentity().GetName();
                candidate.ToLower();
                string needle = playerName;
                needle.ToLower();
                if (candidate == needle)
                    return pb;
            }
        }

        return null;
    }
};
