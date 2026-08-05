modded class PlayerBase
{
    override void OnConnect()
    {
        super.OnConnect();

        if (!GetGame().IsServer())
            return;

        PH_Service.GetInstance().Init();
        PH_Service.GetInstance().RegisterPlayer(this);

        string sessionId = PH_Service.GetInstance().GetSessions().StartSession(this, PH_Constants.EVT_JOIN);

        map<string, string> meta = new map<string, string>();
        meta.Set("sessionId", sessionId);
        if (GetIdentity())
        {
            meta.Set("characterName", GetIdentity().GetName());
            meta.Set("steamId", GetIdentity().GetPlainId());
        }

        PH_Service.GetInstance().LogEvent(this, PH_Constants.CAT_SESSION, PH_Constants.EVT_JOIN, meta);
        PH_Service.GetInstance().LogPlayerState(this);
        PH_Service.GetInstance().GetTracker().ForcePositionLog(this, PH_Constants.EVT_JOIN);
        PH_Log.Info("Player joined: " + GetIdentity().GetPlainId() + " (" + GetIdentity().GetName() + ")");
    }

    override void OnDisconnect()
    {
        if (GetGame().IsServer() && GetIdentity())
        {
            map<string, string> meta = new map<string, string>();
            meta.Set("reason", PH_Constants.EVT_DISCONNECT);
            PH_Service.GetInstance().LogEvent(this, PH_Constants.CAT_SESSION, PH_Constants.EVT_DISCONNECT, meta);
            PH_Service.GetInstance().GetSessions().EndSession(this, PH_Constants.EVT_DISCONNECT);
            PH_Service.GetInstance().SetPlayerOffline(this);
            PH_Service.GetInstance().UnregisterPlayer(this);
            PH_Service.GetInstance().FlushAll();
        }

        super.OnDisconnect();
    }

    override void OnReconnect()
    {
        super.OnReconnect();

        if (!GetGame().IsServer())
            return;

        PH_Service.GetInstance().RegisterPlayer(this);
        string sessionId = PH_Service.GetInstance().GetSessions().StartSession(this, PH_Constants.EVT_RECONNECT);

        map<string, string> meta = new map<string, string>();
        meta.Set("sessionId", sessionId);
        PH_Service.GetInstance().LogEvent(this, PH_Constants.CAT_SESSION, PH_Constants.EVT_RECONNECT, meta);
        GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(PH_PendingDeletionsStore.ProcessForPlayerDelayed, 5000, false, this);
    }

    override void EEHitBy(TotalDamageResult damageResult, int damageType, EntityAI source, int component, string dmgZone, string ammo, vector modelPos, float speedCoef)
    {
        if (GetGame().IsServer() && GetIdentity() && damageResult)
        {
            float damage = damageResult.GetDamage("", "");
            if (damage > 0)
            {
                map<string, string> meta = new map<string, string>();
                meta.Set("damage", damage.ToString());
                meta.Set("damageZone", dmgZone);
                meta.Set("ammo", ammo);
                meta.Set("damageType", damageType.ToString());
                meta.Set("source", PH_ItemSerializer.GetEntityDescription(source));
                string sourceClass = "Unknown";
                if (source)
                    sourceClass = source.GetType();
                meta.Set("sourceClass", sourceClass);
                meta.Set("healthAfter", GetHealth("", "").ToString());
                meta.Set("bloodAfter", GetHealth("GlobalHealth", "Blood").ToString());

                PH_Service.GetInstance().LogEvent(this, PH_Constants.CAT_COMBAT, PH_Constants.EVT_DAMAGE_RECEIVED, meta);

                PH_ConfigData cfg = PH_Config.GetInstance().Get();
                if (cfg.positionOnDamage != 0)
                    PH_Service.GetInstance().GetTracker().ForcePositionLog(this, PH_Constants.EVT_DAMAGE_RECEIVED);
            }
        }

        super.EEHitBy(damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef);
    }

    override void EEKilled(Object killer)
    {
        if (GetGame().IsServer() && GetIdentity())
        {
            string deathEntryId = PH_DeathInventoryStore.CapturePlayerDeath(this);

            map<string, string> meta = new map<string, string>();
            string killerDesc = "Unknown";

            EntityAI killerEntity = EntityAI.Cast(killer);
            if (killerEntity)
                killerDesc = PH_ItemSerializer.GetEntityDescription(killerEntity);
            else if (killer)
                killerDesc = killer.GetType();

            meta.Set("killer", killerDesc);
            string killerClass = "Unknown";
            if (killer)
                killerClass = killer.GetType();
            meta.Set("killerClass", killerClass);
            meta.Set("position", PH_JsonBuilder.VectorToJson(GetPosition()));

            if (deathEntryId != "")
                meta.Set("deathSnapshotId", deathEntryId);

            PH_Service.GetInstance().LogEvent(this, PH_Constants.CAT_COMBAT, PH_Constants.EVT_PLAYER_DEATH, meta);
            PH_Service.GetInstance().GetTracker().ForcePositionLog(this, PH_Constants.EVT_PLAYER_DEATH);
        }

        super.EEKilled(killer);
    }

    override void EEItemIntoHands(EntityAI item)
    {
        super.EEItemIntoHands(item);

        if (!GetGame().IsServer() || !GetIdentity() || !item)
            return;

        map<string, string> meta = new map<string, string>();
        PH_ItemSerializer.AttachItemMeta(meta, item);
        meta.Set("action", "IntoHands");
        PH_Service.GetInstance().LogEvent(this, PH_Constants.CAT_INVENTORY, PH_Constants.EVT_ITEM_MOVE, meta);
    }

    override void EEItemOutOfHands(EntityAI item)
    {
        super.EEItemOutOfHands(item);

        if (!GetGame().IsServer() || !GetIdentity() || !item)
            return;

        map<string, string> meta = new map<string, string>();
        PH_ItemSerializer.AttachItemMeta(meta, item);
        meta.Set("action", "OutOfHands");
        PH_Service.GetInstance().LogEvent(this, PH_Constants.CAT_INVENTORY, PH_Constants.EVT_ITEM_MOVE, meta);
    }
};
