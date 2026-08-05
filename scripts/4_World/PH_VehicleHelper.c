class PH_VehicleHelper
{
    static void LogTransportEvent(Transport transport, Human player, string eventType, string seat)
    {
        if (!GetGame().IsServer())
            return;

        PlayerBase pb = PlayerBase.Cast(player);
        if (!pb || !pb.GetIdentity())
            return;

        map<string, string> meta = new map<string, string>();
        meta.Set("vehicle", transport.GetType());
        meta.Set("vehiclePosition", PH_JsonBuilder.VectorToJson(transport.GetPosition()));
        meta.Set("seat", seat);
        meta.Set("speed", (GetVelocity(transport).Length() * 3.6).ToString());

        PH_Service.GetInstance().LogEvent(pb, PH_Constants.CAT_VEHICLE, eventType, meta);
        PH_Service.GetInstance().GetTracker().ForcePositionLog(pb, eventType);
    }
};
