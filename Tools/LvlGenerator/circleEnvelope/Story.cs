using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Drawing;
using System.Xml;
using System.Windows.Forms;

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
            // ID gets generated like generation * 100 + entry index
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

            public void AddEntry(int nID, int nExits, string strTagsAny, string strTagsAll, string strTagsNone)
            {
                AreaEntry ae = new AreaEntry();

                ae.nChildren = nExits;
                ae.tags_any = strTagsAny;
                ae.tags_all = strTagsAll;
                ae.tags_none = strTagsNone;

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

        // returns an entry that wasn't used during level generation
        public AreaEntry GetAvailableEntry(int nGeneration, int nParentID)
        {
            if ((nGeneration < 0) || (nGeneration >= arrGenerations.Count))
                return null;
            for (int kk = 0; kk < arrGenerations[nGeneration].arrEntries.Count; kk++)
            {
                if ((arrGenerations[nGeneration].arrEntries[kk].bUsed == false) && (arrGenerations[nGeneration].arrEntries[kk].nParentID == nParentID))
                    return arrGenerations[nGeneration].arrEntries[kk];
            }

            return null;
        }

        // clear "used" flag from a generation, for a specific parentID
        public void ClearChildEntries(int nGeneration, int nParentID)
        {
            if ((nGeneration < 0) || (nGeneration >= arrGenerations.Count))
                return;
            for (int kk = 0; kk < arrGenerations[nGeneration].arrEntries.Count; kk++)
            {
                if (arrGenerations[nGeneration].arrEntries[kk].nParentID == nParentID)
                    arrGenerations[nGeneration].arrEntries[kk].bUsed = false;
            }
        }

        // clear "used" flag fr a full generation and children ones
        public void ClearEntriesFromGeneration(int nGeneration)
        {
            if ((nGeneration < 0) || (nGeneration >= arrGenerations.Count))
                return;
            for (int gen = nGeneration; gen < arrGenerations.Count; gen++)
            {
                for (int kk = 0; kk < arrGenerations[gen].arrEntries.Count; kk++)
                {
                    arrGenerations[gen].arrEntries[kk].bUsed = false;
                }
            }
        }

        // computes IDs and other data for the areas (call before generating a level)
        public void ComputeRelationshipsGraph()
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

        public void AddGeneration(bool bAddEmptyArea = true)
        {
            StoryGeneration sg = new StoryGeneration();

            // add starting generation
            if (bAddEmptyArea)
            {
                AreaEntry ae = new AreaEntry();
                ae.nChildren = 1;
                ae.tags_any = "";
                sg.arrEntries.Add(ae);
            }

            arrGenerations.Add(sg);
            nSelGeneration = arrGenerations.Count - 1;
            nSelArea = sg.arrEntries.Count - 1;
        }

        public void AddGenerationEntry(int nGenerationIdx, int nExits, string strTagsAny, string strTagsAll, string strTagsNone)
        {
            if ((nGenerationIdx < 0) || (nGenerationIdx >= arrGenerations.Count))
                return;

            int nEntryID = nGenerationIdx * 1000 + arrGenerations[nGenerationIdx].arrEntries.Count;
            arrGenerations[nGenerationIdx].AddEntry(nEntryID, nExits, strTagsAny, strTagsAll, strTagsNone);
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

        public void SaveStory(string strPath)
        {
            // must compute ids first
            ComputeRelationshipsGraph();

            try
            {
                XmlTextWriter xw = new XmlTextWriter(strPath, null);
                xw.Formatting = Formatting.Indented;
                xw.WriteStartDocument();
                // write elements
                xw.WriteStartElement("LevelStory");
                xw.WriteStartAttribute("Generations");
                xw.WriteValue(arrGenerations.Count);
                xw.WriteEndAttribute();

                for (int kk = 0; kk < arrGenerations.Count; kk++)
                {
                    Story.StoryGeneration gen = arrGenerations[kk];
                    xw.WriteStartElement("Generation");
                    xw.WriteStartAttribute("Index");
                    xw.WriteValue(kk);
                    xw.WriteEndAttribute();
                    xw.WriteStartAttribute("Areas");
                    xw.WriteValue(gen.arrEntries.Count);
                    xw.WriteEndAttribute();
                    for (int jj = 0; jj < gen.arrEntries.Count; jj++)
                    {
                        Story.AreaEntry ae = gen.arrEntries[jj];

                        xw.WriteStartElement("Area");

                        xw.WriteAttributeString("Children", ae.nChildren.ToString());
                        xw.WriteAttributeString("ID", ae.nID.ToString());
                        xw.WriteAttributeString("ParentID", ae.nParentID.ToString());
                        xw.WriteAttributeString("TagsAny", ae.tags_any);
                        xw.WriteAttributeString("TagsAll", ae.tags_all);
                        xw.WriteAttributeString("TagsNone", ae.tags_none);

                        xw.WriteEndElement();
                    }
                    xw.WriteEndElement();
                }
                // end LevelStory
                xw.WriteEndElement();
                // end document
                xw.WriteEndDocument();
                xw.Flush();
                xw.Close();
            }
            catch (Exception ex)
            {
                MessageBox.Show("Error saving story! \n\n" + ex.Message);
            }
        }

        public void LoadStory(string strPath)
        {
            arrGenerations.Clear();
            nSelArea = -1;
            nSelGeneration = -1;

            try
            {
                XmlDocument xdoc = new XmlDocument();
                xdoc.Load(strPath);

                XmlNodeList nodesgen = xdoc.GetElementsByTagName("Generation");
                foreach (XmlNode node in nodesgen)
                {
                    AddGeneration(false);

                    int nindex = Convert.ToInt32(node.Attributes["Index"].InnerText);
                    int nareas = Convert.ToInt32(node.Attributes["Areas"].InnerText);
                    XmlNodeList anodes = node.SelectNodes("Area");
                    foreach (XmlNode anode in anodes)
                    {
                        int nChildren = Convert.ToInt32(anode.Attributes["Children"].InnerText);
                        int nID = Convert.ToInt32(anode.Attributes["ID"].InnerText);
                        int nParentID = Convert.ToInt32(anode.Attributes["ParentID"].InnerText);
                        string tags_any = anode.Attributes["TagsAny"].InnerText;
                        string tags_all = anode.Attributes["TagsAll"].InnerText;
                        string tags_none = anode.Attributes["TagsNone"].InnerText;
                        AddGenerationEntry(nindex, nChildren, tags_any, tags_all, tags_none);
                    }   
                }

            }
            catch (Exception ex)
            {
                MessageBox.Show("Error loading story! \n\n" + ex.Message);
            }

            // compune IDs again
            ComputeRelationshipsGraph();
        }

    }
}
