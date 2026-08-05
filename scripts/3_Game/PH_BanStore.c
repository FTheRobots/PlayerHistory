class PH_BanEntry
{
    string steamId;
    string reason;
    string bannedAt;
    string expiresAt;

    void PH_BanEntry()
    {
        steamId = "";
        reason = "";
        bannedAt = "";
        expiresAt = "";
    }
}

class PH_BanStore
{
    private static const string BANS_DIR = "$profile:" + PH_Constants.ROOT_FOLDER + "\\bans\\";

    static void EnsureDirectory()
    {
        string root = "$profile:" + PH_Constants.ROOT_FOLDER;
        if (!FileExist(root))
            MakeDirectory(root);
        if (!FileExist(BANS_DIR))
            MakeDirectory(BANS_DIR);
    }

    static string GetBanPath(string steamId)
    {
        return BANS_DIR + steamId + ".json";
    }

    static bool IsBanned(string steamId)
    {
        if (steamId == "")
            return false;

        PH_BanEntry entry = LoadBan(steamId);
        if (!entry)
            return false;

        if (entry.expiresAt == "")
            return true;

        if (IsExpired(entry.expiresAt))
        {
            DeleteBan(steamId);
            return false;
        }

        return true;
    }

    static string GetBanReason(string steamId)
    {
        PH_BanEntry entry = LoadBan(steamId);
        if (!entry)
            return "Banned";

        if (entry.reason != "")
            return entry.reason;

        return "Banned";
    }

    static void SaveBan(string steamId, string reason, int durationMinutes)
    {
        if (steamId == "")
            return;

        EnsureDirectory();

        PH_BanEntry entry = new PH_BanEntry();
        entry.steamId = steamId;
        entry.reason = reason;
        entry.bannedAt = PH_Timestamp.NowISO();

        if (durationMinutes > 0)
            entry.expiresAt = AddMinutesIso(entry.bannedAt, durationMinutes);
        else
            entry.expiresAt = "";

        string path = GetBanPath(steamId);
        string err;
        JsonFileLoader<PH_BanEntry>.SaveFile(path, entry, err);
    }

    static void DeleteBan(string steamId)
    {
        string path = GetBanPath(steamId);
        if (FileExist(path))
            DeleteFile(path);
    }

    static PH_BanEntry LoadBan(string steamId)
    {
        string path = GetBanPath(steamId);
        if (!FileExist(path))
            return null;

        PH_BanEntry entry = new PH_BanEntry();
        string err;
        if (!JsonFileLoader<PH_BanEntry>.LoadFile(path, entry, err))
            return null;

        return entry;
    }

    private static bool IsExpired(string expiresAtIso)
    {
        if (expiresAtIso == "")
            return false;

        return expiresAtIso < PH_Timestamp.NowISO();
    }

    private static string AddMinutesIso(string iso, int minutes)
    {
        int year, month, day, hour, minute, second;
        ParseIsoUtc(iso, year, month, day, hour, minute, second);

        int totalMinutes = hour * 60 + minute + minutes;
        day += totalMinutes / (24 * 60);
        totalMinutes = totalMinutes % (24 * 60);
        if (totalMinutes < 0)
        {
            totalMinutes += 24 * 60;
            day -= 1;
        }
        hour = totalMinutes / 60;
        minute = totalMinutes % 60;

        return FormatIsoUtc(year, month, day, hour, minute, second);
    }

    private static void ParseIsoUtc(string iso, out int year, out int month, out int day, out int hour, out int minute, out int second)
    {
        year = 1970;
        month = 1;
        day = 1;
        hour = 0;
        minute = 0;
        second = 0;

        if (iso.Length() < 19)
            return;

        year = iso.Substring(0, 4).ToInt();
        month = iso.Substring(5, 2).ToInt();
        day = iso.Substring(8, 2).ToInt();
        hour = iso.Substring(11, 2).ToInt();
        minute = iso.Substring(14, 2).ToInt();
        second = iso.Substring(17, 2).ToInt();
    }

    private static string FormatIsoUtc(int year, int month, int day, int hour, int minute, int second)
    {
        return year.ToStringLen(4) + "-" + month.ToStringLen(2) + "-" + day.ToStringLen(2) + "T" + hour.ToStringLen(2) + ":" + minute.ToStringLen(2) + ":" + second.ToStringLen(2) + ".000Z";
    }
};
