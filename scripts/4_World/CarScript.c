modded class CarScript
{
    override void OnDriverEnter(Human player)
    {
        super.OnDriverEnter(player);
        PH_VehicleHelper.LogTransportEvent(this, player, PH_Constants.EVT_VEHICLE_ENTER, "Driver");
    }

    override void OnDriverExit(Human player)
    {
        PH_VehicleHelper.LogTransportEvent(this, player, PH_Constants.EVT_VEHICLE_EXIT, "Driver");
        super.OnDriverExit(player);
    }

    override void EEHitBy(TotalDamageResult damageResult, int damageType, EntityAI source, int component, string dmgZone, string ammo, vector modelPos, float speedCoef)
    {
        super.EEHitBy(damageResult, damageType, source, component, dmgZone, ammo, modelPos, speedCoef);

        if (!GetGame().IsServer() || !damageResult)
            return;

        float damage = damageResult.GetDamage("", "");
        if (damage <= 0)
            return;

        PlayerBase driver = PlayerBase.Cast(CrewMember(DayZPlayerConstants.VEHICLESEAT_DRIVER));
        if (!driver || !driver.GetIdentity())
        {
            driver = PH_PlayerHelper.GetClosestPlayer(GetPosition(), 10.0);
        }

        if (!driver || !driver.GetIdentity())
            return;

        map<string, string> meta = new map<string, string>();
        meta.Set("vehicle", GetType());
        meta.Set("damage", damage.ToString());
        meta.Set("damageZone", dmgZone);
        meta.Set("source", PH_ItemSerializer.GetEntityDescription(source));
        meta.Set("vehicleHealth", GetHealth("", "").ToString());
        meta.Set("speed", GetSpeedometer().ToString());

        PH_Service.GetInstance().LogEvent(driver, PH_Constants.CAT_VEHICLE, PH_Constants.EVT_VEHICLE_DAMAGE, meta);
    }

    override void OnContact(string zoneName, vector localPos, IEntity other, Contact data)
    {
        super.OnContact(zoneName, localPos, other, data);

        if (!GetGame().IsServer())
            return;

        PlayerBase driver = PlayerBase.Cast(CrewMember(DayZPlayerConstants.VEHICLESEAT_DRIVER));
        if (!driver || !driver.GetIdentity())
            return;

        map<string, string> meta = new map<string, string>();
        meta.Set("vehicle", GetType());
        meta.Set("zone", zoneName);
        meta.Set("speed", GetSpeedometer().ToString());
        meta.Set("impulse", data.Impulse.ToString());

        if (other)
        {
            Object contactObj = Object.Cast(other);
            if (contactObj)
                meta.Set("contactWith", contactObj.GetType());
        }

        PH_Service.GetInstance().LogEvent(driver, PH_Constants.CAT_VEHICLE, PH_Constants.EVT_VEHICLE_CRASH, meta);
    }
};
