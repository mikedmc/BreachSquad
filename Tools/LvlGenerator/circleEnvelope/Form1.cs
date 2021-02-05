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

        // direction flags
        public const int K_DIR_LEFT = 1;
        public const int K_DIR_UP = 2;
        public const int K_DIR_RIGHT = 4;
        public const int K_DIR_DOWN = 8;
        //--- ZONELE DE INFLUENTA ---
        public class CGridCell
        {

            public bool bFilled;
            public bool bConnected;
            public int connectionFlag;

            public CGridCell()
            {
                bFilled = false;
                connectionFlag = 0;
            }

            public void Reset()
            {
                bFilled = false;
                bConnected = false;
                connectionFlag = 0;
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

            public void ComputeConnectionFlags()
            {
                int vMinX = size.Width, vMinY = size.Height;
                int vMaxX = 0, vMaxY = 0;
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
                        // set flags
                        blocks[xx][yy].connectionFlag = 0;

                        if (blocks[xx][yy].bConnected)
                        {
                            CGridCell block = blocks[xx][yy];
                            if (blocks[xx][yy - 1].bFilled == false)
                                block.connectionFlag |= K_DIR_UP;
                            if (blocks[xx][yy + 1].bFilled == false)
                                block.connectionFlag |= K_DIR_DOWN;
                            if (blocks[xx - 1][yy].bFilled == false)
                                block.connectionFlag |= K_DIR_LEFT;
                            if (blocks[xx + 1][yy].bFilled == false)
                                block.connectionFlag |= K_DIR_RIGHT;
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
                        bw.Write(blocks[xx][yy].bConnected);
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
                        blocks[xx][yy].bConnected = br.ReadBoolean();
                    }
                }
            }
        }
        //areas collection
        ArrayList m_arrAreas = new ArrayList();

        // current tool
        public enum ETool : int
        {
            K_TOOL_AREA = 1,
            K_TOOL_GENERATOR = 2,
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

        public void ToggleGridConnected(int x, int y)
        {
            if (m_area == null)
                return;
            if ((x < 0) || (x >= gridW) || (y < 0) || (y >= gridH))
                return;
            if(m_area.blocks[x][y].bFilled)
                m_area.blocks[x][y].bConnected = !m_area.blocks[x][y].bConnected;
        }

        public void SetGridFlag(int x, int y, int nDirFlag)
        {
            if (m_area == null)
                return;
            if ((x < 0) || (x >= gridW) || (y < 0) || (y >= gridH))
                return;

            m_area.blocks[x][y].connectionFlag = nDirFlag;
        }

        //paint grid
        void PaintGrid(Graphics gr)
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
                        if (m_area.blocks[xx][yy].bConnected)
                            gr.FillRectangle(Brushes.DarkRed, xx * g_nGridSize, yy * g_nGridSize, g_nGridSize, g_nGridSize);
                        else if (m_area.blocks[xx][yy].bFilled)
                            gr.FillRectangle(Brushes.DarkGreen, xx * g_nGridSize, yy * g_nGridSize, g_nGridSize, g_nGridSize);

                        int nDirFlags = m_area.blocks[xx][yy].connectionFlag;
                        if ((nDirFlags & K_DIR_UP) != 0)
                            gr.DrawLine(Pens.Red, xx * g_nGridSize, yy * g_nGridSize, (xx + 1) * g_nGridSize, yy * g_nGridSize);
                        if ((nDirFlags & K_DIR_DOWN) != 0)
                            gr.DrawLine(Pens.Red, xx * g_nGridSize, (yy+1) * g_nGridSize, (xx + 1) * g_nGridSize, (yy+1) * g_nGridSize);
                        if ((nDirFlags & K_DIR_LEFT) != 0)
                            gr.DrawLine(Pens.Red, xx * g_nGridSize, yy * g_nGridSize, xx * g_nGridSize, (yy + 1) * g_nGridSize);
                        if ((nDirFlags & K_DIR_RIGHT) != 0)
                            gr.DrawLine(Pens.Red, (xx + 1) * g_nGridSize, yy * g_nGridSize, (xx + 1) * g_nGridSize, (yy + 1) * g_nGridSize);
                    }
                }

                if ((m_area.AABB.Width > 0) && (m_area.AABB.Height > 0))
                {
                    gr.DrawRectangle(Pens.DarkMagenta, new Rectangle(m_area.AABB.X * g_nGridSize - 1, m_area.AABB.Y * g_nGridSize - 1, m_area.AABB.Width * g_nGridSize + 2, m_area.AABB.Height * g_nGridSize + 2));
                }

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

            m_area = AddNewArea();

            repaintArea(pbgr);
        }

        void repaintArea(Graphics gr)
        {
            gr.Clear(Color.Black);

            PaintGrid(gr);
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

        public CAreaDesc AddNewArea()
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
                }
            }
            area.strName = "A" + area.ID + "_" + area.size.Width + "x" + area.size.Height;

            m_arrAreas.Add(area);
            RefreshAreasList(m_arrAreas.Count - 1);
            return area;
        }

        public void UpdateArea(CAreaDesc area)
        {
            /*
            int vMinX = gridW, vMinY = gridH;
            int vMaxX = 0, vMaxY = 0;
            int blocks = 0;
            for (int yy = 0; yy < gridH; yy++)
            {
                for (int xx = 0; xx < gridW; xx++)
                {
                    if (!tiles[xx][yy].bFilled)
                        continue;
                    blocks++;
                    if (xx > vMaxX) vMaxX = xx;
                    if (xx < vMinX) vMinX = xx;
                    if (yy > vMaxY) vMaxY = yy;
                    if (yy < vMinY) vMinY = yy;
                }
            }

            if (blocks == 0)
            {
                MessageBox.Show("Draw at least one block!");
                return;
            }

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
                    area.blocks[xx][yy] = tiles[vMinX + xx][vMinY + yy];
                }
            }
            //#TODO: should parse connection blocks and set flags

            area.strName = "A" + area.ID + "_" + area.size.Width + "x" + area.size.Height;

            m_arrAreas.Add(area);
            RefreshAreasList(m_arrAreas.Count - 1);
            */
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

        public void SaveAreas(String strPath)
        {

        }

        public void LoadAreas(String strPath)
        {
        }

        private void pictureBox1_MouseDown(object sender, MouseEventArgs e)
        {
            int mx = (int)(e.X / g_nGridSize);
            int my = (int)(e.Y / g_nGridSize);
            if (e.Button == MouseButtons.Left)
            {
                SetGridFilled(mx, my, true);

                repaintArea(pbgr);
                pictureBox1.Refresh();
            }

            //daca dai click dreapta e cancel
            if (e.Button == MouseButtons.Right)
            {
                SetGridFilled(mx, my, false);

                repaintArea(pbgr);
                pictureBox1.Refresh();
            }

            if (e.Button == MouseButtons.Middle)
            {
                ToggleGridConnected(mx, my);

                repaintArea(pbgr);
                pictureBox1.Refresh();
            }
        }

        private void pictureBox1_MouseUp(object sender, MouseEventArgs e)
        {
            if (e.Button == MouseButtons.Left)
            {
            }
        }

        private void pictureBox1_MouseMove(object sender, MouseEventArgs e)
        {
            int mx = (int)(e.X / g_nGridSize);
            int my = (int)(e.Y / g_nGridSize);
            if (e.Button == MouseButtons.Left)
            {
                SetGridFilled(mx, my, true);

                repaintArea(pbgr);
                pictureBox1.Refresh();
            }

            //daca dai click dreapta e cancel
            if (e.Button == MouseButtons.Right)
            {
                SetGridFilled(mx, my, false);

                repaintArea(pbgr);
                pictureBox1.Refresh();
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

            repaintArea(pbgr);
            pictureBox1.Refresh();
        }

        private void butUpdate_Click(object sender, EventArgs e)
        {
            repaintArea(pbgr);
            pictureBox1.Refresh();
        }

        private void tabControl1_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (tabControl1.SelectedIndex == 0)
            {
                g_eTool = ETool.K_TOOL_AREA;
                g_nGridSize = 32;

                repaintArea(pbgr);
                pictureBox1.Refresh();
            }
            else if (tabControl1.SelectedIndex == 1)
            {
                g_eTool = ETool.K_TOOL_GENERATOR;
                g_nGridSize = 16;

                repaintArea(pbgr);
                pictureBox1.Refresh();
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
                repaintArea(pbgr);
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
                area.ComputeConnectionFlags();
            }

            repaintArea(pbgr);
        }

        private void Form1_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.KeyCode == Keys.D1)
            {

            }
        }
    }
}
