using System;
using System.IO;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace HexxEditor
{
    public partial class PrefabsWnd : Form
    {
        EditorWnd parentWnd = null;
        //keeps paths to all prefab files
        string strFolder;
        string[] arrPaths = null;

        public PrefabsWnd(EditorWnd parent)
        {
            InitializeComponent();

            parentWnd = parent;

            PopulateDataFields();
            //scale list column
            listView_prefabs.Columns[0].Width = listView_prefabs.Width - 5;
        }

        public void SetPrefabsFolder(string strPrefabsFolder)
        {
            strFolder = strPrefabsFolder;
            LoadPrefabsNames(strFolder);
        }

        int LoadPrefabsNames(string strPrefabsFolder)
        {
            try
            {
                arrPaths = System.IO.Directory.GetFiles(strPrefabsFolder, "*.dkas_prefab");
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "Error loading prefabs list!");

                arrPaths = null;
                return 0;
            }

            //populate list too
            listView_prefabs.Items.Clear();
            for (int kk = 0; kk < arrPaths.Count(); kk++)
            {
                ListViewItem lvi = new ListViewItem(Path.GetFileNameWithoutExtension(arrPaths[kk]));
                listView_prefabs.Items.Add(lvi);
            }

            return arrPaths.Count();
        }

        // Returns the current selected prefab short name
        public string GetSelectedPrefabName()
        {
            if (listView_prefabs.SelectedItems.Count == 1)
            {
                return listView_prefabs.SelectedItems[0].Text;
            }
            return "Select Prefab from list!";
        }

        // Returns selected prefab path
        public string GetSelectedPrefabPath()
        {
            if (listView_prefabs.SelectedItems.Count == 1)
            {
                return arrPaths[listView_prefabs.SelectedItems[0].Index];
            }
            return null;
        }

        public string GetPrefabsFolder()
        {
            return strFolder;
        }

        void PopulateDataFields()
        {
        }

        private void CollisionWnd_FormClosing(object sender, FormClosingEventArgs e)
        {
            this.Hide();
            e.Cancel = true;
        }

        private void listView_prefabs_SizeChanged(object sender, EventArgs e)
        {
            listView_prefabs.Columns[0].Width = listView_prefabs.Width - 5;
        }

        private void butRefresh_Click(object sender, EventArgs e)
        {
            int nRetVal = LoadPrefabsNames(strFolder);
            if(nRetVal > 0)
                MessageBox.Show("Prefabs list refreshed!", "Info");
        }

        private void setPrefabsFolderToolStripMenuItem_Click(object sender, EventArgs e)
        {
            FolderBrowserDialog fbd = new FolderBrowserDialog();
            DialogResult result = fbd.ShowDialog();

            if (result == DialogResult.OK && !string.IsNullOrWhiteSpace(fbd.SelectedPath))
            {
                SetPrefabsFolder(fbd.SelectedPath);
            }
        }
    }
}
