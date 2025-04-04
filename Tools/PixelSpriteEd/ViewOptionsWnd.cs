using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace InkEd3
{

    public partial class ViewOptionsWnd : Form
    {
        public delegate void ActionCallback(CallbackCommands command, Object parms);
        //callbackul
        ActionCallback callbackFunc = null;
        Sprite sprite = null;

        
        //clasa de transport - parametri necesari la instantierea ferestrei pt setarea corespunzatoare a valorilor controalelor
        public class ViewOptionsWndParams
        {
            //modules
            public int wandTreshold;
            //anim
            public bool showAnimPath;
            //frames
            public bool showLinkedHitpts;
            //options
            public int gridSize;
            public int majorGridSize;
            //export
            public bool bSaveNonIndentedJSON;
        };

        ViewOptionsWndParams wndparams = new ViewOptionsWndParams();

        //scrie schimbarile in structura
        void CompactChanges()
        {
            wndparams.wandTreshold = Convert.ToInt32(modNumWTreshold.Value);
            wndparams.showAnimPath = AnimCBShowPath.Checked;
            wndparams.showLinkedHitpts = FramesCBShowLinkedHitpts.Checked;

            wndparams.gridSize = Convert.ToInt32(OptionsNumGridSize.Value);
            wndparams.majorGridSize = Convert.ToInt32(OptionsNumMajorGrid.Value);

            wndparams.bSaveNonIndentedJSON = chk_optionsExportSmallJSON.Checked;
        }

        //scrie in controale valorile din structura
        void UpdateChanges()
        {
            modNumWTreshold.Value = wndparams.wandTreshold;
            AnimCBShowPath.Checked = wndparams.showAnimPath;
            FramesCBShowLinkedHitpts.Checked = wndparams.showLinkedHitpts;

            OptionsNumGridSize.Value = wndparams.gridSize;
            OptionsNumMajorGrid.Value = wndparams.majorGridSize;

            chk_optionsExportSmallJSON.Checked = wndparams.bSaveNonIndentedJSON;
        }

        public ViewOptionsWnd(ActionCallback n_callbackFunc, Sprite n_sprite, ViewOptionsWndParams wndParams)
        {
            callbackFunc = n_callbackFunc;
            sprite = n_sprite;
            InitializeComponent();

            wndparams = wndParams;
            UpdateChanges();
        }
        
        public ViewOptionsWnd()
        {
            InitializeComponent();
        }


        private void butOK_Click(object sender, EventArgs e)
        {
            //scrie parametrii de transmis
            CompactChanges();
            //cheama actualizarea parametrilor
            callbackFunc(CallbackCommands.UpdateParams, wndparams);
        }

        private void ViewOptionsWnd_FormClosing(object sender, FormClosingEventArgs e)
        {
            if (e.CloseReason == CloseReason.UserClosing)
            {
                e.Cancel = true;
                Hide();
            }
        }
    }
}