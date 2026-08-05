modded class AnimalBase
{
    override void EEKilled(Object killer)
    {
        if (GetGame().IsServer())
        {
            PlayerBase attacker = PH_CombatHelper.ResolvePlayerAttacker(killer);
            if (attacker && attacker.GetIdentity())
            {
                string className = GetType();
                string eventType = PH_AnimalKillHelper.ResolveKillEvent(className);

                map<string, string> meta = new map<string, string>();
                meta.Set("animalType", className);
                meta.Set("animalKind", PH_AnimalKillHelper.ResolveAnimalKind(eventType));
                PH_CombatHelper.AttachKillMeta(meta, this, attacker);

                PH_Service.GetInstance().LogEvent(attacker, PH_Constants.CAT_ANIMAL, eventType, meta);
            }
        }

        super.EEKilled(killer);
    }
};
