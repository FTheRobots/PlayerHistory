modded class ItemBase
{
    override void EEItemLocationChanged(notnull InventoryLocation oldLoc, notnull InventoryLocation newLoc)
    {
        super.EEItemLocationChanged(oldLoc, newLoc);

        if (!GetGame().IsServer())
            return;

        if (PH_ShouldSkipInventoryLocationLog(oldLoc, newLoc))
            return;

        PlayerBase player = PH_FindAttributedPlayer(oldLoc, newLoc);
        if (!player || !player.GetIdentity())
            return;

        string eventType = PH_Constants.EVT_ITEM_MOVE;
        string oldLocName = PH_ItemSerializer.GetInventoryLocationName(oldLoc);
        string newLocName = PH_ItemSerializer.GetInventoryLocationName(newLoc);

        EntityAI oldParent = oldLoc.GetParent();
        EntityAI newParent = newLoc.GetParent();

        if (!oldParent && newParent && newParent.IsInherited(PlayerBase))
            eventType = PH_Constants.EVT_ITEM_PICKUP;
        else if (oldParent && oldParent.IsInherited(PlayerBase) && !newParent)
            eventType = PH_Constants.EVT_ITEM_DROP;
        else if (!oldParent)
            oldLocName = "Ground";
        if (!newParent)
            newLocName = "Ground";

        map<string, string> meta = new map<string, string>();
        PH_ItemSerializer.AttachItemMeta(meta, this);
        meta.Set("from", oldLocName);
        meta.Set("to", newLocName);
        meta.Set("fromEntity", PH_ItemSerializer.GetEntityDescription(oldParent));
        meta.Set("toEntity", PH_ItemSerializer.GetEntityDescription(newParent));
        PH_ItemSerializer.AttachContainerEntityMeta(meta, "from", oldParent);
        PH_ItemSerializer.AttachContainerEntityMeta(meta, "to", newParent);

        PH_Service.GetInstance().LogEvent(player, PH_Constants.CAT_INVENTORY, eventType, meta);

        PH_ConfigData cfg = PH_Config.GetInstance().Get();
        if (cfg.positionOnInventory != 0)
            PH_Service.GetInstance().GetTracker().ForcePositionLog(player, eventType);
    }

    private bool PH_ShouldSkipInventoryLocationLog(notnull InventoryLocation oldLoc, notnull InventoryLocation newLoc)
    {
        EntityAI oldParent = oldLoc.GetParent();
        EntityAI newParent = newLoc.GetParent();

        if (oldParent == newParent && oldParent)
            return true;

        EntityAI weaponParent = oldParent;
        if (!weaponParent)
            weaponParent = newParent;

        if (weaponParent && weaponParent.IsInherited(Weapon_Base))
        {
            if (IsInherited(Ammunition_Base) || IsInherited(Magazine))
                return true;
        }

        return false;
    }

    private PlayerBase PH_FindAttributedPlayer(notnull InventoryLocation oldLoc, notnull InventoryLocation newLoc)
    {
        EntityAI oldOwner = oldLoc.GetParent();
        EntityAI newOwner = newLoc.GetParent();

        if (newOwner && newOwner.IsInherited(PlayerBase))
            return PlayerBase.Cast(newOwner);

        if (oldOwner && oldOwner.IsInherited(PlayerBase))
            return PlayerBase.Cast(oldOwner);

        vector searchPos = vector.Zero;
        if (newOwner)
            searchPos = newOwner.GetPosition();
        else if (oldOwner)
            searchPos = oldOwner.GetPosition();

        if (searchPos == vector.Zero)
            return null;

        return PH_PlayerHelper.GetClosestPlayer(searchPos, 5.0);
    }
};
