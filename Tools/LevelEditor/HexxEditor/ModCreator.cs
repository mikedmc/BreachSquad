using System;
using System.IO;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Xml;
using System.Diagnostics;
using System.Collections;

namespace HexxEditor
{
    public partial class ModCreator : Form
    {
        EditorWnd parentWnd = null;
        eModType currentModType;
        string strImagePath = "";
        string strExePath = "";
        //mod types
        public enum eModType : int { MOD_SINGLE_LEVEL = 0, MOD_GAME_CHANGER = 1, MOD_TYPES_CNT = 2 };
        public static readonly string[] eModType_Names = { "SINGLE_LEVEL", "GAME_CHANGER" };

        ArrayList arrAffectedFiles; //list of affected files

        public ModCreator(EditorWnd pParentWnd)
        {
            parentWnd = pParentWnd;
            strExePath = parentWnd.GetExePath();

            InitializeComponent();
            //set items
            combo_modType.Items.Clear();
            for (int kk = 0; kk < (int)eModType.MOD_TYPES_CNT; kk++)
            {
                combo_modType.Items.Add(eModType_Names[kk]);
            }
            combo_modType.SelectedIndex = 0;
            combo_modType.Enabled = true;
            //set current game version
            text_gameVer.Text = "1.2.2";
            text_author.Text = "J. Doe";
            text_tags.Text = "Levels,Enemies";
            text_modName.Text = "Mod Name";
            text_description.Text = "No description";

            //add current level (at least this one)
            arrAffectedFiles = new ArrayList();
            list_files.Items.Clear();

            string strLevelPath = parentWnd.GetFilePath();
            if (strLevelPath.Length > 0)
                arrAffectedFiles.Add(strLevelPath);

            PopulateAffectedFilesList();
        }

        void PopulateAffectedFilesList()
        {
            string strRootFolder = parentWnd.GetMediaFolderAbsolutePath();
            if (strRootFolder.Length == 0)
            {
                MessageBox.Show("You should load a level first to set the working paths. You can remove the file from the Affected Files list if you don't change it.");
                return;
            }
            //remove another folder level to include "media" in file paths
            strRootFolder = Path.GetDirectoryName(strRootFolder);

            list_files.Items.Clear();
            for (int kk = 0; kk < arrAffectedFiles.Count; kk++)
            {
                string strFilePath = arrAffectedFiles[kk] as string;
                //get file folder relative to media
                string strFileRel = parentWnd.GetRelativePath(strFilePath, strRootFolder);
                //add it
                strFileRel = strFileRel.Replace('\\', '/');
                list_files.Items.Add(strFileRel);
            }
        }

        //sets type of mod to upload
        public void SetModType(eModType modType)
        {
            int nSelection = (int)modType;
            combo_modType.SelectedIndex = nSelection;
            currentModType = modType;
            //set text too
            string strLevelPath = parentWnd.GetFilePath();
            text_modName.Text = Path.GetFileNameWithoutExtension(strLevelPath);
        }

        // Saves mod descriptor. Image name can be overriden for final packaging
        /*
        <ModDescriptor_DKAS
        type = "LEVELS"
        name="Adds Testing level" 
        description="Just adds a single level."
        image="mod_image.png"
	    author="PixelShard"
	    gameVersion="1.0.10"
	    tags="Levels,Enemies"
	    changeNotes="">
        */
        void SaveModXML(string strXMLFilePath, string strOverrideImgPath = "")
        {
            try
            {
                XmlTextWriter xw = new XmlTextWriter(strXMLFilePath, null);
                xw.Formatting = Formatting.Indented;
                xw.WriteStartDocument();

                xw.WriteComment(@"All paths are relative to mod root folder (where this file resides).

""type"" can be SINGLE_LEVEL (level will have mod's name) or GAME_CHANGER (levels don't get loaded in the Downloaded Levels section)

""name"" should be a short(and unique) name.Never update it in the Steam Workshop interface, only update through this file.

""image"" shows up in-game but also in Steam Workshop. Should be a 16:9 PNG image.

""tags"" only show up in Steam Workshop. comma-separated values. preferably one of: ""Levels,Enemies,Objects,Replays,Weapons,Armor,Gear,Total Conversions,Interface,Sound,Other"".

""gameVersion"" leave it as it is in this template (assuming you have the latest game version). It's also shown in-game in the credits screen.
When we'll update the game with a newer version, the mod will probably be invalidated and you'll have to update it for the new version.

""changeNotes"" only used when updating a published mod on Workshop, redundant otherwise
");

                xw.WriteStartElement("ModDescriptor_DKAS");

                xw.WriteAttributeString("type", eModType_Names[(int)currentModType]);
                xw.WriteAttributeString("name", text_modName.Text.Trim());
                xw.WriteAttributeString("description", text_description.Text.Trim());

                if(strOverrideImgPath.Length > 0)
                    xw.WriteAttributeString("image", strOverrideImgPath);
                else
                    xw.WriteAttributeString("image", strImagePath);

                xw.WriteAttributeString("author", text_author.Text.Trim());
                xw.WriteAttributeString("gameVersion", text_gameVer.Text.Trim());
                xw.WriteAttributeString("tags", text_tags.Text.Trim());
                xw.WriteAttributeString("changeNotes", text_changenotes.Text.Trim());

                //write all affected files
                for (int kk = 0; kk < arrAffectedFiles.Count; kk++)
                {
                    //get file folder relative to media
                    string strFilePath = arrAffectedFiles[kk] as string;
                    string strRootFolder = parentWnd.GetMediaFolderAbsolutePath();
                    strRootFolder = Path.GetDirectoryName(strRootFolder); //remove another folder level to include "media" in file paths

                    //now write file's relative path (relative to game's root folder)
                    string strFileRel = parentWnd.GetRelativePath(strFilePath, strRootFolder);
                    strFileRel = strFileRel.Replace('\\', '/');
                    xw.WriteStartElement("File");
                    xw.WriteAttributeString("path", strFileRel);
                    xw.WriteEndElement();
                }

                //end ModDescriptor
                xw.WriteEndElement();
                //close xml doc
                xw.WriteEndDocument();
                xw.Flush();
                xw.Close();
            }
            catch (Exception ex)
            {
                MessageBox.Show("Error saving Mod descriptor XML!\n\r" + ex.Message);
            }
        }

        void LoadModXML(string strXMLFilePath)
        {
            string strRootFolder = parentWnd.GetMediaFolderAbsolutePath();
            if (strRootFolder.Length == 0)
            {
                MessageBox.Show("You should load a level first to set the working paths. You can remove the file from the Affected Files list if you don't change it.");
                return;
            }
            //remove another folder level to include "media" in file paths
            strRootFolder = Path.GetDirectoryName(strRootFolder);

            XmlReaderSettings readerSettings = new XmlReaderSettings();
            readerSettings.IgnoreComments = true;
            using (XmlReader reader = XmlReader.Create(strXMLFilePath, readerSettings))
            {
                XmlDocument xdoc = new XmlDocument();
                xdoc.Load(reader);
                //now read data without comments
                XmlNodeList nodes = xdoc.GetElementsByTagName("ModDescriptor_DKAS");
                foreach (XmlNode node in nodes)
                {
                    XmlNode nodeattr = node.Attributes.GetNamedItem("type");
                    if (nodeattr != null)
                    {
                        if (nodeattr.Value.ToUpper() == eModType_Names[0])
                            combo_modType.SelectedIndex = 0;
                        else
                            combo_modType.SelectedIndex = 1;
                    }

                    nodeattr = node.Attributes.GetNamedItem("name");
                    if (nodeattr != null)
                        text_modName.Text = nodeattr.Value;
                    nodeattr = node.Attributes.GetNamedItem("description");
                    if (nodeattr != null)
                        text_description.Text = nodeattr.Value;
                    nodeattr = node.Attributes.GetNamedItem("author");
                    if (nodeattr != null)
                        text_author.Text = nodeattr.Value;
                    nodeattr = node.Attributes.GetNamedItem("gameVersion");
                    if (nodeattr != null)
                        text_gameVer.Text = nodeattr.Value;
                    nodeattr = node.Attributes.GetNamedItem("tags");
                    if (nodeattr != null)
                        text_tags.Text = nodeattr.Value;
                    nodeattr = node.Attributes.GetNamedItem("changeNotes");
                    if (nodeattr != null)
                        text_changenotes.Text = nodeattr.Value;
                    //load image too
                    nodeattr = node.Attributes.GetNamedItem("image");
                    if (nodeattr != null)
                    {
                        string strImgPath = nodeattr.Value;
                        try
                        {
                            pb_Image.Image = new Bitmap(strImgPath);
                            pb_Image.SizeMode = PictureBoxSizeMode.Zoom;
                            pb_Image.Invalidate();
                            //save image path too
                            strImagePath = strImgPath;
                        }
                        catch (Exception ex)
                        {
                            MessageBox.Show("Error loading image!\n\r" + ex.Message);
                        }
                    }
                }
                //load affected files list
                arrAffectedFiles.Clear();
                XmlNodeList nodesfiles = xdoc.GetElementsByTagName("File");
                foreach (XmlNode node in nodesfiles)
                {
                    string strRelativePath = node.Attributes.GetNamedItem("path").InnerText;
                    strRelativePath = strRelativePath.Replace('/', '\\');
                    string strFullPath = strRootFolder + '\\' + strRelativePath;

                    arrAffectedFiles.Add(strFullPath);
                }
                PopulateAffectedFilesList();
            }
        }
        private void but_Upload_Click(object sender, EventArgs e)
        {
            //validate mod name
            if (text_modName.Text.Length < 5)
            {
                MessageBox.Show("Please provide a name for your mod!\n\rIt should be at least 5 letters long.", "Error", MessageBoxButtons.OK);
                return;
            }
            if (!System.IO.File.Exists(strImagePath))
            {
                MessageBox.Show("Please provide a picture for your mod!\n\rIt should be a PNG (16:9 format for best results)", "Error", MessageBoxButtons.OK);
                return;
            }
            if (list_files.Items.Count == 0)
            {
                MessageBox.Show("You must add at least a file to the Affected Files list!", "Error", MessageBoxButtons.OK);
                return;
            }

            string strLevelPath = parentWnd.GetFilePath();
            if (strLevelPath.Length == 0)
            {
                MessageBox.Show("There is no level loaded or you didn't save it yet!", "Error", MessageBoxButtons.OK);
                return;
            }

            string strRootFolder = parentWnd.GetMediaFolderAbsolutePath();
            //remove another folder level to include "media" in file paths
            strRootFolder = Path.GetDirectoryName(strRootFolder);


            //--- prepare data for packing ---
            try
            {
                //create temp folder
                String strTempDir = Path.GetTempPath() + "DKAS_Ed";
                if (!System.IO.Directory.Exists(strTempDir))
                    System.IO.Directory.CreateDirectory(strTempDir);

                String strWorkDir = System.IO.Path.Combine(strTempDir, "mod_root");
                //delete folder if it existed
                System.IO.Directory.Delete(strTempDir, true);
                //re-create temp folder
                System.IO.Directory.CreateDirectory(strWorkDir);
                //copy image with predefined name
                //#TODO: should allow for JPEG images too
                File.Copy(strImagePath, strWorkDir + "\\mod_image.png", true);
                //create mod descriptor there too, with default image name
                SaveModXML(strWorkDir + "\\mod_desc.xml", "mod_image.png");

                //copy all files from the Affected Files list
                for (int kk = 0; kk < arrAffectedFiles.Count; kk++)
                {
                    //get file folder relative to media
                    string strFilePath = arrAffectedFiles[kk] as string;
                    string strFileRel = parentWnd.GetRelativePath(strFilePath, strRootFolder);
                    string strFileRelDir = strWorkDir + "\\" + Path.GetDirectoryName(strFileRel) + "\\";
                    //create folder
                    parentWnd.CreateFolderIfNeeded(strFileRelDir);

                    File.Copy(strFilePath, strFileRelDir + Path.GetFileName(strFilePath), true);
                }

                if (MessageBox.Show("Mod folder created, press OK to start the upload.\n\rMod folder:\n\r" + strWorkDir, "Info", MessageBoxButtons.OKCancel) == DialogResult.Cancel)
                    return;

                //call on game to upload mod
                string strGamePath = System.IO.Path.Combine(strExePath, "..\\ActionSquad.exe");

                ProcessStartInfo startInfo = new ProcessStartInfo();
                startInfo.CreateNoWindow = false;
                startInfo.UseShellExecute = false;
                startInfo.FileName = strGamePath;
                startInfo.WindowStyle = ProcessWindowStyle.Hidden;
                startInfo.Arguments = " +upload_workshop \"" + strWorkDir + "\"";

                using (Process exeProcess = Process.Start(startInfo))
                {
                    exeProcess.WaitForExit();
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show("Upload error!\n\r" + ex.Message + "\n\r-------------------------\n\r" + ex.ToString(), "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }

        }

        private void but_selectImage_Click(object sender, EventArgs e)
        {
            OpenFileDialog sfd = new OpenFileDialog();
            //fisier binar
            sfd.Filter = "PNG Image (*.png)|*.png|All Files (*.*)|*.*";
            if (sfd.ShowDialog() == DialogResult.Cancel)
                return;
            try
            {
                pb_Image.Image = new Bitmap(sfd.FileName);
                if ((pb_Image.Image.Width > 1024) || (pb_Image.Image.Height > 1024))
                {
                    pb_Image.Image = null;
                    MessageBox.Show("Image too big! Please load a smaller image (max 1024x1024)", "Error!");
                    return;
                }

                pb_Image.SizeMode = PictureBoxSizeMode.Zoom;
                pb_Image.Invalidate();
                //save image path too
                strImagePath = sfd.FileName;
            }
            catch (Exception ex)
            {
                MessageBox.Show("Error loading image!\n\r" + ex.Message);
            }
        }

        private void but_cancel_Click(object sender, EventArgs e)
        {
            this.Close();
        }

        private void saveMODDescriptorToolStripMenuItem_Click(object sender, EventArgs e)
        {
            SaveFileDialog sfd = new SaveFileDialog();
            //fisier binar
            sfd.Filter = "XML Document (*.xml)|*.xml|All Files (*.*)|*.*";
            if (sfd.ShowDialog() == DialogResult.Cancel)
                return;

            SaveModXML(sfd.FileName);
        }

        private void exitToolStripMenuItem_Click(object sender, EventArgs e)
        {
            this.Close();
        }

        private void but_filesRemove_Click(object sender, EventArgs e)
        {
            if (list_files.SelectedIndex >= 0)
                arrAffectedFiles.RemoveAt(list_files.SelectedIndex);

            PopulateAffectedFilesList();
        }

        private void but_filesAdd_Click(object sender, EventArgs e)
        {
            OpenFileDialog sfd = new OpenFileDialog();
            //fisier binar
            sfd.Filter = "All Files (*.*)|*.*";
            if (sfd.ShowDialog() == DialogResult.Cancel)
                return;

            //add relative path to list
            string strRootFolder = parentWnd.GetMediaFolderAbsolutePath();
            if (strRootFolder.Length == 0)
            {
                MessageBox.Show("You have to load a level first!", "Warning", MessageBoxButtons.OK);
                return;
            }
            //remove another folder level to include "media" in file paths
            strRootFolder = Path.GetDirectoryName(strRootFolder);
            //get file folder relative to media
            string strFileRel = parentWnd.GetRelativePath(sfd.FileName, strRootFolder);
            if ((strFileRel.Contains("..")) || (!strFileRel.Contains("media")))
            {
                MessageBox.Show("You can only add files from the game's \"media\" folder", "Warning", MessageBoxButtons.OK);
                return;
            }
            //add it
            arrAffectedFiles.Add(sfd.FileName);
            PopulateAffectedFilesList();
        }

        private void openMODDescriptorToolStripMenuItem_Click(object sender, EventArgs e)
        {
            OpenFileDialog sfd = new OpenFileDialog();
            //fisier binar
            sfd.Filter = "XML Document (*.xml)|*.xml|All Files (*.*)|*.*";
            if (sfd.ShowDialog() == DialogResult.Cancel)
                return;

            LoadModXML(sfd.FileName);
        }

        private void combo_modType_SelectedIndexChanged(object sender, EventArgs e)
        {
            currentModType = (eModType)combo_modType.SelectedIndex;
        }
    }
}
