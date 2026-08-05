class PH_ProfileStore
{
    static void UpdateProfile(string steamId, string characterName, string playerFolder, string ipAddress = "")
    {
        string path = playerFolder + "\\profile.json";
        string err;

        PH_PlayerProfile profile = new PH_PlayerProfile();
        if (JsonFileLoader<PH_PlayerProfile>.LoadFile(path, profile, err))
        {
            profile.lastSeen = PH_Timestamp.NowISO();
            profile.characterName = characterName;
            profile.totalSessions = profile.totalSessions + 1;
            profile.isOnline = true;
            profile.sessionJoinTime = PH_Timestamp.NowISO();
            if (ipAddress != "")
                profile.ipAddress = ipAddress;
        }
        else
        {
            profile.steamId = steamId;
            profile.characterName = characterName;
            profile.firstSeen = PH_Timestamp.NowISO();
            profile.lastSeen = profile.firstSeen;
            profile.totalSessions = 1;
            profile.isOnline = true;
            profile.sessionJoinTime = profile.firstSeen;
            profile.ipAddress = ipAddress;
        }

        JsonFileLoader<PH_PlayerProfile>.SaveFile(path, profile, err);
    }

    static void SetOffline(string steamId, string characterName, string playerFolder)
    {
        string path = playerFolder + "\\profile.json";
        string err;

        PH_PlayerProfile profile = new PH_PlayerProfile();
        if (JsonFileLoader<PH_PlayerProfile>.LoadFile(path, profile, err))
        {
            profile.isOnline = false;
            profile.lastSeen = PH_Timestamp.NowISO();
            if (characterName != "")
                profile.characterName = characterName;
        }
        else
        {
            profile.steamId = steamId;
            profile.characterName = characterName;
            profile.firstSeen = PH_Timestamp.NowISO();
            profile.lastSeen = profile.firstSeen;
            profile.totalSessions = 0;
            profile.isOnline = false;
        }

        JsonFileLoader<PH_PlayerProfile>.SaveFile(path, profile, err);
    }

    static void SaveSession(string playerFolder, PH_SessionData session)
    {
        string path = playerFolder + "\\session.json";
        string err;
        JsonFileLoader<PH_SessionData>.SaveFile(path, session, err);
    }
};
