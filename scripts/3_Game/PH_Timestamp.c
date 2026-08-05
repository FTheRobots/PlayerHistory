class PH_Timestamp
{
    static string NowISO()
    {
        int year, month, day, hour, minute, second, ms;
        GetYearMonthDayUTC(year, month, day);
        GetHourMinuteSecondUTC(hour, minute, second);
        ms = GetGame().GetTime();

        int msPart = ms % 1000;
        string timestamp = year.ToStringLen(4) + "-" + month.ToStringLen(2) + "-" + day.ToStringLen(2) + "T" + hour.ToStringLen(2) + ":" + minute.ToStringLen(2) + ":" + second.ToStringLen(2) + "." + msPart.ToStringLen(3) + "Z";
        return timestamp;
    }

    static string DateFileName()
    {
        int year, month, day;
        GetYearMonthDayUTC(year, month, day);
        return year.ToStringLen(4) + "-" + month.ToStringLen(2) + "-" + day.ToStringLen(2);
    }

    static string GenerateSessionId()
    {
        int year, month, day, hour, minute, second;
        GetYearMonthDayUTC(year, month, day);
        GetHourMinuteSecondUTC(hour, minute, second);
        return year.ToStringLen(4) + month.ToStringLen(2) + day.ToStringLen(2) + hour.ToStringLen(2) + minute.ToStringLen(2) + second.ToStringLen(2) + Math.RandomInt(1000, 9999).ToString();
    }
};
