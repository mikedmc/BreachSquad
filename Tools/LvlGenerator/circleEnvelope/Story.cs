using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Drawing;

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
                if ((nIdx < 0) || (nIdx >= arrEntries.Count))
                    return;
                arrEntries.RemoveAt(nIdx);
            }
        }

        // list of generations
        public List<StoryGeneration> arrGenerations = new List<StoryGeneration>();
        // selected generation and entry indexes
        public int nSelGeneration = -1;
        public int nSelArea = -1;

        public Story()
        {
            StoryGeneration sg = new StoryGeneration();
            // add starting generation
            AreaEntry ae = new AreaEntry();
            ae.nChildren = 1;
            ae.tags_any = "start";
            sg.arrEntries.Add(ae);

            arrGenerations.Add(sg);

            nSelGeneration = 0;
            nSelArea = 0;
        }

        public void AddGeneration()
        {
            StoryGeneration sg = new StoryGeneration();

            // add starting generation
            AreaEntry ae = new AreaEntry();
            ae.nChildren = 1;
            ae.tags_any = "";
            sg.arrEntries.Add(ae);

            arrGenerations.Add(sg);
            nSelGeneration = arrGenerations.Count - 1;
            nSelArea = sg.arrEntries.Count - 1;
        }

        public void AddGenerationEntry(int nGenerationIdx, int nExits, int nAddFlag, int nAvoidFlag, string strTags)
        {
            if ((nGenerationIdx < 0) || (nGenerationIdx >= arrGenerations.Count))
                return;

            arrGenerations[nGenerationIdx].AddEntry(nExits, nAddFlag, nAvoidFlag, strTags);
            nSelArea = arrGenerations[nGenerationIdx].arrEntries.Count - 1;
        }

        public void DeleteGenerationEntry(int nGenerationIdx, int nEntryIdx)
        {
            if ((nGenerationIdx < 0) || (nGenerationIdx >= arrGenerations.Count))
                return;
            if (arrGenerations[nGenerationIdx].arrEntries.Count <= 1)
                return;

            arrGenerations[nGenerationIdx].DeleteEntry(nEntryIdx);
            nSelArea = -1;
        }

        public void SortGenerations()
        {
            for (int kk = 0; kk < arrGenerations.Count; kk++)
            {
                arrGenerations[kk].arrEntries.Sort((x, y) => (x.nChildren > y.nChildren) ? 1 : 0);
            }
        }

        // Returns a color for each flag (8 possible flags)
        public Color GetFlagColor(int flag)
        {
            byte r = 0, g = 0, b = 0;
            if ((flag & 1) != 0) r = 128;
            if ((flag & 2) != 0) g = 128;
            if ((flag & 4) != 0) b = 128;

            if ((flag & 8) != 0) r += 64;
            if ((flag & 16) != 0) g += 64;
            if ((flag & 32) != 0) b += 64;

            return Color.FromArgb(r, g, b);
        }
    }
}
