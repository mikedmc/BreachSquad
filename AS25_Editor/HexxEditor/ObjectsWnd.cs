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
    public partial class ObjectsWnd : Form
    {
        Form1 parentWnd = null;
        Form1.CObject pObject = null;

        public int g_selectedAnim = -1;
        public int g_selectedFrame = -1;

        /// <summary>
        /// General methods
        /// </summary>

        public void SetIsCover(bool bIsCover)
        {
            if (pObject != null)
            {
                chk_useAsCover.Checked = bIsCover;
            }
        }

        public void SetSelectedObject(Form1.CObject pObj)
        {
            pObject = pObj;
            PopulateDataFields();
        }

        void PopulateDataFields()
        {
            if (pObject == null)
            {
                //g_selectedAnim = -1;
                //g_selectedFrame = -1;

                chk_animated.Enabled = false;
                chk_useAsCover.Enabled = false;

                //RemoveSelection();
            }
            else
            {
                chk_useAsCover.Enabled = true;
                chk_animated.Enabled = true;
                chk_animated.Checked = (pObject.flags & Form1.OBJFLAG_ANIMATED) != 0;
                chk_useAsCover.Checked = (pObject.flags & Form1.OBJFLAG_IS_COVER) != 0;

                SetObjectsWndSelection(pObject.animIdx, pObject.frameIdx);
            }
        }

        public ObjectsWnd(Form1 parent)
        {
            InitializeComponent();

            parentWnd = parent;
        }

        public bool LoadBSX(string filename)
        {
            return BSXbrowserCtrl1.LoadBSX(filename);
        }

        private void ObjectsWnd_FormClosing(object sender, FormClosingEventArgs e)
        {
            SetSelectedObject(null);

            this.Hide();
            e.Cancel = true;
        }

        private void openBSXToolStripMenuItem_Click_1(object sender, EventArgs e)
        {
            OpenFileDialog sfd = new OpenFileDialog();
            //fisier binar
            sfd.CheckFileExists = true;
            sfd.Filter = "BSX File (*.bsx)|*.bsx|All Files (*.*)|*.*";
            if (sfd.ShowDialog() == DialogResult.Cancel)
                return;

            BSXbrowserCtrl1.LoadBSX(sfd.FileName);
        }

        public void ResetObjectsWndSelection()
        {
            BSXbrowserCtrl1.RemoveSelection();
        }
        public void SetObjectsWndSelection(int animIdx, int frameIdx)
        {
            BSXbrowserCtrl1.SetSelection(animIdx, frameIdx);
        }

        public BSXAnimBrowser.SpriteLoader GetSpriteLoader()
        {
            return BSXbrowserCtrl1.sprites;
        }

        private void BSXbrowserCtrl1_MyAnimChangedDelegate_1(object sender, EventArgs e)
        {
            BSXAnimBrowser.BSXbrowserCtrl.EventArgsBSX ev = (BSXAnimBrowser.BSXbrowserCtrl.EventArgsBSX)e;

            g_selectedAnim = ev.selectedAnim;
            g_selectedFrame = ev.selectedFrame;
            //daca avem obiect selectat ii schimbam animatia
            if (pObject != null)
            {
                pObject.animIdx = g_selectedAnim;
                pObject.frameIdx = g_selectedFrame;

                parentWnd.PaintMap();
            }
        }

        // Tells us if checkbox is set
        public bool GetFlag_AffectChildren()
        {
            return chk_affectChildren.Checked;
        }

        private void ObjectsWnd_VisibleChanged(object sender, EventArgs e)
        {
            SetSelectedObject(null);
        }

        private void chk_animated_CheckedChanged(object sender, EventArgs e)
        {
            if (pObject == null)
                return;

            if (chk_animated.Checked)
                pObject.flags |= Form1.OBJFLAG_ANIMATED;
            else
                pObject.flags &= ~Form1.OBJFLAG_ANIMATED;
        }

        private void chk_useAsCover_CheckedChanged(object sender, EventArgs e)
        {
            if (pObject == null)
                return;

            if (chk_useAsCover.Checked)
                pObject.flags |= Form1.OBJFLAG_IS_COVER;
            else
                pObject.flags &= ~Form1.OBJFLAG_IS_COVER;

            parentWnd.PaintMap();            
        }

        private void but_toTop_Click(object sender, EventArgs e)
        {
            if (pObject != null)
            {
                //mut obiectul in sus 
                int idxfrom = parentWnd.arrObjects.IndexOf(pObject);

                parentWnd.MoveObjectInList_ToEnd(parentWnd.arrObjects, idxfrom);

                parentWnd.PaintMap();
            }
        }

        private void but_toBack_Click(object sender, EventArgs e)
        {
            if (pObject != null)
            {
                //mut obiectul in sus 
                int idxfrom = parentWnd.arrObjects.IndexOf(pObject);

                parentWnd.MoveObjectInList_ToBeginning(parentWnd.arrObjects, idxfrom);

                parentWnd.PaintMap();
            }
        }

    }
}
