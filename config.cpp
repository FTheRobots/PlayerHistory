class CfgPatches {
    class PlayerHistory {
        units[] = {};
        weapons[] = {};
        requiredVersion = 0.1;
        requiredAddons[] = { "DZ_Data", "DZ_Scripts" };
    };
};

class CfgMods {
    class PlayerHistory {
        dir = "PlayerHistory";
        picture = "";
        action = "";
        hideName = 0;
        hidePicture = 0;
        name = "Player History Recorder";
        credits = "RubeK";
        author = "RubeK";
        authorID = "0";
        version = "1.0.0";
        extra = 0;
        type = "mod";
        dependencies[] = { "Game", "World", "Mission" };
        class defs {
            class gameScriptModule {
                value = "";
                files[] = { "PlayerHistory/scripts/3_Game" };
            };
            class worldScriptModule {
                value = "";
                files[] = { "PlayerHistory/scripts/4_World" };
            };
            class missionScriptModule {
                value = "";
                files[] = { "PlayerHistory/scripts/5_Mission" };
            };
        };
    };
};
