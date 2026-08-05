modded class ZombieBase
{
    override void EEKilled(Object killer)
    {
        if (GetGame().IsServer())
        {
            PlayerBase attacker = PH_CombatHelper.ResolvePlayerAttacker(killer);
            if (attacker && attacker.GetIdentity())
            {
                map<string, string> meta = new map<string, string>();
                meta.Set("zombieType", GetType());
                PH_CombatHelper.AttachKillMeta(meta, this, attacker);
                PH_CombatHelper.AttachWeaponMeta(meta, killer);

                string eventType = PH_Constants.EVT_ZOMBIE_KILL;
                string category = PH_Constants.CAT_ZOMBIE;

                if (IsKindOf("InfectedBanditBase"))
                {
                    eventType = PH_Constants.EVT_BANDIT_KILL;
                    category = PH_Constants.CAT_BANDIT;
                    meta.Set("banditType", GetType());
                }

                PH_Service.GetInstance().LogEvent(attacker, category, eventType, meta);
            }
        }

        super.EEKilled(killer);
    }
};
