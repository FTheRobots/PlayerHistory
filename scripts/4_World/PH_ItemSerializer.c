class PH_ItemSerializer
{
    static string GetDisplayName(EntityAI item)
    {
        if (!item)
            return "Unknown";

        string name = item.GetDisplayName();
        if (name == "")
            name = item.GetType();
        return name;
    }

    static string GetPersistentIdString(EntityAI item)
    {
        if (!item)
            return "";

        int b1, b2, b3, b4;
        item.GetPersistentID(b1, b2, b3, b4);

        if (b1 == 0 && b2 == 0 && b3 == 0 && b4 == 0)
            return "";

        return b1.ToString() + "-" + b2.ToString() + "-" + b3.ToString() + "-" + b4.ToString();
    }

    static void AttachItemMeta(map<string, string> meta, EntityAI item)
    {
        if (!item || !meta)
            return;

        meta.Set("item", SerializeItem(item));

        string pid = GetPersistentIdString(item);
        if (pid != "")
            meta.Set("itemPid", pid);

        meta.Set("itemDisplayName", GetDisplayName(item));
    }

    static string SerializeItem(EntityAI item)
    {
        if (!item)
            return "{}";

        string type = item.GetType();
        string displayName = GetDisplayName(item);
        float health = item.GetHealth("", "");
        float quantity = 1;

        ItemBase ib = ItemBase.Cast(item);
        if (ib && ib.HasQuantity())
            quantity = ib.GetQuantity();

        string result = "{";
        result += PH_JsonBuilder.StringField("classname", type) + ",";
        result += PH_JsonBuilder.StringField("displayName", displayName) + ",";
        result += PH_JsonBuilder.NumberField("health", health) + ",";
        result += PH_JsonBuilder.NumberField("quantity", quantity);

        if (ib)
        {
            result += "," + PH_JsonBuilder.NumberField("temperature", ib.GetTemperature());
            result += "," + PH_JsonBuilder.NumberField("wetness", ib.GetWet());
        }

        string pid = GetPersistentIdString(item);
        if (pid != "")
            result += "," + PH_JsonBuilder.StringField("pid", pid);

        result += "}";
        return result;
    }

    static string GetInventoryLocationName(notnull InventoryLocation loc)
    {
        EntityAI parent = loc.GetParent();
        if (!parent)
            return "Ground";

        if (parent.IsInherited(PlayerBase))
        {
            int slot = loc.GetSlot();
            if (slot != InventorySlots.INVALID)
                return InventorySlots.GetSlotName(slot);
            return "PlayerInventory";
        }

        return parent.GetType();
    }

    static string GetEntityDescription(EntityAI entity)
    {
        if (!entity)
            return "None";

        if (entity.IsInherited(PlayerBase))
        {
            PlayerBase pb = PlayerBase.Cast(entity);
            if (pb.GetIdentity())
                return "Player:" + pb.GetIdentity().GetName();
            return "Player";
        }

        return entity.GetType();
    }

    static void AttachContainerEntityMeta(map<string, string> meta, string sidePrefix, EntityAI entity)
    {
        if (!entity || !meta || entity.IsInherited(PlayerBase))
            return;

        meta.Set(sidePrefix + "ContainerPosition", PH_JsonBuilder.VectorToJson(entity.GetPosition()));

        string pid = GetPersistentIdString(entity);
        if (pid != "")
            meta.Set(sidePrefix + "ContainerPid", pid);
    }
};
