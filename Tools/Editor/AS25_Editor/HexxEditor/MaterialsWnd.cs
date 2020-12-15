using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Drawing.Drawing2D;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.IO;

namespace HexxEditor
{
    public partial class MaterialsWnd : Form
    {
        Form1 parentWnd;
///--- VARIABILE ENGINE ---
        //marimea tilesetului se stabileste aici in fereastra de materiale
        public int TILE_W = 16;
        public int TILE_H = 16;
        public int TILESET_COLUMNS = 0;

        public Graphics pbGr = null;
        public Image g_TilesetImg = null;
        public String g_TilesetName = "";

        public Rectangle g_brush; //in coordonate tiles
        
        float zoom = 2.0f;
        PointF scroll = new PointF(0.0f, 0.0f);

/// --- VARIABILE AJUTATOARE ---
        public Pen g_penGreenDotted;
        public Pen g_penDotted;
        public Point g_clickPos = new Point(0, 0);
        public Point g_releasePos = new Point(0, 0);
        //hover
        public Point g_hoveredTile = new Point(0, 0);

        public bool b_settingBrush = false;

        public MaterialsWnd(Form1 parent)
        {
            InitializeComponent();

            parentWnd = parent;
            b_settingBrush = false;

            HatchBrush aHatchBrush = new HatchBrush(HatchStyle.Plaid, Color.Red, Color.Green);
            g_penGreenDotted = new Pen(aHatchBrush);
            aHatchBrush = new HatchBrush(HatchStyle.DarkDownwardDiagonal, Color.DarkGray);
            g_penDotted = new Pen(aHatchBrush);
            //init image and graphics
            pbTileset.Image = new Bitmap(pbTileset.Width, pbTileset.Height);
            pbGr = Graphics.FromImage(pbTileset.Image);
            pbGr.SmoothingMode = System.Drawing.Drawing2D.SmoothingMode.None;
            pbGr.InterpolationMode = InterpolationMode.NearestNeighbor;
            pbGr.PixelOffsetMode = PixelOffsetMode.HighQuality;

            g_brush.X = g_brush.Y = 0;
            g_brush.Width = g_brush.Height = 1;
            //transport data to parent
            parentWnd.SetMaterialData(TILE_W, TILE_H, 0, 0);
            parentWnd.SetMaterialBrush(g_brush);

            Repaint();
        }

        public void Repaint()
        {
            if (pbGr == null)
                return;

            //fundal intunecat
            if (chk_Invert.Checked)
                pbGr.Clear(Color.FromArgb(180, 180, 180));
            else
                pbGr.Clear(Color.FromArgb(40, 40, 40));
            
            if (g_TilesetImg == null)
            {
                pbGr.DrawString("Load a tileset first! File->Load Tileset...", new Font("Arial", 10), Brushes.LightBlue, 10, 10);
                return;
            }
            //paint tileset
            pbGr.ResetTransform();
            pbGr.ScaleTransform(zoom, zoom);
            pbGr.DrawImage(g_TilesetImg, scroll.X / zoom, scroll.Y / zoom);
            pbGr.ResetTransform();
            //grid
            if (chk_Grid.Checked)
            {
                for (int kk = 0; kk < g_TilesetImg.Width / TILE_W; kk++)
                {
                    pbGr.DrawLine(g_penDotted, scroll.X + kk * TILE_W * zoom, scroll.Y, scroll.X + kk * TILE_W * zoom, scroll.Y + g_TilesetImg.Height * zoom);
                }
                for (int kk = 0; kk < g_TilesetImg.Height / TILE_H; kk++)
                {
                    pbGr.DrawLine(g_penDotted, scroll.X, scroll.Y + kk * TILE_H * zoom, scroll.X + g_TilesetImg.Width * zoom, scroll.Y + kk * TILE_H * zoom);
                }
            }
            //limite imagine
            pbGr.DrawLine(Pens.DarkGray, 0, scroll.Y, pbTileset.Width, scroll.Y);
            pbGr.DrawLine(Pens.DarkGray, scroll.X, 0, scroll.X, pbTileset.Height);

            pbGr.DrawLine(Pens.DarkGray, scroll.X + g_TilesetImg.Width * zoom, 0, scroll.X + g_TilesetImg.Width * zoom, pbTileset.Height);
            pbGr.DrawLine(Pens.DarkGray, 0, scroll.Y + g_TilesetImg.Height * zoom, pbTileset.Width, scroll.Y + g_TilesetImg.Height * zoom);

            //brush
            pbGr.DrawRectangle(g_penGreenDotted, scroll.X + g_brush.X * TILE_W * zoom, scroll.Y + g_brush.Y * TILE_H * zoom, g_brush.Width * TILE_W * zoom, g_brush.Height * TILE_H * zoom);

            //move brush
            if (b_settingBrush)
            {
                Rectangle rect = new Rectangle(Math.Min(g_clickPos.X, g_releasePos.X), Math.Min(g_clickPos.Y, g_releasePos.Y), Math.Abs(g_releasePos.X - g_clickPos.X) + 1, Math.Abs(g_releasePos.Y - g_clickPos.Y) + 1);
                pbGr.DrawRectangle(Pens.LightGreen, scroll.X + rect.X * TILE_W * zoom, scroll.Y + rect.Y * TILE_H * zoom, rect.Width * TILE_W * zoom, rect.Height * TILE_H * zoom);
            }

            pbTileset.Refresh();
        }

        public void LoadTileset(String imgPath)
        {
            g_TilesetImg = new Bitmap(imgPath);
            TILESET_COLUMNS = g_TilesetImg.Width / TILE_W;
            parentWnd.SetMaterialData(TILE_W, TILE_H, TILESET_COLUMNS, TILESET_COLUMNS);

            g_TilesetName = Path.GetFileName(imgPath);

            Repaint();
        }

        private void loadTilesetToolStripMenuItem_Click(object sender, EventArgs e)
        {
            OpenFileDialog sfd = new OpenFileDialog();
            //fisier binar
            sfd.Filter = "PNG File (*.png)|*.png|All Files (*.*)|*.*";
            if (sfd.ShowDialog() == DialogResult.Cancel)
                return;

            LoadTileset(sfd.FileName);

            Repaint();
        }

        private void MaterialsWnd_FormClosing(object sender, FormClosingEventArgs e)
        {
            this.Hide();
            e.Cancel = true;
        }

        private void LimitScroll()
        {
            //limit scroll
            if (g_TilesetImg != null)
            {
                if (scroll.X > 64.0f) scroll.X = 64.0f;
                if (scroll.Y > 64.0f) scroll.Y = 64.0f;

                if (scroll.X < -(g_TilesetImg.Width * zoom - pbTileset.Width + 64.0f))
                    scroll.X = -(g_TilesetImg.Width * zoom - pbTileset.Width + 64.0f);
                if (scroll.Y < -(g_TilesetImg.Height * zoom - pbTileset.Height + 64.0f))
                    scroll.Y = -(g_TilesetImg.Height * zoom - pbTileset.Height + 64.0f);
            }
        }

        private void pbTileset_MouseWheel(object sender, MouseEventArgs e)
        {
            //get lookat point
            PointF vZoomPoint = new PointF((pbTileset.Width / 2.0f - scroll.X)/zoom, (pbTileset.Height / 2.0f - scroll.Y)/zoom);
            if (e.Delta < 0)
            {
                zoom -= 0.25f;
                if (zoom < 1.0)
                {
                    zoom = 1.0f;
                    return;
                }
                scroll.X = -vZoomPoint.X * zoom + pbTileset.Width / 2.0f;
                scroll.Y = -vZoomPoint.Y * zoom + pbTileset.Height / 2.0f;
                LimitScroll();
                Repaint();
            }
            else if (e.Delta > 0)
            {
                zoom += 0.25f;
                if (zoom > 10.0)
                {
                    zoom = 10.0f;
                    return;
                }
                scroll.X = -vZoomPoint.X * zoom + pbTileset.Width / 2.0f;
                scroll.Y = -vZoomPoint.Y * zoom + pbTileset.Height / 2.0f;
                LimitScroll();
                Repaint();
            }
        }

        private void pbTileset_Resize(object sender, EventArgs e)
        {
            pbTileset.Image = new Bitmap(pbTileset.Width, pbTileset.Height);
            pbGr = Graphics.FromImage(pbTileset.Image);
            pbGr.SmoothingMode = System.Drawing.Drawing2D.SmoothingMode.None;
            pbGr.InterpolationMode = InterpolationMode.NearestNeighbor;
            pbGr.PixelOffsetMode = PixelOffsetMode.HighQuality;

            LimitScroll();
            Repaint();
        }

        
        private void pbTileset_MouseDown(object sender, MouseEventArgs e)
        {
            g_hoveredTile.X = (int)((int)(e.X - (int)scroll.X) / zoom) / TILE_W;
            g_hoveredTile.Y = (int)((int)(e.Y - (int)scroll.Y) / zoom) / TILE_H;

            //PAN
            if ((e.Button == System.Windows.Forms.MouseButtons.Middle) || (e.Button == System.Windows.Forms.MouseButtons.Right))
            {
                g_clickPos.X = e.X; g_clickPos.Y = e.Y;
            }
            else if (e.Button == System.Windows.Forms.MouseButtons.Left) //move brush
            {
                b_settingBrush = true;
                g_clickPos.X = g_hoveredTile.X; g_clickPos.Y = g_hoveredTile.Y;
                if (g_clickPos.X < 0) g_clickPos.X = 0;
                if (g_clickPos.Y < 0) g_clickPos.Y = 0;
                if (g_clickPos.X > TILESET_COLUMNS - 1) g_clickPos.X = TILESET_COLUMNS - 1;
                if (g_clickPos.Y > TILESET_COLUMNS - 1) g_clickPos.Y = TILESET_COLUMNS - 1;
                g_releasePos = g_clickPos;
                Repaint();
            }
        }

        private void pbTileset_MouseUp(object sender, MouseEventArgs e)
        {
            g_hoveredTile.X = (int)((int)(e.X - (int)scroll.X) / zoom) / TILE_W;
            g_hoveredTile.Y = (int)((int)(e.Y - (int)scroll.Y) / zoom) / TILE_H;

            if (e.Button == System.Windows.Forms.MouseButtons.Left) //move brush
            {
                if (b_settingBrush)
                {
                    b_settingBrush = false;
                    g_releasePos.X = g_hoveredTile.X; g_releasePos.Y = g_hoveredTile.Y;
                    if (g_releasePos.X < 0) g_releasePos.X = 0;
                    if (g_releasePos.Y < 0) g_releasePos.Y = 0;
                    if (g_releasePos.X > TILESET_COLUMNS - 1) g_releasePos.X = TILESET_COLUMNS - 1;
                    if (g_releasePos.Y > TILESET_COLUMNS - 1) g_releasePos.Y = TILESET_COLUMNS - 1;                    

                    Rectangle rect = new Rectangle(Math.Min(g_clickPos.X, g_releasePos.X), Math.Min(g_clickPos.Y, g_releasePos.Y), Math.Abs(g_releasePos.X - g_clickPos.X) + 1, Math.Abs(g_releasePos.Y - g_clickPos.Y) + 1);
                    g_brush = rect;
                    parentWnd.SetMaterialBrush(g_brush);
                }

                LimitScroll();
                Repaint();
            }
        }

        private void pbTileset_MouseMove(object sender, MouseEventArgs e)
        {
            g_hoveredTile.X = (int)((int)(e.X - (int)scroll.X) / zoom) / TILE_W;
            g_hoveredTile.Y = (int)((int)(e.Y - (int)scroll.Y) / zoom) / TILE_H;

            if ((e.Button == System.Windows.Forms.MouseButtons.Middle) || (e.Button == System.Windows.Forms.MouseButtons.Right))
            {
                scroll.X += e.X - g_clickPos.X;
                scroll.Y += e.Y - g_clickPos.Y;
                g_clickPos.X = e.X;
                g_clickPos.Y = e.Y;
                LimitScroll();
                Repaint();
            }
            else if (e.Button == System.Windows.Forms.MouseButtons.Left) //move brush
            {
                if (b_settingBrush)
                {
                    g_releasePos.X = g_hoveredTile.X; g_releasePos.Y = g_hoveredTile.Y;
                    Repaint();
                }
            }
        }

        private void chk_Invert_CheckedChanged(object sender, EventArgs e)
        {
            Repaint();
        }

        private void chk_Grid_CheckedChanged(object sender, EventArgs e)
        {
            Repaint();
        }

        private void but_resetView_Click(object sender, EventArgs e)
        {
            zoom = 2.0f;
            scroll.X = 0.0f;
            scroll.Y = 0.0f;
            Repaint();
        }
    }
}
