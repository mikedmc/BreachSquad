using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Collections;
using System.Xml;
using System.Xml.Serialization;
using System.IO;

namespace circleEnvelope
{
    public partial class Form1 : Form
    {
        Image pbimg;
        Graphics pbgr;
        // paint origin
        Point vOrigin = new Point(0, 0);
        Point vMouseOld = new Point(0, 0);

        // direction flags
        public const int DIRFLAG_LEFT = 1;
        public const int DIRFLAG_UP = 2;
        public const int DIRFLAG_RIGHT = 4;
        public const int DIRFLAG_DOWN = 8;
        public const int DIRFLAG_ANY = 15;
        // directions
        public const int K_DIR_NONE = 0;
        public const int K_DIR_LEFT = 1;
        public const int K_DIR_UP = 2;
        public const int K_DIR_RIGHT = 3;
        public const int K_DIR_DOWN = 4;

        public static int INVERSE_DIR(int dir)
        {
            switch (dir)
            {
                case K_DIR_LEFT:
                    return K_DIR_RIGHT;
                case K_DIR_UP:
                    return K_DIR_DOWN;
                case K_DIR_RIGHT:
                    return K_DIR_LEFT;
                case K_DIR_DOWN:
                    return K_DIR_UP;
                default:
                    return K_DIR_NONE;
            }
        }

        public static Point DIR_OFFSET(int dir)
        {
            switch (dir)
            {
                case K_DIR_LEFT:
                    return new Point(-1, 0);
                case K_DIR_UP:
                    return new Point(0, -1);
                case K_DIR_RIGHT:
                    return new Point(1, 0);
                case K_DIR_DOWN:
                    return new Point(0, 1);
                default:
                    return new Point(0, 0);
            }
        }
        // reset to 0 after this
        public const int K_DIRS_CNT = 4;

        // level generator class
        CLevelGen g_LevelGen = new CLevelGen();

        //--- ZONELE DE INFLUENTA ---
        public class CGridCell
        {

            public bool bFilled;
            public Int32 connectionDir;

            public CGridCell()
            {
                bFilled = false;
                connectionDir = K_DIR_NONE;
            }

            public void Reset()
            {
                bFilled = false;
                connectionDir = K_DIR_NONE;
            }
        }

        public class CAreaDesc
        {
            public Int32 ID = 0;
            public Size size;
            public CGridCell[][] blocks = null;
            public String strName = "";

            // limits are computed on the fly, do not serialize
            public Rectangle AABB = new Rectangle();
            public List<Point> arrConnectors = new List<Point>();
            public int areaConnDirFlags = 0;

            // returns a vPoint list with all connectors for that direction
            public int GetConnectors(int nDirection, out ArrayList arrConnPos)
            {
                int nCnt = 0;
                arrConnPos = new ArrayList();
                foreach (Point pt in arrConnectors)
                {
                    if (blocks[pt.X][pt.Y].connectionDir == nDirection)
                    {
                        arrConnPos.Add(new Point(pt.X, pt.Y));
                        nCnt++;
                    }
                }
                return nCnt;
            }

            public void ComputeInternalData()
            {
                int vMinX = size.Width, vMinY = size.Height;
                int vMaxX = 0, vMaxY = 0;
                arrConnectors.Clear();
                areaConnDirFlags = 0;
                for (int yy = 0; yy < size.Height; yy++)
                {
                    for (int xx = 0; xx < size.Width; xx++)
                    {
                        if (!blocks[xx][yy].bFilled)
                            continue;
                        // find min max
                        if (xx > vMaxX) vMaxX = xx;
                        if (xx < vMinX) vMinX = xx;
                        if (yy > vMaxY) vMaxY = yy;
                        if (yy < vMinY) vMinY = yy;

                        int nConDir = blocks[xx][yy].connectionDir;
                        if (nConDir != K_DIR_NONE)
                        {
                            arrConnectors.Add(new Point(xx, yy));

                            if (nConDir == K_DIR_LEFT)
                                areaConnDirFlags |= DIRFLAG_LEFT;
                            if (nConDir == K_DIR_UP)
                                areaConnDirFlags |= DIRFLAG_UP;
                            if (nConDir == K_DIR_RIGHT)
                                areaConnDirFlags |= DIRFLAG_RIGHT;
                            if (nConDir == K_DIR_DOWN)
                                areaConnDirFlags |= DIRFLAG_DOWN;
                        }
                    }
                }
                // compute AABB
                AABB.X = vMinX; AABB.Y = vMinY;
                AABB.Width = vMaxX - vMinX + 1;
                AABB.Height = vMaxY - vMinY + 1;
            }

            public void Serialize(BinaryWriter bw)
            {
                bw.Write(ID);
                bw.Write((Int32)size.Width);
                bw.Write((Int32)size.Height);
                bw.Write(strName);
                for (int xx = 0; xx < size.Width; xx++)
                {
                    for (int yy = 0; yy < size.Height; yy++)
                    {
                        bw.Write(blocks[xx][yy].bFilled);
                        bw.Write(blocks[xx][yy].connectionDir);
                    }
                }
            }

            public void Deserialize(BinaryReader br)
            {
                ID = br.ReadInt32();
                size.Width = br.ReadInt32();
                size.Height = br.ReadInt32();
                strName = br.ReadString();

                // allocate
                blocks = new CGridCell[size.Width][];
                for (int xx = 0; xx < size.Width; xx++)
                {
                    blocks[xx] = new CGridCell[size.Height];
                    for (int yy = 0; yy < size.Height; yy++)
                    {
                        blocks[xx][yy] = new CGridCell();
                        blocks[xx][yy].Reset();
                    }
                }

                // read data
                for (int xx = 0; xx < size.Width; xx++)
                {
                    for (int yy = 0; yy < size.Height; yy++)
                    {
                        blocks[xx][yy].bFilled = br.ReadBoolean();
                        blocks[xx][yy].connectionDir = br.ReadInt32();
                    }
                }
            }
        }
        //areas collection
        ArrayList m_arrAreas = new ArrayList();

        // story for the level
        Story g_story = new Story();

        // current tool
        public enum ETool : int
        {
            K_TOOL_AREA = 1,
            K_TOOL_GENERATOR = 2,
            K_TOOL_STORY = 3,
        };
        public ETool g_eTool = ETool.K_TOOL_AREA;

        //grid size
        public int g_nGridSize = 32;
        public int g_nAreaID = 1;

        public CAreaDesc m_area = null;
        public int gridW, gridH;

        public void SetGridFilled(int x, int y, bool bFilled)
        {
            if (m_area == null)
                return;
            if ((x < 0) || (x >= gridW) || (y < 0) || (y >= gridH))
                return;

            m_area.blocks[x][y].bFilled = bFilled;
            if (bFilled == false)
                m_area.blocks[x][y].Reset();
        }

        // changes a block connected state
        public void ToggleGridConnected(int x, int y)
        {
            if (m_area == null)
                return;
            if ((x < 0) || (x >= gridW) || (y < 0) || (y >= gridH))
                return;
            if (m_area.blocks[x][y].bFilled)
            {
                m_area.blocks[x][y].connectionDir++;
                if (m_area.blocks[x][y].connectionDir > K_DIRS_CNT)
                    m_area.blocks[x][y].connectionDir = 0;
            }
        }

        // selects the area from clicking on the graph
        void Story_SelectFromGraph(Point vPos, Story story)
        {
            int areaSize = 32;
            Point spacing = new Point(areaSize, areaSize / 4);
            Point spacingoff = new Point(areaSize + spacing.X, areaSize + spacing.Y);
            Rectangle areaRect = new Rectangle(vOrigin.X, vOrigin.Y, areaSize, areaSize);

            int dx = vPos.X - vOrigin.X;
            int genidx = dx / spacingoff.X;
            if ((dx < 0) || (genidx >= story.arrGenerations.Count))
                return;

            int dy = vPos.Y - vOrigin.Y;
            int areaidx = dy / spacingoff.Y;
            if ((dy < 0) || (areaidx >= story.arrGenerations[genidx].arrEntries.Count))
                return;

            story.nSelGeneration = genidx;
            story.nSelArea = areaidx;

            Story_PopulateGenEntryData();
            RepaintArea(pbgr);
        }

        void DrawStoryTree(Graphics gr, Story story)
        {
            int areaSize = 32;
            Point spacing = new Point(areaSize, areaSize / 4);
            Point spacingoff = new Point(areaSize + spacing.X, areaSize + spacing.Y);
            Rectangle areaRect = new Rectangle(vOrigin.X, vOrigin.Y, areaSize, areaSize);

            int nSelGen = story.nSelGeneration;
            int nSelArea = story.nSelArea;

            Font fnt = new Font("Arial", 9, FontStyle.Regular);

            Point vCur = new Point(vOrigin.X, vOrigin.Y);
            for (int kk = 0; kk < story.arrGenerations.Count; kk++)
            {
                // paint generation numbers
                if (kk == nSelGen)
                    gr.FillRectangle(Brushes.DarkOrange, vOrigin.X + kk * spacingoff.X, vOrigin.Y - 20, spacingoff.X, 18);
                gr.DrawString(kk.ToString(), fnt, Brushes.AliceBlue, vOrigin.X + 5 + kk * spacingoff.X, vOrigin.Y - 20);

                // paint boxes
                int curChild = 0;

                Story.StoryGeneration gen = story.arrGenerations[kk];
                for (int ll = 0; ll < gen.arrEntries.Count; ll++)
                {
                    Story.AreaEntry area = gen.arrEntries[ll];
                    Pen pn = Pens.Gray;

                    areaRect.X = vCur.X; areaRect.Y = vCur.Y;
                    // paint add flag
                    if (area.addFlag != 0)
                    {
                        Brush brf = new SolidBrush(g_story.GetFlagColor(area.addFlag));
                        gr.FillRectangle(brf, areaRect.X, areaRect.Y, 10, 10);
                    }
                    if (area.avoidFlag != 0)
                    {
                        Brush brf = new SolidBrush(g_story.GetFlagColor(area.avoidFlag));
                        gr.FillRectangle(brf, areaRect.X + areaRect.Width - 10, areaRect.Y, 10, 10);
                        gr.DrawLine(Pens.DarkGray, areaRect.X + areaRect.Width - 10, areaRect.Y, areaRect.X + areaRect.Width, areaRect.Y + 10);
                    }
                    // paint node
                    gr.DrawRectangle(pn, areaRect);
                    // selected one
                    if ((kk == nSelGen) && (ll == nSelArea))
                    {
                        gr.DrawRectangle(Pens.GreenYellow, areaRect.X - 2, areaRect.Y - 2, areaRect.Width + 4, areaRect.Height + 4);
                    }
                    // paint links
                    for (int oo = 0; oo < area.nChildren; oo++)
                    {
                        Pen pnl = Pens.Green;
                        gr.DrawLine(pnl, areaRect.Right, areaRect.Y + areaRect.Height / 2, areaRect.Right + spacing.X, vOrigin.Y + areaSize / 2 + curChild * (areaSize + spacing.Y));
                        curChild++;
                    }

                    // advance cursor
                    vCur.Y += spacing.Y + areaSize;
                }

                vCur.X += spacing.X + areaSize;
                vCur.Y = vOrigin.Y;
            }
        }

        void DrawArrow(Graphics gr, PointF vCenter, int nDirection, float fSize)
        {
            Point vOff = DIR_OFFSET(nDirection);
            PointF vDir = new PointF((float)vOff.X * fSize + vCenter.X, (float)vOff.Y * fSize + vCenter.Y);
            gr.DrawLine(Pens.Red, vCenter, vDir);
        }

        //paint grid
        void PaintScene(Graphics gr)
        {
            switch (g_eTool)
            {
                case ETool.K_TOOL_AREA:
                    {
                        Pen pgrey = new Pen(Color.FromArgb(28, 28, 28));
                        for (int kk = 0; kk <= gridW; kk++)
                        {
                            gr.DrawLine(pgrey, kk * g_nGridSize, 0, kk * g_nGridSize, pictureBox1.Height);
                        }
                        for (int ll = 0; ll <= gridH; ll++)
                        {
                            gr.DrawLine(pgrey, 0, ll * g_nGridSize, pictureBox1.Width, ll * g_nGridSize);
                        }

                        if (m_area != null)
                        {
                            for (int xx = 0; xx < gridW; xx++)
                            {
                                for (int yy = 0; yy < gridH; yy++)
                                {
                                    if (m_area.blocks[xx][yy].connectionDir > K_DIR_NONE)
                                        gr.FillRectangle(Brushes.Green, xx * g_nGridSize, yy * g_nGridSize, g_nGridSize, g_nGridSize);
                                    else if (m_area.blocks[xx][yy].bFilled)
                                        gr.FillRectangle(Brushes.DarkGreen, xx * g_nGridSize, yy * g_nGridSize, g_nGridSize, g_nGridSize);

                                    int nDir = m_area.blocks[xx][yy].connectionDir;
                                    if (nDir == K_DIR_UP)
                                        gr.FillRectangle(Brushes.Red, xx * g_nGridSize, yy * g_nGridSize, g_nGridSize, 3);
                                    if (nDir == K_DIR_DOWN)
                                        gr.FillRectangle(Brushes.Red, xx * g_nGridSize, (yy + 1) * g_nGridSize - 3, g_nGridSize, 3);
                                    if (nDir == K_DIR_LEFT)
                                        gr.FillRectangle(Brushes.Red, xx * g_nGridSize, yy * g_nGridSize, 3, g_nGridSize);
                                    if (nDir == K_DIR_RIGHT)
                                        gr.FillRectangle(Brushes.Red, (xx + 1) * g_nGridSize - 3, yy * g_nGridSize, 3, g_nGridSize);
                                }
                            }

                            if ((m_area.AABB.Width > 0) && (m_area.AABB.Height > 0))
                            {
                                gr.DrawRectangle(Pens.DarkMagenta, new Rectangle(m_area.AABB.X * g_nGridSize - 1, m_area.AABB.Y * g_nGridSize - 1, m_area.AABB.Width * g_nGridSize + 2, m_area.AABB.Height * g_nGridSize + 2));
                            }

                        }
                    }
                    break;

                case ETool.K_TOOL_GENERATOR:
                    {
                        Font fnt = new Font("Arial", 8, FontStyle.Regular);
                        int scale = 10;
                        if ((g_LevelGen.m_levelAABB.Width > 0) && (g_LevelGen.m_levelAABB.Height > 0))
                            scale = Math.Min(pictureBox1.Width / g_LevelGen.m_levelAABB.Width, pictureBox1.Height / g_LevelGen.m_levelAABB.Height);

                        int blSize = scale;

                        foreach (CLevelGen.CPlacedArea pa in g_LevelGen.m_arrPlaced)
                        {
                            Point vOrigin = new Point(pa.AABB.X - g_LevelGen.m_levelAABB.X, pa.AABB.Y - g_LevelGen.m_levelAABB.Y);
                            vOrigin.X *= blSize; vOrigin.Y *= blSize;
                            Rectangle paRect = new Rectangle(vOrigin.X, vOrigin.Y, pa.AABB.Width * blSize, pa.AABB.Height * blSize);

                            int nMinLuminance = Math.Max(255 - pa.nGeneration * 15, 40);
                            SolidBrush brcol = new SolidBrush(Color.FromArgb(128, 128, nMinLuminance));
                            Brush brdark = new SolidBrush(Color.FromArgb(128, 40,40,40));

                            gr.FillRectangle(brdark, paRect);
                            for (int xx = 0; xx < pa.AABB.Width; xx++)
                            {
                                for (int yy = 0; yy < pa.AABB.Height; yy++)
                                {
                                    if (pa.blocks[xx, yy].connectionDir != 0)
                                    {
                                        gr.FillRectangle(Brushes.DarkGreen, new Rectangle(vOrigin.X + xx * blSize, vOrigin.Y + yy * blSize, blSize, blSize));
                                        DrawArrow(gr, new PointF(vOrigin.X + xx * blSize + blSize / 2.0f, vOrigin.Y + yy * blSize + blSize / 2.0f),
                                            pa.blocks[xx, yy].connectionDir, blSize / 2.0f);
                                    }
                                    else if (pa.blocks[xx, yy].bFilled)
                                        gr.FillRectangle(brcol, new Rectangle(vOrigin.X + xx * blSize, vOrigin.Y + yy * blSize, blSize, blSize));
                                }
                            }
                            // generation
                            gr.DrawString(pa.nGeneration.ToString(), fnt, Brushes.White, new Point(paRect.X + paRect.Width/2, paRect.Y + paRect.Height/2));
                            gr.DrawRectangle(new Pen(Color.FromArgb(60,60,60)), paRect);
                        }
                    }
                    break;

                case ETool.K_TOOL_STORY:
                    {
                        DrawStoryTree(gr, g_story);
                    }
                    break;
            }
        }

        //---
        public Form1()
        {
            InitializeComponent();

            pbimg = new Bitmap(pictureBox1.Width, pictureBox1.Height);
            pbgr = Graphics.FromImage(pbimg);

            pictureBox1.Image = pbimg;

            //save grtid max size for blocks
            gridW = (int)((float)pictureBox1.Width / g_nGridSize);
            gridH = (int)((float)pictureBox1.Height / g_nGridSize);

            m_area = AddNewArea(null);

            RepaintArea(pbgr);
        }

        void RepaintArea(Graphics gr)
        {
            gr.Clear(Color.Black);

            PaintScene(gr);
            pictureBox1.Refresh();
        }

        CAreaDesc GetAreaByID(int areaID)
        {
            foreach (CAreaDesc area in m_arrAreas)
            {
                if (area.ID == areaID)
                    return area;
            }
            return null;
        }

        public CAreaDesc AddNewArea(CAreaDesc pClone = null)
        {
            // save it now
            CAreaDesc area = new CAreaDesc();
            area.ID = g_nAreaID;
            g_nAreaID++;
            area.size.Width = gridW; // vMaxX - vMinX + 1;
            area.size.Height = gridH; // vMaxY - vMinY + 1;
            area.blocks = new CGridCell[area.size.Width][];
            for (int xx = 0; xx < area.size.Width; xx++)
            {
                area.blocks[xx] = new CGridCell[area.size.Height];
                for (int yy = 0; yy < area.size.Height; yy++)
                {
                    area.blocks[xx][yy] = new CGridCell();
                    area.blocks[xx][yy].Reset();

                    if (pClone != null)
                    {
                        area.blocks[xx][yy].bFilled = pClone.blocks[xx][yy].bFilled;
                        area.blocks[xx][yy].connectionDir = pClone.blocks[xx][yy].connectionDir;
                    }
                }
            }
            area.strName = "A" + area.ID + "_" + area.size.Width + "x" + area.size.Height;

            m_arrAreas.Add(area);
            RefreshAreasList(m_arrAreas.Count - 1);
            return area;
        }


        // -1 to keep old selection
        public void RefreshAreasList(int selectedIdx)
        {
            int nsel = lbAreas.SelectedIndex;
            if (selectedIdx >= 0)
                nsel = selectedIdx;

            lbAreas.Items.Clear();
            foreach (CAreaDesc area in m_arrAreas)
            {
                lbAreas.Items.Add(area.strName);
            }
            lbAreas.SelectedIndex = nsel;
        }


        private void pictureBox1_MouseDown(object sender, MouseEventArgs e)
        {
            switch (g_eTool)
            {
                case ETool.K_TOOL_AREA:
                    {
                        int mx = (int)(e.X / g_nGridSize);
                        int my = (int)(e.Y / g_nGridSize);

                        if (e.Button == MouseButtons.Left)
                        {
                            SetGridFilled(mx, my, true);

                            RepaintArea(pbgr);
                            pictureBox1.Refresh();
                        }

                        if (e.Button == MouseButtons.Right)
                        {
                            SetGridFilled(mx, my, false);

                            RepaintArea(pbgr);
                            pictureBox1.Refresh();
                        }

                        if (e.Button == MouseButtons.Middle)
                        {
                            ToggleGridConnected(mx, my);

                            RepaintArea(pbgr);
                            pictureBox1.Refresh();
                        }
                    }
                    break;
                case ETool.K_TOOL_STORY:
                    {
                        // select area
                        if (e.Button == MouseButtons.Left)
                        {
                            Story_SelectFromGraph(new Point(e.X, e.Y), g_story);
                        }
                    }
                    break;
            }
        }

        private void pictureBox1_MouseUp(object sender, MouseEventArgs e)
        {
        }

        private void pictureBox1_MouseMove(object sender, MouseEventArgs e)
        {
            Point vDelta = new Point(e.X - vMouseOld.X, e.Y - vMouseOld.Y);
            vMouseOld.X = e.X; vMouseOld.Y = e.Y;

            switch (g_eTool)
            {
                case ETool.K_TOOL_AREA:
                    {
                        int mx = (int)(e.X / g_nGridSize);
                        int my = (int)(e.Y / g_nGridSize);

                        if (e.Button == MouseButtons.Left)
                        {
                            SetGridFilled(mx, my, true);

                            RepaintArea(pbgr);
                            pictureBox1.Refresh();
                        }

                        //daca dai click dreapta e cancel
                        if (e.Button == MouseButtons.Right)
                        {
                            SetGridFilled(mx, my, false);

                            RepaintArea(pbgr);
                            pictureBox1.Refresh();
                        }
                    }
                    break;

                case ETool.K_TOOL_STORY:
                    {
                        if (e.Button == MouseButtons.Middle)
                        {
                            vOrigin.X += vDelta.X;
                            vOrigin.Y += vDelta.Y;

                            RepaintArea(pbgr);
                            pictureBox1.Refresh();
                        }
                    }
                    break;
            }
        }

        private void butClear_Click(object sender, EventArgs e)
        {
            if (m_area == null)
                return;

            for (int kk = 0; kk < gridW; kk++)
            {
                for (int ii = 0; ii < gridH; ii++)
                {
                    m_area.blocks[kk][ii].Reset();
                }
            }

            RepaintArea(pbgr);
            pictureBox1.Refresh();
        }

        private void tabControl1_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (tabControl1.SelectedIndex == 0)
            {
                g_eTool = ETool.K_TOOL_AREA;

                RepaintArea(pbgr);
            }
            else if (tabControl1.SelectedIndex == 1)
            {
                g_eTool = ETool.K_TOOL_GENERATOR;

                RepaintArea(pbgr);
            }
            else if (tabControl1.SelectedIndex == 2)
            {
                g_eTool = ETool.K_TOOL_STORY;
                vOrigin.X = 16;
                vOrigin.Y = 32;

                RepaintArea(pbgr);
            }
        }

        private void butSaveArea_Click(object sender, EventArgs e)
        {
            m_area = AddNewArea();
        }

        private void but_DelArea_Click(object sender, EventArgs e)
        {
            if (m_arrAreas.Count <= 1)
                return;
            if (lbAreas.SelectedIndex >= 0)
            {
                m_arrAreas.RemoveAt(lbAreas.SelectedIndex);
                lbAreas.SelectedIndex = 0;
                RefreshAreasList(0);
            }
        }

        private void lbAreas_MouseDoubleClick(object sender, MouseEventArgs e)
        {
            int index = lbAreas.IndexFromPoint(e.Location);
            if (index != System.Windows.Forms.ListBox.NoMatches)
            {
                MessageBox.Show(index.ToString());
            }
        }

        private void lbAreas_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (lbAreas.SelectedIndex >= 0)
            {
                m_area = m_arrAreas[lbAreas.SelectedIndex] as CAreaDesc;
                tb_areaName.Text = m_area.strName;
                RepaintArea(pbgr);
            }
        }

        private void tb_areaName_TextChanged(object sender, EventArgs e)
        {
            if (m_area == null)
                return;
            m_area.strName = tb_areaName.Text;
            RefreshAreasList(-1);
        }

        private void saveAreasToolStripMenuItem_Click(object sender, EventArgs e)
        {
            SaveFileDialog dlg = new SaveFileDialog();
            dlg.Filter = "AREAS File (*.areas)|*.areas||";
            dlg.DefaultExt = "areas";
            if (dlg.ShowDialog() != DialogResult.Cancel)
            {
                try
                {
                    FileStream pak = File.OpenWrite(dlg.FileName);
                    BinaryWriter bw = new BinaryWriter(pak);

                    Int32 arrcount = m_arrAreas.Count;
                    bw.Write(arrcount);

                    foreach (CAreaDesc ad in m_arrAreas)
                    {
                        ad.Serialize(bw);
                    }

                    bw.Flush();
                    bw.Close();
                    pak.Close();
                }
                catch (Exception ex)
                {
                    MessageBox.Show("Error exporting areas: " + ex.Message);
                    return;
                }

                MessageBox.Show("Areas exported in: " + dlg.FileName);
            }
        }

        private void loadAreasToolStripMenuItem_Click(object sender, EventArgs e)
        {
            OpenFileDialog dlg = new OpenFileDialog();
            dlg.Filter = "AREAS File (*.areas)|*.areas||";
            dlg.DefaultExt = "areas";
            if (dlg.ShowDialog() != DialogResult.Cancel)
            {
                // remove them all
                m_arrAreas.RemoveRange(0, m_arrAreas.Count);

                try
                {
                    FileStream pak = File.OpenRead(dlg.FileName);
                    BinaryReader bw = new BinaryReader(pak);

                    Int32 arrcount = 0;
                    arrcount = bw.ReadInt32();

                    for (int kk = 0; kk < arrcount; kk++)
                    {
                        CAreaDesc ad = new CAreaDesc();
                        ad.Deserialize(bw);
                        ad.ComputeInternalData();
                        m_arrAreas.Add(ad);
                    }
                    bw.Close();
                    pak.Close();

                    RefreshAreasList(0);
                }
                catch (Exception ex)
                {
                    MessageBox.Show("Error loading areas: " + ex.Message);
                    return;
                }

                MessageBox.Show("Areas loaded from: " + dlg.FileName);
            }
        }

        private void but_ComputeFlags_Click(object sender, EventArgs e)
        {
            if (lbAreas.SelectedIndex >= 0)
            {
                CAreaDesc area = m_arrAreas[lbAreas.SelectedIndex] as CAreaDesc;
                area.ComputeInternalData();
            }

            RepaintArea(pbgr);
        }

        private void butGenerate_Click(object sender, EventArgs e)
        {
            ArrayList inventory = new ArrayList();

            foreach (CAreaDesc ad in m_arrAreas)
            {
                // make sure we have everything computed
                ad.ComputeInternalData();

                CLevelGen.CInventoryArea ia = new CLevelGen.CInventoryArea();
                ia.area = ad;
                ia.nAvailable = 10;
                ia.nConsumed = 0;
                inventory.Add(ia);
            }

            g_LevelGen.GenerateLevel(inventory, (int)numGenerations.Value);

            RepaintArea(pbgr);
        }

        private void butCloneArea_Click(object sender, EventArgs e)
        {
            m_area = AddNewArea(m_area);

        }

        private void Form1_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.KeyCode == Keys.D1)
            {

            }
        }

        private void lbStory_SelectedIndexChanged(object sender, EventArgs e)
        {
            Story_PopulateGenEntryData();
        }

        private void but_addStoryGen_Click(object sender, EventArgs e)
        {
            g_story.AddGeneration();

            Story_PopulateGenEntryData();
        }

        private void wa_butAddArea_Click(object sender, EventArgs e)
        {
            if ((g_story.nSelGeneration < 0) || (g_story.nSelGeneration >= g_story.arrGenerations.Count))
            {
                return;
            }

            g_story.AddGenerationEntry(g_story.nSelGeneration, 1, 0, 0, "");
            Story_PopulateGenEntryData();
            RepaintArea(pbgr);
        }

        private bool Story_IsGenAreaSelected(Story story)
        {
            int nGenIdx = story.nSelGeneration;
            int nEntryIdx = story.nSelArea;
            if ((nGenIdx < 0) || (nGenIdx >= g_story.arrGenerations.Count))
                return false;
            else if ((nEntryIdx < 0) || (nEntryIdx >= g_story.arrGenerations[nGenIdx].arrEntries.Count))
                return false;

            return true;
        }

        private void Story_PopulateGenEntryData()
        {
            bool bEnabled = Story_IsGenAreaSelected(g_story);
            if(!bEnabled)
            {
                groupBox_storyarea.Enabled = false;

                //disable
                groupBox_storyarea.Enabled = false;

                RepaintArea(pbgr);
                return;
            }

            int nGenIdx = g_story.nSelGeneration;
            int nEntryIdx = g_story.nSelArea;

            //enable
            groupBox_storyarea.Enabled = true;

            Story.AreaEntry entry = g_story.arrGenerations[nGenIdx].arrEntries[nEntryIdx];
            wa_numChildren.Value = entry.nChildren;
            wa_numAddFlag.Value = entry.addFlag;
            wa_numAvoidFlag.Value= entry.avoidFlag;
            wa_tbTags.Text = entry.tags_any;

            RepaintArea(pbgr);
        }

        private void wa_numChildren_ValueChanged(object sender, EventArgs e)
        {
            if (!Story_IsGenAreaSelected(g_story))
                return;
            int nGenIdx = g_story.nSelGeneration;
            int nEntryIdx = g_story.nSelArea;

            Story.AreaEntry entry = g_story.arrGenerations[nGenIdx].arrEntries[nEntryIdx];
            entry.nChildren = (int)wa_numChildren.Value;

            RepaintArea(pbgr);
        }

        private void wa_tbTags_TextChanged(object sender, EventArgs e)
        {
            if (!Story_IsGenAreaSelected(g_story))
                return;
            int nGenIdx = g_story.nSelGeneration;
            int nEntryIdx = g_story.nSelArea;

            Story.AreaEntry entry = g_story.arrGenerations[nGenIdx].arrEntries[nEntryIdx];
            entry.tags_any = wa_tbTags.Text;
        }

        private void wa_numAddFlag_ValueChanged(object sender, EventArgs e)
        {
            if (!Story_IsGenAreaSelected(g_story))
                return;
            int nGenIdx = g_story.nSelGeneration;
            int nEntryIdx = g_story.nSelArea;

            Story.AreaEntry entry = g_story.arrGenerations[nGenIdx].arrEntries[nEntryIdx];
            entry.addFlag = (int)wa_numAddFlag.Value;

            RepaintArea(pbgr);
        }

        private void wa_numAvoidFlag_ValueChanged(object sender, EventArgs e)
        {
            if (!Story_IsGenAreaSelected(g_story))
                return;
            int nGenIdx = g_story.nSelGeneration;
            int nEntryIdx = g_story.nSelArea;

            Story.AreaEntry entry = g_story.arrGenerations[nGenIdx].arrEntries[nEntryIdx];
            entry.avoidFlag = (int)wa_numAvoidFlag.Value;

            RepaintArea(pbgr);
        }

        private void wa_butDelEntry_Click(object sender, EventArgs e)
        {
            if (!Story_IsGenAreaSelected(g_story))
                return;
            int nGenIdx = g_story.nSelGeneration;
            int nEntryIdx = g_story.nSelArea;

            g_story.DeleteGenerationEntry(nGenIdx, nEntryIdx);
            Story_PopulateGenEntryData();
            RepaintArea(pbgr);
        }

        private void but_generateFromStory_Click(object sender, EventArgs e)
        {
            g_story.SortGenerations();
            RepaintArea(pbgr);
        }

        private void but_GenFromStory_Click(object sender, EventArgs e)
        {

        }

        private void saveStoryToolStripMenuItem_Click(object sender, EventArgs e)
        {
            SaveFileDialog dlg = new SaveFileDialog();
            dlg.Filter = "XML Level Story (*.story)|*.story||";
            dlg.DefaultExt = "story";
            if (dlg.ShowDialog() == DialogResult.Cancel)
                return;

            try
            {
                XmlTextWriter xw = new XmlTextWriter(dlg.FileName, null);
                xw.Formatting = Formatting.Indented;
                xw.WriteStartDocument();
                // write elements
                xw.WriteStartElement("LevelStory");
                xw.WriteStartAttribute("Generations");
                xw.WriteValue(g_story.arrGenerations.Count);
                xw.WriteEndAttribute();

                for (int kk = 0; kk < g_story.arrGenerations.Count; kk++)
                {
                    Story.StoryGeneration gen = g_story.arrGenerations[kk];
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
                        xw.WriteAttributeString("AddFlag", ae.addFlag.ToString());
                        xw.WriteAttributeString("AvoidFlag", ae.avoidFlag.ToString());
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

    }
}
