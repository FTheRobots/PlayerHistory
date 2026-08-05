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
};
