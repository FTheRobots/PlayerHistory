modded class ActionBase
{
    override void OnEndServer(ActionData action_data)
    {
        super.OnEndServer(action_data);

        if (!GetGame().IsServer() || !action_data || !action_data.m_Player)
            return;

        PlayerBase player = action_data.m_Player;
        if (!player.GetIdentity())
            return;

        string actionName = ClassName();
        string adminMsg = GetAdminLogMessage(action_data);

        map<string, string> meta = new map<string, string>();
        meta.Set("actionClass", actionName);
        if (adminMsg != "")
            meta.Set("adminLog", adminMsg);

        if (action_data.m_Target && action_data.m_Target.GetObject())
        {
            Object target = action_data.m_Target.GetObject();
            meta.Set("target", target.GetType());
            meta.Set("targetPosition", PH_JsonBuilder.VectorToJson(target.GetPosition()));
        }

        if (action_data.m_MainItem)
            PH_ItemSerializer.AttachItemMeta(meta, action_data.m_MainItem);

        PH_Service.GetInstance().LogEvent(player, PH_Constants.CAT_ACTION, PH_Constants.EVT_ACTION_COMPLETE, meta);
    }
};
