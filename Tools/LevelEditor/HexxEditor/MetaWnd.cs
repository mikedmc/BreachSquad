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
using System.Xml;

namespace HexxEditor
{
    public partial class MetaWnd : Form
    {
        EditorWnd parentWnd = null;

        public MetaWnd(EditorWnd pParent)
        {
            //save pointer to parent
            parentWnd = pParent;

            InitializeComponent();
        }

        private void BkgSelector_FormClosing(object sender, FormClosingEventArgs e)
        {
            EditorWnd.CAreaMetadata meta = new EditorWnd.CAreaMetadata
            {
                tags = tb_labels.Text
            };
            parentWnd.SetLevelMetadata(meta);
        }

        public void SetMetadata(EditorWnd.CAreaMetadata meta)
        {
            tb_labels.Text = meta.tags;
        }
    }
}
