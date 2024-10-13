using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.IO;

namespace InkEd3
{
    public partial class FlagsBuilderWnd : Form
    {
        String exePath = "";

        private string ToBinaryString(UInt32 v, int no_octets)
        {
            string txt = "";
            for (int i = 0; i < no_octets * 8; i++)
                txt += ((v >> (no_octets * 8 - 1 - i)) & 1).ToString();
            return txt;
        }

        public class BitsGroup
        {
            private int bitsCnt;
            private int value;
            public String description;

            public BitsGroup(int nrBits)
            {
                bitsCnt = nrBits;
                value = 0;
            }

            //seteaza valoare in decimal
            public void SetValue(int nvalue)
            {
                value = nvalue;
            }
            public int GetValue()
            {
                return value;
            }
            //intoarce nr de 
            public int GetBitsCnt()
            {
                return bitsCnt;
            }
            //intoarce valoarea maxima in decimal
            public int getMaxValue()
            {
                int maxval = 0;
                for(int kk=0; kk<bitsCnt; kk++)
                    maxval += ((int)Math.Pow(2, kk));
                return maxval;
            }
            //intoarce stringul cu bitii in binar
            public String getBitsString()
            {
                string txt = "";
                for (int i = 0; i < bitsCnt; i++)
                    txt += ((value >> (bitsCnt - 1 - i)) & 1).ToString();
                return txt;
            }
        };

        //date controale
        List<BitsGroup> bitsList = new List<BitsGroup>();
        public int totalBytes = 0;

        public FlagsBuilderWnd()
        {
            InitializeComponent();
            exePath = Path.GetDirectoryName(System.Reflection.Assembly.GetExecutingAssembly().GetName().CodeBase);
            exePath = exePath.Substring(6);
            //initializare lista configuratii
            try
            {
                StreamReader strrd = new StreamReader(exePath + "\\Data\\FlagsBuilder\\flags_configs.txt");

                String line;
                while (!strrd.EndOfStream)
                {
                    line = strrd.ReadLine();
                    line = line.Trim().Replace(".txt", "");
                    cbConfigs.Items.Add(line);
                }
                cbConfigs.Text = "--- SELECT CONFIGURATION ---";

                strrd.Close();
            }
            catch (Exception ex)
            {
                MessageBox.Show("Could not read FlagsBuilder configurations !\nSolution: PANIC !!!\n\n"+ex.ToString(), "ERROR !!!", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        private void cbConfigs_SelectedIndexChanged(object sender, EventArgs e)
        {
            ComboBox cb = sender as ComboBox;
            try
            {
                tbFlagValue.Text = "0";

                bitsList.Clear();
                panelControls.Controls.Clear();

                StreamReader strrd = new StreamReader(exePath + "\\Data\\FlagsBuilder\\" + cb.Text + ".txt");

                String line;
                strrd.ReadLine();
                line = strrd.ReadLine();
                totalBytes = Convert.ToInt32(line);
                if ((totalBytes != 1) && (totalBytes != 2) && (totalBytes != 3) && (totalBytes != 4))
                {
                    strrd.Close();
                    throw new Exception("Settings file error !\n" + exePath + "\\Data\\FlagsBuilder\\" + cb.Text + ".txt\n\nNumber of Bytes must be 1, 2, 3 or 4 !\n");
                }

                while (!strrd.EndOfStream)
                {
                    String desc = strrd.ReadLine();
                    int bits = Convert.ToInt32(strrd.ReadLine().Trim());
                    BitsGroup bgr = new BitsGroup(bits);
                    bgr.description = desc;

                    bitsList.Add(bgr);
                }

                strrd.Close();
                //initializeaza controale
                int lasty = 10;
                for (int kk = 0; kk < bitsList.Count; kk++)
                {
                    BitsGroup bg = bitsList[kk];
                    Label lb = new Label();
                    lb.Location = new Point(10, lasty);
                    lb.Width = 260; lb.Height = 50;
                    lb.Text = bg.GetBitsCnt().ToString() + " BITS : " + bg.description;

                    panelControls.Controls.Add(lb);

                    NumericUpDown numud = new NumericUpDown();
                    numud.Maximum = bg.getMaxValue();
                    numud.Value = 0;
                    numud.Location = new Point(270, lasty);
                    numud.Width = 50;

                    numud.ValueChanged += new EventHandler(numud_ValueChanged);

                    panelControls.Controls.Add(numud);

                    lasty += 50;
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show("Could not initialize configuration !\n\n" + ex.ToString(), "ERROR !!!", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }

        }


        //cand se schimba numericele
        void numud_ValueChanged(object sender, EventArgs e)
        {
            String binaryVal = ".";
            UInt32 lastval = 0;
            for(int kk = bitsList.Count-1; kk>=0; kk--)
            {
                NumericUpDown num = panelControls.Controls[kk * 2 + 1] as NumericUpDown;
                bitsList[kk].SetValue((int)num.Value);
                binaryVal += bitsList[kk].getBitsString()+".";

                lastval = lastval | (UInt32)bitsList[kk].GetValue();
                if(kk-1 >= 0)
                    lastval = lastval << bitsList[kk-1].GetBitsCnt();
            }
            binaryVal += "";
            tbBinaryValue.Text = binaryVal;
            tbFlagValue.Text = lastval.ToString();
        }

        //despacheteaza flagurile
        private void butUnpack_Click(object sender, EventArgs e)
        {
            UInt32 value = 0;
            try
            {
                value = Convert.ToUInt32(tbFlagValue.Text.Trim());
            }
            catch (Exception)
            {
                MessageBox.Show("Invalid value !", "WARNING !");
                return;
            }

            try
            {
                String str = ToBinaryString(value, totalBytes);
                String copy = str;
                //scrie valorile unde trebuie
                for (int kk = bitsList.Count - 1; kk >= 0; kk--)
                {
                    NumericUpDown num = panelControls.Controls[kk * 2 + 1] as NumericUpDown;
                    UInt32 valor = Convert.ToUInt32(str.Substring(0, bitsList[kk].GetBitsCnt()), 2);
                    num.Value = valor;
                    str = str.Substring(bitsList[kk].GetBitsCnt());
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show("UNPACKING FAILED ! Maybe flag was saved with another configuration !\n"+ex.ToString(), "WARNING", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
            }

        }

        private void FlagsBuilderWnd_FormClosing(object sender, FormClosingEventArgs e)
        {
            this.Hide();
        }
    }
}