#ifdef EXPANSIONMODAI
modded class eAIBase
{
    override void EEKilled(Object killer)
    {
        if (GetGame().IsServer())
        {
            PlayerBase attacker = PH_CombatHelper.ResolvePlayerAttacker(killer);
            if (attacker && attacker.GetIdentity())
            {
                map<string, string> meta = new map<string, string>();
                meta.Set("banditType", GetType());
                PH_CombatHelper.AttachKillMeta(meta, this, attacker);
                PH_CombatHelper.AttachWeaponMeta(meta, killer);

                PH_Service.GetInstance().LogEvent(attacker, PH_Constants.CAT_BANDIT, PH_Constants.EVT_BANDIT_KILL, meta);
            }
        }

        super.EEKilled(killer);
    }
};
#endif
