class PH_Log
{
    static void Info(string msg)
    {
        if (!GetGame())
            return;
        Print("[PlayerHistory] " + msg);
    }

    static void Error(string msg)
    {
        if (!GetGame())
            return;
        if (!GetGame().GetMission())
            return;
        Print("[PlayerHistory] ERROR: " + msg);
    }
};
