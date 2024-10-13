using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace InkEditor
{
    public partial class FontWnd : Form
    {
        public InkEd3.Sprite spr;
        public List<int> modIdxs;

        Graphics pbGr = null;
        Color pbSampleGridColor = Color.DarkGray;

        public FontWnd()
        {
            InitializeComponent();

            pbSample.Image = new Bitmap(pbSample.Width, pbSample.Height);
            pbGr = Graphics.FromImage(pbSample.Image);
        }

        void RepaintTextSample(Graphics gr, int w, int h)
        {
            if ((spr == null) || (gr == null))
                return;

            DrawString(pbGr, modIdxs, pbSample.Width, pbSample.Height);
            pbSample.Refresh();
        }

        private void button1_Click(object sender, EventArgs e)
        {
            InkEd3.FontData fd = new InkEd3.FontData();
            fd.ID = IDBox.Text;
            fd.LetterSpacing = Convert.ToInt32(LetterSpacing.Value);
            fd.RowHeight = Convert.ToInt32(RowHeight.Value);
            fd.RowSpacing = Convert.ToInt32(RowSpacing.Value);
            fd.SpaceSize = Convert.ToInt32(SpaceSize.Value);

            InkEd3.Sprite.fontData = fd;
            InkEd3.Sprite.saveFontData = checkBox1.Checked;

            this.Close();
        }

        private void FontWnd_Load(object sender, EventArgs e)
        {
            if (InkEd3.Sprite.fontData == null) return;

            checkBox1.Checked = InkEd3.Sprite.saveFontData;

            IDBox.Text = InkEd3.Sprite.fontData.ID;
            LetterSpacing.Value = InkEd3.Sprite.fontData.LetterSpacing;
            RowHeight.Value = InkEd3.Sprite.fontData.RowHeight;
            RowSpacing.Value = InkEd3.Sprite.fontData.RowSpacing;
            SpaceSize.Value = InkEd3.Sprite.fontData.SpaceSize;

            button2_Click(null, null);
        }

        private void textBox1_TextChanged(object sender, EventArgs e)
        {
            RepaintTextSample(pbGr, pbSample.Width, pbSample.Height);
        }

        private void DrawString(Graphics g, List<int> s, int nWidth, int nHeight)
        {
            if (s == null) return;

            int lposx = 0, lposy = 2 * Convert.ToInt32(RowHeight.Value);
            Pen p = new Pen(Color.Gray, 1);

            //clear
            if(chk_invert.Checked)
                g.Clear(Color.FromArgb(16, 16, 16));
            else
                g.Clear(Color.FromArgb(240, 240, 240));

            for (int ii = 0; ii < s.Count(); ii++)
            {
                int idx = s[ii];
                //daca e spatiu
                if (idx < 0)
                {
                    lposx += Convert.ToInt32(SpaceSize.Value);
                    continue;
                }

                InkEd3.Frame frm = spr.frames[idx];

                if (frm.fmodules.Count == 0) continue;

                InkEd3.FrameModule fmod = frm.fmodules[0];

                Rectangle BBox = frm.BBox;
                if (frm.BBox.Width == 0 || frm.BBox.Height == 0)
                {
                    BBox = new Rectangle(0, 0, fmod.module.w, 0);
                }

                int ox, oy, offx, offy;
                ox = fmod.ox;
                oy = fmod.oy;
                offx = 0;// -BBox.X;
                offy = 0;// -(BBox.Y + BBox.Height);
                
                if (lposx + BBox.Width + Convert.ToInt32(LetterSpacing.Value) >= nWidth - 30)
                {
                    lposx = 0;

                    if (chk_baseline.Checked)
                        g.DrawLine(p, new Point(0, lposy), new Point(nWidth, lposy));

                    lposy += Convert.ToInt32(RowSpacing.Value) + Convert.ToInt32(RowHeight.Value);

                    if (lposy > this.Height) return;
                }

                fmod.module.Paint(g, ox + offx + lposx, oy + offy + lposy);
                lposx += BBox.Width + Convert.ToInt32(LetterSpacing.Value);
            }

            if (chk_baseline.Checked)
                g.DrawLine(p, new Point(0, lposy), new Point(this.Width, lposy));
        }

        private void FontWnd_Paint(object sender, PaintEventArgs e)
        {
            if (spr == null) return;
            DrawString(pbGr, modIdxs, pbSample.Width, pbSample.Height);
        }

        private bool FindIn(bool[] a, bool e)
        {
            for (int ii = 0; ii < a.Length; ii++)
                if (a[ii] == e) return true;
            return false;
        }

        private double MapToInterval(double x, double a, double b)
        {
            return x * (b - a) + a;
        }

        private int GetIdxWithProb(Random ran, int maxv)
        {
            double p = ran.NextDouble();
            int idx;

            do
            {
                idx = ran.Next(maxv);
                if (idx <= 61) p -= MapToInterval(ran.NextDouble(), 0.6, 1.0);
                else p -= MapToInterval(ran.NextDouble(), 0.0, 0.2);
            } while (p > 0.0);

            return idx;
        }

        public const int LETTER_TYPE_MAJUSCULE = 0;
        public const int LETTER_TYPE_MINUSCULE = 1;
        public const int LETTER_TYPE_NUMBER = 2;
        public const int LETTER_TYPE_SYMBOL = 3;

        private int GenerateLetter(int letterType, Random rnd)
        {
            double v1 = rnd.NextDouble();
            double v2 = rnd.NextDouble();

            switch (letterType)
            {
                case LETTER_TYPE_MAJUSCULE:
                    {
                        if (v2 < 0.8f)
                            return (int)(v1 * 25); //normale
                        else
                            return (int)(100 + v1 * (129 - 100)); //speciale
                    }
                default:
                case LETTER_TYPE_MINUSCULE:
                    {
                        if(v2 < 0.8f)
                            return (int)(26 + v1 * (51 - 26)); //normale
                        else
                            return (int)(130 + v1 * (162 - 130)); //speciale
                    }
                case LETTER_TYPE_NUMBER:
                    {
                        return (int)(52 + v1 * (61 - 52));
                    }
                case LETTER_TYPE_SYMBOL:
                    {
                        return (int)(62 + v1 * (99 - 62));
                    }
            }
        }

        private void GenerateWord(List<int> outBuffer, Random rnd)
        {
            //genereaza majuscula sau nu
            double v1 = rnd.NextDouble();
            int wlen = 3 + (int)(v1 * 7);

            if (v1 < 0.2f) //cifre sau simboluri
            {
                v1 = rnd.NextDouble();
                if (v1 < 0.4f) //simboluri
                {
                    for (int kk = 0; kk < wlen; kk++)
                    {
                        int letterIdx = GenerateLetter(LETTER_TYPE_SYMBOL, rnd);
                        outBuffer.Add(letterIdx);
                    }
                }
                else //cifre
                {
                    for (int kk = 0; kk < wlen; kk++)
                    {
                        int letterIdx = GenerateLetter(LETTER_TYPE_NUMBER, rnd);
                        outBuffer.Add(letterIdx);
                    }
                }
            }
            else  //cuvant normal
            {
                for (int kk = 0; kk < wlen; kk++)
                {
                    int letterIdx = 0;

                    v1 = rnd.NextDouble();
                    if (kk == 0)
                    {
                        letterIdx = GenerateLetter(LETTER_TYPE_MAJUSCULE, rnd);
                    }
                    else
                    {
                        letterIdx = GenerateLetter(LETTER_TYPE_MINUSCULE, rnd);
                    }

                    outBuffer.Add(letterIdx);
                }
            }
        }

        //generare text de test
        private void button2_Click(object sender, EventArgs e)
        {
            if (spr == null || spr.modules == null || spr.modules.Count == 0) return;

            Random ran = new Random();
            modIdxs = new List<int>();
            bool[] times = new bool[spr.modules.Count];
            //adauga alfabetul
            for (int idx = 0; idx < spr.modules.Count; idx++)
            {
                modIdxs.Add(idx);
            }
            //adauga spatiu
            modIdxs.Add(-1);
            //adauga 200 de cuvinte
            for (int kk = 0; kk < 200; kk++)
            {
                GenerateWord(modIdxs, ran);
                //adauga spatiu
                modIdxs.Add(-1);
            }
            /*
            while (FindIn(times, false))
            {
                int idx = GetIdxWithProb(ran, spr.modules.Count);
                times[idx] = true;

                modIdxs.Add(idx);
            }
            */
            RepaintTextSample(pbGr, pbSample.Width, pbSample.Height);

            this.Refresh();
        }

        private void chk_baseline_CheckedChanged(object sender, EventArgs e)
        {
            RepaintTextSample(pbGr, pbSample.Width, pbSample.Height);
        }

        private void pbSample_SizeChanged(object sender, EventArgs e)
        {
            pbSample.Image = new Bitmap(pbSample.Width, pbSample.Height);
            pbGr = Graphics.FromImage(pbSample.Image);

            RepaintTextSample(pbGr, pbSample.Width, pbSample.Height);
        }

        private void chk_invert_CheckedChanged(object sender, EventArgs e)
        {
            RepaintTextSample(pbGr, pbSample.Width, pbSample.Height);
        }
    }
}
