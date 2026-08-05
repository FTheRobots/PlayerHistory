class PH_AnimalKillHelper
{
    static string ResolveKillEvent(string className)
    {
        if (className == "")
            return PH_Constants.EVT_ANIMAL_KILL;

        if (ContainsToken(className, "Ursus") || ContainsToken(className, "Bear"))
            return PH_Constants.EVT_BEAR_KILL;

        if (ContainsToken(className, "CanisLupus") || ContainsToken(className, "Wolf"))
            return PH_Constants.EVT_WOLF_KILL;

        if (ContainsToken(className, "SusScrofa") || ContainsToken(className, "Boar"))
            return PH_Constants.EVT_BOAR_KILL;

        if (ContainsToken(className, "Cervus") || ContainsToken(className, "Capreolus") || ContainsToken(className, "Rangifer") || ContainsToken(className, "Deer") || ContainsToken(className, "Stag"))
            return PH_Constants.EVT_DEER_KILL;

        if (ContainsToken(className, "Ovis") || ContainsToken(className, "Sheep"))
            return PH_Constants.EVT_SHEEP_KILL;

        if (ContainsToken(className, "Capra") || ContainsToken(className, "Goat"))
            return PH_Constants.EVT_GOAT_KILL;

        if (ContainsToken(className, "Vulpes") || ContainsToken(className, "Fox"))
            return PH_Constants.EVT_FOX_KILL;

        if (ContainsToken(className, "Lepus") || ContainsToken(className, "Hare"))
            return PH_Constants.EVT_HARE_KILL;

        if (ContainsToken(className, "Gallus") || ContainsToken(className, "Hen"))
            return PH_Constants.EVT_HEN_KILL;

        if (ContainsToken(className, "Bos") || ContainsToken(className, "Cow"))
            return PH_Constants.EVT_COW_KILL;

        return PH_Constants.EVT_ANIMAL_KILL;
    }

    static string ResolveAnimalKind(string eventType)
    {
        if (eventType == PH_Constants.EVT_BEAR_KILL)
            return "Bear";
        if (eventType == PH_Constants.EVT_WOLF_KILL)
            return "Wolf";
        if (eventType == PH_Constants.EVT_BOAR_KILL)
            return "Boar";
        if (eventType == PH_Constants.EVT_DEER_KILL)
            return "Deer";
        if (eventType == PH_Constants.EVT_SHEEP_KILL)
            return "Sheep";
        if (eventType == PH_Constants.EVT_GOAT_KILL)
            return "Goat";
        if (eventType == PH_Constants.EVT_FOX_KILL)
            return "Fox";
        if (eventType == PH_Constants.EVT_HARE_KILL)
            return "Hare";
        if (eventType == PH_Constants.EVT_HEN_KILL)
            return "Hen";
        if (eventType == PH_Constants.EVT_COW_KILL)
            return "Cow";

        return "Animal";
    }

    private static bool ContainsToken(string className, string token)
    {
        if (className == "" || token == "")
            return false;

        return className.IndexOf(token) != -1;
    }
};
