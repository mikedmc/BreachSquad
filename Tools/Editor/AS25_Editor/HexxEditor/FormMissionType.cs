using System;
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
    public partial class FormMissionType : Form
    {
        Form1 parentWnd = null;

        public FormMissionType(Form1 pParentWnd)
        {
            parentWnd = pParentWnd;

            InitializeComponent();

            if (pParentWnd.g_missionType == Form1.K_MISSION_TYPE_SAVE_HOSTAGES)
                radio_hostages.Checked = true;
            else if (pParentWnd.g_missionType == Form1.K_MISSION_TYPE_DEFUSE_BOMB)
                radio_bomb.Checked = true;
            else if (pParentWnd.g_missionType == Form1.K_MISSION_TYPE_ARREST_WARRANT)
                radio_arrest.Checked = true;
            else
                radio_eliminate.Checked = true;
        }

        private void FormMissionType_FormClosing(object sender, FormClosingEventArgs e)
        {
            if (radio_hostages.Checked)
                parentWnd.g_missionType = Form1.K_MISSION_TYPE_SAVE_HOSTAGES;
            else if (radio_bomb.Checked)
                parentWnd.g_missionType = Form1.K_MISSION_TYPE_DEFUSE_BOMB;
            else if (radio_arrest.Checked)
                parentWnd.g_missionType = Form1.K_MISSION_TYPE_ARREST_WARRANT;
            else
                parentWnd.g_missionType = Form1.K_MISSION_TYPE_ELIMINATE_ALL;
        }
    }
}
