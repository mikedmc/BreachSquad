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
    public partial class ExportWnd : Form
    {
        public ExportWnd()
        {
            InitializeComponent();
            h_a = 0xffff;
            h_b1 = 0;
            h_b2 = 0;
            label12.Text = "HFlag: " + ToBinaryString(Sprite.VERSION, 1) + "|" + ToBinaryString(h_a, 2) + "|" + ToBinaryString(h_b1, 2) + " " +ToBinaryString(h_b2, 2);
            radioButton1.Checked = true;

            populateConfigsList();
        }

        private string ToBinaryString(int v, int no_octets)
        {
            string txt = "";
            for (int i = 0; i < no_octets * 8; i++)
                txt += ((v >> (no_octets * 8 - 1 - i)) & 1).ToString();
            return txt;
        }

        //private byte h_ver;
        public UInt16 h_a;
        public UInt16 h_b1, h_b2;
        public Sprite sprite = null;

        private void radioButton1_CheckedChanged(object sender, EventArgs e)
        {
            radioButton6.Checked = true;
            radioButton15.Checked = true;
            radioButton9.Checked = true;
            radioButton12.Checked = true;
            radioButton18.Checked = true;
            radioButton27.Checked = true;
            radioButton24.Checked = true;
            radioButton21.Checked = true;
            radioButton33.Checked = true;
            radioButton30.Checked = true;
            radioButton36.Checked = true;
            radioButton39.Checked = true;
            radioButton45.Checked = true;
            radioButton48.Checked = true;
        }

        private void radioButton2_CheckedChanged(object sender, EventArgs e)
        {
            radioButton5.Checked = true;
            radioButton14.Checked = true;
            radioButton8.Checked = true;
            radioButton11.Checked = true;
            radioButton17.Checked = true;
            radioButton26.Checked = true;
            radioButton23.Checked = true;
            radioButton20.Checked = true;
            radioButton32.Checked = true;
            radioButton29.Checked = true;
            radioButton35.Checked = true;
            radioButton38.Checked = true;
            radioButton44.Checked = true;
            radioButton47.Checked = true;
        }

        private void radioButton3_CheckedChanged(object sender, EventArgs e)
        {
            radioButton4.Checked = true;
            radioButton13.Checked = true;
            radioButton7.Checked = true;
            radioButton10.Checked = true;
            radioButton16.Checked = true;
            radioButton25.Checked = true;
            radioButton23.Checked = true;
            radioButton19.Checked = true;
            radioButton31.Checked = true;
            radioButton28.Checked = true;
            radioButton34.Checked = true;
            radioButton37.Checked = true;
            radioButton43.Checked = true;
            radioButton46.Checked = true;

        }

        //val poate fi doar 0, 1, 2 sau 3 (adica cei mai nesemnificativi 2 biti)
        void SetBits2masks(ref UInt16 target1, ref UInt16 target2, UInt16 mask, bool check1, bool check2, bool check3)
        {
            if (check1)
            {
                SetBit(ref h_b1, mask, false);
                SetBit(ref h_b2, mask, true);
            }
            else if (check2)
            {
                SetBit(ref h_b1, mask, true);
                SetBit(ref h_b2, mask, false);
            }
            else if (check3)
            {
                SetBit(ref h_b1, mask, true);
                SetBit(ref h_b2, mask, true);
            }
        }
        
        void SetBit(ref UInt16 target, UInt16 mask, bool val)
        {
            if (val)
                target |= mask;
            else
                target &= (ushort)~mask;
            label12.Text = "HFlag: " + ToBinaryString(Sprite.VERSION, 1) + "|" + ToBinaryString(h_a, 2) + "|" + ToBinaryString(h_b1, 2) + " " + ToBinaryString(h_b2, 2);
        }


        private void button4_Click(object sender, EventArgs e)
        {
            if (sprite == null) 
                return;
            SaveFileDialog sfd = new SaveFileDialog();
            sfd.Filter = "Borealis2 Sprite Files (*.spr)|*.spr|All Files (*.*)|*.*";
            if (sfd.ShowDialog() == DialogResult.Cancel)
                return;
            string path = sfd.FileName;
            if (File.Exists(path))
                File.Delete(path);
            //vede ce fel de fisier de definitii se doreste
            if(radioButton50.Checked)
                sprite.Export(path, true, Sprite.VERSION, h_a, h_b1, h_b2);
            else
                sprite.Export(path, false, Sprite.VERSION, h_a, h_b1, h_b2);
        }


        #region EXPORT_CHECKBOXES
        private void checkBox12_CheckedChanged(object sender, EventArgs e)
        {
            SetBit(ref h_a, Sprite.MASK_EXPORT_IMAGENAMES, checkBox12.Checked);
        }

        private void checkBox1_CheckedChanged(object sender, EventArgs e)
        {
            SetBit(ref h_a, Sprite.MASK_EXPORT_MODULES, checkBox1.Checked);
        }

        private void checkBox2_CheckedChanged(object sender, EventArgs e)
        {
            SetBit(ref h_a, Sprite.MASK_EXPORT_MODULE_IMAGEIDX, checkBox2.Checked);
        }

        private void checkBox4_CheckedChanged(object sender, EventArgs e)
        {
            SetBit(ref h_a, Sprite.MASK_EXPORT_FRAMEMODULES, checkBox4.Checked);
        }

        private void checkBox3_CheckedChanged(object sender, EventArgs e)
        {
            SetBit(ref h_a, Sprite.MASK_EXPORT_FRAMEMODULES_FLAGS, checkBox3.Checked);
        }

        private void checkBox6_CheckedChanged(object sender, EventArgs e)
        {
            SetBit(ref h_a, Sprite.MASK_EXPORT_FRAMES, checkBox6.Checked);
        }

        private void checkBox5_CheckedChanged(object sender, EventArgs e)
        {
            SetBit(ref h_a, Sprite.MASK_EXPORT_FRAMEBBOX, checkBox5.Checked);
        }

        private void checkBox7_CheckedChanged(object sender, EventArgs e)
        {
            SetBit(ref h_a, Sprite.MASK_EXPORT_FRAMEHITPOINTS, checkBox7.Checked);
        }

        private void checkBox10_CheckedChanged(object sender, EventArgs e)
        {
            SetBit(ref h_a, Sprite.MASK_EXPORT_AFRAMES, checkBox10.Checked);
        }

        private void checkBox9_CheckedChanged(object sender, EventArgs e)
        {
            SetBit(ref h_a, Sprite.MASK_EXPORT_AFRAMES_OFFSETS, checkBox9.Checked);
        }

        private void checkBox11_CheckedChanged(object sender, EventArgs e)
        {
            SetBit(ref h_a, Sprite.MASK_EXPORT_AFRAMES_DURATION, checkBox11.Checked);
        }

        private void checkBox8_CheckedChanged(object sender, EventArgs e)
        {
            SetBit(ref h_a, Sprite.MASK_EXPORT_AFRAMES_FLAGS, checkBox8.Checked);
        }

        private void checkBox13_CheckedChanged(object sender, EventArgs e)
        {
            SetBit(ref h_a, Sprite.MASK_EXPORT_FRAMEHITPOINT_FLAGS, checkBox13.Checked);
        }

        private void checkBox15_CheckedChanged(object sender, EventArgs e)
        {
            SetBit(ref h_a, Sprite.MASK_EXPORT_ANIMATIONS, checkBox15.Checked);
        }
        #endregion


        #region EXPORT_RADIO

        private void radio_modxywh_changed(object sender, EventArgs e)
        {
            SetBits2masks(ref h_b1, ref h_b2, Sprite.MASK_EXPORTSZ_MODULEXYWH, radioButton6.Checked, radioButton5.Checked, radioButton4.Checked);
        }

        private void radio_moduleindex_changed(object sender, EventArgs e)
        {
            SetBits2masks(ref h_b1, ref h_b2, Sprite.MASK_EXPORTSZ_FMODULE_MODULEINDEX, radioButton15.Checked, radioButton14.Checked, radioButton13.Checked);
        }

        private void radio_fmod_oxoy(object sender, EventArgs e)
        {
            SetBits2masks(ref h_b1, ref h_b2, Sprite.MASK_EXPORTSZ_FMODULE_OFFSETS, radioButton9.Checked, radioButton8.Checked, radioButton7.Checked);
        }

        private void radio_fmod_flags(object sender, EventArgs e)
        {
            SetBits2masks(ref h_b1, ref h_b2, Sprite.MASK_EXPORTSZ_FMODULE_FLAGS, radioButton12.Checked, radioButton11.Checked, radioButton10.Checked);
        }

        private void radio_frm_fmodidx(object sender, EventArgs e)
        {
            SetBits2masks(ref h_b1, ref h_b2, Sprite.MASK_EXPORTSZ_FRAME_FMODULEINDEX, radioButton18.Checked, radioButton17.Checked, radioButton16.Checked);
        }

        private void radio_frm_bbox(object sender, EventArgs e)
        {
            SetBits2masks(ref h_b1, ref h_b2, Sprite.MASK_EXPORTSZ_FRAME_BBOX, radioButton27.Checked, radioButton26.Checked, radioButton25.Checked);
        }

        private void radio_frm_hitptxy(object sender, EventArgs e)
        {
            SetBits2masks(ref h_b1, ref h_b2, Sprite.MASK_EXPORTSZ_FRAME_HITPOINTS_XY, radioButton24.Checked, radioButton23.Checked, radioButton22.Checked);
        }

        private void radio_frm_hitptflags(object sender, EventArgs e)
        {
            SetBits2masks(ref h_b1, ref h_b2, Sprite.MASK_EXPORTSZ_FRAME_HITPOINTS_FLAGS, radioButton21.Checked, radioButton20.Checked, radioButton19.Checked);
        }

        private void radio_afrm_frmidx(object sender, EventArgs e)
        {
            SetBits2masks(ref h_b1, ref h_b2, Sprite.MASK_EXPORTSZ_AFRAME_FRAMEINDEX, radioButton33.Checked, radioButton32.Checked, radioButton31.Checked);
        }

        private void radio_afrm_duration(object sender, EventArgs e)
        {
            SetBits2masks(ref h_b1, ref h_b2, Sprite.MASK_EXPORTSZ_AFRAME_FRAMEDURATION, radioButton30.Checked, radioButton29.Checked, radioButton28.Checked);
        }

        private void radio_afrm_move(object sender, EventArgs e)
        {
            SetBits2masks(ref h_b1, ref h_b2, Sprite.MASK_EXPORTSZ_AFRAME_OFFSETS, radioButton36.Checked, radioButton35.Checked, radioButton34.Checked);
        }

        private void radio_afrm_flags(object sender, EventArgs e)
        {
            SetBits2masks(ref h_b1, ref h_b2, Sprite.MASK_EXPORTSZ_AFRAME_FLAGS, radioButton48.Checked, radioButton47.Checked, radioButton46.Checked);
        }

        private void radio_anim_flags(object sender, EventArgs e)
        {
            SetBits2masks(ref h_b1, ref h_b2, Sprite.MASK_EXPORTSZ_ANIMATION_FLAGS, radioButton39.Checked, radioButton38.Checked, radioButton37.Checked);
        }

        private void radio_anim_afindex(object sender, EventArgs e)
        {
            SetBits2masks(ref h_b1, ref h_b2, Sprite.MASK_EXPORTSZ_ANIMATION_AFRAMEINDEX, radioButton45.Checked, radioButton44.Checked, radioButton43.Checked);
        }

        #endregion

        void WriteTripleRadio(StreamWriter swr, String radioComboName, bool v1, bool v2, bool v3)
        {
            int value = 0;
            if (v1)
                value = 1;
            else if(v2)
                value = 2;
            else if(v3)
                value = 3;
            swr.WriteLine(radioComboName + " " + value);
        }
        void WriteCheckbox(StreamWriter swr, String CheckboxName, bool v1)
        {
            int value = 0;
            if (v1) value = 1;
            else value = 0;
            swr.WriteLine(CheckboxName + " " + value);
        }
        void ReadTripleRadio(StreamReader srd, RadioButton rb1, RadioButton rb2, RadioButton rb3)
        {
            String line = srd.ReadLine();
            line = line.Trim();
            Int32 val = Convert.ToInt32(line[line.Length - 1].ToString());
            rb1.Checked = false;
            rb2.Checked = false;
            rb3.Checked = false;
            if (val == 3)
                rb3.Checked = true;
            else if (val == 2)
                rb2.Checked = true;
            else
                rb1.Checked = true;
        }
        void ReadCheckbox(StreamReader srd, CheckBox cb)
        {
            String line = srd.ReadLine();
            line = line.Trim();
            Int32 val = Convert.ToInt32(line[line.Length - 1].ToString());
            if (val == 1)
                cb.Checked = true;
            else
                cb.Checked = false;
        }
        //save config
        void SaveExportCfg(String filename)
        {
            String exePath = Path.GetDirectoryName(System.Reflection.Assembly.GetExecutingAssembly().GetName().CodeBase);
            exePath = exePath.Substring(6);
            //initializare lista configuratii
            try
            {
                StreamWriter strwr = new StreamWriter(exePath + "\\Data\\ExportCfg\\" + filename.Replace(' ', '_') + ".txt");
                //scrie radios
                WriteTripleRadio(strwr, "MASK_EXPORTSZ_MODULEXYWH", radioButton6.Checked, radioButton5.Checked, radioButton4.Checked);
                WriteTripleRadio(strwr, "MASK_EXPORTSZ_FMODULE_MODULEINDEX", radioButton15.Checked, radioButton14.Checked, radioButton13.Checked);
                WriteTripleRadio(strwr, "MASK_EXPORTSZ_FMODULE_OFFSETS", radioButton9.Checked, radioButton8.Checked, radioButton7.Checked);
                WriteTripleRadio(strwr, "MASK_EXPORTSZ_FMODULE_FLAGS", radioButton12.Checked, radioButton11.Checked, radioButton10.Checked);
                WriteTripleRadio(strwr, "MASK_EXPORTSZ_FRAME_FMODULEINDEX", radioButton18.Checked, radioButton17.Checked, radioButton16.Checked);
                WriteTripleRadio(strwr, "MASK_EXPORTSZ_FRAME_BBOX", radioButton27.Checked, radioButton26.Checked, radioButton25.Checked);
                WriteTripleRadio(strwr, "MASK_EXPORTSZ_FRAME_HITPOINTS_XY", radioButton24.Checked, radioButton23.Checked, radioButton22.Checked);
                WriteTripleRadio(strwr, "MASK_EXPORTSZ_FRAME_HITPOINTS_FLAGS", radioButton21.Checked, radioButton20.Checked, radioButton19.Checked);
                WriteTripleRadio(strwr, "MASK_EXPORTSZ_AFRAME_FRAMEINDEX", radioButton33.Checked, radioButton32.Checked, radioButton31.Checked);
                WriteTripleRadio(strwr, "MASK_EXPORTSZ_AFRAME_FRAMEDURATION", radioButton30.Checked, radioButton29.Checked, radioButton28.Checked);
                WriteTripleRadio(strwr, "MASK_EXPORTSZ_AFRAME_OFFSETS", radioButton36.Checked, radioButton35.Checked, radioButton34.Checked);
                WriteTripleRadio(strwr, "MASK_EXPORTSZ_AFRAME_FLAGS", radioButton48.Checked, radioButton47.Checked, radioButton46.Checked);
                WriteTripleRadio(strwr, "MASK_EXPORTSZ_ANIMATION_FLAGS", radioButton39.Checked, radioButton38.Checked, radioButton37.Checked);
                WriteTripleRadio(strwr, "MASK_EXPORTSZ_ANIMATION_AFRAMEINDEX", radioButton45.Checked, radioButton44.Checked, radioButton43.Checked);

                //scrie checkboxes
                WriteCheckbox(strwr, "MASK_EXPORT_IMAGENAMES", checkBox12.Checked);
                WriteCheckbox(strwr, "MASK_EXPORT_MODULES",  checkBox1.Checked);
                WriteCheckbox(strwr, "MASK_EXPORT_MODULE_IMAGEIDX",  checkBox2.Checked);
                WriteCheckbox(strwr, "MASK_EXPORT_FRAMEMODULES",  checkBox4.Checked);
                WriteCheckbox(strwr, "MASK_EXPORT_FRAMEMODULES_FLAGS", checkBox3.Checked);
                WriteCheckbox(strwr, "MASK_EXPORT_FRAMES", checkBox6.Checked);
                WriteCheckbox(strwr, "MASK_EXPORT_FRAMEBBOX", checkBox5.Checked);
                WriteCheckbox(strwr, "MASK_EXPORT_FRAMEHITPOINTS", checkBox7.Checked);
                WriteCheckbox(strwr, "MASK_EXPORT_FRAMEHITPOINT_FLAGS", checkBox13.Checked);
                WriteCheckbox(strwr, "MASK_EXPORT_AFRAMES", checkBox10.Checked);
                WriteCheckbox(strwr, "MASK_EXPORT_AFRAMES_DURATION", checkBox11.Checked);
                WriteCheckbox(strwr, "MASK_EXPORT_AFRAMES_OFFSETS", checkBox9.Checked);
                WriteCheckbox(strwr, "MASK_EXPORT_AFRAMES_FLAGS", checkBox8.Checked);
                WriteCheckbox(strwr, "MASK_EXPORT_ANIMATIONS", checkBox15.Checked);

                strwr.Flush();
                strwr.Close();
                //reincarca drop-ul de configs si seteaza selectia pe ultimul salvat
                populateConfigsList();
                //mesaj de ok
                MessageBox.Show("Configuration saved !", "INFO", MessageBoxButtons.OK, MessageBoxIcon.Information);
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.ToString(), "Config saving error !", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }
        //load config
        void LoadExportCfg(String filename)
        {
            String exePath = Path.GetDirectoryName(System.Reflection.Assembly.GetExecutingAssembly().GetName().CodeBase);
            exePath = exePath.Substring(6);
            try
            {
                StreamReader strrd = new StreamReader(exePath + "\\Data\\ExportCfg\\" + filename.Replace(' ', '_') + ".txt");
                //citeste radios
                ReadTripleRadio(strrd, radioButton6, radioButton5, radioButton4);
                ReadTripleRadio(strrd, radioButton15, radioButton14, radioButton13);
                ReadTripleRadio(strrd, radioButton9, radioButton8, radioButton7);
                ReadTripleRadio(strrd, radioButton12, radioButton11, radioButton10);
                ReadTripleRadio(strrd, radioButton18, radioButton17, radioButton16);
                ReadTripleRadio(strrd, radioButton27, radioButton26, radioButton25);
                ReadTripleRadio(strrd, radioButton24, radioButton23, radioButton22);
                ReadTripleRadio(strrd, radioButton21, radioButton20, radioButton19);
                ReadTripleRadio(strrd, radioButton33, radioButton32, radioButton31);
                ReadTripleRadio(strrd, radioButton30, radioButton29, radioButton28);
                ReadTripleRadio(strrd, radioButton36, radioButton35, radioButton34);
                ReadTripleRadio(strrd, radioButton48, radioButton47, radioButton46);
                ReadTripleRadio(strrd, radioButton39, radioButton38, radioButton37);
                ReadTripleRadio(strrd, radioButton45, radioButton44, radioButton43);

                //read checkboxes
                ReadCheckbox(strrd, checkBox12);
                ReadCheckbox(strrd, checkBox1);
                ReadCheckbox(strrd, checkBox2);
                ReadCheckbox(strrd, checkBox4);
                ReadCheckbox(strrd, checkBox3);
                ReadCheckbox(strrd, checkBox6);
                ReadCheckbox(strrd, checkBox5);
                ReadCheckbox(strrd, checkBox7);
                ReadCheckbox(strrd, checkBox13);
                ReadCheckbox(strrd, checkBox10);
                ReadCheckbox(strrd, checkBox11);
                ReadCheckbox(strrd, checkBox9);
                ReadCheckbox(strrd, checkBox8);
                ReadCheckbox(strrd, checkBox15);

                strrd.Close();

                cbPreset.Text = filename;
                //mesaj de ok
                //MessageBox.Show("Configuration loaded !", "INFO", MessageBoxButtons.OK, MessageBoxIcon.Information);
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.ToString(), "Config loading error !", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        void populateConfigsList()
        {
            try
            {

                cbPreset.Items.Clear();

                String exePath = Path.GetDirectoryName(System.Reflection.Assembly.GetExecutingAssembly().GetName().CodeBase);
                exePath = exePath.Substring(6);
                exePath += "\\Data\\ExportCfg\\";
                DirectoryInfo di = new DirectoryInfo(exePath);
                FileInfo[] rgFiles = di.GetFiles("*.txt");
                foreach (FileInfo fi in rgFiles)
                {
                    String nme = fi.Name.ToString();
                    cbPreset.Items.Add(nme.Substring(0, nme.Length-4));
                }

            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.ToString(), "Config loading error !", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        private void button2_Click(object sender, EventArgs e)
        {
            if (cbPreset.Text.Length > 2)
            {
                SaveExportCfg(cbPreset.Text);
            }
            else
            {
                MessageBox.Show("Introduceti numele preset-ului in dropdown-ul de preseturi.\n Numele trebuie sa aiba cel putin 3 caractere !", "Filename missing !", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        private void button1_Click(object sender, EventArgs e)
        {
            LoadExportCfg(cbPreset.Text);
        }
   }
}