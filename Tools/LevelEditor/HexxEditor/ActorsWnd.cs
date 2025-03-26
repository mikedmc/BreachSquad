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
    public partial class ActorsWnd : Form
    {
        EditorWnd parentWnd = null;
        public EditorWnd.CActor pActor = null;

        class TemplateAnimCombo
        {
            public string templateName;
            public string[] strBehaviorNames;
            public int animIdx;

            public TemplateAnimCombo()
            {
                templateName = "";
                animIdx = -1;
                strBehaviorNames = null;
            }
        }

        public ArrayList templateList = new ArrayList();
        // Gets the index of a specific template in the templates list
        int GetTemplateIndexInList(string strTemplateName)
        {
            for (int kk = 0; kk < templateList.Count; kk++)
            {
                TemplateAnimCombo cTemplate = templateList[kk] as TemplateAnimCombo;
                if (cTemplate.templateName == strTemplateName)
                    return kk;
            }
            return -1;
        }

        public bool g_hideActors
        {
            get { return chk_hideActors.Checked; }
        }

        public void SetActorTemplate(EditorWnd.CActor act)
        {
            act.animIdx = -1;
            if (lvActors.SelectedIndices.Count == 0)
                return;
            int nIdx = lvActors.SelectedIndices[0];
            TemplateAnimCombo selTemplate = templateList[nIdx] as TemplateAnimCombo;
            act.templateName = selTemplate.templateName;
            act.animIdx = selTemplate.animIdx;

            //if (combo_templates.SelectedIndex < 0)
            //    return;
            //act.templateName = combo_templates.SelectedItem as String;
            //act.animIdx = (templateList[combo_templates.SelectedIndex] as TemplateAnimCombo).animIdx;
        }

        public void SetActorAnimByTemplate(EditorWnd.CActor act)
        {
            act.animIdx = -1;
            for (int kk = 0; kk < templateList.Count; kk++)
            {
                TemplateAnimCombo cmb = templateList[kk] as TemplateAnimCombo;
                if (cmb.templateName == act.templateName)
                {
                    act.animIdx = cmb.animIdx;
                    return;
                }
            }

            MessageBox.Show("SetActorAnimByTemplate::couldn't find template: " + act.templateName + "\n\rActor ID:" + act.ID);
        }

        public ActorsWnd(EditorWnd parent)
        {
            InitializeComponent();
            comboStates.DropDownStyle = ComboBoxStyle.DropDownList;

            parentWnd = parent;
            SetSelectedActor(null);
        }

        //se cheama dupa ce am incarcat nivelul
        public void PopulateTemplatesList(String path)
        {
            //fill templates list
            try
            {
                Size szIcon = new Size(48, 48);
                //clear everything
                lvActors.Items.Clear();
                templateList.Clear();

                ImageList imgList = new ImageList();
                imgList.ImageSize = szIcon;

                XmlReaderSettings readerSettings = new XmlReaderSettings();
                readerSettings.IgnoreComments = true;
                using (XmlReader reader = XmlReader.Create(path, readerSettings))
                {
                    XmlDocument xdoc = new XmlDocument();
                    xdoc.Load(reader);
                    //now read data without comments

                    XmlNodeList nodes = xdoc.GetElementsByTagName("ActorTemplates");
                    foreach (XmlNode node in nodes[0].ChildNodes)
                    {
                        //see if templates is excluded
                        XmlNode nodeattr = node.Attributes.GetNamedItem("bEditorEnabled");
                        if ((nodeattr != null) && (nodeattr.Value.ToLower() == "false"))
                            continue;

                        XmlNode anims = node["ANIMS"];//.ChildNodes[0];
                        //no ANIMS children so skip template
                        if (anims == null)
                            continue;

                        string animName = "";
                        foreach (XmlNode anmnode in anims.ChildNodes)
                        {
                            if (anmnode.Name != "REF_POSE")
                                continue;

                            animName = anmnode.Attributes.GetNamedItem("set0").Value;
                            break;
                        }

                        //not found REF_POSE then skip node entirely
                        if (animName == "")
                            continue;

                        string templateName = node.Name;

                        int animIdx = -1;
                        for (int kk = 0; kk < bsXbrowserCtrl1.sprites.anims.Count; kk++)
                        {
                            BSXAnimBrowser.Animation anm = bsXbrowserCtrl1.sprites.anims[kk];
                            if (anm.name == animName)
                            {
                                animIdx = kk;
                                break;
                            }
                        }

                        if (animIdx == -1)
                        {
                            MessageBox.Show("Couldn't find animation by name!", "Error!");
                        }

                        TemplateAnimCombo templ = new TemplateAnimCombo();
                        templ.templateName = templateName;
                        templ.animIdx = animIdx;

                        //create character image
                        Bitmap pImage = new Bitmap(szIcon.Width, szIcon.Height);
                        Graphics pGr = Graphics.FromImage(pImage);
                        pGr.Clear(Color.FromArgb(128, 128, 128));
                        pGr.ScaleTransform(1.5f, 1.5f);
                        BSXAnimBrowser.Frame fr = bsXbrowserCtrl1.sprites.anims[animIdx].aframes[0].frame;
                        fr.Paint(pGr, (szIcon.Width / 2.0f) / 1.5f, szIcon.Height / 1.5f);
                        imgList.Images.Add(pImage);

                        //find behaviors
                        XmlNodeList xmlStatesList = node.SelectNodes("AI/STATE");
                        if (xmlStatesList.Count > 0)
                        {
                            templ.strBehaviorNames = new string[xmlStatesList.Count + 1];
                            templ.strBehaviorNames[0] = "";
                            for (int kk = 0; kk < xmlStatesList.Count; kk++)
                            {
                                XmlNode xmlstate = xmlStatesList[kk];
                                templ.strBehaviorNames[kk + 1] = xmlstate.Attributes.GetNamedItem("name").Value;
                            }
                        }
                        //add template
                        templateList.Add(templ);
                    }
                    //link image list to control
                    lvActors.SmallImageList = imgList;
                    lvActors.View = View.SmallIcon;
                    lvActors.Alignment = ListViewAlignment.SnapToGrid;
                    lvActors.Items.Clear();
                    for (int kk = 0; kk < templateList.Count; kk++)
                    {
                        TemplateAnimCombo templ = templateList[kk] as TemplateAnimCombo;
                        ListViewItem lvi = new ListViewItem();
                        string strActorName = templ.templateName;
                        strActorName = strActorName.Replace("ACTOR_", "").Replace("_", " ");
                        lvi.Text = strActorName;
                        lvi.ImageIndex = kk;
                        lvActors.Items.Add(lvi);
                    }
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show("Could not read actors_data.xml !\nSolution: PANIC !!!\n\n" + ex.ToString(), "ERROR !!!", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }

            //select first item
            if (lvActors.Items.Count > 0)
                lvActors.Items[0].Selected = true;
            SetSelectedActor(null);
        }

        void PopulateDataFields(bool bDontSelectInList = false)
        {
            if (pActor == null)
            {
                groupProperties.Enabled = false;
            }
            else
            {
                groupProperties.Enabled = true;

                int nIdx = GetTemplateIndexInList(pActor.templateName);
                if ((nIdx >= 0) && (nIdx < lvActors.Items.Count) && (bDontSelectInList == false))
                {
                    lvActors.Items[nIdx].Selected = true;
                    lvActors.EnsureVisible(nIdx);
                }

                chk_bCollisions.Checked = pActor.bHasCollision;
                chk_Gravity.Checked = pActor.bHasGravity;
                chk_lookLeft.Checked = pActor.bLookLeft;

                chk_setAngle.Checked = pActor.bSetAngle;
                num_Angle.Value = (decimal)pActor.fAngle;
                //states combo
                comboStates.Items.Clear();
                if ((nIdx >= 0) && (nIdx < templateList.Count))
                {
                    TemplateAnimCombo anmcmb = templateList[nIdx] as TemplateAnimCombo;
                    foreach (string str in anmcmb.strBehaviorNames)
                    {
                        comboStates.Items.Add(str);
                    }
                    comboStates.SelectedItem = pActor.strSelectedAIState;
                }
            }
        }

        public void SetSelectedActor(EditorWnd.CActor selActor)
        {
            pActor = selActor;
            PopulateDataFields();
        }

        private void Actors_FormClosing(object sender, FormClosingEventArgs e)
        {
            SetSelectedActor(null);

            this.Hide();
            e.Cancel = true;
        }

        private void chk_lookLeft_CheckedChanged(object sender, EventArgs e)
        {
            if (pActor == null)
                return;

            pActor.bLookLeft = chk_lookLeft.Checked;
            parentWnd.PaintMap();
        }

        private void openActorsTemplateToolStripMenuItem_Click(object sender, EventArgs e)
        {
            OpenFileDialog sfd = new OpenFileDialog();
            //fisier binar
            sfd.Filter = "XML files (*.xml)|*.xml|All Files (*.*)|*.*";
            if (sfd.ShowDialog() == DialogResult.Cancel)
                return;

            PopulateTemplatesList(sfd.FileName);
        }

        private void chk_bCollisions_CheckedChanged(object sender, EventArgs e)
        {
            if (pActor == null)
                return;

            pActor.bHasCollision = chk_bCollisions.Checked;
            parentWnd.PaintMap();
        }

        private void chk_Gravity_CheckedChanged(object sender, EventArgs e)
        {
            if (pActor == null)
                return;

            pActor.bHasGravity = chk_Gravity.Checked;
            parentWnd.PaintMap();
        }

        private void chk_setAngle_CheckedChanged(object sender, EventArgs e)
        {
            if (chk_setAngle.Checked)
                num_Angle.Enabled = true;
            else
                num_Angle.Enabled = false;

            if (pActor == null)
                return;

            pActor.bSetAngle = chk_setAngle.Checked;
            num_Angle.Value = (decimal)pActor.fAngle;
            parentWnd.PaintMap();
        }

        private void num_Angle_ValueChanged(object sender, EventArgs e)
        {
            if (pActor == null)
                return;

            pActor.fAngle = (float)num_Angle.Value;
            parentWnd.PaintMap();
        }

        private void comboStates_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (pActor == null)
                return;

            pActor.strSelectedAIState = comboStates.SelectedItem as string;
        }

        private void chk_hideActors_CheckedChanged(object sender, EventArgs e)
        {
            parentWnd.PaintMap();
        }

        private void openActorsBSXToolStripMenuItem_Click(object sender, EventArgs e)
        {
            OpenFileDialog sfd = new OpenFileDialog();
            //fisier binar
            sfd.CheckFileExists = true;
            sfd.Filter = "BSX File (*.bsx)|*.bsx|All Files (*.*)|*.*";
            if (sfd.ShowDialog() == DialogResult.Cancel)
                return;

            bsXbrowserCtrl1.LoadBSX(sfd.FileName);
        }

        public bool LoadBSX(string filename)
        {
            return bsXbrowserCtrl1.LoadBSX(filename);
        }

        public BSXAnimBrowser.SpriteLoader GetSpriteLoader()
        {
            return bsXbrowserCtrl1.sprites;
        }

        private void lvActors_SelectedIndexChanged(object sender, EventArgs e)
        {
            if ((pActor == null) || (lvActors.SelectedIndices.Count == 0))
                return;

            SetActorTemplate(pActor);
            PopulateDataFields(true);
            parentWnd.PaintMap();
        }
    }
}
