class PH_JsonBuilder
{
    static string Escape(string s)
    {
        if (s == "")
            return "";

        string result = s;
        string bs = "\\";
        string q = "\"";

        result.Replace(bs, bs + bs);
        result.Replace(q, bs + q);

        string nl = "\n";
        result.Replace(nl, bs + "n");

        string cr = "\r";
        result.Replace(cr, bs + "r");

        string tab = "\t";
        result.Replace(tab, bs + "t");

        return result;
    }

    static string VectorToJson(vector v)
    {
        return "[" + v[0].ToString() + "," + v[1].ToString() + "," + v[2].ToString() + "]";
    }

    static string StringField(string key, string value)
    {
        return "\"" + key + "\":\"" + Escape(value) + "\"";
    }

    static string NumberField(string key, float value)
    {
        return "\"" + key + "\":" + value.ToString();
    }

    static string IntField(string key, int value)
    {
        return "\"" + key + "\":" + value.ToString();
    }

    static string BoolField(string key, bool value)
    {
        if (value)
            return "\"" + key + "\":true";
        return "\"" + key + "\":false";
    }

    static string VectorField(string key, vector v)
    {
        return "\"" + key + "\":" + VectorToJson(v);
    }

    static string MetadataObject(map<string, string> fields)
    {
        string result = "{";
        bool first = true;

        for (int i = 0; i < fields.Count(); i++)
        {
            string key = fields.GetKey(i);
            string val = fields.GetElement(i);

            if (!first)
                result += ",";
            first = false;

            result += StringField(key, val);
        }

        result += "}";
        return result;
    }
};
