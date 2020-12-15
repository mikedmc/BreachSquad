using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace HexxEditor
{
    public partial class LightsWnd : Form
    {
        Form1 parentWnd = null;
        Form1.CLight pLight = null;
        //animatia selectata pentru frame
        public int g_selectedAnim = -1;

        public LightsWnd(Form1 parent)
        {
            InitializeComponent();
            combo_lightType.DropDownStyle = ComboBoxStyle.DropDownList;
            PopulateDataFields();

            parentWnd = parent;
        }

        public bool ShowLightsImage
        {
            get { return chk_showSpotImg.Checked; }
        }

        private static String HexConverter(System.Drawing.Color c)
        {
            return "#" + c.A.ToString("X2") + c.R.ToString("X2") + c.G.ToString("X2") + c.B.ToString("X2");
        } 
               
        /// <summary>
        /// Scrie cifrele din campurile ferestrei in functie de selected light
        /// </summary>
        void PopulateDataFields()
        {
            if(pLight == null)
            {
                groupLightData.Enabled = false;

                g_selectedAnim = -1;
                bsXbrowserCtrl1.SetSelection(g_selectedAnim, -1);
            }
            else
            {
                groupLightData.Enabled = true;

                combo_lightType.SelectedIndex = pLight.type;

                butColor.BackColor = pLight.color;
                butColor.Text = HexConverter(pLight.color);

                chk_castShadows.Checked = pLight.castsShadows;
                num_Angle.Value = (decimal)pLight.angle;
                hScrollBarAlpha.Value = pLight.color.A;
                numZCoord.Value = pLight.posZ;
                num_lightIntensity.Value = (decimal)pLight.fIntensity;
                num_atmoAttenuation.Value = (decimal)pLight.nAtmoAttenuationPerc;

                g_selectedAnim = pLight.animId;
                bsXbrowserCtrl1.SetSelection(g_selectedAnim, -1);
            }
        }

        public void SetSelectedLight(Form1.CLight light)
        {
            pLight = light;
            PopulateDataFields();
        }

        private void LightsWnd_FormClosing(object sender, FormClosingEventArgs e)
        {
            this.Hide();
            e.Cancel = true;
        }

        private void combo_lightType_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (pLight == null)
                return;
            pLight.type = combo_lightType.SelectedIndex;

            parentWnd.PaintMap();
        }

        private void butColor_Click(object sender, EventArgs e)
        {
            if (pLight == null)
                return;

            ColorDialog MyDialog = new ColorDialog();

            MyDialog.AllowFullOpen = true;
            MyDialog.AnyColor = true;
            MyDialog.FullOpen = true;
            MyDialog.ShowHelp = false;
            MyDialog.SolidColorOnly = false;
            MyDialog.Color = pLight.color;

            // Open color selection dialog box
            MyDialog.ShowDialog();

            pLight.color = Color.FromArgb(hScrollBarAlpha.Value, (int)MyDialog.Color.R,
                                                      (int)MyDialog.Color.G,
                                                      (int)MyDialog.Color.B);


            butColor.BackColor = pLight.color;
            butColor.Text = HexConverter(pLight.color);
        }

        private void chk_castShadows_CheckedChanged(object sender, EventArgs e)
        {
            if (pLight == null)
                return;
            pLight.castsShadows = chk_castShadows.Checked;
            parentWnd.PaintMap();
        }

        private void num_angle_ValueChanged(object sender, EventArgs e)
        {
            if (pLight == null)
                return;
            pLight.angle = (float)num_Angle.Value;
            parentWnd.PaintMap();
        }

        private void openLightsBSXToolStripMenuItem_Click(object sender, EventArgs e)
        {
            OpenFileDialog sfd = new OpenFileDialog();
            //fisier binar
            sfd.CheckFileExists = true;
            sfd.Filter = "BSX File (*.bsx)|*.bsx|All Files (*.*)|*.*";
            if (sfd.ShowDialog() == DialogResult.Cancel)
                return;

            bsXbrowserCtrl1.LoadBSX(sfd.FileName);
        }

        public bool LoadBSX(string filename)
        {
            return bsXbrowserCtrl1.LoadBSX(filename);
        }

        public BSXAnimBrowser.SpriteLoader GetSpriteLoader()
        {
            return bsXbrowserCtrl1.sprites;
        }

        private void bsXbrowserCtrl1_MyAnimChangedDelegate(object sender, EventArgs e)
        {
            BSXAnimBrowser.BSXbrowserCtrl.EventArgsBSX ev = (BSXAnimBrowser.BSXbrowserCtrl.EventArgsBSX)e;

            g_selectedAnim = ev.selectedAnim;
            if (pLight == null)
                return;

            int oldAnimId = pLight.animId;
            pLight.animId = g_selectedAnim;
            //ii transmit sa seteze si dreptunghiul
            if(oldAnimId != g_selectedAnim)
                parentWnd.SetLightAreaFromAnim(pLight);

            parentWnd.PaintMap();
        }

        private void chk_showSpotImg_CheckedChanged(object sender, EventArgs e)
        {
            parentWnd.PaintMap();
        }

        private void hScrollBarAlpha_ValueChanged(object sender, EventArgs e)
        {
            if (pLight == null)
                return;

            pLight.color = Color.FromArgb(hScrollBarAlpha.Value, pLight.color);
            butColor.BackColor = pLight.color;
            butColor.Text = HexConverter(pLight.color);
        }

        private void numZCoord_ValueChanged(object sender, EventArgs e)
        {
            if (pLight == null)
                return;

            pLight.posZ = (int)numZCoord.Value;
        }

        private void num_lightIntensity_ValueChanged(object sender, EventArgs e)
        {
            if (pLight == null)
                return;

            pLight.fIntensity = (float)num_lightIntensity.Value;
        }

        private void num_atmoAtten_ValueChanged(object sender, EventArgs e)
        {
            if (pLight == null)
                return;

            pLight.nAtmoAttenuationPerc = (int)num_atmoAttenuation.Value;
        }
    }
}
