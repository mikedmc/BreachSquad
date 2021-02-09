using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace circleEnvelope
{
    public class Story
    {
        public class AreaEntry
        {
            // number of children of the room (connections will be children + 1)
            public int nChildren;
            // flag that gets OR-ed on all children
            public int addFlag;
            // flag to avoid when finding a placement for the area
            public int avoidFlag;
            // area tags filters
            public string tags_any; // any of the tags will add it
            public string tags_all; // will add it only if contains ALL tags (doesn't exclude ANY filter)
            public string tags_none;// will remove it if it contains any of the tags here

            public AreaEntry()
            {
                nChildren = 0;
                addFlag = 0;
                avoidFlag = 0;
                tags_any = "";
                tags_all = "";
                tags_none = "";
            }

            public string GetName()
            {
                return "C[" + nChildren + "]T[" + tags_any + "]F[" + addFlag + "]NF[" + avoidFlag + "]";
            }
        }

        // the story is composed of generations
        public class StoryGeneration
        {
            public List<AreaEntry> arrEntries = new List<AreaEntry>();

            public void AddEntry(int nExits, int nAddFlag, int nAvoidFlag, string strTags)
            {
                AreaEntry ae = new AreaEntry();

                ae.nChildren = nExits;
                ae.addFlag = nAddFlag;
                ae.avoidFlag = nAvoidFlag;
                ae.tags_any = strTags;

                arrEntries.Add(ae);
            }

            public void UpdateEntry(int nIdx, int nExits, int nAddFlag, int nAvoidFlag, string strTags)
            {
                AreaEntry ae = arrEntries[nIdx];

                ae.nChildren = nExits;
                ae.addFlag = nAddFlag;
                ae.avoidFlag = nAvoidFlag;
                ae.tags_any = strTags;
            }

            public void DeleteEntry(int nIdx)
            {
                arrEntries.RemoveAt(nIdx);
            }
        }

        // list of generations
        public List<StoryGeneration> arrGenerations = new List<StoryGeneration>();

        public Story()
        {
            StoryGeneration sg = new StoryGeneration();
            // add starting generation
            AreaEntry ae = new AreaEntry();
            ae.nChildren = 1;
            ae.tags_any = "start";
            sg.arrEntries.Add(ae);

            arrGenerations.Add(sg);
        }

        public void AddGeneration()
        {
            StoryGeneration sg = new StoryGeneration();

            arrGenerations.Add(sg);
        }
    }
}
