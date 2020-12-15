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
    public partial class CollisionWnd : Form
    {
        Form1 parentWnd = null;
        public Form1.CCollisionElement pElement = null;

        public void SetSelectedCollision(Form1.CCollisionElement pColl)
        {
            pElement = pColl;
            PopulateDataFields();
        }
        //get/set
        public bool ShrinkByAxis
        {
            get { return chk_shrinkByAxis.Checked; }
        }

        public bool HideWater
        {
            get { return chk_hideWater.Checked; }
        }
        public bool HideFOW
        {
            get { return chk_hideFOW.Checked; }
        }
        public bool HideTriggers
        {
            get { return chk_hideTriggers.Checked; }
        }

        public CollisionWnd(Form1 parent)
        {
            InitializeComponent();

            combo_CollType.DropDownStyle = ComboBoxStyle.DropDownList;

            parentWnd = parent;
            PopulateDataFields();
        }

        void PopulateDataFields()
        {
            if (pElement == null)
            {
                groupBox_collisions.Enabled = false;
            }
            else
            {
                groupBox_collisions.Enabled = true;
                chk_castShadows.Checked = pElement.castShadows;
                combo_CollType.SelectedIndex = pElement.type;
            }
        }

        private void CollisionWnd_FormClosing(object sender, FormClosingEventArgs e)
        {
            this.Hide();
            e.Cancel = true;
        }

        private void chk_castShadows_CheckedChanged(object sender, EventArgs e)
        {
            if (pElement == null)
                return;

            pElement.castShadows = chk_castShadows.Checked;
        }


        private void combo_CollType_SelectedIndexChanged(object sender, EventArgs e)
        {
  
            //set type now
            pElement.type = combo_CollType.SelectedIndex;

            parentWnd.PaintMap();

        }

        private void combo_CollType_SelectionChangeCommitted(object sender, EventArgs e)
        {
            if (pElement == null)
                return;

            
            switch (combo_CollType.SelectedIndex)
            {
                case Form1.K_COLL_TYPE_SOLID:
                case Form1.K_COLL_TYPE_BOX:
                case Form1.K_COLL_TYPE_COVER:
                case Form1.K_COLL_TYPE_MOVING_PLATFORM:
                case Form1.K_COLL_TYPE_LEDGE:
                    {
                        chk_castShadows.Checked = true;
                    }
                    break;
                case Form1.K_COLL_TYPE_LADDER:
                case Form1.K_COLL_TYPE_WATER:
                case Form1.K_COLL_TYPE_STAIRS:
                case Form1.K_COLL_TYPE_PARTICLE_SYSTEM:
                case Form1.K_COLL_TYPE_ROOM_OCCLUDER:
                case Form1.K_COLL_TYPE_TRIGGER:
                    {
                        chk_castShadows.Checked = false;
                    }
                    break;
            }
            //set type now
            pElement.type = combo_CollType.SelectedIndex;

            parentWnd.PaintMap();
             
        }
    }
}
