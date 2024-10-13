using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace InkEd3
{
    public partial class HitPointWnd : Form
    {
        public delegate void ActionCallback(CallbackCommands command, Object parms);
        ActionCallback callbackFunc = null;

        public int valX, valY, valFlags;

        public HitPointWnd()
        {
            InitializeComponent();
        }

        public HitPointWnd(ActionCallback ncallBackFunc, int nX, int nY, int nFlags)
        {
            InitializeComponent();

            valX = nX;
            valY = nY;
            valFlags = nFlags;

            callbackFunc = ncallBackFunc;

            tbHitptX.Text = valX.ToString();
            tbHitptY.Text = valY.ToString();
            tbHitptFlags.Text = valFlags.ToString();
        }

        public void RefreshValues(int nX, int nY, int nFlags)
        {
            valX = nX;
            valY = nY;
            valFlags = nFlags;

            tbHitptX.Text = valX.ToString();
            tbHitptY.Text = valY.ToString();
            tbHitptFlags.Text = valFlags.ToString();
        }

        private void butOK_Click(object sender, EventArgs e)
        {
            try {
                valX = Convert.ToInt32(tbHitptX.Text);
                valY = Convert.ToInt32(tbHitptY.Text);
                valFlags = Convert.ToInt32(tbHitptFlags.Text);
            }
            catch (Exception)
            {
                MessageBox.Show("The fields contain wrong data !\n The values could not be converted into integers.", "WARNING !", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
                return;
            }

            HitPoint hpfl = new HitPoint(valX, valY);
            hpfl.flags = valFlags;
            callbackFunc(CallbackCommands.UpdateHitpoint, hpfl as Object);
        }
    }
}