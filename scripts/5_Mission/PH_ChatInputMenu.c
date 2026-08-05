#ifndef EXPANSIONMODCHAT
modded class ChatInputMenu
{
    override bool OnChange(Widget w, int x, int y, bool finished)
    {
        bool result = super.OnChange(w, x, y, finished);

        if (!finished || !GetGame().IsMultiplayer())
            return result;

        string text = m_edit_box.GetText();
        text.TrimInPlace();
        if (text == "" || text.IndexOf("#login ") == 0)
            return result;

        PH_VanillaChatClient.NotifyServerChat(CCDirect, text);
        return result;
    }
}
#endif
