using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Drawing;
using System.Collections;
using System.Windows.Forms;

namespace circleEnvelope
{
    class CLevelGen
    {
        // list of available areas
        public class CInventoryArea
        {
            public Form1.CAreaDesc area = null;
            public int nAvailable = 0;
            public int nConsumed = 0;
        };

        // data copied from inventory area
        public class CPlacedArea
        {
            public class CAreaConnector
            {
                public Point pos = new Point(); //position of connector in local space (blocks coords)
                public int dir = Form1.K_DIR_NONE;    //direction of connection
                public CPlacedArea pConnectedArea = null; // reference to neighbouring connected area
            }

            public Rectangle AABB = new Rectangle(); // world space rectangle
            public Form1.CGridCell[,] blocks = null; //blocks in matrix of AABB.w/h
            public List<CAreaConnector> arrConnections = new List<CAreaConnector>();

            public int nGeneration = 0;

            // returns 
            public ArrayList GetShuffledAvailableConnectors(bool bShuffle)
            {
                ArrayList arrConn = new ArrayList();
                for (int ncon = 0; ncon < arrConnections.Count; ncon++)
                {
                    // only add not connected connectors
                    if (arrConnections[ncon].pConnectedArea == null)
                        arrConn.Add(arrConnections[ncon]);
                }

                if (bShuffle)
                    CLevelGen.ShuffleList(arrConn);

                return arrConn;
            }

            public CPlacedArea(Form1.CAreaDesc area, Point vPos)
            {
                AABB = area.AABB;
                AABB.X = vPos.X; AABB.Y = vPos.Y;
                nGeneration = 0;
                // allocate blocks
                blocks = new Form1.CGridCell[AABB.Width, AABB.Height];
                for (int yy = 0; yy < AABB.Height; yy++)
                {
                    for (int xx = 0; xx < AABB.Width; xx++)
                    {
                        // copy active area
                        blocks[xx, yy] = new Form1.CGridCell();
                        blocks[xx, yy].bFilled = area.blocks[area.AABB.X + xx][area.AABB.Y + yy].bFilled;
                        blocks[xx, yy].connectionDir = area.blocks[area.AABB.X + xx][area.AABB.Y + yy].connectionDir;
                    }
                }
                // copy connectors
                for (int kk = 0; kk < area.arrConnectors.Count; kk++)
                {
                    CAreaConnector nc = new CAreaConnector();
                    nc.dir = area.blocks[area.arrConnectors[kk].X][area.arrConnectors[kk].Y].connectionDir;
                    nc.pConnectedArea = null;
                    // bring connector position in placed area space:
                    nc.pos = area.arrConnectors[kk];
                    nc.pos.X -= area.AABB.X; nc.pos.Y -= area.AABB.Y;

                    arrConnections.Add(nc);
                }
            }
        }

        public static Random rnd = new Random();

        Point posStart = new Point(10000, 10000);
        // placed CPlacedArea elements
        public ArrayList m_arrPlaced = new ArrayList();
        public Rectangle m_levelAABB = new Rectangle();

        public ArrayList FilterAreas(ArrayList inventory, int nMinConnectors, int nMaxConnectors, int dirFlags, string strTagInclude = "", string strTagExclude = "")
        {
            ArrayList retList = new ArrayList();
            foreach (CInventoryArea iarea in inventory)
            {
                if ((iarea.nConsumed < iarea.nAvailable) &&
                    (iarea.area.arrConnectors.Count >= nMinConnectors) && 
                    (iarea.area.arrConnectors.Count <= nMaxConnectors) &&
                    ((iarea.area.areaConnDirFlags & dirFlags) != 0))
                {
                    bool bAdd = true;
                    // check tags too
                    if ((strTagInclude.Length > 0) && (!iarea.area.strTags.Contains(strTagInclude)))
                        bAdd = false;
                    if ((strTagExclude.Length > 0) && (iarea.area.strTags.Contains(strTagExclude)))
                        bAdd = false;

                    if (bAdd)
                        retList.Add(iarea);
                }
            }

            return retList;
        }

        public static void ShuffleList(ArrayList list)
        {
            int nCount = list.Count;
            if (nCount <= 1)
                return;
            for (int kk = 0; kk < nCount * 2; kk++)
            {
                int a = rnd.Next(list.Count);
                int b = rnd.Next(list.Count);
                if (a == b)
                    continue;
                object temp = list[a];
                list[a] = list[b];
                list[b] = temp;
            }
        }

        Form1.CGridCell GetPlacedBlockAt(Point vPos)
        {
            foreach (CPlacedArea pa in m_arrPlaced)
            {
                if (!pa.AABB.Contains(vPos))
                    continue;
                Point vLocal = new Point(vPos.X - pa.AABB.X, vPos.Y - pa.AABB.Y);
                return pa.blocks[vLocal.X, vLocal.Y];
            }
            return null;
        }

        bool IsZoneClear(CInventoryArea iarea, Point vPos)
        {
            Rectangle AABBtest = iarea.area.AABB;
            AABBtest.X = vPos.X;
            AABBtest.Y = vPos.Y;
            // check overlapping blocks
            for (int xx = 0; xx < AABBtest.Width; xx++)
            {
                for (int yy = 0; yy < AABBtest.Height; yy++)
                {
                    // check map occupation only on occupied blocks in current test area
                    Form1.CGridCell block = iarea.area.blocks[xx + iarea.area.AABB.X][yy + iarea.area.AABB.Y];
                    if (block.bFilled)
                    {
                        Form1.CGridCell blockPlaced = GetPlacedBlockAt(new Point(xx + vPos.X, yy + vPos.Y));
                        if ((blockPlaced != null) && (blockPlaced.bFilled))
                            return false;
                        // check borders for blocked connectors and random connections
                        int[] directions = new int[] { Form1.K_DIR_LEFT, Form1.K_DIR_UP, Form1.K_DIR_RIGHT, Form1.K_DIR_DOWN };

                        // check on all neighbours as blocks might overlap
                        for (int kk = 0; kk < Form1.K_DIRS_CNT; kk++)
                        {
                            Point vOff = Form1.DIR_OFFSET(directions[kk]);
                            Form1.CGridCell pNeigh = GetPlacedBlockAt(new Point(xx + vPos.X + vOff.X, yy + vPos.Y + vOff.Y));
                            // for each neighbour that is alrady placed
                            if ((pNeigh != null) && (pNeigh.bFilled))
                            {
                                int dir = directions[kk];
                                int dir_inv = Form1.INVERSE_DIR(directions[kk]);
                                // remote blocked connection
                                if ((pNeigh.connectionDir == dir_inv) && (block.connectionDir != dir))
                                    return false;
                                // local block blocked connection
                                if (block.connectionDir == dir)
                                {
                                    // only allowed if remote block has matching connector
                                    if (pNeigh.connectionDir != dir_inv)
                                        return false;
                                    else
                                    {
                                        // check for random connection using the area generations or current stitch point
                                        //MessageBox.Show("Random connection found UP!");
                                    }
                                }
                            }
                        }
                    }
                }
            }
            return true;
        }

        public void RemoveChildrenOf(CPlacedArea parent)
        {
            for (int kk = m_arrPlaced.Count - 1; kk >= 0; kk--) 
            {
                CPlacedArea area = m_arrPlaced[kk] as CPlacedArea;
                // skip lower generation areas (parents)
                if (area.nGeneration <= parent.nGeneration)
                    continue;
                foreach (CPlacedArea.CAreaConnector pconn in area.arrConnections)
                {
                    if (pconn.pConnectedArea == parent)
                    {
                        //remove and break
                        m_arrPlaced.RemoveAt(kk);
                        break;
                    }
                }
            }
        }

        // removes all generations >= nMinGeneration
        public void RemoveGenerations(int nMinGeneration)
        {
            Console.WriteLine("Removing generations >= " + nMinGeneration);
            // unlink remaining generations from useless ones
            for (int kk = 0; kk < m_arrPlaced.Count; kk++)
            {
                CPlacedArea area = m_arrPlaced[kk] as CPlacedArea;
                if (area.nGeneration < nMinGeneration)
                {
                    foreach (CPlacedArea.CAreaConnector conn in area.arrConnections)
                    {
                        if (conn.pConnectedArea.nGeneration >= nMinGeneration)
                            conn.pConnectedArea = null;
                    }
                }
            }
            // delete useless generations
            for (int kk = m_arrPlaced.Count - 1; kk >= 0; kk--)
            {
                CPlacedArea area = m_arrPlaced[kk] as CPlacedArea;
                if (area.nGeneration >= nMinGeneration)
                {
                    m_arrPlaced.RemoveAt(kk);
                }
            }
        }

        public bool PlaceRandomArea(ArrayList inventory, CPlacedArea parent, CPlacedArea.CAreaConnector parentConn, int nGeneration, int nMaxGeneration /**/, string strTagInclude = "", string strTagExclude = "")
        {
            int nDirFlag = Form1.DIRFLAG_ANY;
            if (parentConn.dir == Form1.K_DIR_LEFT) nDirFlag = Form1.DIRFLAG_RIGHT;
            if (parentConn.dir == Form1.K_DIR_UP) nDirFlag = Form1.DIRFLAG_DOWN;
            if (parentConn.dir == Form1.K_DIR_RIGHT) nDirFlag = Form1.DIRFLAG_LEFT;
            if (parentConn.dir == Form1.K_DIR_DOWN) nDirFlag = Form1.DIRFLAG_UP;

            Point vDirOff = Form1.DIR_OFFSET(parentConn.dir);
            Point vStitchPt = new Point(parentConn.pos.X + parent.AABB.X, parentConn.pos.Y + parent.AABB.Y);
            vStitchPt.X += vDirOff.X; vStitchPt.Y += vDirOff.Y;

            // last generation only adds endings (here it should follow the level story data)
            int nMinConn = 2;
            int nMaxConn = 5;
            if (nGeneration >= nMaxGeneration)
            {
                nMinConn = 1;
                nMaxConn = 1;
            }
            ArrayList availableList = FilterAreas(inventory, nMinConn, nMaxConn, nDirFlag, strTagInclude, strTagExclude);
            ShuffleList(availableList);

            if (availableList.Count == 0)
            {
                Console.WriteLine("Insufficient rooms in inventory! dirflag:" + nDirFlag);
            }

            foreach (CInventoryArea iarea in availableList)
            {
                int tryConnDir = Form1.INVERSE_DIR(parentConn.dir);
                //gets list of all connectors for a specified direction and shuffles them
                ArrayList arrConn = null;
                int nRetConn = iarea.area.GetConnectors(tryConnDir, out arrConn);
                // we have no connectors that way, try next
                if (nRetConn <= 0)
                    continue;
                ShuffleList(arrConn);
                for (int kk = 0; kk < arrConn.Count; kk++)
                {
                    // find position of connection point
                    Point tryConnPt = (Point)arrConn[kk];
                    // bring connector pos in relative space
                    tryConnPt.X -= iarea.area.AABB.X;
                    tryConnPt.Y -= iarea.area.AABB.Y;

                    // find origin for area to place
                    Point tryPos = new Point(vStitchPt.X - tryConnPt.X, vStitchPt.Y - tryConnPt.Y);
                    // see if area is clear 
                    if (IsZoneClear(iarea, tryPos))
                    {
                        // consume from set
                        iarea.nConsumed++;
                        // all good, add new area
                        CPlacedArea na = new CPlacedArea(iarea.area, tryPos);
                        na.nGeneration = nGeneration;
                        // point parent connection to this
                        parentConn.pConnectedArea = na;
                        //make child point to parent too
                        Point vStitchLocal = new Point(vStitchPt.X - na.AABB.X, vStitchPt.Y - na.AABB.Y);
                        foreach (CPlacedArea.CAreaConnector con in na.arrConnections)
                        {
                            if (con.pos == vStitchLocal)
                            {
                                con.pConnectedArea = parent;
                                break;
                            }
                        }

                        m_arrPlaced.Add(na);

                        return true;
                    }
                }
            }
            // no area fits
            return false;
        }


        // generates level with additional step for corridors, using fCorridorProb as probability of attaching a corridor (0..1)
        public bool GenerateLevelWithCorridors(ArrayList inventory, int maxDepth, float fCorridorProb)
        {
            Console.WriteLine("\n\nGenerating level - corridors probability:" + fCorridorProb.ToString());
            m_arrPlaced.RemoveRange(0, m_arrPlaced.Count);

            bool bLevelGenerated = true;

            ArrayList availableList = FilterAreas(inventory, 1, 1, Form1.DIRFLAG_ANY, "start");
            if (availableList.Count > 0)
            {
                ShuffleList(availableList);
                // place starting area:
                CInventoryArea selarea = availableList[0] as CInventoryArea;
                CPlacedArea pa = new CPlacedArea(selarea.area, posStart);
                selarea.nConsumed++;
                m_arrPlaced.Add(pa);

                int nMaxDepth = maxDepth;

                int nCurrGeneration = 0;
                while (nCurrGeneration < nMaxDepth)
                {
                    ///--- place corridors before placing next generation, don't try too hard
                    // for each placed area of current generation:
                    int nPlacedCntCorr = m_arrPlaced.Count; //save count before, it grows
                    for (int kk = 0; kk < nPlacedCntCorr; kk++)
                    {
                        // find placed area
                        CPlacedArea placed = m_arrPlaced[kk] as CPlacedArea;
                        if (placed.nGeneration != nCurrGeneration)
                            continue;

                        // get the shuffled connectors
                        ArrayList arrConn = placed.GetShuffledAvailableConnectors(true);
                        // take connectors one by one and try to place random corridor
                        for (int ncon = 0; ncon < arrConn.Count; ncon++)
                        {
                            // probability of corridor presence computed on each connector
                            if (rnd.Next(1000) > (int)(999 * fCorridorProb))
                                continue;

                            CPlacedArea.CAreaConnector curcon = arrConn[ncon] as CPlacedArea.CAreaConnector;
                            // Place random area tries to place all available items, randomized
                            bool bPlaced = PlaceRandomArea(inventory, placed, curcon, placed.nGeneration, nMaxDepth, "hall");
                            if (!bPlaced)
                            {
                                Console.WriteLine("Could not place corridor.");
                            }
                        }
                    }

                    ///--- place actual rooms (corridors must be excluded)
                    //_ASSERT(nCurrGeneration < 50);
                    int generationTries = 10;
                    bool bGenerationPlaced = false;
                    while ((generationTries > 0) && (bGenerationPlaced == false))
                    {
                        bGenerationPlaced = true;
                        // for each placed area of current generation:
                        int nPlacedCnt = m_arrPlaced.Count; //save count before, it grows
                        for (int kk = 0; kk < nPlacedCnt; kk++)
                        {
                            // find placed area
                            CPlacedArea placed = m_arrPlaced[kk] as CPlacedArea;
                            if (placed.nGeneration != nCurrGeneration)
                                continue;
                            // for each area try connecting the children N times
                            int childTries = 10;
                            bool bChildrenPlaced = false;
                            while ((childTries > 0) && (bChildrenPlaced == false))
                            {
                                bChildrenPlaced = true;
                                // get the shuffled connectors
                                ArrayList arrConn = placed.GetShuffledAvailableConnectors(true);
                                // take connectors one by one and try to place random children
                                for (int ncon = 0; ncon < arrConn.Count; ncon++)
                                {
                                    CPlacedArea.CAreaConnector curcon = arrConn[ncon] as CPlacedArea.CAreaConnector;
                                    // Place random area tries to place all available items with future depth
                                    bool bPlaced = PlaceRandomArea(inventory, placed, curcon, placed.nGeneration + 1, nMaxDepth, "", "hall");
                                    if (!bPlaced)
                                    {
                                        Console.WriteLine("Could not place children! Removing them! try:" + childTries);
                                        bChildrenPlaced = false;
                                        //remove already placed children of this parent area
                                        RemoveChildrenOf(placed);
                                    }
                                }

                                childTries--;
                            }
                            // failed to place children after many tries:
                            if (bChildrenPlaced == false)
                            {
                                Console.WriteLine("Generation failed! try:" + generationTries);
                                bGenerationPlaced = false;
                                //remove parent generations and all of their children
                                if (nCurrGeneration > 0)
                                {
                                    RemoveGenerations(nCurrGeneration);
                                    nCurrGeneration--;
                                }
                                else
                                {
                                    // returned to starting point, failed generating level!
                                    generationTries = 0;
                                }
                                // exit generations for
                                break;
                            }
                        }

                        generationTries--;
                    }

                    // AL GOOD, prepare next generation
                    if (bGenerationPlaced == true)
                    {
                        nCurrGeneration++;
                    }
                    else
                    {
                        m_arrPlaced.RemoveRange(0, m_arrPlaced.Count);
                        bLevelGenerated = false;
                        nCurrGeneration = maxDepth; //force exit while
                        MessageBox.Show("Could not generate level!");
                        break;
                    }
                }
            }

            if (bLevelGenerated)
            {
                // find level AABB
                Point vMin = new Point(1000000, 1000000);
                Point vMax = new Point(-1000000, -1000000);
                foreach (CPlacedArea pa in m_arrPlaced)
                {
                    if (pa.AABB.X < vMin.X) vMin.X = pa.AABB.X;
                    if (pa.AABB.Y < vMin.Y) vMin.Y = pa.AABB.Y;
                    if (pa.AABB.Right > vMax.X) vMax.X = pa.AABB.Right;
                    if (pa.AABB.Bottom > vMax.Y) vMax.Y = pa.AABB.Bottom;
                }

                m_levelAABB = new Rectangle(vMin.X, vMin.Y, vMax.X - vMin.X, vMax.Y - vMin.Y);
                Console.WriteLine("-- Level generation OK!");
            }
            else
            {
                m_levelAABB = new Rectangle(0, 0, 0, 0);
            }

            return bLevelGenerated;
        }

        // generates random level, corridors included in possible rooms
        public bool GenerateLevel(ArrayList inventory, int maxDepth)
        {
            Console.WriteLine("\n\nGenerating level...");
            m_arrPlaced.RemoveRange(0, m_arrPlaced.Count);

            bool bLevelGenerated = true;

            // add first area, starting area (only one exit)
            ArrayList availableList = FilterAreas(inventory, 1, 1, Form1.DIRFLAG_ANY);
            if (availableList.Count > 0)
            {
                ShuffleList(availableList);
                // place starting area:
                CInventoryArea selarea = availableList[0] as CInventoryArea;
                CPlacedArea pa = new CPlacedArea(selarea.area, posStart);
                selarea.nConsumed++;
                m_arrPlaced.Add(pa);

                int nMaxDepth = maxDepth;

                int nCurrGeneration = 0;
                while (nCurrGeneration < nMaxDepth)
                {
                    //_ASSERT(nCurrGeneration < 50);
                    int generationTries = 10;
                    bool bGenerationPlaced = false;
                    while ((generationTries > 0) && (bGenerationPlaced == false))
                    {
                        bGenerationPlaced = true;
                        // for each placed area of current generation:
                        int nPlacedCnt = m_arrPlaced.Count; //save count before, it grows
                        for (int kk = 0; kk < nPlacedCnt; kk++)
                        {
                            // find placed area
                            CPlacedArea placed = m_arrPlaced[kk] as CPlacedArea;
                            if (placed.nGeneration != nCurrGeneration)
                                continue;
                            // for each area try connecting the children N times
                            int childTries = 10;
                            bool bChildrenPlaced = false;
                            while ((childTries > 0) && (bChildrenPlaced == false))
                            {
                                bChildrenPlaced = true;
                                // get the shuffled connectors
                                ArrayList arrConn = placed.GetShuffledAvailableConnectors(true);
                                // take connectors one by one and try to place random children
                                for (int ncon = 0; ncon < arrConn.Count; ncon++)
                                {
                                    CPlacedArea.CAreaConnector curcon = arrConn[ncon] as CPlacedArea.CAreaConnector;
                                    // Place random area tries to place all available items
                                    bool bPlaced = PlaceRandomArea(inventory, placed, curcon, placed.nGeneration + 1, nMaxDepth);
                                    if (!bPlaced)
                                    {
                                        Console.WriteLine("Could not place children! Removing them! try:" + childTries);
                                        bChildrenPlaced = false;
                                        //remove already placed children of this parent area
                                        RemoveChildrenOf(placed);
                                    }
                                }

                                childTries--;
                            }
                            // failed to place children after many tries:
                            if (bChildrenPlaced == false)
                            {
                                Console.WriteLine("Generation failed! try:" + generationTries);
                                bGenerationPlaced = false;
                                //remove parent generations and all of their children
                                if (nCurrGeneration > 0)
                                {
                                    RemoveGenerations(nCurrGeneration);
                                    nCurrGeneration--;
                                }
                                else
                                {
                                    // returned to starting point, failed generating level!
                                    generationTries = 0;
                                }
                                // exit generations for
                                break;
                            }
                        }

                        generationTries--;
                    }

                    // AL GOOD, prepare next generation
                    if (bGenerationPlaced == true)
                    {
                        nCurrGeneration++;
                    }
                    else
                    {
                        m_arrPlaced.RemoveRange(0, m_arrPlaced.Count);
                        bLevelGenerated = false;
                        nCurrGeneration = maxDepth; //force exit while
                        MessageBox.Show("Could not generate level!");
                        break;
                    }
                }
            }

            if (bLevelGenerated)
            {
                // find level AABB
                Point vMin = new Point(1000000, 1000000);
                Point vMax = new Point(-1000000, -1000000);
                foreach (CPlacedArea pa in m_arrPlaced)
                {
                    if (pa.AABB.X < vMin.X) vMin.X = pa.AABB.X;
                    if (pa.AABB.Y < vMin.Y) vMin.Y = pa.AABB.Y;
                    if (pa.AABB.Right > vMax.X) vMax.X = pa.AABB.Right;
                    if (pa.AABB.Bottom > vMax.Y) vMax.Y = pa.AABB.Bottom;
                }

                m_levelAABB = new Rectangle(vMin.X, vMin.Y, vMax.X - vMin.X, vMax.Y - vMin.Y);
                Console.WriteLine("-- Level generation OK!");
            }
            else
            {
                m_levelAABB = new Rectangle(0, 0, 0, 0);
            }

            return bLevelGenerated;
        }
    }
}
