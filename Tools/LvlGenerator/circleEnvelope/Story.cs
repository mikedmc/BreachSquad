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
            // area tags filters
            public string tags_any; // any of the tags will add it
            public string tags_all; // will add it only if contains ALL tags (doesn't exclude ANY filter)
            public string tags_none;// will remove it if it contains any of the tags here

            // usage flag when generating the level
            public bool bUsed;
            // ID gets generated like generation * 1000 + entry index
            public int nID;
            public int nParentID;


            public AreaEntry()
            {
                nChildren = 0;
                tags_any = "";
                tags_all = "";
                tags_none = "";

                bUsed = false;
                nID = 0;
                nParentID = -1;
            }
        }

        // the story is composed of generations
        public class StoryGeneration
        {
            public List<AreaEntry> arrEntries = new List<AreaEntry>();

            public void AddEntry(int nID, int nExits, int nAddFlag, int nAvoidFlag, string strTags)
            {
                AreaEntry ae = new AreaEntry();

                ae.nChildren = nExits;
                ae.tags_any = strTags;

                arrEntries.Add(ae);
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

        // removes areas children number where next area doesn't have enough children
        public void TrimLooseEnds()
        {
            for (int gg = 0; gg < arrGenerations.Count; gg++)
            {
                int nChildIdx = 0;
                StoryGeneration gen = arrGenerations[gg];
                for (int aa = 0; aa < gen.arrEntries.Count; aa++)
                {
                    AreaEntry area = gen.arrEntries[aa];
                    area.bUsed = false;
                    area.nID = gg * 1000 + aa;
                    // set children parents ids
                    if (gg < arrGenerations.Count - 1)
                    {
                        for (int cc = 0; cc < area.nChildren; cc++)
                        {
                            if (cc < arrGenerations[gg + 1].arrEntries.Count)
                            {
                                if (nChildIdx + cc < arrGenerations[gg + 1].arrEntries.Count)
                                    (arrGenerations[gg + 1] as StoryGeneration).arrEntries[nChildIdx + cc].nParentID = area.nID;
                            }
                        }
                        nChildIdx += area.nChildren;
                    }
                }
            }
        }

        // computes IDs and other data for the areas (call before generating a level)
        public void ComputeRelationships()
        {
            nSelArea = -1;
            nSelGeneration = -1;

            for (int gg = 0; gg < arrGenerations.Count; gg++) 
            {
                int nChildIdx = 0;
                StoryGeneration gen = arrGenerations[gg];
                for (int aa = 0; aa < gen.arrEntries.Count; aa++) 
                {
                    AreaEntry area = gen.arrEntries[aa];
                    area.bUsed = false;
                    area.nID = gg * 100 + aa;
                    // set children parents ids
                    if (gg < arrGenerations.Count - 1)
                    {
                        int nFakeChildren = 0;
                        for (int cc = 0; cc < area.nChildren; cc++)
                        {
                            if (nChildIdx + cc < arrGenerations[gg + 1].arrEntries.Count)
                            {
                                (arrGenerations[gg + 1] as StoryGeneration).arrEntries[nChildIdx + cc].nParentID = area.nID;
                            }
                            else
                            {
                                nFakeChildren++;
                            }
                        }
                        area.nChildren -= nFakeChildren;
                        nChildIdx += area.nChildren;
                    }
                    // trim children for last generation
                    if (gg == arrGenerations.Count - 1)
                    {
                        area.nChildren = 0;
                    }
                }
            }
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

            int nEntryID = nGenerationIdx * 1000 + arrGenerations[nGenerationIdx].arrEntries.Count;
            arrGenerations[nGenerationIdx].AddEntry(nEntryID, nExits, nAddFlag, nAvoidFlag, strTags);
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
