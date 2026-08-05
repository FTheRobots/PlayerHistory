modded class BoatScript
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
};
