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

        Random rnd = new Random();

        Point posStart = new Point(10000, 10000);
        // placed CPlacedArea elements
        public ArrayList m_arrPlaced = new ArrayList();
        public Rectangle m_levelAABB = new Rectangle();

        public ArrayList FilterAreas(ArrayList inventory, int nMinConnectors, int nMaxConnectors, int dirFlags)
        {
            ArrayList retList = new ArrayList();
            foreach (CInventoryArea iarea in inventory)
            {
                if ((iarea.nConsumed < iarea.nAvailable) &&
                    (iarea.area.arrConnectors.Count >= nMinConnectors) && 
                    (iarea.area.arrConnectors.Count <= nMaxConnectors) &&
                    ((iarea.area.areaConnDirFlags & dirFlags) != 0))
                {
                    retList.Add(iarea);
                }
            }

            return retList;
        }

        public void ShuffleList(ArrayList list)
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
                        // TOP
                        if (yy == 0)
                        {
                            Form1.CGridCell blockU = GetPlacedBlockAt(new Point(xx + vPos.X, yy + vPos.Y - 1));
                            if ((blockU != null) && (blockU.bFilled))
                            {
                                // remote blocked connection
                                if ((blockU.connectionDir == Form1.K_DIR_DOWN) && (block.connectionDir != Form1.K_DIR_UP))
                                    return false;
                                // local block blocked connection
                                if (block.connectionDir == Form1.K_DIR_UP)
                                {
                                    // only allowed if remote block has matching connector
                                    if (blockU.connectionDir != Form1.K_DIR_DOWN)
                                        return false;
                                    else
                                    {
                                        //MessageBox.Show("Random connection found UP!");
                                    }
                                }
                            }
                        }
                        // BOTTOM
                        if (yy == AABBtest.Height - 1)
                        {
                            Form1.CGridCell blockD = GetPlacedBlockAt(new Point(xx + vPos.X, yy + vPos.Y + 1));
                            if ((blockD != null) && (blockD.bFilled))
                            {
                                // remote blocked connection
                                if ((blockD.connectionDir == Form1.K_DIR_UP) && (block.connectionDir != Form1.K_DIR_DOWN))
                                    return false;
                                // local block blocked connection
                                if (block.connectionDir == Form1.K_DIR_DOWN)
                                {
                                    // only allowed if remote block has matching connector
                                    if (blockD.connectionDir != Form1.K_DIR_UP)
                                        return false;
                                    else
                                    {
                                        //MessageBox.Show("Random connection found DOWN!");
                                    }
                                }
                            }
                        }
                        // LEFT
                        if (xx == 0)
                        {
                            Form1.CGridCell blockL = GetPlacedBlockAt(new Point(xx + vPos.X - 1, yy + vPos.Y));
                            if ((blockL != null) && (blockL.bFilled))
                            {
                                // remote blocked connection
                                if ((blockL.connectionDir == Form1.K_DIR_RIGHT) && (block.connectionDir != Form1.K_DIR_LEFT))
                                    return false;
                                // local block blocked connection
                                if (block.connectionDir == Form1.K_DIR_LEFT)
                                {
                                    // only allowed if remote block has matching connector
                                    if (blockL.connectionDir != Form1.K_DIR_RIGHT)
                                        return false;
                                    else
                                    {
                                        //MessageBox.Show("Random connection found DOWN!");
                                    }
                                }
                            }
                        }
                        // RIGHT
                        if (xx == AABBtest.Width - 1)
                        {
                            Form1.CGridCell blockR = GetPlacedBlockAt(new Point(xx + vPos.X + 1, yy + vPos.Y));
                            if ((blockR != null) && (blockR.bFilled))
                            {
                                // remote blocked connection
                                if ((blockR.connectionDir == Form1.K_DIR_LEFT) && (block.connectionDir != Form1.K_DIR_RIGHT))
                                    return false;
                                // local block blocked connection
                                if (block.connectionDir == Form1.K_DIR_RIGHT)
                                {
                                    // only allowed if remote block has matching connector
                                    if (blockR.connectionDir != Form1.K_DIR_LEFT)
                                        return false;
                                    else
                                    {
                                        //MessageBox.Show("Random connection found DOWN!");
                                    }
                                }
                            }
                        }
                    }
                }
            }
            return true;
        }

        public bool PlaceRandomArea(ArrayList inventory, CPlacedArea parent, CPlacedArea.CAreaConnector parentConn, int nGeneration)
        {
            int nDirFlag = Form1.DIRFLAG_ANY;
            if (parentConn.dir == Form1.K_DIR_LEFT) nDirFlag = Form1.DIRFLAG_RIGHT;
            if (parentConn.dir == Form1.K_DIR_UP) nDirFlag = Form1.DIRFLAG_DOWN;
            if (parentConn.dir == Form1.K_DIR_RIGHT) nDirFlag = Form1.DIRFLAG_LEFT;
            if (parentConn.dir == Form1.K_DIR_DOWN) nDirFlag = Form1.DIRFLAG_UP;

            Point vDirOff = Form1.DIR_OFFSET(parentConn.dir);
            Point vStitchPt = new Point(parentConn.pos.X + parent.AABB.X, parentConn.pos.Y + parent.AABB.Y);
            vStitchPt.X += vDirOff.X; vStitchPt.Y += vDirOff.Y;

            ArrayList availableList = FilterAreas(inventory, 2, 5, nDirFlag);
            ShuffleList(availableList);

            foreach (CInventoryArea iarea in availableList)
            {
                // find position of connection point
                Point tryConnPt;
                int tryConnDir = Form1.INVERSE_DIR(parentConn.dir);
                //#TODO: sa ia o lista cu toti conectorii posibili si sa le faca shuffle si sa ii incerce pe rand, si sa ii si testeze pe rand daca se conecteaza corect
                if (iarea.area.GetConnectorPos(tryConnDir, out tryConnPt))
                {
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


        public bool GenerateLevel(ArrayList inventory, int maxDepth)
        {
            m_arrPlaced.RemoveRange(0, m_arrPlaced.Count);
            
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


                int tries = 0;
                bool bFinished = false;
                int nMaxDepth = maxDepth;
                int nCurrGeneration = 0;

                for (int nDepth = 0; nDepth < nMaxDepth; nDepth++)
                {
                    bool bGenerationPlaced = true;
                    // for each placed area of current generation:
                    int nPlacedCnt = m_arrPlaced.Count; //save count before, it grows
                    for (int kk = 0; kk < nPlacedCnt; kk++)
                    {
                        // find placed area
                        CPlacedArea placed = m_arrPlaced[kk] as CPlacedArea;
                        if (placed.nGeneration != nCurrGeneration)
                            continue;
                        // get the shuffled connectors
                        ArrayList arrConn = new ArrayList();
                        for (int ncon = 0; ncon < placed.arrConnections.Count; ncon++)
                        {
                            // only add not connected connectors
                            if (placed.arrConnections[ncon].pConnectedArea == null)
                                arrConn.Add(placed.arrConnections[ncon]);
                        }
                        ShuffleList(arrConn);
                        // take connectors one by one:
                        for (int ncon = 0; ncon < arrConn.Count; ncon++)
                        {
                            CPlacedArea.CAreaConnector curcon = arrConn[ncon] as CPlacedArea.CAreaConnector;
                            bool bPlaced = PlaceRandomArea(inventory, placed, curcon, placed.nGeneration + 1);
                            if (!bPlaced)
                            {
                                //MessageBox.Show("Could not place area!");
                                bGenerationPlaced = false;
                            }
                        }
                    }

                    // AL GOOD, prepare next generation
                    if(bGenerationPlaced == true)
                        nCurrGeneration++;
                }
            }


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

            return true;
        }
    }
}
