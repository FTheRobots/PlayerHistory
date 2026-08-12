/** First-run defaults: balanced forensic preset with periodic inventory snapshots off. */
class PH_ConfigData
{
    int enabled = 1;

    int logSession = 1;
    int logPosition = 1;
    int logInventory = 1;
    int logCombat = 1;
    int logVehicle = 1;
    int logAction = 1;
    int logWorld = 1;
    int logZombie = 1;
    int logAnimal = 1;
    int logBandit = 1;
    int logBaseBuilding = 1;
    int logPlayerState = 1;
    int logChat = 1;
    float positionMinDistance = 10.0;
    float positionIntervalSeconds = 45.0;
    bool positionOnAction = false;
    bool positionOnDamage = false;
    bool positionOnInventory = false;

    int writeBatchSize = 25;
    float flushIntervalSeconds = 5.0;
    bool compressOldLogs = false;
    int maxBufferPerPlayer = 500;
    int scanEntityCounts = 0;
    float serverSnapshotIntervalSeconds = 15.0;
    float forcedPositionMinIntervalSeconds = 3.0;

    float inventorySnapshotInterval = 900.0;
    int logInventorySnapshots = 0;
    float playerStateInterval = 120.0;

    float deathNearbyItemRadius = 2.5;

    int inventorySnapshotMaxRetained = 24;
};

class PH_Config
{
    private static ref PH_Config instance;
    private ref PH_ConfigData m_Data;
    private string m_ConfigPath;

    static PH_Config GetInstance()
    {
        if (!instance)
            instance = new PH_Config();
        return instance;
    }

    void PH_Config()
    {
        m_ConfigPath = "$profile:" + PH_Constants.ROOT_FOLDER + "\\config.json";
        m_Data = new PH_ConfigData();
        Load();
    }

    PH_ConfigData Get()
    {
        return m_Data;
    }

    bool IsCategoryEnabled(string category)
    {
        if (m_Data.enabled == 0)
            return false;

        switch (category)
        {
            case PH_Constants.CAT_SESSION: return m_Data.logSession != 0;
            case PH_Constants.CAT_POSITION: return m_Data.logPosition != 0;
            case PH_Constants.CAT_INVENTORY: return m_Data.logInventory != 0;
            case PH_Constants.CAT_COMBAT: return m_Data.logCombat != 0;
            case PH_Constants.CAT_VEHICLE: return m_Data.logVehicle != 0;
            case PH_Constants.CAT_ACTION: return m_Data.logAction != 0;
            case PH_Constants.CAT_WORLD: return m_Data.logWorld != 0;
            case PH_Constants.CAT_ZOMBIE: return m_Data.logZombie != 0;
            case PH_Constants.CAT_ANIMAL: return m_Data.logAnimal != 0;
            case PH_Constants.CAT_BANDIT: return m_Data.logBandit != 0;
            case PH_Constants.CAT_BASEBUILDING: return m_Data.logBaseBuilding != 0;
            case PH_Constants.CAT_PLAYER_STATE: return m_Data.logPlayerState != 0;
            case PH_Constants.CAT_CHAT: return m_Data.logChat != 0;
        }
        return true;
    }

    void Load()
    {
        if (!FileExist("$profile:" + PH_Constants.ROOT_FOLDER))
            MakeDirectory("$profile:" + PH_Constants.ROOT_FOLDER);

        string err;
        PH_ConfigData loaded;
        if (JsonFileLoader<PH_ConfigData>.LoadFile(m_ConfigPath, loaded, err))
        {
            m_Data = loaded;
        }
        else
        {
            Save();
        }
    }

    void Save()
    {
        string err;
        JsonFileLoader<PH_ConfigData>.SaveFile(m_ConfigPath, m_Data, err);
    }
};
