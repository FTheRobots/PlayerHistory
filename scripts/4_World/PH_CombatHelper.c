class PH_CombatHelper
{
    static PlayerBase ResolvePlayerAttacker(Object killer)
    {
        if (!killer)
            return null;

        PlayerBase attacker = PlayerBase.Cast(killer);
        if (attacker && attacker.GetIdentity())
            return attacker;

        EntityAI killerEntity = EntityAI.Cast(killer);
        if (!killerEntity)
            return null;

        attacker = PlayerBase.Cast(killerEntity.GetHierarchyRootPlayer());
        if (attacker && attacker.GetIdentity())
            return attacker;

        return null;
    }

    static void AttachKillMeta(map<string, string> meta, EntityAI victim, PlayerBase attacker)
    {
        if (!meta || !victim || !attacker)
            return;

        meta.Set("targetType", victim.GetType());
        meta.Set("distance", vector.Distance(attacker.GetPosition(), victim.GetPosition()).ToString());
        meta.Set("targetPosition", PH_JsonBuilder.VectorToJson(victim.GetPosition()));
    }

    static void AttachWeaponMeta(map<string, string> meta, Object killer)
    {
        if (!meta || !killer)
            return;

        string weaponClass = killer.GetType();
        EntityAI killerEntity = EntityAI.Cast(killer);

        if (killerEntity)
        {
            if (killerEntity.IsInherited(Weapon_Base))
            {
                weaponClass = killerEntity.GetType();
            }
            else
            {
                PlayerBase rootPlayer = PlayerBase.Cast(killerEntity.GetHierarchyRootPlayer());
                if (rootPlayer)
                {
                    EntityAI inHands = rootPlayer.GetHumanInventory().GetEntityInHands();
                    if (inHands)
                        weaponClass = inHands.GetType();
                }
            }
        }

        meta.Set("sourceClass", weaponClass);
        meta.Set("weaponClass", weaponClass);
    }

    static void CopyCombatMeta(map<string, string> dest, map<string, string> src)
    {
        if (!dest || !src)
            return;

        string val;
        if (src.Find("damage", val)) dest.Set("damage", val);
        if (src.Find("damageZone", val)) dest.Set("damageZone", val);
        if (src.Find("ammo", val)) dest.Set("ammo", val);
        if (src.Find("damageType", val)) dest.Set("damageType", val);
        if (src.Find("source", val)) dest.Set("source", val);
        if (src.Find("sourceClass", val)) dest.Set("sourceClass", val);
        if (src.Find("weaponClass", val)) dest.Set("weaponClass", val);
    }
};
