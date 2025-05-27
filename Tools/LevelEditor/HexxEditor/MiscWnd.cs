using System;
using System.Collections;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace HexxEditor
{
    public partial class MiscWnd : Form
    {
        EditorWnd parentWnd = null;
        EditorWnd.CMiscObjectBase pMisc = null;

        bool bIsFillingData = false;

        public MiscWnd(EditorWnd parent)
        {
            InitializeComponent();

            parentWnd = parent;
        }

        public void SetSelectedMisc(EditorWnd.CMiscObjectBase pMiscObject)
        {
            pMisc = pMiscObject;

            PopulateDataFields();
        }


        void PopulateDataFields()
        {
            if (pMisc == null)
            {
                groupBox1.Enabled = false;
                groupBox1.Text = "Misc ID:";
            }
            else
            {
                bIsFillingData = true;

                groupBox1.Enabled = true;
                groupBox1.Text = "Misc ID: " + pMisc.ID;

                text_MiscParams.Text = "";
                for (int kk = 0; kk < pMisc.listParams.Count / 2; kk++)
                {
                    string param = pMisc.listParams[kk * 2] as string;
                    string value = pMisc.listParams[kk * 2 + 1] as string;
                    text_MiscParams.Text += param + " = " + value + ";\r\n";
                }

                bIsFillingData = false;
            }
        }

        private void butTrails_Click(object sender, EventArgs e)
        {
            parentWnd.g_brushMode = EditorWnd.BRUSH_MODE_MISC;
            parentWnd.g_brushValue = EditorWnd.K_MISC_RAILS;
            parentWnd.g_selectedMisc = null;
            SetSelectedMisc(null);
        }

        /// <summary>
        /// Intoarce parametrii de tip nume=valoare dintr-un string cu separatori = si ;
        /// Exemplu input: a=1.0;b=mihai are mere;c=12
        /// </summary>
        /// <param name="inputText">textul cu parametrii</param>
        /// <param name="variables">Array de stringuri de tipul [nume][valoare][nume][val...</param>
        /// <returns>nr de params convertiti sau -1 pt eroare</returns>
        private int GetParamsFromString(string inputText, out ArrayList variables)
        {
            variables = new ArrayList();

            string line = text_MiscParams.Text.Trim();
            if (line.Length == 0)
            {
                variables.Clear();
                return 0;
            }

            string[] tokens = line.Split(';');

            int varcnt = 0;

            foreach (string token in tokens)
            {
                string trimmed = token.Trim();
                if (trimmed.Length == 0)
                    continue;

                string[] vars = trimmed.Split('=');
                if (vars.Count() != 2)
                {
                    variables.Clear();
                    return -1;
                }

                variables.Add(vars[0].Trim());
                variables.Add(vars[1].Trim());

                varcnt++;
            }

            return varcnt;
        }


        private void text_MiscParams_TextChanged(object sender, EventArgs e)
        {
            if ((pMisc == null) || (bIsFillingData))
                return;

            int varcnt = GetParamsFromString(text_MiscParams.Text, out pMisc.listParams);
            if (varcnt == -1)
            {
                text_MiscParams.BackColor = Color.Red;
            }
            else
            {
                text_MiscParams.BackColor = Color.White;
            }

        }

        private void MiscWnd_FormClosing(object sender, FormClosingEventArgs e)
        {
            SetSelectedMisc(null);

            this.Hide();
            e.Cancel = true;
        }

        private void but_doorLocked_Click(object sender, EventArgs e)
        {
            parentWnd.g_brushMode = EditorWnd.BRUSH_MODE_MACRO;
            parentWnd.g_brushValue = EditorWnd.K_MACRO_DOOR_LOCKED;
            parentWnd.g_selectedMisc = null;
            SetSelectedMisc(null);
        }

        private void but_doorUnlocked_Click(object sender, EventArgs e)
        {
            parentWnd.g_brushMode = EditorWnd.BRUSH_MODE_MACRO;
            parentWnd.g_brushValue = EditorWnd.K_MACRO_DOOR_UNLOCKED;
            parentWnd.g_selectedMisc = null;
            SetSelectedMisc(null);
        }

        private void but_doorMetallic_Click(object sender, EventArgs e)
        {
            parentWnd.g_brushMode = EditorWnd.BRUSH_MODE_MACRO;
            parentWnd.g_brushValue = EditorWnd.K_MACRO_DOOR_METALLIC;
            parentWnd.g_selectedMisc = null;
            SetSelectedMisc(null);
        }

        private void but_MetalDoorUnlocker_Click(object sender, EventArgs e)
        {
            parentWnd.g_brushMode = EditorWnd.BRUSH_MODE_MACRO;
            parentWnd.g_brushValue = EditorWnd.K_MACRO_KEYCARD_RED;
            parentWnd.g_selectedMisc = null;
            SetSelectedMisc(null);
        }

        private void but_FrontStairs_Click(object sender, EventArgs e)
        {
            parentWnd.g_brushMode = EditorWnd.BRUSH_MODE_MACRO;
            parentWnd.g_brushValue = EditorWnd.K_MACRO_FRONT_SOLO_STAIRS;
            parentWnd.g_selectedMisc = null;
            SetSelectedMisc(null);
        }

        private void but_FrontDoor_Click(object sender, EventArgs e)
        {
            parentWnd.g_brushMode = EditorWnd.BRUSH_MODE_MACRO;
            parentWnd.g_brushValue = EditorWnd.K_MACRO_FRONT_TEAM_DOOR;
            parentWnd.g_selectedMisc = null;
            SetSelectedMisc(null);
        }

        private void but_Checkpoint_Click(object sender, EventArgs e)
        {
            parentWnd.g_brushMode = EditorWnd.BRUSH_MODE_MISC;
            parentWnd.g_brushValue = EditorWnd.K_MISC_SPAWNPOINT;
            parentWnd.g_selectedMisc = null;
            SetSelectedMisc(null);
        }

        private void but_Script_Click(object sender, EventArgs e)
        {
            parentWnd.g_brushMode = EditorWnd.BRUSH_MODE_MISC;
            parentWnd.g_brushValue = EditorWnd.K_MISC_SCRIPT;
            parentWnd.g_selectedMisc = null;
            SetSelectedMisc(null);
        }

        private void but_windProfile_Click(object sender, EventArgs e)
        {
            parentWnd.g_brushMode = EditorWnd.BRUSH_MODE_MACRO;
            parentWnd.g_brushValue = EditorWnd.K_MACRO_WINDOW_PROFILE;
            parentWnd.g_selectedMisc = null;
            SetSelectedMisc(null);
        }

        private void but_windowHoriz_Click(object sender, EventArgs e)
        {
            parentWnd.g_brushMode = EditorWnd.BRUSH_MODE_MACRO;
            parentWnd.g_brushValue = EditorWnd.K_MACRO_WINDOW_PROFILE_HORIZONTAL;
            parentWnd.g_selectedMisc = null;
            SetSelectedMisc(null);
        }

        private void but_soloDoor_Click(object sender, EventArgs e)
        {
            parentWnd.g_brushMode = EditorWnd.BRUSH_MODE_MACRO;
            parentWnd.g_brushValue = EditorWnd.K_MACRO_FRONT_SOLO_DOOR;
            parentWnd.g_selectedMisc = null;
            SetSelectedMisc(null);
        }

        private void but_keycardGold_Click(object sender, EventArgs e)
        {
            parentWnd.g_brushMode = EditorWnd.BRUSH_MODE_MACRO;
            parentWnd.g_brushValue = EditorWnd.K_MACRO_KEYCARD_GOLD;
            parentWnd.g_selectedMisc = null;
            SetSelectedMisc(null);
        }
    }
}
