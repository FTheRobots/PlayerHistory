#ifndef EXPANSIONMODCHAT
class PH_VanillaChatClient
{
    static void NotifyServerChat(int channel, string text)
    {
        if (!GetGame().IsMultiplayer())
            return;

        PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
        if (!player || !player.GetIdentity())
            return;

        text.TrimInPlace();
        if (text == "")
            return;

        Param2<int, string> payload = new Param2<int, string>(channel, text);
        player.RPCSingleParam(PH_Constants.PH_RPC_CHAT_LOG, payload, true, player.GetIdentity());
    }
}
#endif
