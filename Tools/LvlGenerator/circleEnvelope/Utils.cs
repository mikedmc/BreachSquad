using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace circleEnvelope
{
    class Utils
    {
        public static bool StringContainsAnyTag(string strString, string strTags, char cSeparator)
        {
            string[] tags = strTags.Split(cSeparator);
            for (int kk = 0; kk < tags.Length; kk++)
            {
                if (strString.ToLower().Contains(tags[kk].ToLower()))
                    return true;
            }
            return false;
        }

        public static bool StringContainsAllTags(string strString, string strTags, char cSeparator)
        {
            string[] tags = strTags.Split(cSeparator);
            int nContained = 0;
            for (int kk = 0; kk < tags.Length; kk++)
            {
                if (strString.ToLower().Contains(tags[kk].ToLower()))
                    nContained++;
            }
            return (nContained == tags.Length);
        }

    }
}
