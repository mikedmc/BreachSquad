using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using System.IO;
using Newtonsoft.Json.Linq;
using Newtonsoft.Json.Serialization;
using Newtonsoft.Json;

namespace InkEd3
{
    public partial class SpriteWnd : Form
    {
        #region Color Constants
        Color colListBack = Color.FromArgb(255, 43, 43, 43);
        Color colListText = Color.FromArgb(255, 128, 128, 128);
        Color colListSel = Color.FromArgb(255, 255, 128, 128);

        //colors
        Color colClear;
        Color colGridS;
        Color colGridM;
        Color colOrigin;
        //image boxes
        Color colThumbsBack;
        Color colThumbsSelection;

        string strLoadedFileName = "";
        bool bFileChanged = false;

        //daca afiseaza grid sau nu
        bool showAnimXRay = false;
        bool showGrid = true;
        bool invertGrid = false;
        int gridSize = 16;
        int majorGridLines = 10; //din 10 in 10
        bool bExportSmallJSON = true;

        Point cursorPbDraw; //pozitie cursor
        bool cursorHighlight = false;  //face cruce pe cursor

        public static Font fontArialBold = new Font("Arial", 8, FontStyle.Bold);
        public static Font fontArialMic = new Font("Arial", 6, FontStyle.Regular);

        public Rectangle NormalizeRectangle(Rectangle rect)
        {
            Rectangle final = new Rectangle();
            final = rect;

            if (rect.Width < 0)
            {
                final.X = rect.Left + rect.Width;
                final.Width = -rect.Width;
            }
            if (rect.Height < 0)
            {
                final.Y = rect.Top + rect.Height;
                final.Height = -rect.Height;
            }
            return final;
        }

        public HitPointWnd hitptWnd = null;
        public ViewOptionsWnd viewoptionsWnd = null;
        public FlagsBuilderWnd flagsWnd = null;

        //daca conversia a avut loc, intoarce true, altfel intoarce false
        public bool SafeConvertToInt32(string svalue, ref Int32 retval)
        {
            try
            {
                retval = Convert.ToInt32(svalue);
            }
            catch (Exception)
            {
                return false;
            }
            return true;
        }

        #endregion
        //constructor
        const int TIMER_UPDATE_PERIOD = 16; //ms

        public SpriteWnd()
        {
            InitializeComponent();

            showAnimXRay = enableXRayToolStripMenuItem.Checked;
            //prima dimensionare
            ResizeDrawArea();
            //intializarea listview-uri
            lvMain.ListViewItemSorter = new IndexComparer();
            lvSecond.ListViewItemSorter = new IndexComparer();
            //defaule intra pe module view
            InitModuleView();
            //mouse wheel init
            this.MouseWheel += new MouseEventHandler(MainWnd_MouseWheel);

            lvMain.MouseWheel += new MouseEventHandler(MainWnd_MouseWheel);
            lvSecond.MouseWheel += new MouseEventHandler(MainWnd_MouseWheel);

            //center origin
            ox = w / 2;
            oy = h / 2;
            //set initial views centers
            oxM = oxF = oxA = ox;
            oyM = oyF = oyA = oy;

            showGridGToolStripMenuItem.Checked = showGrid;
            invertColorsIToolStripMenuItem.Checked = invertGrid;
            highlightCursorHToolStripMenuItem.Checked = cursorHighlight;

            setInterfaceColors();

            lvMain.GridLines = false;
            lvSecond.GridLines = false;

            ToolStripMFA.Visible = true;
            ToolStripModules.Visible = true;

            ts_Modules_Click(null, null);
        }

        //tratarea evenimentului de mouse wheel pentru zoom pe draw area
        void MainWnd_MouseWheel(object sender, MouseEventArgs e)
        {

            Point locCur = pbDraw.PointToClient(Cursor.Position);
            if ((locCur.X >= 0) && (locCur.Y >= 0) && (locCur.X < pbDraw.Width) && (locCur.Y < pbDraw.Height))
            {
                if (e.Delta > 0)
                {
                    //screenc
                    int scrcx = w / 2 - ox;
                    int scrcy = h / 2 - oy;

                    ox -= (int)(scrcx / scale);
                    oy -= (int)(scrcy / scale);

                    scale += 1.0f;
                }
                else if (e.Delta < 0)
                {
                    if (scale > 1.0f)
                    {
                        //screenc
                        int scrcx = w / 2 - ox;
                        int scrcy = h / 2 - oy;

                        ox += (int)(scrcx / scale);
                        oy += (int)(scrcy / scale);

                        scale -= 1.0f;
                        if (scale < 1.0f)
                            scale = 1.0f;

                    }
                }
            }

            statusZoom.Text = "Zoom: " + Math.Round(scale * 100) + "%";
            pbDraw.Refresh();
        }

        //variabilele mele globale (sau majoritatea macar)
        public Sprite sprite = new Sprite(); //sprite-ul curent

        public void RefreshDrawArea()
        {
            pbDraw.Refresh();
        }

        #region ModuleView

        float scaleM = 1.0f; //zoom-ul pt module view 
        int oxM = 100, oyM = 100; //offset-ul sist de referinta pt module view

        int wandTreshold = 0;
        //initializeaza coloanele listei Main si alte elemente ale ecranului de module
        void InitModuleView()
        {
            tsb_modSelect.Checked = true;
            tsb_modCreate.Checked = false;
            tsb_modWand.Checked = false;
            tool = Tools.Select;

            scale = scaleM;
            ox = oxM;
            oy = oyM;
            lvMain.Columns.Clear();
            lvMain.Columns.Add("Index", 40);
            lvMain.Columns.Add("Image", 50);
            lvMain.Columns.Add("X", 45);
            lvMain.Columns.Add("Y", 45);
            lvMain.Columns.Add("W", 45);
            lvMain.Columns.Add("H", 45);
            lvMain.Columns.Add("Name", 60);
            splitMS.Panel2Collapsed = true;
            populateModuleList();
            populateImageThumbs();
        }

        //(re)populeaza lista Main cu modulele (deja) existente in sprite
        void populateModuleList()
        {
            lvMain.Items.Clear();
            //populeaza lista cu module
            lvMain.BeginUpdate();
            foreach (Module mod in sprite.modules)
            {
                addModule(mod, false);
            }
            lvMain.EndUpdate();
            if (sprite.modules.Count != 0)
            {
                SelectModule(0);
                lvMain.AutoResizeColumns(ColumnHeaderAutoResizeStyle.ColumnContent);
            }
        }

        //pune in thumbs imaginile pe care se construiesc modulele
        void populateImageThumbs()
        {
            AddThumbs(sprite.images.Count);
        }

        Module module = null; //modulul curent

        //sterge modul in sprite + reindexare
        void DeleteModule(int idx) //merge bine doar pt unul o data
        {
            sprite.DeleteModule(idx);
            SetModified(true);
        }

        //face resize la toate modulele selectate
        void ResizeSelectedModules(int dx, int dy)
        {
            foreach (Module mod in sprite.modules)
            {
                ListViewItem lvi = lvMain.Items[mod.index];
                if (lvi.Selected)
                {
                    mod.w += dx;
                    mod.h += dy;

                    lvi.SubItems[4].Text = "" + mod.w;
                    lvi.SubItems[5].Text = "" + mod.h;
                    if (!sprite.IsModuleCorrect(mod))
                    {
                        lvi.ForeColor = colListSel;
                    }
                    else
                    {
                        lvi.ForeColor = colListText;
                    }
                }
            }
            SetModified(true);
        }

        //modifica w,h al modulului curent si actulizeaza si main list view
        void ResizeModule(int dx, int dy)
        {
            if (module == null)
                return;
            module.w += dx;
            module.h += dy;

            ListViewItem lvi = GetItemByIndex(lvMain, module.index);
            lvi.SubItems[4].Text = "" + module.w;
            lvi.SubItems[5].Text = "" + module.h;
            if (!sprite.IsModuleCorrect(module))
            {
                lvi.ForeColor = colListSel;
            }
            else
            {
                lvi.ForeColor = colListText;
            }
            SetModified(true);
        }

        //muta toate modulele selectate
        void MoveSelectedModules(int dx, int dy)
        {
            foreach (Module mod in sprite.modules)
            {
                ListViewItem lvi = lvMain.Items[mod.index];
                if (lvi.Selected)
                {
                    mod.x += dx;
                    mod.y += dy;

                    lvi.SubItems[2].Text = "" + mod.x;
                    lvi.SubItems[3].Text = "" + mod.y;
                    if (!sprite.IsModuleCorrect(mod))
                    {
                        lvi.ForeColor = colListSel;
                    }
                    else
                    {
                        lvi.ForeColor = colListText;
                    }
                }
            }
            SetModified(true);
        }

        //modifica x,y al modulului curent si actulizeaza si main list view
        void MoveModule(int dx, int dy)
        {
            if (module == null)
                return;
            module.x += dx;
            module.y += dy;
            ListViewItem lvi = lvMain.Items[module.index];
            lvi.SubItems[2].Text = "" + module.x;
            lvi.SubItems[3].Text = "" + module.y;
            if (!sprite.IsModuleCorrect(module))
            {
                lvi.ForeColor = colListSel;
            }
            else
            {
                lvi.ForeColor = colListText;
            }
            //lvMain.Refresh();
            SetModified(true);
        }

        //adauga in lista un modul deja existent
        void addModule(Module mod, bool select)
        {
            ListViewItem lvi = new ListViewItem();
            lvi.Text = "" + mod.index;
            lvi.SubItems.Add("" + mod.imageID);
            lvi.SubItems.Add("" + mod.x);
            lvi.SubItems.Add("" + mod.y);
            lvi.SubItems.Add("" + mod.w);
            lvi.SubItems.Add("" + mod.h);
            lvi.SubItems.Add(mod.strName);
            lvi.BackColor = colListBack;

            if (!sprite.IsModuleCorrect(mod))
            {
                lvi.ForeColor = colListSel;
            }
            else
            {
                lvi.ForeColor = colListText;
            }
            //cand adauga modulul il si selecteaza doar pe el
            if (select)
            {
                RemoveSelection(lvMain);
                lvi.Selected = true;
            }
            lvMain.Items.Add(lvi);
        }

        //creaza si adauga un nou modul
        Module NewModule(int imgIdx, double x, double y, double w, double h, bool select = true)
        {
            module = sprite.AddModule();
            module.index = lvMain.Items.Count; //asta e append, pt insert va fi alt index
            module.x = (int)x;
            module.y = (int)y;
            module.w = (int)w;
            module.h = (int)h;
            module.imageID = imgIdx;
            addModule(module, select);
            SetModified(true);

            return module;
        }

        Module NewModule(Module clone)
        {
            module = sprite.AddModule();
            module.index = lvMain.Items.Count; //asta e append, pt insert va fi alt index
            module.x = (int)clone.x;
            module.y = (int)clone.y;
            module.w = (int)clone.w;
            module.h = (int)clone.h;
            module.imageID = clone.imageID;
            addModule(module, true);
            SetModified(true);

            return module;
        }

        Module NewModuleOff(Module clone, int offsetX, int offsetY)
        {
            module = sprite.AddModule();
            module.index = lvMain.Items.Count; //asta e append, pt insert va fi alt index
            module.x = (int)clone.x + offsetX;
            module.y = (int)clone.y + offsetY;
            module.w = (int)clone.w;
            module.h = (int)clone.h;
            module.imageID = clone.imageID;
            addModule(module, false);
            SetModified(true);

            return module;
        }

        //cloneaza modulul selectat in lista
        void CloneModule()
        {
            foreach (ListViewItem lvi in lvMain.SelectedItems)
            {
                Module sel = sprite.GetModuleByIndex(GetIndex(lvi));
                Module mod = NewModule(sel);
            }
            SetModified(true);
        }

        //sabileste noul modul curent si reflecta schimbarile in zona de desen
        void SelectModule(int i)
        {
            module = sprite.GetModuleByIndex(i);
            if (module != null)
            {
                imgIdx = module.imageID;
                img = null;
                if ((imgIdx >= 0) && (imgIdx < sprite.images.Count))
                    img = sprite.images[imgIdx];
            }
            pbDraw.Refresh();
            pThumbs.Refresh();
        }

        //interschimbarea a 2 module in lista din sprite (ar putea fi implementata in Sprite)
        //folosita la mutarea in list view
        void SwapModules(int minIdx, int maxIdx, bool loop)
        {
            if (minIdx > maxIdx) return;

            if (loop == false)
            {
                Module mi = sprite.GetModuleByIndex(minIdx);
                Module mj = sprite.GetModuleByIndex(minIdx + 1);
                int temp = mi.index;
                mi.index = mj.index;
                mj.index = temp;
                return;
            }

            for (int ii = minIdx + 1; ii <= maxIdx; ii++)
            {
                Module mi = sprite.GetModuleByIndex(ii);
                Module mj = sprite.GetModuleByIndex(minIdx);
                int temp = mi.index;
                mi.index = mj.index;
                mj.index = temp;
            }
            SetModified(true);
        }

        private void LoadImageOverIDX(int imageIdx)
        {
            OpenFileDialog ofd = new OpenFileDialog();
            ofd.Filter = "Portable Network Graphics (*.png)|*.png";
            if (ofd.ShowDialog() == DialogResult.Cancel)
                return;

            sprite.images[imageIdx].Dispose();
            using (FileStream stream = new FileStream(ofd.FileName, FileMode.Open, FileAccess.Read))
            {
                sprite.images[imageIdx] = Image.FromStream(stream);
            }
            sprite.imgPath[imageIdx] = Path.GetFileName(ofd.FileName);

            img = sprite.images[imageIdx];
            pbDraw.Refresh();
            pThumbs.Refresh();
        }

        private void SaveImageAs(Image sImage)
        {
            SaveFileDialog sfd = new SaveFileDialog();
            sfd.Filter = "Portable Network Graphics (*.png)|*.png|All Files (*.*)|*.*";
            if (sfd.ShowDialog() == DialogResult.Cancel)
                return;

            if (File.Exists(sfd.FileName))
                File.Delete(sfd.FileName);

            try
            {
                sImage.Save(sfd.FileName, System.Drawing.Imaging.ImageFormat.Png);
            }
            catch (Exception ex)
            {
                MessageBox.Show("Save Image Failed !\n" + ex.ToString(), "ERROR !", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        //face mirror pe X la imagine si la toate modulele, apoi face mirror la aframe-uri, frame-uri si final la animatie
        private void CreateMirroredAnimations_X()
        {
            DialogResult dr = MessageBox.Show(this, "This tool will mirror EVERYTHING to the right (all selected animations and their aframes, frames, fmodules, modules and images).\nThis operation cannot be undone !\nAre you sure you want to continue ?", "Create Mirrored Animations Warning", MessageBoxButtons.YesNo);
            if (dr != DialogResult.Yes) return;

            lvMain.BeginUpdate();

            int[] widths = new int[sprite.images.Count];
            for (int kk = 0; kk < sprite.images.Count; kk++)
            {
                Image oimg = (Image)sprite.images[kk].Clone();// Image.FromFile(sprite.imgPath[imgIdx]);
                widths[kk] = oimg.Width;
                sprite.images[kk] = new Bitmap(oimg.Width * 2, oimg.Height, System.Drawing.Imaging.PixelFormat.Format32bppArgb);

                Graphics ig = Graphics.FromImage(sprite.images[kk]);
                ig.DrawImageUnscaled(oimg, 0, 0);
                oimg.RotateFlip(RotateFlipType.RotateNoneFlipX);
                ig.DrawImageUnscaled(oimg, oimg.Width, 0);
                oimg.Dispose();
                oimg = null;
                ig = null;
                //pune pointerul din nou
                img = sprite.images[kk];
            }
            //--- acum flipeaza toate modulele ---
            sprite.modules.Sort(Sprite.CompareEntities);
            //retine nr de module
            populateModuleList(); //cand adaug modul indexul se ia dupa ultimul element din lvMain;
            int modcnt = sprite.modules.Count;
            for (int ll = 0; ll < modcnt; ll++)
            {
                Module sel = sprite.modules[ll];
                Module mod = NewModuleOff(sel, 2 * widths[sel.imageID] - 2 * sel.x - sel.w, 0);
            }
            sprite.MakeModuleImages();
            //--- acum flipeaza frame-urile ---
            sprite.frames.Sort(Sprite.CompareEntities);
            //retine nr de frames
            PopulateFrameList(); //cand adaug frame se ia indexul dupa ultimul item din lvmain
            int frmcnt = sprite.frames.Count;
            for (int ll = 0; ll < frmcnt; ll++)
            {
                Frame ofrm = sprite.frames[ll];

                Frame f = sprite.AddFrame();
                f.index = ll + frmcnt;
                foreach (FrameModule fm in ofrm.fmodules)
                {
                    Module fmd = sprite.modules[modcnt + fm.module.index];
                    FrameModule nfm = f.AddFModule(fmd, fm.flags);
                    nfm.ox = -(fmd.w + fm.ox);
                    nfm.oy = fm.oy;
                    nfm.index = fm.index;
                    nfm.flags = fm.flags;
                }
                frame = f;
                AddFrame(f, true);
            }
            //--- acum flipeaza si animatiile ---
            PopulateAnimList();
            sprite.anims.Sort(Sprite.CompareEntities);
            int anmcnt = sprite.anims.Count;
            for (int ll = 0; ll < anmcnt; ll++)
            {
                Animation origa = sprite.anims[ll];

                Animation anm = sprite.AddAnim();
                anm.name = origa.name + "_FX";
                anm.index = lvMain.Items.Count;
                foreach (AnimFrame afm in origa.aframes)
                {
                    Frame nfrm = sprite.frames[frmcnt + afm.frame.index];
                    AnimFrame nfm = anm.AddAFrame(nfrm);
                    nfm.ox = -afm.ox;
                    nfm.oy = afm.oy;
                    nfm.index = afm.index;
                    nfm.strFlags = afm.strFlags;
                }
                anim = anm;
                AddAnim(anm, false);
            }


            lvMain.EndUpdate();

            switch (view)
            {
                case Views.moduleView:
                    {
                        populateModuleList();
                    }
                    break;
                case Views.frameView:
                    {
                        PopulateFrameList();
                        PopulateFModulesList();
                    }
                    break;
                case Views.animView:
                    {
                        PopulateAnimList();
                        PopulateAFramesList();
                    }
                    break;
            }
            //refresh ALL
            pThumbs.Refresh();
            pbDraw.Refresh();

        }


        //face mirror pe X la imagine si la modulele selectate
        private void CreateMirroredImageAndModules_X()
        {
            DialogResult dr = MessageBox.Show(this, "This tool will mirror the current image and all SELECTED modules to the right.\nThis operation cannot be undone !\nAre you sure you want to continue ?", "Create Mirrored Picture Warning", MessageBoxButtons.YesNo);
            if (dr != DialogResult.Yes) return;

            Image oimg = (Image)sprite.images[imgIdx].Clone();// Image.FromFile(sprite.imgPath[imgIdx]);
            int oimgW = oimg.Width;
            sprite.images[imgIdx] = new Bitmap(oimg.Width * 2, oimg.Height, System.Drawing.Imaging.PixelFormat.Format32bppArgb);

            Graphics ig = Graphics.FromImage(sprite.images[imgIdx]);
            ig.DrawImageUnscaled(oimg, 0, 0);
            oimg.RotateFlip(RotateFlipType.RotateNoneFlipX);
            ig.DrawImageUnscaled(oimg, oimg.Width, 0);
            oimg.Dispose();
            oimg = null;
            ig = null;
            //pune pointerul din nou
            img = sprite.images[imgIdx];
            //--- acum flipeaza si modulele ---
            foreach (ListViewItem lvi in lvMain.SelectedItems)
            {
                Module sel = sprite.GetModuleByIndex(GetIndex(lvi));
                Module mod = NewModuleOff(sel, 2 * oimgW - 2 * sel.x - sel.w, 0);
            }

            pbDraw.Refresh();
            pThumbs.Refresh();
        }

        //face mirror pe Y la imagine si la modulele selectate
        private void CreateMirroredImageAndModules_Y()
        {
            DialogResult dr = MessageBox.Show(this, "This tool will mirror downwards the selected image and all SELECTED modules.\nThis operation cannot be undone !\nAre you sure you want to continue ?", "Create Mirrored Picture Warning", MessageBoxButtons.YesNo);
            if (dr != DialogResult.Yes) return;

            Image oimg = (Image)sprite.images[imgIdx].Clone();// Image.FromFile(sprite.imgPath[imgIdx]);
            int oimgH = oimg.Height;
            sprite.images[imgIdx] = new Bitmap(oimg.Width, oimg.Height * 2, System.Drawing.Imaging.PixelFormat.Format32bppArgb);

            Graphics ig = Graphics.FromImage(sprite.images[imgIdx]);
            ig.DrawImageUnscaled(oimg, 0, 0);
            oimg.RotateFlip(RotateFlipType.RotateNoneFlipY);
            ig.DrawImageUnscaled(oimg, 0, oimg.Height);
            oimg.Dispose();
            oimg = null;
            ig = null;
            //pune pointerul din nou
            img = sprite.images[imgIdx];

            //--- acum flipeaza si modulele ---
            foreach (ListViewItem lvi in lvMain.SelectedItems)
            {
                Module sel = sprite.GetModuleByIndex(GetIndex(lvi));
                Module mod = NewModuleOff(sel, 0, 2 * oimgH - 2 * sel.y - sel.h);
            }

            pbDraw.Refresh();
            pThumbs.Refresh();
        }

        //scaleaza totul
        private void ScaleAnimations(float percent)
        {
            DialogResult dr = MessageBox.Show(this, "This tool will scale EVERYTHING (all selected animations and their aframes, frames, fmodules, modules and images).\nThis operation cannot be undone !\nAre you sure you want to continue ?", "Scale Animations Warning", MessageBoxButtons.YesNo);
            if (dr != DialogResult.Yes) return;

            lvMain.BeginUpdate();

            //--- scaleaza toate modulele ---
            sprite.modules.Sort(Sprite.CompareEntities);
            //retine nr de module
            populateModuleList(); //cand adaug modul indexul se ia dupa ultimul element din lvMain;
            int modcnt = sprite.modules.Count;
            for (int ll = 0; ll < modcnt; ll++)
            {
                Module sel = sprite.modules[ll];
                sel.w = (int)(sel.w * percent);
                sel.h = (int)(sel.h * percent);
                sel.x = (int)(sel.x * percent);
                sel.y = (int)(sel.y * percent);
            }
            sprite.MakeModuleImages();
            //--- acum scaleaza frame-urile ---
            sprite.frames.Sort(Sprite.CompareEntities);
            //retine nr de frames
            PopulateFrameList(); //cand adaug frame se ia indexul dupa ultimul item din lvmain
            int frmcnt = sprite.frames.Count;
            for (int ll = 0; ll < frmcnt; ll++)
            {
                Frame ofrm = sprite.frames[ll];
                ofrm.BBox.X = (int)(ofrm.BBox.X * percent);
                ofrm.BBox.Y = (int)(ofrm.BBox.Y * percent);
                ofrm.BBox.Width = (int)(ofrm.BBox.Width * percent);
                ofrm.BBox.Height = (int)(ofrm.BBox.Height * percent);

                foreach (HitPoint hp in ofrm.hitPoints)
                {
                    hp.X = (int)(hp.X * percent);
                    hp.Y = (int)(hp.Y * percent);
                }

                foreach (FrameModule fm in ofrm.fmodules)
                {
                    fm.ox = (int)(fm.ox * percent);
                    fm.oy = (int)(fm.oy * percent);
                }


            }
            //--- acum flipeaza si animatiile ---
            PopulateAnimList();
            sprite.anims.Sort(Sprite.CompareEntities);
            int anmcnt = sprite.anims.Count;
            for (int ll = 0; ll < anmcnt; ll++)
            {
                Animation origa = sprite.anims[ll];

                foreach (AnimFrame afm in origa.aframes)
                {
                    afm.ox = (int)(afm.ox * percent);
                    afm.oy = (int)(afm.oy * percent);
                }
            }


            lvMain.EndUpdate();

            switch (view)
            {
                case Views.moduleView:
                    {
                        populateModuleList();
                    }
                    break;
                case Views.frameView:
                    {
                        PopulateFrameList();
                        PopulateFModulesList();
                    }
                    break;
                case Views.animView:
                    {
                        PopulateAnimList();
                        PopulateAFramesList();
                    }
                    break;
            }
            //refresh ALL
            pThumbs.Refresh();
            pbDraw.Refresh();

        }


        #endregion

        #region FrameView

        Frame frame = null; //frame-ul curent
        FrameModule fmodule = null; //frame module-ul curent
        float scaleF = 1.0f;
        int oxF = 100, oyF = 100;
        //daca sa apara in bara de jos flipate modulele, ca sa le poti baga direct
        bool showFlipXMods = false;
        bool showFlipYMods = false;

        //in fereastra de optiuni. Arata hitpts legate intre ele pt a defini contururi cu ele
        bool showLinkedHitpts = false;
        bool showLevelsOnHitPoints = false;
        //index hitpt selectat
        int selectedHitPt = -1;
        int oldhitptIdx = -1; //folosit la selectie hitpoint


        void InitFrameView()
        {
            lvMain.Items.Clear();
            lvMain.Columns.Clear();
            lvMain.Columns.Add("Index", 20);
            lvMain.Columns.Add("FModules", 30);
            lvMain.Columns.Add("BBox(xywh)", 80);
            lvMain.Columns.Add("HitPoints", 40);
            splitMS.Panel2Collapsed = false;
            lvSecond.Items.Clear();
            lvSecond.Columns.Clear();
            lvSecond.Columns.Add("Index", 40);
            lvSecond.Columns.Add("OX", 40);
            lvSecond.Columns.Add("OY", 40);
            lvSecond.Columns.Add("Flip Flags", 80);
            sprite.MakeModuleImages();
            AddThumbs(sprite.modules.Count);
            fmodule = null;
            scale = scaleF;
            ox = oxF;
            oy = oyF;
            PopulateFrameList();

            tool = Tools.Select;
            tsb_frmSelect.Checked = true;
            tsb_frmBBox.Checked = false;
            tsb_frmPoints.Checked = false;
        }

        void PopulateFrameList(int selectindex)
        {
            lvMain.BeginUpdate();
            lvMain.Items.Clear();
            foreach (Frame f in sprite.frames)
                AddFrame(f, false);
            SelectFrame(selectindex);
            lvMain.Items[selectindex].Selected = true;
            lvMain.EndUpdate();
            /*
            lvMain.AutoResizeColumns(ColumnHeaderAutoResizeStyle.ColumnContent);
            */
        }
        void PopulateFrameList()
        {
            lvMain.BeginUpdate();
            lvMain.Items.Clear();
            foreach (Frame f in sprite.frames)
                AddFrame(f, false);
            SelectFrame(0);
            lvMain.EndUpdate();
            /*
            if (sprite.frames.Count > 0)
                lvMain.AutoResizeColumns(ColumnHeaderAutoResizeStyle.ColumnContent);
                */
        }

        void AddFrame(Frame f, bool selectLast)
        {
            item = new ListViewItem();
            item.Text = "" + f.index;
            item.SubItems.Add("" + f.fmodules.Count);
            if (selectLast)
            {
                RemoveSelection(lvMain);
                item.Selected = true;
            }
            string collrect = f.BBox.X + "," + f.BBox.Y + ";" + f.BBox.Width + "," + f.BBox.Height;
            item.SubItems.Add(collrect);
            item.SubItems.Add(f.hitPoints.Count.ToString());
            item.BackColor = colListBack;
            item.ForeColor = colListText;
            lvMain.Items.Add(item);
            lvMain.Focus();
            SetModified(true);
        }

        void EditFrameBBox(Frame frm, int moveX, int moveY, int changeW, int changeH)
        {
            frm.BBox.X += moveX;
            frm.BBox.Y += moveY;
            frm.BBox.Width += changeW;
            frm.BBox.Height += changeH;
            string collrect = frm.BBox.X + "," + frm.BBox.Y + ";" + frm.BBox.Width + "," + frm.BBox.Height;
            lvMain.Items[frm.index].SubItems[2].Text = collrect;
            SetModified(true);
        }

        void NewFrame()
        {
            frame = sprite.AddFrame();
            frame.index = lvMain.Items.Count;
            AddFrame(frame, true);
        }

        //cloneaza toate frame-urile selectate
        void CloneSelectedFrames()
        {
            foreach (ListViewItem lvi in lvMain.SelectedItems)
            {
                CloneFrame(lvi.Index);
            }
            SetModified(true);
        }

        //cloneaza frame-ul cu index si il adauga la sfarsitul listei
        void CloneFrame(int index)
        {
            Frame ofrm = sprite.GetFrameByIndex(index);
            if (ofrm == null) return;

            Frame f = sprite.AddFrame();
            f.index = lvMain.Items.Count;
            foreach (FrameModule fm in ofrm.fmodules)
            {
                FrameModule nfm = f.AddFModule(fm.module, fm.flags);
                nfm.ox = fm.ox;
                nfm.oy = fm.oy;
                nfm.index = fm.index;
                nfm.flags = fm.flags;
            }
            //bbox
            f.BBox = ofrm.BBox;
            //hitpoints
            for (int kk = 0; kk < ofrm.hitPoints.Count; kk++)
            {
                HitPoint nhp = new HitPoint(ofrm.hitPoints[kk].X, ofrm.hitPoints[kk].Y);
                nhp.flags = ofrm.hitPoints[kk].flags;
                f.hitPoints.Add(nhp);
            }
            //select new frame
            frame = f;
            AddFrame(f, true);
        }

        //hitpoints list
        void PopulatePointsList()
        {
            lvSecond.BeginUpdate();
            lvSecond.Items.Clear();
            if (frame == null)
            {
                lvSecond.EndUpdate();
                return;
            }
            for (int kk = 0; kk < frame.hitPoints.Count; kk++)
            {
                HitPoint hp = frame.hitPoints[kk];

                ListViewItem lvi = new ListViewItem();
                lvi.Text = "" + kk;
                lvi.SubItems.Add("" + hp.X);
                lvi.SubItems.Add("" + hp.Y);
                lvi.BackColor = colListBack;
                lvi.ForeColor = colListText;
                lvi.SubItems.Add("" + hp.flags);
                lvSecond.Items.Add(lvi);
            }
            lvSecond.EndUpdate();
        }
        //populeaza lista de frame modules pt frame-ul curent
        void PopulateFModulesList()
        {
            lvSecond.BeginUpdate();
            lvSecond.Items.Clear();
            if (frame == null)
            {
                lvSecond.EndUpdate();
                return;
            }
            foreach (FrameModule fm in frame.fmodules)
                AddFModule(fm);
            lvSecond.EndUpdate();
        }

        void SelectFrame(int i)
        {
            frame = sprite.GetFrameByIndex(i);
            fmodule = null;
            PopulateFModulesList();
            pbDraw.Refresh();
            lvMain.Focus();
        }

        void HFilpFlop()
        {
            if (lvSecond.SelectedItems.Count > 0)
            {
                Rectangle rect = frame.GetFModuleByIndex(lvSecond.SelectedItems[0].Index).GetRect();

                foreach (ListViewItem lvi in lvSecond.SelectedItems)
                {
                    FrameModule fm = frame.GetFModuleByIndex(lvi.Index);
                    rect = Rectangle.Union(rect, fm.GetRect());
                }

                int dx = -rect.X;
                if (dx == 0) dx = -rect.Width;

                MoveSelectedFModules(dx, 0);

                pbDraw.Refresh();
                return;
            }

            foreach (ListViewItem flvi in lvMain.SelectedItems)
            {
                Frame frm = sprite.GetFrameByIndex(flvi.Index);
                if (frm == null) return;
                if (frm.fmodules.Count == 0) continue;

                Rectangle rect = frm.fmodules[0].GetRect();

                foreach (FrameModule fm in frm.fmodules)
                    rect = Rectangle.Union(rect, fm.GetRect());

                int dx = -rect.X;
                if (dx == 0) dx = -rect.Width;

                foreach (FrameModule fm in frm.fmodules)
                    fm.ox += dx;
            }
            pbDraw.Refresh();
        }

        void VFilpFlop()
        {
            if (lvSecond.SelectedItems.Count > 0)
            {
                Rectangle rect = frame.GetFModuleByIndex(lvSecond.SelectedItems[0].Index).GetRect();

                foreach (ListViewItem lvi in lvSecond.SelectedItems)
                {
                    FrameModule fm = frame.GetFModuleByIndex(lvi.Index);
                    rect = Rectangle.Union(rect, fm.GetRect());
                }

                int dy = -rect.Y;
                if (dy == 0) dy = -rect.Height;

                MoveSelectedFModules(0, dy);

                pbDraw.Refresh();
                return;
            }

            foreach (ListViewItem flvi in lvMain.SelectedItems)
            {
                Frame frm = sprite.GetFrameByIndex(flvi.Index);
                if (frm == null) return;
                if (frm.fmodules.Count == 0) continue;

                Rectangle rect = frm.fmodules[0].GetRect();

                foreach (FrameModule fm in frm.fmodules)
                    rect = Rectangle.Union(rect, fm.GetRect());

                int dy = -rect.Y;
                if (dy == 0) dy = -rect.Height;

                MoveSelectedFModules(0, dy);

                foreach (FrameModule fm in frm.fmodules)
                    fm.oy += dy;
            }
            pbDraw.Refresh();
        }

        void HCenterSelectedFModules()
        {
            if (lvSecond.SelectedItems.Count > 0)
            {
                Rectangle rect = frame.GetFModuleByIndex(lvSecond.SelectedItems[0].Index).GetRect();

                foreach (ListViewItem lvi in lvSecond.SelectedItems)
                {
                    FrameModule fm = frame.GetFModuleByIndex(lvi.Index);
                    rect = Rectangle.Union(rect, fm.GetRect());
                }

                int dx = -rect.Width / 2 - rect.X;

                MoveSelectedFModules(dx, 0);

                pbDraw.Refresh();
                return;
            }

            foreach (ListViewItem flvi in lvMain.SelectedItems)
            {
                Frame frm = sprite.GetFrameByIndex(flvi.Index);
                if (frm == null) return;
                if (frm.fmodules.Count == 0) continue;

                Rectangle rect = frm.fmodules[0].GetRect();

                foreach (FrameModule fm in frm.fmodules)
                    rect = Rectangle.Union(rect, fm.GetRect());

                int dx = -rect.Width / 2 - rect.X;

                foreach (FrameModule fm in frm.fmodules)
                    fm.ox += dx;
            }
            pbDraw.Refresh();
        }

        void VCenterSelectedFModules()
        {
            if (lvSecond.SelectedItems.Count > 0)
            {
                Rectangle rect = frame.GetFModuleByIndex(lvSecond.SelectedItems[0].Index).GetRect();

                foreach (ListViewItem lvi in lvSecond.SelectedItems)
                {
                    FrameModule fm = frame.GetFModuleByIndex(lvi.Index);
                    rect = Rectangle.Union(rect, fm.GetRect());
                }

                int dy = -rect.Height / 2 - rect.Y;

                MoveSelectedFModules(0, dy);

                pbDraw.Refresh();
                return;
            }

            foreach (ListViewItem flvi in lvMain.SelectedItems)
            {
                Frame frm = sprite.GetFrameByIndex(flvi.Index);
                if (frm == null) return;
                if (frm.fmodules.Count == 0) continue;

                Rectangle rect = frm.fmodules[0].GetRect();

                foreach (FrameModule fm in frm.fmodules)
                    rect = Rectangle.Union(rect, fm.GetRect());

                int dy = -rect.Height / 2 - rect.Y;

                foreach (FrameModule fm in frm.fmodules)
                    fm.oy += dy;
            }
            pbDraw.Refresh();
        }

        void MoveSelectedFrames(int dx, int dy)
        {
            foreach (ListViewItem lvi in lvMain.SelectedItems)
            {
                Frame frm = sprite.GetFrameByIndex(lvi.Index);
                if (frm == null) return;

                if (frm.BBox.Width > 0 && frm.BBox.Height > 0)
                {
                    frm.BBox.X += dx;
                    frm.BBox.Y += dy;
                }

                foreach (FrameModule fm in frm.fmodules)
                {
                    fm.ox += dx;
                    fm.oy += dy;
                }
            }
            PopulateFModulesList();
            SetModified(true);
        }

        void DeleteFrame(int i)
        {
            sprite.DeleteFrame(i);
            SetModified(true);
        }

        void SwapFrames(int minIdx, int maxIdx, bool loop)
        {
            if (minIdx > maxIdx) return;

            if (loop == false)
            {
                Frame fi = sprite.GetFrameByIndex(minIdx);
                Frame fj = sprite.GetFrameByIndex(minIdx + 1);
                int temp = fi.index;
                fi.index = fj.index;
                fj.index = temp;

                return;
            }

            for (int ii = minIdx + 1; ii <= maxIdx; ii++)
            {
                Frame fi = sprite.GetFrameByIndex(ii);
                Frame fj = sprite.GetFrameByIndex(minIdx);
                int temp = fi.index;
                fi.index = fj.index;
                fj.index = temp;
            }
        }

        //creaza un frame module si il adauga in liste
        void NewFModule(int idx)
        {
            module = sprite.GetModuleByIndex(idx);//.modules[idx];
            int flags = 0;
            if (showFlipXMods) flags = flags | 1;
            if (showFlipYMods) flags = flags | 2;
            fmodule = frame.AddFModule(module, flags);
            fmodule.index = lvSecond.Items.Count;
            AddFModule(fmodule);
            GetItemByIndex(lvMain, frame.index).SubItems[1].Text = "" + frame.fmodules.Count; //refresh la nr de fmodule
            SetModified(true);
        }

        //adauga un frame module deja existent in second list view
        void AddFModule(FrameModule fm)
        {
            ListViewItem lvi = new ListViewItem();
            lvi.Text = "" + fm.index;
            lvi.SubItems.Add("" + fm.ox);
            lvi.SubItems.Add("" + fm.oy);
            lvi.BackColor = colListBack;
            lvi.ForeColor = colListText;
            string flagExpl = fm.flags.ToString();// +" - ";
            //if ((fm.flags & 1) != 0) flagExpl += " flipX";
            //if ((fm.flags & 2) != 0) flagExpl += " flipY";
            //if ((fm.flags & 3) == 0) flagExpl += " NO Flips";
            lvi.SubItems.Add(flagExpl);
            lvSecond.Items.Add(lvi);
            SetModified(true);
        }

        void CloneSelectedFModules()
        {
            if (frame == null) return;
            foreach (ListViewItem lvi in lvSecond.SelectedItems)
            {
                CloneFModule(frame.GetFModuleByIndex(lvi.Index));
            }
            SetModified(true);
        }

        void CloneFModule(FrameModule fmod)
        {
            FrameModule fm = frame.AddFModule(fmod.module, fmod.flags);
            fm.index = lvSecond.Items.Count;
            fmodule = fm;
            fm.ox = fmod.ox;
            fm.oy = fmod.oy;
            fmodule.flags = fm.flags;
            AddFModule(fm);
            GetItemByIndex(lvMain, frame.index).SubItems[1].Text = "" + frame.fmodules.Count; //refresh la nr de fmodule
            SetModified(true);
        }

        //misca toate fmodules selectate din lista
        void MoveSelectedFModules(int dx, int dy)
        {
            foreach (ListViewItem lvi in lvSecond.SelectedItems)
            {
                FrameModule fm = frame.GetFModuleByIndex(lvi.Index);
                if (fm == null) continue;
                fm.ox += dx;
                fm.oy += dy;
                lvi.SubItems[1].Text = "" + fm.ox;
                lvi.SubItems[2].Text = "" + fm.oy;
            }
            SetModified(true);
        }
        //misca main Fmodule
        void MoveFModule(int dx, int dy)
        {
            if (fmodule == null)
                return;
            fmodule.ox += dx;
            fmodule.oy += dy;
            ListViewItem lvi = lvSecond.Items[fmodule.index];
            lvi.SubItems[1].Text = "" + fmodule.ox;
            lvi.SubItems[2].Text = "" + fmodule.oy;
            SetModified(true);
        }

        //selecteaza noul frame module curent si da refresh la draw area
        void SelectFModule(int i)
        {
            if (frame == null)
                return;
            fmodule = frame.GetFModuleByIndex(i);
            pbDraw.Refresh();
        }

        void SwapFModules(int minIdx, int maxIdx, bool loop)
        {
            if (frame == null) return;
            if (minIdx > maxIdx) return;

            if (loop == false)
            {
                FrameModule fmi = frame.GetFModuleByIndex(minIdx);
                FrameModule fmj = frame.GetFModuleByIndex(minIdx + 1);
                int temp = fmi.index;
                fmi.index = fmj.index;
                fmj.index = temp;

                return;
            }

            for (int ii = minIdx + 1; ii <= maxIdx; ii++)
            {
                FrameModule fmi = frame.GetFModuleByIndex(ii);
                FrameModule fmj = frame.GetFModuleByIndex(minIdx);
                int temp = fmi.index;
                fmi.index = fmj.index;
                fmj.index = temp;
            }

            frame.SortFModules(); //e important pt ordinea de desenare
            SetModified(true);
        }

        void DeleteFModule(int i)
        {
            if (frame == null)
                return;
            frame.DeleteFModule(i);
            GetItemByIndex(lvMain, frame.index).SubItems[1].Text = "" + frame.fmodules.Count; //refresh la nr de fmodule
            pbDraw.Refresh();
        }

        #endregion

        #region AnimView

        Animation anim = null;
        AnimFrame aframe = null;
        float scaleA = 1.0f;
        int oxA = 100, oyA = 100;
        //sa nu mai faca updates pe selindexchanged pe delete si altele
        bool supressSelIdxChanged = false;

        bool showAnimationPath = false;
        //cand e true, face play la animatie
        bool AnimPlaying = false;
        //var secundare folosite pt animatie
        int currentAnimPreviewIdx = 0;
        int currentAnimPreviewDuration = 1;

        void InitAnimView()
        {
            lvMain.Items.Clear();
            lvMain.Columns.Clear();
            lvMain.Columns.Add("Index", 40);
            lvMain.Columns.Add("Frames", 50);
            lvMain.Columns.Add("Flags", 80);
            lvMain.Columns.Add("Name", 200);
            splitMS.Panel2Collapsed = false;
            lvSecond.Items.Clear();
            lvSecond.Columns.Clear();
            lvSecond.Columns.Add("Index", 40);
            lvSecond.Columns.Add("Duration", 40);
            lvSecond.Columns.Add("Xmove", 40);
            lvSecond.Columns.Add("Ymove", 40);
            lvSecond.Columns.Add("Flags", 80);
            sprite.MakeModuleImages();
            AddThumbs(sprite.frames.Count);
            scale = scaleA;
            ox = oxA;
            oy = oyA;
            PopulateAnimList();
            pbDraw.Refresh();

            AnimPlaying = false;
            tsb_anmPlay.Checked = AnimPlaying;
        }

        void AddAnim(Animation a, bool selectLast)
        {
            item = new ListViewItem();
            item.Text = "" + a.index;
            item.SubItems.Add("" + a.aframes.Count);
            string loopflag = a.flags.ToString();
            if ((a.flags & 1) != 0) loopflag += " - Looping";
            else loopflag += " - NO Loop";
            item.SubItems.Add(loopflag);
            item.SubItems.Add(a.name);
            item.BackColor = colListBack;
            item.ForeColor = colListText;
            if (selectLast)
            {
                RemoveSelection(lvMain);
                item.Selected = true;
            }
            lvMain.Items.Add(item);
        }

        void DeleteAnim(int idx)
        {
            sprite.DeleteAnim(idx);
            SetModified(true);
        }

        void CloneSelectedAnimations()
        {
            foreach (ListViewItem lvi in lvMain.SelectedItems)
            {
                CloneAnim(lvi.Index);
            }
        }

        void CloneAnim(int index)
        {
            Animation origa = sprite.GetAnimByIndex(index);
            if (origa == null) return;

            Animation anm = sprite.AddAnim();
            anm.name = origa.name;
            anm.index = lvMain.Items.Count;
            foreach (AnimFrame afm in origa.aframes)
            {
                AnimFrame nfm = anm.AddAFrame(afm.frame);
                nfm.ox = afm.ox;
                nfm.oy = afm.oy;
                nfm.index = afm.index;
                nfm.strFlags = afm.strFlags;
            }
            anim = anm;
            AddAnim(anm, false);
        }

        void NewAnim()
        {
            anim = sprite.AddAnim();
            anim.index = lvMain.Items.Count;
            AddAnim(anim, true);
        }

        void AddAFrame(AnimFrame af)
        {
            ListViewItem lvi = new ListViewItem();
            lvi.Text = "" + af.index;
            lvi.SubItems.Add("" + af.time);
            lvi.SubItems.Add("" + af.ox);
            lvi.SubItems.Add("" + af.oy);
            lvi.BackColor = colListBack;
            lvi.ForeColor = colListText;
            lvi.SubItems.Add(af.strFlags);
            lvSecond.Items.Add(lvi);
        }

        void NewAFrame(int i)
        {
            frame = sprite.GetFrameByIndex(i);//.frames[i];
            aframe = anim.AddAFrame(frame);
            aframe.index = lvSecond.Items.Count;
            AddAFrame(aframe);
            GetItemByIndex(lvMain, anim.index).SubItems[1].Text = "" + anim.aframes.Count; //refresh la nr de aframes
        }

        void GoToSelectedAFrame_Frame()
        {
            if (aframe != null)
            {
                SaveViewData();
                view = Views.frameView;
                InitFrameView();
                ts_Modules.Checked = false;
                ts_Frames.Checked = true;
                ts_Animations.Checked = false;

                tool = Tools.Select;
                pbDraw.Cursor = Cursors.Default;

                ToolStripAnims.Hide();
                ToolStripFrames.Show();
                ToolStripModules.Hide();

                SelectFrame(aframe.frame.index);
                lvMain.Items[aframe.frame.index].Selected = true;
                lvMain.Items[aframe.frame.index].Focused = true;

                pbDraw.Refresh();
            }
        }

        void GoToSelectedModule()
        {
            if (frame == null) return;

            SaveViewData();
            view = Views.moduleView;
            InitModuleView();

            ts_Modules.Checked = true;
            ts_Frames.Checked = false;
            ts_Animations.Checked = false;

            tool = Tools.Select;
            pbDraw.Cursor = Cursors.Default;

            ToolStripAnims.Hide();
            ToolStripFrames.Hide();
            ToolStripModules.Show();

            Module mod = frame.fmodules[lvSecond.SelectedIndices[0]].module;
            int modi = mod.index;

            ox = -mod.x + w / 2 - mod.w / 2;
            oy = -mod.y + h / 2 - mod.h / 2;

            SelectModule(modi);
            lvMain.Items[modi].Selected = true;
            lvMain.Items[modi].Focused = true;

            pbDraw.Refresh();
        }


        void CloneSelectedAFrames()
        {
            foreach (ListViewItem lvi in lvSecond.SelectedItems)
            {
                CloneAFrame(lvi.Index);
            }
        }

        void CloneAFrame(int index)
        {
            if (anim == null) return;
            AnimFrame af = anim.GetAFrameByIndex(index);

            AnimFrame nf = anim.AddAFrame(af.frame);
            nf.index = lvSecond.Items.Count;
            nf.time = af.time;
            nf.ox = af.ox;
            nf.oy = af.oy;
            aframe = nf;
            AddAFrame(nf);
        }

        void MoveAFrame(AnimFrame aframe, int dx, int dy)
        {
            if (aframe == null)
                return;
            aframe.ox += dx;
            aframe.oy += dy;
            // update ox and oy coords in list
            lvSecond.Items[aframe.index].SubItems[2].Text = aframe.ox.ToString();
            lvSecond.Items[aframe.index].SubItems[3].Text = aframe.oy.ToString();
        }

        void MoveSelectedAFrames(int dx, int dy)
        {
            //none selected? move all
            if (lvSecond.SelectedItems.Count <= 0)
            {
                foreach (AnimFrame afrm in anim.aframes)
                {
                    MoveAFrame(afrm, dx, dy);
                }
            }
            else
            {
                foreach (ListViewItem lvi in lvSecond.SelectedItems)
                {
                    AnimFrame afrm = anim.GetAFrameByIndex(lvi.Index);
                    if (afrm == null) return;

                    MoveAFrame(afrm, dx, dy);
                }
            }
        }

        void PopulateAnimList(int selectidx)
        {
            lvMain.BeginUpdate();
            lvMain.Items.Clear();
            foreach (Animation a in sprite.anims)
                AddAnim(a, false);
            SelectAnim(selectidx);
            lvMain.EndUpdate();
        }

        void PopulateAnimList()
        {
            lvMain.BeginUpdate();
            lvMain.Items.Clear();
            foreach (Animation a in sprite.anims)
                AddAnim(a, false);
            SelectAnim(0);
            lvMain.EndUpdate();
            if (sprite.anims.Count > 0)
                lvMain.AutoResizeColumns(ColumnHeaderAutoResizeStyle.ColumnContent);
        }

        void SelectAnim(int i)
        {
            anim = sprite.GetAnimByIndex(i);
            SelectAFrame(0);
            PopulateAFramesList();
        }

        void SwapAnims(int minIdx, int maxIdx, bool loop)
        {
            if (minIdx > maxIdx) return;

            if (loop == false)
            {
                Animation ai = sprite.GetAnimByIndex(minIdx);
                Animation aj = sprite.GetAnimByIndex(minIdx + 1);
                int temp = ai.index;
                ai.index = aj.index;
                aj.index = temp;

                return;
            }

            for (int ii = minIdx + 1; ii <= maxIdx; ii++)
            {
                Animation ai = sprite.GetAnimByIndex(ii);
                Animation aj = sprite.GetAnimByIndex(minIdx);
                int temp = ai.index;
                ai.index = aj.index;
                aj.index = temp;
            }
        }

        void SelectAFrame(int i)
        {
            if (anim == null)
                return;
            aframe = anim.GetAFrameByIndex(i);
            pbDraw.Refresh();
        }

        void PopulateAFramesList()
        {
            if (anim == null)
                return;
            lvSecond.BeginUpdate();
            lvSecond.Items.Clear();
            foreach (AnimFrame af in anim.aframes)
                AddAFrame(af);
            lvSecond.EndUpdate();
        }

        void SwapAFrames(int minIdx, int maxIdx, bool loop)
        {
            if (anim == null) return;
            if (minIdx > maxIdx) return;

            if (loop == false)
            {
                AnimFrame afi = anim.GetAFrameByIndex(minIdx);
                AnimFrame afj = anim.GetAFrameByIndex(minIdx + 1);
                int temp = afi.index;
                afi.index = afj.index;
                afj.index = temp;

                return;
            }

            for (int ii = minIdx + 1; ii <= maxIdx; ii++)
            {
                AnimFrame afi = anim.GetAFrameByIndex(ii);
                AnimFrame afj = anim.GetAFrameByIndex(minIdx);
                int temp = afi.index;
                afi.index = afj.index;
                afj.index = temp;
            }
        }

        private void DeleteAFrame(int idx)
        {
            if (anim == null)
                return;
            anim.DeleteAFrame(idx);
            GetItemByIndex(lvMain, anim.index).SubItems[1].Text = "" + anim.aframes.Count; //refresh la nr de aframes
            pbDraw.Refresh();
        }

        // timer ticks every TIMER_DURATION
        private void timer1_Tick(object sender, EventArgs e)
        {
            if ((anim == null) || (aframe == null) || (!lvMain.Focused))
                return;

            currentAnimPreviewDuration -= TIMER_UPDATE_PERIOD;
            if (currentAnimPreviewDuration <= 0)
            {
                currentAnimPreviewIdx++;
                if (currentAnimPreviewIdx >= anim.aframes.Count)
                {
                    currentAnimPreviewIdx = 0;
                }
                aframe = anim.GetAFrameByIndex(currentAnimPreviewIdx);
                currentAnimPreviewDuration += aframe.time;
            }
            pbDraw.Refresh();
        }

        #endregion

        #region ListView

        ListViewItem item = null; //item-ul curent din lista (folosit in primul rand pt swap)

        //converteste valoarea din prima coloana (indexul) intr-un int
        int GetIndex(ListViewItem lvi)
        {
            return Convert.ToInt32(lvi.Text);
        }

        //returneaza un element dintr-un list view dupa index
        ListViewItem GetItemByIndex(ListView lv, int index)
        {
            foreach (ListViewItem lvi in lv.Items)
                if (GetIndex(lvi) == index)
                    return lvi;
            return null;
        }
        //fn generica pt deselectare
        void RemoveSelection(ListView lv)
        {
            foreach (ListViewItem lvi in lv.Items)
            {
                lvi.Selected = false;
                lvi.Focused = false;
            }
        }

        //procedura generica de selectare a unui intr-un list view
        void SelectItem(ListView lv, int index)
        {
            foreach (ListViewItem lvi in lv.Items)
            {
                lvi.Selected = false;
                lvi.Focused = false;
            }
            GetItemByIndex(lv, index).Selected = true;
            lv.Focus();
        }

        //evenimente mouse generice
        //mouseDown - seteaza element curent
        private void lvMain_MouseDown(object sender, MouseEventArgs e)
        {
            if (e.Button == MouseButtons.Left)
            {
                ListView lv = sender as ListView;
                item = lv.GetItemAt(e.X, e.Y);
                dragList = true;
            }
            else if (e.Button == MouseButtons.Right)
            {
                cmLists.Items.Clear();

                if (sender == lvMain)
                {
                    switch (view)
                    {
                        case Views.moduleView:
                            {
                                cmLists.Items.Add("Insert");
                                cmLists.Items[cmLists.Items.Count - 1].Name = "InsertModule";
                                cmLists.Items[cmLists.Items.Count - 1].Click += new EventHandler(cmLists_Click);
                                cmLists.Items.Add("Clone");
                                cmLists.Items[cmLists.Items.Count - 1].Name = "CloneModule";
                                cmLists.Items[cmLists.Items.Count - 1].Click += new EventHandler(cmLists_Click);
                                cmLists.Items.Add("-");
                                cmLists.Items.Add("Delete");
                                cmLists.Items[cmLists.Items.Count - 1].Name = "DeleteModule";
                                cmLists.Items[cmLists.Items.Count - 1].Click += new EventHandler(cmLists_Click);
                            }
                            break;
                        case Views.frameView:
                            {
                                cmLists.Items.Add("Insert");
                                cmLists.Items[cmLists.Items.Count - 1].Name = "InsertFrame";
                                cmLists.Items[cmLists.Items.Count - 1].Click += new EventHandler(cmLists_Click);
                                cmLists.Items.Add("Clone");
                                cmLists.Items[cmLists.Items.Count - 1].Name = "CloneFrame";
                                cmLists.Items[cmLists.Items.Count - 1].Click += new EventHandler(cmLists_Click);
                                cmLists.Items.Add("-");
                                cmLists.Items.Add("Flip X (flag)");
                                cmLists.Items[cmLists.Items.Count - 1].Name = "FlipFrameX";
                                cmLists.Items[cmLists.Items.Count - 1].Click += new EventHandler(cmLists_Click);
                                cmLists.Items.Add("Flip Y (flag)");
                                cmLists.Items[cmLists.Items.Count - 1].Name = "FlipFrameY";
                                cmLists.Items[cmLists.Items.Count - 1].Click += new EventHandler(cmLists_Click);
                                cmLists.Items.Add("-");
                                cmLists.Items.Add("Find Bounding Box");
                                cmLists.Items[cmLists.Items.Count - 1].Name = "FrameComputeBBox";
                                cmLists.Items[cmLists.Items.Count - 1].Click += new EventHandler(cmLists_Click);
                                cmLists.Items.Add("-");
                                cmLists.Items.Add("Delete");
                                cmLists.Items[cmLists.Items.Count - 1].Name = "DeleteFrame";
                                cmLists.Items[cmLists.Items.Count - 1].Click += new EventHandler(cmLists_Click);
                            }
                            break;
                        case Views.animView:
                            {
                                cmLists.Items.Add("Toggle Loop Flag");
                                cmLists.Items[cmLists.Items.Count - 1].Name = "ToggleLoopAnim";
                                cmLists.Items[cmLists.Items.Count - 1].Click += new EventHandler(cmLists_Click);
                                cmLists.Items.Add("Clear Loop Flag");
                                cmLists.Items[cmLists.Items.Count - 1].Name = "ClearLoopAnim";
                                cmLists.Items[cmLists.Items.Count - 1].Click += new EventHandler(cmLists_Click);
                                cmLists.Items.Add("Clone");
                                cmLists.Items[cmLists.Items.Count - 1].Name = "CloneAnimation";
                                cmLists.Items[cmLists.Items.Count - 1].Click += new EventHandler(cmLists_Click);
                                cmLists.Items.Add("-");
                                cmLists.Items.Add("Delete");
                                cmLists.Items[cmLists.Items.Count - 1].Name = "DeleteAnimation";
                                cmLists.Items[cmLists.Items.Count - 1].Click += new EventHandler(cmLists_Click);
                            }
                            break;
                    }
                }
                else if (sender == lvSecond)
                {
                    switch (view)
                    {
                        case Views.frameView:
                            {

                                cmLists.Items.Add("GoToModule");
                                cmLists.Items[cmLists.Items.Count - 1].Name = "GoToModule";
                                cmLists.Items[cmLists.Items.Count - 1].Click += new EventHandler(cmLists_Click);
                                cmLists.Items.Add("Flip X (flag)");
                                cmLists.Items[cmLists.Items.Count - 1].Name = "FlipFModuleX";
                                cmLists.Items[cmLists.Items.Count - 1].Click += new EventHandler(cmLists_Click);
                                cmLists.Items.Add("Flip Y (flag)");
                                cmLists.Items[cmLists.Items.Count - 1].Name = "FlipFModuleY";
                                cmLists.Items[cmLists.Items.Count - 1].Click += new EventHandler(cmLists_Click);
                                cmLists.Items.Add("Clear Flags");
                                cmLists.Items[cmLists.Items.Count - 1].Name = "ClearFModuleFlags";
                                cmLists.Items[cmLists.Items.Count - 1].Click += new EventHandler(cmLists_Click);
                                cmLists.Items.Add("-");
                                cmLists.Items.Add("Clone");
                                cmLists.Items[cmLists.Items.Count - 1].Name = "CloneFModules";
                                cmLists.Items[cmLists.Items.Count - 1].Click += new EventHandler(cmLists_Click);
                                cmLists.Items.Add("-");
                                cmLists.Items.Add("Delete");
                                cmLists.Items[cmLists.Items.Count - 1].Name = "DeleteFModule";
                                cmLists.Items[cmLists.Items.Count - 1].Click += new EventHandler(cmLists_Click);
                            }
                            break;
                        case Views.animView:
                            {
                                cmLists.Items.Add("Clone");
                                cmLists.Items[cmLists.Items.Count - 1].Name = "CloneAFrame";
                                cmLists.Items[cmLists.Items.Count - 1].Click += new EventHandler(cmLists_Click);
                                cmLists.Items.Add("-");
                                cmLists.Items.Add("GoToFrame");
                                cmLists.Items[cmLists.Items.Count - 1].Name = "GoToFrame";
                                cmLists.Items[cmLists.Items.Count - 1].Click += new EventHandler(cmLists_Click);
                                cmLists.Items.Add("-");
                                cmLists.Items.Add("Delete");
                                cmLists.Items[cmLists.Items.Count - 1].Name = "DeleteAFrame";
                                cmLists.Items[cmLists.Items.Count - 1].Click += new EventHandler(cmLists_Click);
                            }
                            break;
                    }
                }
                cmLists.Show(sender as ListViewEx, e.Location);
            }
        }

        //mousUp - nimic interesant; marcheaza sfarsitul drag-ului
        private void lvMain_MouseUp(object sender, MouseEventArgs e)
        {
            if (e.Button == MouseButtons.Left)
            {
                ListView lv = sender as ListView;
                lv.Cursor = Cursors.Default;
                dragList = false;
            }
        }

        //mouseMove - mutarea unui element (ca in winamp)
        //se face prin interschimbarea indexului si apoi resortare, in rest lista de Items ramane neschimbata
        private void lvMain_MouseMove(object sender, MouseEventArgs e)
        {
            if ((dragList == true) && (e.Button == MouseButtons.Left))
            {
                ListView lv = sender as ListView;
                lv.Cursor = Cursors.Hand;
                ListViewItem target = lv.GetItemAt(e.X, e.Y);
                if (target == null || target == item)
                    return;

                int i = lv.Items.IndexOf(item);
                int j = lv.Items.IndexOf(target);

                lv.BeginUpdate();

                int minIdx = Math.Min(i, j);
                int maxIdx = Math.Max(i, j);

                if (i < j)
                {
                    string temp = lv.Items[i].Text;
                    lv.Items[i].Text = lv.Items[i + 1].Text;
                    lv.Items[i + 1].Text = temp;
                }
                else
                {
                    for (int ii = minIdx; ii < maxIdx; ii++)
                    {
                        string temp = lv.Items[ii].Text;
                        lv.Items[ii].Text = lv.Items[ii + 1].Text;
                        lv.Items[ii + 1].Text = temp;
                    }
                }

                lv.Sort();

                bool loop = (j < i);

                if (view == Views.moduleView)
                    SwapModules(minIdx, maxIdx, loop);
                else if (view == Views.frameView)
                {
                    if (lv == lvMain)
                        SwapFrames(minIdx, maxIdx, loop);
                    else if (lv == lvSecond)
                        SwapFModules(minIdx, maxIdx, loop);
                }
                else if (view == Views.animView)
                {
                    if (lv == lvMain)
                        SwapAnims(minIdx, maxIdx, loop);
                    else if (lv == lvSecond)
                        SwapAFrames(minIdx, maxIdx, loop);
                }
                lv.EndUpdate();
            }
        }

        void DeleteItem(ListView lv)
        {
            if (lv.SelectedIndices.Count == 0)
                return;

            lv.SuspendLayout();
            supressSelIdxChanged = true;

            List<int> idxlist = new List<int>(lv.SelectedItems.Count);
            foreach (ListViewItem sel in lv.SelectedItems)
            {
                int idx = GetIndex(sel);
                idxlist.Add(idx);
            }
            for (int k = 0; k < idxlist.Count; k++)
            {
                int idx = idxlist[k];

                for (int l = k + 1; l < idxlist.Count; l++)
                {
                    if (idxlist[l] > idxlist[k])
                        idxlist[l]--;
                }

                if (lv == lvMain)
                {
                    if (view == Views.moduleView)
                    {
                        DeleteModule(idx);
                    }
                    else if (view == Views.frameView)
                    {
                        DeleteFrame(idx);
                    }
                    else if (view == Views.animView)
                    {
                        DeleteAnim(idx);
                    }
                }
                else if (lv == lvSecond)
                {
                    if (view == Views.frameView)
                        DeleteFModule(idx);
                    else if (view == Views.animView)
                        DeleteAFrame(idx);
                }
            }

            idxlist = null;

            //repopuleaza listele
            if (lv == lvMain)
            {
                if (view == Views.moduleView)
                {
                    populateModuleList();
                }
                else if (view == Views.frameView)
                {
                    PopulateFrameList();
                }
                else if (view == Views.animView)
                {
                    PopulateAnimList();
                }
            }
            else if (lv == lvSecond)
            {
                if (view == Views.frameView)
                {
                    PopulateFModulesList();
                }
                else if (view == Views.animView)
                {
                    PopulateAFramesList();
                }
            }

            supressSelIdxChanged = false;

            lv.ResumeLayout(true);
            pbDraw.Refresh();
        }

        #endregion

        #region MainListView

        public class IndexComparer : System.Collections.IComparer
        {
            public int Compare(object a, object b)
            {
                int i = Convert.ToInt32((a as ListViewItem).Text);
                int j = Convert.ToInt32((b as ListViewItem).Text);
                if (i > j)
                    return 1;
                if (i < j)
                    return -1;
                return 0;
            }
        }

        bool dragList = false;
        private void lvMain_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (supressSelIdxChanged) return;

            int idx = -1;
            if (lvMain.SelectedIndices.Count != 0)
                idx = GetIndex(lvMain.SelectedItems[0]);

            //face cu bold pe cel activ
            for (int h = 0; h < lvMain.Items.Count; h++)
            {
                lvMain.Items[h].BackColor = colListBack;
            }
            if (idx >= 0)
            {
                lvMain.Items[idx].BackColor = Color.LightGray;
            }

            switch (view)
            {
                case Views.moduleView:
                    SelectModule(idx);
                    break;
                case Views.frameView:
                    SelectFrame(idx);
                    break;
                case Views.animView:
                    SelectAnim(idx);
                    break;
            }
        }


        private void lvMain_KeyUp(object sender, KeyEventArgs e)
        {
            if (e.KeyCode == Keys.Delete && lvMain.SelectedIndices.Count != 0)
            {
                DeleteModule(lvMain.SelectedIndices[0]);
            }
        }

        private void lvMain_CellEdited(object sender, EventArgs e)
        {
            if ((view == Views.animView) && (anim != null))
            {
                switch (lvMain.EditColumn)
                {
                    case 1: //frames - not editable
                        {
                            ListViewItem lvi = GetItemByIndex(lvMain, anim.index);
                            lvi.SubItems[1].Text = "" + anim.aframes.Count;
                        }
                        break;
                    case 2: //flags
                        {
                            anim.flags = Convert.ToInt32(lvMain.EditText);
                        }
                        break;
                    case 3: //anim name
                        {
                            //#TODO: check if name is unique
                            anim.name = lvMain.EditText;
                        }
                        break;
                }
            }
            else
            if (view == Views.moduleView && module != null)
            {
                switch (lvMain.EditColumn)
                {
                    case 1: //image id
                        int newimgid = -1;
                        if (SafeConvertToInt32(lvMain.EditText, ref newimgid))
                        {
                            if ((newimgid < 0) || (newimgid >= sprite.images.Count))
                            {
                                InitModuleView();
                            }
                            else
                            {
                                module.imageID = newimgid;
                                if (module.imageID != imgIdx)
                                {
                                    imgIdx = module.imageID;
                                    img = sprite.images[imgIdx];
                                }
                            }
                        }
                        else
                        {
                            InitModuleView();
                        }
                        break;
                    case 2: //x
                        {
                            int newval = -1;
                            if (SafeConvertToInt32(lvMain.EditText, ref newval))
                            {
                                module.x = newval;
                            }
                            else
                            {
                                InitModuleView();
                            }
                        }
                        break;
                    case 3: //y
                        {
                            int newval = -1;
                            if (SafeConvertToInt32(lvMain.EditText, ref newval))
                            {
                                module.y = newval;
                            }
                            else
                            {
                                InitModuleView();
                            }
                        }
                        break;
                    case 4: //w
                        {
                            int newval = -1;
                            if (SafeConvertToInt32(lvMain.EditText, ref newval))
                            {
                                module.w = newval;
                            }
                            else
                            {
                                InitModuleView();
                            }
                        }
                        break;
                    case 5: //h
                        {
                            int newval = -1;
                            if (SafeConvertToInt32(lvMain.EditText, ref newval))
                            {
                                module.h = newval;
                            }
                            else
                            {
                                InitModuleView();
                            }
                        }
                        break;
                }
                pbDraw.Refresh();
            }
        }

        #endregion

        #region SecondListView

        private void clearToolStripMenuItem1_Click(object sender, EventArgs e)
        {
            lvSecond.Items.Clear();
            if (view == Views.frameView)
                if (frame != null)
                    frame.fmodules.Clear();
            pbDraw.Refresh();
        }

        //as putea sa o unesc cu cea de la lvMain
        private void lvSecond_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (lvSecond.SelectedIndices.Count == 0)
                return;
            //colorare selectie activa
            for (int h = 0; h < lvSecond.Items.Count; h++)
            {
                lvSecond.Items[h].BackColor = colListBack;
            }
            if (lvSecond.SelectedItems.Count > 0)
            {
                lvSecond.Items[lvSecond.SelectedIndices[0]].BackColor = Color.LightGray;
            }

            if (view == Views.frameView && frame != null)
            {
                SelectFModule(lvSecond.SelectedIndices[0]);
                tool = Tools.Select;

                tsb_frmSelect.Checked = true;
                tsb_frmBBox.Checked = false;
                tsb_frmPoints.Checked = false;
                //pbDraw.Refresh();
            }
            else if (view == Views.animView && anim != null)
            {
                SelectAFrame(lvSecond.SelectedIndices[0]);
                //stop animation
                AnimPlaying = false;
                tsb_anmPlay.Checked = AnimPlaying;
                timer1.Enabled = AnimPlaying;
                //pbDraw.Refresh();
            }
        }


        private void lvSecond_CellEdited(object sender, EventArgs e)
        {
            if (view == Views.frameView && frame != null)
            {
                fmodule = frame.GetFModuleByIndex(GetIndex(lvSecond.EditItem));
                switch (lvSecond.EditColumn)
                {
                    case 1: //OX
                        fmodule.ox = Convert.ToInt32(lvSecond.EditText);
                        break;
                    case 2: //OY
                        fmodule.oy = Convert.ToInt32(lvSecond.EditText);
                        break;
                }
                pbDraw.Refresh();
            }
            else if ((view == Views.animView) && (aframe != null))
            {
                aframe = anim.GetAFrameByIndex(GetIndex(lvSecond.EditItem));
                switch (lvSecond.EditColumn)
                {
                    case 1: //time
                        {
                            int newval = -1;
                            if (SafeConvertToInt32(lvSecond.EditText, ref newval))
                            {
                                if (newval > 0)
                                {
                                    aframe.time = newval;
                                }
                                else
                                {
                                    PopulateAFramesList();
                                    MessageBox.Show("Only values grater than 0 allowed !", "WARNING !", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
                                }
                            }
                            else
                            {
                                PopulateAFramesList();
                            }
                        }
                        break;
                    case 2: //xmove
                        {
                            int newval = -1;
                            if (SafeConvertToInt32(lvSecond.EditText, ref newval))
                            {
                                aframe.ox = newval;
                            }
                            else
                            {
                                PopulateAFramesList();
                            }
                        }
                        break;
                    case 3: //ymove
                        {
                            int newval = -1;
                            if (SafeConvertToInt32(lvSecond.EditText, ref newval))
                            {
                                aframe.oy = newval;
                            }
                            else
                            {
                                PopulateAFramesList();
                            }
                        }
                        break;
                    case 4: //flags
                        {
                            aframe.strFlags = lvSecond.EditText;
                        }
                        break;
                }
            }
        }

        #endregion

        #region DrawArea

        Image img = null; //imaginea curenta
        int imgIdx = 0;
        int ox = 100, oy = 100; //originea sist de coord
        int w, h; //latimea si inaltimea zonei de desen
        float scale = 1.0f;
        Rectangle N, S, E, W, NE, NW, SE, SW;

        void DrawResize(Graphics g, int rx, int ry, int rw, int rh)
        {
            Pen pn = new Pen(Brushes.DarkRed);
            NE = new Rectangle(rx, ry, 6, 6);
            g.DrawRectangle(pn, NE);
            NW = new Rectangle(rx + rw - 6, ry, 6, 6);
            g.DrawRectangle(pn, NW);
            SW = new Rectangle(rx + rw - 6, ry + rh - 6, 6, 6);
            g.DrawRectangle(pn, SW);
            SE = new Rectangle(rx, ry + rh - 6, 6, 6);
            g.DrawRectangle(pn, SE);
            N = new Rectangle(rx + rw / 2 - 3, ry, 6, 6);
            g.DrawRectangle(pn, N);
            S = new Rectangle(rx + rw / 2 - 3, ry + rh - 6, 6, 6);
            g.DrawRectangle(pn, S);
            E = new Rectangle(rx, ry + rh / 2 - 3, 6, 6);
            g.DrawRectangle(pn, E);
            W = new Rectangle(rx + rw - 6, ry + rh / 2 - 3, 6, 6);
            g.DrawRectangle(pn, W);
        }

        void DrawFrame(Graphics g, int rx, int ry, int rw, int rh)
        {
            Pen pen = new Pen(Color.Red);
            pen.DashStyle = System.Drawing.Drawing2D.DashStyle.Dash;
            Rectangle rect = new Rectangle(rx, ry, rw, rh);
            rect = NormalizeRectangle(rect);
            g.DrawRectangle(pen, rect);
        }

        int TX(int x)
        {
            return (int)(x * scale + ox);
        }

        int TY(int y)
        {
            return (int)(y * scale + oy);
        }

        int TD(int d)
        {
            return (int)(d * scale);
        }

        void PaintDrawArea(Graphics g)
        {
            if (g == null)
                return;
            g.SmoothingMode = System.Drawing.Drawing2D.SmoothingMode.None;
            g.CompositingQuality = System.Drawing.Drawing2D.CompositingQuality.HighSpeed;
            g.InterpolationMode = System.Drawing.Drawing2D.InterpolationMode.NearestNeighbor;
            //clear color
            g.Clear(colClear);
            //paint grid
            int cntx = (int)(w) / TD(gridSize);
            int cnty = (int)(h) / TD(gridSize);
            float stx = (int)ox % TD(gridSize);
            int parityx = ((int)ox) / TD(gridSize);
            float sty = (int)oy % TD(gridSize);
            int parityy = ((int)oy) / TD(gridSize);

            Pen pen_gr1 = new Pen(colGridM);
            Pen pen_gr2 = new Pen(colGridS);
            Pen pen_axis = new Pen(colOrigin);
            //paint grid - last
            if (showGrid)
            {
                for (int i = 0; i <= cntx; i++)
                {
                    float x = stx + i * gridSize * scale;
                    if ((i - parityx) % majorGridLines == 0)
                        g.DrawLine(pen_gr1, x, 0, x, h);
                    else
                    {
                        if (scale > 0.25) g.DrawLine(pen_gr2, x, 0, x, h);
                    }
                }
                for (int i = 0; i <= cnty; i++)
                {
                    float y = sty + i * gridSize * scale;
                    if ((i - parityy) % majorGridLines == 0)
                        g.DrawLine(pen_gr1, 0, y, w, y);
                    else
                    {
                        if (scale > 0.25) g.DrawLine(pen_gr2, 0, y, w, y);
                    }
                }
            }
            //draw axis
            g.DrawLine(pen_axis, 0, oy, w, oy);
            g.DrawLine(pen_axis, ox, 0, ox, h);

            g.TranslateTransform(ox, oy);
            g.ScaleTransform(scale, scale);

            switch (view)
            {
                case Views.moduleView:
                    if (img != null)
                    {
                        g.DrawImage(img, -0.5f, -0.5f, new Rectangle(-1, -1, img.Width + 2, img.Height + 2), GraphicsUnit.Pixel);
                    }
                    break;
                case Views.frameView:
                    {
                        if (frame != null)
                            frame.Paint(g);
                    }
                    break;
                case Views.animView:
                    //la xRay deseneaza toate frames
                    if ((anim != null) && (showAnimXRay))
                    {
                        for (int kk = 0; kk < anim.aframes.Count; kk++)
                        {
                            AnimFrame afrm = anim.aframes[kk];
                            if (afrm == aframe)
                                continue;

                            afrm.frame.PaintWithAlpha(g, afrm.ox, afrm.oy, 0.3f);
                        }
                    }
                    //acum deseneaza frame-ul curent
                    if (aframe != null)
                    {
                        aframe.frame.Paint(g, aframe.ox, aframe.oy);
                    }
                    break;
            }
            g.ResetTransform();

            // paint bbox on anim view
            if (view == Views.animView)
            {
                if (aframe != null)
                {
                    // paint bbox on selected frame
                    Pen pen = new Pen(Color.DarkOrange);
                    pen.DashStyle = System.Drawing.Drawing2D.DashStyle.DashDotDot;
                    g.DrawRectangle(pen, TX(aframe.frame.BBox.X + aframe.ox), TY(aframe.frame.BBox.Y + aframe.oy), TD(aframe.frame.BBox.Width), TD(aframe.frame.BBox.Height));
                }
            }
            //mouse cursor highlight
            if (cursorHighlight)
            {
                Pen curpen = new Pen(colOrigin);
                curpen.DashStyle = System.Drawing.Drawing2D.DashStyle.Dot;

                g.DrawLine(curpen, 0, TY(IY(cursorPbDraw.Y)), pbDraw.Width, TY(IY(cursorPbDraw.Y)));
                g.DrawLine(curpen, TX(IX(cursorPbDraw.X)), 0, TX(IX(cursorPbDraw.X)), pbDraw.Height);
            }

            switch (view)
            {
                case Views.moduleView:
                    //deseneaza contur imaginii
                    if (img != null)
                    {
                        Pen bluepen = new Pen(Color.BlueViolet);
                        bluepen.DashStyle = System.Drawing.Drawing2D.DashStyle.Dot;
                        g.DrawRectangle(bluepen, TX(0), TY(0), TD(img.Width), TD(img.Height));
                    }
                    //deseneaza toate modulele
                    Pen pen = new Pen(Color.Green);
                    pen.DashStyle = System.Drawing.Drawing2D.DashStyle.Dash;
                    Rectangle rect1;
                    foreach (Module mod in sprite.modules)
                    {
                        //filtreaza sa se vada doar modulele dupa imaginea curenta
                        if ((mod.imageID != imgIdx) || (mod.index >= lvMain.Items.Count)) continue;

                        if (!sprite.IsModuleCorrect(mod))
                        {
                            rect1 = new Rectangle(TX(mod.x), TY(mod.y), TD(mod.w), TD(mod.h));
                            rect1 = NormalizeRectangle(rect1);
                            Brush br = new SolidBrush(Color.FromArgb(100, 250, 0, 0));
                            g.FillRectangle(br, rect1);
                            //daca e selectat il deseneaza rosu
                            if (lvMain.Items[mod.index].Selected)
                            {
                                DrawFrame(g, rect1.X, rect1.Y, rect1.Width, rect1.Height);
                            }
                            else
                            {
                                g.DrawRectangle(pen, rect1);
                            }
                        }
                        else
                        {
                            if (lvMain.Items[mod.index].Selected)
                            {
                                DrawFrame(g, TX(mod.x), TY(mod.y), TD(mod.w), TD(mod.h));
                            }
                            else
                            {
                                g.DrawRectangle(pen, TX(mod.x), TY(mod.y), TD(mod.w), TD(mod.h));
                            }
                        }

                    }

                    if (tool == Tools.Module) //deseneaza selectia de modul
                    {
                        //deseneaza contur imaginii
                        if (img != null)
                        {
                            Pen bluepen = new Pen(Color.BlueViolet);
                            bluepen.DashStyle = System.Drawing.Drawing2D.DashStyle.Dot;
                            g.DrawRectangle(bluepen, TX(0), TY(0), TD(img.Width), TD(img.Height));
                        }
                        //ca sa poata desena, daca sunt coord negative, le reasheaza
                        Rectangle final = NormalizeRectangle(rect);

                        pen = new Pen(Color.LightGreen);
                        pen.DashStyle = System.Drawing.Drawing2D.DashStyle.Dash;
                        g.DrawRectangle(pen, TX(final.X), TY(final.Y), TD(final.Width), TD(final.Height));
                        pen = new Pen(Color.Red);
                        pen.DashStyle = System.Drawing.Drawing2D.DashStyle.Dot;
                        g.DrawRectangle(pen, -1 + ox + final.X * scale, -1 + oy + final.Y * scale, final.Width * scale + 2, final.Height * scale + 2);
                    }
                    else if (tool == Tools.Select)
                    {
                        if ((pbStartMX != pbEndMX) && (pbStartMY != pbEndMY))
                        {
                            //deseneaza dreptunghiul de selectie intre pct de start si mouse
                            Rectangle nrct = new Rectangle(pbStartMX, pbStartMY, pbEndMX - pbStartMX, pbEndMY - pbStartMY);
                            RectangleF final = NormalizeRectangle(nrct);

                            pen = new Pen(Color.Crimson);
                            pen.DashStyle = System.Drawing.Drawing2D.DashStyle.Dash;
                            g.DrawRectangle(pen, ox + final.X * scale, oy + final.Y * scale, final.Width * scale, final.Height * scale);
                        }
                    }

                    //deseneaza modulul selectat, cel principal
                    if ((lvMain.SelectedItems.Count == 1) && (module != null) && (module.imageID == imgIdx))
                    {
                        Module mod = module;
                        int rx = (int)(mod.x * scale + ox);
                        int ry = (int)(mod.y * scale + oy);
                        int rw = (int)(mod.w * scale);
                        int rh = (int)(mod.h * scale);

                        DrawFrame(g, rx, ry, rw, rh);
                        DrawResize(g, rx, ry, rw, rh);
                    }

                    //paint help
                    if (img == null)
                    {
                        g.DrawString("No image loaded or no valid module selected. Click the bottom area to add pictures.", fontArialBold, Brushes.Red, 5, 5);
                    }
                    else
                    {
                        if (tool == Tools.Select)
                        {
                            g.DrawString("CTRL+dir:move module ALT+dir:scale module", fontArialBold, Brushes.Red, 5, 5);
                        }
                        else if (tool == Tools.Module)
                        {
                            g.DrawString("Click and drag to define modules", fontArialBold, Brushes.Red, 5, 5);
                        }
                    }
                    break;
                case Views.frameView:
                    {
                        if (frame == null) break;
                        foreach (ListViewItem lvi in lvSecond.SelectedItems)
                        {
                            FrameModule fm = frame.GetFModuleByIndex(lvi.Index);
                            if (fm != null)
                                DrawFrame(g, TX(fm.ox), TY(fm.oy), TD(fm.module.w), TD(fm.module.h));
                        }

                        //desenare BBox
                        if (lvMain.Focused)
                        {
                            pen = new Pen(Color.DarkOrange);
                            pen.DashStyle = System.Drawing.Drawing2D.DashStyle.DashDotDot;
                            g.DrawRectangle(pen, TX(frame.BBox.X), TY(frame.BBox.Y), TD(frame.BBox.Width), TD(frame.BBox.Height));
                            if (tool == Tools.BBox)
                            {
                                DrawResize(g, TX(frame.BBox.X), TY(frame.BBox.Y), TD(frame.BBox.Width), TD(frame.BBox.Height));
                            }
                        }
                        //desenare hitpts
                        if (tsb_frmPoints.Checked)
                        {

                            for (int kk = 0; kk < frame.hitPoints.Count; kk++)
                            {
                                if (showLevelsOnHitPoints == true)
                                {
                                    g.ScaleTransform(scale, scale);
                                    sprite.frames[4].Paint(g, (int)(TX((int)(frame.hitPoints[kk].X)) / (float)scale), (int)(TY((int)(frame.hitPoints[kk].Y)) / (float)scale));
                                    g.ResetTransform();
                                }

                                Pen pn = Pens.Red;
                                if (kk == selectedHitPt) pn = Pens.LightGoldenrodYellow;

                                g.DrawRectangle(Pens.Black, TX((int)(frame.hitPoints[kk].X)) - 4, TY((int)(frame.hitPoints[kk].Y)) - 4, 8, 8);
                                g.DrawRectangle(pn, TX((int)(frame.hitPoints[kk].X)) - 3, TY((int)(frame.hitPoints[kk].Y)) - 3, 6, 6);
                                g.DrawLine(pn, TX(frame.hitPoints[kk].X) - 3, TY(frame.hitPoints[kk].Y) - 3, TX(frame.hitPoints[kk].X) + 3, TY(frame.hitPoints[kk].Y) + 3);
                                g.DrawLine(pn, TX(frame.hitPoints[kk].X) - 3, TY(frame.hitPoints[kk].Y) + 3, TX(frame.hitPoints[kk].X) + 3, TY(frame.hitPoints[kk].Y) - 3);
                                //le si leaga intre ele
                                if ((showLinkedHitpts) && (frame.hitPoints.Count > 1))
                                {
                                    int nextkk = kk + 1;
                                    if (nextkk >= frame.hitPoints.Count) nextkk = 0;
                                    g.DrawLine(Pens.Red, TX(frame.hitPoints[kk].X) + scale / 2, TY(frame.hitPoints[kk].Y) + scale / 2, TX(frame.hitPoints[nextkk].X) + scale / 2, TY(frame.hitPoints[nextkk].Y) + scale / 2);
                                }
                                //cand faci destul zoom iti arata si valorile hitpts-urilor
                                if (scale >= 8.0f)
                                {
                                    g.DrawString(frame.hitPoints[kk].flags.ToString(), fontArialMic, Brushes.YellowGreen, TX(frame.hitPoints[kk].X) + 1, TY(frame.hitPoints[kk].Y) + 1);
                                }
                            }

                        }
                        //desenare selectie
                        if (tool == Tools.Select)
                        {
                            if ((pbStartMX != pbEndMX) && (pbStartMY != pbEndMY))
                            {
                                //deseneaza dreptunghiul de selectie intre pct de start si mouse
                                Rectangle nrct = new Rectangle(pbStartMX, pbStartMY, pbEndMX - pbStartMX, pbEndMY - pbStartMY);
                                RectangleF final = NormalizeRectangle(nrct);

                                pen = new Pen(Color.Crimson);
                                pen.DashStyle = System.Drawing.Drawing2D.DashStyle.Dash;
                                g.DrawRectangle(pen, ox + final.X * scale, oy + final.Y * scale, final.Width * scale, final.Height * scale);
                            }
                        }
                    }
                    break;
                case Views.animView:
                    {
                        if ((anim != null) && (aframe != null) && (!AnimPlaying))
                        {
                            Rectangle rct = aframe.frame.GetRect();
                            DrawFrame(g, TX(aframe.ox + rct.X), TY(aframe.oy + rct.Y), TD(rct.Width), TD(rct.Height));
                        }
                        //deseneaza traiectoria
                        if ((anim != null) && (showAnimationPath))
                        {
                            Pen pn = new Pen(Brushes.Red);
                            pn.DashStyle = System.Drawing.Drawing2D.DashStyle.Dash;

                            for (int ii = 0; ii < anim.aframes.Count; ii++)
                            {
                                AnimFrame anf = anim.GetAFrameByIndex(ii);
                                g.DrawEllipse(Pens.DarkRed, TX(anf.ox) - 2, TY(anf.oy) - 2, 5, 5);
                                int nextidx = ii + 1;
                                if (nextidx >= anim.aframes.Count) nextidx = 0;
                                if (nextidx != 0)
                                {
                                    AnimFrame nextf = anim.GetAFrameByIndex(nextidx);
                                    g.DrawLine(pn, TX(anf.ox), TY(anf.oy), TX(nextf.ox), TY(nextf.oy));
                                }
                                else if ((anim.flags & 1) != 0)//daca e loopable sa deseneze si linie intre ultimul si primul frame
                                {
                                    AnimFrame nextf = anim.GetAFrameByIndex(nextidx);
                                    pn.Color = Color.LightCoral;
                                    g.DrawLine(pn, TX(anf.ox), TY(anf.oy), TX(nextf.ox), TY(nextf.oy));
                                }
                            }
                        }
                    }
                    break;
            }
        }

        void ResizeDrawArea()
        {
            w = pbDraw.ClientSize.Width;
            h = pbDraw.ClientSize.Height;
            //ox = w / 2;
            //oy = h / 2;
            pbDraw.Refresh();
        }

        private void pbDraw_Resize(object sender, EventArgs e)
        {
            ResizeDrawArea();
        }

        private void pbDraw_Paint(object sender, PaintEventArgs e)
        {
            PaintDrawArea(e.Graphics);
        }

        bool dragScroll = false;
        int pbdX, pbdY;
        int pbStartMX, pbStartMY;
        int pbEndMX, pbEndMY;
        Rectangle rect;
        int Dx, Dy;

        int IX(int x) {
            return (int)((x - ox) / scale);
        }

        int IY(int y) {
            return (int)((y - oy) / scale);
        }

        private void pbDraw_MouseDown(object sender, MouseEventArgs e)
        {
            pbdX = e.X;
            pbdY = e.Y;
            dragScroll = true;
            Dx = Dy = 0;

            if (e.Button == MouseButtons.Right)
            {
                //dragScroll = true;
                lastTool = tool;
            }
            else if (e.Button == MouseButtons.Left)
            {
                switch (view)
                {
                    case Views.moduleView:
                        {
                            if (tool == Tools.Select)
                            {

                                pbStartMX = IX(e.X);
                                pbStartMY = IY(e.Y);
                                pbEndMX = pbStartMX;
                                pbEndMY = pbStartMY;

                                if ((lvMain.SelectedItems.Count == 1) && (module != null) && (module.w != 0) && (module.h != 0) && (module.imageID == imgIdx))
                                {
                                    //daca e in dreptunghiul modulului
                                    if (pointIn(e.X, e.Y, new Rectangle(TX(module.x), TY(module.y), TD(module.w), TD(module.h))))
                                    {
                                        //verifica colturile de resize
                                        if (pointIn(e.X, e.Y, NE))
                                        {
                                            pbDraw.Cursor = Cursors.SizeNWSE;
                                            if (tool != Tools.Relocate && dragScroll)
                                            {
                                                lastTool = tool;
                                                tool = Tools.Relocate;
                                                resizeMode = ResizeMode.XY;
                                                relocMode = ResizeMode.XY;
                                            }
                                        }
                                        else if (pointIn(e.X, e.Y, SW))
                                        {
                                            pbDraw.Cursor = Cursors.SizeNWSE;
                                            if (tool != Tools.Resize && dragScroll)
                                            {
                                                lastTool = tool;
                                                tool = Tools.Resize;
                                                resizeMode = ResizeMode.XY;
                                            }
                                        }
                                        else if (pointIn(e.X, e.Y, NW))
                                        {
                                            pbDraw.Cursor = Cursors.SizeNESW;
                                            if (tool != Tools.Relocate && dragScroll)
                                            {
                                                lastTool = tool;
                                                tool = Tools.Relocate;
                                                resizeMode = ResizeMode.XY;
                                                relocMode = ResizeMode.Y;
                                            }
                                        }
                                        else if (pointIn(e.X, e.Y, SE))
                                        {
                                            pbDraw.Cursor = Cursors.SizeNESW;
                                            if (tool != Tools.Relocate && dragScroll)
                                            {
                                                lastTool = tool;
                                                tool = Tools.Relocate;
                                                resizeMode = ResizeMode.XY;
                                                relocMode = ResizeMode.X;
                                            }
                                        }
                                        else if (pointIn(e.X, e.Y, N))
                                        {
                                            pbDraw.Cursor = Cursors.SizeNS;
                                            if (tool != Tools.Relocate && dragScroll)
                                            {
                                                lastTool = tool;
                                                tool = Tools.Relocate;
                                                resizeMode = ResizeMode.Y;
                                                relocMode = ResizeMode.Y;
                                            }
                                        }
                                        else if (pointIn(e.X, e.Y, S))
                                        {
                                            pbDraw.Cursor = Cursors.SizeNS;
                                            if (tool != Tools.Resize && dragScroll)
                                            {
                                                lastTool = tool;
                                                tool = Tools.Resize;
                                                resizeMode = ResizeMode.Y;
                                            }
                                        }
                                        else if (pointIn(e.X, e.Y, E))
                                        {
                                            pbDraw.Cursor = Cursors.SizeWE;
                                            if (tool != Tools.Relocate && dragScroll)
                                            {
                                                lastTool = tool;
                                                tool = Tools.Relocate;
                                                resizeMode = ResizeMode.X;
                                                relocMode = ResizeMode.X;
                                            }
                                        }
                                        else if (pointIn(e.X, e.Y, W))
                                        {
                                            pbDraw.Cursor = Cursors.SizeWE;
                                            if (tool != Tools.Resize && dragScroll)
                                            {
                                                lastTool = tool;
                                                tool = Tools.Resize;
                                                resizeMode = ResizeMode.X;
                                            }
                                        }
                                        else //daca nu e nici un colt inseamna ca e move
                                        {
                                            pbDraw.Cursor = Cursors.SizeAll;
                                            if (tool != Tools.Move && dragScroll)
                                            {
                                                lastTool = tool;
                                                tool = Tools.Move;
                                            }
                                        }
                                    }
                                    else //inseamna ca e in afara
                                    {
                                        pbDraw.Cursor = Cursors.Default;
                                    }

                                }
                                else if (lvMain.SelectedItems.Count > 1)
                                {
                                    foreach (Module mod in sprite.modules)
                                    {
                                        if (mod.index >= lvMain.Items.Count) continue;
                                        if ((lvMain.Items[mod.index].Selected) && (pointIn(e.X, e.Y, new Rectangle(TX(mod.x), TY(mod.y), TD(mod.w), TD(mod.h)))))
                                        {
                                            pbDraw.Cursor = Cursors.SizeAll;
                                            if (tool != Tools.Move && dragScroll)
                                            {
                                                lastTool = tool;
                                                tool = Tools.Move;
                                            }
                                        }
                                    }
                                }



                            }
                            else if (tool == Tools.Module)
                            {
                                rect = new Rectangle(IX(e.X), IY(e.Y), 0, 0);
                                //dragScroll = true;
                            }
                            else if (tool == Tools.Wand)
                            {
                                int X = IX(e.X);
                                int Y = IY(e.Y);

                                bool inmod = false;
                                for (int kk = 0; kk < sprite.modules.Count; kk++)
                                {
                                    Module mod = sprite.modules[kk];
                                    if ((mod.imageID == imgIdx) && (pointIn(e.X, e.Y, new Rectangle(TX(mod.x), TY(mod.y), TD(mod.w), TD(mod.h)))))
                                    {
                                        inmod = true;
                                        break;
                                    }
                                }

                                if (!inmod)
                                {
                                    Rectangle bb = FindWandBB(img as Bitmap, X, Y, (byte)wandTreshold);

                                    if (bb.Width >= 1 && bb.Height >= 1)
                                        NewModule(imgIdx, bb.X, bb.Y, bb.Width, bb.Height);
                                }
                            }
                        }
                        break;
                    case Views.frameView:
                        {
                            if (tool == Tools.Select)
                            {
                                pbStartMX = IX(e.X);
                                pbStartMY = IY(e.Y);
                                pbEndMX = pbStartMX;
                                pbEndMY = pbStartMY;

                                if (frame != null)
                                {
                                    foreach (ListViewItem lvi in lvSecond.SelectedItems)
                                    {
                                        FrameModule fm = frame.GetFModuleByIndex(lvi.Index);
                                        if (pointIn(IX(e.X), IY(e.Y), new Rectangle(fm.ox, fm.oy, fm.module.w, fm.module.h)))
                                        {
                                            lastTool = tool;
                                            tool = Tools.Move;
                                            pbDraw.Cursor = Cursors.SizeAll;
                                            break;
                                        }
                                    }
                                    //Color col = Color.White;
                                    //if (fmodule.module.image != null)
                                    //    col = (fmodule.module.image as Bitmap).GetPixel(IX(e.X) - fmodule.ox, IY(e.Y) - fmodule.oy);
                                    //if (col.A != 0)
                                }
                            }
                            else if (tool == Tools.HitPoints)
                            {
                                if (frame == null) break;

                                HitPoint hpt = new HitPoint(IX(e.X), IY(e.Y));
                                if (e.X - ox < 0) hpt.X--;
                                if (e.Y - oy < 0) hpt.Y--;

                                //verifica daca exista deja
                                oldhitptIdx = selectedHitPt;
                                selectedHitPt = -1;
                                for (int kk = 0; kk < frame.hitPoints.Count; kk++)
                                {
                                    if ((Math.Abs(frame.hitPoints[kk].X - hpt.X) < 4) && (Math.Abs(frame.hitPoints[kk].Y - hpt.Y) < 4))
                                    {
                                        selectedHitPt = kk;

                                        if (hitptWnd != null)
                                        {
                                            hitptWnd.RefreshValues(frame.hitPoints[selectedHitPt].X, frame.hitPoints[selectedHitPt].Y, frame.hitPoints[selectedHitPt].flags);
                                        }

                                        break;
                                    }
                                }
                            }
                            else if (tool == Tools.BBox)
                            {
                                if (frame == null) break;
                                pbStartMX = IX(e.X);
                                pbStartMY = IY(e.Y);
                                pbEndMX = pbStartMX;
                                pbEndMY = pbStartMY;

                                if ((lvMain.SelectedItems.Count == 1) && (frame.BBox.Width != 0) && (frame.BBox.Height != 0))
                                {
                                    Rectangle bbox = frame.BBox;
                                    bbox.X = TX(bbox.X); bbox.Y = TY(bbox.Y);
                                    bbox.Width = TD(bbox.Width); bbox.Height = TD(bbox.Height);
                                    //daca e in dreptunghiul modulului
                                    if (pointIn(e.X, e.Y, bbox))
                                    {
                                        //verifica colturile de resize
                                        if (pointIn(e.X, e.Y, NE))
                                        {
                                            if (tool != Tools.BBox_Relocate && dragScroll)
                                            {
                                                lastTool = tool;
                                                tool = Tools.BBox_Relocate;
                                                resizeMode = ResizeMode.XY;
                                                relocMode = ResizeMode.XY;
                                            }
                                        }
                                        else if (pointIn(e.X, e.Y, SW))
                                        {
                                            if (tool != Tools.BBox_Resize && dragScroll)
                                            {
                                                lastTool = tool;
                                                tool = Tools.BBox_Resize;
                                                resizeMode = ResizeMode.XY;
                                            }
                                        }
                                        else if (pointIn(e.X, e.Y, NW))
                                        {
                                            if (tool != Tools.BBox_Relocate && dragScroll)
                                            {
                                                lastTool = tool;
                                                tool = Tools.BBox_Relocate;
                                                resizeMode = ResizeMode.XY;
                                                relocMode = ResizeMode.Y;
                                            }
                                        }
                                        else if (pointIn(e.X, e.Y, SE))
                                        {
                                            if (tool != Tools.BBox_Relocate && dragScroll)
                                            {
                                                lastTool = tool;
                                                tool = Tools.BBox_Relocate;
                                                resizeMode = ResizeMode.XY;
                                                relocMode = ResizeMode.X;
                                            }
                                        }
                                        else if (pointIn(e.X, e.Y, N))
                                        {
                                            if (tool != Tools.BBox_Relocate && dragScroll)
                                            {
                                                lastTool = tool;
                                                tool = Tools.BBox_Relocate;
                                                resizeMode = ResizeMode.Y;
                                                relocMode = ResizeMode.Y;
                                            }
                                        }
                                        else if (pointIn(e.X, e.Y, S))
                                        {
                                            if (tool != Tools.BBox_Resize && dragScroll)
                                            {
                                                lastTool = tool;
                                                tool = Tools.BBox_Resize;
                                                resizeMode = ResizeMode.Y;
                                            }
                                        }
                                        else if (pointIn(e.X, e.Y, E))
                                        {
                                            if (tool != Tools.BBox_Relocate && dragScroll)
                                            {
                                                lastTool = tool;
                                                tool = Tools.BBox_Relocate;
                                                resizeMode = ResizeMode.X;
                                                relocMode = ResizeMode.X;
                                            }
                                        }
                                        else if (pointIn(e.X, e.Y, W))
                                        {
                                            if (tool != Tools.BBox_Resize && dragScroll)
                                            {
                                                lastTool = tool;
                                                tool = Tools.BBox_Resize;
                                                resizeMode = ResizeMode.X;
                                            }
                                        }
                                        else //daca nu e nici un colt inseamna ca e move
                                        {
                                            if (tool != Tools.BBox_Move && dragScroll)
                                            {
                                                lastTool = tool;
                                                tool = Tools.BBox_Move;
                                            }
                                        }
                                    }

                                }
                            }
                        }
                        break;
                    case Views.animView:
                        {
                            if ((anim != null) && (aframe != null))
                            {
                                Rectangle frct = aframe.frame.GetRect();
                                if (pointIn(IX(e.X), IY(e.Y), new Rectangle(aframe.ox + frct.X, aframe.oy + frct.Y, frct.Width, frct.Height)))
                                {
                                    lastTool = tool;
                                    tool = Tools.Move;
                                    pbDraw.Cursor = Cursors.SizeAll;
                                    break;
                                }
                            }
                        }
                        break;
                }
            }
        }

        //intoarce culoare 0 transparent in afara imaginii
        Color SafeGetPixel(Bitmap bmp, int x, int y)
        {
            if ((x < 0) || (y < 0) || (x >= bmp.Width) || (y >= bmp.Height))
                return Color.FromArgb(0x00000000);
            return bmp.GetPixel(x, y);
        }

        //vede daca dreptunghiul intersecteaza marginea si intoarce pe flags ce margine intersecteaza
        const int K_RECT_EDGE_LEFT = 1;
        const int K_RECT_EDGE_RIGHT = 2;
        const int K_RECT_EDGE_UP = 4;
        const int K_RECT_EDGE_DOWN = 8;
        int IsRectEdgeAlphaPositive(Bitmap bmp, Rectangle rect, byte alphaThreshold)
        {
            int rectFlags = 0;
            //for separat ca sa mearga mai repede
            for (int xx = rect.Left; xx <= rect.Right; xx++)
            {
                if (SafeGetPixel(bmp, xx, rect.Top).A > alphaThreshold)
                {
                    rectFlags |= K_RECT_EDGE_UP;
                    break;
                }
            }
            for (int xx = rect.Left; xx <= rect.Right; xx++)
            {
                if (SafeGetPixel(bmp, xx, rect.Bottom).A > alphaThreshold)
                {
                    rectFlags |= K_RECT_EDGE_DOWN;
                    break;
                }
            }
            for (int yy = rect.Top; yy <= rect.Bottom; yy++)
            {
                if (SafeGetPixel(bmp, rect.Left, yy).A > alphaThreshold)
                {
                    rectFlags |= K_RECT_EDGE_LEFT;
                    break;
                }
            }

            for (int yy = rect.Top; yy <= rect.Bottom; yy++)
            {
                if (SafeGetPixel(bmp, rect.Right, yy).A > alphaThreshold)
                {
                    rectFlags |= K_RECT_EDGE_RIGHT;
                    break;
                }
            }

            return rectFlags;
        }

        Rectangle FindWandBB(Bitmap bmp, int x, int y, byte alphaThreshold)
        {
            if (bmp == null)
                return new Rectangle(0, 0, 0, 0);

            Rectangle outrect = new Rectangle(x, y, 0, 0);
            //daca dai click pe pixel gol intoarce dreptunghi cu latura 0
            if (bmp.GetPixel(x, y).A < alphaThreshold)
                return outrect;
            //gaseste dreptunghiul initial
            //walk left
            int px = x, py = y;
            while ((px > 0) && (bmp.GetPixel(px, py).A > alphaThreshold)) px--;
            outrect.X = px + 1;
            //walk right
            px = x;
            while ((px < bmp.Width - 1) && (bmp.GetPixel(px, py).A > alphaThreshold)) px++;
            outrect.Width = px - outrect.X;
            //walk up
            px = x; py = y;
            while ((py > 0) && (bmp.GetPixel(px, py).A > alphaThreshold)) py--;
            outrect.Y = py + 1;
            //walk down
            py = y;
            while ((py < bmp.Height - 1) && (bmp.GetPixel(px, py).A > alphaThreshold)) py++;
            outrect.Height = py - outrect.Y;
            //dupa gasirea primului dreptunghi il mareste pe axe pana cand laturile nu ating pixeli plini
            bool found = false;
            while (!found)
            {
                int rectflags = IsRectEdgeAlphaPositive(bmp, outrect, alphaThreshold);

                if ((rectflags & K_RECT_EDGE_LEFT) != 0)
                {
                    outrect.X--;
                    outrect.Width++;
                }
                if ((rectflags & K_RECT_EDGE_RIGHT) != 0)
                {
                    outrect.Width++;
                }
                if ((rectflags & K_RECT_EDGE_UP) != 0)
                {
                    outrect.Y--;
                    outrect.Height++;
                }
                if ((rectflags & K_RECT_EDGE_DOWN) != 0)
                {
                    outrect.Height++;
                }

                if (rectflags != 0)
                    found = false;
                else
                {
                    found = true;
                    //reglaje finale
                    outrect.X++;
                    outrect.Width--;
                    outrect.Y++;
                    outrect.Height--;
                }
            }


            return outrect;
        }

        //gaseste toate modulele dintr-o imagine si intoarce numarul acestora

        //clasa folosita la sortarea modulelor dupa X apoi Y
        int DetectModules(int imageIdx, byte threshold)
        {
            Bitmap bmp = sprite.images[imgIdx] as Bitmap;
            int count = 0;

            for (int yy = 0; yy < bmp.Height; yy++)
            {
                for (int xx = 0; xx < bmp.Width; xx++)
                {
                    if (bmp.GetPixel(xx, yy).A > threshold)
                    {
                        if (sprite.GetModuleByPoint(new Point(xx, yy), imageIdx) == null)
                        {
                            Rectangle rect = FindWandBB(bmp, xx, yy, threshold);
                            if ((rect.Width > 0) && (rect.Height > 0))
                            {
                                NewModule(imageIdx, (double)rect.X, (double)rect.Y, (double)rect.Width, (double)rect.Height, false);
                                count++;
                            }
                        }
                    }
                }

            }

            return count;
        }


        bool pointIn(int x, int y, Rectangle r)
        {
            return ((x >= r.Left) && (x <= r.Right) && (y >= r.Top) && (y <= r.Bottom));
        }

        private void pbDraw_MouseMove(object sender, MouseEventArgs e)
        {
            //save mouse pos
            cursorPbDraw.X = e.X;
            cursorPbDraw.Y = e.Y;

            bool askRedraw = false;

            if (dragScroll)
            {
                int deltaX = e.X - pbdX;
                int deltaY = e.Y - pbdY;
                Dx += deltaX;
                Dy += deltaY;
                int s = (int)Math.Round(scale); //dimensiunea pixelului
                int dx = Dx / s;
                int dy = Dy / s;
                Dx %= s;
                Dy %= s;
                pbdX = e.X;
                pbdY = e.Y;
                if (e.Button == MouseButtons.Right)
                {
                    ox += deltaX;
                    oy += deltaY;
                    askRedraw = true;
                }
                else if (e.Button == MouseButtons.Left)
                {
                    if (tool == Tools.Module)
                    {
                        rect.Width = IX(e.X) - rect.Left;
                        rect.Height = IY(e.Y) - rect.Top;
                        statusDim.Text = "W: " + rect.Width + " H: " + rect.Height;
                        askRedraw = true;
                    }
                    else if (tool == Tools.HitPoints)
                    {
                        if ((frame != null) && (selectedHitPt >= 0) && (selectedHitPt < frame.hitPoints.Count))
                        {
                            frame.hitPoints[selectedHitPt].X += dx;
                            frame.hitPoints[selectedHitPt].Y += dy;
                            oldhitptIdx = -1;
                            askRedraw = true;
                        }
                    }
                    else if (tool == Tools.Select)
                    {
                        if ((view == Views.moduleView) || (view == Views.frameView))
                        {
                            pbEndMX = IX(e.X);
                            pbEndMY = IY(e.Y);
                            askRedraw = true;
                        }
                    }
                    else if (tool == Tools.Resize)
                    {
                        if (view == Views.moduleView)
                        {
                            if (resizeMode == ResizeMode.XY)
                                ResizeModule(dx, dy);
                            else if (resizeMode == ResizeMode.Y)
                                ResizeModule(0, dy);
                            else if (resizeMode == ResizeMode.X)
                                ResizeModule(dx, 0);
                            askRedraw = true;
                        }
                    }
                    else if (tool == Tools.Relocate)
                    {
                        if (view == Views.moduleView)
                        {
                            int sx = -1, sy = -1;
                            if (relocMode == ResizeMode.XY)
                            {
                                MoveModule(dx, dy);
                            }
                            else if (relocMode == ResizeMode.X)
                            {
                                MoveModule(dx, 0);
                                sy = 1;
                            }
                            else if (relocMode == ResizeMode.Y)
                            {
                                MoveModule(0, dy);
                                sx = 1;
                            }

                            if (resizeMode == ResizeMode.XY)
                                ResizeModule(sx * dx, sy * dy);
                            else if (resizeMode == ResizeMode.Y)
                                ResizeModule(0, sy * dy);
                            else if (resizeMode == ResizeMode.X)
                                ResizeModule(sx * dx, 0);
                            askRedraw = true;
                        }
                    }
                    else if (tool == Tools.Move)
                    {
                        if (view == Views.moduleView)
                        {
                            MoveSelectedModules(dx, dy);
                        }
                        else if (view == Views.frameView)
                        {
                            if (tool == Tools.Move)
                            {
                                MoveSelectedFModules(dx, dy);
                            }
                        }
                        else if (view == Views.animView)
                        {
                            MoveAFrame(aframe, dx, dy);
                        }

                        askRedraw = true;
                    }
                    else if (pointIn(e.X, e.Y, rect))
                    {
                        if (tool == Tools.Select)
                        {
                            lastTool = tool;
                            tool = Tools.Move;
                        }
                    }
                    //BBox
                    if (frame != null)
                    {
                        if (tool == Tools.BBox_Resize)
                        {
                            if (resizeMode == ResizeMode.XY)
                                EditFrameBBox(frame, 0, 0, dx, dy);
                            else if (resizeMode == ResizeMode.Y)
                                EditFrameBBox(frame, 0, 0, 0, dy);
                            else if (resizeMode == ResizeMode.X)
                                EditFrameBBox(frame, 0, 0, dx, 0);

                            askRedraw = true;
                        }
                        else if (tool == Tools.BBox_Move)
                        {
                            EditFrameBBox(frame, dx, dy, 0, 0);

                            askRedraw = true;
                        }
                        else if (tool == Tools.BBox_Relocate)
                        {
                            int sx = -1, sy = -1;
                            if (relocMode == ResizeMode.XY)
                            {
                                EditFrameBBox(frame, dx, dy, 0, 0);
                            }
                            else if (relocMode == ResizeMode.X)
                            {
                                EditFrameBBox(frame, dx, 0, 0, 0);
                                sy = 1;
                            }
                            else if (relocMode == ResizeMode.Y)
                            {
                                EditFrameBBox(frame, 0, dy, 0, 0);
                                sx = 1;
                            }

                            if (resizeMode == ResizeMode.XY)
                                EditFrameBBox(frame, 0, 0, sx * dx, sy * dy);
                            else if (resizeMode == ResizeMode.Y)
                                EditFrameBBox(frame, 0, 0, 0, sy * dy);
                            else if (resizeMode == ResizeMode.X)
                                EditFrameBBox(frame, 0, 0, sx * dx, 0);

                            askRedraw = true;
                        }
                    }
                }
            }
            //cand afisez cursor highlight
            if (cursorHighlight)
                askRedraw = true;
            //daca am cerut redraw, redesenez area
            if (askRedraw)
            {
                pbDraw.Refresh();
            }

            //update status bar
            statusCoord.Text = "X: " + (int)((e.X - ox) / scale) + " Y: " + (int)((e.Y - oy) / scale);
        }

        enum ResizeMode { XY, X, Y, None };
        ResizeMode resizeMode, relocMode;

        private void pbDraw_MouseUp(object sender, MouseEventArgs e)
        {
            if (e.Button == MouseButtons.Left)
            {
                switch (view)
                {
                    case Views.moduleView:
                        {
                            //dragScroll = false;
                            if (tool == Tools.Module)
                            {
                                //adauga un modul nou
                                RectangleF final = NormalizeRectangle(rect);
                                if((final.Width > 0) && (final.Height > 0))
                                    NewModule(imgIdx, final.X, final.Y, final.Width, final.Height);

                                rect.Y = 0;
                                rect.X = 0;
                                rect.Width = 0;
                                rect.Height = 0;

                                pbDraw.Cursor = Cursors.Default;
                                module = sprite.modules[sprite.modules.Count - 1];
                            }
                            else if (tool == Tools.Select)
                            {
                                pbEndMX = IX(e.X);
                                pbEndMY = IY(e.Y);


                                bool modsSel = false;
                                if ((pbStartMX != pbEndMX) && (pbStartMY != pbEndMY))
                                {
                                    lvMain.SuspendLayout();

                                    Rectangle recto = new Rectangle(pbStartMX, pbStartMY, pbEndMX - pbStartMX, pbEndMY - pbStartMY);
                                    recto = NormalizeRectangle(recto);
                                    //vede ce module se afla EXCLUSIV inauntrul selectiei si le selecteaza
                                    if (Control.ModifierKeys != Keys.Control)
                                        RemoveSelection(lvMain);

                                    module = null;

                                    supressSelIdxChanged = true;

                                    foreach (Module mod in sprite.modules)
                                    {
                                        if ((mod.imageID == imgIdx) && (pointIn(mod.x, mod.y, recto)) && (pointIn(mod.x + mod.w, mod.y + mod.h, recto)))
                                        {
                                            lvMain.Items[mod.index].Selected = true;
                                            module = mod;

                                            modsSel = true;
                                        }
                                    }

                                    supressSelIdxChanged = false;
                                    lvMain.ResumeLayout(true);
                                    //modsSel = false;
                                    //desface selectia
                                    pbStartMX = pbStartMY = pbEndMX = pbEndMY = 0;
                                }

                                if (!modsSel)
                                {
                                    foreach (Module mod in sprite.modules)
                                    {
                                        int x = IX(e.X);
                                        int y = IY(e.Y);
                                        //Color col = Color.White;
                                        if ((mod.imageID == imgIdx) && (pointIn(x, y, new Rectangle(mod.x, mod.y, mod.w, mod.h))))
                                        {
                                            if (Control.ModifierKeys != Keys.Control)
                                            {
                                                SelectModule(mod.index);
                                                SelectItem(lvMain, mod.index);
                                            }
                                            else
                                            {
                                                lvMain.Items[mod.index].Selected = !lvMain.Items[mod.index].Selected;
                                            }
                                            break;
                                        }
                                    }
                                }
                            }
                            else if (tool == Tools.Resize || tool == Tools.Move)
                            {
                                tool = Tools.Select;
                                pbDraw.Cursor = Cursors.Default;
                            }
                            else if (tool != Tools.Wand)
                            {
                                tool = lastTool; // Tools.Select;
                                pbDraw.Cursor = Cursors.Default;
                            }
                        }
                        break;
                    case Views.frameView:
                        {
                            if (tool == Tools.Select)
                            {
                                //reset tool
                                lastTool = tool;
                                tool = Tools.Select;
                                pbDraw.Cursor = Cursors.Default;

                                if (frame != null)
                                {
                                    pbEndMX = IX(e.X);
                                    pbEndMY = IY(e.Y);

                                    bool fmodsSel = false;
                                    if ((pbStartMX != pbEndMX) && (pbStartMY != pbEndMY))
                                    {
                                        lvSecond.SuspendLayout();

                                        Rectangle recto = new Rectangle(pbStartMX, pbStartMY, pbEndMX - pbStartMX, pbEndMY - pbStartMY);
                                        recto = NormalizeRectangle(recto);
                                        //vede ce fmodule se afla EXCLUSIV inauntrul selectiei si le selecteaza
                                        if (Control.ModifierKeys != Keys.Control)
                                            RemoveSelection(lvSecond);

                                        fmodule = null;

                                        foreach (FrameModule fmod in frame.fmodules)
                                        {
                                            if ((pointIn(fmod.ox, fmod.oy, recto)) && (pointIn(fmod.ox + fmod.module.w, fmod.oy + fmod.module.h, recto)))
                                            {
                                                lvSecond.Items[fmod.index].Selected = true;
                                                fmodule = fmod;

                                                fmodsSel = true;
                                            }
                                        }

                                        lvSecond.ResumeLayout(true);
                                        //desface selectia
                                        pbStartMX = pbStartMY = pbEndMX = pbEndMY = 0;
                                        lvSecond.Focus();
                                    }

                                    if (!fmodsSel)
                                    {
                                        foreach (FrameModule fmod in frame.fmodules)
                                        {
                                            int x = IX(e.X);
                                            int y = IY(e.Y);
                                            //Color col = Color.White;
                                            if (pointIn(x, y, new Rectangle(fmod.ox, fmod.oy, fmod.module.w, fmod.module.h)))
                                            {
                                                if (Control.ModifierKeys != Keys.Control)
                                                {
                                                    SelectFModule(fmod.index);
                                                    SelectItem(lvSecond, fmod.index);
                                                    lvSecond.Focus();
                                                }
                                                else
                                                {
                                                    lvSecond.Items[fmod.index].Selected = !lvSecond.Items[fmod.index].Selected;
                                                    lvSecond.Focus();
                                                }
                                                break;
                                            }
                                        }
                                    }
                                }
                            }
                            else if (tool == Tools.Move)
                            {
                                tool = Tools.Select;
                                pbDraw.Cursor = Cursors.Default;
                            }
                            else if (tool == Tools.BBox_Resize || tool == Tools.BBox_Move || tool == Tools.BBox_Relocate)
                            {
                                tool = Tools.BBox;
                                pbDraw.Cursor = Cursors.Default;
                            }
                            else if (tool == Tools.HitPoints)
                            {
                                if (frame == null) break;
                                if (selectedHitPt == -1)
                                {
                                    HitPoint hpt = new HitPoint(IX(e.X), IY(e.Y));
                                    if (e.X - ox < 0) hpt.X--;
                                    if (e.Y - oy < 0) hpt.Y--;

                                    frame.hitPoints.Add(hpt);
                                    //adauga in lista nr de hitpts
                                    lvMain.Items[frame.index].SubItems[3].Text = frame.hitPoints.Count.ToString();
                                    selectedHitPt = frame.hitPoints.Count - 1;
                                    if (hitptWnd != null)
                                    {
                                        hitptWnd.RefreshValues(frame.hitPoints[selectedHitPt].X, frame.hitPoints[selectedHitPt].Y, frame.hitPoints[selectedHitPt].flags);
                                    }
                                }
                                else
                                if ((selectedHitPt == oldhitptIdx) && (selectedHitPt >= 0) && (selectedHitPt < frame.hitPoints.Count))
                                {
                                    if (hitptWnd == null)
                                    {
                                        hitptWnd = new HitPointWnd(OptionsWindowCallback, frame.hitPoints[selectedHitPt].X, frame.hitPoints[selectedHitPt].Y, frame.hitPoints[selectedHitPt].flags);
                                        hitptWnd.Show();
                                    }
                                }
                            }
                        }
                        break;
                    case Views.animView:
                        {
                            //reset tool
                            lastTool = tool;
                            tool = Tools.Select;
                            pbDraw.Cursor = Cursors.Default;
                        }
                        break;
                }
            }

            dragScroll = false;
            pbDraw.Refresh();
        }

        #endregion

        #region Thumbs

        PictureBox[] pbThumbs; //vector de controale continute in panel-ul de jos

        //curata si adauga n controale PictureBox in panel-ul de thumbs
        void AddThumbs(int n)
        {
            pThumbs.AutoScroll = false;
            pThumbs.AutoScrollPosition = new Point(0, 0);
            //pThumbs.SuspendLayout();

            pThumbs.Controls.Clear();

            if (n == 0)
            {
                hScrollBarThumbs.Enabled = false;
                return;
            }
            pbThumbs = new PictureBox[n];

            int h = pThumbs.ClientSize.Height;
            int d = 4; //distanta dintre ele
            int w = h - 2 * d;

            //find visible
            int totalw = n * (w + d) + d;
            int scrollsize = totalw;// - pThumbs.ClientSize.Width + d * 2;
            if (scrollsize < 0)
                scrollsize = 0;

            hScrollBarThumbs.Enabled = true;
            hScrollBarThumbs.Minimum = 0;
            hScrollBarThumbs.Maximum = scrollsize;
            hScrollBarThumbs.Value = 0;

            if (scrollsize <= 0)
            {
                hScrollBarThumbs.Enabled = false;
            }

            for (int i = 0; i < n; i++)
            {
                pbThumbs[i] = new PictureBox();
                pbThumbs[i].BackColor = colThumbsBack;

                pbThumbs[i].Paint += new PaintEventHandler(pbModules_Paint);
                pbThumbs[i].DoubleClick += new EventHandler(pbThumbs_DoubleClick);
                pbThumbs[i].MouseClick += new MouseEventHandler(pbThumbs_Click);

                //aflu locatia reala
                Point location = new Point((w + d) * i + d, d);
                //scad scroll
                location.X -= hScrollBarThumbs.Value;
                //pozitionez
                if (location.X + w < 0)
                    location.X = -100;
                else if (location.X > pThumbs.ClientSize.Width)
                    location.X = pThumbs.ClientSize.Width + 100;

                pbThumbs[i].Location = location;

                pbThumbs[i].Height = w;
                pbThumbs[i].Width = w;

            }
            pbThumbs[n - 1].BackColor = colThumbsSelection;

            pThumbs.Controls.AddRange(pbThumbs);
            //pThumbs.ResumeLayout(true);
            ResizeThumbs();
        }

        //pe click selecteaza thumb-ul curent
        private void pbThumbs_Click(object sender, MouseEventArgs e)
        {
            if ((e.Button == MouseButtons.Left) || (e.Button == MouseButtons.Right))
            {
                PictureBox pb = sender as PictureBox;
                foreach (PictureBox pbi in pbThumbs)
                {
                    pbi.BackColor = colThumbsBack;
                    pbi.BorderStyle = BorderStyle.None;
                }

                if (view == Views.moduleView)
                {
                    //ii pune contur la poza selectata
                    pb.BorderStyle = BorderStyle.Fixed3D;
                    pb.BackColor = colThumbsSelection;
                    //schimba imaginea afisata
                    int idx = pThumbs.Controls.IndexOf(sender as PictureBox);
                    img = sprite.images[idx];
                    imgIdx = idx;
                    pbDraw.Refresh();
                    //deselecteaza modul
                    //RemoveSelection(lvMain);
                }

            }

            if (e.Button == MouseButtons.Right)
            {
                cmImages.Items.Clear();
                switch (view)
                {
                    case Views.moduleView:
                        {
                            cmImages.Items.Add("Move Selected Modules Here");
                            cmImages.Items[cmImages.Items.Count - 1].Name = "MoveModulesHere";
                            cmImages.Items[cmImages.Items.Count - 1].Click += new EventHandler(cmImages_Click);
                            cmImages.Items.Add("-");
                            cmImages.Items.Add("Mirror X (mod+img)");
                            cmImages.Items[cmImages.Items.Count - 1].Name = "CreateFlipXModules";
                            cmImages.Items[cmImages.Items.Count - 1].Click += new EventHandler(cmImages_Click);
                            cmImages.Items.Add("Mirror Y (mod+img)");
                            cmImages.Items[cmImages.Items.Count - 1].Name = "CreateFlipYModules";
                            cmImages.Items[cmImages.Items.Count - 1].Click += new EventHandler(cmImages_Click);
                            cmImages.Items.Add("-");
                            cmImages.Items.Add("Save Image As...");
                            cmImages.Items[cmImages.Items.Count - 1].Name = "SaveImageAs";
                            cmImages.Items[cmImages.Items.Count - 1].Click += new EventHandler(cmImages_Click);
                            cmImages.Items.Add("Load Image...");
                            cmImages.Items[cmImages.Items.Count - 1].Name = "LoadImage";
                            cmImages.Items[cmImages.Items.Count - 1].Click += new EventHandler(cmImages_Click);
                            cmImages.Items.Add("-");
                            cmImages.Items.Add("Remove Image");
                            cmImages.Items[cmImages.Items.Count - 1].Name = "RemoveImage";
                            cmImages.Items[cmImages.Items.Count - 1].Click += new EventHandler(cmImages_Click);
                        }
                        break;
                    case Views.frameView:
                        {
                            int idx = pThumbs.Controls.IndexOf(sender as PictureBox);

                            cmImages.Items.Add("All Modules to Frames [from " + idx + "]");
                            cmImages.Items[cmImages.Items.Count - 1].Name = "AddOneModulePerFrame";
                            cmImages.Items[cmImages.Items.Count - 1].Click += new EventHandler(cmImages_Click);
                            cmImages.Items[cmImages.Items.Count - 1].ImageIndex = idx;

                            cmImages.Items.Add("-");

                            if (showFlipXMods == true)
                                cmImages.Items.Add("Disable FlipX mods");
                            else
                                cmImages.Items.Add("Enable FlipX mods");
                            cmImages.Items[cmImages.Items.Count - 1].Name = "ShowFlipXMods";
                            cmImages.Items[cmImages.Items.Count - 1].Click += new EventHandler(cmImages_Click);

                            if (showFlipYMods == true)
                                cmImages.Items.Add("Disable FlipY mods");
                            else
                                cmImages.Items.Add("Enable FlipY mods");
                            cmImages.Items[cmImages.Items.Count - 1].Name = "ShowFlipYMods";
                            cmImages.Items[cmImages.Items.Count - 1].Click += new EventHandler(cmImages_Click);
                        }
                        break;
                    case Views.animView:
                        {
                        }
                        break;
                }
                cmImages.Show(sender as PictureBox, e.Location);
            }

        }

        //pe dublu-click insereaza frame module in frame sau anim frame in anim
        void pbThumbs_DoubleClick(object sender, EventArgs e)
        {
            int idx = pThumbs.Controls.IndexOf(sender as PictureBox);
            if (view == Views.moduleView)
            {
                img = sprite.images[idx];
                imgIdx = idx;
                //seteaza imageID la modulele selectate
                foreach (ListViewItem lvi in lvMain.SelectedItems)
                {
                    sprite.modules[lvi.Index].imageID = idx;
                    lvi.SubItems[1].Text = idx.ToString();
                }
                pbDraw.Refresh();
            }
            else
            if (view == Views.frameView)
            {
                if (frame == null)
                {
                    NewFrame();
                }
                NewFModule(idx);
                pbDraw.Refresh();
            }
            else if (view == Views.animView)
            {
                if (anim == null)
                    NewAnim();
                NewAFrame(idx);
                pbDraw.Refresh();
            }
        }

        //handler pt butonul de add din meniul contextula pt panel-ul de thumbs
        //in module view permite adaugarea unei imagini noi
        private void addToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (view == Views.moduleView)
            {
                OpenFileDialog ofd = new OpenFileDialog
                {
                    AutoUpgradeEnabled = false
                };
                ofd.Filter = "Portable Network Graphics (*.png)|*.png";
                if (ofd.ShowDialog() == DialogResult.Cancel)
                    return;

                using (FileStream stream = new FileStream(ofd.FileName, FileMode.Open, FileAccess.Read))
                {
                    img = Image.FromStream(stream);
                }

                sprite.images.Add(img);
                sprite.imgPath.Add(Path.GetFileName(ofd.FileName));
                AddThumbs(sprite.images.Count);
                imgIdx = sprite.images.Count - 1;
                pbDraw.Refresh();
            }
        }

        private void hScrollBarThumbs_ValueChanged(object sender, EventArgs e)
        {
            ResizeThumbs();
        }

        void ResizeThumbs()
        {
            //set scrollbar size
            splitContainer1.SplitterDistance = splitContainer1.Height - 15;

            //pThumbs.SuspendLayout();

            pThumbs.AutoScrollPosition = new Point(0, 0);
            pThumbs.AutoScroll = false;

            int h = pThumbs.ClientSize.Height;
            int d = 4;
            int w = h - 2 * d;

            int n = 0;
            if (pbThumbs != null)
                n = pbThumbs.Length;
            int totalw = n * (w + d) + d;
            int scrollsize = totalw;// - pThumbs.ClientSize.Width + d * 2;
            if (scrollsize < 0)
                scrollsize = 0;

            if (scrollsize <= 0)
            {
                hScrollBarThumbs.Enabled = false;
            }
            else
            {
                hScrollBarThumbs.Enabled = true;
            }

            hScrollBarThumbs.Minimum = 0;
            hScrollBarThumbs.Maximum = scrollsize;
            hScrollBarThumbs.SmallChange = w;
            hScrollBarThumbs.LargeChange = pThumbs.ClientSize.Width;
            //cursor scroll
            if (hScrollBarThumbs.Maximum - pThumbs.ClientSize.Width > 0)
            {
                if (hScrollBarThumbs.Value > hScrollBarThumbs.Maximum - pThumbs.ClientSize.Width)
                    hScrollBarThumbs.Value = hScrollBarThumbs.Maximum - pThumbs.ClientSize.Width;
            }

            if (pbThumbs != null)
            {
                for (int i = 0; i < pbThumbs.Length; i++)
                {
                    PictureBox thumb = pbThumbs[i];
                    if (thumb != null)
                    {
                        //aflu locatia reala
                        Point location = new Point((w + d) * i + d, d);
                        //scad scroll
                        location.X -= hScrollBarThumbs.Value;
                        //pozitionez
                        if (location.X + w < 0)
                        {
                            //location.X = -100;
                            thumb.Visible = false;
                        }
                        else if (location.X > pThumbs.ClientSize.Width)
                        {
                            //location.X = pThumbs.ClientSize.Width + 100;
                            thumb.Visible = false;
                        }
                        else
                        {
                            thumb.Visible = true;
                            thumb.Location = location;
                            thumb.Height = w;
                            thumb.Width = w;
                        }

                    }
                }
            }
            //pThumbs.ResumeLayout(true);
        }

        private void pThumbs_Resize(object sender, EventArgs e)
        {
            ResizeThumbs();
        }

        void pbModules_Paint(object sender, PaintEventArgs e)
        {
            if (!(sender is PictureBox))
                return;

            PictureBox pbThumb = sender as PictureBox;
            int idx = pThumbs.Controls.IndexOf(pbThumb);
            if (idx < 0)
                return;
            if (view == Views.frameView)
            {
                PictureBox pb = sender as PictureBox;
                int wt = pb.ClientRectangle.Width;
                Module mod = sprite.GetModuleByIndex(idx);
                if (mod == null)
                    return;

                if (showFlipXMods && showFlipYMods)
                {
                    mod.Paint(e.Graphics, (wt - mod.w) / 2, (wt - mod.h) / 2, RotateFlipType.RotateNoneFlipXY);
                    e.Graphics.DrawString("xy", fontArialBold, Brushes.Red, 0, 0);
                }
                else if (showFlipXMods == true)
                {
                    mod.Paint(e.Graphics, (wt - mod.w) / 2, (wt - mod.h) / 2, RotateFlipType.RotateNoneFlipX);
                    e.Graphics.DrawString("x", fontArialBold, Brushes.Red, 0, 0);
                }
                else if (showFlipYMods == true)
                {
                    mod.Paint(e.Graphics, (wt - mod.w) / 2, (wt - mod.h) / 2, RotateFlipType.RotateNoneFlipY);
                    e.Graphics.DrawString("y", fontArialBold, Brushes.Red, 0, 0);
                }
                else
                {
                    float dx = (float)wt / (float)mod.w;
                    float dy = (float)wt / (float)mod.h;
                    float x, y;

                    x = (wt - mod.w) / 2;
                    y = (wt - mod.h) / 2;

                    if (dx < 1.0f || dy < 1.0f)
                    {
                        float s = Math.Min(dx, dy);

                        x = (wt / s - mod.w) * 0.5f;
                        y = (wt / s - mod.h) * 0.5f;

                        e.Graphics.ScaleTransform(s, s);
                    }

                    mod.Paint(e.Graphics, (int)x, (int)y);
                    e.Graphics.ResetTransform();
                }
            }
            else if (view == Views.moduleView)
            {
                if (idx >= sprite.images.Count)
                    return;

                Rectangle rectdst = new Rectangle(0, 0, pbThumb.Width, pbThumb.Height);
                int iw = sprite.images[idx].Width;
                int ih = sprite.images[idx].Height;
                if (iw > ih)
                {
                    float raport = (float)ih / (float)iw;
                    rectdst.Width = pbThumb.Width;
                    rectdst.Height = (int)((float)pbThumb.Width * raport);
                    rectdst.Y = (pbThumb.Height - rectdst.Height) / 2;
                }
                else
                {
                    float raport = (float)iw / (float)ih;
                    rectdst.Height = pbThumb.Height;
                    rectdst.Width = (int)((float)pbThumb.Height * raport);
                    rectdst.X = (pbThumb.Width - rectdst.Width) / 2;
                }

                sprite.PaintImageSmooth(e.Graphics, idx, rectdst);
            }
            else if (view == Views.animView)
            {
                PictureBox pb = sender as PictureBox;
                int wt = pb.ClientRectangle.Width;

                Rectangle r = sprite.frames[idx].GetRect();
                //float scalex = ((float)wt / (float)r.Width);
                //float scaley = ((float)wt / (float)r.Height);
                //float scale = 1.0f;
                //if(scalex < scale)
                //    scale = scalex;
                //if(scaley < scale)
                //    scale = scaley;
                //e.Graphics.ResetTransform();
                //e.Graphics.ScaleTransform(scale, scale);
                sprite.GetFrameByIndex(idx).Paint(e.Graphics, (int)(-r.X + wt / 2 - r.Width / 2), (int)(-r.Y + wt / 2 - r.Height / 2));
            }
        }

        #endregion

        #region FileOperations

        // format of json frame structure when loading from JSON file
        public class CJSONimportFrame
        {
            public string strFrameName;

            public Rectangle frame;
            public bool rotated;
            public bool trimmed;
            public Rectangle spriteSourceSize;
            public Size sourceSize;
            public PointF anchor;

            public CJSONimportFrame()
            {
                strFrameName = "";
                frame = new Rectangle();
                rotated = false;
                trimmed = false;
                spriteSourceSize = new Rectangle();
                sourceSize = new Size();
                anchor = new PointF();
            }
        }
        // global list for loaded json frames. Only used when importing from json
        public List<CJSONimportFrame> g_JSONframes = new List<CJSONimportFrame>();
        public string g_JSONfilename = "";

        // Returns the json import frame by its name
        public CJSONimportFrame GetJSONframeByName(string strName)
        {
            for (int kk = 0; kk < g_JSONframes.Count; kk++)
            {
                CJSONimportFrame fr = g_JSONframes[kk] as CJSONimportFrame;
                if (fr.strFrameName == strName)
                    return fr;
            }

            return null;
        }

        // classes used for exporting JSON
        public class CJSONexportSprite
        {
            public string spriteName; //targeted sprite
            public float anchorX;
            public float anchorY;
        }
        public class CJSONexportFrame
        {
            public int[] boundingBox = new int[4]; // bounding box of frame in absolute offsets: x,y,w,h
            public int offX; // contains data from animation oX and oY
            public int offY;
            public string strFlags; //string flags
            public List<CJSONexportSprite> sprites;
        }
        public class CJSONexportAnim
        {
            public string name;
            public bool looping;
            public List<CJSONexportFrame> frames;
        }
        public class CJSONexportFile
        {
            public string targetFilename;      //name of pixi json file that it complements
            public List<CJSONexportAnim> animations;
        }

        // Saves all necessary data into a JSON format
        public CJSONexportFile GetJSONfromEditor()
        {
            CJSONexportFile fl = new CJSONexportFile();
            fl.targetFilename = Path.GetFileName(g_JSONfilename);
            fl.animations = new List<CJSONexportAnim>();

            //add animations

            ///--- sort anims by ID ---
            List<Animation> tempAnims = new List<Animation>();
            sprite.anims.Sort(Sprite.CompareEntities);
            foreach (Animation anm in sprite.anims)
            {
                tempAnims.Add(anm);
            }

            for (int kk = 0; kk < tempAnims.Count; kk++)
            {
                Animation anim = sprite.anims[kk] as Animation;

                CJSONexportAnim nanim = new CJSONexportAnim();
                nanim.name = anim.name;
                nanim.looping = ((anim.flags & 1) != 0) ? true : false;
                nanim.frames = new List<CJSONexportFrame>();


                ///--- sort animFrames by ID ---
                List<AnimFrame> tempAframes = new List<AnimFrame>();
                anim.aframes.Sort(Sprite.CompareEntities);
                foreach (AnimFrame afm in anim.aframes)
                {
                    tempAframes.Add(afm);
                }

                for (int ll = 0; ll < tempAframes.Count; ll++)
                {
                    AnimFrame aframe = tempAframes[ll] as AnimFrame;

                    CJSONexportFrame nframe = new CJSONexportFrame();
                    //write as array so it's easy to load
                    nframe.boundingBox[0] = aframe.frame.BBox.X;
                    nframe.boundingBox[1] = aframe.frame.BBox.Y;
                    nframe.boundingBox[2] = aframe.frame.BBox.Width;
                    nframe.boundingBox[3] = aframe.frame.BBox.Height;
                    //save aframe flags too 
                    nframe.strFlags = aframe.strFlags;

                    nframe.offX = aframe.ox;
                    nframe.offY = aframe.oy;
                    nframe.sprites = new List<CJSONexportSprite>();

                    for (int mm = 0; mm < aframe.frame.fmodules.Count; mm++)
                    {
                        FrameModule fmodule = aframe.frame.fmodules[mm] as FrameModule;

                        CJSONexportSprite nsprite = new CJSONexportSprite();
                        nsprite.spriteName = fmodule.module.strName;
                        // compute anchor relative to original source rectangle (defaults to center)
                        nsprite.anchorX = 0.5f;
                        nsprite.anchorY = 0.5f;
                        CJSONimportFrame infr = GetJSONframeByName(fmodule.module.strName);
                        if (infr != null)
                        {
                            float ancX = (float)(infr.spriteSourceSize.X - fmodule.ox) / (float)infr.sourceSize.Width;
                            float ancY = (float)(infr.spriteSourceSize.Y - fmodule.oy) / (float)infr.sourceSize.Height;

                            nsprite.anchorX = ancX;
                            nsprite.anchorY = ancY;
                        }
                        else
                        {
                            MessageBox.Show("Couldn't find sprite in imported frames: [" + fmodule.module.strName + "] !", "Warning!");
                        }

                        nframe.sprites.Add(nsprite);
                    }

                    nanim.frames.Add(nframe);
                }

                fl.animations.Add(nanim);
            }

            return fl;
        }


        // Imports modules from JSON exported by TexturePacker for PixiJS
        public void ImportModulesFromTexpackerJSON(string strJSONpath)
        {
            // clear imported frames list
            g_JSONframes.Clear();
            g_JSONfilename = "";

            bool bRotationNotAllowed = false;
            bool bAnchorNotAllowed = false;
            // load json data
            try
            {
                string jsontext = File.ReadAllText(strJSONpath);
                JObject rss = JObject.Parse(jsontext);

                // remove modules ONLY
                sprite.images.RemoveRange(0, sprite.images.Count);
                sprite.imgPath.RemoveRange(0, sprite.imgPath.Count);
                sprite.modules.RemoveRange(0, sprite.modules.Count);
                img = null;
                imgIdx = 0;
                module = null;

                //get image name
                string strImgName = (string)rss["meta"]["image"];
                string strImgPath = Path.GetDirectoryName(strJSONpath) + "\\" + strImgName;
                ///--- load image ---
                sprite.imgPath.Add(strImgName);
                Image limg = null;
                using (var bmpTemp = new Bitmap(strImgPath))
                {
                    limg = new Bitmap(bmpTemp);
                }
                sprite.images.Add(limg);

                ///--- load modules ---
                JObject oFrames = (JObject)rss["frames"];

                // get standard key/value pairs:
                foreach (var oFrm in oFrames)
                {
                    CJSONimportFrame nfrm = new CJSONimportFrame();

                    string strModuleName = oFrm.Key.ToString();
                    nfrm.strFrameName = strModuleName;
                    nfrm.frame.X = (int)(rss["frames"][strModuleName]["frame"]["x"]);
                    nfrm.frame.Y = (int)(rss["frames"][strModuleName]["frame"]["y"]);
                    nfrm.frame.Width = (int)(rss["frames"][strModuleName]["frame"]["w"]);
                    nfrm.frame.Height = (int)(rss["frames"][strModuleName]["frame"]["h"]);
                    // rotation not supported yet
                    nfrm.rotated = (bool)(rss["frames"][strModuleName]["rotated"]);
                    if (nfrm.rotated)
                        bRotationNotAllowed = true;

                    nfrm.trimmed = (bool)(rss["frames"][strModuleName]["trimmed"]);

                    if (rss["frames"][strModuleName]["anchor"] != null)
                    {
                        nfrm.anchor.X = (float)(rss["frames"][strModuleName]["anchor"]["x"]);
                        nfrm.anchor.Y = (float)(rss["frames"][strModuleName]["anchor"]["y"]);
                        if ((nfrm.anchor.X != 0.0f) || (nfrm.anchor.Y != 0.0f))
                            bAnchorNotAllowed = true;
                    }

                    nfrm.spriteSourceSize.X = (int)(rss["frames"][strModuleName]["spriteSourceSize"]["x"]);
                    nfrm.spriteSourceSize.Y = (int)(rss["frames"][strModuleName]["spriteSourceSize"]["y"]);
                    nfrm.spriteSourceSize.Width = (int)(rss["frames"][strModuleName]["spriteSourceSize"]["w"]);
                    nfrm.spriteSourceSize.Height = (int)(rss["frames"][strModuleName]["spriteSourceSize"]["h"]);

                    nfrm.sourceSize.Width = (int)(rss["frames"][strModuleName]["sourceSize"]["w"]);
                    nfrm.sourceSize.Height = (int)(rss["frames"][strModuleName]["sourceSize"]["h"]);

                    g_JSONframes.Add(nfrm);
                }

                // add modules
                int idx = 0;
                for (int kk = 0; kk < g_JSONframes.Count; kk++)
                {
                    CJSONimportFrame pfr = g_JSONframes[kk] as CJSONimportFrame;
                    // add modules
                    Module mod = new Module();
                    mod.index = Convert.ToInt32(idx);
                    mod.ID = Convert.ToInt32(idx);
                    mod.imageID = 0; //only using one image so default to 0
                    mod.x = pfr.frame.X;
                    mod.y = pfr.frame.Y;
                    mod.w = pfr.frame.Width;
                    mod.h = pfr.frame.Height;
                    mod.strName = pfr.strFrameName;
                    sprite.modules.Add(mod);

                    idx++;
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show("JSON File failed to open!\n" + ex.Message, "ERROR !", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }

            //keep loaded filename for export
            g_JSONfilename = strJSONpath;


            if (bRotationNotAllowed)
            {
                MessageBox.Show("Rotated sprites are not supported yet. They are loaded but output might be broken! Export the source JSON without 'allow rotation'", "Warning!", MessageBoxButtons.OK, MessageBoxIcon.Information);
            }
            if (bAnchorNotAllowed)
            {
                MessageBox.Show("Some sprites have anchors. Anchors should be always set to TOP-LEFT! Export the source JSON without anchors", "Warning!", MessageBoxButtons.OK, MessageBoxIcon.Information);
            }

            //refresh view
            if (sprite.images.Count != 0)
            {
                img = sprite.images[0];
                imgIdx = 0;
            }

            // check frame modules to make sure we don't have wrong modules in it and to link to good modules if we have the same order
            foreach (Frame f in sprite.frames)
            {
                for (int i = f.fmodules.Count - 1; i >= 0; i--)
                {
                    FrameModule fm = f.fmodules[i];
                    Module nmod = sprite.GetModuleByName(fm.module.strName);
                    if (nmod == null)
                    {
                        f.DeleteFModule(fm);
                    }
                    else
                    {
                        fm.module = nmod;
                        fm.moduleID = nmod.ID;
                    }
                }
            }
        }

        private void openToolStripMenuItem_Click(object sender, EventArgs e)
        {
            OpenFileDialog ofd = new OpenFileDialog
            {
                AutoUpgradeEnabled = false
            };
            ofd.Filter = "SpriteX (*.bsx)|*.bsx|All Files (*.*)|*.*";
            ofd.DefaultExt = ".bsx";
            if (ofd.ShowDialog() == DialogResult.Cancel)
                return;
            ResetVars();
            sprite.Open(ofd.FileName);
            SetWorkingFile(ofd.FileName);
            SetModified(false);
            if (sprite.images.Count != 0)
            {
                img = sprite.images[0];
                imgIdx = 0;
                //pbDraw.Refresh();
            }

            switch (view)
            {
                case Views.moduleView:
                    InitModuleView();
                    break;
                case Views.frameView:
                    InitFrameView();
                    break;
                case Views.animView:
                    InitAnimView();
                    break;
            }

            pbDraw.Refresh();
        }

        private void saveAsToolStripMenuItem_Click(object sender, EventArgs e)
        {
            SaveFileDialog sfd = new SaveFileDialog();
            sfd.Filter = "SpriteX (*.bsx)|*.bsx|All Files (*.*)|*.*";
            sfd.DefaultExt = ".bsx";
            if (sfd.ShowDialog() == DialogResult.Cancel)
                return;

            if (File.Exists(sfd.FileName))
                File.Delete(sfd.FileName);

            SetWorkingFile(sfd.FileName);
            sprite.Save(sfd.FileName);
            SetModified(false);
        }

        void SetModified(bool val)
        {
            bFileChanged = val;
            Text = "SpriteEd " + Path.GetFileName(strLoadedFileName);
            if (bFileChanged)
                Text += "*";
        }

        private void SetWorkingFile(string filename)
        {
            strLoadedFileName = filename;
            Text = "SpriteEd " + filename;
            if (bFileChanged)
                Text += "*";
        }

        private void newToolStripMenuItem_Click(object sender, EventArgs e)
        {
            //verifica document precedent
            ResetVars();
            InitModuleView();
            pbDraw.Refresh();

            SetModified(false);
            SetWorkingFile("");
        }

        void ResetVars()
        {
            sprite.images.RemoveRange(0, sprite.images.Count);
            sprite.imgPath.RemoveRange(0, sprite.imgPath.Count);
            sprite = new Sprite();
            module = null;
            frame = null;
            fmodule = null;
            aframe = null;
            anim = null;
            img = null;
            imgIdx = 0;

            g_JSONfilename = "";
            g_JSONframes.Clear();
        }

        #endregion 

        #region MainMenu
        
        private void exportToolStripMenuItem_Click(object sender, EventArgs e)
        {
            ExportWnd exp = new ExportWnd();
            exp.sprite = sprite;
            exp.ShowDialog();
        }

        #endregion        


        //apasarea tastelor e tratata aici, de la toate controalele
        private void SpriteWnd_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.KeyCode == Keys.Home)
            {
                ox = w / 2;
                oy = h / 2;
                pbDraw.Refresh();
                return;
            }

            if (e.KeyCode == Keys.V)
            {
                switch (view)
                {
                    case Views.frameView:
                        if (Control.ModifierKeys == Keys.Shift)
                            VFilpFlop();
                        else
                            VCenterSelectedFModules();
                        break;
                }
            }

            if (e.KeyCode == Keys.H)
            {
                switch (view)
                {
                    case Views.frameView:
                        if (Control.ModifierKeys == Keys.Shift)
                            HFilpFlop();
                        else
                            HCenterSelectedFModules();
                        break;
                }
            }

            if (e.KeyCode == Keys.G)
            {
                showGridToolStripMenuItem_Click(sender, null);
            }

            if (e.KeyCode == Keys.I)
            {
                invertColorsIToolStripMenuItem_Click(sender, null);
            }

            if (e.KeyCode == Keys.C)
            {
                highlightCursorHToolStripMenuItem_Click(sender, null);
            }

            switch (view)
            {
                case Views.moduleView:
                    {
                        //cand dai escape sa te puna pe tool-ul normal
                        if (e.KeyCode == Keys.Escape)
                        {
                            if (tool != Tools.Select)
                            {
                                lastTool = tool;
                                tool = Tools.Select;
                                tsb_modSelect.Checked = true;
                                tsb_modCreate.Checked = false;
                                tsb_modWand.Checked = false;
                                pbDraw.Cursor = Cursors.Default;
                            }
                        }
                        //miscare module
                        if (tool == Tools.Select)
                        {
                            if (e.Control)
                            {
                                if (e.KeyCode == Keys.Left)
                                {
                                    MoveSelectedModules(-1, 0);
                                    pbDraw.Refresh();
                                }
                                else
                                if (e.KeyCode == Keys.Right)
                                {
                                    MoveSelectedModules(1, 0);
                                    pbDraw.Refresh();
                                }
                                else
                                if (e.KeyCode == Keys.Up)
                                {
                                    MoveSelectedModules(0, -1);
                                    pbDraw.Refresh();
                                }
                                else
                                if (e.KeyCode == Keys.Down)
                                {
                                    MoveSelectedModules(0, 1);
                                    pbDraw.Refresh();
                                }
                            }
                            else if (e.Alt)
                            {
                                if (e.KeyCode == Keys.Left)
                                {
                                    ResizeSelectedModules(-1, 0);
                                    pbDraw.Refresh();
                                }
                                else if (e.KeyCode == Keys.Right)
                                {
                                    ResizeSelectedModules(1, 0);
                                    pbDraw.Refresh();
                                }
                                else if (e.KeyCode == Keys.Up)
                                {
                                    ResizeSelectedModules(0, -1);
                                    pbDraw.Refresh();
                                }
                                else if (e.KeyCode == Keys.Down)
                                {
                                    ResizeSelectedModules(0, 1);
                                    pbDraw.Refresh();
                                }
                            }
                            else if (e.KeyCode == Keys.Delete)
                            {
                                DeleteItem(lvMain);
                            }
                        }
                    }
                    break;
                case Views.frameView:
                    {
                        if (e.KeyCode == Keys.Insert)
                        {
                            NewFrame();
                        }

                        if (tool == Tools.Select)
                        {
                            if (sender == lvMain)
                            {
                                if (e.Control)
                                {
                                    if (e.KeyCode == Keys.Up)
                                    {
                                        MoveSelectedFrames(0, -1);
                                        pbDraw.Refresh();
                                    }
                                    else if (e.KeyCode == Keys.Down)
                                    {
                                        MoveSelectedFrames(0, 1);
                                        pbDraw.Refresh();
                                    }
                                    else if (e.KeyCode == Keys.Left)
                                    {
                                        MoveSelectedFrames(-1, 0);
                                        pbDraw.Refresh();
                                    }
                                    else if (e.KeyCode == Keys.Right)
                                    {
                                        MoveSelectedFrames(1, 0);
                                        pbDraw.Refresh();
                                    }
                                }
                                break;
                            }
                        }

                        if (tool == Tools.Select)
                        {
                            if (e.Control)
                            {
                                if (e.KeyCode == Keys.Left)
                                {
                                    MoveSelectedFModules(-1, 0);
                                    pbDraw.Refresh();
                                }
                                else if (e.KeyCode == Keys.Right)
                                {
                                    MoveSelectedFModules(1, 0);
                                    pbDraw.Refresh();
                                }
                                else if (e.KeyCode == Keys.Up)
                                {
                                    MoveSelectedFModules(0, -1);
                                    pbDraw.Refresh();
                                }
                                else if (e.KeyCode == Keys.Down)
                                {
                                    MoveSelectedFModules(0, 1);
                                    pbDraw.Refresh();
                                }
                            }
                            else if (e.KeyCode == Keys.Delete)
                            {
                                if (lvMain.Focused)
                                {
                                    DeleteItem(lvMain);
                                    lvSecond.Items.Clear();
                                    aframe = null;
                                    RemoveSelection(lvMain);
                                    pbDraw.Refresh();
                                }
                                else if (lvSecond.Focused)
                                {
                                    DeleteItem(lvSecond);
                                    fmodule = null;
                                    RemoveSelection(lvSecond);
                                    pbDraw.Refresh();
                                }
                            }
                        }
                        else if (tool == Tools.BBox)
                        {
                            if (frame == null) break;
                            if (e.Control)
                            {
                                if (e.KeyCode == Keys.Up)
                                {
                                    MoveSelectedBBoxes(0, -1);
                                    pbDraw.Refresh();
                                }
                                if (e.KeyCode == Keys.Down)
                                {
                                    MoveSelectedBBoxes(0, 1);
                                    pbDraw.Refresh();
                                }
                                if (e.KeyCode == Keys.Left)
                                {
                                    MoveSelectedBBoxes(-1, 0);
                                    pbDraw.Refresh();
                                }
                                if (e.KeyCode == Keys.Right)
                                {
                                    MoveSelectedBBoxes(1, 0);
                                    pbDraw.Refresh();
                                }
                                //if (e.KeyCode == Keys.Left)
                                //{
                                //    EditFrameBBox(frame, -1, 0, 0, 0);
                                //    pbDraw.Refresh();
                                //}
                                //else if (e.KeyCode == Keys.Right)
                                //{
                                //    EditFrameBBox(frame, 1, 0, 0, 0);
                                //    pbDraw.Refresh();
                                //}
                                //else if (e.KeyCode == Keys.Up)
                                //{
                                //    EditFrameBBox(frame, 0, -1, 0, 0);
                                //    pbDraw.Refresh();
                                //}
                                //else if (e.KeyCode == Keys.Down)
                                //{
                                //    EditFrameBBox(frame, 0, 1, 0, 0);
                                //    pbDraw.Refresh();
                                //}
                            }
                            else if (e.Alt)
                            {
                                if (e.KeyCode == Keys.Up)
                                {
                                    ResizeSelectedBBoxes(0, -1);
                                    pbDraw.Refresh();
                                }
                                if (e.KeyCode == Keys.Down)
                                {
                                    ResizeSelectedBBoxes(0, 1);
                                    pbDraw.Refresh();
                                }
                                if (e.KeyCode == Keys.Left)
                                {
                                    ResizeSelectedBBoxes(-1, 0);
                                    pbDraw.Refresh();
                                }
                                if (e.KeyCode == Keys.Right)
                                {
                                    ResizeSelectedBBoxes(1, 0);
                                    pbDraw.Refresh();
                                }
                                //if (e.KeyCode == Keys.Left)
                                //{
                                //    EditFrameBBox(frame, 0, 0, -1, 0);
                                //    pbDraw.Refresh();
                                //}
                                //else if (e.KeyCode == Keys.Right)
                                //{
                                //    EditFrameBBox(frame, 0, 0, 1, 0);
                                //    pbDraw.Refresh();
                                //}
                                //else if (e.KeyCode == Keys.Up)
                                //{
                                //    EditFrameBBox(frame, 0, 0, 0, -1);
                                //    pbDraw.Refresh();
                                //}
                                //else if (e.KeyCode == Keys.Down)
                                //{
                                //    EditFrameBBox(frame, 0, 0, 0, 1);
                                //    pbDraw.Refresh();
                                //}
                            }
                        }
                        else if (tool == Tools.HitPoints)
                        {
                            if (e.KeyCode == Keys.Delete)
                            {
                                if((frame != null) && (selectedHitPt >= 0)&&(selectedHitPt < frame.hitPoints.Count))
                                {
                                    HitPoint selpt = frame.hitPoints[selectedHitPt];
                                    frame.hitPoints.Remove(selpt);
                                    selectedHitPt = -1;

                                    lvMain.Items[frame.index].SubItems[3].Text = frame.hitPoints.Count.ToString();

                                    pbDraw.Refresh();
                                }
                            }
                        }
                    }
                    break;
                case Views.animView:
                    {
                        if (e.KeyCode == Keys.Insert)
                        {
                            NewAnim();
                            pbDraw.Refresh();
                        }
                        if (e.Control)
                        {
                            if (e.KeyCode == Keys.Left)
                            {
                                MoveSelectedAFrames(-1, 0);
                                pbDraw.Refresh();
                            }
                            else if (e.KeyCode == Keys.Right)
                            {
                                MoveSelectedAFrames(1, 0);
                                pbDraw.Refresh();
                            }
                            else if (e.KeyCode == Keys.Up)
                            {
                                MoveSelectedAFrames(0, -1);
                                pbDraw.Refresh();
                            }
                            else if (e.KeyCode == Keys.Down)
                            {
                                MoveSelectedAFrames(0, 1);
                                pbDraw.Refresh();
                            }
                        }
                        else if (e.KeyCode == Keys.Delete)
                        {
                            if (lvMain.Focused)
                            {
                                DeleteItem(lvMain);
                                lvSecond.Items.Clear();
                                aframe = null;
                                RemoveSelection(lvMain);
                                anim = null;
                                pbDraw.Refresh();
                            }
                            else if (lvSecond.Focused)
                            {
                                DeleteItem(lvSecond);
                                fmodule = null;
                                RemoveSelection(lvSecond);
                                aframe = null;
                                pbDraw.Refresh();
                            }
                        }
                    }
                    break;
            }

        }

        private void ResizeSelectedBBoxes(int dx, int dy)
        {
            foreach(ListViewItem lvi in lvMain.SelectedItems)
            {
                Frame fr = sprite.GetFrameByIndex(lvi.Index);

                fr.BBox.Width += dx;
                fr.BBox.Height += dy;

                if (fr.BBox.Width < 0)
                    fr.BBox.Width = 0;
                if (fr.BBox.Height < 0)
                    fr.BBox.Height = 0;

                string collrect = fr.BBox.X + "," + fr.BBox.Y + ";" + fr.BBox.Width + "," + fr.BBox.Height;
                lvMain.Items[fr.index].SubItems[2].Text = collrect;
            }
        }

        private void MoveSelectedBBoxes(int dx, int dy)
        {
            foreach (ListViewItem lvi in lvMain.SelectedItems)
            {
                Frame fr = sprite.GetFrameByIndex(lvi.Index);

                fr.BBox.X += dx;
                fr.BBox.Y += dy;

                if (fr.BBox.Width < 0)
                    fr.BBox.Width = 0;
                if (fr.BBox.Height < 0)
                    fr.BBox.Height = 0;

                string collrect = fr.BBox.X + "," + fr.BBox.Y + ";" + fr.BBox.Width + "," + fr.BBox.Height;
                lvMain.Items[fr.index].SubItems[2].Text = collrect;
            }
        }

        private void pThumbs_MouseDown(object sender, MouseEventArgs e)
        {
            if (view == Views.moduleView)
            {
                cmThumbs.Show(pThumbs, e.Location);
            }
        }

        //--- tratare meniu popup pt liste ---
        private void cmLists_Click(object sender, EventArgs e)
        {
            string name = (sender as ToolStripItem).Name;
            switch (name)
            {
                //--- frames ---
                case "GoToModule":
                    GoToSelectedModule();
                    break;

                case "ToggleLoopAnim":
                    {
                        foreach (ListViewItem lvi in lvMain.SelectedItems)
                        {
                            Animation anm = sprite.GetAnimByIndex(lvi.Index);
                            anm.flags = anm.flags ^ 1;
                        }
                        PopulateAnimList(lvMain.SelectedItems[0].Index);
                        pbDraw.Refresh();
                    }
                    break;
                case "ClearLoopAnim":
                    {
                        foreach (ListViewItem lvi in lvMain.SelectedItems)
                        {
                            Animation anm = sprite.GetAnimByIndex(lvi.Index);
                            if((anm.flags & 1) != 0)
                            {
                                anm.flags = anm.flags ^ 1;
                            }
                        }
                        PopulateAnimList(lvMain.SelectedItems[0].Index);
                        pbDraw.Refresh();
                    }
                    break;
                case "DeleteAnimation":
                    {
                        DeleteItem(lvMain);
                    }
                    break;
                case "CloneAnimation":
                    {
                        CloneSelectedAnimations();
                    }
                    break;
                case "CloneAFrame":
                    {
                        CloneSelectedAFrames();
                    }
                    break;
                case "GoToFrame":
                    {
                        GoToSelectedAFrame_Frame();
                    }
                    break;
                case "DeleteAFrame":
                    {
                        DeleteItem(lvSecond);
                    }
                    break;
                //--- modules ---
                case "InsertModule":
                    {
                        NewModule(imgIdx, 0, 0, 10, 10);
                        pbDraw.Refresh();
                    }
                    break;
                case "CloneModule":
                    {
                        CloneModule();
                    }
                    break;
                case "DeleteModule":
                    {
                        DeleteItem(lvMain);
                    }
                    break;
                // --- frames ----
                case "InsertFrame":
                    {
                        NewFrame();
                        pbDraw.Refresh();
                    }
                    break;
                case "CloneFrame":
                    {
                        CloneSelectedFrames();
                    }
                    break;
                case "DeleteFrame":
                    {
                        DeleteItem(lvMain);
                        frame = null;
                        RemoveSelection(lvMain);
                        pbDraw.Refresh();
                    }
                    break;
                case "FrameComputeBBox":
                    {
                        foreach (ListViewItem lvi in lvMain.SelectedItems)
                        {
                            Frame frm = sprite.GetFrameByIndex(lvi.Index);
                            frm.BBox = frm.GetRect();

                            string collrect = frm.BBox.X + "," + frm.BBox.Y + ";" + frm.BBox.Width + "," + frm.BBox.Height;
                            lvi.SubItems[2].Text = collrect;

                        }
                        pbDraw.Refresh();
                    }
                    break;
                case "FlipFrameX":
                    {
                        foreach (ListViewItem lvi in lvMain.SelectedItems)
                        {
                            Frame frm = sprite.GetFrameByIndex(lvi.Index);

                            foreach (FrameModule fm in frm.fmodules)
                            {
                                fm.ox = -fm.ox - fm.module.w;
                                fm.flags = fm.flags ^ 1;
                            }
                        }
                        PopulateFModulesList();
                        pbDraw.Refresh();
                    }
                    break;
                case "FlipFrameY":
                    {
                        foreach (ListViewItem lvi in lvMain.SelectedItems)
                        {
                            Frame frm = sprite.GetFrameByIndex(lvi.Index);
                            foreach (FrameModule fm in frm.fmodules)
                            {
                                fm.oy = -fm.oy - fm.module.h;
                                fm.flags = fm.flags ^ 2;
                            }
                        }

                        PopulateFModulesList();
                        pbDraw.Refresh();
                    }
                    break;
                //--- fmodules ---
                case "CloneFModules":
                    {
                        CloneSelectedFModules();
                    }
                    break;
                case "DeleteFModule":
                    {
                        DeleteItem(lvSecond);
                        fmodule = null;
                        RemoveSelection(lvSecond);
                        pbDraw.Refresh();
                    }
                    break;
                case "FlipFModuleX":
                    {
                        foreach (ListViewItem lvi in lvSecond.SelectedItems)
                        {
                            FrameModule fm = frame.GetFModuleByIndex(lvi.Index);
                            fm.flags = fm.flags ^ 1;
                        }
                        PopulateFModulesList();
                        pbDraw.Refresh();
                    }
                    break;
                case "FlipFModuleY":
                    {
                        foreach (ListViewItem lvi in lvSecond.SelectedItems)
                        {
                            FrameModule fm = frame.GetFModuleByIndex(lvi.Index);
                            fm.flags = fm.flags ^ 2;
                        }
                        PopulateFModulesList();
                        pbDraw.Refresh();
                    }
                    break;
                case "ClearFModuleFlags":
                    {
                        foreach (ListViewItem lvi in lvSecond.SelectedItems)
                        {
                            FrameModule fm = frame.GetFModuleByIndex(lvi.Index);
                            fm.flags = 0;
                        }
                        PopulateFModulesList();
                        pbDraw.Refresh();
                    }
                    break;
                //--- no action fallback ---
                default:
                    {
                        MessageBox.Show("No action defined for: " + name);
                    }
                    break;
            }
        }

                      
        //--- tratare meniu popup pt click pe imaginile din bara de jos ---
        private void cmImages_Click(object sender, EventArgs e)
        {
            ToolStripItem pItem = (sender as ToolStripItem);
            string name = pItem.Name;
            switch (name)
            {
                //--- frame view ---
                case "ShowFlipXMods":
                    {
                        showFlipXMods = !showFlipXMods;
                        pThumbs.Refresh();
                    }
                    break;
                case "ShowFlipYMods":
                    {
                        showFlipYMods = !showFlipYMods;
                        pThumbs.Refresh();
                    }
                    break;
                case "AddOneModulePerFrame":
                    {
                        int idx = pItem.ImageIndex;
                        for (int kk = idx; kk < sprite.modules.Count; kk++)
                        {
                            Module mod = sprite.GetModuleByIndex(kk);
                            Frame nfrm = sprite.AddFrame();
                            nfrm.index = lvMain.Items.Count;
                            FrameModule nfm = nfrm.AddFModule(mod, 0);
                            nfm.ox = 0;
                            nfm.oy = 0;
                            nfm.index = 0;
                            nfm.flags = 0;
                            AddFrame(nfrm, true);
                        }
                    }
                    break;
                //--- module View ---
                case "LoadImage":
                    {
                        LoadImageOverIDX(imgIdx);
                    }
                    break;
                case "SaveImageAs":
                    {
                        SaveImageAs(img);                       
                    }
                    break;
                case "CreateFlipXModules":
                    {
                        CreateMirroredImageAndModules_X();
                    }
                    break;
                case "CreateFlipYModules":
                    {
                        CreateMirroredImageAndModules_Y();
                    }
                    break;
                case "MoveModulesHere":
                    {
                        if (view == Views.moduleView)
                        {
                            //seteaza imageID la modulele selectate
                            foreach (ListViewItem lvi in lvMain.SelectedItems)
                            {
                                sprite.modules[lvi.Index].imageID = imgIdx;
                                lvi.SubItems[1].Text = imgIdx.ToString();
                            }
                            pbDraw.Refresh();
                        }
                    }
                    break;
                case "RemoveImage":
                    {
                        DialogResult dr = MessageBox.Show(this, "This option will remove the selected image from the list and set the image modules->imgID on -1.\nYou can move modules from an image to another by selecting them and selecting \"Move Modules Here\" from the new image context menu.\nThis operation cannot be undone !\n\nAre you sure you want to continue ?", "Remove Image Warning", MessageBoxButtons.YesNo);
                        if (dr != DialogResult.Yes) return;

                        foreach (Module md in sprite.modules)
                        {
                            if (md.imageID > imgIdx)
                            {
                                md.imageID--;
                            }
                            else if (md.imageID == imgIdx)
                            {
                                md.imageID = -1;
                            }
                        }
                        //scoate imaginea din lista 
                        sprite.images.RemoveAt(imgIdx);
                        sprite.imgPath.RemoveAt(imgIdx);

                        sprite.MakeModuleImages();
                        AddThumbs(sprite.images.Count);
                        populateModuleList();
                        //muta cursourl de selectie pe poza anterioara
                        imgIdx--;
                        if (imgIdx >= 0)
                        {
                            img = sprite.images[imgIdx];
                        }
                        else
                        {
                            img = null;
                        }

                        pbDraw.Refresh();
                    }
                    break;
                default:
                    {
                        MessageBox.Show("No action defined !", "WARNING!", MessageBoxButtons.OK, MessageBoxIcon.Information);
                    }
                    break;
            }
        }

        //ca sa inchida fereastra
        private void exitToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (bFileChanged)
            {
                DialogResult dr = MessageBox.Show(this, "You have unsaved changes! \nAre you sure you want to discard changes ?", "Discard Changes", MessageBoxButtons.YesNo);
                if (dr != DialogResult.Yes) return;

            }

            this.Dispose();
        }


        //scrie in structura de transport toate valorile editabile din fereastra de optiuni
        ViewOptionsWnd.ViewOptionsWndParams CompactChanges()
        {
            ViewOptionsWnd.ViewOptionsWndParams wndparams = new ViewOptionsWnd.ViewOptionsWndParams();
            wndparams.showAnimPath = showAnimationPath;
            wndparams.wandTreshold = wandTreshold;
            wndparams.showLinkedHitpts = showLinkedHitpts;

            wndparams.gridSize = gridSize;
            wndparams.majorGridSize = majorGridLines;

            wndparams.bSaveNonIndentedJSON = bExportSmallJSON;

            return wndparams;
        }
        //citeste din structura de transport datele si le pune in engine
        void UpdateChanges(ViewOptionsWnd.ViewOptionsWndParams wndparams)
        {
            showAnimationPath = wndparams.showAnimPath;
            wandTreshold = wndparams.wandTreshold;
            timer1.Interval = TIMER_UPDATE_PERIOD;
            showLinkedHitpts = wndparams.showLinkedHitpts;

            majorGridLines = wndparams.majorGridSize;
            gridSize = wndparams.gridSize;

            bExportSmallJSON = wndparams.bSaveNonIndentedJSON;
        }

        //callbackul unde se proceseaza toate mesajele directe primite din fereastra de optiuni si alte ferestre
        private void OptionsWindowCallback(CallbackCommands command, Object parms)
        {
            switch (command)
            {
                case CallbackCommands.UpdateParams:
                    {
                        UpdateChanges(parms as ViewOptionsWnd.ViewOptionsWndParams);

                        viewoptionsWnd.Hide();
                        viewoptionsWnd.Dispose();
                        viewoptionsWnd = null;

                        pbDraw.Refresh();
                    }
                    break;
                case CallbackCommands.UpdateHitpoint:
                    {
                        if ((frame != null) && (selectedHitPt >= 0) && (selectedHitPt < frame.hitPoints.Count))
                        {
                            HitPoint nhp = parms as HitPoint;
                            frame.hitPoints[selectedHitPt].X = nhp.X;
                            frame.hitPoints[selectedHitPt].Y = nhp.Y;
                            frame.hitPoints[selectedHitPt].flags = nhp.flags;

                            pbDraw.Refresh();
                        }

                        hitptWnd.Hide();
                        hitptWnd.Dispose();
                        hitptWnd = null;
                    }
                    break;
                default:
                    {
                        MessageBox.Show("Callback NETRATAT!\n" + command.ToString());
                    }
                    break;
            }
        }

        //click meniu showgrid
        private void showGridToolStripMenuItem_Click(object sender, EventArgs e)
        {
            showGrid = !showGrid;
            showGridGToolStripMenuItem.Checked = showGrid;
            pbDraw.Refresh();
        }

        private void setInterfaceColors()
        {
            //light theme
            if (invertGrid)
            {
                colClear = Color.FromArgb(255, 200, 200, 200);
                colGridS = Color.FromArgb(255, 170, 170, 170);
                colGridM = Color.FromArgb(255, 140, 140, 140);
                colOrigin = Color.FromArgb(255, 33, 33, 33);

                colThumbsBack = Color.FromArgb(255, 200, 200, 200);
                colThumbsSelection = Color.DarkOrange;
            }
            else //dark theme
            {
                colClear = Color.FromArgb(255, 43, 43, 43);
                colGridS = Color.FromArgb(255, 54, 54, 54);
                colGridM = Color.FromArgb(255, 74, 74, 74);
                colOrigin = Color.FromArgb(255, 145, 145, 145);

                colThumbsBack = Color.FromArgb(255, 43, 43, 43);
                colThumbsSelection = Color.DarkOrange;
            }
        }

        private void invertColorsIToolStripMenuItem_Click(object sender, EventArgs e)
        {
            invertGrid = !invertGrid;

            setInterfaceColors();

            if (pbThumbs != null)
            {
                foreach (PictureBox pbi in pbThumbs)
                    pbi.BackColor = colThumbsBack;
            }

            invertColorsIToolStripMenuItem.Checked = invertGrid;
            pbDraw.Refresh();
        }

        private void flagsBuilderToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if ((flagsWnd == null) || (flagsWnd.IsDisposed))
            {
                flagsWnd = new FlagsBuilderWnd();
                flagsWnd.TopMost = true;
                flagsWnd.Show();
            }
        }

        private void flipAnimationsToolStripMenuItem_Click(object sender, EventArgs e)
        {
            CreateMirroredAnimations_X();
        }

        private void transformModulesToolStripMenuItem_Click(object sender, EventArgs e)
        {
            ScaleAnimations(1.25f);           
        }

        private void scaleAllBy625ToolStripMenuItem_Click(object sender, EventArgs e)
        {
            ScaleAnimations(0.625f);           
        }

        private void optionsToolStripMenuItem1_Click(object sender, EventArgs e)
        {
            ViewOptionsWnd.ViewOptionsWndParams wndParams = CompactChanges();

            if (viewoptionsWnd == null)
            {
                viewoptionsWnd = new ViewOptionsWnd(OptionsWindowCallback, sprite, wndParams);
                viewoptionsWnd.Show();
            }
        }

        #region TOOLSTRIP_BUTTONS
        enum Tools { Select, Pan, Zoom, Module, Resize, Move, Wand, Relocate, BBox, HitPoints, BBox_Move, BBox_Resize, BBox_Relocate };
        Tools tool = Tools.Select;
        Tools lastTool = Tools.Select;

        enum Views { moduleView, frameView, animView };

        private void modulesFromPixiJSJSONToolStripMenuItem_Click(object sender, EventArgs e)
        {
            OpenFileDialog ofd = new OpenFileDialog()
            {
                AutoUpgradeEnabled = false
            };

            ofd.Filter = "Pixi graphics JSON file (*.json)|*.json";
            if (ofd.ShowDialog() == DialogResult.Cancel)
                return;

            ImportModulesFromTexpackerJSON(ofd.FileName);

            ts_Modules_Click(null, null);
            pbDraw.Refresh();
        }

        private void exportJSONDescriptorToolStripMenuItem_Click(object sender, EventArgs e)
        {
            SaveFileDialog sfd = new SaveFileDialog();
            sfd.Filter = "PixiJS animation descriptor JSON (*.json)|*.json|All Files (*.*)|*.*";
            sfd.DefaultExt = ".json";
            sfd.FileName = "DESC_" + Path.GetFileNameWithoutExtension(strLoadedFileName);
            if (sfd.ShowDialog() == DialogResult.Cancel)
                return;

            if (File.Exists(sfd.FileName))
                File.Delete(sfd.FileName);

            try
            {
                CJSONexportFile nfile = GetJSONfromEditor();
                //export json now, formatted or not
                if (bExportSmallJSON)
                    File.WriteAllText(sfd.FileName, JsonConvert.SerializeObject(nfile, Formatting.None));
                else
                    File.WriteAllText(sfd.FileName, JsonConvert.SerializeObject(nfile, Formatting.Indented));
            }
            catch (Exception ex)
            {
                MessageBox.Show("Couldn't save JSON project! " + ex.Message, "Warning!", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
            }
        }

        private void asepriteJSONTilemapToolStripMenuItem_Click(object sender, EventArgs e)
        {
            OpenFileDialog ofd = new OpenFileDialog()
            {
                AutoUpgradeEnabled = false
            };

            ofd.Filter = "Aseprite JSON file (*.json)|*.json";
            if (ofd.ShowDialog() == DialogResult.Cancel)
                return;


            InkEditor.AsepriteImport aseImporter = new InkEditor.AsepriteImport();
            if (aseImporter.ImportModulesFromJSON(ofd.FileName))
            {
                // use data to create modules and frames
                // remove modules ONLY
                sprite.images.RemoveRange(0, sprite.images.Count);
                sprite.imgPath.RemoveRange(0, sprite.imgPath.Count);
                sprite.modules.RemoveRange(0, sprite.modules.Count);
                sprite.frames.Clear();
                sprite.anims.Clear();
                img = null;
                imgIdx = 0;
                module = null;
                // switch to module view
                ts_Modules_Click(null, null);

                ///--- load image ---
                sprite.imgPath.Add(aseImporter.strImgName);
                Image limg = null;
                using (var bmpTemp = new Bitmap(aseImporter.strImgPath))
                {
                    limg = new Bitmap(bmpTemp);
                }
                sprite.images.Add(limg);

                ///--- add modules ---
                for (int ll = 0; ll < aseImporter.arrUniqueModules.Count; ll++)
                {
                    Rectangle rec = aseImporter.arrUniqueModules[ll];
                    // add modules
                    Module mod = new Module();
                    mod.index = Convert.ToInt32(ll);
                    mod.ID = Convert.ToInt32(ll);
                    mod.imageID = 0; //only using one image so default to 0
                    mod.x = rec.X;
                    mod.y = rec.Y;
                    mod.w = rec.Width;
                    mod.h = rec.Height;
                    mod.strName = "";
                    sprite.modules.Add(mod);
                }

                ///--- add frames ---
                int frmidx = 0;
                int animidx = 0;
                foreach (var tag in aseImporter.mapTags)
                {
                    if (tag.Value.framesCount <= 0)
                        continue;

                    // add animation so we have the pointer here
                    Animation nanimation = sprite.AddAnim();
                    nanimation.index = animidx++;
                    nanimation.name = tag.Key.ToUpper();
                    AddAnim(nanimation, true);
                    int aframe_idx = 0;

                    for (int kk = 0; kk < tag.Value.framesCount; kk++)
                    {
                        Frame f = sprite.AddFrame();
                        f.index = frmidx;

                        InkEditor.AsepriteFrame[] arrframes = aseImporter.GetTagFrameLayers(tag.Key, kk);

                        int nfm_idx = 0;
                        int nfm_maxduration = 1;
                        for (int ii = 0; ii < aseImporter.arrLayers.Count; ii++)
                        {
                            if (arrframes[ii] == null)
                                continue;
                            var asef = arrframes[ii];
                            if (asef.duration > nfm_maxduration)
                                nfm_maxduration = asef.duration;
                            
                            Module fmd = sprite.modules[asef.uniqueModuleIndex];
                            // compute flags: write layer number avoiding flip flags 2b
                            int fmd_flags = (ii + 1) << 2;
                            FrameModule nfm = f.AddFModule(fmd, fmd_flags);
                            nfm.ox = asef.posX;
                            nfm.oy = asef.posY;
                            nfm.index = nfm_idx++;
                            nfm.flags = fmd_flags;
                        }

                        //frame = f;
                        AddFrame(f, true);
                        AnimFrame addedaframe = nanimation.AddAFrame(f);
                        addedaframe.time = nfm_maxduration;
                        addedaframe.index = aframe_idx++;

                        frmidx++;
                    }
                }

                ///--- LAST: refresh view ---
                if (sprite.images.Count != 0)
                {
                    img = sprite.images[0];
                    imgIdx = 0;
                }

                InitModuleView();
                pbDraw.Refresh();
            }
        }

        private void editFramesToolStripMenuItem_Click(object sender, EventArgs e)
        {

        }

        private void saveToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (strLoadedFileName == "")
            {
                saveAsToolStripMenuItem_Click(sender, e);
            }
            else
            {
                sprite.Save(strLoadedFileName);
                SetModified(false);
            }
        }

        private void aboutToolStripMenuItem_Click_1(object sender, EventArgs e)
        {
            MessageBox.Show("SpriteEd v1.0.5 04.apr.2025", "About");
        }

        Views view = Views.moduleView; //view-ul curent

        void SaveViewData()
        {
            switch (view)
            {
                case Views.moduleView:
                    scaleM = scale;
                    oxM = ox;
                    oyM = oy;
                    break;
                case Views.frameView:
                    scaleF = scale;
                    oxF = ox;
                    oyF = oy;
                    break;
                case Views.animView:
                    scaleA = scale;
                    oxA = ox;
                    oyA = oy;
                    break;
            }
        }
        
        private void ts_Modules_Click(object sender, EventArgs e)
        {
            SaveViewData();
            view = Views.moduleView;
            InitModuleView();
            ts_Modules.Checked = true;
            ts_Frames.Checked = false;
            ts_Animations.Checked = false;

            ToolStripAnims.Hide();
            ToolStripFrames.Hide();
            ToolStripModules.Show();

            pbDraw.Refresh();
        }

        private void ts_Frames_Click(object sender, EventArgs e)
        {
            SaveViewData();
            view = Views.frameView;
            InitFrameView();
            ts_Modules.Checked = false;
            ts_Frames.Checked = true;
            ts_Animations.Checked = false;

            tool = Tools.Select;
            pbDraw.Cursor = Cursors.Default;

            ToolStripAnims.Hide();
            ToolStripFrames.Show();
            ToolStripModules.Hide();

            pbDraw.Refresh();
        }

        private void ts_Animations_Click(object sender, EventArgs e)
        {
            SaveViewData();
            view = Views.animView;
            InitAnimView();
            ts_Modules.Checked = false;
            ts_Frames.Checked = false;
            ts_Animations.Checked = true;

            ToolStripAnims.Show();
            ToolStripFrames.Hide();
            ToolStripModules.Hide();

            pbDraw.Refresh();

        }

        private void tsb_frmSelect_Click(object sender, EventArgs e)
        {
            lastTool = tool;
            tool = Tools.Select;
            selectedHitPt = -1;

            tsb_frmSelect.Checked = true;
            tsb_frmBBox.Checked = false;
            tsb_frmPoints.Checked = false;

            pbDraw.Refresh();
        }

        private void tsb_frmBBox_Click(object sender, EventArgs e)
        {
            lastTool = tool;
            tool = Tools.BBox;

            tsb_frmSelect.Checked = false;
            tsb_frmBBox.Checked = true;
            tsb_frmPoints.Checked = false;

            pbDraw.Refresh();

        }

        private void tsb_frmPoints_Click(object sender, EventArgs e)
        {
            tool = Tools.HitPoints;

            tsb_frmSelect.Checked = false;
            tsb_frmBBox.Checked = false;
            tsb_frmPoints.Checked = true;

            pbDraw.Refresh();
        }

        private void tsb_anmPlay_Click(object sender, EventArgs e)
        {
            AnimPlaying = !AnimPlaying;
            tsb_anmPlay.Checked = AnimPlaying;
            if (AnimPlaying)
            {
                currentAnimPreviewIdx = 0;
                if (anim.aframes.Count > 0)
                    currentAnimPreviewDuration = anim.GetAFrameByIndex(0).time;
                else
                    currentAnimPreviewDuration = TIMER_UPDATE_PERIOD;

                timer1.Interval = TIMER_UPDATE_PERIOD;
            }
            timer1.Enabled = AnimPlaying;
        }

        private void tsb_modSelect_Click(object sender, EventArgs e)
        {
            tool = Tools.Select;
            pbDraw.Cursor = Cursors.Default;
            tsb_modSelect.Checked = true;
            tsb_modCreate.Checked = false;
            tsb_modWand.Checked = false;
        }

        private void tsb_modCreate_Click(object sender, EventArgs e)
        {
            tool = Tools.Module;
            tsb_modSelect.Checked = false;
            tsb_modCreate.Checked = true;
            tsb_modWand.Checked = false;
        }

        private void tsb_modWand_Click(object sender, EventArgs e)
        {
            tool = Tools.Wand;
            tsb_modSelect.Checked = false;
            tsb_modCreate.Checked = false;
            tsb_modWand.Checked = true;
        }
 
        #endregion

        private void editModulesToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if ((imgIdx >= 0) && (imgIdx < sprite.images.Count))
            {
                lvMain.BeginUpdate();
                int found = DetectModules(imgIdx, (byte)wandTreshold);
                lvMain.EndUpdate();

                if(sprite.modules.Count > 0)
                    SelectModule(0);

                RefreshDrawArea();

                MessageBox.Show("Found " + found + " modules.", "Detect Modules Finished!", MessageBoxButtons.OK, MessageBoxIcon.Information);
            }
        }

        private void highlightCursorHToolStripMenuItem_Click(object sender, EventArgs e)
        {
            cursorHighlight = !cursorHighlight;
            highlightCursorHToolStripMenuItem.Checked = cursorHighlight;
            pbDraw.Refresh();
        }

        private void enableXRayToolStripMenuItem_Click(object sender, EventArgs e)
        {
            showAnimXRay = enableXRayToolStripMenuItem.Checked;
            pbDraw.Refresh();
        }

        private void fontHeaderToolStripMenuItem_Click(object sender, EventArgs e)
        {
            sprite.MakeModuleImages();
            InkEditor.FontWnd fwnd = new InkEditor.FontWnd();
            fwnd.spr = this.sprite;
            fwnd.Show();
        }
    }
}