using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.IO;
using System.Collections;

namespace HexxEditor
{
    public partial class AIwnd : Form
    {
        Form1 parentWnd = null;
        Form1.CBehaviorContainer pLogic = null;

        bool bIsFillingData = false;

        public int TargetID_property
        {
            get { return (int)num_targetID.Value; }
            set { num_targetID.Value = value; }
        }

        public int InteractTimer_property
        {
            get { return (int)num_interactTimer.Value; }
            set { num_interactTimer.Value = value; }
        }

        //clasa pentru umplerea combo box-ului
        public class ComboNameValue
        {
            private string myDisplayName;
            private string myValue;

            public ComboNameValue(string strDisplayName, string strValue)
            {
                this.myDisplayName = strDisplayName;
                this.myValue = strValue;
            }
            public string DisplayName
            {
                get
                {
                    return myDisplayName;
                }
            }

            public string Value
            {
                get
                {
                    return myValue;
                }
            }

        }

        public AIwnd(Form1 parent)
        {
            InitializeComponent();
            combo_AI.DropDownStyle = ComboBoxStyle.DropDownList;

            parentWnd = parent;
            //fill behavior list
            String exePath = Path.GetDirectoryName(System.Reflection.Assembly.GetExecutingAssembly().GetName().CodeBase);
            exePath = exePath.Substring(6);
            //initializare lista configuratii
            try
            {
                ArrayList MyFields = new ArrayList();
                MyFields.Add(new ComboNameValue("EMPTY", ""));

                StreamReader strrd = new StreamReader(exePath + "\\editorData\\behaviors.txt");

                String line;
                while (!strrd.EndOfStream)
                {
                    line = strrd.ReadLine();
                    String[] tokens = line.Split(';');
                    MyFields.Add(new ComboNameValue(tokens[1], tokens[0]));
                }

                combo_AI.DataSource = MyFields;
                combo_AI.DisplayMember = "DisplayName";
                combo_AI.ValueMember = "Value";

                combo_AI.SelectedValue = "";

                strrd.Close();
            }
            catch (Exception ex)
            {
                MessageBox.Show("Could not read behaviors.txt !\nSolution: PANIC !!!\n\n" + ex.ToString(), "ERROR !!!", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }

        }

        void PopulateDataFields()
        {
            if (pLogic == null)
            {
                groupBehavior.Enabled = false;
                groupScript.Enabled = false;
            }
            else
            {
                bIsFillingData = true;

                groupBehavior.Enabled = true;
                groupScript.Enabled = true;

                combo_AI.SelectedValue = pLogic.strAIname;

                text_AIparams.Text = "";
                for (int kk = 0; kk < pLogic.listAIparams.Count / 2; kk++)
                {
                    string param = pLogic.listAIparams[kk * 2] as string;
                    string value = pLogic.listAIparams[kk * 2 + 1] as string;
                    text_AIparams.Text += param + " = " + value + ";\r\n";
                }

                text_scriptName.Text = pLogic.strScriptName;
                num_targetID.Value = pLogic.targetID;

                chk_canInteract.Checked = pLogic.bCanInteract;
                chk_startHidden.Checked = pLogic.bStartHidden;
                chk_hideInteract.Checked = pLogic.bHideInteractIcon;
                num_interactTimer.Value = pLogic.nInteractTimer;

                bIsFillingData = false;
            }
        }

        public void SetSelectedLogic(Form1.CBehaviorContainer pBehavior)
        {
            pLogic = pBehavior;

            PopulateDataFields();
        }

        private void AIwnd_FormClosing(object sender, FormClosingEventArgs e)
        {
            this.Hide();
            e.Cancel = true;
        }

        //callbacks
        private void num_targetID_ValueChanged(object sender, EventArgs e)
        {
            if (pLogic == null)
                return;
            pLogic.targetID = (int)num_targetID.Value;
        }

        private void text_scriptName_TextChanged(object sender, EventArgs e)
        {
            if (pLogic == null)
                return;
            pLogic.strScriptName = text_scriptName.Text;
        }

        private void combo_AI_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (pLogic == null)
                return;
            if (combo_AI.SelectedValue == null)
                return;
            pLogic.strAIname = combo_AI.SelectedValue.ToString();
        }

        private void chk_canInteract_CheckedChanged(object sender, EventArgs e)
        {
            if (pLogic == null)
                return;
            pLogic.bCanInteract = chk_canInteract.Checked;
        }

        /// <summary>
        /// Intoarce parametrii de tip nume=valoare dintr-un string cu separatori = si ;
        /// Exemplu input: a=1.0;b=mihai are mere;c=12
        /// </summary>
        /// <param name="inputText">textul cu parametrii</param>
        /// <param name="variables">Array de stringuri de tipul [nume][valoare][nume][val...</param>
        /// <returns>nr de params convertiti sau -1 pt eroare</returns>
        public int GetParamsFromString(string inputText, out ArrayList variables)
        {
            variables = new ArrayList();

            string line = inputText;
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

        private void text_AIparams_TextChanged(object sender, EventArgs e)
        {
            if ((pLogic == null) || (bIsFillingData))
                return;

            int varcnt = GetParamsFromString(text_AIparams.Text, out pLogic.listAIparams);
            if (varcnt == -1)
            {
                text_AIparams.BackColor = Color.Red;
            }
            else
            {
                text_AIparams.BackColor = Color.White;
            }
        }

        private void chk_startHidden_CheckedChanged(object sender, EventArgs e)
        {
            if (pLogic == null)
                return;
            pLogic.bStartHidden = chk_startHidden.Checked;
        }

        private void num_interactTimer_ValueChanged(object sender, EventArgs e)
        {
            if (pLogic == null)
                return;
            pLogic.nInteractTimer = (Int32)num_interactTimer.Value;
        }

        private void butHelpTimer_Click(object sender, EventArgs e)
        {
            MessageBox.Show("The interact timer tells how much time the player must spend before interacting. More players means shorter time.\n\rNegative values mean ALL players must activate at the same time.", "Interact timer help");
        }

        private void chk_hideInteract_CheckedChanged(object sender, EventArgs e)
        {
            if (pLogic == null)
                return;
            pLogic.bHideInteractIcon = chk_hideInteract.Checked;
        }
    }
}
