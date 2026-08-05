class PH_AdminCommands
{
    void ProcessPending(array<PlayerBase> players)
    {
        if (!GetGame().IsServer())
            return;

        string root = "$profile:" + PH_Constants.ROOT_FOLDER;
        string commandsDir = root + "\\" + PH_Constants.COMMANDS_FOLDER;
        if (!FileExist(commandsDir))
            MakeDirectory(commandsDir);

        string path = root + "\\" + PH_Constants.COMMANDS_PENDING_FILE;
        if (!FileExist(path))
            return;

        PH_CommandQueue queue = new PH_CommandQueue();
        string err;
        if (!JsonFileLoader<PH_CommandQueue>.LoadFile(path, queue, err))
            return;

        if (!queue.commands || queue.commands.Count() == 0)
            return;

        for (int i = 0; i < queue.commands.Count(); i++)
        {
            PH_CommandEntry cmd = queue.commands.Get(i);
            if (!cmd)
                continue;

            ExecuteCommand(cmd, players);
        }

        DeleteFile(path);
    }

    private void ExecuteCommand(PH_CommandEntry cmd, array<PlayerBase> players)
    {
        if (!cmd || cmd.steamId == "")
            return;

        string type = cmd.type;
        type.ToLower();

        if (type == "deletedeath")
        {
            if (cmd.deathEntryId == "")
                return;

            if (PH_DeathInventoryStore.DeleteEntry(cmd.steamId, cmd.deathEntryId))
                PH_Log.Info("Admin deleted death snapshot " + cmd.deathEntryId + " for " + cmd.steamId);
            return;
        }

        if (type == "deleteinventory")
        {
            if (cmd.deathEntryId == "")
                return;

            if (PH_DeathInventoryStore.DeleteInventorySnapshot(cmd.steamId, cmd.deathEntryId))
                PH_Log.Info("Admin deleted inventory snapshot " + cmd.deathEntryId + " for " + cmd.steamId);
            return;
        }

        if (type == "deleteitem")
        {
            if (cmd.itemPid == "")
                return;

            PlayerBase deleteTarget = FindPlayerBySteamId(cmd.steamId, players);
            if (!deleteTarget)
            {
                PH_PendingDeletionsStore.QueueDeletion(cmd.steamId, cmd.itemPid);
                return;
            }

            if (PH_DeathInventoryStore.DeleteItemByPid(deleteTarget, cmd.itemPid))
                PH_Log.Info("Admin deleted item " + cmd.itemPid + " from " + cmd.steamId);
            else
                PH_Log.Info("Admin delete item failed pid=" + cmd.itemPid + " for " + cmd.steamId);
            return;
        }

        PlayerBase player = FindPlayerBySteamId(cmd.steamId, players);
        if (!player)
        {
            PH_Log.Info("Admin command skipped — player not online: " + cmd.steamId);
            return;
        }

        if (type == "heal")
        {
            player.SetHealth("", "", player.GetMaxHealth("", ""));
            player.SetHealth("GlobalHealth", "Blood", player.GetMaxHealth("GlobalHealth", "Blood"));
            player.SetHealth("GlobalHealth", "Shock", player.GetMaxHealth("GlobalHealth", "Shock"));
            PH_Log.Info("Admin heal: " + cmd.steamId);
            return;
        }

        if (type == "kill")
        {
            player.SetHealth("", "", 0);
            PH_Log.Info("Admin kill: " + cmd.steamId);
            return;
        }

        if (type == "teleport")
        {
            float x = cmd.x;
            float z = cmd.z;
            float y = GetGame().SurfaceY(x, z);
            if (y <= 0)
                y = 0.5;

            vector dest = Vector(x, y + 0.15, z);
            player.SetPosition(dest);
            PH_Log.Info("Admin teleport: " + cmd.steamId + " -> " + x.ToString() + " " + y.ToString() + " " + z.ToString());
            return;
        }

        if (type == "spawn")
        {
            if (cmd.classname == "")
                return;

            int qty = cmd.quantity;
            if (qty <= 0)
                qty = 1;

            for (int i = 0; i < qty; i++)
            {
                EntityAI item = player.GetInventory().CreateInInventory(cmd.classname);
                if (!item)
                {
                    vector spawnPos = player.GetPosition();
                    spawnPos[1] = GetGame().SurfaceY(spawnPos[0], spawnPos[2]) + 0.05;
                    item = EntityAI.Cast(GetGame().CreateObjectEx(cmd.classname, spawnPos, ECE_PLACE_ON_SURFACE));
                }
            }

            PH_Log.Info("Admin spawn " + cmd.classname + " for " + cmd.steamId);
            return;
        }

        if (type == "restoredeath")
        {
            if (cmd.deathEntryId == "")
                return;

            bool forceRestore = cmd.forceRestore != 0;
            string forceSuffix = "";
            if (forceRestore)
                forceSuffix = " (force)";
            if (PH_DeathInventoryStore.RestoreEntryToPlayer(cmd.steamId, cmd.deathEntryId, player, forceRestore))
                PH_Log.Info("Admin restored death snapshot " + cmd.deathEntryId + " for " + cmd.steamId + forceSuffix);
            else
                PH_Log.Info("Admin restore death failed " + cmd.deathEntryId + " for " + cmd.steamId);
            return;
        }

        if (type == "restoreinventory")
        {
            if (cmd.deathEntryId == "")
                return;

            bool invForceRestore = cmd.forceRestore != 0;
            string invForceSuffix = "";
            if (invForceRestore)
                invForceSuffix = " (force)";
            if (PH_DeathInventoryStore.RestoreInventorySnapshotToPlayer(cmd.steamId, cmd.deathEntryId, player, invForceRestore))
                PH_Log.Info("Admin restored inventory snapshot " + cmd.deathEntryId + " for " + cmd.steamId + invForceSuffix);
            else
                PH_Log.Info("Admin restore inventory failed " + cmd.deathEntryId + " for " + cmd.steamId);
            return;
        }

        if (type == "captureinventory")
        {
            if (PH_DeathInventoryStore.CaptureLiveInventory(player))
                PH_Log.Info("Admin captured live inventory for " + cmd.steamId);
            else
                PH_Log.Info("Admin capture inventory failed for " + cmd.steamId);
            return;
        }

        if (type == "message")
        {
            PH_AdminPlayerActions.SendPrivateMessage(player, cmd.message);
            PH_Log.Info("Admin message to " + cmd.steamId);
            return;
        }

        if (type == "kick")
        {
            PH_AdminPlayerActions.KickPlayer(player, cmd.message);
            return;
        }

        if (type == "ban")
        {
            PH_AdminPlayerActions.BanPlayer(player, cmd.message, cmd.banDurationMinutes);
        }
    }

    private PlayerBase FindPlayerBySteamId(string steamId, array<PlayerBase> players)
    {
        foreach (PlayerBase player : players)
        {
            if (!player || !player.GetIdentity())
                continue;

            if (player.GetIdentity().GetPlainId() == steamId)
                return player;
        }

        return null;
    }
};
