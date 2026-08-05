class PH_Constants
{
    static const string ROOT_FOLDER = "PlayerHistory";
    static const string EVENTS_FOLDER = "events";
    static const string SNAPSHOTS_FOLDER = "snapshots";
    static const string STATISTICS_FOLDER = "statistics";
    static const string COMMANDS_FOLDER = "commands";
    static const string DEATHS_FOLDER = "deaths";
    static const string INVENTORY_SNAPSHOTS_FOLDER = "inventory_snapshots";
    static const string LIVE_INVENTORY_FILE = "live_inventory.json";
    static const string PENDING_DELETIONS_FILE = "pending_deletions.json";
    static const string SERVER_SNAPSHOT_FILE = "server.json";
    static const string COMMANDS_PENDING_FILE = "commands\\pending.json";

    static const string CAT_SESSION = "Session";
    static const string CAT_POSITION = "Position";
    static const string CAT_INVENTORY = "Inventory";
    static const string CAT_COMBAT = "Combat";
    static const string CAT_VEHICLE = "Vehicle";
    static const string CAT_ACTION = "Action";
    static const string CAT_WORLD = "World";
    static const string CAT_ZOMBIE = "Zombie";
    static const string CAT_ANIMAL = "Animal";
    static const string CAT_BANDIT = "Bandit";
    static const string CAT_BASEBUILDING = "BaseBuilding";
    static const string CAT_PLAYER_STATE = "PlayerState";
    static const string CAT_CHAT = "Chat";

    static const string EVT_CHAT = "ChatMessage";

    static const int PH_RPC_CHAT_LOG = 7847291;
    static const string EVT_JOIN = "Join";
    static const string EVT_DISCONNECT = "Disconnect";
    static const string EVT_RECONNECT = "Reconnect";
    static const string EVT_RESPAWN = "Respawn";
    static const string EVT_KICK = "Kick";
    static const string EVT_BAN = "Ban";
    static const string EVT_TIMEOUT = "Timeout";
    static const string EVT_SERVER_RESTART = "ServerRestart";

    static const string EVT_POSITION_UPDATE = "PositionUpdate";
    static const string EVT_POSITION_SNAPSHOT = "PositionSnapshot";

    static const string EVT_ITEM_PICKUP = "ItemPickup";
    static const string EVT_ITEM_DROP = "ItemDrop";
    static const string EVT_ITEM_MOVE = "ItemMove";
    static const string EVT_ITEM_USE = "ItemUse";
    static const string EVT_ITEM_ATTACH = "ItemAttach";
    static const string EVT_ITEM_DETACH = "ItemDetach";
    static const string EVT_CONTAINER_OPEN = "ContainerOpen";
    static const string EVT_CONTAINER_CLOSE = "ContainerClose";
    static const string EVT_INVENTORY_SNAPSHOT = "InventorySnapshot";

    static const string EVT_DAMAGE_DEALT = "DamageDealt";
    static const string EVT_DAMAGE_RECEIVED = "DamageReceived";
    static const string EVT_SHOT_FIRED = "ShotFired";
    static const string EVT_PLAYER_KILLED = "PlayerKilled";
    static const string EVT_PLAYER_DEATH = "PlayerDeath";
    static const string EVT_UNCONSCIOUS = "Unconscious";
    static const string EVT_REGAIN_CONSCIOUS = "RegainConscious";
    static const string EVT_BLEEDING = "Bleeding";

    static const string EVT_VEHICLE_ENTER = "VehicleEnter";
    static const string EVT_VEHICLE_EXIT = "VehicleExit";
    static const string EVT_VEHICLE_DOOR = "VehicleDoor";
    static const string EVT_VEHICLE_ENGINE = "VehicleEngine";
    static const string EVT_VEHICLE_CRASH = "VehicleCrash";
    static const string EVT_VEHICLE_DAMAGE = "VehicleDamage";
    static const string EVT_VEHICLE_INVENTORY = "VehicleInventory";

    static const string EVT_ACTION_START = "ActionStart";
    static const string EVT_ACTION_COMPLETE = "ActionComplete";
    static const string EVT_ACTION_INTERRUPT = "ActionInterrupt";

    static const string EVT_DOOR_OPEN = "DoorOpen";
    static const string EVT_DOOR_CLOSE = "DoorClose";
    static const string EVT_CRAFT = "Craft";
    static const string EVT_COOK = "Cook";
    static const string EVT_IGNITE = "Ignite";
    static const string EVT_EXTINGUISH = "Extinguish";

    static const string EVT_ZOMBIE_KILL = "ZombieKill";
    static const string EVT_ANIMAL_KILL = "AnimalKill";
    static const string EVT_BEAR_KILL = "BearKill";
    static const string EVT_WOLF_KILL = "WolfKill";
    static const string EVT_BOAR_KILL = "BoarKill";
    static const string EVT_DEER_KILL = "DeerKill";
    static const string EVT_SHEEP_KILL = "SheepKill";
    static const string EVT_GOAT_KILL = "GoatKill";
    static const string EVT_FOX_KILL = "FoxKill";
    static const string EVT_HARE_KILL = "HareKill";
    static const string EVT_HEN_KILL = "HenKill";
    static const string EVT_COW_KILL = "CowKill";
    static const string EVT_BANDIT_KILL = "BanditKill";

    static const string EVT_BUILD = "Build";
    static const string EVT_DISMANTLE = "Dismantle";
    static const string EVT_LOCK = "Lock";
    static const string EVT_UNLOCK = "Unlock";
};
