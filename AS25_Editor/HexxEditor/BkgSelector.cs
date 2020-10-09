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
    public partial class BkgSelector : Form
    {
        Form1 parentWnd = null;

        //keeps background data loaded from the XML
        public class CBkgDef
        {
            public string strName;      //friendly list name
            public string strAIdata;    //actual hardcoded AI data

            public CBkgDef()
            {
                strName = "";
                strAIdata = "";
            }
        }
        //list of loaded bkg templates
        List<CBkgDef> arrBackgrounds = new List<CBkgDef>();

        public BkgSelector(Form1 pParent)
        {
            //save pointer to parent
            parentWnd = pParent;

            InitializeComponent();

            lv_backgrounds.Columns[0].Width = lv_backgrounds.Width - 5;
            LoadBackgroundsList();

            //select actual background
            ArrayList arrParams = parentWnd.GetLevelBackgroundData();
            //format list to string
            string strParams = "";
            for (int kk = 0; kk < arrParams.Count / 2; kk++)
            {
                string param = arrParams[kk * 2] as string;
                string value = arrParams[kk * 2 + 1] as string;
                strParams += param + " = " + value + ";";
            }
            //find actual background !!!
            strParams = strParams.Replace(" ", "").ToUpper();
            int nFoundIdx = -1;
            for (int kk = 0; kk < arrBackgrounds.Count(); kk++)
            {
                string strData = arrBackgrounds[kk].strAIdata.Replace(" ", "").ToUpper();
                if (strData == strParams)
                {
                    nFoundIdx = kk;
                    break;
                }
            }
            if (nFoundIdx == -1)
            {
                MessageBox.Show("Current background wasn't found in the background templates list. Select a background from the list to set one or don't select one to keep your current background.", "Info");
            }
            else
            {
                lv_backgrounds.Items[nFoundIdx].Selected = true;
            }

        }

        public int LoadBackgroundsList()
        {
            //fill behavior list
            String exePath = Path.GetDirectoryName(System.Reflection.Assembly.GetExecutingAssembly().GetName().CodeBase);
            exePath = exePath.Substring(6);
            string strPathToXML = exePath + "\\editorData\\backgrounds.xml";

            lv_backgrounds.Items.Clear();
            arrBackgrounds.Clear();

            try
            {
                XmlReaderSettings readerSettings = new XmlReaderSettings();
                readerSettings.IgnoreComments = true;
                using (XmlReader reader = XmlReader.Create(strPathToXML, readerSettings))
                {
                    XmlDocument xdoc = new XmlDocument();
                    xdoc.Load(reader);

                    XmlNodeList nodes = xdoc.GetElementsByTagName("BACKGROUNDS");
                    foreach (XmlNode node in nodes[0].ChildNodes)
                    {
                        CBkgDef pDef = new CBkgDef();
                        XmlNode xNode = null;

                        xNode = node.Attributes.GetNamedItem("sName");
                        if (xNode != null)
                            pDef.strName = xNode.Value;

                        xNode = node.Attributes.GetNamedItem("sAI");
                        if (xNode != null)
                            pDef.strAIdata = xNode.Value;
                        //add template
                        arrBackgrounds.Add(pDef);
                    }
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show("Could not read backgrounds!\n\n" + ex.Message, "Warning", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }

            //add all to list
            lv_backgrounds.Items.Clear();
            for (int kk = 0; kk < arrBackgrounds.Count(); kk++)
            {
                ListViewItem lvi = new ListViewItem(Path.GetFileNameWithoutExtension(arrBackgrounds[kk].strName));
                lv_backgrounds.Items.Add(lvi);
            }

            return lv_backgrounds.Items.Count;
        }

        private void lv_backgrounds_SizeChanged(object sender, EventArgs e)
        {
            lv_backgrounds.Columns[0].Width = lv_backgrounds.Width - 5;
        }

        private void BkgSelector_FormClosing(object sender, FormClosingEventArgs e)
        {
            ArrayList arrParams;
            int nSelectedIdx = -1;
            if (lv_backgrounds.SelectedIndices.Count > 0)
                nSelectedIdx = lv_backgrounds.SelectedIndices[0];
            if (nSelectedIdx >= 0)
            {
                parentWnd.g_wndAI.GetParamsFromString(arrBackgrounds[nSelectedIdx].strAIdata, out arrParams);
                parentWnd.SetLevelBackgroundData(arrParams);
            }
            else
            {
                MessageBox.Show("Background not modified! No selection was made.", "Info");
            }
        }
    }
}
