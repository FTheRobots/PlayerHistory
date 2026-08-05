class PH_PlayerBuffer
{
    ref array<string> lines;
    string steamId;
    string currentDateFile;
    int lineCount;
    bool flushing;

    void PH_PlayerBuffer(string id)
    {
        steamId = id;
        lines = new array<string>();
        currentDateFile = PH_Timestamp.DateFileName();
        lineCount = 0;
        flushing = false;
    }
};

class PH_EventWriter
{
    private ref map<string, ref PH_PlayerBuffer> m_Buffers;
    private bool m_FlushScheduled;

    void PH_EventWriter()
    {
        m_Buffers = new map<string, ref PH_PlayerBuffer>();
        m_FlushScheduled = false;
    }

    void WriteEvent(string steamId, string jsonLine)
    {
        if (steamId == "" || jsonLine == "")
            return;

        PH_PlayerBuffer buffer;
        if (!m_Buffers.Find(steamId, buffer) || !buffer)
        {
            buffer = new PH_PlayerBuffer(steamId);
            m_Buffers.Set(steamId, buffer);
        }

        buffer.lines.Insert(jsonLine);
        buffer.lineCount++;

        PH_ConfigData cfg = PH_Config.GetInstance().Get();
        if (buffer.lineCount >= cfg.writeBatchSize)
            FlushPlayer(steamId);
    }

    private void EnsureEventsFolder(string steamId)
    {
        string root = "$profile:" + PH_Constants.ROOT_FOLDER;
        if (!FileExist(root))
            MakeDirectory(root);

        string playerFolder = root + "\\" + steamId;
        if (!FileExist(playerFolder))
            MakeDirectory(playerFolder);

        string eventsPath = playerFolder + "\\" + PH_Constants.EVENTS_FOLDER;
        if (!FileExist(eventsPath))
            MakeDirectory(eventsPath);
    }

    void FlushPlayer(string steamId)
    {
        PH_PlayerBuffer buffer;
        if (!m_Buffers.Find(steamId, buffer) || !buffer || buffer.lines.Count() == 0)
            return;

        if (buffer.flushing)
            return;

        buffer.flushing = true;

        EnsureEventsFolder(steamId);

        string dateFile = PH_Timestamp.DateFileName();
        string filePath = "$profile:" + PH_Constants.ROOT_FOLDER + "\\" + steamId + "\\" + PH_Constants.EVENTS_FOLDER + "\\" + dateFile + ".jsonl";

        FileHandle fh = OpenFile(filePath, FileMode.APPEND);
        if (fh == 0 && !FileExist(filePath))
        {
            fh = OpenFile(filePath, FileMode.WRITE);
        }

        if (fh == 0)
        {
            PH_Log.Error("Cannot write log file: " + filePath);
            buffer.flushing = false;
            return;
        }

        for (int i = 0; i < buffer.lines.Count(); i++)
            FPrintln(fh, buffer.lines.Get(i));

        CloseFile(fh);
        buffer.lines.Clear();
        buffer.lineCount = 0;
        buffer.currentDateFile = dateFile;
        buffer.flushing = false;
    }

    void FlushAll()
    {
        array<string> steamIds = new array<string>();
        for (int i = 0; i < m_Buffers.Count(); i++)
            steamIds.Insert(m_Buffers.GetKey(i));

        for (int j = 0; j < steamIds.Count(); j++)
            FlushPlayer(steamIds.Get(j));
    }

    void ScheduleFlush()
    {
        if (m_FlushScheduled)
            return;

        m_FlushScheduled = true;
        float interval = PH_Config.GetInstance().Get().flushIntervalSeconds * 1000;
        GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(DoScheduledFlush, interval, true);
    }

    void DoScheduledFlush()
    {
        if (!GetGame().IsServer())
            return;

        if (!GetGame().GetMission())
            return;

        FlushAll();
    }
};
