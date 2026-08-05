class PH_PendingDeletionsStore
{
    private static string GetPath(string steamId)
    {
        return "$profile:" + PH_Constants.ROOT_FOLDER + "\\" + steamId + "\\" + PH_Constants.PENDING_DELETIONS_FILE;
    }

    private static bool LoadFile(string steamId, out PH_PendingDeletionsFile file)
    {
        file = new PH_PendingDeletionsFile();
        string path = GetPath(steamId);
        if (!FileExist(path))
            return true;

        string err;
        if (!JsonFileLoader<PH_PendingDeletionsFile>.LoadFile(path, file, err))
        {
            PH_Log.Error("Failed to load pending deletions for " + steamId + ": " + err);
            file = new PH_PendingDeletionsFile();
            return false;
        }

        if (!file.deletions)
            file.deletions = new array<ref PH_PendingDeletionEntry>();

        return true;
    }

    private static bool SaveFile(string steamId, PH_PendingDeletionsFile file)
    {
        if (!file)
            return false;

        PH_Service.GetInstance().EnsurePlayerFolder(steamId);
        string path = GetPath(steamId);

        if (!file.deletions || file.deletions.Count() == 0)
        {
            if (FileExist(path))
                DeleteFile(path);
            return true;
        }

        JsonFileLoader<PH_PendingDeletionsFile>.JsonSaveFile(path, file);
        return true;
    }

    static bool QueueDeletion(string steamId, string itemPid)
    {
        if (steamId == "" || itemPid == "")
            return false;

        PH_PendingDeletionsFile file;
        LoadFile(steamId, file);

        foreach (PH_PendingDeletionEntry existing : file.deletions)
        {
            if (existing && existing.itemPid == itemPid)
                return true;
        }

        PH_PendingDeletionEntry entry = new PH_PendingDeletionEntry();
        entry.itemPid = itemPid;
        entry.queuedAt = PH_Timestamp.NowISO();
        file.deletions.Insert(entry);

        SaveFile(steamId, file);
        PH_Log.Info("Queued item deletion on login pid=" + itemPid + " steam=" + steamId);
        return true;
    }

    static void ProcessForPlayerDelayed(PlayerBase player)
    {
        ProcessForPlayer(player);
    }

    static void ProcessForPlayer(PlayerBase player)
    {
        if (!GetGame().IsServer() || !player || !player.GetIdentity())
            return;

        string steamId = player.GetIdentity().GetPlainId();
        PH_PendingDeletionsFile file;
        if (!LoadFile(steamId, file) || !file.deletions || file.deletions.Count() == 0)
            return;

        foreach (PH_PendingDeletionEntry entry : file.deletions)
        {
            if (!entry || entry.itemPid == "")
                continue;

            if (PH_DeathInventoryStore.DeleteItemByPid(player, entry.itemPid))
            {
                PH_Log.Info("Applied queued item deletion pid=" + entry.itemPid + " steam=" + steamId);

                map<string, string> meta = new map<string, string>();
                meta.Set("itemPid", entry.itemPid);
                meta.Set("queuedAt", entry.queuedAt);
                meta.Set("source", "PendingDeletionOnLogin");
                PH_Service.GetInstance().LogEvent(player, PH_Constants.CAT_INVENTORY, "ItemDeleted", meta);
            }
            else
            {
                PH_Log.Info("Queued item deletion skipped (not on player) pid=" + entry.itemPid + " steam=" + steamId);
            }
        }

        file.deletions.Clear();
        SaveFile(steamId, file);
    }
};
