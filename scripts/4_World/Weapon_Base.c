modded class Weapon_Base
{
    override void EEFired(int muzzleType, int mode, string ammoType)
    {
        if (GetGame().IsServer())
        {
            PlayerBase player = PlayerBase.Cast(GetHierarchyRootPlayer());
            if (player && player.GetIdentity())
            {
                map<string, string> meta = new map<string, string>();
                meta.Set("sourceClass", GetType());
                meta.Set("weaponClass", GetType());
                meta.Set("ammo", ammoType);
                meta.Set("fireMode", mode.ToString());
                meta.Set("muzzleType", muzzleType.ToString());
                PH_Service.GetInstance().LogEvent(player, PH_Constants.CAT_COMBAT, PH_Constants.EVT_SHOT_FIRED, meta);
            }
        }

        super.EEFired(muzzleType, mode, ammoType);
    }
}
