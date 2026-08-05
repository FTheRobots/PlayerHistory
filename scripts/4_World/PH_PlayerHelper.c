class PH_PlayerHelper
{
    static PlayerBase GetClosestPlayer(vector pos, float maxDist)
    {
        array<Man> players = new array<Man>();
        GetGame().GetPlayers(players);

        PlayerBase closest;
        float closestDist = maxDist;

        for (int i = 0; i < players.Count(); i++)
        {
            PlayerBase pb = PlayerBase.Cast(players.Get(i));
            if (!pb || !pb.GetIdentity())
                continue;

            float dist = vector.Distance(pos, pb.GetPosition());
            if (dist < closestDist)
            {
                closestDist = dist;
                closest = pb;
            }
        }

        return closest;
    }

    static string GetPlayerIp(PlayerIdentity identity)
    {
        if (!identity)
            return "";

        return "";
    }
};
