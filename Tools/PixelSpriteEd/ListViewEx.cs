using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Text;
using System.Windows.Forms;

namespace InkEd3
{
    public partial class ListViewEx : ListView
    {
        public ListViewEx()
        {
            InitializeComponent();
            View = View.Details;
            FullRowSelect = true;
            GridLines = true;
            InitEdit();
        }

        TextBox editBox = new EditBox();
        public int EditColumn;
        public ListViewItem EditItem = null;
        public ListViewItem.ListViewSubItem EditSubItem = null;
        public string EditText = "";

        #region EditBox

        class EditBox : TextBox
        {
            const int WM_KEYDOWN = 0x100;
            const int WM_SYSKEYDOWN = 0x104;

            protected override bool ProcessCmdKey(ref Message msg, Keys keyData)
            {
                if ((msg.Msg == WM_KEYDOWN) || (msg.Msg == WM_SYSKEYDOWN))
                {
                    switch (keyData)
                    {
                        case Keys.Escape:                        
                            Hide();
                            break;
                        
                        case Keys.Tab:
                            break;

                        default:
                            return base.ProcessCmdKey(ref msg, keyData);
                    }
                    return true;
                }
                else
                    return base.ProcessCmdKey(ref msg, keyData);
            }
        }

        void InitEdit()
        {
            Controls.AddRange(new System.Windows.Forms.Control[] { editBox });
            editBox.BorderStyle = BorderStyle.None;
            editBox.KeyUp += new KeyEventHandler(editBox_KeyUp);
            editBox.Leave += new EventHandler(editBox_Leave);
            editBox.Hide();
        }

        void editBox_Leave(object sender, EventArgs e)
        {
            editBox.Hide();
        }

        void editBox_KeyUp(object sender, KeyEventArgs e)
        {
            if (e.KeyCode == Keys.Return && EditSubItem != null)
            {
                EditSubItem.Text = editBox.Text;
                editBox.Hide();
                OnCellEdited(new EventArgs());
            }
            else if (e.KeyCode == Keys.Tab)
            {
                EditSubItem.Text = editBox.Text;
                OnCellEdited(new EventArgs());
                if (EditColumn < EditItem.SubItems.Count - 1)
                {
                    EditColumn++;
                    EditSubItem = EditItem.SubItems[EditColumn];
                    ShowEdit();
                }
                else
                {
                    int row = Items.IndexOf(EditItem);
                    if (row < Items.Count - 1)
                    {
                        EditItem.Selected = false;
                        EditItem = Items[row + 1];
                        EditItem.Selected = true;
                        EditColumn = 1;
                        EditSubItem = EditItem.SubItems[EditColumn];
                        ShowEdit();
                    }
                }
            }
        }

        void ShowEdit()
        {
            //afiseaza edit box
            string text = EditSubItem.Text;
            editBox.Location = new Point(EditSubItem.Bounds.X + 1, EditSubItem.Bounds.Y);
            editBox.Size = new Size(EditSubItem.Bounds.Width - 2, EditSubItem.Bounds.Height);
            editBox.Show();
            editBox.Text = text;
            //editBox.AcceptsReturn = true;
            editBox.Focus();
            editBox.SelectAll();
        }

        #endregion

        protected override void OnDoubleClick(EventArgs e)
        {
            if (EditItem == null || EditSubItem == null || EditColumn == 0)
                return;
            ShowEdit();
            base.OnDoubleClick(e);
        }

        protected override void OnMouseDown(MouseEventArgs e)
        {
            EditItem = GetItemAt(e.X, e.Y);
            if (EditItem != null)
            {
                EditSubItem = EditItem.GetSubItemAt(e.X, e.Y);
                if(EditSubItem != null)
                    EditColumn = EditItem.SubItems.IndexOf(EditSubItem);
            }
            base.OnMouseDown(e);
        }


        [Category("Action")]
        public event EventHandler CellEdited;

        protected virtual void OnCellEdited(EventArgs e)
        {
            EditText = editBox.Text;
            if (CellEdited != null)
                CellEdited(this, e);
        }

    }
}
