using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Drawing.Drawing2D;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.IO;
using System.Collections;
using System.Diagnostics;
using System.Xml;

namespace HexxEditor
{
    public partial class Form1 : Form
    {
        //helper forms
        public MaterialsWnd g_wndMaterials;
        public LightsWnd g_wndLights;
        public CollisionWnd g_wndCollisions;
        public ObjectsWnd g_wndObjects;
        public PrefabsWnd g_wndPrefabs;
        public AIwnd g_wndAI;
        public MiscWnd g_wndMisc;
        public ActorsWnd g_wndActors;

        //save file version
        public const int K_CURRENT_VERSION = 1015;

        //mission types
        public const byte K_MISSION_TYPE_ELIMINATE_ALL = 0;
        public const byte K_MISSION_TYPE_SAVE_HOSTAGES = 1;
        public const byte K_MISSION_TYPE_DEFUSE_BOMB = 2;
        public const byte K_MISSION_TYPE_ARREST_WARRANT = 3;

        //layers
        /*
        public const int K_LAYER_BACK = 0;
        public const int K_LAYER_MID = 1;
        public const int K_LAYER_FRONT = 2;
        public const int K_LAYERS_CNT = 3;
        */

        enum ELayer
        {
            UNDER_FLOOR = 0,
            FLOOR = 1,
            FLOOR_DECO1,
            FLOOR_DECO2,
            WALLS,
            CEILING_DECO,
            CEILING,

            LAYERS_CNT
        };

        // directii generice
        public const int K_DIR_NONE = 0;
        public const int K_DIR_LEFT = 1;
        public const int K_DIR_UP = 2;
        public const int K_DIR_RIGHT = 3;
        public const int K_DIR_DOWN = 4;

        //constante setate in fereastra de tileset
        public int TILE_WIDTH = 0;
        public int TILE_HWIDTH = 0;
        public int TILE_HEIGHT = 0;
        public int TILE_HHEIGHT = 0;

        //constante
        public const int K_SKY_TILES_ADDED = 0; //cam un ecran pe verticala
        public const int K_LIGHT_DEFAULT_RADIUS = 64; //marimea luminii cand o adaugi
        public const int K_CORNER_SIZE = 8; //marimea patratelelor pt scalare
        public const float K_LIGHT_DEFAULT_FSCALING = 0.6f; //scalarea zonei luminii in fn de marimea animatiei
        //colturi scalare - flaguri cu ce poti modifica. Daca sunt toate setate inseamna ca trebuie mutat
        public const int K_SCALE_FLAG_X = 1;
        public const int K_SCALE_FLAG_Y = 2;
        public const int K_SCALE_FLAG_W = 4;
        public const int K_SCALE_FLAG_H = 8;
        public const int K_SCALE_FLAG_MOVE = 15;
        //global snap threshold
        public const int K_SNAP_THRESHOLD = 3;
        public const int K_SNAP_THRESHOLD_COLLISIONS = 5;
        //undo stream for saving the level quickly
        MemoryStream UndoMemStream = null;
        //undo types
        public const int K_UNDO_DISABLED = 0;
        public const int K_UNDO_TILES = 1;
        public const int K_UNDO_PREFAB = 2;

        public int gUndoStatus = K_UNDO_DISABLED;

        //coordonate camera
        public PointF CAMERA_ORIGIN; //in pixeli
        public Point LEVEL_OFFSET; //de unde incepe de fapt nivelul in editor, in tiles
        public PointF cameraPos;

        PointF g_lastSearchPos = new PointF(0.0f, 0.0f); //pozitia ultimului ID cautat sau 0.0f,0.0f daca nu am cautat nimic
        int g_lastSearchID = -1;
        int g_lastSearchBrush = -1;

        public float zoomLevel;
        public bool invertBackground = false;
        public bool g_bCreatingItem = false; //daca creez un item care nu se face dintr-un click (collbox)
        public bool g_bDraggingItem = false; //daca trag de un obiect
        public UInt16 g_nScalingItemFlags = 0;  //daca scalez un obiect aici se salveaza flagurile elementelor scalate

        private string g_strFilePath = "";
        private bool g_bFileModified = false;
        public void SetFileModified()
        {
            g_bFileModified = true;
            if (this.Text[this.Text.Length - 1] != '*')
                this.Text = this.Text + "*";
        }
        public string GetFilePath()
        {
            return g_strFilePath;
        }

        // Returns the "media" folder absolute path
        public string GetMediaFolderAbsolutePath()
        {
            if (g_strFilePath.Length == 0)
                return g_strFilePath;

            //remove last 3 files and folders (media folder structure is hardcoded)
            string strMediaPath = Path.GetDirectoryName(g_strFilePath);
            strMediaPath = Path.GetDirectoryName(strMediaPath);
            strMediaPath = Path.GetDirectoryName(strMediaPath);

            return strMediaPath;
        }
        //gives file path relative to specified folder
        public string GetRelativePath(string filespec, string folder)
        {
            Uri pathUri = new Uri(filespec);
            // Folders must end in a slash
            if (!folder.EndsWith(Path.DirectorySeparatorChar.ToString()))
            {
                folder += Path.DirectorySeparatorChar;
            }
            Uri folderUri = new Uri(folder);
            return Uri.UnescapeDataString(folderUri.MakeRelativeUri(pathUri).ToString().Replace('/', Path.DirectorySeparatorChar));
        }

        /// <summary>
        /// Create the folder if not existing for a full file name
        /// </summary>
        /// <param name="filename">full path of the file</param>
        public void CreateFolderIfNeeded(string filename)
        {
            string folder = System.IO.Path.GetDirectoryName(filename);
            if (!System.IO.Directory.Exists(folder))
            {
                System.IO.Directory.CreateDirectory(folder);
            }
        }

        public string GetExePath()
        {
            //find output path
            String strExePath = Path.GetDirectoryName(System.Reflection.Assembly.GetExecutingAssembly().GetName().CodeBase);
            strExePath = strExePath.Substring(6); //remove 'file://' prefix
            return strExePath;
        }

        public int g_selectedLayer = (int)ELayer.FLOOR;
        public byte g_missionType = K_MISSION_TYPE_ELIMINATE_ALL;
        CheckBox[] layers_checkboxes;
        RadioButton[] layers_radios;
        //picturebox data
        Image pbImg;
        Graphics pbGr;

        Image tilesImg = null; //imaginea cu elementele de grafica gen iconuri si alte chestii folosite in joc

        public Point g_hoveredTile = new Point(0, 0);
        public Point g_lastHoveredTile = new Point(0, 0);
        public PointF g_lastHoveredPoint = new Point(0, 0);
        public Point g_lastMousePos = new Point(0, 0);
        public Point g_lastMouseDownPos = new Point(0, 0);
        //--- POINTERI LA SPRITES din alte ferestre ---
        public BSXAnimBrowser.SpriteLoader g_sprActors = null;
        public BSXAnimBrowser.SpriteLoader g_sprObjects = null;
        public BSXAnimBrowser.SpriteLoader g_sprLights = null;

        //--- ID-feeder ---
        private UInt32 g_nLastID = 0; //ultimul id folosit - don't set manually
        public UInt32 GetUniqueID()
        {
            g_nLastID++;
            return g_nLastID;
        }

        //--- clasa container pt behavior ---
        public class CBehaviorContainer
        {
            public int targetID;
            public string strActions;
            public string strAIname;
            public ArrayList listAIparams; //lista de strings de forma [nume param][val param][nume param etc...
            public bool bCanInteract;
            public bool bHideInteractIcon;  //when enabled the icon doesn't get painted
            public Int32 nInteractTimer;   //cate secunde tii pe interact ca sa interactionezi
            public bool bStartHidden;

            public CBehaviorContainer(CBehaviorContainer sourceBehavior)
            {
                targetID = sourceBehavior.targetID;
                strActions = sourceBehavior.strActions;
                strAIname = sourceBehavior.strAIname;
                bCanInteract = sourceBehavior.bCanInteract;
                bHideInteractIcon = sourceBehavior.bHideInteractIcon;
                bStartHidden = sourceBehavior.bStartHidden;
                nInteractTimer = sourceBehavior.nInteractTimer;

                listAIparams = new ArrayList(sourceBehavior.listAIparams);
            }

            public CBehaviorContainer()
            {
                targetID = -1;
                strActions = "";
                strAIname = "";
                bCanInteract = false;
                bHideInteractIcon = false;
                bStartHidden = false;
                nInteractTimer = 0;

                listAIparams = new ArrayList();
                listAIparams.Clear();
            }

            // Formats the aprams to a string human readable form
            public string FormatToString()
            {
                string retstr = "";
                for (int kk = 0; kk < listAIparams.Count / 2; kk++)
                {
                    string param = listAIparams[kk * 2] as string;
                    string value = listAIparams[kk * 2 + 1] as string;
                    retstr += param + " = " + value + ";\r\n";
                }

                return retstr;
            }

            public void Save(BinaryWriter bw)
            {
                if (bw == null)
                    return;
                //can interact
                byte u1b = 0;
                if (bCanInteract)
                    u1b |= 0x1;
                if (bHideInteractIcon)
                    u1b |= 0x2;
                bw.Write(u1b);
                //interact timer
                bw.Write(nInteractTimer);
                //start hidden
                u1b = 0;
                if (bStartHidden)
                    u1b = 1;
                bw.Write(u1b);

                bw.Write((Int32)targetID);
                bw.Write(strActions);

                bw.Write(strAIname);
                u1b = (Byte)(listAIparams.Count / 2); //nr de params
                bw.Write(u1b);
                for (int i = 0; i < listAIparams.Count; i++)
                {
                    bw.Write(listAIparams[i] as string);
                }
            }

            public void Load(BinaryReader br, int dwOffsetID = 0)
            {
                if (br == null)
                    return;
                //can interact
                bCanInteract = false;
                byte u1b = br.ReadByte();
                //interact flags
                if ((u1b & 0x1) != 0)
                    bCanInteract = true;
                if ((u1b & 0x2) != 0)
                    bHideInteractIcon = true;
                //interact timer
                nInteractTimer = br.ReadInt32();
                //start hidden
                bStartHidden = false;
                u1b = br.ReadByte();
                if (u1b != 0)
                    bStartHidden = true;

                targetID = br.ReadInt32();
                if (targetID >= 0)
                    targetID += dwOffsetID;

                strActions = br.ReadString();

                strAIname = br.ReadString();

                listAIparams.Clear();
                u1b = br.ReadByte(); //nr params
                for (int i = 0; i < u1b; i++)
                {
                    string paramname = br.ReadString();
                    string paramval = br.ReadString();

                    listAIparams.Add(paramname);
                    listAIparams.Add(paramval);
                }
            }
        }


        /// <summary>
        /// face snap to grid la un punct pe distanta din threshold
        /// </summary>
        /// <param name="pt">punctul sursa</param>
        /// <param name="threshold">distanta pe care face snap-ul</param>
        /// <returns></returns>
        public Point SnapPointToGrid(Point pt, int threshold)
        {
            Point outpt = pt;
            if (outpt.X % TILE_WIDTH <= threshold)
                outpt.X = (outpt.X / TILE_WIDTH) * TILE_WIDTH;
            if (outpt.X % TILE_WIDTH >= TILE_WIDTH - threshold)
                outpt.X = (outpt.X / TILE_WIDTH) * TILE_WIDTH + TILE_WIDTH;
            if (outpt.Y % TILE_HEIGHT <= threshold)
                outpt.Y = (outpt.Y / TILE_HEIGHT) * TILE_HEIGHT;
            if (outpt.Y % TILE_HEIGHT >= TILE_HEIGHT - threshold)
                outpt.Y = (outpt.Y / TILE_HEIGHT) * TILE_HEIGHT + TILE_HEIGHT;

            return outpt;
        }
        public PointF SnapPointToGrid(PointF pt, int threshold)
        {
            Point outpt = new Point((int)pt.X, (int)pt.Y);
            if (outpt.X % TILE_WIDTH <= threshold)
                outpt.X = (outpt.X / TILE_WIDTH) * TILE_WIDTH;
            if (outpt.X % TILE_WIDTH >= TILE_WIDTH - threshold)
                outpt.X = (outpt.X / TILE_WIDTH) * TILE_WIDTH + TILE_WIDTH;
            if (outpt.Y % TILE_HEIGHT <= threshold)
                outpt.Y = (outpt.Y / TILE_HEIGHT) * TILE_HEIGHT;
            if (outpt.Y % TILE_HEIGHT >= TILE_HEIGHT - threshold)
                outpt.Y = (outpt.Y / TILE_HEIGHT) * TILE_HEIGHT + TILE_HEIGHT;

            return outpt;
        }



        //--- misc objects ---
        public const byte K_MISC_RAIL = 0;      //tip RAIL pt lifturi //params: none
        public const byte K_MISC_BACKGROUND = 1; //nume background. //params: str_bsx = night.bsx (seteaza fundalul nivelului, apare unul singur pe nivel)
        public const byte K_MISC_FRONTLAYEROBJ = 2; //tip obiect in prim plan, peste joc. strAnim=nume anim din background curent, nFrame=1 - numar frame
        public const byte K_MISC_SCRIPT = 3;    //script care se ruleaza la inceputul nivelului

        //clasa de baza din care sunt derivate toate
        public class CMiscObjectBase
        {
            public UInt32 ID;
            public byte type;
            public ArrayList listParams; //lista de proprietati

            public CMiscObjectBase()
            {
                listParams = new ArrayList();
                listParams.Clear();
            }

            public virtual void Paint(Graphics gr, Form1 parentForm)
            {
                gr.DrawString("CMiscObjectBase::Not implemented!", new Font("Arial", 8), Brushes.Red, 10.0f, 10.0f);
            }

            public virtual bool GetSelection(PointF pt)
            {
                return false;
            }
            //pentru editare
            public virtual void OnMouseDown(PointF pt) { }
            public virtual void OnMouseUp(PointF pt) { }
            public virtual void OnMouseMove(PointF pt) { }
            public virtual void OnKeypress(KeyEventArgs keyEvent) { }

            public virtual PointF GetOrigin()
            {
                return new PointF(0.0f, 0.0f);
            }
        }

        //clasele particulare fiecarui obiect

        //BACKGROUND
        public class CMiscObject_Background : CMiscObjectBase
        {
            public PointF pos; //pozitie icon
            //private
            const float SELECTION_DISTANCE = 20.0f;

            public CMiscObject_Background()
            {
                type = K_MISC_BACKGROUND;
                pos = new PointF(0.0f, 0.0f);
            }

            public override void Paint(Graphics gr, Form1 parentForm)
            {
                PointF npt = parentForm.WorldToScreen(pos);
                if (parentForm.g_selectedMisc == this)
                    gr.DrawRectangle(Pens.LightGreen, npt.X - SELECTION_DISTANCE, npt.Y - SELECTION_DISTANCE, 2.0f * SELECTION_DISTANCE, 2.0f * SELECTION_DISTANCE);
                else
                    gr.DrawRectangle(Pens.Green, npt.X - SELECTION_DISTANCE, npt.Y - SELECTION_DISTANCE, 2.0f * SELECTION_DISTANCE, 2.0f * SELECTION_DISTANCE);
                gr.DrawString("Background", new Font("Arial", 8), Brushes.Green, npt.X - SELECTION_DISTANCE, npt.Y - SELECTION_DISTANCE);
                for (int kk = 0; kk < listParams.Count / 2; kk++)
                {
                    string name = listParams[kk * 2] as string;
                    string val = listParams[kk * 2 + 1] as string;
                    gr.DrawString(name + ":" + val, new Font("Arial", 8), Brushes.Green, npt.X - SELECTION_DISTANCE, npt.Y - SELECTION_DISTANCE + 10 + kk * 10);
                }
            }

            public override PointF GetOrigin()
            {
                return pos;
            }

            public override bool GetSelection(PointF pt)
            {
                if ((Math.Abs(pt.X - pos.X) < SELECTION_DISTANCE) && (Math.Abs(pt.Y - pos.Y) < SELECTION_DISTANCE))
                {
                    return true;
                }
                return false;
            }
            //pentru editare
            public override void OnMouseDown(PointF pt)
            {

            }
            public override void OnMouseMove(PointF pt)
            {
                pos = pt;
            }
        }

        //SCRIPT
        public class CMiscObject_Script : CMiscObjectBase
        {
            public PointF pos; //pozitie icon
            //private
            const float SELECTION_DISTANCE = 20.0f;

            public CMiscObject_Script()
            {
                type = K_MISC_SCRIPT;
                pos = new PointF(0.0f, 0.0f);
            }

            public override void Paint(Graphics gr, Form1 parentForm)
            {
                PointF npt = parentForm.WorldToScreen(pos);
                if (parentForm.g_selectedMisc == this)
                    gr.DrawRectangle(Pens.LightGreen, npt.X - SELECTION_DISTANCE, npt.Y - SELECTION_DISTANCE, 2.0f * SELECTION_DISTANCE, 2.0f * SELECTION_DISTANCE);
                else
                    gr.DrawRectangle(Pens.Green, npt.X - SELECTION_DISTANCE, npt.Y - SELECTION_DISTANCE, 2.0f * SELECTION_DISTANCE, 2.0f * SELECTION_DISTANCE);
                gr.DrawString("Script", new Font("Arial", 8), Brushes.Green, npt.X - SELECTION_DISTANCE, npt.Y - SELECTION_DISTANCE);
                for (int kk = 0; kk < listParams.Count / 2; kk++)
                {
                    string name = listParams[kk * 2] as string;
                    string val = listParams[kk * 2 + 1] as string;
                    gr.DrawString(name + ":" + val, new Font("Arial", 8), Brushes.Green, npt.X - SELECTION_DISTANCE, npt.Y - SELECTION_DISTANCE + 10 + kk * 10);
                }
            }

            public override PointF GetOrigin()
            {
                return pos;
            }

            public override bool GetSelection(PointF pt)
            {
                if ((Math.Abs(pt.X - pos.X) < SELECTION_DISTANCE) && (Math.Abs(pt.Y - pos.Y) < SELECTION_DISTANCE))
                {
                    return true;
                }
                return false;
            }
            //pentru editare
            public override void OnMouseDown(PointF pt)
            {

            }
            public override void OnMouseMove(PointF pt)
            {
                pos = pt;
            }
        }


        //FRONT LAYER
        public class CMiscObject_FrontLayerObj : CMiscObjectBase
        {
            public PointF pos; //pozitie icon
            //private
            const float SELECTION_DISTANCE = 20.0f;

            public CMiscObject_FrontLayerObj()
            {
                type = K_MISC_FRONTLAYEROBJ;
                pos = new PointF(0.0f, 0.0f);
            }

            public override void Paint(Graphics gr, Form1 parentForm)
            {
                PointF npt = parentForm.WorldToScreen(pos);
                Pen colpen = Pens.Red;
                if (parentForm.g_selectedMisc == this)
                    colpen = Pens.OrangeRed;

                gr.DrawRectangle(colpen, npt.X - SELECTION_DISTANCE, npt.Y - SELECTION_DISTANCE, 2.0f * SELECTION_DISTANCE, 2.0f * SELECTION_DISTANCE);
                gr.DrawLine(colpen, npt.X, npt.Y - SELECTION_DISTANCE, npt.X, npt.Y + SELECTION_DISTANCE);
                gr.DrawLine(colpen, npt.X - SELECTION_DISTANCE, npt.Y, npt.X + SELECTION_DISTANCE, npt.Y);

                gr.DrawString("FirstLayerObj", new Font("Arial", 8), Brushes.LightBlue, npt.X, npt.Y);
                for (int kk = 0; kk < listParams.Count / 2; kk++)
                {
                    string name = listParams[kk * 2] as string;
                    string val = listParams[kk * 2 + 1] as string;
                    gr.DrawString(name + ":" + val, new Font("Arial", 8), Brushes.LightBlue, npt.X, npt.Y + 10 + kk * 10);
                }
            }

            public override PointF GetOrigin()
            {
                return pos;
            }

            public override bool GetSelection(PointF pt)
            {
                if ((Math.Abs(pt.X - pos.X) < SELECTION_DISTANCE) && (Math.Abs(pt.Y - pos.Y) < SELECTION_DISTANCE))
                {
                    return true;
                }
                return false;
            }
            //pentru editare
            public override void OnMouseDown(PointF pt)
            {

            }
            public override void OnMouseMove(PointF pt)
            {
                pos = pt;
            }
        }

        //RAILS
        public class CMiscObject_Rail : CMiscObjectBase
        {
            public ArrayList listPoints;
            //private
            const float SELECTION_DISTANCE = 5.0f;
            public int selectedNodeIdx;

            public CMiscObject_Rail()
            {
                //set type
                type = K_MISC_RAIL;
                //set others
                listPoints = new ArrayList();
                selectedNodeIdx = -1;
            }

            public override bool GetSelection(PointF pt)
            {
                foreach (PointF nod in listPoints)
                {
                    if ((Math.Abs(pt.X - nod.X) < SELECTION_DISTANCE) && (Math.Abs(pt.Y - nod.Y) < SELECTION_DISTANCE))
                    {
                        return true;
                    }
                }
                return false;
            }

            public override PointF GetOrigin()
            {
                return (PointF)listPoints[0];
            }

            public override void OnMouseDown(PointF pt)
            {
                selectedNodeIdx = -1;
                for (int kk = 0; kk < listPoints.Count; kk++)
                {
                    PointF nod = (PointF)listPoints[kk];
                    if ((Math.Abs(pt.X - nod.X) < SELECTION_DISTANCE) && (Math.Abs(pt.Y - nod.Y) < SELECTION_DISTANCE))
                    {
                        selectedNodeIdx = kk;
                        return;
                    }
                }
                //daca nu am selectat nod il adaug
                if (selectedNodeIdx == -1)
                {
                    listPoints.Add(pt);
                    selectedNodeIdx = listPoints.Count - 1;
                }
            }
            public override void OnMouseUp(PointF pt)
            {
            }
            public override void OnMouseMove(PointF pt)
            {
                if (selectedNodeIdx >= 0)
                {
                    listPoints[selectedNodeIdx] = pt;
                }
            }

            public override void OnKeypress(KeyEventArgs keyEvent)
            {
                switch (keyEvent.KeyCode)
                {
                    case Keys.Delete:
                        if (selectedNodeIdx >= 0)
                        {
                            listPoints.RemoveAt(selectedNodeIdx);
                            selectedNodeIdx = -1;
                        }
                        break;
                    case Keys.Left:
                        {
                            if (keyEvent.Control)
                            {
                                if (selectedNodeIdx >= 0)
                                {
                                    PointF pt = (PointF)listPoints[selectedNodeIdx];
                                    pt.X -= 1.0f;
                                    listPoints[selectedNodeIdx] = pt;
                                }
                            }
                        }
                        break;
                    case Keys.Right:
                        {
                            if (keyEvent.Control)
                            {
                                if (selectedNodeIdx >= 0)
                                {
                                    PointF pt = (PointF)listPoints[selectedNodeIdx];
                                    pt.X += 1.0f;
                                    listPoints[selectedNodeIdx] = pt;
                                }
                            }
                        }
                        break;
                    case Keys.Up:
                        {
                            if (keyEvent.Control)
                            {
                                if (selectedNodeIdx >= 0)
                                {
                                    PointF pt = (PointF)listPoints[selectedNodeIdx];
                                    pt.Y -= 1.0f;
                                    listPoints[selectedNodeIdx] = pt;
                                }
                            }
                        }
                        break;
                    case Keys.Down:
                        {
                            if (keyEvent.Control)
                            {
                                if (selectedNodeIdx >= 0)
                                {
                                    PointF pt = (PointF)listPoints[selectedNodeIdx];
                                    pt.Y += 1.0f;
                                    listPoints[selectedNodeIdx] = pt;
                                }
                            }
                        }
                        break;
                }
            }

            public override void Paint(Graphics gr, Form1 parentForm)
            {
                for (int kk = 0; kk < listPoints.Count; kk++)
                {
                    Color baseColor = Color.Green;
                    if (this == parentForm.g_selectedMisc)
                        baseColor = Color.LightGreen;

                    PointF nod = (PointF)listPoints[kk];
                    nod = parentForm.WorldToScreen(nod);
                    if (kk == selectedNodeIdx)
                        gr.DrawEllipse(Pens.LightGreen, nod.X - SELECTION_DISTANCE, nod.Y - SELECTION_DISTANCE, SELECTION_DISTANCE * 2.0f, SELECTION_DISTANCE * 2.0f);
                    else
                        gr.DrawEllipse(Pens.Red, nod.X - SELECTION_DISTANCE, nod.Y - SELECTION_DISTANCE, SELECTION_DISTANCE * 2.0f, SELECTION_DISTANCE * 2.0f);

                    if (kk < listPoints.Count - 1)
                    {
                        Pen linepen = new Pen(baseColor, 3);
                        AdjustableArrowCap myArrow = new AdjustableArrowCap(4, 4, false);
                        Pen capPen = new Pen(Color.Black);
                        linepen.CustomEndCap = myArrow;

                        PointF to = parentForm.WorldToScreen((PointF)listPoints[kk + 1]);
                        gr.DrawLine(linepen, nod, to);
                    }
                    //paint ID
                    if (kk == 0)
                    {
                        gr.DrawString("ID:" + ID, new Font("Arial", 8, FontStyle.Bold), Brushes.Red, nod);
                    }
                }
            }

        }
        //lista de misc objects
        public ArrayList arrMisc = new ArrayList();
        public CMiscObjectBase g_selectedMisc = null;


        //--- collisions ---
        public const byte K_COLL_TYPE_SOLID = 0;  //blocheaza trecerea
        public const byte K_COLL_TYPE_STAIRS = 1; //scari cu trepte. se urca cu jump
        public const byte K_COLL_TYPE_WATER = 2; //bloc de apa fara gravitatie
        public const byte K_COLL_TYPE_LADDER = 3; //bloc pe care urci gen liane, scari drepte, etc
        public const byte K_COLL_TYPE_BOX = 4; //sunt tot stairs dar iti seteaza treapta egala cu inaltimea
        public const byte K_COLL_TYPE_TRIGGER = 5; //box invizibil care porneste scripturi sau face touch
        public const byte K_COLL_TYPE_PARTICLE_SYSTEM = 6; //box invizibil care comanda generare de particule
        public const byte K_COLL_TYPE_ROOM_OCCLUDER = 7; //box care ascunde camerele nedescoperite
        public const byte K_COLL_TYPE_COVER = 8; //box dupa care se poate ascunde playerul
        public const byte K_COLL_TYPE_MOVING_PLATFORM = 9; //box care se misca (elevators?)
        public const byte K_COLL_TYPE_LEDGE = 10; //box care opreste gloantele dar sari prin el

        //colors for the boxes
        Color[] g_arrCollColors = new Color[] { Color.Red, Color.LemonChiffon, Color.Blue, Color.Yellow, Color.Orange, Color.DarkGreen, Color.MediumVioletRed, Color.MediumPurple, Color.LightGreen, Color.DarkRed, Color.OrangeRed };

        public class CCollisionElement
        {
            public UInt32 ID;
            public int type;

            public bool castShadows;
            public RectangleF rect;
            //date despre logica
            public CBehaviorContainer logic;


            public CCollisionElement()
            {
                type = K_COLL_TYPE_SOLID;

                castShadows = true;
                rect = new RectangleF();

                logic = new CBehaviorContainer();
            }

            public CCollisionElement(CCollisionElement srcElement)
            {
                type = srcElement.type;

                castShadows = srcElement.castShadows;
                rect = new RectangleF(srcElement.rect.X, srcElement.rect.Y, srcElement.rect.Width, srcElement.rect.Height);

                logic = new CBehaviorContainer(srcElement.logic);
            }

            public bool IsSolid()
            {
                if ((type == K_COLL_TYPE_ROOM_OCCLUDER) || (type == K_COLL_TYPE_TRIGGER) || (type == K_COLL_TYPE_WATER) || (type == K_COLL_TYPE_PARTICLE_SYSTEM))
                    return false;

                return true;
            }
        }

        public ArrayList arrCollisions = new ArrayList();
        public CCollisionElement g_selectedCollision = null;

        public CCollisionElement GetCollisionElement(Point pt)
        {
            for (int kk = 0; kk < arrCollisions.Count; kk++)
            {
                CCollisionElement coll = arrCollisions[kk] as CCollisionElement;
                if (coll == g_selectedCollision) //skip current collision
                    continue;
                //hide unwanted collision rects
                if ((g_wndCollisions.HideWater) && (coll.type == K_COLL_TYPE_WATER))
                    continue;
                if ((g_wndCollisions.HideFOW) && (coll.type == K_COLL_TYPE_ROOM_OCCLUDER))
                    continue;
                if ((g_wndCollisions.HideTriggers) && (coll.type == K_COLL_TYPE_TRIGGER))
                    continue;

                if (PointInRect(pt, coll.rect))
                    return coll;
            }
            return null;
        }

        //--- luminile ---
        public const int K_LIGHT_AMBIENTAL = 0;
        public const int K_LIGHT_AREA = 1;
        public const int K_LIGHT_POINT = 2;
        public const int K_LIGHT_DIRECTIONAL = 3;
        public const int K_LIGHT_IES_REALISTIC_OBSOLETE = 4; //initially we had this but was too complicated
        //si numarul lor
        public const int K_LIGHTS_COUNT = 5;

        //-- light props flags ---
        public const UInt16 K_LIGHT_FLAG_CAST_SHADOWS = 1;
        public const UInt16 K_LIGHT_FLAG_LENS_FLARE_OBSOLETE = 2;

        //TODO: ar trebui parametru de layer sau flag cu layerele pe care le influenteaza?
        public class CLight
        {
            public UInt32 ID; //id unic lumina
            public int type;
            public PointF pos;
            public int posZ; //coordonata Z
            public float angle; //folosit la rotirea texturii
            public bool castsShadows;
            public int nAtmoAttenuationPerc; //0-100 - atttenuation of volumetric light
            public Color color;
            public float fIntensity;

            public int animId; //id animatie lumina curenta
            //proprietati pentru diversele tipuri de 
            public RectangleF area;
            //date despre logica
            public CBehaviorContainer logic;

            public CLight(CLight lightSource)
            {
                ID = 0;
                animId = lightSource.animId;
                type = lightSource.type;
                pos = lightSource.pos;
                area = new RectangleF(lightSource.area.X, lightSource.area.Y, lightSource.area.Width, lightSource.area.Height);
                angle = lightSource.angle;
                castsShadows = lightSource.castsShadows;
                nAtmoAttenuationPerc = lightSource.nAtmoAttenuationPerc;
                color = lightSource.color;
                posZ = lightSource.posZ;
                fIntensity = lightSource.fIntensity;

                logic = new CBehaviorContainer(lightSource.logic);
            }

            public CLight()
            {
                ID = 0;
                animId = -1;
                type = K_LIGHT_POINT;
                pos = new PointF(0.0f, 0.0f);
                area = new RectangleF(-K_LIGHT_DEFAULT_RADIUS, -K_LIGHT_DEFAULT_RADIUS, 2 * K_LIGHT_DEFAULT_RADIUS, 2 * K_LIGHT_DEFAULT_RADIUS);
                angle = 0.0f;
                castsShadows = false;
                nAtmoAttenuationPerc = 0;
                color = Color.FromArgb(255, 255, 255);
                posZ = 100;
                fIntensity = 1.0f;

                logic = new CBehaviorContainer();
            }
        }
        public ArrayList arrLights = new ArrayList();
        public CLight g_selectedLight = null;

        CLight GetLight(PointF clickPos)
        {
            for (int kk = 0; kk < arrLights.Count; kk++)
            {
                CLight light = arrLights[kk] as CLight;

                if ((clickPos.X > light.pos.X - 20.0f / zoomLevel) && (clickPos.Y > light.pos.Y - 20.0f / zoomLevel) && (clickPos.X < light.pos.X + 20.0f / zoomLevel) && (clickPos.Y < light.pos.Y + 20.0f / zoomLevel))
                    return light;
            }
            return null;
        }

        bool PointInLight(CLight light, Point pos)
        {
            if ((pos.X > light.pos.X - 20.0f / zoomLevel) && (pos.Y > light.pos.Y - 20.0f / zoomLevel) && (pos.X < light.pos.X + 20.0f / zoomLevel) && (pos.Y < light.pos.Y + 20.0f / zoomLevel))
                return true;
            return false;
        }

        //Seteaza bounding box-ul egal cu cel din animatie
        public void SetLightAreaFromAnim(CLight light)
        {
            if (g_sprLights.bLoaded == false)
                return;

            if (((light.type == K_LIGHT_POINT) || (light.type == K_LIGHT_AREA)) && (light.animId >= 0))
            {
                //folosim primul frame, adica cel al spotului.
                Rectangle rect = g_sprLights.anims[light.animId].aframes[0].frame.GetRect();
                light.area = rect;
                //scale area
                light.area.X *= K_LIGHT_DEFAULT_FSCALING; light.area.Y *= K_LIGHT_DEFAULT_FSCALING;
                light.area.Width *= K_LIGHT_DEFAULT_FSCALING; light.area.Height *= K_LIGHT_DEFAULT_FSCALING;

                light.area.X += light.pos.X;
                light.area.Y += light.pos.Y;
            }
        }

        //--- actorii --- inamici si personaj principal ---
        public class CActor
        {
            public UInt32 ID;
            public Point pos;
            public bool bLookLeft;
            public string templateName; //nume template oameni
            public bool bHasCollision, bHasGravity; //flags pt ingame
            //idx animatie pentru afisare (nu se salveaza)
            public int animIdx;
            //unghiul (daca e setat)
            public float fAngle;
            public bool bSetAngle;
            //date despre logica
            public CBehaviorContainer logic;
            public string strSelectedAIState;

            public CActor()
            {
                ID = 0;
                animIdx = 0;
                pos.X = pos.Y = 0;
                bLookLeft = false;
                bHasCollision = true;
                bHasGravity = true;

                bSetAngle = false;
                fAngle = 0.0f;
                strSelectedAIState = "";  //no state selected

                logic = new CBehaviorContainer();
            }
        }

        public ArrayList arrActors = new ArrayList();
        public CActor g_selectedActor = null;

        public CActor GetActor(Point pos)
        {
            for (int kk = 0; kk < arrActors.Count; kk++)
            {
                CActor obj = arrActors[kk] as CActor;
                //toggle between selected and the one behind it
                if (obj == g_selectedActor)
                    continue;

                Rectangle boxrect = new Rectangle(-10, -10, 20, 20);
                if (obj.animIdx >= 0)
                    boxrect = g_sprActors.anims[obj.animIdx].aframes[0].frame.BBox_real;
                boxrect.X += obj.pos.X; boxrect.Y += obj.pos.Y;
                if (boxrect.Contains(pos))
                    return obj;
            }
            return null;
        }


        //--- obiectele/decorurile scriptabile ---
        public const UInt32 OBJFLAG_FLIPX = 1;
        public const UInt32 OBJFLAG_FLIPY = 2;
        public const UInt32 OBJFLAG_FLIPXORY = 3;
        public const UInt32 OBJFLAG_ANIMATED = 4;
        public const UInt32 OBJFLAG_IS_COVER = 8;

        public class CObject
        {
            public UInt32 ID;
            public int animIdx;
            public int frameIdx;
            public Point pos;
            public int layer; //pe ce layer este obiectul de decor

            public UInt32 flags; //flaguri de tipul OBJFLAG_ pt flipuri, banimated, bIsCover
            //date despre logica
            public CBehaviorContainer logic;

            public CObject()
            {
                ID = 0;
                flags = 0;
                animIdx = frameIdx = 0;
                pos.X = pos.Y = 0;
                layer = (int)ELayer.FLOOR;

                logic = new CBehaviorContainer();
            }
        }
        public ArrayList arrObjects = new ArrayList();
        public CObject g_selectedObject = null;
        public PointF g_selectedObjectOldPos = new PointF(0.0f, 0.0f); //used in order to get absolute delta when moving objects

        public bool PointInObject(CObject obj, Point pos)
        {
            if (obj == null)
                return false;

            Rectangle boxrect = new Rectangle(-10, -10, 20, 20);
            if ((obj.animIdx >= 0) && (obj.frameIdx >= 0))
                boxrect = g_sprObjects.anims[obj.animIdx].aframes[obj.frameIdx].frame.BBox_real;
            //pentru cele flipate pe X nu pastrez bboxul ci se flipeaza in fn de originea lor
            int offx = 0;
            if ((obj.flags & OBJFLAG_FLIPX) != 0)
                offx = -2 * (boxrect.X + boxrect.Width / 2);

            boxrect.X += obj.pos.X + offx; boxrect.Y += obj.pos.Y;

            if (boxrect.Contains(pos))
                return true;
            //not inside:
            return false;
        }

        public CObject GetObject(Point pos)
        {
            for (int kk = 0; kk < arrObjects.Count; kk++)
            {
                CObject obj = arrObjects[kk] as CObject;
                if (obj == g_selectedObject)
                    continue;

                Rectangle boxrect = new Rectangle(-10, -10, 20, 20);
                if ((obj.animIdx >= 0) && (obj.frameIdx >= 0))
                    boxrect = g_sprObjects.anims[obj.animIdx].aframes[obj.frameIdx].frame.BBox_real;
                //pentru cele flipate pe X nu pastrez bboxul ci se flipeaza in fn de originea lor
                int offx = 0;
                if ((obj.flags & OBJFLAG_FLIPX) != 0)
                    offx = -2 * (boxrect.X + boxrect.Width / 2);

                boxrect.X += obj.pos.X + offx; boxrect.Y += obj.pos.Y;

                if (boxrect.Contains(pos))
                    return obj;
            }
            return null;
        }

        //intoarce pozitia targetului in fn de ID
        public PointF GetTargetPosByID(int targetID)
        {
            for (int kk = 0; kk < arrObjects.Count; kk++)
            {
                CObject obj = arrObjects[kk] as CObject;
                if (obj.ID == targetID)
                {
                    Rectangle boxrect = new Rectangle(-10, -10, 20, 20);
                    if ((obj.animIdx >= 0) && (obj.frameIdx >= 0))
                        boxrect = g_sprObjects.anims[obj.animIdx].aframes[obj.frameIdx].frame.BBox_real;
                    //pentru cele flipate pe X nu pastrez bboxul ci se flipeaza in fn de originea lor
                    int offx = 0;
                    if ((obj.flags & OBJFLAG_FLIPX) != 0)
                        offx = -2 * (boxrect.X + boxrect.Width / 2);

                    boxrect.X += obj.pos.X + offx; boxrect.Y += obj.pos.Y;

                    return new PointF(boxrect.X + boxrect.Width / 2, boxrect.Y + boxrect.Height / 2);
                }
            }
            for (int kk = 0; kk < arrLights.Count; kk++)
            {
                CLight light = arrLights[kk] as CLight;
                if (light.ID == targetID)
                    return new PointF(light.pos.X, light.pos.Y);
            }
            for (int kk = 0; kk < arrCollisions.Count; kk++)
            {
                CCollisionElement coll = arrCollisions[kk] as CCollisionElement;
                if (coll.ID == targetID)
                    return new PointF(coll.rect.X + coll.rect.Width / 2.0f, coll.rect.Y + coll.rect.Height / 2.0f);
            }
            for (int kk = 0; kk < arrMisc.Count; kk++)
            {
                CMiscObjectBase misc = arrMisc[kk] as CMiscObjectBase;
                if (misc.ID == targetID)
                    return misc.GetOrigin();
            }
            for (int kk = 0; kk < arrActors.Count; kk++)
            {
                CActor actor = arrActors[kk] as CActor;
                Rectangle boxrect = g_sprActors.anims[actor.animIdx].aframes[0].frame.BBox_real;
                boxrect.X += actor.pos.X; boxrect.Y += actor.pos.Y;

                if (actor.ID == targetID)
                    return new PointF(boxrect.X + boxrect.Width / 2, boxrect.Y + boxrect.Height / 2);
            }
            return new PointF(0, 0);
        }

        // Returns the number of collisions that point to targetID
        public int GetCollisionsWithTargetIDCount(int targetID, bool bOnlyCountSolids = false)
        {
            int nCount = 0;
            if (targetID < 0)
                return 0;

            for (int kk = arrCollisions.Count - 1; kk >= 0; kk--)
            {
                CCollisionElement coll = arrCollisions[kk] as CCollisionElement;
                if (coll.logic.targetID == targetID)
                {
                    if (bOnlyCountSolids)
                    {
                        if (coll.IsSolid() == false)
                            continue;
                    }

                    nCount++;
                }
            }

            return nCount;
        }

        // Deletes all collisions that point to targetID
        public void DeleteCollisionElementsWithTargetID(int targetID)
        {
            if (targetID < 0)
                return;

            for (int kk = arrCollisions.Count - 1; kk >= 0; kk--)
            {
                CCollisionElement coll = arrCollisions[kk] as CCollisionElement;
                if (coll.logic.targetID == targetID)
                    arrCollisions.RemoveAt(kk);
            }

            SetFileModified();
        }

        // Removes target from objects that point to targetID
        public void RemoveTargetFromAllChildren(int targetID)
        {
            for (int kk = 0; kk < arrObjects.Count; kk++)
            {
                CObject obj = arrObjects[kk] as CObject;
                if (obj.logic.targetID == targetID)
                    obj.logic.targetID = -1;
            }
            for (int kk = 0; kk < arrLights.Count; kk++)
            {
                CLight light = arrLights[kk] as CLight;
                if (light.logic.targetID == targetID)
                    light.logic.targetID = -1;
            }
            for (int kk = 0; kk < arrCollisions.Count; kk++)
            {
                CCollisionElement coll = arrCollisions[kk] as CCollisionElement;
                if (coll.logic.targetID == targetID)
                    coll.logic.targetID = -1;
            }
            for (int kk = 0; kk < arrActors.Count; kk++)
            {
                CActor actor = arrActors[kk] as CActor;
                if (actor.logic.targetID == targetID)
                    actor.logic.targetID = -1;
            }

            SetFileModified();
        }
       
        // Saves all elements in outArrElements
        // Returns selection hash (to see if different)
        public int GetElementsUIDsAtPoint(Point vPos, out ArrayList outArrElements, bool bSelectObjects, bool bSelectLights, bool bSelectCollisions, bool bSelectActors)
        {
            int outHash = 0;
            outArrElements = new ArrayList();
            if (bSelectObjects)
            {
                for (int kk = 0; kk < arrObjects.Count; kk++)
                {
                    CObject obj = arrObjects[kk] as CObject;
                    if (PointInObject(obj, vPos))
                    {
                        outArrElements.Add(obj);
                        outHash += (int)obj.ID + (int)obj.pos.X + (int)obj.pos.Y;
                    }
                }
            }

            if (bSelectLights)
            {
                for (int kk = 0; kk < arrLights.Count; kk++)
                {
                    CLight light = arrLights[kk] as CLight;
                    if (PointInLight(light, vPos))
                    {
                        outArrElements.Add(light);
                        outHash += (int)light.ID + (int)light.pos.X + (int)light.pos.Y;
                    }
                }
            }

            if (bSelectCollisions)
            {
                for (int kk = 0; kk < arrCollisions.Count; kk++)
                {
                    CCollisionElement coll = arrCollisions[kk] as CCollisionElement;
                    if (PointInRect(vPos, coll.rect))
                    {
                        outArrElements.Add(coll);
                        outHash += (int)coll.ID + (int)coll.rect.X + (int)coll.rect.Y + (int)coll.rect.Width + (int)coll.rect.Height;
                    }
                }
            }

            if (bSelectActors)
            {
                for (int kk = 0; kk < arrActors.Count; kk++)
                {
                    CActor actor = arrActors[kk] as CActor;
                    Rectangle boxrect = new Rectangle(-10, -10, 20, 20);
                        if(actor.animIdx >= 0)
                            boxrect = g_sprActors.anims[actor.animIdx].aframes[0].frame.BBox_real;
                    boxrect.X += actor.pos.X; boxrect.Y += actor.pos.Y;
                    if (boxrect.Contains(vPos))
                    {
                        outArrElements.Add(actor);
                        outHash += (int)actor.ID + (int)actor.pos.X + (int)actor.pos.Y;
                    }
                }
            }

            return outHash;
        }

        //automatic selection 
        int g_SelectionLastHash = 0;
        int g_SelectionLastIndex = 0; //last index in selection list
        // Returns current selection at point
        int SelectElementAt(Point vPos, bool bSelectObjects, bool bSelectLights, bool bSelectCollisions, bool bSelectActors)
        {
            ArrayList arrElementsAtPoint;
            int nNewHash = GetElementsUIDsAtPoint(vPos, out arrElementsAtPoint, bSelectObjects, bSelectLights, bSelectCollisions, bSelectActors);

            if (nNewHash == 0) //no object, deselect
            {
                g_SelectionLastIndex = -1;

                g_selectedLight = null;
                g_selectedCollision = null;
                g_selectedObject = null;
                g_selectedActor = null;

                return -1;
            }
            else if (nNewHash != g_SelectionLastHash)
            {
                g_SelectionLastHash = nNewHash;
                g_SelectionLastIndex = 0;
                //deselect all
                g_selectedLight = null;
                g_selectedCollision = null;
                g_selectedObject = null;
                g_selectedActor = null;
            }
            else //same selection, just select the next one in the list
            {
                //deselect all
                g_selectedLight = null;
                g_selectedCollision = null;
                g_selectedObject = null;
                g_selectedActor = null;

                g_SelectionLastIndex++;
                if (g_SelectionLastIndex > arrElementsAtPoint.Count - 1)
                    g_SelectionLastIndex = 0;
            }

            if ((g_SelectionLastIndex >= 0) && (g_SelectionLastIndex < arrElementsAtPoint.Count))
            {
                if (arrElementsAtPoint[g_SelectionLastIndex].GetType() == typeof(CObject))
                {
                    g_selectedObject = arrElementsAtPoint[g_SelectionLastIndex] as CObject;
                    return (int)g_selectedObject.ID;
                }
                else if (arrElementsAtPoint[g_SelectionLastIndex].GetType() == typeof(CLight))
                {
                    g_selectedLight = arrElementsAtPoint[g_SelectionLastIndex] as CLight;
                    return (int)g_selectedLight.ID;
                }
                else if (arrElementsAtPoint[g_SelectionLastIndex].GetType() == typeof(CCollisionElement))
                {
                    g_selectedCollision = arrElementsAtPoint[g_SelectionLastIndex] as CCollisionElement;
                    return (int)g_selectedCollision.ID;
                }
                else if (arrElementsAtPoint[g_SelectionLastIndex].GetType() == typeof(CActor))
                {
                    g_selectedActor = arrElementsAtPoint[g_SelectionLastIndex] as CActor;
                    return (int)g_selectedActor.ID;
                }
            }

            return -1;
        }

        //-----------------------------
        public class CTile
        {
            public int[] tileID;

            public bool IsEmpty()
            {
                for (int kk = 0; kk < (int)ELayer.LAYERS_CNT; kk++)
                {
                    if (tileID[kk] >= 0)
                        return false;
                }
                return true;
            }

            public CTile()
            {
                tileID = new int[(int)ELayer.LAYERS_CNT];
                for (int kk = 0; kk < (int)ELayer.LAYERS_CNT; kk++)
                {
                    tileID[kk] = -1;
                }
            }
        }

        #region TILE BLOCKS


        public const int BLOCK_W = 8;
        public const int BLOCK_H = 8;

        public class CTileBlock
        {
            public Image layerImg; //image that contains all layers
            public Graphics graphics; //graphics to image

            public CTile[,] tiles;
            public Point pos; //in tiles

            public bool bHasUndo;
            public CTile[,] tiles_undo;

            public CTileBlock(int nTileSize)
            {
                tiles = new CTile[BLOCK_W, BLOCK_H];
                tiles_undo = new CTile[BLOCK_W, BLOCK_H];

                for (int kk = 0; kk < BLOCK_W; kk++)
                {
                    for (int ll = 0; ll < BLOCK_H; ll++)
                    {
                        tiles[kk, ll] = new CTile();
                        tiles_undo[kk, ll] = new CTile();
                    }
                }

                graphics = null;
                layerImg = null;
                if (nTileSize > 0)
                {
                    layerImg = new Bitmap(nTileSize * BLOCK_W, nTileSize * BLOCK_H, System.Drawing.Imaging.PixelFormat.Format32bppArgb);
                    graphics = Graphics.FromImage(layerImg);
                    graphics.Clear(Color.Transparent);
                }
                else
                {
                    MessageBox.Show("Error!", "Could not create Tile Block image! Tile size is 0!", MessageBoxButtons.OK);
                }

                bHasUndo = false;
            }

            // returns true if it  has walkable tiles on specified extremity
            public bool HasConnectionOnSide(int eDirection)
            {
                //#TODO: sa ia in considerare toate layerele de floor nu doar primul (pentru cand ai tranzitii)
                switch (eDirection)
                {
                    case K_DIR_UP:
                        for (int kk = 0; kk < BLOCK_W; kk++)
                        {
                            if (tiles[kk, 0].tileID[(int)ELayer.FLOOR] >= 0)
                                return true;
                        }
                        break;
                    case K_DIR_DOWN:
                        for (int kk = 0; kk < BLOCK_W; kk++)
                        {
                            if (tiles[kk, BLOCK_H - 1].tileID[(int)ELayer.FLOOR] >= 0)
                                return true;
                        }
                        break;
                    case K_DIR_LEFT:
                        for (int kk = 0; kk < BLOCK_W; kk++)
                        {
                            if (tiles[0, kk].tileID[(int)ELayer.FLOOR] >= 0)
                                return true;
                        }
                        break;
                    case K_DIR_RIGHT:
                        for (int kk = 0; kk < BLOCK_W; kk++)
                        {
                            if (tiles[BLOCK_W - 1, kk].tileID[(int)ELayer.FLOOR] >= 0)
                                return true;
                        }
                        break;
                    default:
                        MessageBox.Show("Specify direction!");
                        break;
                }

                return false;
            }

            public void Undo_SaveState()
            {
                if (bHasUndo == false)
                {
                    bHasUndo = true;
                    for (int kk = 0; kk < BLOCK_W; kk++)
                    {
                        for (int ll = 0; ll < BLOCK_H; ll++)
                        {
                            for (int mm = 0; mm < (int)ELayer.LAYERS_CNT; mm++)
                            {
                                tiles_undo[kk, ll].tileID[mm] = tiles[kk, ll].tileID[mm];
                            }
                        }
                    }
                }
            }

            public void Undo_ClearUndo()
            {
                bHasUndo = false;
            }

            // Undoes the last change
            public void UndoChange()
            {
                if (bHasUndo)
                {
                    bHasUndo = false;
                    for (int kk = 0; kk < BLOCK_W; kk++)
                    {
                        for (int ll = 0; ll < BLOCK_H; ll++)
                        {
                            for (int mm = 0; mm < (int)ELayer.LAYERS_CNT; mm++)
                            {
                                tiles[kk, ll].tileID[mm] = tiles_undo[kk, ll].tileID[mm];
                            }
                        }
                    }
                }
            }
        }

        #endregion

        #region BLOCKS

        public class CBlocks
        {
            public List<CTileBlock> Blocks = new List<CTileBlock>();

            public int nTileW { get; set; }
            public int nTileH { get; set; }
            public int nTilesetColumns { get; set; }

            public CBlocks()
            {
                Blocks = new List<CTileBlock>();
            }

            public void ClearAllBlocks()
            {
                Blocks.RemoveRange(0, Blocks.Count);
            }

            //sterge blocurile goale
            public void ClearEmptyblocks()
            {
                for (int kk = Blocks.Count - 1; kk >= 0; kk--)
                {
                    CTileBlock tb = Blocks[kk] as CTileBlock;
                    bool erase = true;
                    for (int xx = 0; xx < BLOCK_W; xx++)
                    {
                        for (int yy = 0; yy < BLOCK_H; yy++)
                        {
                            for (int lay = 0; lay < (int)ELayer.LAYERS_CNT; lay++)
                            {
                                if (tb.tiles[xx, yy].tileID[lay] >= 0)
                                {
                                    erase = false;
                                    goto NOTEMPTY;
                                }
                            }
                        }
                    }

                NOTEMPTY:
                    if(erase)
                        Blocks.RemoveAt(kk);
                }
            }

            //Force clears undo
            public void Undo_ClearUndo()
            {
                for (int kk = Blocks.Count - 1; kk >= 0; kk--)
                {
                    CTileBlock tb = Blocks[kk] as CTileBlock;
                    tb.bHasUndo = false;
                }
            }


            //sets tile, returns affected block
            public CTileBlock setTile(int xtl, int ytl, int tileID, int layerIDX)
            {
                CTileBlock foundtb = null;
                for (int kk = 0; kk < Blocks.Count; kk++)
                {
                    CTileBlock tb = Blocks[kk] as CTileBlock;
                    if ((tb.pos.X == xtl / BLOCK_W) && (tb.pos.Y == ytl / BLOCK_H))
                    {
                        foundtb = tb;
                    }
                }
                //daca nu a gasit tile block il adauga
                if (foundtb == null)
                {
                    foundtb = new CTileBlock(nTileW);
                    foundtb.pos.X = xtl / BLOCK_W;
                    foundtb.pos.Y = ytl / BLOCK_H;

                    Blocks.Add(foundtb);
                }

                //save undo state
                foundtb.Undo_SaveState();
                //set tile
                foundtb.tiles[xtl % BLOCK_W, ytl % BLOCK_H].tileID[layerIDX] = tileID;
                return foundtb;
            }

            //intoarce un tile
            public CTile getTile(int xtl, int ytl)
            {
                if ((xtl < 0) || (ytl < 0))
                    return null;
                CTileBlock foundtb = null;
                for (int kk = 0; kk < Blocks.Count; kk++)
                {
                    CTileBlock tb = Blocks[kk] as CTileBlock;
                    if ((tb.pos.X == xtl / BLOCK_W) && (tb.pos.Y == ytl / BLOCK_H))
                    {
                        foundtb = tb;
                    }
                }
                //daca nu a gasit tile block intoarce null
                if (foundtb == null)
                {
                    return null;
                }
                else
                {
                    return foundtb.tiles[xtl % BLOCK_W, ytl % BLOCK_H];
                }
            }

            public CTileBlock getBlockAt(int xtl, int ytl)
            {
                if ((xtl < 0) || (ytl < 0))
                    return null;
                CTileBlock foundtb = null;
                for (int kk = 0; kk < Blocks.Count; kk++)
                {
                    CTileBlock tb = Blocks[kk] as CTileBlock;
                    if ((tb.pos.X == xtl / BLOCK_W) && (tb.pos.Y == ytl / BLOCK_H))
                    {
                        foundtb = tb;
                    }
                }
                //daca nu a gasit tile block intoarce null
                if (foundtb == null)
                {
                    return null;
                }
                else
                {
                    return foundtb;
                }
            }
        }

        //builds the tileset image for a block
        public void BuildBlockImage(CTileBlock tb)
        {
            Rectangle srcr = new Rectangle();

            tb.graphics.Clear(Color.Transparent);

            for (int layer = 0; layer < (int)ELayer.LAYERS_CNT; layer++)
            {
                for (int yy = 0; yy < BLOCK_H; yy++)
                {
                    for (int xx = 0; xx < BLOCK_W; xx++)
                    {
                        if (!layers_checkboxes[layer].Checked)
                            continue;

                        int tileID = tb.tiles[xx, yy].tileID[layer];
                        if (tileID < 0)
                            continue;

                        srcr.X = (tileID % TILESET_COLUMNS) * TILE_WIDTH;
                        srcr.Y = (tileID / TILESET_COLUMNS) * TILE_HEIGHT;
                        srcr.Width = TILE_WIDTH; srcr.Height = TILE_HEIGHT;

                        tb.graphics.DrawImage(g_wndMaterials.g_TilesetImg, xx * TILE_WIDTH, yy * TILE_HEIGHT, srcr, GraphicsUnit.Pixel);
                    }
                }
            }
        }

        public void BuildAllBlockImages()
        {
            for (int kk = 0; kk < gMap.Blocks.Count; kk++)
            {
                CTileBlock tb = gMap.Blocks[kk] as CTileBlock;
                BuildBlockImage(tb);
            }
        }
        #endregion

        public void MoveSelectionToLayer(Rectangle selRectTL, int toLayer)
        {
            if ((toLayer < 0) || (toLayer >= (int)ELayer.LAYERS_CNT) || (toLayer == g_selectedLayer))
                return;
            //clear undo state before operation
            gMap.Undo_ClearUndo();

            for (int yy = 0; yy < selRectTL.Height; yy++)
            {
                for (int xx = 0; xx < selRectTL.Width; xx++)
                {
                    CTile tl = gMap.getTile(selRectTL.X + xx, selRectTL.Y + yy);
                    if ((tl == null) || (tl.tileID[g_selectedLayer] < 0))
                        continue;
                    gMap.setTile(selRectTL.X + xx, selRectTL.Y + yy, tl.tileID[g_selectedLayer], toLayer);
                    gMap.setTile(selRectTL.X + xx, selRectTL.Y + yy, -1, g_selectedLayer);
                }
            }

            Undo_SetCurrent(K_UNDO_TILES);

            BuildAllBlockImages();
        }

        /// <summary>
        /// Umple selectia curenta cu tile-ul sau blocul de tiles selectate in materialEditor
        /// </summary>
        /// <param name="selRectTL">Zona care trebuie umpluta, data in tiles</param>
        public void FillSelectionWithTileBrush(Rectangle selRectTL)
        {
            //clear undo state before operation
            gMap.Undo_ClearUndo();

            for (int yy = 0; yy < selRectTL.Height; yy++)
            {
                for (int xx = 0; xx < selRectTL.Width; xx++)
                {
                    int tileID = (g_matBrush.X + (xx % g_matBrush.Width) + (g_matBrush.Y + (yy % g_matBrush.Height)) * TILESET_COLUMNS);
                    gMap.setTile(selRectTL.X + xx, selRectTL.Y + yy, tileID, g_selectedLayer);
                }
            }

            Undo_SetCurrent(K_UNDO_TILES);

            BuildAllBlockImages();
        }
        /// <summary>
        /// Copiaza toata zona selectata in noua zona indicata prin pastePosTL doar din layerele vizibile!
        /// </summary>
        /// <param name="selRectTL">Zona din care copiaza, data in tiles</param>
        /// <param name="pastePosTL">Pozitia unde copiaza, in tiles</param>
        public void CopyPasteSelection(Rectangle selRectTL, Point pastePosTL, bool allVisibleLayers)
        {
            //clear undo state before operation
            gMap.Undo_ClearUndo();

            if (allVisibleLayers)
            {
                for (int kk = 0; kk < (int)ELayer.LAYERS_CNT; kk++)
                {
                    for (int yy = 0; yy < selRectTL.Height; yy++)
                    {
                        for (int xx = 0; xx < selRectTL.Width; xx++)
                        {
                            CTile tl = gMap.getTile(selRectTL.X + xx, selRectTL.Y + yy);
                            if(tl != null)
                                gMap.setTile(pastePosTL.X + xx, pastePosTL.Y + yy, tl.tileID[kk], kk);
                        }
                    }
                }
            }
            else
            {
                for (int yy = 0; yy < selRectTL.Height; yy++)
                {
                    for (int xx = 0; xx < selRectTL.Width; xx++)
                    {
                        CTile tl = gMap.getTile(selRectTL.X + xx, selRectTL.Y + yy);
                        if (tl != null)
                        {
                            gMap.setTile(pastePosTL.X + xx, pastePosTL.Y + yy, tl.tileID[g_selectedLayer], g_selectedLayer);
                        }
                    }
                }
            }

            Undo_SetCurrent(K_UNDO_TILES);

            BuildAllBlockImages();
        }

        /// <summary>
        /// Sterge tot ce e in selectie de pe layerul curent sau de pe toate layerele vizibile
        /// </summary>
        public void DeleteFromSelection(Rectangle selRectTL, bool allVisibleLayers)
        {
            //clear undo state before operation
            gMap.Undo_ClearUndo();

            if (allVisibleLayers)
            {
                for (int kk = 0; kk < (int)ELayer.LAYERS_CNT; kk++)
                {
                    if (!layers_checkboxes[kk].Checked)
                        continue;
                    for (int yy = 0; yy < selRectTL.Height; yy++)
                    {
                        for (int xx = 0; xx < selRectTL.Width; xx++)
                        {
                            gMap.setTile(selRectTL.X + xx, selRectTL.Y + yy, -1, kk);
                        }
                    }
                }
            }
            else
            {
                for (int yy = 0; yy < selRectTL.Height; yy++)
                {
                    for (int xx = 0; xx < selRectTL.Width; xx++)
                    {
                        gMap.setTile(selRectTL.X + xx, selRectTL.Y + yy, -1, g_selectedLayer);
                    }
                }
            }

            Undo_SetCurrent(K_UNDO_TILES);

            gMap.ClearEmptyblocks();
            BuildAllBlockImages();
        }

        public CBlocks gMap = new CBlocks();
        public Point gLevelOrigin = new Point(0, 0);
        //brushes
        public const int BRUSH_MODE_TILES = 0;
        public const int BRUSH_MODE_LIGHTS = 1;
        public const int BRUSH_MODE_COLLISIONS = 2;
        public const int BRUSH_MODE_OBJECTS = 3;
        public const int BRUSH_MODE_GROUND_LEVEL = 4;
        public const int BRUSH_MODE_AI = 5;
        public const int BRUSH_MODE_MISC = 6;
        public const int BRUSH_MODE_ACTORS = 7;
        public const int BRUSH_MODE_MACRO = 8;  //folosit ca sa adauge automat obiecte complexe
        public const int BRUSH_MODE_PREFABS = 9;  
        //macros subtypes
        public const byte K_MACRO_DOOR_LOCKED = 0; //usa incuiata care poate fi sparta
        public const byte K_MACRO_DOOR_UNLOCKED = 1; //usa descuiata care poate fi sparta sau deschisa
        public const byte K_MACRO_DOOR_METALLIC = 2; //usa metalica ce nu poate fi breached
        public const byte K_MACRO_KEYCARD_RED = 3; //calculatorul care deschide usa metalica
        public const byte K_MACRO_FRONT_TEAM_DOOR = 4; //usa inchisa care asteapta echipa
        public const byte K_MACRO_FRONT_SOLO_STAIRS = 5;
        public const byte K_MACRO_FRONT_SOLO_DOOR = 6; 
        public const byte K_MACRO_SPAWNPOINT = 7; //checkpoint
        public const byte K_MACRO_WINDOW_PROFILE = 8;
        public const byte K_MACRO_WINDOW_PROFILE_HORIZONTAL = 9;
        public const byte K_MACRO_KEYCARD_GOLD = 10;

        public int g_brushMode = BRUSH_MODE_TILES; //tipul brushului
        public int g_brushValue = -1;                //valoare brush folosita mai mult la Misc objects

        //cand se selecteaza zone sau dreptunghiuri (cand faci collision boxes sau lumini gen area)
        public RectangleF g_SelectedArea;
        public Rectangle g_SelectedAreaTL; //tile-urile cuprinse de selectedArea
        //comanda selectia pe orice mod
        public const int K_SEL_STATUS_EMPTY = 0; //nimic
        public const int K_SEL_STATUS_START = 1; //setezi asta ca sa ceri inceput de selectie
        public const int K_SEL_STATUS_DRAGGING = 2;
        public int g_selectingStatus = K_SEL_STATUS_EMPTY;
        //variabile din material editor
        public Rectangle g_matBrush = new Rectangle(0, 0, 0, 0);
        public int TILESET_COLUMNS = 0;

        public void SetMaterialData(int tileW, int tileH, int nOldTilesetColumns, int nTilesetColumns)
        {
            //vede daca s-a schimbat marime tileset si daca da reindexeaza id-uri tiles
            if ((TILESET_COLUMNS > 0) && (nOldTilesetColumns != nTilesetColumns))
            {
                MessageBox.Show("Tileset size changed! Reindexing tile IDs!", "Warning", MessageBoxButtons.OK);

                int OLD_TILESET_COLUMNS = nOldTilesetColumns;
                TILESET_COLUMNS = nTilesetColumns;
                //deseneaza tabla de joc
                for (int kk = 0; kk < gMap.Blocks.Count; kk++)
                {
                    CTileBlock tb = gMap.Blocks[kk] as CTileBlock;

                    for (int layer = 0; layer < (int)ELayer.LAYERS_CNT; layer++)
                    {
                        for (int yy = 0; yy < BLOCK_H; yy++)
                        {
                            for (int xx = 0; xx < BLOCK_W; xx++)
                            {
                                int tileID = tb.tiles[xx, yy].tileID[layer];
                                if (tileID < 0)
                                    continue;
                                //new tileid
                                int tlsX = tileID % OLD_TILESET_COLUMNS;
                                int tlsY = tileID / OLD_TILESET_COLUMNS;
                                tb.tiles[xx, yy].tileID[layer] = tlsY * TILESET_COLUMNS + tlsX;
                            }
                        }
                    }
                }
            }
            //salvam noile date
            TILE_WIDTH = tileW;
            TILE_HEIGHT = tileH;
            TILE_HWIDTH = TILE_WIDTH / 2;
            TILE_HHEIGHT = TILE_HEIGHT / 2;
            TILESET_COLUMNS = nTilesetColumns;
            //notify map too
            gMap.nTileW = TILE_WIDTH;
            gMap.nTileH = TILE_HEIGHT;
            gMap.nTilesetColumns = nTilesetColumns;
        }

        public void GetMaterialData(out int tileW, out int tileH)
        {
            tileW = TILE_WIDTH;
            tileH = TILE_HEIGHT;
        }

        public void SetMaterialBrush(Rectangle brushRect)
        {
            g_matBrush = brushRect;
        }

        public int startXtl = -1, startYtl = -1;
        public int endXtl = -1, endYtl = -1;

        public Form1()
        {
            InitializeComponent();

            g_wndMaterials = new MaterialsWnd(this);
            g_wndLights = new LightsWnd(this);
            g_wndCollisions = new CollisionWnd(this);
            g_wndObjects = new ObjectsWnd(this);
            g_wndAI = new AIwnd(this);
            g_wndMisc = new MiscWnd(this);
            g_wndActors = new ActorsWnd(this);
            g_wndPrefabs = new PrefabsWnd(this);

            //link sprites
            g_sprObjects = g_wndObjects.GetSpriteLoader();
            g_sprLights = g_wndLights.GetSpriteLoader();
            g_sprActors = g_wndActors.GetSpriteLoader();
            //link picture box data
            pbImg = new Bitmap(pictureBox1.Width, pictureBox1.Height);
            pbGr = Graphics.FromImage(pbImg);
            pictureBox1.Image = pbImg;
            pbGr.SmoothingMode = System.Drawing.Drawing2D.SmoothingMode.None;
            pbGr.InterpolationMode = InterpolationMode.NearestNeighbor;
            pbGr.PixelOffsetMode = PixelOffsetMode.HighQuality;

            tilesImg = new Bitmap(GetType(), "tiles.png");

            layers_checkboxes = new CheckBox[] { chk_layer0, chk_layer1, chk_layer2, chk_layer3, chk_layer4, chk_layer5, chk_layer6 };
            layers_radios = new RadioButton[] { radio_layer0, radio_layer1, radio_layer2, radio_layer3, radio_layer4, radio_layer5, radio_layer6 };

            ResetLevel();
            //afisez fereastra
            butWndMaterials_Click(this, null);

            SetStatusBarMessage("Copyright 2018 PixelShard - www.pixelshard.com");

            PaintMap();
        }

        //--- utilities ---
        public RectangleF WorldToScreen(RectangleF inRect)
        {
            return new RectangleF((inRect.X - cameraPos.X) * zoomLevel, (inRect.Y - cameraPos.Y) * zoomLevel, inRect.Width * zoomLevel, inRect.Height * zoomLevel);
        }

        public Rectangle WorldToScreen(Rectangle inRect)
        {
            return new Rectangle((int)((inRect.X - cameraPos.X) * zoomLevel), (int)((inRect.Y - cameraPos.Y) * zoomLevel), (int)(inRect.Width * zoomLevel), (int)(inRect.Height * zoomLevel));
        }

        public PointF WorldToScreen(PointF inPoint)
        {
            return new PointF((inPoint.X - cameraPos.X) * zoomLevel, (inPoint.Y - cameraPos.Y) * zoomLevel);
        }
        public PointF ScreenToWorld(PointF inPoint)
        {
            return new PointF((inPoint.X * (1.0f / zoomLevel) + cameraPos.X), (inPoint.Y * (1.0f / zoomLevel) + cameraPos.Y));
        }

        public RectangleF NormalizeRect(RectangleF inRect)
        {
            RectangleF outr = new RectangleF();
            outr = inRect;
            if (inRect.Width < 0.0f)
            {
                outr.X = inRect.X + inRect.Width;
                outr.Width = -inRect.Width;
            }
            if (inRect.Height < 0.0f)
            {
                outr.Y = inRect.Y + inRect.Height;
                outr.Height = -inRect.Height;
            }
            return outr;
        }
        public Rectangle NormalizeRect(Rectangle inRect)
        {
            Rectangle outr = new Rectangle();
            outr = inRect;
            if (inRect.Width < 0)
            {
                outr.X = inRect.X + inRect.Width;
                outr.Width = -inRect.Width;
            }
            if (inRect.Height < 0)
            {
                outr.Y = inRect.Y + inRect.Height;
                outr.Height = -inRect.Height;
            }
            return outr;
        }

        public Rectangle RectFtoRect(RectangleF inrect)
        {
            return new Rectangle((int)inrect.X, (int)inrect.Y, (int)inrect.Width, (int)inrect.Height);
        }

        public bool PointInRect(Point pt, Rectangle rect)
        {
            if ((pt.X < rect.Left) || (pt.Y < rect.Top) || (pt.X > rect.Right) || (pt.Y > rect.Bottom))
                return false;

            return true;
        }

        public bool PointInRect(Point pt, RectangleF rectf)
        {
            Rectangle rect = RectFtoRect(rectf);
            if ((pt.X < rect.Left) || (pt.Y < rect.Top) || (pt.X > rect.Right) || (pt.Y > rect.Bottom))
                return false;

            return true;
        }

        public UInt16 GrabbedScalingCorners(Point mousePt, RectangleF rect)
        {
            float cornersize = K_CORNER_SIZE / zoomLevel;
            if (PointInRect(mousePt, new RectangleF(rect.X, rect.Y, cornersize, cornersize)))
            {
                return K_SCALE_FLAG_X | K_SCALE_FLAG_Y;
            }
            if (PointInRect(mousePt, new RectangleF(rect.X + rect.Width - cornersize, rect.Y, cornersize, cornersize)))
            {
                return K_SCALE_FLAG_W | K_SCALE_FLAG_Y;
            }
            if (PointInRect(mousePt, new RectangleF(rect.X, rect.Y + rect.Height - cornersize, cornersize, cornersize)))
            {
                return K_SCALE_FLAG_X | K_SCALE_FLAG_H;
            }
            if (PointInRect(mousePt, new RectangleF(rect.X + rect.Width - cornersize, rect.Y + rect.Height - cornersize, cornersize, K_CORNER_SIZE)))
            {
                return K_SCALE_FLAG_W | K_SCALE_FLAG_H;
            }

            if (PointInRect(mousePt, new RectangleF(rect.X + rect.Width / 2 - cornersize / 2, rect.Y, cornersize, cornersize)))
            {
                return K_SCALE_FLAG_Y;
            }
            if (PointInRect(mousePt, new RectangleF(rect.X + rect.Width / 2 - cornersize / 2, rect.Bottom - cornersize, cornersize, cornersize)))
            {
                return K_SCALE_FLAG_H;
            }
            if (PointInRect(mousePt, new RectangleF(rect.X, rect.Y + rect.Height / 2 - cornersize / 2, cornersize, cornersize)))
            {
                return K_SCALE_FLAG_X;
            }
            if (PointInRect(mousePt, new RectangleF(rect.Right - cornersize, rect.Y + rect.Height / 2 - cornersize / 2, cornersize, cornersize)))
            {
                return K_SCALE_FLAG_W;
            }

            //daca e in interior e move
            if (PointInRect(mousePt, rect))
                return K_SCALE_FLAG_MOVE;
            return 0;
        }

        public void DrawScalableRect(Pen pn, Rectangle rect)
        {
            pbGr.DrawRectangle(pn, rect);
            
            pbGr.DrawRectangle(pn, rect.X, rect.Y, K_CORNER_SIZE, K_CORNER_SIZE);
            pbGr.DrawRectangle(pn, rect.X + rect.Width - K_CORNER_SIZE, rect.Y, K_CORNER_SIZE, K_CORNER_SIZE);
            pbGr.DrawRectangle(pn, rect.X, rect.Y + rect.Height - K_CORNER_SIZE, K_CORNER_SIZE, K_CORNER_SIZE);
            pbGr.DrawRectangle(pn, rect.X + rect.Width - K_CORNER_SIZE, rect.Y + rect.Height - K_CORNER_SIZE, K_CORNER_SIZE, K_CORNER_SIZE);
            //middle
            pbGr.DrawRectangle(pn, rect.X + rect.Width / 2 - K_CORNER_SIZE / 2, rect.Y, K_CORNER_SIZE, K_CORNER_SIZE);
            pbGr.DrawRectangle(pn, rect.X + rect.Width / 2 - K_CORNER_SIZE / 2, rect.Bottom - K_CORNER_SIZE, K_CORNER_SIZE, K_CORNER_SIZE);
            pbGr.DrawRectangle(pn, rect.X, rect.Y + rect.Height / 2 - K_CORNER_SIZE / 2, K_CORNER_SIZE, K_CORNER_SIZE);
            pbGr.DrawRectangle(pn, rect.Right - K_CORNER_SIZE, rect.Y + rect.Height / 2 - K_CORNER_SIZE / 2, K_CORNER_SIZE, K_CORNER_SIZE);
        }
        public void DrawScalableRect(Pen pn, RectangleF rect)
        {
            pbGr.DrawRectangle(pn, rect.X, rect.Y, rect.Width, rect.Height);

            pbGr.DrawRectangle(pn, rect.X, rect.Y, K_CORNER_SIZE, K_CORNER_SIZE);
            pbGr.DrawRectangle(pn, rect.X + rect.Width - K_CORNER_SIZE, rect.Y, K_CORNER_SIZE, K_CORNER_SIZE);
            pbGr.DrawRectangle(pn, rect.X, rect.Y + rect.Height - K_CORNER_SIZE, K_CORNER_SIZE, K_CORNER_SIZE);
            pbGr.DrawRectangle(pn, rect.X + rect.Width - K_CORNER_SIZE, rect.Y + rect.Height - K_CORNER_SIZE, K_CORNER_SIZE, K_CORNER_SIZE);
            //middle
            pbGr.DrawRectangle(pn, rect.X + rect.Width / 2 - K_CORNER_SIZE / 2, rect.Y, K_CORNER_SIZE, K_CORNER_SIZE);
            pbGr.DrawRectangle(pn, rect.X + rect.Width / 2 - K_CORNER_SIZE / 2, rect.Bottom - K_CORNER_SIZE, K_CORNER_SIZE, K_CORNER_SIZE);
            pbGr.DrawRectangle(pn, rect.X, rect.Y + rect.Height / 2 - K_CORNER_SIZE / 2, K_CORNER_SIZE, K_CORNER_SIZE);
            pbGr.DrawRectangle(pn, rect.Right - K_CORNER_SIZE, rect.Y + rect.Height / 2 - K_CORNER_SIZE / 2, K_CORNER_SIZE, K_CORNER_SIZE);
        }


        public void PaintMap(int mouseX = 0, int mouseY = 0)
        {
            if (pbGr == null)
                return;
            //cursor pos
            g_hoveredTile.X = (int)(mouseX * (1.0f / zoomLevel) + cameraPos.X) / TILE_HEIGHT;
            g_hoveredTile.Y = (int)(mouseY * (1.0f / zoomLevel) + cameraPos.Y) / TILE_HEIGHT;
            PointF cursorPosF = new PointF((mouseX * (1.0f / zoomLevel) + cameraPos.X), (mouseY * (1.0f / zoomLevel) + cameraPos.Y));
            Point cursorPos = new Point((int)cursorPosF.X, (int)cursorPosF.Y);

            pbGr.ResetTransform();
            //paint axis
            Pen pn1 = new Pen(Color.FromArgb(160, 160, 160));
            Pen pn2 = new Pen(Color.FromArgb(80, 80, 80));
            Pen pn3 = new Pen(Color.FromArgb(40, 40, 40));

            Brush aHatchBrush = new HatchBrush(HatchStyle.DarkDownwardDiagonal, Color.Red);
            Pen penDotted = new Pen(aHatchBrush);
            aHatchBrush = new HatchBrush(HatchStyle.DarkDownwardDiagonal, Color.FromArgb(0, 255, 0), Color.FromArgb(0, 0, 255));
            Pen penDottedGr1 = new Pen(aHatchBrush);

            Font arial10b = new Font("Arial", 10, FontStyle.Bold);
            Rectangle AABBlevel = new Rectangle();

            if (invertBackground)
            {
                pbGr.Clear(Color.FromArgb(160, 160, 160));

                pn1 = new Pen(Color.FromArgb(20, 20, 20));
                pn2 = new Pen(Color.FromArgb(80, 80, 80));
                pn3 = new Pen(Color.FromArgb(120, 120, 120));
            }
            else
            {
                pbGr.Clear(Color.FromArgb(20, 20, 20));
            }

            //paint origin
            Pen pngr = new Pen(Brushes.Brown, 3);
            pbGr.DrawLine(pngr, 0, (gLevelOrigin.Y - cameraPos.Y) * zoomLevel, pictureBox1.Width, (gLevelOrigin.Y - cameraPos.Y) * zoomLevel);
            pbGr.DrawLine(pngr, (gLevelOrigin.X - cameraPos.X) * zoomLevel, 0, (gLevelOrigin.X - cameraPos.X) * zoomLevel, pictureBox1.Height);
            //grid sub tiles
            if ((showGridToolStripMenuItem.Checked) && (!frontLayerGridToolStripMenuItem.Checked))
            {
                //grid mic tiles
                if (zoomLevel >= 1.0f)
                {
                    for (int kk = 0; kk < 2 + (int)((float)(pictureBox1.Width / (TILE_WIDTH)) * (1.0f / zoomLevel)); kk++)
                    {
                        float x = zoomLevel * (kk * TILE_WIDTH - cameraPos.X % (TILE_WIDTH));
                        pbGr.DrawLine(pn3, x, 0, x, pictureBox1.Height);
                    }
                    for (int kk = 0; kk < 2 + (int)((float)(pictureBox1.Height / (TILE_HEIGHT)) * (1.0f / zoomLevel)); kk++)
                    {
                        float y = zoomLevel * (kk * TILE_HEIGHT - cameraPos.Y % (TILE_HEIGHT));
                        pbGr.DrawLine(pn3, 0, y, pictureBox1.Width, y);
                    }
                }
                //axele verticale si blocks
                for (int kk = 0; kk < 2 + (int)((float)(pictureBox1.Width / (BLOCK_W * TILE_WIDTH)) * (1.0f / zoomLevel)); kk++)
                {
                    float x = zoomLevel * (kk * BLOCK_W * TILE_WIDTH - cameraPos.X % (BLOCK_W * TILE_WIDTH));
                    pbGr.DrawLine(pn2, x, 0, x, pictureBox1.Height);
                }
                for (int kk = 0; kk < 2 + (int)((float)(pictureBox1.Height / (BLOCK_H * TILE_HEIGHT)) * (1.0f / zoomLevel)); kk++)
                {
                    float y = zoomLevel * (kk * BLOCK_H * TILE_HEIGHT - cameraPos.Y % (BLOCK_H * TILE_HEIGHT));
                    pbGr.DrawLine(pn2, 0, y, pictureBox1.Width, y);
                }
            }

            if (g_wndMaterials.g_TilesetImg == null)
            {
                pbGr.DrawString("Load a tileset in the materials window or open a saved level!", new Font("Arial", 10), Brushes.Green, 10, 10);
            }

            pbGr.ScaleTransform(zoomLevel, zoomLevel);
            ///--- deseneaza nivelul
            if (g_wndMaterials.g_TilesetImg != null)
            {
                Int32 blminx = 100000, blminy = 100000, blmaxx = -100000, blmaxy = -100000;
                //deseneaza tabla de joc
                for (int kk = 0; kk < gMap.Blocks.Count; kk++)
                {
                    CTileBlock tb = gMap.Blocks[kk] as CTileBlock;
                    if (tb.pos.X < blminx) blminx = tb.pos.X;
                    if (tb.pos.Y < blminy) blminy = tb.pos.Y;
                    if (tb.pos.X > blmaxx) blmaxx = tb.pos.X;
                    if (tb.pos.Y > blmaxy) blmaxy = tb.pos.Y;

                    //daca nu sunt in ecran nu le deseneaza
                    if (((tb.pos.X + 1) * BLOCK_W * TILE_WIDTH < cameraPos.X) || ((tb.pos.Y + 1) * BLOCK_H * TILE_HEIGHT < cameraPos.Y) ||
                       tb.pos.X * BLOCK_W * TILE_WIDTH > cameraPos.X + (int)(pictureBox1.Width * (1.0f / zoomLevel)) ||
                       tb.pos.Y * BLOCK_H * TILE_HEIGHT > cameraPos.Y + (int)(pictureBox1.Height * (1.0f / zoomLevel)) )
                            continue;

                    //paint block image all at once
                    pbGr.DrawImage(tb.layerImg, tb.pos.X * BLOCK_W * TILE_WIDTH - cameraPos.X, tb.pos.Y * BLOCK_H * TILE_HEIGHT - cameraPos.Y);
                    //deseneaza patratele rosii pe tile-urile care se suprapun
                    if ((chk_showOverlappingTiles.Checked) && (g_brushMode == BRUSH_MODE_TILES))
                    {
                        for (int yy = 0; yy < BLOCK_H; yy++)
                        {
                            for (int xx = 0; xx < BLOCK_W; xx++)
                            {
                                int cnt = 0;
                                for (int lay = 0; lay < (int)ELayer.LAYERS_CNT; lay++)
                                {
                                    if (tb.tiles[xx, yy].tileID[lay] >= 0)
                                        cnt++;
                                }

                                if (cnt > 1)
                                {
                                    Brush fillbr = new SolidBrush(Color.FromArgb(cnt * 64, Color.Red));

                                    pbGr.FillRectangle(fillbr, new RectangleF(tb.pos.X * BLOCK_W * TILE_WIDTH + xx * TILE_WIDTH - cameraPos.X,
                                    tb.pos.Y * BLOCK_H * TILE_HEIGHT + yy * TILE_HEIGHT - cameraPos.Y, TILE_WIDTH, TILE_HEIGHT));
                                }
                            }
                        }
                    }
                }

                // set level bbox
                AABBlevel = new Rectangle(blminx * TILE_WIDTH * BLOCK_W - 1, 
                    blminy * TILE_HEIGHT * BLOCK_H - 1, 
                    ((blmaxx - blminx + 1) * TILE_WIDTH * BLOCK_W + 2), 
                    ((blmaxy - blminy + 1) * TILE_HEIGHT * BLOCK_H + 2));

            }


            //only target of selected element gets painted
            PointF vTargetSrc = new PointF(0.0f, 0.0f);
            PointF vTargetDst = new PointF(0.0f, 0.0f);

            ///---paint objects---
            if (g_sprObjects != null)
            {
                RectangleF scrrect = new RectangleF(cameraPos.X, cameraPos.Y, cameraPos.X + (int)(pictureBox1.Width * (1.0f / zoomLevel)), cameraPos.Y + (int)(pictureBox1.Height * (1.0f / zoomLevel)));

                for (int kk = 0; kk < arrObjects.Count; kk++)
                {
                    CObject obj = arrObjects[kk] as CObject;
                    //ascunde obj in fn de layer
                    if (!layers_checkboxes[obj.layer].Checked)
                        continue;
                    //daca nu e in ecran nu il deseneaza
                    if (!scrrect.Contains(obj.pos))
                        continue;
                    //desenez obiect
                    if ((obj.animIdx == -1) || (obj.frameIdx == -1))
                    {
                        RectangleF objbbox = new RectangleF(-10, -10, 20, 20);
                        objbbox.X += obj.pos.X - cameraPos.X; objbbox.Y += obj.pos.Y - cameraPos.Y;
                        Pen np = new Pen(Color.DarkRed, 1);
                        pbGr.DrawRectangle(np, objbbox.X, objbbox.Y, objbbox.Width, objbbox.Height);
                        pbGr.DrawString("NoAnimObj!!!", new Font("Arial", 6.0f), Brushes.Red, obj.pos.X - cameraPos.X, obj.pos.Y - cameraPos.Y);
                        continue;
                    }

                    BSXAnimBrowser.Frame fr = g_sprObjects.anims[obj.animIdx].aframes[obj.frameIdx].frame;
                    if ((obj.flags & OBJFLAG_FLIPXORY) != 0)
                    {
                        int offx = 0;
                        if ((obj.flags & OBJFLAG_FLIPX) != 0) //la flipX nu pastreaza in acelasi bbox pt ca strica animatiile flipate
                        {
                            Rectangle flipbbox = fr.BBox_real;
                            offx = -2 * (flipbbox.X + flipbbox.Width / 2);
                        }
                        fr.PaintFlip(pbGr, (float)obj.pos.X + offx - cameraPos.X, (float)obj.pos.Y - cameraPos.Y, (obj.flags & OBJFLAG_FLIPX) != 0, (obj.flags & OBJFLAG_FLIPY) != 0);
                    }
                    else
                    {
                        fr.Paint(pbGr, (float)obj.pos.X - cameraPos.X, (float)obj.pos.Y - cameraPos.Y);
                    }
                    //paint cover icon
                    if ((obj.flags & OBJFLAG_IS_COVER) != 0)
                    {
                        Rectangle flipbbox = fr.BBox_real;
                        int offx = 0;
                        if((obj.flags & OBJFLAG_FLIPX) != 0)
                            offx = -2 * (flipbbox.X + flipbbox.Width / 2);
                        pbGr.DrawString("C", new Font("Arial", 6, FontStyle.Bold), Brushes.GreenYellow, (float)obj.pos.X + flipbbox.Y + offx + flipbbox.Width/2 - cameraPos.X, (float)obj.pos.Y + flipbbox.Y - cameraPos.Y + flipbbox.Height/2);
                    }
                    //desenez dreptunghiuri colorate pe elementele active
                    if (g_brushMode == BRUSH_MODE_AI)
                    {
                        if ((obj.logic.bCanInteract) || (obj.logic.strAIname.Length > 0) || (obj.logic.strActions.Length > 0) || (obj.logic.targetID >= 0))
                        {
                            RectangleF objbbox = fr.BBox_real;
                            objbbox.X += obj.pos.X - cameraPos.X; objbbox.Y += obj.pos.Y - cameraPos.Y;
                            objbbox.Inflate(2, 2);
                            Pen np = new Pen(Color.Green, 2);
                            pbGr.DrawRectangle(np, objbbox.X, objbbox.Y, objbbox.Width, objbbox.Height);
                        }
                        //target link
                        if (obj == g_selectedObject)
                        {
                            if (obj.logic.targetID >= 0)
                            {
                                vTargetDst = GetTargetPosByID(obj.logic.targetID);
                                vTargetSrc = GetTargetPosByID((int)obj.ID);
                            }
                        }
                    }
                    //desenez bbox pe selectie
                    if (g_selectedObject == obj)
                    {
                        RectangleF objbbox = fr.BBox_real;
                        int offx = 0;
                        if ((obj.flags & OBJFLAG_FLIPX) != 0) //la flipX nu pastreaza in acelasi bbox pt ca strica animatiile flipate
                        {
                            Rectangle flipbbox = fr.BBox_real;
                            offx = -2 * (flipbbox.X + flipbbox.Width / 2);
                        }
                        objbbox.X += offx;

                        objbbox.X += obj.pos.X - cameraPos.X; objbbox.Y += obj.pos.Y - cameraPos.Y;
                        Pen np = new Pen(Color.DarkRed, 1);
                        pbGr.DrawRectangle(np, objbbox.X, objbbox.Y, objbbox.Width, objbbox.Height);
                    }
                }
                ///--- ACTORS ---
                if (!g_wndActors.g_hideActors)
                {
                    for (int kk = 0; kk < arrActors.Count; kk++)
                    {
                        CActor act = arrActors[kk] as CActor;
                        //daca nu e in ecran nu il deseneaza
                        if (!scrrect.Contains(act.pos))
                            continue;
                        //no animation set?
                        if (act.animIdx < 0)
                        {
                            RectangleF objbbox = new RectangleF(-10, -10, 20, 20);
                            objbbox.X += act.pos.X - cameraPos.X; objbbox.Y += act.pos.Y - cameraPos.Y;
                            Pen np = new Pen(Color.DarkRed, 1);
                            pbGr.DrawRectangle(np, objbbox.X, objbbox.Y, objbbox.Width, objbbox.Height);
                            pbGr.DrawString("NoAnimActor!!!", new Font("Arial", 6.0f), Brushes.Red, act.pos.X - cameraPos.X, act.pos.Y - cameraPos.Y);
                            continue;
                        }
                        //paint actor
                        BSXAnimBrowser.Frame fr = g_sprActors.anims[act.animIdx].aframes[0].frame;
                        if (act.bLookLeft)
                        {
                            fr.PaintFlip(pbGr, (float)act.pos.X - cameraPos.X, (float)act.pos.Y - cameraPos.Y, act.bLookLeft, false);
                        }
                        else
                        {
                            fr.Paint(pbGr, (float)act.pos.X - cameraPos.X, (float)act.pos.Y - cameraPos.Y);
                        }
                        //apare H pe el daca e ascuns
                        if (act.logic.bStartHidden)
                        {
                            pbGr.DrawString("H", new Font("Arial", 8), Brushes.Red, act.pos.X - cameraPos.X, act.pos.Y - cameraPos.Y);
                        }
                        //desenez bbox pe selectie
                        if (g_selectedActor == act)
                        {
                            RectangleF objbbox = fr.BBox_real;
                            objbbox.X += act.pos.X - cameraPos.X; objbbox.Y += act.pos.Y - cameraPos.Y;
                            Pen np = new Pen(Color.DarkRed, 1);
                            pbGr.DrawRectangle(np, objbbox.X, objbbox.Y, objbbox.Width, objbbox.Height);
                        }
                        //desenez unghiul
                        if (act.bSetAngle)
                        {
                            RectangleF objbbox = fr.BBox_real;
                            objbbox.X += act.pos.X - cameraPos.X; objbbox.Y += act.pos.Y - cameraPos.Y;
                            //angle
                            PointF scrcenter = new PointF(objbbox.X + objbbox.Width / 2.0f, objbbox.Y + objbbox.Height / 2.0f);
                            PointF scrangle = new PointF(scrcenter.X + (objbbox.Width) * (float)Math.Cos((act.fAngle / 360.0f) * 2.0f * Math.PI), scrcenter.Y + (objbbox.Height) * (float)Math.Sin((act.fAngle / 360.0f) * 2.0f * Math.PI));
                            pbGr.DrawLine(Pens.DarkRed, scrcenter, scrangle);
                        }

                        //desenez dreptunghiuri colorate pe elementele active
                        if (g_brushMode == BRUSH_MODE_AI)
                        {
                            if ((act.logic.bCanInteract) || (act.logic.strAIname.Length > 0) || (act.logic.strActions.Length > 0) || (act.logic.targetID >= 0))
                            {
                                RectangleF objbbox = fr.BBox_real;
                                objbbox.X += act.pos.X - cameraPos.X; objbbox.Y += act.pos.Y - cameraPos.Y;
                                objbbox.Inflate(2, 2);
                                Pen np = new Pen(Color.Green, 2);
                                pbGr.DrawRectangle(np, objbbox.X, objbbox.Y, objbbox.Width, objbbox.Height);
                            }
                            //target link
                            if (act == g_selectedActor)
                            {
                                if (act.logic.targetID >= 0)
                                {
                                    vTargetDst = GetTargetPosByID(act.logic.targetID);
                                    vTargetSrc = GetTargetPosByID((int)act.ID);
                                }
                            }
                        }
                    }
                }
                //deseneaza obiectul de pe cursor
                if (g_brushMode == BRUSH_MODE_OBJECTS)
                {
                    if ((g_selectedObject == null) && (g_wndObjects.g_selectedAnim >= 0) && (g_wndObjects.g_selectedFrame >= 0))
                    {
                        Point objcur = new Point((int)cursorPos.X, (int)cursorPos.Y);
                        if (chk_snapToGrid.Checked)
                        {
                            objcur = SnapPointToGrid(objcur, K_SNAP_THRESHOLD);
                        }

                        g_sprObjects.anims[g_wndObjects.g_selectedAnim].aframes[g_wndObjects.g_selectedFrame].frame.Paint(pbGr, (float)objcur.X - cameraPos.X, (float)objcur.Y - cameraPos.Y);
                    }
                }
                else if (g_brushMode == BRUSH_MODE_ACTORS)
                {
                    if (g_selectedActor == null)
                    {
                        CActor tmpact = new CActor();
                        g_wndActors.SetActorTemplate(tmpact);
                        if (tmpact.animIdx >= 0)
                        {
                            Point objcur = new Point((int)cursorPos.X, (int)cursorPos.Y);
                            if (chk_snapToGrid.Checked)
                            {
                                objcur = SnapPointToGrid(objcur, K_SNAP_THRESHOLD);
                            }

                            g_sprActors.anims[tmpact.animIdx].aframes[0].frame.Paint(pbGr, (float)objcur.X - cameraPos.X, (float)objcur.Y - cameraPos.Y);
                        }
                    }
                }
                else if (g_brushMode == BRUSH_MODE_MACRO)
                {
                    Point objcur = cursorPos;
                    if (chk_snapToGrid.Checked)
                        objcur = SnapPointToGrid(cursorPos, K_SNAP_THRESHOLD);

                    switch (g_brushValue)
                    {
                        case K_MACRO_WINDOW_PROFILE_HORIZONTAL:
                            {
                                int anmidx = g_sprObjects.GetAnimIdxByName("WINDOWS_SECTION");
                                int frameidx = 4;
                                if (anmidx >= 0)
                                {
                                    g_sprObjects.anims[anmidx].aframes[frameidx].frame.Paint(pbGr, (float)objcur.X - cameraPos.X, (float)objcur.Y - cameraPos.Y);
                                }
                            }
                            break;
                        case K_MACRO_WINDOW_PROFILE:
                            {
                                int anmidx = g_sprObjects.GetAnimIdxByName("WINDOWS_SECTION");
                                int frameidx = 0;
                                if (anmidx >= 0)
                                {
                                    g_sprObjects.anims[anmidx].aframes[frameidx].frame.Paint(pbGr, (float)objcur.X - cameraPos.X, (float)objcur.Y - cameraPos.Y);
                                }
                            }
                            break;
                        case K_MACRO_DOOR_LOCKED:
                        case K_MACRO_DOOR_UNLOCKED:
                            {
                                int anmidx = g_sprObjects.GetAnimIdxByName("DOORS_SECTION");
                                int frameidx = 0;
                                if (g_brushValue == K_MACRO_DOOR_LOCKED)
                                    frameidx = 4;
                                if (anmidx >= 0)
                                {
                                    g_sprObjects.anims[anmidx].aframes[frameidx].frame.Paint(pbGr, (float)objcur.X - cameraPos.X, (float)objcur.Y - cameraPos.Y);
                                }
                            }
                            break;
                        case K_MACRO_DOOR_METALLIC:
                            {
                                int anmidx = g_sprObjects.GetAnimIdxByName("DOOR_SLIDING_LOCKED");
                                int frameidx = 0;
                                if (anmidx >= 0)
                                {
                                    g_sprObjects.anims[anmidx].aframes[frameidx].frame.Paint(pbGr, (float)objcur.X - cameraPos.X, (float)objcur.Y - cameraPos.Y);
                                }
                            }
                            break;
                        case K_MACRO_KEYCARD_RED:
                            {
                                int anmidx = g_sprObjects.GetAnimIdxByName("KEYCARD_RED");
                                int frameidx = 0;
                                if (anmidx >= 0)
                                {
                                    g_sprObjects.anims[anmidx].aframes[frameidx].frame.Paint(pbGr, (float)objcur.X - cameraPos.X, (float)objcur.Y - cameraPos.Y);
                                }
                            }
                            break;
                        case K_MACRO_KEYCARD_GOLD:
                            {
                                int anmidx = g_sprObjects.GetAnimIdxByName("KEYCARD_YELLOW");
                                int frameidx = 0;
                                if (anmidx >= 0)
                                {
                                    g_sprObjects.anims[anmidx].aframes[frameidx].frame.Paint(pbGr, (float)objcur.X - cameraPos.X, (float)objcur.Y - cameraPos.Y);
                                }
                            }
                            break;
                        case K_MACRO_FRONT_SOLO_STAIRS:
                            {
                                int anmidx = g_sprObjects.GetAnimIdxByName("DOORS_PORTALS");
                                int frameidx = 0;
                                if (anmidx >= 0)
                                {
                                    g_sprObjects.anims[anmidx].aframes[frameidx].frame.Paint(pbGr, (float)objcur.X - cameraPos.X, (float)objcur.Y - cameraPos.Y);
                                }
                            }
                            break;
                        case K_MACRO_FRONT_SOLO_DOOR:
                            {
                                int anmidx = g_sprObjects.GetAnimIdxByName("DOORS_FRONT");
                                int frameidx = 14;
                                if (anmidx >= 0)
                                {
                                    g_sprObjects.anims[anmidx].aframes[frameidx].frame.Paint(pbGr, (float)objcur.X - cameraPos.X, (float)objcur.Y - cameraPos.Y);
                                }
                            }
                            break;
                        case K_MACRO_FRONT_TEAM_DOOR:
                            {
                                int anmidx = g_sprObjects.GetAnimIdxByName("DOORS_FRONT");
                                int frameidx = 0;
                                if (anmidx >= 0)
                                {
                                    g_sprObjects.anims[anmidx].aframes[frameidx].frame.Paint(pbGr, (float)objcur.X - cameraPos.X, (float)objcur.Y - cameraPos.Y);
                                }
                            }
                            break;
                        case K_MACRO_SPAWNPOINT:
                            {
                                int anmidx = g_sprObjects.GetAnimIdxByName("CHECKPOINT_OFF");
                                int frameidx = 0;
                                if (anmidx >= 0)
                                {
                                    g_sprObjects.anims[anmidx].aframes[frameidx].frame.Paint(pbGr, (float)objcur.X - cameraPos.X, (float)objcur.Y - cameraPos.Y);
                                }
                            }
                            break;
                    }
                }
                else if (g_brushMode == BRUSH_MODE_PREFABS)
                {
                    PointF vPos = new PointF((float)cursorPos.X, (float)cursorPos.Y);
                    if (chk_snapToGrid.Checked)
                        vPos = SnapPointToGrid(vPos, K_SNAP_THRESHOLD);

                    string strPrefabName = g_wndPrefabs.GetSelectedPrefabName();
                    pbGr.ResetTransform();
                    vPos = WorldToScreen(vPos);

                    pbGr.DrawLine(Pens.LightGreen, vPos.X - 15.0f, vPos.Y, vPos.X + 15.0f, vPos.Y);
                    pbGr.DrawLine(Pens.LightGreen, vPos.X, vPos.Y - 15.0f, vPos.X, vPos.Y + 15.0f);
                    pbGr.DrawString(strPrefabName, arial10b, new SolidBrush(Color.Red), vPos.X + 15, vPos.Y - 25);

                }
                //deseneaza id obiect selectat
                if (g_selectedObject != null)
                {
                    pbGr.ResetTransform();
                    RectangleF bbox = new RectangleF(-10.0f, -10.0f, 20.0f, 20.0f);
                    if ((g_selectedObject.animIdx >= 0) && (g_selectedObject.frameIdx >= 0))
                    {
                        BSXAnimBrowser.Frame fr = g_sprObjects.anims[g_selectedObject.animIdx].aframes[g_selectedObject.frameIdx].frame;
                        bbox = fr.BBox;
                    }
                    bbox.X += g_selectedObject.pos.X; bbox.Y += g_selectedObject.pos.Y;
                    bbox = WorldToScreen(bbox);
                    pbGr.DrawString(g_selectedObject.ID.ToString(), arial10b, new SolidBrush(Color.Black), bbox.X + 1, bbox.Y - 15);
                    pbGr.DrawString(g_selectedObject.ID.ToString(), arial10b, new SolidBrush(Color.Red), bbox.X, bbox.Y - 16);
                }
            }
            //reset transform
            pbGr.ResetTransform();

            //grid over the tiles
            if ((showGridToolStripMenuItem.Checked) && (frontLayerGridToolStripMenuItem.Checked))
            {
                //grid mic tiles
                for (int kk = 0; kk < 2 + (int)((float)(pictureBox1.Width / (TILE_WIDTH)) * (1.0f / zoomLevel)); kk++)
                {
                    float x = zoomLevel * (kk * TILE_WIDTH - cameraPos.X % (TILE_WIDTH));
                    pbGr.DrawLine(pn3, x, 0, x, pictureBox1.Height);
                }
                for (int kk = 0; kk < 2 + (int)((float)(pictureBox1.Height / (TILE_HEIGHT)) * (1.0f / zoomLevel)); kk++)
                {
                    float y = zoomLevel * (kk * TILE_HEIGHT - cameraPos.Y % (TILE_HEIGHT));
                    pbGr.DrawLine(pn3, 0, y, pictureBox1.Width, y);
                }
                //axele verticale si blocks
                for (int kk = 0; kk < 2 + (int)((float)(pictureBox1.Width / (BLOCK_W * TILE_WIDTH)) * (1.0f / zoomLevel)); kk++)
                {
                    float x = zoomLevel * (kk * BLOCK_W * TILE_WIDTH - cameraPos.X % (BLOCK_W * TILE_WIDTH));
                    pbGr.DrawLine(pn2, x, 0, x, pictureBox1.Height);
                }
                for (int kk = 0; kk < 2 + (int)((float)(pictureBox1.Height / (BLOCK_H * TILE_HEIGHT)) * (1.0f / zoomLevel)); kk++)
                {
                    float y = zoomLevel * (kk * BLOCK_H * TILE_HEIGHT - cameraPos.Y % (BLOCK_H * TILE_HEIGHT));
                    pbGr.DrawLine(pn2, 0, y, pictureBox1.Width, y);
                }
            }

            ///--- Deseneaza luminile ---
            if ((g_brushMode == BRUSH_MODE_LIGHTS) || (g_brushMode == BRUSH_MODE_AI))
            {
                for (int kk = 0; kk < arrLights.Count; kk++)
                {
                    CLight light = arrLights[kk] as CLight;
                    Rectangle srcr = new Rectangle(32 * light.type, 0, 32, 32);
                    //draw light image
                    if ((g_wndLights.ShowLightsImage) && (g_brushMode == BRUSH_MODE_LIGHTS))
                    {
                        //deseneaza spot lumina
                        if ((light.animId >= 0) && (g_selectedLight == light))
                        {
                            if ((light.type == K_LIGHT_POINT) || (light.type == K_LIGHT_AREA))
                            {
                                Rectangle origrect = g_sprLights.anims[light.animId].aframes[0].frame.GetRect();
                                pbGr.ScaleTransform(zoomLevel, zoomLevel);
                                pbGr.TranslateTransform(light.pos.X - cameraPos.X, light.pos.Y - cameraPos.Y);
                                pbGr.RotateTransform(light.angle);
                                pbGr.ScaleTransform((light.area.Width / (float)origrect.Width), (light.area.Height / (float)origrect.Height));

                                g_sprLights.anims[light.animId].aframes[0].frame.Paint(pbGr, 0.0f, 0.0f);

                                pbGr.ResetTransform();
                            }
                        }
                    }


                    if (light == g_selectedLight)
                    {
                        srcr.X += 32 * K_LIGHTS_COUNT;
                        RectangleF scrrect = WorldToScreen(light.area);
                        if (light.type == K_LIGHT_POINT)
                        {
                            pbGr.DrawEllipse(Pens.Green, scrrect.X, scrrect.Y, scrrect.Width, scrrect.Height);
                        }
                        //zona activa
                        if(light.type != K_LIGHT_AMBIENTAL)
                            DrawScalableRect(Pens.Green, scrrect);
                        //angle
                        PointF scrcenter = new PointF(scrrect.X + scrrect.Width / 2.0f, scrrect.Y + scrrect.Height / 2.0f);
                        PointF scrangle = new PointF(scrcenter.X + (scrrect.Width / 2.0f) * (float)Math.Cos((light.angle / 360.0f) * 2.0f * Math.PI), scrcenter.Y + (scrrect.Height / 2.0f) * (float)Math.Sin((light.angle / 360.0f) * 2.0f * Math.PI));
                        pbGr.DrawLine(Pens.Green, scrcenter, scrangle);
                    }

                    //icons
                    PointF lpos = WorldToScreen(light.pos);
                    pbGr.DrawImage(tilesImg, lpos.X - 16.0f, lpos.Y - 16.0f, srcr, GraphicsUnit.Pixel);
                    //culoare lumina
                    pbGr.FillRectangle(new SolidBrush(light.color), lpos.X - 16, lpos.Y + 16, 32, 12);
                    //daca are logic part
                    if (g_brushMode == BRUSH_MODE_AI)
                    {
                        if ((light.logic.bCanInteract) || (light.logic.strAIname.Length > 0) || (light.logic.strActions.Length > 0) || (light.logic.targetID >= 0))
                        {
                            Pen np = new Pen(Color.Green, 2);
                            pbGr.DrawRectangle(np, lpos.X - 18, lpos.Y + 14, 36, 16);

                            if ((light == g_selectedLight) && (light.logic.targetID >= 0))
                            {
                                vTargetDst = GetTargetPosByID(light.logic.targetID);
                                vTargetSrc.X = light.pos.X; vTargetSrc.Y = light.pos.Y;
                            }
                        }
                    }
                    //ID
                    Color textcol = Color.FromArgb(light.color.ToArgb() ^ 0xffffff); // culoare inversata ca sa se vada pe orice culoare
                    String lightStr = light.ID.ToString();
                    if (light.castsShadows)
                        lightStr = "[" + lightStr + "]";

                    pbGr.DrawString(lightStr, arial10b, new SolidBrush(textcol), lpos.X - 16, lpos.Y + 16 - 2);
                }
            }

            ///--- collision rects ---
            if (g_brushMode == BRUSH_MODE_COLLISIONS)
            {
                //sa deseneze doar ce e in ecran
                for (int kk = 0; kk < arrCollisions.Count; kk++)
                {
                    CCollisionElement coll = arrCollisions[kk] as CCollisionElement;
                    Color col = g_arrCollColors[(int)coll.type];
                    //hide unwanted collision rects
                    if ((g_wndCollisions.HideWater) && (coll.type == K_COLL_TYPE_WATER))
                        continue;
                    if ((g_wndCollisions.HideFOW) && (coll.type == K_COLL_TYPE_ROOM_OCCLUDER))
                        continue;
                    if ((g_wndCollisions.HideTriggers) && (coll.type == K_COLL_TYPE_TRIGGER))
                        continue;

                    Brush fillbr = new SolidBrush(Color.FromArgb(128, col));

                    RectangleF rct = WorldToScreen(coll.rect);
                    pbGr.FillRectangle(fillbr, rct);
                    if(coll.type != K_COLL_TYPE_SOLID)
                        pbGr.DrawRectangle(new Pen(col), rct.X, rct.Y, rct.Width, rct.Height);

                    if (coll == g_selectedCollision)
                    {
                        DrawScalableRect(Pens.Cyan, rct);
                        //scrie dimensiuni si pozitie
                        pbGr.DrawString("ID:" + coll.ID + " xy:" + (coll.rect.X - CAMERA_ORIGIN.X) + "," + (coll.rect.Y - CAMERA_ORIGIN.Y) + "\nwh:" + coll.rect.Width + "," + coll.rect.Height, new Font("Arial", 8), Brushes.LightBlue, rct.X + 6, rct.Y + 6);
                    }
                }
                //selectie curenta
                if (g_bCreatingItem)
                {
                    RectangleF selrect = new RectangleF(g_SelectedArea.X, g_SelectedArea.Y, g_SelectedArea.Width, g_SelectedArea.Height);
                    selrect = WorldToScreen(NormalizeRect(selrect));
                    Brush fillbr = new SolidBrush(Color.FromArgb(128, Color.Red));
                    pbGr.FillRectangle(fillbr, RectFtoRect(selrect));
                }
            }
            else if (g_brushMode == BRUSH_MODE_AI)
            {
                //sa deseneze doar ce e in ecran
                for (int kk = 0; kk < arrCollisions.Count; kk++)
                {
                    CCollisionElement coll = arrCollisions[kk] as CCollisionElement;
                    Color col = g_arrCollColors[(int)coll.type];
                    //hide unwanted collision rects
                    if ((g_wndCollisions.HideWater) && (coll.type == K_COLL_TYPE_WATER))
                        continue;
                    if ((g_wndCollisions.HideFOW) && (coll.type == K_COLL_TYPE_ROOM_OCCLUDER))
                        continue;
                    if ((g_wndCollisions.HideTriggers) && (coll.type == K_COLL_TYPE_TRIGGER))
                        continue;

                    RectangleF rct = WorldToScreen(coll.rect);
                    if ((coll.logic.bCanInteract) || (coll.logic.strAIname.Length > 0) || (coll.logic.strActions.Length > 0) || (coll.logic.targetID >= 0))
                    {
                        Brush fillbr = new SolidBrush(Color.FromArgb(128, col));
                        pbGr.FillRectangle(fillbr, rct);
                    }
                    else
                        pbGr.DrawRectangle(new Pen(col), rct.X, rct.Y, rct.Width, rct.Height);


                    if (coll == g_selectedCollision)
                    {
                        DrawScalableRect(Pens.Cyan, rct);
                        //scrie dimensiuni si pozitie
                        pbGr.DrawString("ID:" + coll.ID + " xy:" + (coll.rect.X - CAMERA_ORIGIN.X) + "," + (coll.rect.Y - CAMERA_ORIGIN.Y) + "\nwh:" + coll.rect.Width + "," + coll.rect.Height, new Font("Arial", 8, FontStyle.Bold), Brushes.LightBlue, rct.X + 6, rct.Y + 6);

                        if (coll.logic.targetID >= 0)
                        {
                            vTargetDst = GetTargetPosByID(coll.logic.targetID);
                            vTargetSrc.X = coll.rect.X + coll.rect.Width / 2; vTargetSrc.Y = coll.rect.Y + coll.rect.Height / 2;
                       }
                    }
                }
            }

            //--- painting selected object's target
            if ((Math.Abs(vTargetSrc.X - vTargetDst.X) > 0.0f) || (Math.Abs(vTargetSrc.Y - vTargetDst.Y) > 0.0f))
            {
                vTargetSrc = WorldToScreen(vTargetSrc);
                vTargetDst = WorldToScreen(vTargetDst);

                Pen linepen = new Pen(Color.Black, 3);
                AdjustableArrowCap myArrow = new AdjustableArrowCap(4, 4, false);
                linepen.CustomEndCap = myArrow;
                pbGr.DrawLine(linepen, vTargetSrc, vTargetDst);

                vTargetDst.Y -= 2.0f; vTargetSrc.Y -= 2.0f;
                linepen.Color = Color.BlueViolet;
                pbGr.DrawLine(linepen, vTargetSrc, vTargetDst);
            }

            ///--- misc objects ---
            if ((g_brushMode == BRUSH_MODE_MISC) || (g_brushMode == BRUSH_MODE_AI))
            {
                for (int kk = 0; kk < arrMisc.Count; kk++)
                {
                    CMiscObjectBase obj = arrMisc[kk] as CMiscObjectBase;
                    obj.Paint(pbGr, this);
                }
            }

            //last found ID
            if(g_lastSearchID >= 0)
            {
                RectangleF rct2 = new RectangleF();
                rct2.X = g_lastSearchPos.X - 20.0f; rct2.Y = g_lastSearchPos.Y - 20.0f;
                rct2.Width = 40.0f; rct2.Height = 40.0f;
                rct2 = WorldToScreen(rct2);

                Pen np = new Pen(Brushes.Red, 5);
                pbGr.DrawEllipse(np, rct2);
                pbGr.DrawLine(Pens.Red, rct2.X, rct2.Y, rct2.Right, rct2.Bottom);
                pbGr.DrawLine(Pens.Red, rct2.X, rct2.Bottom, rct2.Right, rct2.Y);
            }

            //area selection (selectia deja facuta)
            if (g_brushMode == BRUSH_MODE_TILES)
            {
                // tiles selection
                if ((g_SelectedArea.Width > 0.0f) && (g_SelectedArea.Height > 0.0f))
                {
                    RectangleF rct2 = g_SelectedAreaTL;
                    rct2.X = (rct2.X * TILE_WIDTH - cameraPos.X) * zoomLevel; rct2.Y = (rct2.Y * TILE_HEIGHT - cameraPos.Y) * zoomLevel;
                    rct2.Width = rct2.Width * TILE_WIDTH * zoomLevel; rct2.Height = rct2.Height * TILE_HEIGHT * zoomLevel;
                    pbGr.DrawRectangle(penDottedGr1, RectFtoRect(rct2));
                }

                // level bounds
                RectangleF rct3 = AABBlevel;
                rct3.X = (AABBlevel.X - cameraPos.X) * zoomLevel; rct3.Y = (AABBlevel.Y - cameraPos.Y) * zoomLevel;
                rct3.Width = AABBlevel.Width * zoomLevel; rct3.Height = AABBlevel.Height * zoomLevel;
                pbGr.DrawRectangle(Pens.BlueViolet, RectFtoRect(rct3));

            }

            //gridul in timpul selectiei
            if (g_selectingStatus == K_SEL_STATUS_DRAGGING)
            {
                RectangleF selrect = new RectangleF(g_SelectedArea.X, g_SelectedArea.Y, g_lastHoveredPoint.X - g_SelectedArea.X, g_lastHoveredPoint.Y - g_SelectedArea.Y);
                selrect = WorldToScreen(NormalizeRect(selrect));
                pbGr.DrawRectangle(penDotted, RectFtoRect(selrect));
            }

            //brush
            if((g_brushMode == BRUSH_MODE_TILES) && (g_selectingStatus == K_SEL_STATUS_EMPTY))
            {
                Rectangle brushrect = new Rectangle((int)(zoomLevel * (g_hoveredTile.X * TILE_WIDTH)) - (int)(cameraPos.X * zoomLevel),
                    (int)(zoomLevel * ((g_hoveredTile.Y) * TILE_HEIGHT)) - (int)(cameraPos.Y * zoomLevel), (int)(zoomLevel * g_matBrush.Width * TILE_WIDTH), (int)(zoomLevel * g_matBrush.Height * TILE_HEIGHT));
                pbGr.DrawRectangle(penDotted, brushrect);
            }

            pictureBox1.Refresh();

            UpdateStatusBarMessage();
        }

        //mouse events
        public PointF lastMousePos;
        public Point lastClickPos; //folosit ca sa stiu daca am miscat mouse in unele cazuri
        public bool draggingMap = false;

        private void pictureBox1_MouseDown(object sender, MouseEventArgs e)
        {
            g_lastMousePos.X = e.X; g_lastMousePos.Y = e.Y;
            g_lastMouseDownPos = g_lastMousePos;
            g_hoveredTile.X = (int)(e.X * (1.0f / zoomLevel) + cameraPos.X) / TILE_HEIGHT;
            g_hoveredTile.Y = (int)(e.Y * (1.0f / zoomLevel) + cameraPos.Y) / TILE_HEIGHT;

            PointF cursorPosF = new PointF((e.X * (1.0f / zoomLevel) + cameraPos.X), (e.Y * (1.0f / zoomLevel) + cameraPos.Y));
            Point cursorPos = new Point((int)cursorPosF.X, (int)cursorPosF.Y);
            g_lastHoveredPoint = cursorPosF;

            //daca ai apasat middle button sa faca pan
            if (e.Button == MouseButtons.Middle)
            {
                Cursor.Current = Cursors.SizeAll;
            }
            if (g_selectingStatus != K_SEL_STATUS_EMPTY)
            {
                Cursor.Current = Cursors.Cross;
            }

            if (e.Button == MouseButtons.Left)
            {
                SetFileModified();

                //salvam click pos
                lastClickPos = new Point(e.X, e.Y);

                if (g_selectingStatus == K_SEL_STATUS_START)
                {
                    g_SelectedArea.X = cursorPosF.X; g_SelectedArea.Y = cursorPosF.Y;
                    g_SelectedArea.Width = g_SelectedArea.Height = 0.0f;

                    g_SelectedAreaTL.X = g_hoveredTile.X; g_SelectedAreaTL.Y = g_hoveredTile.Y;
                    g_SelectedAreaTL.Width = g_SelectedAreaTL.Height = 1;

                    g_selectingStatus = K_SEL_STATUS_DRAGGING;
                }
                else if ((g_brushMode == BRUSH_MODE_TILES) && (g_wndMaterials.g_TilesetImg != null))
                {
                    //clear undo state when starting to paint
                    gMap.Undo_ClearUndo();

                    int tileID = g_matBrush.X + g_matBrush.Y * TILESET_COLUMNS;
                    if ((g_matBrush.Width == 1) && (g_matBrush.Height == 1))
                    {
                        CTileBlock tb = gMap.setTile(g_hoveredTile.X, g_hoveredTile.Y, tileID, g_selectedLayer);
                        BuildBlockImage(tb);
                    }
                    else
                    {
                        for (int kk = 0; kk < g_matBrush.Width; kk++)
                        {
                            for (int ll = 0; ll < g_matBrush.Height; ll++)
                            {
                                int tileIDs = g_matBrush.X + kk + (g_matBrush.Y + ll) * TILESET_COLUMNS;
                                CTileBlock tb = gMap.setTile(g_hoveredTile.X + kk, g_hoveredTile.Y + ll, tileIDs, g_selectedLayer);
                                BuildBlockImage(tb);
                            }
                        }
                    }
                }
                else if (g_brushMode == BRUSH_MODE_LIGHTS)
                {
                    //vede daca dau click pe scalarea luminii curente
                    if (g_selectedLight != null)
                    {
                        Point cpos = new Point((int)cursorPosF.X, (int)cursorPosF.Y);
                        g_nScalingItemFlags = GrabbedScalingCorners(cpos, g_selectedLight.area);
                    }

                    //daca am dat click pe scalare
                    if ((g_nScalingItemFlags != K_SCALE_FLAG_MOVE) && (g_nScalingItemFlags != 0))
                    {
                        g_bDraggingItem = false;
                        lastMousePos.X = e.X; lastMousePos.Y = e.Y;
                    }
                    else
                    {
                        g_nScalingItemFlags = 0;
                        //dragging selected light?
                        if ((g_selectedLight != null) && (PointInLight(g_selectedLight, cursorPos)))
                        {
                            g_bDraggingItem = true;
                            lastMousePos.X = e.X; lastMousePos.Y = e.Y;
                        }
                        //adding new light?
                        else 
                        {
                            CLight nLight = new CLight();
                            nLight.ID = GetUniqueID();
                            nLight.type = K_LIGHT_POINT;
                            nLight.pos = cursorPosF;
                            nLight.angle = 0.0f;
                            nLight.castsShadows = false;
                            nLight.nAtmoAttenuationPerc = 0;
                            //move area
                            nLight.area.X = nLight.pos.X - K_LIGHT_DEFAULT_RADIUS; nLight.area.Y = nLight.pos.Y - K_LIGHT_DEFAULT_RADIUS;
                            nLight.area.Width = nLight.area.Height = 2 * K_LIGHT_DEFAULT_RADIUS;
                            //set image
                            nLight.animId = 0;
                            SetLightAreaFromAnim(nLight);

                            arrLights.Add(nLight);

                            g_selectedLight = arrLights[arrLights.Count - 1] as CLight;
                            g_wndLights.SetSelectedLight(g_selectedLight);
                            //daca am snap to grid
                            bool snapGrid = chk_snapToGrid.Checked;

                            if ((snapGrid) && (g_selectedLight != null))
                            {
                                if ((g_selectedLight.type != K_LIGHT_POINT) && (g_selectedLight.type != K_LIGHT_AREA))
                                {
                                    g_selectedLight.area.X = (int)Math.Round(g_selectedLight.area.X / (float)TILE_WIDTH) * TILE_WIDTH;
                                    g_selectedLight.area.Y = (int)Math.Round(g_selectedLight.area.Y / (float)TILE_HEIGHT) * TILE_HEIGHT;
                                    g_selectedLight.area.Width = (int)Math.Round(g_selectedLight.area.Width / (float)TILE_WIDTH) * TILE_WIDTH;
                                    g_selectedLight.area.Height = (int)Math.Round(g_selectedLight.area.Height / (float)TILE_HEIGHT) * TILE_HEIGHT;

                                    g_selectedLight.pos.X = g_selectedLight.area.X + g_selectedLight.area.Width / 2.0f;
                                    g_selectedLight.pos.Y = g_selectedLight.area.Y + g_selectedLight.area.Height / 2.0f;
                                }
                                PaintMap(e.X, e.Y);
                            }
                        }
                    }
                }
                else if (g_brushMode == BRUSH_MODE_COLLISIONS)
                {
                    //daca dau click in colboxul selectat
                    if (g_selectedCollision != null)
                    {
                        if (PointInRect(cursorPos, g_selectedCollision.rect))
                        {
                            g_nScalingItemFlags = GrabbedScalingCorners(cursorPos, g_selectedCollision.rect);
                            if (g_nScalingItemFlags == K_SCALE_FLAG_MOVE)
                            {
                                g_bDraggingItem = true;
                                g_nScalingItemFlags = 0;
                            }
                            else
                            {
                                g_bDraggingItem = false;
                            }

                            lastMousePos.X = e.X; lastMousePos.Y = e.Y;

                            PaintMap(e.X, e.Y);
                            return;
                        }
                    }
                    else //nu selectez deci adaug
                    {
                        Point pt = cursorPos;
                        if (chk_snapToGrid.Checked)
                            pt = SnapPointToGrid(cursorPos, K_SNAP_THRESHOLD_COLLISIONS);

                        //folosesc datele din selectedArea ca sa facem collBoxul
                        g_SelectedArea.X = pt.X; g_SelectedArea.Y = pt.Y;
                        g_SelectedArea.Width = g_SelectedArea.Height = 0.0f;

                        g_bCreatingItem = true;
                    }
                }
                else if (g_brushMode == BRUSH_MODE_GROUND_LEVEL)
                {
                    gLevelOrigin.Y = g_hoveredTile.Y * TILE_HEIGHT;
                    gLevelOrigin.X = g_hoveredTile.X * TILE_WIDTH;
                }
                else if (g_brushMode == BRUSH_MODE_OBJECTS)
                {
                    //daca am selectie facuta vad daca am facut click stanga in obiect
                    if (PointInObject(g_selectedObject, cursorPos))
                    {
                        //salvez date mouse
                        g_bDraggingItem = true;
                        lastMousePos = cursorPosF;
                        //save starting pos
                        g_selectedObjectOldPos = g_selectedObject.pos;
                    }
                    else
                    {
                        if (g_selectedObject != null) //daca am facut click in afara obiectului selectat
                        {
                            g_wndObjects.SetSelectedObject(null);
                            g_selectedObject = null;
                        }
                        else
                        //click stanga adauga obiect nou daca am selectie facuta in fereastra de obiecte
                        if ((g_sprObjects != null) && (g_wndObjects.g_selectedAnim >= 0) && (g_wndObjects.g_selectedFrame >= 0))
                        {
                            //snap to grid
                            Point objcur = new Point((int)cursorPos.X, (int)cursorPos.Y);
                            if (chk_snapToGrid.Checked)
                            {
                                objcur = SnapPointToGrid(objcur, K_SNAP_THRESHOLD);
                            }

                            if ((g_sprObjects != null) && (g_wndObjects.g_selectedAnim >= 0) && (g_wndObjects.g_selectedFrame >= 0))
                            {
                                CObject nobj = new CObject();
                                nobj.ID = GetUniqueID();
                                nobj.pos.X = objcur.X;
                                nobj.pos.Y = objcur.Y;
                                nobj.layer = g_selectedLayer;
                                nobj.animIdx = g_wndObjects.g_selectedAnim;
                                nobj.frameIdx = g_wndObjects.g_selectedFrame;
                                //daca animatia e looping setez flagul de animated
                                if ((g_sprObjects.anims[g_wndObjects.g_selectedAnim].flags & 1) != 0)
                                    nobj.flags |= OBJFLAG_ANIMATED;

                                arrObjects.Add(nobj);
                            }
                            //selectez obiectul abia aparut
                            g_wndObjects.SetSelectedObject(null);
                            g_selectedObject = null;
                        }
                    }
                }
                else if (g_brushMode == BRUSH_MODE_MISC)
                {
                    if (g_selectedMisc == null) //nu am nimic selectat
                    {
                        //1. vad daca la click imi apare selectie
                        foreach (CMiscObjectBase misc in arrMisc)
                        {
                            if (misc.GetSelection(cursorPosF))
                            {
                                g_selectedMisc = misc;
                                g_wndMisc.SetSelectedMisc(g_selectedMisc);

                                g_selectedMisc.OnMouseDown(cursorPosF);
                            }
                        }
                        //2. daca nu se selecteaza nimic adaug obiect nou in fn de brush
                        if (g_selectedMisc == null)
                        {
                            switch (g_brushValue)
                            {
                                case K_MISC_RAIL:
                                    CMiscObject_Rail newrail = new CMiscObject_Rail();
                                    newrail.ID = GetUniqueID();

                                    arrMisc.Add(newrail);
                                    g_selectedMisc = newrail;

                                    g_wndMisc.SetSelectedMisc(g_selectedMisc);

                                    g_selectedMisc.OnMouseDown(cursorPosF);
                                    break;

                                case K_MISC_SCRIPT:
                                    CMiscObject_Script news = new CMiscObject_Script();
                                    news.ID = GetUniqueID();
                                    news.pos = cursorPosF;
                                    //params
                                    news.listParams.Add("str_script"); //name
                                    news.listParams.Add("SCRIPT_NAME"); //val

                                    arrMisc.Add(news);
                                    g_selectedMisc = news;

                                    g_wndMisc.SetSelectedMisc(g_selectedMisc);

                                    g_selectedMisc.OnMouseDown(cursorPosF);
                                    break;

                                case K_MISC_BACKGROUND:
                                    g_brushValue = -1;
                                    /*
                                     * //this adds a background object on click (not needed anymore)
                                    CMiscObject_Background newbg = new CMiscObject_Background();
                                    newbg.ID = GetUniqueID();
                                    newbg.pos = cursorPosF;
                                    //params
                                    newbg.listParams.Add("str_bsx"); //name
                                    newbg.listParams.Add(".bsx"); //val
                                    newbg.listParams.Add("str_anim"); //name
                                    newbg.listParams.Add("SET_BG_ANIM"); //val
                                    newbg.listParams.Add("str_water_anim"); //name
                                    newbg.listParams.Add("SET_WATER_ANIM"); //val

                                    arrMisc.Add(newbg);
                                    g_selectedMisc = newbg;

                                    g_wndMisc.SetSelectedMisc(g_selectedMisc);

                                    g_selectedMisc.OnMouseDown(cursorPosF);
                                    */
                                    break;

                                case K_MISC_FRONTLAYEROBJ:
                                    CMiscObject_FrontLayerObj newflo = new CMiscObject_FrontLayerObj();
                                    newflo.ID = GetUniqueID();
                                    newflo.pos = cursorPosF;
                                    //params
                                    newflo.listParams.Add("strAnim"); //name
                                    newflo.listParams.Add("NEARCAMERA"); //val
                                    newflo.listParams.Add("nFrame"); //name
                                    newflo.listParams.Add("0"); //val

                                    arrMisc.Add(newflo);
                                    g_selectedMisc = newflo;

                                    g_wndMisc.SetSelectedMisc(g_selectedMisc);

                                    g_selectedMisc.OnMouseDown(cursorPosF);
                                    break;

                                default:
                                case -1:
                                    {
                                        MessageBox.Show("Please select a MiscObject type first!");
                                        g_selectedMisc = null;
                                        g_wndMisc.SetSelectedMisc(null);
                                    }
                                    break;
                            }
                        }
                    }
                    else //daca este selectat il editeaza
                    {
                        switch (g_selectedMisc.type)
                        {
                            case K_MISC_RAIL:
                                g_selectedMisc.OnMouseDown(cursorPosF);
                                break;
                        }
                    }
                }
                else if (g_brushMode == BRUSH_MODE_MACRO)
                {
                    //snap to grid
                    Point clickpt = new Point((int)cursorPos.X, (int)cursorPos.Y);
                    if (chk_snapToGrid.Checked)
                        clickpt = SnapPointToGrid(clickpt, K_SNAP_THRESHOLD);

                    switch (g_brushValue)
                    {
                        case K_MACRO_DOOR_UNLOCKED:
                            {
                                int anmidx = g_sprObjects.GetAnimIdxByName("DOORS_SECTION");
                                if (anmidx < 0)
                                {
                                    MessageBox.Show("Animation DOORS_SECTION not found or object sprites not loaded!");
                                    break;
                                }

                                //add collbox
                                CCollisionElement coll = new CCollisionElement();
                                coll.ID = GetUniqueID();
                                coll.rect = new RectangleF(clickpt.X + 7, clickpt.Y - TILE_HEIGHT * 2 - 1, 7, TILE_HEIGHT * 2 + 2);
                                coll.castShadows = true;
                                coll.type = K_COLL_TYPE_SOLID;
                                arrCollisions.Add(coll);

                                //Add door
                                CObject nobj = new CObject();
                                nobj.ID = GetUniqueID();
                                nobj.pos.X = clickpt.X;
                                nobj.pos.Y = clickpt.Y;
                                nobj.layer = 1;
                                nobj.animIdx = anmidx;
                                nobj.frameIdx = 0;
                                //daca animatia e looping setez flagul de animated
                                if ((g_sprObjects.anims[nobj.animIdx].flags & 1) != 0)
                                    nobj.flags |= OBJFLAG_ANIMATED;

                                arrObjects.Add(nobj);

                                //set door AI
                                nobj.logic.targetID = (int)coll.ID;
                                nobj.logic.strActions = "ACTIVE_OPEN_DOOR_NO_CLOSE";
                                nobj.logic.bCanInteract = true;
                                nobj.logic.bHideInteractIcon = false;
                                nobj.logic.nInteractTimer = 0;
                                nobj.logic.strAIname = "AI_ACTIVE_DOOR_SECTION";

                                //set collbox logic
                                coll.logic.targetID = (int)nobj.ID;
                                coll.logic.strAIname = "AI_COLL_BREAKABLE_DOOR";
                                coll.logic.strActions = "TOGGLE_HIDDEN_FLAG";
                                //params (name, val, name, val...)
                                coll.logic.listAIparams.Add("f_life");
                                coll.logic.listAIparams.Add("9.0");
                            }
                            break;
                        case K_MACRO_DOOR_LOCKED:
                            {
                                int anmidx = g_sprObjects.GetAnimIdxByName("DOORS_SECTION");
                                if (anmidx < 0)
                                {
                                    MessageBox.Show("Animation DOORS_SECTION not found or object sprites not loaded!");
                                    break;
                                }

                                //add collbox
                                CCollisionElement coll = new CCollisionElement();
                                coll.ID = GetUniqueID();
                                coll.rect = new RectangleF(clickpt.X + 7, clickpt.Y - TILE_HEIGHT * 2 - 1, 7, TILE_HEIGHT * 2 + 2);
                                coll.castShadows = true;
                                coll.type = K_COLL_TYPE_SOLID;
                                arrCollisions.Add(coll);

                                //Add door
                                CObject nobj = new CObject();
                                nobj.ID = GetUniqueID();
                                nobj.pos.X = clickpt.X;
                                nobj.pos.Y = clickpt.Y;
                                nobj.layer = 1; //back layer
                                nobj.animIdx = anmidx;
                                nobj.frameIdx = 4;
                                //daca animatia e looping setez flagul de animated
                                if ((g_sprObjects.anims[nobj.animIdx].flags & 1) != 0)
                                    nobj.flags |= OBJFLAG_ANIMATED;

                                arrObjects.Add(nobj);

                                //set door AI
                                nobj.logic.targetID = (int)coll.ID;
                                nobj.logic.strActions = "ACTIVE_LOCKED_BREAKABLE";
                                nobj.logic.bCanInteract = true;
                                nobj.logic.bHideInteractIcon = false;
                                nobj.logic.nInteractTimer = 0;
                                nobj.logic.strAIname = "AI_ACTIVE_DOOR_SECTION";
                                //params (name, val, name, val...)
                                nobj.logic.listAIparams.Add("n_locked");
                                nobj.logic.listAIparams.Add("1"); 
                                nobj.logic.listAIparams.Add("f_lockpickTime");
                                nobj.logic.listAIparams.Add("2.5"); 

                                //set collbox logic
                                coll.logic.targetID = (int)nobj.ID;
                                coll.logic.strAIname = "AI_COLL_BREAKABLE_DOOR";
                                coll.logic.strActions = "TOGGLE_HIDDEN_FLAG";
                                //params (name, val, name, val...)
                                coll.logic.listAIparams.Add("f_life");
                                coll.logic.listAIparams.Add("90.0");
                            }
                            break;
                        case K_MACRO_WINDOW_PROFILE_HORIZONTAL:
                            {
                                int anmidx = g_sprObjects.GetAnimIdxByName("WINDOWS_SECTION");
                                if (anmidx < 0)
                                {
                                    MessageBox.Show("Animation WINDOWS_SECTION not found or object sprites not loaded!");
                                    break;
                                }

                                //add collbox
                                CCollisionElement coll = new CCollisionElement();
                                coll.ID = GetUniqueID();
                                coll.rect = new RectangleF(clickpt.X - 1, clickpt.Y + 1, 55, 10);
                                coll.castShadows = false;
                                coll.type = K_COLL_TYPE_SOLID;
                                arrCollisions.Add(coll);

                                //Add window
                                CObject nobj = new CObject();
                                nobj.ID = GetUniqueID();
                                nobj.pos.X = clickpt.X;
                                nobj.pos.Y = clickpt.Y;
                                nobj.layer = 1; //back layer
                                nobj.animIdx = anmidx;
                                nobj.frameIdx = 4;

                                arrObjects.Add(nobj);

                                //set window AI
                                nobj.logic.bCanInteract = false;
                                nobj.logic.bHideInteractIcon = false;

                                //set collbox logic
                                coll.logic.targetID = (int)nobj.ID;
                                coll.logic.strAIname = "AI_COLL_BREAKABLE_WINDOW";
                                coll.logic.strActions = "TOGGLE_HIDDEN_FLAG";
                                //params (name, val, name, val...)
                                coll.logic.listAIparams.Add("f_life");
                                coll.logic.listAIparams.Add("2.0");
                            }
                            break;
                        case K_MACRO_WINDOW_PROFILE:
                            {
                                int anmidx = g_sprObjects.GetAnimIdxByName("WINDOWS_SECTION");
                                if (anmidx < 0)
                                {
                                    MessageBox.Show("Animation WINDOWS_SECTION not found or object sprites not loaded!");
                                    break;
                                }

                                //add collbox
                                CCollisionElement coll = new CCollisionElement();
                                coll.ID = GetUniqueID();
                                coll.rect = new RectangleF(clickpt.X + 7, clickpt.Y - 1, 7, TILE_HEIGHT * 2 + 6);
                                coll.castShadows = false;
                                coll.type = K_COLL_TYPE_SOLID;
                                arrCollisions.Add(coll);

                                //Add window
                                CObject nobj = new CObject();
                                nobj.ID = GetUniqueID();
                                nobj.pos.X = clickpt.X;
                                nobj.pos.Y = clickpt.Y;
                                nobj.layer = 1; //back layer
                                nobj.animIdx = anmidx;
                                nobj.frameIdx = 0;

                                arrObjects.Add(nobj);

                                //set window AI
                                nobj.logic.bCanInteract = false;
                                nobj.logic.bHideInteractIcon = false;

                                //set collbox logic
                                coll.logic.targetID = (int)nobj.ID;
                                coll.logic.strAIname = "AI_COLL_BREAKABLE_WINDOW";
                                coll.logic.strActions = "TOGGLE_HIDDEN_FLAG";
                                //params (name, val, name, val...)
                                coll.logic.listAIparams.Add("f_life");
                                coll.logic.listAIparams.Add("2.0");
                            }
                            break;
                        case K_MACRO_DOOR_METALLIC:
                            {
                                int anmidx = g_sprObjects.GetAnimIdxByName("DOOR_SLIDING_LOCKED");
                                if (anmidx < 0)
                                {
                                    MessageBox.Show("Animation DOOR_SLIDING_LOCKED not found or object sprites not loaded!");
                                    break;
                                }

                                //add collbox
                                CCollisionElement coll = new CCollisionElement();
                                coll.ID = GetUniqueID();
                                coll.rect = new RectangleF(clickpt.X + 7, clickpt.Y - TILE_HEIGHT * 2 - 1, 7, TILE_HEIGHT * 2 + 2);
                                coll.castShadows = true;
                                coll.type = K_COLL_TYPE_SOLID;
                                arrCollisions.Add(coll);

                                //Add door
                                CObject nobj = new CObject();
                                nobj.ID = GetUniqueID();
                                nobj.pos.X = clickpt.X;
                                nobj.pos.Y = clickpt.Y;
                                nobj.layer = 1;
                                nobj.animIdx = anmidx;
                                nobj.frameIdx = 0;
                                //daca animatia e looping setez flagul de animated
                                if ((g_sprObjects.anims[nobj.animIdx].flags & 1) != 0)
                                    nobj.flags |= OBJFLAG_ANIMATED;

                                arrObjects.Add(nobj);

                                //set door AI
                                nobj.logic.targetID = (int)coll.ID;
                                nobj.logic.strActions = "SLIDING_DOOR_REDKEY";
                                nobj.logic.bCanInteract = true;
                                nobj.logic.bHideInteractIcon = false;
                                nobj.logic.nInteractTimer = 0;

                                //set collbox logic
                                coll.logic.targetID = (int)nobj.ID;
                                coll.logic.strAIname = "AI_COLL_BREAKABLE_DOOR";
                                coll.logic.strActions = "TOGGLE_HIDDEN_FLAG";
                                //params (name, val, name, val...)
                                coll.logic.listAIparams.Add("f_life");
                                coll.logic.listAIparams.Add("400.0");
                                coll.logic.listAIparams.Add("b_reinforced");
                                coll.logic.listAIparams.Add("1");

                            }
                            break;
                        case K_MACRO_KEYCARD_RED:
                            {
                                int anmidx = g_sprObjects.GetAnimIdxByName("KEYCARD_RED");
                                if (anmidx < 0)
                                {
                                    MessageBox.Show("Animation KEYCARD_RED not found or object sprites not loaded!");
                                    break;
                                }

                                //Add light
                                CLight nLight = new CLight();
                                nLight.ID = GetUniqueID();
                                nLight.type = K_LIGHT_POINT;
                                nLight.pos.X = clickpt.X; nLight.pos.Y = clickpt.Y;
                                nLight.angle = 0.0f;
                                nLight.castsShadows = false;
                                nLight.nAtmoAttenuationPerc = 0;
                                //move area
                                nLight.area.X = nLight.pos.X - K_LIGHT_DEFAULT_RADIUS; nLight.area.Y = nLight.pos.Y - K_LIGHT_DEFAULT_RADIUS;
                                nLight.area.Width = nLight.area.Height = 2 * K_LIGHT_DEFAULT_RADIUS;
                                //set image
                                nLight.color = Color.FromArgb(255, 255, 40, 40);
                                nLight.animId = 4;
                                SetLightAreaFromAnim(nLight);

                                //logic
                                nLight.logic.strActions = "TOGGLE_HIDDEN_FLAG";

                                arrLights.Add(nLight);

                                //Add object
                                CObject nobj = new CObject();
                                nobj.ID = GetUniqueID();
                                nobj.pos.X = clickpt.X;
                                nobj.pos.Y = clickpt.Y;
                                nobj.layer = 1; //back layer
                                nobj.animIdx = anmidx;
                                nobj.frameIdx = 0;
                                //daca animatia e looping setez flagul de animated
                                if ((g_sprObjects.anims[nobj.animIdx].flags & 1) != 0)
                                    nobj.flags |= OBJFLAG_ANIMATED;

                                arrObjects.Add(nobj);

                                //set door AI
                                nobj.logic.targetID = (int)nLight.ID;
                                nobj.logic.strActions = "PICK_RED_KEY";
                                nobj.logic.bCanInteract = true;
                                nobj.logic.bHideInteractIcon = false;
                                nobj.logic.nInteractTimer = 0;
                            }
                            break;
                        case K_MACRO_KEYCARD_GOLD:
                            {
                                int anmidx = g_sprObjects.GetAnimIdxByName("KEYCARD_YELLOW");
                                if (anmidx < 0)
                                {
                                    MessageBox.Show("Animation not found or object sprites not loaded!");
                                    break;
                                }

                                //Add light
                                CLight nLight = new CLight();
                                nLight.ID = GetUniqueID();
                                nLight.type = K_LIGHT_POINT;
                                nLight.pos.X = clickpt.X; nLight.pos.Y = clickpt.Y;
                                nLight.angle = 0.0f;
                                nLight.castsShadows = false;
                                nLight.nAtmoAttenuationPerc = 0;
                                //move area
                                nLight.area.X = nLight.pos.X - K_LIGHT_DEFAULT_RADIUS; nLight.area.Y = nLight.pos.Y - K_LIGHT_DEFAULT_RADIUS;
                                nLight.area.Width = nLight.area.Height = 2 * K_LIGHT_DEFAULT_RADIUS;
                                //set image
                                nLight.color = Color.FromArgb(255, 240, 240, 120);
                                nLight.animId = 4;
                                SetLightAreaFromAnim(nLight);

                                //logic
                                nLight.logic.strActions = "TOGGLE_HIDDEN_FLAG";

                                arrLights.Add(nLight);

                                //Add object
                                CObject nobj = new CObject();
                                nobj.ID = GetUniqueID();
                                nobj.pos.X = clickpt.X;
                                nobj.pos.Y = clickpt.Y;
                                nobj.layer = 1; //back layer
                                nobj.animIdx = anmidx;
                                nobj.frameIdx = 0;
                                //daca animatia e looping setez flagul de animated
                                if ((g_sprObjects.anims[nobj.animIdx].flags & 1) != 0)
                                    nobj.flags |= OBJFLAG_ANIMATED;

                                arrObjects.Add(nobj);

                                //set door AI
                                nobj.logic.targetID = (int)nLight.ID;
                                nobj.logic.strActions = "PICK_GOLD_KEY";
                                nobj.logic.bCanInteract = true;
                                nobj.logic.bHideInteractIcon = false;
                                nobj.logic.nInteractTimer = 0;
                            }
                            break;
                        case K_MACRO_FRONT_SOLO_STAIRS:
                            {
                                int anmidx = g_sprObjects.GetAnimIdxByName("DOORS_PORTALS");
                                if (anmidx < 0)
                                {
                                    MessageBox.Show("Animation not found or object sprites not loaded!");
                                    break;
                                }

                                //Add object
                                CObject nobj = new CObject();
                                nobj.ID = GetUniqueID();
                                nobj.pos.X = clickpt.X;
                                nobj.pos.Y = clickpt.Y;
                                nobj.layer = 1;
                                nobj.animIdx = anmidx;
                                nobj.frameIdx = 0;
                                //daca animatia e looping setez flagul de animated
                                if ((g_sprObjects.anims[nobj.animIdx].flags & 1) != 0)
                                    nobj.flags |= OBJFLAG_ANIMATED;

                                arrObjects.Add(nobj);

                                //set door AI
                                nobj.logic.targetID = -1;
                                nobj.logic.strActions = "PLAYER_SOLO_TELEPORT";
                                nobj.logic.bCanInteract = true;
                                nobj.logic.bHideInteractIcon = false;
                                nobj.logic.nInteractTimer = 0;
                                nobj.logic.strAIname = "AI_ACTIVE_DOORFACE_AUTOCLOSE";
                                //params (name, val, name, val...)
                                //f_slowTimeDuration, b_EnterHiddenRoom, b_DontChangeFrames
                                //nobj.logic.listAIparams.Add("f_slowTimeDuration");
                                //nobj.logic.listAIparams.Add("0.0");
                                //nobj.logic.listAIparams.Add("b_EnterHiddenRoom");
                                //nobj.logic.listAIparams.Add("0");
                                nobj.logic.listAIparams.Add("b_DontChangeFrames");
                                nobj.logic.listAIparams.Add("1");

                                MessageBox.Show("Don't forget to set the targetID of the paired stairway!");
                            }
                            break;
                        case K_MACRO_FRONT_SOLO_DOOR:
                            {
                                int anmidx = g_sprObjects.GetAnimIdxByName("DOORS_FRONT");
                                if (anmidx < 0)
                                {
                                    MessageBox.Show("Animation not found or object sprites not loaded!");
                                    break;
                                }

                                //Add object
                                CObject nobj = new CObject();
                                nobj.ID = GetUniqueID();
                                nobj.pos.X = clickpt.X;
                                nobj.pos.Y = clickpt.Y;
                                nobj.layer = 1;
                                nobj.animIdx = anmidx;
                                nobj.frameIdx = 14;

                                arrObjects.Add(nobj);

                                //set door AI
                                nobj.logic.targetID = -1;
                                nobj.logic.strActions = "PLAYER_SOLO_TELEPORT";
                                nobj.logic.bCanInteract = true;
                                nobj.logic.bHideInteractIcon = false;
                                nobj.logic.nInteractTimer = 0;
                                nobj.logic.strAIname = "AI_ACTIVE_DOORFACE_AUTOCLOSE";
                                //params (name, val, name, val...)
                                //f_slowTimeDuration, b_EnterHiddenRoom, b_DontChangeFrames
                                nobj.logic.listAIparams.Add("f_slowTimeDuration");
                                nobj.logic.listAIparams.Add("0.0");
                                nobj.logic.listAIparams.Add("b_EnterHiddenRoom");
                                nobj.logic.listAIparams.Add("0");
                                nobj.logic.listAIparams.Add("b_DontChangeFrames");
                                nobj.logic.listAIparams.Add("0");

                                MessageBox.Show("Don't forget to set the targetID of the paired Solo Door!");
                            }
                            break;
                        case K_MACRO_FRONT_TEAM_DOOR:
                            {
                                int anmidx = g_sprObjects.GetAnimIdxByName("DOORS_FRONT");
                                if (anmidx < 0)
                                {
                                    MessageBox.Show("Animation not found or object sprites not loaded!");
                                    break;
                                }

                                //Add object
                                CObject nobj = new CObject();
                                nobj.ID = GetUniqueID();
                                nobj.pos.X = clickpt.X;
                                nobj.pos.Y = clickpt.Y;
                                nobj.layer = 1;
                                nobj.animIdx = anmidx;
                                nobj.frameIdx = 0;
                                //daca animatia e looping setez flagul de animated
                                if ((g_sprObjects.anims[nobj.animIdx].flags & 1) != 0)
                                    nobj.flags |= OBJFLAG_ANIMATED;

                                arrObjects.Add(nobj);

                                //set door AI
                                nobj.logic.targetID = -1;
                                nobj.logic.strActions = "ACTIVE_TEAM_TELEPORTER_2FRAMES";
                                nobj.logic.bCanInteract = true;
                                nobj.logic.bHideInteractIcon = false;
                                nobj.logic.nInteractTimer = 0;
                                nobj.logic.strAIname = "AI_ACTIVE_TEAM_TELEPORTER_2FRAMES";
                                //params (name, val, name, val...)
                                //f_slowTimeDuration, b_EnterHiddenRoom, b_DontChangeFrames
                                nobj.logic.listAIparams.Add("f_slowTimeDuration");
                                nobj.logic.listAIparams.Add("0.0");
                                nobj.logic.listAIparams.Add("b_EnterHiddenRoom");
                                nobj.logic.listAIparams.Add("0");
                                nobj.logic.listAIparams.Add("f_teleportDuration");
                                nobj.logic.listAIparams.Add("0.0");

                                MessageBox.Show("Don't forget to set the targetID of the paired door!\r\n If you want to add a slow motion room try the Prefabs tab.");
                            }
                            break;
                        case K_MACRO_SPAWNPOINT:
                            {
                                int anmidx = g_sprObjects.GetAnimIdxByName("CHECKPOINT_OFF");
                                if (anmidx < 0)
                                {
                                    MessageBox.Show("Animation CHECKPOINT not found or object sprites not loaded!");
                                    break;
                                }

                                //Add object
                                CObject nobj = new CObject();
                                nobj.ID = GetUniqueID();
                                nobj.pos.X = clickpt.X;
                                nobj.pos.Y = clickpt.Y;
                                nobj.layer = 1;
                                nobj.animIdx = anmidx;
                                nobj.frameIdx = 0;
                                nobj.logic.bStartHidden = true;

                                arrObjects.Add(nobj);

                                //set AI
                                nobj.logic.strAIname = "AI_ACTIVE_CHECKPOINT";
                                nobj.logic.listAIparams.Add("n_isFirst");
                                nobj.logic.listAIparams.Add("1");

                                nobj.logic.targetID = -1;
                                nobj.logic.strActions = "TOUCH_CHECKPOINT";
                                nobj.logic.bCanInteract = false;
                                nobj.logic.bHideInteractIcon = false;
                                nobj.logic.nInteractTimer = 0;
                            }
                            break;
                    }
                }
                else if (g_brushMode == BRUSH_MODE_ACTORS)
                {
                    if (g_sprActors != null)
                    {
                        if (g_selectedActor == null) //adaug unul nou
                        {
                            //snap to grid
                            Point objcur = new Point((int)cursorPos.X, (int)cursorPos.Y);
                            if (chk_snapToGrid.Checked)
                            {
                                objcur = SnapPointToGrid(objcur, K_SNAP_THRESHOLD);
                            }

                            CActor nact = new CActor();
                            nact.ID = GetUniqueID();
                            nact.pos.X = objcur.X;
                            nact.pos.Y = objcur.Y;
                            g_wndActors.SetActorTemplate(nact);
                            //daca nu am incarcat nivel nu adauga
                            if (nact.animIdx >= 0)
                            {
                                arrActors.Add(nact);
                                //selectez obiectul abia aparut
                                //g_selectedActor = arrActors[arrActors.Count - 1] as CActor;
                                //g_wndActors.SetSelectedActor(g_selectedActor);
                            }
                        }
                        else //daca e actor selectat pot sa ii fac drag
                        {
                            g_bDraggingItem = true;
                            //salvez date mouse
                            lastMousePos = cursorPosF;
                        }
                    }
                }
                else if (g_brushMode == BRUSH_MODE_PREFABS)
                {
                    //snap to grid
                    PointF vPos = new PointF(cursorPos.X, cursorPos.Y);
                    if (chk_snapToGrid.Checked)
                        vPos = SnapPointToGrid(vPos, K_SNAP_THRESHOLD);

                    //save backup for undo
                    SaveFileToMemoryBackup();
                    //add prefab
                    string strPrefabPath = g_wndPrefabs.GetSelectedPrefabPath();
                    AddPrefab(vPos, strPrefabPath);
                    Undo_SetCurrent(K_UNDO_PREFAB);

                    SetFileModified();
                }

                PaintMap(e.X, e.Y);
            }
            else if (e.Button == MouseButtons.Right)
            {
                if (g_brushMode == BRUSH_MODE_TILES)
                {
                    SetFileModified();
                    //clear undo state before operation
                    gMap.Undo_ClearUndo();

                    if ((g_matBrush.Width == 1) && (g_matBrush.Height == 1))
                    {
                        CTileBlock tb = gMap.setTile(g_hoveredTile.X, g_hoveredTile.Y, -1, g_selectedLayer);
                        BuildBlockImage(tb);
                    }
                    else
                    {
                        for (int kk = 0; kk < g_matBrush.Width; kk++)
                        {
                            for (int ll = 0; ll < g_matBrush.Height; ll++)
                            {
                                CTileBlock tb = gMap.setTile(g_hoveredTile.X + kk, g_hoveredTile.Y + ll, -1, g_selectedLayer);
                                BuildBlockImage(tb);
                            }
                        }
                    }
                    gMap.ClearEmptyblocks();
                }
                else if (g_brushMode == BRUSH_MODE_LIGHTS)
                {
                    int selObjID = SelectElementAt(cursorPos, false, true, false, false);
                    g_wndLights.SetSelectedLight(g_selectedLight);
                }
                else if (g_brushMode == BRUSH_MODE_OBJECTS)
                {
                    int selObjID = SelectElementAt(cursorPos, true, false, false, false);
                    g_wndObjects.SetSelectedObject(g_selectedObject);
                }
                else if (g_brushMode == BRUSH_MODE_COLLISIONS)
                {
                    int selObjID = SelectElementAt(cursorPos, false, false, true, false);
                    g_wndCollisions.SetSelectedCollision(g_selectedCollision);
                }
                else if (g_brushMode == BRUSH_MODE_ACTORS)
                {
                    int selObjID = SelectElementAt(cursorPos, false, false, false, true);
                    g_wndActors.SetSelectedActor(g_selectedActor);
                }
                else if (g_brushMode == BRUSH_MODE_AI)
                {
                    //select only lights
                    bool bOnlyLights = false;
                    bool bOnlyCollision = false;
                    bool bOnlyActors = false;

                    bool bAltDown = false;
                    bool bShiftDown = false;

                    if ((Control.ModifierKeys & Keys.Alt) != Keys.None)
                    {
                        bAltDown = true;
                    }
                    if ((Control.ModifierKeys & Keys.Shift) != Keys.None)
                    {
                        bShiftDown = true;
                    }

                    if (bAltDown && bShiftDown)
                        bOnlyCollision = true;
                    else if (bAltDown)
                        bOnlyLights = true;
                    else if (bShiftDown)
                        bOnlyActors = true;

                    bool bPickID = false;
                    if ((Control.ModifierKeys & Keys.Control) != Keys.None)
                    {
                        bPickID = true;
                    }

                    CObject obj = GetObject(cursorPos);
                    CLight light = GetLight(cursorPosF);
                    CActor actor = GetActor(cursorPos);
                    CCollisionElement coll = GetCollisionElement(cursorPos);

                    if (bOnlyLights)
                    {
                        obj = null;
                        coll = null;
                        actor = null;
                    }
                    else if (bOnlyCollision)
                    {
                        obj = null;
                        light = null;
                        actor = null;
                    }
                    else if (bOnlyActors)
                    {
                        obj = null;
                        coll = null;
                        light = null;
                    }

                    //nothing clicked, remove selection
                    if ((obj == null) && (coll == null) && (light == null) && (actor == null))
                    {
                        g_selectedObject = null;
                        g_selectedLight = null;
                        g_selectedActor = null;
                        g_selectedCollision = null;

                        g_wndAI.SetSelectedLogic(null);
                    }

                    if (bPickID)
                    {
                        if ((g_selectedObject != null) || (g_selectedLight != null) || (g_selectedCollision != null) || (g_selectedActor != null))
                        {
                            if (obj != null)
                                g_wndAI.TargetID_property = (int)obj.ID;
                            else if (light != null)
                                g_wndAI.TargetID_property = (int)light.ID;
                            else if (coll != null)
                                g_wndAI.TargetID_property = (int)coll.ID;
                            else if (actor != null)
                                g_wndAI.TargetID_property = (int)actor.ID;
                        }
                    }
                    else
                    {
                        if (obj != null)
                        {
                            g_selectedLight = null;
                            g_selectedObject = obj;
                            g_selectedCollision = null;
                            g_selectedActor = null;

                            g_wndAI.SetSelectedLogic(g_selectedObject.logic);
                        }
                        else if (light != null)
                        {
                            g_selectedLight = light;
                            g_selectedObject = null;
                            g_selectedCollision = null;
                            g_selectedActor = null;

                            g_wndAI.SetSelectedLogic(g_selectedLight.logic);
                        }
                        else if (coll != null)
                        {
                            g_selectedLight = null;
                            g_selectedObject = null;
                            g_selectedActor = null;
                            g_selectedCollision = coll;

                            g_wndAI.SetSelectedLogic(g_selectedCollision.logic);
                        }
                        else if (actor != null)
                        {
                            g_selectedLight = null;
                            g_selectedObject = null;
                            g_selectedCollision = null;
                            g_selectedActor = actor;

                            g_wndAI.SetSelectedLogic(g_selectedActor.logic);
                        }
                    }
                }
                else if (g_brushMode == BRUSH_MODE_MISC)
                {
                    g_selectedMisc = null;
                    g_wndMisc.SetSelectedMisc(null);
                    //1. vad daca la click imi apare selectie
                    foreach (CMiscObjectBase misc in arrMisc)
                    {
                        if (misc.GetSelection(cursorPosF))
                        {
                            g_selectedMisc = misc;
                            g_wndMisc.SetSelectedMisc(g_selectedMisc);

                            g_selectedMisc.OnMouseDown(cursorPosF);
                            break;
                        }
                    }
                }

                PaintMap(e.X, e.Y);
            }
            else if (e.Button == MouseButtons.Middle)
            {
                lastMousePos.X = e.X;
                lastMousePos.Y = e.Y;

                draggingMap = true;
            }

            UpdateStatusBarMessage();
        }

        private void pictureBox1_MouseMove(object sender, MouseEventArgs e)
        {
            bool repaint = false;
            g_lastMousePos.X = e.X; g_lastMousePos.Y = e.Y;
            g_hoveredTile.X = (int)(e.X * (1.0f / zoomLevel) + cameraPos.X) / TILE_HEIGHT;
            g_hoveredTile.Y = (int)(e.Y * (1.0f / zoomLevel) + cameraPos.Y) / TILE_HEIGHT;
            PointF cursorPosF = new PointF((e.X * (1.0f / zoomLevel) + cameraPos.X), (e.Y * (1.0f / zoomLevel) + cameraPos.Y));
            Point cursorPos = new Point((int)cursorPosF.X, (int)cursorPosF.Y);
            Point cursorPosSnapped = SnapPointToGrid(cursorPos, K_SNAP_THRESHOLD);

            g_lastHoveredPoint = cursorPosF;

            if ((g_hoveredTile.X != g_lastHoveredTile.X) || (g_hoveredTile.Y != g_lastHoveredTile.Y))
            {
                g_lastHoveredTile = g_hoveredTile;
                repaint = true;
            }

            //daca suntem pe obiect cere repaint ca sa se vada obiectul selectat
            if ((g_brushMode == BRUSH_MODE_OBJECTS) && (g_wndObjects.g_selectedAnim >= 0) && (g_wndObjects.g_selectedFrame >= 0))
            {
                repaint = true;
            }
            else if ((g_brushMode == BRUSH_MODE_ACTORS) && (g_sprActors.bLoaded))
            {
                repaint = true;
            }
            else if ((g_brushMode == BRUSH_MODE_MACRO) && (g_sprObjects.bLoaded))
            {
                repaint = true;
            }
            else if (g_brushMode == BRUSH_MODE_PREFABS)
            {
                repaint = true;
            }

            if (e.Button == MouseButtons.Middle)
                Cursor.Current = Cursors.SizeAll;
            else if(g_selectingStatus != K_SEL_STATUS_EMPTY)
                Cursor.Current = Cursors.Cross;
            else
                Cursor.Current = Cursors.Default;

            int tlx = g_hoveredTile.X;
            int tly = g_hoveredTile.Y;
            if (e.Button == MouseButtons.Left)
            {
                if (g_selectingStatus == K_SEL_STATUS_DRAGGING)
                {
                    lastMousePos.X = e.X; lastMousePos.Y = e.Y;

                    repaint = true;
                }
                else if ((g_brushMode == BRUSH_MODE_TILES) && (g_wndMaterials.g_TilesetImg != null))
                {
                    int tileID = g_matBrush.X + g_matBrush.Y * TILESET_COLUMNS;
                    if ((g_matBrush.Width == 1) && (g_matBrush.Height == 1))
                    {
                        CTileBlock tb = gMap.setTile(tlx, tly, tileID, g_selectedLayer);
                        BuildBlockImage(tb);
                    }
                    else
                    {
                        for (int kk = 0; kk < g_matBrush.Width; kk++)
                        {
                            for (int ll = 0; ll < g_matBrush.Height; ll++)
                            {
                                int tileIDs = g_matBrush.X + kk + (g_matBrush.Y + ll) * TILESET_COLUMNS;
                                CTileBlock tb = gMap.setTile(tlx + kk, tly + ll, tileIDs, g_selectedLayer);
                                BuildBlockImage(tb);
                            }
                        }
                    }
                    repaint = true;
                }
                else if (g_brushMode == BRUSH_MODE_LIGHTS)
                {
                    if ((g_bDraggingItem) && (g_selectedLight != null))
                    {
                        float dx = (e.X - lastMousePos.X) * (1.0f / zoomLevel);
                        float dy = (e.Y - lastMousePos.Y) * (1.0f / zoomLevel);
                        g_selectedLight.pos.X += dx;
                        g_selectedLight.pos.Y += dy;
                        //mut si dreptunghiul luminii
                        g_selectedLight.area.X += dx;
                        g_selectedLight.area.Y += dy;

                        lastMousePos.X = e.X; lastMousePos.Y = e.Y;

                        repaint = true;
                    }
                    else if ((g_nScalingItemFlags != 0) && (g_selectedLight != null))
                    {
                        float dx = (e.X - lastMousePos.X) * (1.0f / zoomLevel);
                        float dy = (e.Y - lastMousePos.Y) * (1.0f / zoomLevel);
                        //salvez datele inainte de modificare
                        RectangleF origaabb = g_selectedLight.area;
                        PointF origpos = g_selectedLight.pos;

                        if ((g_nScalingItemFlags & K_SCALE_FLAG_X) != 0)
                        {
                            g_selectedLight.area.X += dx;
                            g_selectedLight.area.Width -= dx;
                        }
                        if ((g_nScalingItemFlags & K_SCALE_FLAG_W) != 0)
                        {
                            g_selectedLight.area.Width += dx;
                        }
                        if ((g_nScalingItemFlags & K_SCALE_FLAG_Y) != 0)
                        {
                            g_selectedLight.area.Y += dy;
                            g_selectedLight.area.Height -= dy;
                        }
                        if ((g_nScalingItemFlags & K_SCALE_FLAG_H) != 0)
                        {
                            g_selectedLight.area.Height += dy;
                        }

                        //daca e pointlight fortez sa scaleze cu acelasi aspect ratio si centrul luminii sa ramana unde trebuie
                        if ((g_selectedLight.type == K_LIGHT_POINT) || (g_selectedLight.type == K_LIGHT_AREA))
                        {
                            if ((Control.ModifierKeys & Keys.Shift) != Keys.None)
                            {
                                float diffx = g_selectedLight.area.Width - origaabb.Width;
                                float diffy = g_selectedLight.area.Height - origaabb.Height;
                                if (Math.Abs(diffx) > Math.Abs(diffy))
                                    g_selectedLight.area.Height = g_selectedLight.area.Width * (origaabb.Height / origaabb.Width);
                                else
                                    g_selectedLight.area.Width = g_selectedLight.area.Height * (origaabb.Width / origaabb.Height);
                            }
                            //repozitionez si lumina
                            g_selectedLight.pos.X = ((origpos.X - origaabb.X) / origaabb.Width) * g_selectedLight.area.Width + g_selectedLight.area.X;
                            g_selectedLight.pos.Y = ((origpos.Y - origaabb.Y) / origaabb.Height) * g_selectedLight.area.Height + g_selectedLight.area.Y;
                        }
                        //daca nu este pointlight nu conteaza pozitia luminii deci o fortez sa fie centrata
                        else
                        {
                            //forteaza sa fie patrat si repozitioneaza lumina in centru
                            g_selectedLight.pos.X = g_selectedLight.area.X + g_selectedLight.area.Width / 2.0f;
                            g_selectedLight.pos.Y = g_selectedLight.area.Y + g_selectedLight.area.Height / 2.0f;
                        }

                        repaint = true;
                        lastMousePos.X = e.X; lastMousePos.Y = e.Y;
                    }
                }
                else if (g_brushMode == BRUSH_MODE_COLLISIONS)
                {
                    if (g_bCreatingItem)
                    {
                        Point pt = cursorPos;
                        if (chk_snapToGrid.Checked)
                            pt = SnapPointToGrid(cursorPos, K_SNAP_THRESHOLD_COLLISIONS);
                        //folosesc datele din selectedArea ca sa facem collBoxul
                        g_SelectedArea.Width = pt.X - g_SelectedArea.X;
                        g_SelectedArea.Height = pt.Y - g_SelectedArea.Y;

                        repaint = true;
                    }
                    else
                    {
                        if ((g_bDraggingItem) && (g_selectedCollision != null))
                        {
                            float dx = (e.X - lastMousePos.X) * (1.0f / zoomLevel);
                            float dy = (e.Y - lastMousePos.Y) * (1.0f / zoomLevel);

                            g_selectedCollision.rect.X += dx;
                            g_selectedCollision.rect.Y += dy;

                            repaint = true;
                            lastMousePos.X = e.X; lastMousePos.Y = e.Y;
                        }
                        else if ((g_nScalingItemFlags != 0) && (g_selectedCollision != null))
                        {
                            float dx = (e.X - lastMousePos.X) * (1.0f / zoomLevel);
                            float dy = (e.Y - lastMousePos.Y) * (1.0f / zoomLevel);

                            if ((g_nScalingItemFlags & K_SCALE_FLAG_X) != 0)
                            {
                                g_selectedCollision.rect.X += dx;
                                g_selectedCollision.rect.Width -= dx;
                            }
                            if ((g_nScalingItemFlags & K_SCALE_FLAG_W) != 0)
                            {
                                g_selectedCollision.rect.Width += dx;
                            }
                            if ((g_nScalingItemFlags & K_SCALE_FLAG_Y) != 0)
                            {
                                g_selectedCollision.rect.Y += dy;
                                g_selectedCollision.rect.Height -= dy;
                            }
                            if ((g_nScalingItemFlags & K_SCALE_FLAG_H) != 0)
                            {
                                g_selectedCollision.rect.Height += dy;
                            }

                            repaint = true;
                            lastMousePos.X = e.X; lastMousePos.Y = e.Y;
                        }
                    }
                }
                else if (g_brushMode == BRUSH_MODE_OBJECTS)
                {
                    if ((g_bDraggingItem) && (g_selectedObject != null))
                    {
                        float dx = cursorPosF.X - lastMousePos.X;
                        float dy = cursorPosF.Y - lastMousePos.Y;

                        if ((dx != 0.0f) || (dy != 0.0f))
                        {
                            PointF npos = new PointF(lastMousePos.X + dx, lastMousePos.Y + dy);
                            if (chk_snapToGrid.Checked)
                            {
                                npos = SnapPointToGrid(npos, K_SNAP_THRESHOLD);
                            }

                            g_selectedObject.pos.X = (int)npos.X;
                            g_selectedObject.pos.Y = (int)npos.Y;

                            repaint = true;
                        }
                    }
                }
                else if (g_brushMode == BRUSH_MODE_ACTORS)
                {
                    if ((g_bDraggingItem) && (g_selectedActor != null))
                    {
                        float dx = cursorPosF.X - lastMousePos.X;
                        float dy = cursorPosF.Y - lastMousePos.Y;

                        if ((dx != 0.0f) || (dy != 0.0f))
                        {
                            PointF npos = new PointF(lastMousePos.X + dx, lastMousePos.Y + dy);
                            if (chk_snapToGrid.Checked)
                            {
                                npos = SnapPointToGrid(npos, K_SNAP_THRESHOLD);
                            }

                            if (g_selectedActor != null)
                            {
                                g_selectedActor.pos.X = (int)npos.X;
                                g_selectedActor.pos.Y = (int)npos.Y;
                            }
                            repaint = true;
                        }
                    }
                }
                else if (g_brushMode == BRUSH_MODE_MISC)
                {
                    if (g_selectedMisc != null)
                    {
                        if (chk_snapToGrid.Checked)
                            g_selectedMisc.OnMouseMove(cursorPosSnapped);
                        else
                            g_selectedMisc.OnMouseMove(cursorPos);
                        repaint = true;
                    }
                }
            }
            else if (e.Button == MouseButtons.Right)
            {
                if (g_brushMode == BRUSH_MODE_TILES)
                {
                    if ((g_matBrush.Width == 1) && (g_matBrush.Height == 1))
                    {
                        CTileBlock tb = gMap.setTile(tlx, tly, -1, g_selectedLayer);
                        BuildBlockImage(tb);
                    }
                    else
                    {
                        for (int kk = 0; kk < g_matBrush.Width; kk++)
                        {
                            for (int ll = 0; ll < g_matBrush.Height; ll++)
                            {
                                CTileBlock tb = gMap.setTile(tlx + kk, tly + ll, -1, g_selectedLayer);
                                BuildBlockImage(tb);
                            }
                        }
                    }
                    gMap.ClearEmptyblocks();
                }

                repaint = true;
            }
            else if (e.Button == MouseButtons.Middle)
            {
                if (draggingMap)
                {
                    cameraPos.X -= (e.X - lastMousePos.X) * (1.0f / zoomLevel);
                    cameraPos.Y -= (e.Y - lastMousePos.Y) * (1.0f / zoomLevel);

                    lastMousePos.X = e.X;
                    lastMousePos.Y = e.Y;

                    repaint = true;
                }
            }
            //daca s-a cerut repaint redeseneaza
            if (repaint)
                PaintMap(e.X, e.Y);
        }

        private void pictureBox1_MouseUp(object sender, MouseEventArgs e)
        {
            bool repaint = false;

            g_lastMousePos.X = e.X; g_lastMousePos.Y = e.Y;
            g_hoveredTile.X = (int)(e.X * (1.0f / zoomLevel) + cameraPos.X) / TILE_HEIGHT;
            g_hoveredTile.Y = (int)(e.Y * (1.0f / zoomLevel) + cameraPos.Y) / TILE_HEIGHT;

            PointF cursorPos = new PointF((e.X * (1.0f / zoomLevel) + cameraPos.X), (e.Y * (1.0f / zoomLevel) + cameraPos.Y));
            g_lastHoveredPoint = cursorPos;

            if (e.Button == MouseButtons.Middle)
            {
                Cursor.Current = Cursors.Default;
            }

            if (e.Button == MouseButtons.Left)
            {
                if (g_selectingStatus == K_SEL_STATUS_DRAGGING)
                {
                    g_SelectedArea.Width = cursorPos.X - g_SelectedArea.X; g_SelectedArea.Height = cursorPos.Y - g_SelectedArea.Y;
                    g_SelectedAreaTL.Width = g_hoveredTile.X - g_SelectedAreaTL.X; g_SelectedAreaTL.Height = g_hoveredTile.Y - g_SelectedAreaTL.Y;
                    g_SelectedArea = NormalizeRect(g_SelectedArea);
                    g_SelectedAreaTL = NormalizeRect(g_SelectedAreaTL);
                    g_SelectedAreaTL.Width += 1;
                    g_SelectedAreaTL.Height += 1;

                    g_selectingStatus = K_SEL_STATUS_EMPTY;

                    repaint = true;
                }
                else if (g_brushMode == BRUSH_MODE_TILES)
                {
                    //finished painting so set current undo state
                    Undo_SetCurrent(K_UNDO_TILES);
                }
                else if (g_brushMode == BRUSH_MODE_OBJECTS)
                {
                    if (g_bDraggingItem)
                        g_bDraggingItem = false;

                    //finished dragging, offer to move linked boxes
                    if (g_selectedObject != null)
                    {
                        int nLinkedBoxes = GetCollisionsWithTargetIDCount((int)g_selectedObject.ID, true);
                        if (nLinkedBoxes > 0)
                        {
                            //DialogResult dr = MessageBox.Show("There are moving collision elements that point to this object. We suggest you move those too.\n\rDo you want to move them now?", "Info", MessageBoxButtons.YesNo);
                            //if (dr == DialogResult.Yes)
                            if(g_wndObjects.GetFlag_AffectChildren())
                            {
                                float dX = (g_selectedObject.pos.X - g_selectedObjectOldPos.X);
                                float dY = (g_selectedObject.pos.Y - g_selectedObjectOldPos.Y);

                                for (int kk = arrCollisions.Count - 1; kk >= 0; kk--)
                                {
                                    CCollisionElement coll = arrCollisions[kk] as CCollisionElement;
                                    if (coll.logic.targetID == g_selectedObject.ID)
                                    {
                                        //#HARDCODE: only move solid collisions
                                        if (coll.IsSolid() == false)
                                            continue;
                                        //move it
                                        coll.rect.X += dX;
                                        coll.rect.Y += dY;
                                    }
                                }
                            }
                        }
                    }
                    repaint = true;
                }
                else if (g_brushMode == BRUSH_MODE_LIGHTS)
                {
                    bool snapGrid = chk_snapToGrid.Checked;

                    if (g_bDraggingItem)
                    {
                        g_bDraggingItem = false;
                        if ((snapGrid) && (g_selectedLight != null))
                        {
                            if ((g_selectedLight.type != K_LIGHT_POINT) && (g_selectedLight.type != K_LIGHT_AREA))
                            {
                                g_selectedLight.area.X = (int)Math.Round(g_selectedLight.area.X / (float)TILE_WIDTH) * TILE_WIDTH;
                                g_selectedLight.area.Y = (int)Math.Round(g_selectedLight.area.Y / (float)TILE_HEIGHT) * TILE_HEIGHT;
                                g_selectedLight.area.Width = (int)Math.Round(g_selectedLight.area.Width / (float)TILE_WIDTH) * TILE_WIDTH;
                                g_selectedLight.area.Height = (int)Math.Round(g_selectedLight.area.Height / (float)TILE_HEIGHT) * TILE_HEIGHT;

                                g_selectedLight.pos.X = g_selectedLight.area.X + g_selectedLight.area.Width / 2.0f;
                                g_selectedLight.pos.Y = g_selectedLight.area.Y + g_selectedLight.area.Height / 2.0f;
                            }
                            repaint = true;
                        }
                    }
                    if (g_nScalingItemFlags != 0)
                    {
                        g_nScalingItemFlags = 0;
                        //daca e point light forteaza sa fie patrat
                        if ((g_selectedLight.type != K_LIGHT_POINT) && (g_selectedLight.type != K_LIGHT_AREA))
                        {
                            if ((snapGrid) && (g_selectedLight != null))
                            {
                                g_selectedLight.area.X = (int)Math.Round(g_selectedLight.area.X / (float)TILE_WIDTH) * TILE_WIDTH;
                                g_selectedLight.area.Y = (int)Math.Round(g_selectedLight.area.Y / (float)TILE_HEIGHT) * TILE_HEIGHT;
                                g_selectedLight.area.Width = (int)Math.Round(g_selectedLight.area.Width / (float)TILE_WIDTH) * TILE_WIDTH;
                                g_selectedLight.area.Height = (int)Math.Round(g_selectedLight.area.Height / (float)TILE_HEIGHT) * TILE_HEIGHT;
                            }

                            g_selectedLight.pos.X = g_selectedLight.area.X + g_selectedLight.area.Width / 2.0f;
                            g_selectedLight.pos.Y = g_selectedLight.area.Y + g_selectedLight.area.Height / 2.0f;
                        }

                        repaint = true;
                    }

                }
                else if (g_brushMode == BRUSH_MODE_COLLISIONS)
                {
                    bool snapGrid = chk_snapToGrid.Checked;
                    int shrinkPx = 1; //constant!

                    if (g_bCreatingItem)
                    {
                        //daca dai doar click nu adauga
                        if ((Math.Abs(g_SelectedArea.Width) <= 0.0f) || (Math.Abs(g_SelectedArea.Height) <= 0.0f))
                            return;

                        Rectangle rct = RectFtoRect(g_SelectedArea);

                        //verificari dimensiuni minime
                        rct = NormalizeRect(rct);
                        if (rct.Width < 3)
                            rct.Width = 3;
                        if (rct.Height < 3)
                            rct.Height = 3;

                        //shrink
                        if ((g_wndCollisions.ShrinkByAxis) && ((rct.Width <= TILE_WIDTH) || (rct.Height <= TILE_HEIGHT)))
                        {
                            if (rct.Height > rct.Width) //daca e vertical, de un tile
                            {
                                //#HACK: aici e facut sa se potriveasca fix pe engine-ul cu perspectiva la peretii sectiune
                                if (rct.Height <= 3 * TILE_HEIGHT)  //poate fi perete deasupra usilor
                                {
                                    rct.X += 6;
                                    rct.Y -= 1;
                                    rct.Width = 9;
                                }
                                else  //este perete normal vertical
                                {
                                    rct.X += 6;
                                    rct.Width = 9;
                                    rct.Y -= 1;
                                    rct.Height += 2;
                                }
                            }
                            else if (rct.Width > rct.Height) //orizontal sau patrat
                            {
                                rct.Y += shrinkPx;
                                rct.Height -= 2 * shrinkPx;
                                rct.X += 6;
                                rct.Width -= 7;
                            }
                            else
                            {
                                rct.X += shrinkPx; rct.Y += shrinkPx;
                                rct.Width -= 2 * shrinkPx; rct.Height -= 2 * shrinkPx;
                            }
                        }
                        else
                        {
                            rct.X += shrinkPx; rct.Y += shrinkPx;
                            rct.Width -= 2 * shrinkPx; rct.Height -= 2 * shrinkPx;
                        }

                        //adaug collision shape
                        if ((rct.Width > 0) && (rct.Height > 0))
                        {

                            CCollisionElement coll = new CCollisionElement();
                            coll.ID = GetUniqueID();
                            coll.rect = rct;
                            coll.castShadows = true;
                            coll.type = K_COLL_TYPE_SOLID;

                            arrCollisions.Add(coll);
                            //g_selectedCollision = arrCollisions[arrCollisions.Count - 1] as CCollisionElement;
                        }
                        //nu selectam ultimul collrect adaugat!!
                        //g_wndCollisions.SetSelectedCollision(g_selectedCollision);
                        g_bCreatingItem = false;

                        repaint = true;
                    }
                    else
                    {
                        bool bMoved = ((lastClickPos.X != e.X) || (lastClickPos.Y != e.Y));
                        if (bMoved)
                        {
                            //verificari dimensiuni minime
                            g_selectedCollision.rect = NormalizeRect(g_selectedCollision.rect);

                            if (g_bDraggingItem)
                            {
                                g_bDraggingItem = false;
                                if (snapGrid)
                                {
                                    g_selectedCollision.rect.X = (int)Math.Round(g_selectedCollision.rect.X / (float)TILE_WIDTH) * TILE_WIDTH;
                                    g_selectedCollision.rect.Y = (int)Math.Round(g_selectedCollision.rect.Y / (float)TILE_HEIGHT) * TILE_HEIGHT;
                                    g_selectedCollision.rect.Width = (int)Math.Round(g_selectedCollision.rect.Width / (float)TILE_WIDTH) * TILE_WIDTH;
                                    g_selectedCollision.rect.Height = (int)Math.Round(g_selectedCollision.rect.Height / (float)TILE_HEIGHT) * TILE_HEIGHT;
                                    //micsoreaza dupa snap
                                    if (g_wndCollisions.ShrinkByAxis)
                                    {
                                        if (g_selectedCollision.rect.Width > g_selectedCollision.rect.Height)
                                            g_selectedCollision.rect.Inflate(0.0f, -shrinkPx);
                                        else
                                            g_selectedCollision.rect.Inflate(-shrinkPx, 0.0f);
                                    }
                                    else
                                        g_selectedCollision.rect.Inflate(-shrinkPx, -shrinkPx);

                                    repaint = true;
                                }
                                else //poate fi pus doar pe intregi de pixeli
                                {
                                    g_selectedCollision.rect.X = (int)(g_selectedCollision.rect.X);
                                    g_selectedCollision.rect.Y = (int)(g_selectedCollision.rect.Y);
                                    repaint = true;
                                }
                            }
                            else if (g_nScalingItemFlags != 0)
                            {
                                g_nScalingItemFlags = 0;
                                if (snapGrid)
                                {
                                    g_selectedCollision.rect.X = (int)Math.Round(g_selectedCollision.rect.X / (float)TILE_WIDTH) * TILE_WIDTH;
                                    g_selectedCollision.rect.Y = (int)Math.Round(g_selectedCollision.rect.Y / (float)TILE_HEIGHT) * TILE_HEIGHT;
                                    g_selectedCollision.rect.Width = (int)Math.Round(g_selectedCollision.rect.Width / (float)TILE_WIDTH) * TILE_WIDTH;
                                    g_selectedCollision.rect.Height = (int)Math.Round(g_selectedCollision.rect.Height / (float)TILE_HEIGHT) * TILE_HEIGHT;
                                    //micsoreaza dupa snap
                                    if (g_wndCollisions.ShrinkByAxis)
                                    {
                                        if (g_selectedCollision.rect.Width > g_selectedCollision.rect.Height)
                                            g_selectedCollision.rect.Inflate(0.0f, -shrinkPx);
                                        else
                                            g_selectedCollision.rect.Inflate(-shrinkPx, 0.0f);
                                    }
                                    else
                                        g_selectedCollision.rect.Inflate(-shrinkPx, -shrinkPx);

                                    repaint = true;
                                }
                                else //poate fi pus doar pe intregi de pixeli
                                {
                                    g_selectedCollision.rect.X = (int)(g_selectedCollision.rect.X);
                                    g_selectedCollision.rect.Y = (int)(g_selectedCollision.rect.Y);
                                    g_selectedCollision.rect.Width = (int)(g_selectedCollision.rect.Width);
                                    g_selectedCollision.rect.Height = (int)(g_selectedCollision.rect.Height);
                                    repaint = true;
                                }
                            }

                            //minimum size
                            if (g_selectedCollision.rect.Width < 5)
                                g_selectedCollision.rect.Width = 5;
                            if (g_selectedCollision.rect.Height < 5)
                                g_selectedCollision.rect.Height = 5;
                        }
                    }
                }
                else if (g_brushMode == BRUSH_MODE_MISC)
                {
                    if (g_bDraggingItem)
                        g_bDraggingItem = false;

                    if (g_selectedMisc != null)
                    {
                        g_selectedMisc.OnMouseUp(cursorPos);
                    }
                }
                else if (g_brushMode == BRUSH_MODE_ACTORS)
                {
                    if (g_bDraggingItem)
                        g_bDraggingItem = false;
                }

            }
            else if (e.Button == MouseButtons.Middle)
            {
                draggingMap = false;
            }
            else if (e.Button == MouseButtons.Right)
            {
                if (g_brushMode == BRUSH_MODE_TILES)
                {
                    //finished painting so set current undo state
                    Undo_SetCurrent(K_UNDO_TILES);
                }
            }

            //repaint map
            if (repaint)
                PaintMap(e.X, e.Y);

            UpdateStatusBarMessage();
        }

        private void aboutToolStripMenuItem_Click(object sender, EventArgs e)
        {
            MessageBox.Show("BreacherSquad Levels Editor\nv1.2.3 from 07-Mar-2020\n(c)2021 PixelShard", "About", MessageBoxButtons.OK);
        }

        private void exitToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (g_bFileModified)
            {
                if (MessageBox.Show("Really QUIT without saving?", "Discard changes", MessageBoxButtons.YesNo) == DialogResult.No)
                {
                    return;
                }
            }
            //exit
            this.Dispose();
        }


        // saves data about the level blocks and connectivity
        public void SaveLevelDescriptorXML(string strPath)
        {
            //gaseste minimul si maximul tablei de joc, in blocuri si salveaza latimea si inaltimea nivelului, in blocuri
            Int32 blminx = 100000, blminy = 100000, blmaxx = -100000, blmaxy = -100000;
            for (int kk = 0; kk < gMap.Blocks.Count; kk++)
            {
                CTileBlock blk = gMap.Blocks[kk] as CTileBlock;
                if (blk.pos.X < blminx) blminx = blk.pos.X;
                if (blk.pos.Y < blminy) blminy = blk.pos.Y;
                if (blk.pos.X > blmaxx) blmaxx = blk.pos.X;
                if (blk.pos.Y > blmaxy) blmaxy = blk.pos.Y;
            }

            if ((blminx == 100000) || (blminy == 100000)) //no tiles
            {
                MessageBox.Show("Cand't save level descriptor for empty levels!");
                return;
            }

            // level description is saved like a string, top left to bottom right, line by line
            // 0 - not set, 1 - set block, LURD - connector direction on set block
            string strDesc = "";

            for (int yy = blminy; yy <= blmaxy; yy++)
            {
                for (int xx = blminx; xx <= blmaxx; xx++)
                {
                    CTileBlock blk = gMap.getBlockAt(xx * BLOCK_W + BLOCK_W / 2, yy * BLOCK_H + BLOCK_H / 2);
                    if (blk == null)
                    {
                        strDesc += "0";
                        continue;
                    }
                    //  check if bordering
                    CTileBlock testblk = null;
                    // LEFT
                    testblk = gMap.getBlockAt(blk.pos.X * BLOCK_W - BLOCK_W, blk.pos.Y * BLOCK_H);
                    if ((testblk == null) && (blk.HasConnectionOnSide(K_DIR_LEFT)))
                    {
                        strDesc += "L";
                        //MessageBox.Show("Found connection LEFT on block [" + blk.pos.X + "][" + blk.pos.Y + "]");
                        continue;
                    }
                    //RIGHT
                    testblk = gMap.getBlockAt(blk.pos.X * BLOCK_W + BLOCK_W, blk.pos.Y * BLOCK_H);
                    if ((testblk == null) && (blk.HasConnectionOnSide(K_DIR_RIGHT)))
                    {
                        strDesc += "R";
                        //MessageBox.Show("Found connection RIGHT on block [" + blk.pos.X + "][" + blk.pos.Y + "]");
                        continue;
                    }
                    //UP
                    testblk = gMap.getBlockAt(blk.pos.X * BLOCK_W, blk.pos.Y * BLOCK_H - BLOCK_H);
                    if ((testblk == null) && (blk.HasConnectionOnSide(K_DIR_UP)))
                    {
                        strDesc += "U";
                        //MessageBox.Show("Found connection UP on block [" + blk.pos.X + "][" + blk.pos.Y + "]");
                        continue;
                    }
                    //DOWN
                    testblk = gMap.getBlockAt(blk.pos.X * BLOCK_W, blk.pos.Y * BLOCK_H + BLOCK_W);
                    if ((testblk == null) && (blk.HasConnectionOnSide(K_DIR_DOWN)))
                    {
                        strDesc += "D";
                        //MessageBox.Show("Found connection DOWN on block [" + blk.pos.X + "][" + blk.pos.Y + "]");
                        continue;
                    }
                    // no connection but set:
                    strDesc += "1";
                }
            }



            try
            {
                XmlTextWriter xw = new XmlTextWriter(strPath, null);
                xw.Formatting = Formatting.Indented;
                xw.WriteStartDocument();
                // write elements
                xw.WriteStartElement("Area");
                xw.WriteAttributeString("File", Path.GetFileNameWithoutExtension(strPath));
                int blocksW = blmaxx - blminx + 1;
                int blocksH = blmaxy - blminy + 1;
                xw.WriteAttributeString("BlocksW", blocksW.ToString());
                xw.WriteAttributeString("BlocksH", blocksH.ToString());

                xw.WriteAttributeString("ConnectorsDesc", strDesc);
                xw.WriteAttributeString("Tags", "");
                // end LevelStory
                xw.WriteEndElement();
                // end document
                xw.WriteEndDocument();
                xw.Flush();
                xw.Close();
            }
            catch (Exception ex)
            {
                MessageBox.Show("Error saving area descriptor! \n\n" + ex.Message);
            }

            //MessageBox.Show(strDesc);
        }

        public bool SaveLevel_V2(string strPath, bool bExportPrefab = false, Stream pDestStream = null)
        {
            // save additional file with area descriptor
            SaveLevelDescriptorXML(Path.GetDirectoryName(strPath) + "\\" + Path.GetFileNameWithoutExtension(strPath) + ".area_desc");

            //gaseste minimul si maximul tablei de joc, in blocuri si salveaza latimea si inaltimea nivelului, in blocuri
            Int32 blminx = 100000, blminy = 100000, blmaxx = -100000, blmaxy = -100000;
            for (int kk = 0; kk < gMap.Blocks.Count; kk++)
            {
                CTileBlock blk = gMap.Blocks[kk] as CTileBlock;
                if (blk.pos.X < blminx) blminx = blk.pos.X;
                if (blk.pos.Y < blminy) blminy = blk.pos.Y;
                if (blk.pos.X > blmaxx) blmaxx = blk.pos.X;
                if (blk.pos.Y > blmaxy) blmaxy = blk.pos.Y;
            }

            bool bSkipLimitSearch = false;
            if ((blminx == 100000) || (blminy == 100000)) //no tiles
            {
                if (!bExportPrefab)
                {
                    MessageBox.Show("You can't export empty levels! You have to place some tiles!", "WARNING!", MessageBoxButtons.OK);
                    return false;
                }
                else
                {
                    blminx = blmaxx = gLevelOrigin.X / (TILE_WIDTH * BLOCK_W);
                    blminy = blmaxy = gLevelOrigin.Y / (TILE_HEIGHT * BLOCK_H);
                    bSkipLimitSearch = true;
                }
            }

            Point levelUL = new Point(blminx * BLOCK_W, blminy * BLOCK_H);
            Point levelDR = new Point((blmaxx + 1) * BLOCK_W, (blmaxy + 1) * BLOCK_H + BLOCK_H);

            if ((blminx == blmaxx) || (blminy == blmaxy)) //single block or no block
            {
                if (gMap.Blocks.Count > 0)
                {
                    levelDR.X = blmaxx * BLOCK_W + BLOCK_W;
                    levelDR.Y = blmaxy * BLOCK_H + BLOCK_H;
                }
                else
                {
                    //let it know we have no tiles
                    levelDR.X = levelUL.X - 1;
                    levelDR.Y = levelUL.Y - 1;
                }
            }

            //gaseste limitele exacte in fn de tiles
            if (!bSkipLimitSearch)
            {
                bool gasit = false;
                //orizontala xmin
                gasit = false;
                while (!gasit)
                {
                    for (int yy = levelUL.Y; yy <= levelDR.Y; yy++)
                    {
                        CTile tl = gMap.getTile(levelUL.X, yy);
                        if ((tl != null) && (!tl.IsEmpty()))
                        {
                            gasit = true;
                            break;
                        }
                    }
                    if (!gasit)
                        levelUL.X++;
                    if (levelUL.X >= levelDR.X)
                        gasit = true;
                }
                //orizontala xmax
                gasit = false;
                while (!gasit)
                {
                    for (int yy = levelUL.Y; yy <= levelDR.Y; yy++)
                    {
                        CTile tl = gMap.getTile(levelDR.X, yy);
                        if ((tl != null) && (!tl.IsEmpty()))
                        {
                            gasit = true;
                            break;
                        }
                    }
                    if (!gasit)
                        levelDR.X--;
                    if (levelDR.X <= levelUL.X)
                        gasit = true;
                }
                //verticala ymin
                gasit = false;
                while (!gasit)
                {
                    for (int xx = levelUL.X; xx <= levelDR.X; xx++)
                    {
                        CTile tl = gMap.getTile(xx, levelUL.Y);
                        if ((tl != null) && (!tl.IsEmpty()))
                        {
                            gasit = true;
                            break;
                        }
                    }
                    if (!gasit)
                        levelUL.Y++;
                    if (levelUL.Y >= levelDR.Y)
                        gasit = true;
                }
                //verticala maxy
                gasit = false;
                while (!gasit)
                {
                    for (int xx = levelUL.X; xx <= levelDR.X; xx++)
                    {
                        CTile tl = gMap.getTile(xx, levelDR.Y);
                        if ((tl != null) && (!tl.IsEmpty()))
                        {
                            gasit = true;
                            break;
                        }
                    }
                    if (!gasit)
                        levelDR.Y--;
                    if (levelDR.Y <= levelUL.Y)
                        gasit = true;
                }
            }
            //--- adauga un nr de tiles pentru cer, deasupra nivelului ---
            if(!bExportPrefab)
                levelUL.Y -= K_SKY_TILES_ADDED;


            if (((levelDR.X - levelUL.X) > 500) || ((levelDR.Y - levelUL.Y) > 500))
            {
                if (MessageBox.Show("Warning! Level too big: W:" + (levelDR.X - levelUL.X + 1) + " H:" + (levelDR.Y - levelUL.Y + 1) + "\nDoriti sa continuati?", "WARNING!", MessageBoxButtons.OKCancel) == DialogResult.Cancel)
                    return false;
            }

            //face diverse verificari
            string errorstxt = "";
            int errors = 0;

            if (!bExportPrefab)
            {
                if (g_wndMaterials.g_TilesetName.Length <= 0)
                {
                    errorstxt += "- No Tileset loaded! Load a tileset first from the materials window.\n";
                    errors++;
                }

                if ((g_sprObjects == null) || (g_sprObjects.bLoaded == false))
                {
                    errorstxt += "- Objects sprite not loaded! Load a sprite first from the objects window.\n";
                    errors++;
                }

                if ((g_sprActors == null) || (g_sprActors.bLoaded == false))
                {
                    errorstxt += "- Actors sprite not loaded! Load a sprite first from the actors window.\n";
                    errors++;
                }

                if ((levelDR.X == levelUL.X) || (levelDR.Y == levelUL.Y))
                {
                    errorstxt += "- Can't save an empty level!\n";
                    errors++;
                }
                //can't save levels without a background
                /*
                bool bFoundBg = false;
                for (int kk = 0; kk < arrMisc.Count; kk++)
                {
                    CMiscObjectBase obj = (arrMisc[kk] as CMiscObjectBase);
                    if (obj.type == K_MISC_BACKGROUND)
                        bFoundBg = true;
                }
                if (bFoundBg == false)
                {
                    errorstxt += "You can't save a level without setting a background!\n\rGo to Edit->Set Level Background and select one.";
                    errors++;
                }
                */
            }

            //can't save objects with negative anims/frames
            for (int kk = 0; kk < arrObjects.Count; kk++)
            {
                CObject obj = (arrObjects[kk] as CObject);
                if ((obj.animIdx < 0) || (obj.frameIdx < 0))
                {
                    errorstxt += "- Object ID " + obj.ID + " doesn't have correct animation or frame!\n";
                    errors++;
                }
            }

            if (errors > 0)
            {
                MessageBox.Show("You have errors:\n" + errorstxt, "Warning!", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
                return false;
            }

            try
            {
                //--- start writing level ---
                Stream pLocalStream = null;
                if (pDestStream == null)
                {
                    if (File.Exists(strPath))
                        File.Delete(strPath);

                    FileStream fs = new FileStream(strPath, FileMode.Create);
                    pLocalStream = fs;
                }
                else
                {
                    pLocalStream = pDestStream;
                }

                BinaryWriter bw = new BinaryWriter(pLocalStream);

                Byte ub;
                Int16 s2b;
                UInt16 u2b;
                UInt32 u4b;
                int s4b;
                //0. scrie versiune fisier si alte date necesare
                Int32[] verdata = new Int32[] { K_CURRENT_VERSION, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
                for (int kk = 0; kk < 10; kk++)
                    bw.Write(verdata[kk]);

                //1. scrie tipul misiunii
                ub = g_missionType;
                bw.Write(ub);

                //scrie numele tilesetului si date despre tileset
                bw.Write(g_wndMaterials.g_TilesetName);
                ub = (byte)TILE_WIDTH; bw.Write(ub);
                ub = (byte)TILE_HEIGHT; bw.Write(ub);
                u2b = (UInt16)TILESET_COLUMNS; bw.Write(u2b);

                //3. scrie marimea exacta in tiles a nivelului
                u2b = (UInt16)(levelDR.X - levelUL.X + 1);
                bw.Write(u2b);
                u2b = (UInt16)(levelDR.Y - levelUL.Y + 1);
                bw.Write(u2b);
                //origine nivel
                s2b = (short)(gLevelOrigin.Y - levelUL.Y * TILE_WIDTH);
                bw.Write(s2b);
                s2b = (short)(gLevelOrigin.X - levelUL.X * TILE_HEIGHT);
                bw.Write(s2b);

                //6. scrie tile-urile pe rand, toata matricea
                for (int yy = levelUL.Y; yy <= levelDR.Y; yy++)
                {
                    for (int xx = levelUL.X; xx <= levelDR.X; xx++)
                    {
                        CTile tl = gMap.getTile(xx, yy);
                        if (tl != null)
                        {
                            //scrie layerele de tiles
                            for (int kk = 0; kk < (int)ELayer.LAYERS_CNT; kk++)
                            {
                                s4b = tl.tileID[kk];
                                if (s4b < 0)
                                    s4b = -1;
                                bw.Write(s4b);
                            }
                        }
                        else //daca nu e bloc
                        {
                            s4b = -1;
                            for (int kk = 0; kk < (int)ELayer.LAYERS_CNT; kk++)
                            {
                                bw.Write(s4b);
                            }
                        }
                    }
                }

                ///--- scrie luminile ---
                // calea catre bsx
                string bsxLightsFilename = Path.GetFileName(g_sprLights.loadedPath);
                bw.Write(bsxLightsFilename);
                //nr lumini
                u4b = (UInt32)arrLights.Count;
                bw.Write(u4b);
                //date fiecare 
                for (int kk = 0; kk < arrLights.Count; kk++)
                {
                    CLight light = arrLights[kk] as CLight;
                    //ID
                    u4b = light.ID;
                    bw.Write(u4b);
                    //tip
                    ub = (byte)light.type;
                    bw.Write(ub);
                    //atmospheric volume atten
                    u4b = (UInt32)light.nAtmoAttenuationPerc;
                    bw.Write(u4b);
                    //intensitate
                    bw.Write(light.fIntensity);
                    //pozitia
                    s4b = (Int32)(light.pos.X - (levelUL.X * TILE_WIDTH));
                    bw.Write(s4b);
                    s4b = (Int32)(light.pos.Y - (levelUL.Y * TILE_HEIGHT));
                    bw.Write(s4b);
                    s4b = (Int32)light.posZ;
                    bw.Write(s4b);
                    //anim name
                    string strLightAnm = "_empty_"; //this way when loading we don't find it and we set it on -1
                    if ((light.animId >= 0) && (light.animId < g_sprLights.anims.Count))
                        strLightAnm = g_sprLights.anims[light.animId].name;
                    bw.Write(strLightAnm);
                    //scrie culoarea
                    bw.Write(light.color.A);
                    bw.Write(light.color.R);
                    bw.Write(light.color.G);
                    bw.Write(light.color.B);
                    //scrie zona luminii
                    s4b = (Int32)(light.area.X - (levelUL.X * TILE_WIDTH)); bw.Write(s4b);
                    s4b = (Int32)(light.area.Y - (levelUL.Y * TILE_WIDTH)); bw.Write(s4b);
                    s4b = (Int32)light.area.Width; bw.Write(s4b);
                    s4b = (Int32)light.area.Height; bw.Write(s4b);
                    //unghi
                    s2b = (Int16)(light.angle); //0-360
                    bw.Write(s2b);
                    //light flags
                    u2b = 0;
                    if (light.castsShadows)
                        u2b |= K_LIGHT_FLAG_CAST_SHADOWS;
                    bw.Write(u2b);
                    //logic
                    light.logic.Save(bw);
                }

                //scrie formele de coliziune
                u4b = (UInt32)arrCollisions.Count;
                bw.Write(u4b);
                //date fiecare element
                for (int kk = 0; kk < arrCollisions.Count; kk++)
                {
                    CCollisionElement coli = arrCollisions[kk] as CCollisionElement;
                    //vor fi poate mai multe tipuri de forme dar deocamdata consider totul dreptunghiuri
                    //ID
                    u4b = coli.ID;
                    bw.Write(u4b);
                    //collision rect
                    s4b = (Int32)(coli.rect.X - (levelUL.X * TILE_WIDTH));
                    bw.Write(s4b);
                    s4b = (Int32)(coli.rect.Y - (levelUL.Y * TILE_HEIGHT));
                    bw.Write(s4b);
                    u4b = (UInt32)coli.rect.Width;
                    bw.Write(u4b);
                    u4b = (UInt32)coli.rect.Height;
                    bw.Write(u4b);
                    //type
                    byte tip = (byte)coli.type;
                    bw.Write(tip);
                    //cast shadows
                    ub = 0;
                    if (coli.castShadows)
                        ub = 1;
                    bw.Write(ub);

                    //logic
                    coli.logic.Save(bw);
                }

                //scrie decoratiunile/actives
                string bsxFilename = Path.GetFileName(g_sprObjects.loadedPath);
                bw.Write(bsxFilename);

                //nr deco
                u4b = (UInt32)arrObjects.Count;
                bw.Write(u4b);
                //date fiecare deco
                for (int kk = 0; kk < arrObjects.Count; kk++)
                {
                    CObject obj = arrObjects[kk] as CObject;
                    //ID
                    bw.Write((UInt32)obj.ID);
                    //layer
                    ub = (byte)obj.layer;
                    bw.Write(ub);
                    //pozitia
                    s4b = (Int32)(obj.pos.X - (levelUL.X * TILE_WIDTH));
                    bw.Write(s4b);
                    s4b = (Int32)(obj.pos.Y - (levelUL.Y * TILE_HEIGHT));
                    bw.Write(s4b);
                    //scrie anim si frame
                    string strAnmName = "_empty_"; //this way when loading we don't find it and we set it on -1
                    if ((obj.animIdx >= 0) && (obj.animIdx < g_sprObjects.anims.Count))
                        strAnmName = g_sprObjects.anims[obj.animIdx].name;
                    bw.Write(strAnmName);
                    //scrie frame
                    u2b = (UInt16)(obj.frameIdx);
                    bw.Write(u2b);
                    //scrie flags
                    bw.Write(obj.flags);

                    //logic
                    obj.logic.Save(bw);
                }

                ///--- scrie Actorii - inamici si personaj ---
                //scrie bsx pt actori
                string bsxActorFilename = Path.GetFileName(g_sprActors.loadedPath);
                bw.Write(bsxActorFilename);
                //their number too
                u4b = (UInt32)arrActors.Count;
                bw.Write(u4b);
                //date fiecare actor
                for (int kk = 0; kk < arrActors.Count; kk++)
                {
                    CActor act = arrActors[kk] as CActor;
                    //ID
                    bw.Write((UInt32)act.ID);
                    //pozitia
                    s4b = (Int32)(act.pos.X - (levelUL.X * TILE_WIDTH));
                    bw.Write(s4b);
                    s4b = (Int32)(act.pos.Y - (levelUL.Y * TILE_HEIGHT));
                    bw.Write(s4b);
                    //unghi si boolean SetAngle
                    bw.Write((Byte)((act.bSetAngle) ? 1 : 0));
                    s2b = (Int16)(act.fAngle); //0-360
                    bw.Write(s2b);
                    //scrie nume template
                    bw.Write(act.templateName);
                    //scrie nume starting AI state
                    bw.Write(act.strSelectedAIState);
                    //directie start
                    bw.Write((Byte)((act.bLookLeft) ? 1 : 0));
                    //collision/gravity
                    bw.Write((Byte)((act.bHasCollision) ? 1 : 0));
                    bw.Write((Byte)((act.bHasGravity) ? 1 : 0));
                    //logic
                    act.logic.Save(bw);
                }

                //scrie misc objects (rails samd)
                u4b = (UInt32)arrMisc.Count;
                bw.Write(u4b);
                //date fiecare misc object
                for (int kk = 0; kk < arrMisc.Count; kk++)
                {
                    CMiscObjectBase baseobj = arrMisc[kk] as CMiscObjectBase;
                    //datele generice:
                    //tipul
                    byte u1b = baseobj.type;
                    bw.Write(u1b);
                    //ID
                    bw.Write(baseobj.ID);
                    //params
                    u1b = (Byte)(baseobj.listParams.Count / 2); //nr de params
                    bw.Write(u1b);
                    for (int i = 0; i < baseobj.listParams.Count; i++)
                    {
                        bw.Write(baseobj.listParams[i] as string);
                    }
                    //datele particulare
                    switch (baseobj.type)
                    {
                        case K_MISC_RAIL:
                            CMiscObject_Rail rail = arrMisc[kk] as CMiscObject_Rail;
                            //nr de puncte
                            u2b = (UInt16)rail.listPoints.Count;
                            bw.Write(u2b);
                            //coordonate puncte
                            for (int i = 0; i < rail.listPoints.Count; i++)
                            {
                                PointF ptpos = (PointF)(rail.listPoints[i]);
                                s4b = (Int32)((int)ptpos.X - (levelUL.X * TILE_WIDTH));
                                bw.Write(s4b);
                                s4b = (Int32)((int)ptpos.Y - (levelUL.Y * TILE_HEIGHT));
                                bw.Write(s4b);
                            }
                            break;
                        case K_MISC_SCRIPT:
                            {
                                CMiscObject_Script scr = arrMisc[kk] as CMiscObject_Script;
                                PointF pos = (PointF)(scr.pos);
                                s4b = (Int32)((int)pos.X - (levelUL.X * TILE_WIDTH));
                                bw.Write(s4b);
                                s4b = (Int32)((int)pos.Y - (levelUL.Y * TILE_HEIGHT));
                                bw.Write(s4b);
                            }
                            break;
                        case K_MISC_BACKGROUND:
                            {
                                CMiscObject_Background back = arrMisc[kk] as CMiscObject_Background;
                                PointF pos = (PointF)(back.pos);
                                s4b = (Int32)((int)pos.X - (levelUL.X * TILE_WIDTH));
                                bw.Write(s4b);
                                s4b = (Int32)((int)pos.Y - (levelUL.Y * TILE_HEIGHT));
                                bw.Write(s4b);
                            }
                            break;
                        case K_MISC_FRONTLAYEROBJ:
                            {
                                CMiscObject_FrontLayerObj obj = arrMisc[kk] as CMiscObject_FrontLayerObj;
                                PointF pos = (PointF)(obj.pos);
                                s4b = (Int32)((int)pos.X - (levelUL.X * TILE_WIDTH));
                                bw.Write(s4b);
                                s4b = (Int32)((int)pos.Y - (levelUL.Y * TILE_HEIGHT));
                                bw.Write(s4b);
                            }
                            break;

                        default:
                            MessageBox.Show("SaveAs:Misc object type not saved!");
                            break;
                    }
                }

                bw.Flush();
                //close stream if reading from file or rewind if reading from memory
                if (pDestStream == null)
                    pLocalStream.Dispose();
                else
                    pLocalStream.Position = 0;
            }
            catch (Exception ex)
            {
                MessageBox.Show("Save failed with exception:\n\r" + ex.Message);
                return false;
            }
            //file saved on disk
            if (pDestStream == null)
            {
                this.Text = Path.GetFileNameWithoutExtension(strPath);
                g_bFileModified = false;
            }

            return true;
        }


        private void saveAsToolStripMenuItem_Click(object sender, EventArgs e)
        {
            SaveFileDialog sfd = new SaveFileDialog();
            //fisier binar
            sfd.Filter = "BreacherSquad Area (*.area)|*.area|All Files (*.*)|*.*";
            if (sfd.ShowDialog() == DialogResult.Cancel)
                return;

            SaveLevel_V2(sfd.FileName);
            //save absolute file path
            g_strFilePath = sfd.FileName;
        }

        private void but_setStart_Click(object sender, EventArgs e)
        {
        }

        public void SwapObjectsInList(ArrayList list, int idxFrom, int idxTo)
        {
            if (idxFrom == idxTo)
                return;
            if ((idxFrom < 0) || (idxTo < 0))
                return;
            if ((idxTo > list.Count - 1) || (list.Count < 2))
                return;

            Object temp = list[idxFrom];
            list[idxFrom] = list[idxTo];
            list[idxTo] = temp;
        }

        public void MoveObjectInList_ToBeginning(ArrayList list, int nObjIdx)
        {
            if ((nObjIdx < 0) || (nObjIdx > list.Count - 1) || (list.Count < 2))
                return;

            Object temp = list[nObjIdx];
            list.RemoveAt(nObjIdx);
            list.Insert(0, temp);
        }

        public void MoveObjectInList_ToEnd(ArrayList list, int nObjIdx)
        {
            if ((nObjIdx < 0) || (nObjIdx > list.Count - 1) || (list.Count < 2))
                return;

            Object temp = list[nObjIdx];
            list.RemoveAt(nObjIdx);
            list.Add(temp);
        }


        //override la toate
        protected override bool ProcessDialogKey(Keys keyData)
        {
            //OnKeyDown(new KeyEventArgs(keyData));  //da mesajele de 2 ori daca o chem
            return base.ProcessDialogKey(keyData);
        }

        //repaints map and marks file as modified
        void RepaintAfterChange()
        {
            PaintMap(g_lastMousePos.X, g_lastMousePos.Y);
            SetFileModified();
        }

        protected override void OnKeyDown(KeyEventArgs keyEvent)
        {
            //in cazul obiectelor misc trimit toate keypresses
            if ((g_brushMode == BRUSH_MODE_MISC) && (g_selectedMisc != null))
            {
                g_selectedMisc.OnKeypress(keyEvent);
                RepaintAfterChange();
            }

            // Ctrl+S Save
            if (keyEvent.Control && keyEvent.KeyCode == Keys.S)       
            {
                // Do what you want here
                keyEvent.SuppressKeyPress = true;  // Stops other controls on the form receiving event.
                saveToolStripMenuItem_Click(this, new EventArgs());
                return;
            }

            // Ctrl+Z Undo
            if (keyEvent.Control && keyEvent.KeyCode == Keys.Z)
            {
                keyEvent.SuppressKeyPress = true;  // Stops other controls on the form receiving event.
                undoToolStripMenuItem_Click(this, new EventArgs());
                return;
            }


            switch (keyEvent.KeyCode)
            {
                case Keys.PageUp:
                    {
                        if (keyEvent.Control)
                        {
                            if (g_brushMode == BRUSH_MODE_TILES)
                            {
                                if (g_selectedLayer < (int)ELayer.LAYERS_CNT - 1)
                                    MoveSelectionToLayer(g_SelectedAreaTL, g_selectedLayer + 1);
                                RepaintAfterChange();
                            }
                            else if (g_brushMode == BRUSH_MODE_OBJECTS)
                            {
                                if (g_selectedObject != null)
                                {
                                    if (g_selectedObject.layer < (int)ELayer.LAYERS_CNT - 1)
                                        g_selectedObject.layer++;
                                    RepaintAfterChange();
                                }
                            }
                        }
                        else
                        {
                            if (g_brushMode == BRUSH_MODE_OBJECTS)
                            {
                                //mut obiectul in sus in lista (spre sfarsit)
                                if (g_selectedObject != null)
                                {
                                    int idx = arrObjects.IndexOf(g_selectedObject);
                                    if (idx < arrObjects.Count - 1)
                                    {
                                        arrObjects[idx] = arrObjects[idx + 1];
                                        arrObjects[idx + 1] = g_selectedObject;
                                    }
                                }

                                RepaintAfterChange();
                            }
                        }
                    }
                    break;
                case Keys.PageDown:
                    {
                        if (keyEvent.Control)
                        {
                            if (g_brushMode == BRUSH_MODE_TILES)
                            {
                                if (g_selectedLayer > 0)
                                    MoveSelectionToLayer(g_SelectedAreaTL, g_selectedLayer - 1);
                                RepaintAfterChange();
                            }
                            else if (g_brushMode == BRUSH_MODE_OBJECTS)
                            {
                                if (g_selectedObject != null)
                                {
                                    if (g_selectedObject.layer > 0)
                                        g_selectedObject.layer--;
                                    RepaintAfterChange();
                                }
                            }
                        }
                        else
                        {
                            if (g_brushMode == BRUSH_MODE_OBJECTS)
                            {
                                if (g_selectedObject != null)
                                {
                                    int idx = arrObjects.IndexOf(g_selectedObject);
                                    if (idx > 0)
                                    {
                                        arrObjects[idx] = arrObjects[idx - 1];
                                        arrObjects[idx - 1] = g_selectedObject;
                                    }
                                }
                                RepaintAfterChange();
                            }
                        }
                    }
                    break;
                case Keys.Escape:
                    {
                    }
                    break;
                case Keys.V:
                    {
                        if (keyEvent.Control)
                        {
                            if (g_brushMode == BRUSH_MODE_TILES)
                            {
                                if (keyEvent.Shift)
                                    CopyPasteSelection(g_SelectedAreaTL, g_hoveredTile, true);
                                else
                                    CopyPasteSelection(g_SelectedAreaTL, g_hoveredTile, false);
                                RepaintAfterChange();
                            }
                            else if (g_brushMode == BRUSH_MODE_LIGHTS)
                            {
                                if (g_selectedLight != null)
                                {
                                    CLight nLight = new CLight(g_selectedLight);
                                    nLight.ID = GetUniqueID();
                                    //move light a little to the right
                                    nLight.pos.X += 20.0f;
                                    nLight.area.X += 20.0f;

                                    arrLights.Add(nLight);

                                    g_selectedLight = nLight;
                                    g_wndLights.SetSelectedLight(g_selectedLight);

                                    RepaintAfterChange();
                                }
                            }
                            else if (g_brushMode == BRUSH_MODE_COLLISIONS)
                            {
                                if (g_selectedCollision != null)
                                {
                                    CCollisionElement nColl = new CCollisionElement(g_selectedCollision);
                                    nColl.ID = GetUniqueID();
                                    //move light a little to the right
                                    nColl.rect.X += 20.0f;
                                    nColl.rect.Y += 20.0f;

                                    arrCollisions.Add(nColl);

                                    g_selectedCollision = nColl;
                                    g_wndCollisions.SetSelectedCollision(g_selectedCollision);

                                    RepaintAfterChange();
                                }
                            }
                        }
                    }
                    break;

                case Keys.D1:
                case Keys.NumPad1:
                    {
                        if (keyEvent.Control)
                        {
                            if (g_brushMode == BRUSH_MODE_TILES)
                            {
                                MoveSelectionToLayer(g_SelectedAreaTL, 0);
                                RepaintAfterChange();
                            }
                            else if (g_brushMode == BRUSH_MODE_OBJECTS)
                            {
                                if (g_selectedObject != null)
                                {
                                    g_selectedObject.layer = 0;
                                    RepaintAfterChange();
                                }
                            }
                        }
                        else
                        {
                            radio_layer0.Checked = true;
                        }
                    }
                    break;
                case Keys.D2:
                case Keys.NumPad2:
                    {
                        if (keyEvent.Control)
                        {
                            if (g_brushMode == BRUSH_MODE_TILES)
                            {
                                MoveSelectionToLayer(g_SelectedAreaTL, 1);
                                RepaintAfterChange();
                            }
                            else if (g_brushMode == BRUSH_MODE_OBJECTS)
                            {
                                if (g_selectedObject != null)
                                {
                                    g_selectedObject.layer = 1;
                                    RepaintAfterChange();
                                }
                            }
                        }
                        else
                        {
                            radio_layer1.Checked = true;
                        }
                    }
                    break;
                case Keys.D3:
                case Keys.NumPad3:
                    {
                        if (keyEvent.Control)
                        {
                            if (g_brushMode == BRUSH_MODE_TILES)
                            {
                                MoveSelectionToLayer(g_SelectedAreaTL, 2);
                                RepaintAfterChange();
                            }
                            else if (g_brushMode == BRUSH_MODE_OBJECTS)
                            {
                                if (g_selectedObject != null)
                                {
                                    g_selectedObject.layer = 2;
                                    RepaintAfterChange();
                                }
                            }
                        }
                        else
                        {
                            radio_layer2.Checked = true;
                        }
                    }
                    break;
                case Keys.D4:
                case Keys.NumPad4:
                    {
                        if (keyEvent.Control)
                        {
                            if (g_brushMode == BRUSH_MODE_TILES)
                            {
                                MoveSelectionToLayer(g_SelectedAreaTL, 3);
                                RepaintAfterChange();
                            }
                            else if (g_brushMode == BRUSH_MODE_OBJECTS)
                            {
                                if (g_selectedObject != null)
                                {
                                    g_selectedObject.layer = 3;
                                    RepaintAfterChange();
                                }
                            }
                        }
                        else
                        {
                            radio_layer3.Checked = true;
                        }
                    }
                    break;
                case Keys.D5:
                case Keys.NumPad5:
                    {
                        if (keyEvent.Control)
                        {
                            if (g_brushMode == BRUSH_MODE_TILES)
                            {
                                MoveSelectionToLayer(g_SelectedAreaTL, 4);
                                RepaintAfterChange();
                            }
                            else if (g_brushMode == BRUSH_MODE_OBJECTS)
                            {
                                if (g_selectedObject != null)
                                {
                                    g_selectedObject.layer = 4;
                                    RepaintAfterChange();
                                }
                            }
                        }
                        else
                        {
                            radio_layer4.Checked = true;
                        }
                    }
                    break;
                case Keys.D6:
                case Keys.NumPad6:
                    {
                        if (keyEvent.Control)
                        {
                            if (g_brushMode == BRUSH_MODE_TILES)
                            {
                                MoveSelectionToLayer(g_SelectedAreaTL, 5);
                                RepaintAfterChange();
                            }
                            else if (g_brushMode == BRUSH_MODE_OBJECTS)
                            {
                                if (g_selectedObject != null)
                                {
                                    g_selectedObject.layer = 5;
                                    RepaintAfterChange();
                                }
                            }
                        }
                        else
                        {
                            radio_layer5.Checked = true;
                        }
                    }
                    break;
                case Keys.D7:
                case Keys.NumPad7:
                    {
                        if (keyEvent.Control)
                        {
                            if (g_brushMode == BRUSH_MODE_TILES)
                            {
                                MoveSelectionToLayer(g_SelectedAreaTL, 6);
                                RepaintAfterChange();
                            }
                            else if (g_brushMode == BRUSH_MODE_OBJECTS)
                            {
                                if (g_selectedObject != null)
                                {
                                    g_selectedObject.layer = 6;
                                    RepaintAfterChange();
                                }
                            }
                        }
                        else
                        {
                            radio_layer6.Checked = true;
                        }
                    }
                    break;

                case Keys.F:
                    {
                        if (keyEvent.Control)
                        {
                            if (g_brushMode == BRUSH_MODE_TILES)
                            {
                                FillSelectionWithTileBrush(g_SelectedAreaTL);
                                RepaintAfterChange();
                            }
                        }
                    }
                    break;
                case Keys.S:
                    {
                        if (g_selectingStatus == K_SEL_STATUS_EMPTY)
                        {
                            g_selectingStatus = K_SEL_STATUS_START;
                        }
                        RepaintAfterChange();
                    }
                    break;
                case Keys.G:
                    {
                        frontLayerGridToolStripMenuItem.Checked = !frontLayerGridToolStripMenuItem.Checked;
                        RepaintAfterChange();
                    }
                    break;
                case Keys.I:
                    {
                        invertBackground = !invertBackground;
                        invertBackgroundToolStripMenuItem.Checked = invertBackground;
                        RepaintAfterChange();
                    }
                    break;
                case Keys.Home:
                    {
                        zoomLevel = 2.0f;
                        cameraPos.X = CAMERA_ORIGIN.X - (pictureBox1.Width / 2) * (1.0f / zoomLevel);
                        cameraPos.Y = CAMERA_ORIGIN.Y - (pictureBox1.Height / 2) * (1.0f / zoomLevel);
                        RepaintAfterChange();
                    }
                    break;
                case Keys.Delete:
                    {
                        if(g_brushMode == BRUSH_MODE_LIGHTS)
                        {
                            if(g_selectedLight != null)
                            {
                                RemoveTargetFromAllChildren((int)g_selectedLight.ID);

                                arrLights.Remove(g_selectedLight);
                                g_selectedLight = null;
                                g_wndLights.SetSelectedLight(g_selectedLight);
                                RepaintAfterChange();
                            }
                        }
                        if (g_brushMode == BRUSH_MODE_COLLISIONS)
                        {
                            if (g_selectedCollision != null)
                            {
                                RemoveTargetFromAllChildren((int)g_selectedCollision.ID);

                                arrCollisions.Remove(g_selectedCollision);
                                g_selectedCollision = null;
                                g_wndCollisions.SetSelectedCollision(g_selectedCollision);
                                RepaintAfterChange();
                            }
                        }
                        if (g_brushMode == BRUSH_MODE_OBJECTS)
                        {
                            if (g_selectedObject != null)
                            {
                                int nLinkedCollisions = GetCollisionsWithTargetIDCount((int)g_selectedObject.ID);
                                if (nLinkedCollisions > 0)
                                {
                                    DialogResult dr = MessageBox.Show("There are collision elements that point to this object.\n\rDo you want to delete those too?", "Info", MessageBoxButtons.YesNoCancel);
                                    if (dr == DialogResult.Yes)
                                    {
                                        DeleteCollisionElementsWithTargetID((int)g_selectedObject.ID);
                                    }
                                    else if (dr == DialogResult.Cancel)
                                    {
                                        break;
                                    }
                                }

                                RemoveTargetFromAllChildren((int)g_selectedObject.ID);

                                arrObjects.Remove(g_selectedObject);
                                g_selectedObject = null;
                                g_wndObjects.SetSelectedObject(null);

                                RepaintAfterChange();
                            }
                        }
                        if (g_brushMode == BRUSH_MODE_ACTORS)
                        {
                            if (g_selectedActor != null)
                            {
                                RemoveTargetFromAllChildren((int)g_selectedActor.ID);

                                arrActors.Remove(g_selectedActor);
                                g_selectedActor = null;
                                g_wndActors.SetSelectedActor(null);
                                RepaintAfterChange();
                            }
                        }
                        if (g_brushMode == BRUSH_MODE_TILES)
                        {
                            if (keyEvent.Shift)
                                DeleteFromSelection(g_SelectedAreaTL, true);
                            else
                                DeleteFromSelection(g_SelectedAreaTL, false);
                            RepaintAfterChange();
                        }
                        if (g_brushMode == BRUSH_MODE_MISC)
                        {
                            if (g_selectedMisc == null)
                                break;
                            switch (g_selectedMisc.type)
                            {
                                case K_MISC_FRONTLAYEROBJ:
                                case K_MISC_SCRIPT:
                                case K_MISC_BACKGROUND:
                                    {
                                        RemoveTargetFromAllChildren((int)g_selectedMisc.ID);

                                        arrMisc.Remove(g_selectedMisc);
                                        g_selectedMisc = null;
                                        g_wndMisc.SetSelectedMisc(null);

                                        RepaintAfterChange();
                                    }
                                    break;
                                case K_MISC_RAIL:
                                    //daca am sters toate nodurile sterg railul
                                    CMiscObject_Rail rail = g_selectedMisc as CMiscObject_Rail;
                                    if (rail.listPoints.Count <= 0)
                                    {
                                        RemoveTargetFromAllChildren((int)g_selectedMisc.ID);

                                        arrMisc.Remove(g_selectedMisc);
                                        g_selectedMisc = null;
                                        g_wndMisc.SetSelectedMisc(null);

                                        RepaintAfterChange();

                                        MessageBox.Show("Rail deleted!");
                                    }
                                    break;
                            }
                        }
                    }
                    break;
                case Keys.Left:
                    {
                        if (g_brushMode == BRUSH_MODE_OBJECTS)
                        {
                            if (g_selectedObject != null)
                            {
                                g_selectedObject.pos.X -= 1;
                                RepaintAfterChange();
                            }
                        }
                        else if ((g_brushMode == BRUSH_MODE_COLLISIONS) && (g_selectedCollision != null))
                        {
                            if (keyEvent.Control)
                            {
                                g_selectedCollision.rect.X -= 1.0f;
                                RepaintAfterChange();
                            }
                            else if (keyEvent.Alt)
                            {
                                g_selectedCollision.rect.Width -= 1.0f;
                                RepaintAfterChange();
                            }
                        }
                        else if (g_brushMode == BRUSH_MODE_LIGHTS)
                        {
                            if ((g_selectedLight != null) && (keyEvent.Control))
                            {
                                g_selectedLight.pos.X -= 1;
                                g_selectedLight.area.X -= 1;
                                RepaintAfterChange();
                            }
                        }
                    }
                    break;
                case Keys.Right:
                    {
                        if (g_brushMode == BRUSH_MODE_OBJECTS)
                        {
                            if (g_selectedObject != null)
                            {
                                g_selectedObject.pos.X += 1;
                                RepaintAfterChange();
                            }
                        }
                        else if ((g_brushMode == BRUSH_MODE_COLLISIONS) && (g_selectedCollision != null))
                        {
                            if (keyEvent.Control)
                            {
                                g_selectedCollision.rect.X += 1.0f;
                                RepaintAfterChange();
                            }
                            else if (keyEvent.Alt)
                            {
                                g_selectedCollision.rect.Width += 1.0f;
                                RepaintAfterChange();
                            }
                        }
                        else if (g_brushMode == BRUSH_MODE_LIGHTS)
                        {
                            if ((g_selectedLight != null) && (keyEvent.Control))
                            {
                                g_selectedLight.pos.X += 1;
                                g_selectedLight.area.X += 1;
                                RepaintAfterChange();
                            }
                        }
                    }
                    break;
                case Keys.Up:
                    {
                        if (g_brushMode == BRUSH_MODE_OBJECTS)
                        {
                            if (g_selectedObject != null)
                            {
                                g_selectedObject.pos.Y -= 1;
                                RepaintAfterChange();
                            }
                        }
                        else if ((g_brushMode == BRUSH_MODE_COLLISIONS) && (g_selectedCollision != null))
                        {
                            if (keyEvent.Control)
                            {
                                g_selectedCollision.rect.Y -= 1.0f;
                                RepaintAfterChange();
                            }
                            else if (keyEvent.Alt)
                            {
                                g_selectedCollision.rect.Height -= 1.0f;
                                RepaintAfterChange();
                            }
                        }
                        else if (g_brushMode == BRUSH_MODE_LIGHTS)
                        {
                            if ((g_selectedLight != null) && (keyEvent.Control))
                            {
                                g_selectedLight.pos.Y -= 1;
                                g_selectedLight.area.Y -= 1;
                                RepaintAfterChange();
                            }
                        }
                    }
                    break;
                case Keys.Down:
                    {
                        if (g_brushMode == BRUSH_MODE_OBJECTS)
                        {
                            if (g_selectedObject != null)
                            {
                                g_selectedObject.pos.Y += 1;
                                RepaintAfterChange();
                            }
                        }
                        else if ((g_brushMode == BRUSH_MODE_COLLISIONS) && (g_selectedCollision != null))
                        {
                            if (keyEvent.Control)
                            {
                                g_selectedCollision.rect.Y += 1.0f;
                                RepaintAfterChange();
                            }
                            else if (keyEvent.Alt)
                            {
                                g_selectedCollision.rect.Height += 1.0f;
                                RepaintAfterChange();
                            }
                        }
                        else if (g_brushMode == BRUSH_MODE_LIGHTS)
                        {
                            if ((g_selectedLight != null) && (keyEvent.Control))
                            {
                                g_selectedLight.pos.Y += 1;
                                g_selectedLight.area.Y += 1;
                                RepaintAfterChange();
                            }
                        }
                    }
                    break;
                case Keys.Add:
                    {
                    }
                    break;
                case Keys.Subtract:
                    {
                    }
                    break;
                case Keys.C:
                    {
                        if (g_brushMode == BRUSH_MODE_OBJECTS)
                        {
                            if (g_selectedObject != null)
                                g_wndObjects.SetIsCover((g_selectedObject.flags & OBJFLAG_IS_COVER) == 0);
                        }
                    }
                    break;
                case Keys.X:
                    {
                        if (g_brushMode == BRUSH_MODE_OBJECTS)
                        {
                            if (g_selectedObject != null)
                                g_selectedObject.flags ^= OBJFLAG_FLIPX;
                            RepaintAfterChange();
                        }
                        if (g_brushMode == BRUSH_MODE_ACTORS)
                        {
                            if (g_selectedActor != null)
                                g_selectedActor.bLookLeft = !g_selectedActor.bLookLeft;

                            g_wndActors.SetSelectedActor(g_selectedActor);
                            RepaintAfterChange();
                        }
                    }
                    break;
                case Keys.Y:
                    {
                        if (g_brushMode == BRUSH_MODE_OBJECTS)
                        {
                            if (g_selectedObject != null)
                                g_selectedObject.flags ^= OBJFLAG_FLIPY;
                            RepaintAfterChange();
                        }
                    }
                    break;
            }
        }

        //mouse wheel
        private void pictureBox1_MouseWheel(object sender, MouseEventArgs e)
        {
            float offoldx = pictureBox1.Width * (1.0f / zoomLevel);
            float offoldy = pictureBox1.Height * (1.0f / zoomLevel);

            if (e.Delta < 0)
            {
                zoomLevel -= 0.1f;
                if (zoomLevel <= 0.5f)
                    zoomLevel = 0.5f;
            }
            else if (e.Delta > 0)
            {

                zoomLevel += 0.1f;
                if (zoomLevel >= 5.0f)
                    zoomLevel = 5.0f;
            }

            float offnewx = pictureBox1.Width * (1.0f / zoomLevel);
            float offnewy = pictureBox1.Height * (1.0f / zoomLevel);
            //zoom on screen center
            cameraPos.X -= (offnewx - offoldx) / 2.0f;
            cameraPos.Y -= (offnewy - offoldy) / 2.0f;

            PaintMap();
        }

        public bool LoadLevel_V2(string strPath, bool bLoadPrefab = false, int nPrefabTileX = 0, int nPrefabTileY = 0, Stream pSrcStream = null)
        {
            //get base levels folder
            string strBaseFolder = strPath;

            ArrayList arrIDs = new ArrayList();
            UInt32 dwStartID = 0;
            if (bLoadPrefab)
                dwStartID = GetUniqueID();
            // reset it here
            g_nLastID = 0;

            try
            {
                strBaseFolder = Path.GetDirectoryName(strPath);
                if (strBaseFolder.Length > 0) //remove another folder child 
                    strBaseFolder = Path.GetDirectoryName(strBaseFolder);

                Stream pLocalStream = null;
                if (pSrcStream == null)
                {
                    FileStream fs = new FileStream(strPath, FileMode.Open);
                    pLocalStream = fs;
                }
                else
                {
                    pLocalStream = pSrcStream;
                }

                BinaryReader bw = new BinaryReader(pLocalStream);

                if ((pLocalStream == null) || (bw == null))
                    return false;

                int nLayersCnt = (int)ELayer.LAYERS_CNT;

                Byte ub;
                Int16 s2b;
                UInt16 u2b;
                //0. read first 10 integers
                Int32[] arrInts = new Int32[10];
                for (int kk = 0; kk < 10; kk++)
                {
                    arrInts[kk] = bw.ReadInt32();
                }
                //version check
                if (arrInts[0] != K_CURRENT_VERSION)
                {
                    if (arrInts[0] == 1013) //versiunea cu 2 layere de tiles (in loc de 3 adaugat in 1014)
                    {
                        nLayersCnt = 2;
                        MessageBox.Show("Loading from older format with only 2 layers! All objects and tiles will go the the Back and Front layers! Check all objects and layers again!");
                    }
                    else if (arrInts[0] == 1014) //versiune cu IES lights
                    {
                    }
                    else
                    {
                        MessageBox.Show("Level failed to load! Unhandled version of file found: " + arrInts[0]);
                        pLocalStream.Close();
                        return false;
                    }
                }

                //1. tipul misiunii
                ub = bw.ReadByte();
                if (!bLoadPrefab)
                    g_missionType = ub;
                //scrie numele tilesetului si date despre tileset
                String tilesetName = bw.ReadString();
                int a, b;
                a = bw.ReadByte(); //tilew
                b = bw.ReadByte(); //tileH
                int nLvlTilesetColumns = bw.ReadUInt16(); //tileset columns
                if (!bLoadPrefab)
                {
                    g_wndMaterials.LoadTileset(strBaseFolder + "\\data\\" + tilesetName); //seteaza singur toate chestiile legate de tileset
                }

                //marime exacta nivel in tiles
                UInt16 levelw = bw.ReadUInt16();
                UInt16 levelh = bw.ReadUInt16();
                //level origin
                Point vLocalLvlOrigin = new Point();
                vLocalLvlOrigin.Y = bw.ReadInt16();
                vLocalLvlOrigin.X = bw.ReadInt16();

                Point LOCAL_OFFSET = LEVEL_OFFSET;
                if (!bLoadPrefab)
                {
                    gLevelOrigin.Y = vLocalLvlOrigin.Y;
                    gLevelOrigin.X = vLocalLvlOrigin.X;

                    LOCAL_OFFSET = LEVEL_OFFSET;
                    LOCAL_OFFSET.Y -= gLevelOrigin.Y / TILE_HEIGHT;
                    gLevelOrigin.Y = LEVEL_OFFSET.Y * TILE_HEIGHT;
                    LOCAL_OFFSET.X -= gLevelOrigin.X / TILE_WIDTH;
                    gLevelOrigin.X = LEVEL_OFFSET.X * TILE_WIDTH;
                }
                else
                {
                    //prefab TileX and Y are in local level coords
                    LOCAL_OFFSET.X = nPrefabTileX - vLocalLvlOrigin.X / TILE_WIDTH;
                    LOCAL_OFFSET.Y = nPrefabTileY - vLocalLvlOrigin.Y / TILE_HEIGHT;
                }

                //tile-urile pe rand, toata matricea
                for (int yy = 0; yy < levelh; yy++)
                {
                    for (int xx = 0; xx < levelw; xx++)
                    {
                        for (int kk = 0; kk < nLayersCnt; kk++)
                        {
                            Int32 lev = bw.ReadInt32();
                            if (lev < 0)
                                lev = -1;
                            gMap.setTile(LOCAL_OFFSET.X + xx, LOCAL_OFFSET.Y + yy, lev, kk);
                        }
                    }
                }

                //auto adjust if we change the tileset resolution
                if ((!bLoadPrefab) && (nLvlTilesetColumns != TILESET_COLUMNS))
                    SetMaterialData(TILE_WIDTH, TILE_HEIGHT, nLvlTilesetColumns, TILESET_COLUMNS);
                //build images for all blocks
                BuildAllBlockImages();

                ///--- lights luminile ---
                String bsxName = bw.ReadString();
                if (!bLoadPrefab)
                {
                    arrLights.RemoveRange(0, arrLights.Count);
                    g_wndLights.LoadBSX(strBaseFolder + "\\data\\" + bsxName); //seteaza singur toate chestiile legate de tileset
                }

                UInt32 nrlights = bw.ReadUInt32();
                //date fiecare 
                for (int kk = 0; kk < nrlights; kk++)
                {
                    CLight light = new CLight();

                    light.ID = bw.ReadUInt32() + dwStartID;

                    if(arrIDs.Contains(light.ID))
                    {
                        MessageBox.Show("Light ID conflict: " + light.ID, "Warning!");
                    }
                    arrIDs.Add(light.ID);

                    if (g_nLastID < light.ID)
                        g_nLastID = light.ID;

                    light.type = bw.ReadByte();
                    light.nAtmoAttenuationPerc = (int)bw.ReadUInt32();
                    light.fIntensity = bw.ReadSingle();
                    PointF lpos = new PointF();
                    lpos.X = bw.ReadInt32() + LOCAL_OFFSET.X * TILE_WIDTH;
                    lpos.Y = bw.ReadInt32() + LOCAL_OFFSET.Y * TILE_HEIGHT;
                    light.posZ = bw.ReadInt32();
                    //read anim name 
                    string strLightAnm = bw.ReadString();
                    //e ok sa ramana setata pe -1 daca nu avem animatie la lumina (poate fi ambientala)
                    light.animId = g_sprLights.GetAnimIdxByName(strLightAnm);

                    light.color = Color.FromArgb(bw.ReadByte(), bw.ReadByte(), bw.ReadByte(), bw.ReadByte());
                    light.pos = lpos;

                    light.area.X = bw.ReadInt32() + LOCAL_OFFSET.X * TILE_WIDTH;
                    light.area.Y = bw.ReadInt32() + LOCAL_OFFSET.Y * TILE_WIDTH;
                    light.area.Width = bw.ReadInt32();
                    light.area.Height = bw.ReadInt32();

                    //restul de date
                    light.angle = bw.ReadInt16();
                    //shadows
                    u2b = bw.ReadUInt16();
                    light.castsShadows = ((u2b & K_LIGHT_FLAG_CAST_SHADOWS) != 0);

                    //logic
                    light.logic.Load(bw, (int)dwStartID);

                    //--- converts loaded IES lights into normal ones ---
                    if (light.type == K_LIGHT_IES_REALISTIC_OBSOLETE)
                    {
                        light.type = K_LIGHT_POINT;
                        light.animId = 0; //circular light
                        MessageBox.Show("Converted light from type IES to point light. ID:" + light.ID);
                    }
                    //when loading from older version replace subtype with atmospheric attenuation
                    //we used the UINT32 of subtype to save the attenuation (backwards compatibility)
                    if (arrInts[0] == 1014)
                    {
                        if (light.type == K_LIGHT_POINT)
                            light.nAtmoAttenuationPerc = 0;
                        else
                            light.nAtmoAttenuationPerc = 100;
                    }

                    //lights images changed?. re-center
                    if (((light.type == K_LIGHT_POINT) || (light.type == K_LIGHT_AREA)) && (light.animId >= 0))
                    {
                        //folosim primul frame, adica cel al spotului.
                        RectangleF rect = g_sprLights.anims[light.animId].aframes[0].frame.GetRect();

                        float scaleX = light.area.Width / rect.Width;
                        float scaleY = light.area.Height / rect.Height;
                        rect.X *= scaleX; rect.Y *= scaleY;
                        rect.Width *= scaleX; rect.Height *= scaleY;

                        light.area = rect;
                        light.area.X += light.pos.X;
                        light.area.Y += light.pos.Y;
                    }

                    arrLights.Add(light);
                }

                ///--- formele de coliziune ---
                if (!bLoadPrefab)
                {
                    arrCollisions.RemoveRange(0, arrCollisions.Count);
                }    
                UInt32 lCount = bw.ReadUInt32();
                //date fiecare element
                for (int kk = 0; kk < lCount; kk++)
                {
                    Rectangle crect = new Rectangle();
                    //ID
                    UInt32 nID = bw.ReadUInt32() + dwStartID;

                    if (arrIDs.Contains(nID))
                    {
                        MessageBox.Show("Collision ID conflict: " + nID, "Warning!");
                    }
                    arrIDs.Add(nID);

                    if (g_nLastID < nID)
                        g_nLastID = nID;
                    //collision rect
                    crect.X = (int)(bw.ReadInt32() + LOCAL_OFFSET.X * TILE_WIDTH);
                    crect.Y = (int)(bw.ReadInt32() + LOCAL_OFFSET.Y * TILE_HEIGHT);
                    crect.Width = (int)bw.ReadUInt32();
                    if (crect.Width <= 0)
                        crect.Width = TILE_WIDTH;
                    crect.Height = (int)bw.ReadUInt32();
                    if (crect.Height <= 0)
                        crect.Height = TILE_HEIGHT;
                    //type (ub)
                    byte tip = bw.ReadByte();
                    //cast shadows
                    bool castSh = false;
                    ub = bw.ReadByte();
                    if (ub != 0)
                        castSh = true;

                    CCollisionElement col = new CCollisionElement();
                    col.ID = nID;
                    col.rect = crect;
                    col.castShadows = castSh;
                    col.type = (int)tip;
                    col.castShadows = castSh;

                    //logic
                    col.logic.Load(bw, (int)dwStartID);

                    arrCollisions.Add(col);
                }

                //decoratiunile
                bsxName = bw.ReadString();
                if (!bLoadPrefab)
                {
                    arrObjects.RemoveRange(0, arrObjects.Count);
                    g_wndObjects.LoadBSX(strBaseFolder + "\\data\\" + bsxName); //seteaza singur toate chestiile legate de tileset
                }

                //save anim and frame numbers for later
                int nAnimsCnt = g_wndObjects.GetSpriteLoader().anims.Count;
                //nr deco
                UInt32 objcnt = (UInt32)bw.ReadUInt32();
                //date fiecare deco
                for (int kk = 0; kk < objcnt; kk++)
                {
                    CObject obj = new CObject();

                    obj.ID = bw.ReadUInt32() + dwStartID;
                    if (arrIDs.Contains(obj.ID))
                    {
                        MessageBox.Show("Object ID conflict: " + obj.ID, "Warning!");
                    }
                    arrIDs.Add(obj.ID);

                    if (g_nLastID < obj.ID)
                        g_nLastID = obj.ID;

                    obj.layer = bw.ReadByte();
                    //move from old format to new format
                    if (nLayersCnt == 2)
                        obj.layer += 1; //0,1 becomes 1,2 (mid and front)
                    //pozitia
                    obj.pos.X = (int)(bw.ReadInt32() + LOCAL_OFFSET.X * TILE_WIDTH);
                    obj.pos.Y = (int)(bw.ReadInt32() + LOCAL_OFFSET.Y * TILE_HEIGHT);
                    //anim
                    string strObjAnm = bw.ReadString();
                    obj.animIdx = g_sprObjects.GetAnimIdxByName(strObjAnm);
                    if (obj.animIdx < 0)
                    {
                        obj.animIdx = 0;
                        MessageBox.Show("Object ID:" + obj.ID + " animation not found:[" + strObjAnm + "]. Animation was reset to the first animation in the file!");
                    }
                    //read frame number
                    obj.frameIdx = bw.ReadUInt16();
                    //check anim and frame (or reset on 0)
                    if (obj.animIdx >= nAnimsCnt)
                    {
                        obj.animIdx = -1;
                        MessageBox.Show("Wrong animation on Object ID:" + obj.ID + "\r\nResetting it to -1", "Warning!");
                    }
                    if (obj.frameIdx >= g_sprObjects.anims[obj.animIdx].aframes.Count)
                    {
                        obj.frameIdx = -1;
                        MessageBox.Show("Wrong frameIdx on Object ID:" + obj.ID + "\r\nResetting it to -1", "Warning!");
                    }
                    //flags
                    obj.flags = bw.ReadUInt32();

                    //logic
                    obj.logic.Load(bw, (int)dwStartID);

                    //converteste usile de model vechi in usi de model nou (breach doar cu melee)
                    if ((obj.logic.strActions == "ACTIVE_OPEN_DOOR_NO_CLOSE") && (obj.logic.nInteractTimer > 0))
                    {
                        obj.logic.strActions = "ACTIVE_LOCKED_BREAKABLE";
                        obj.logic.nInteractTimer = 0;

                        MessageBox.Show("Updated locked door ID " + obj.ID);
                    }

                    arrObjects.Add(obj);
                }

                //reading actors bsx
                bsxName = bw.ReadString();
                if (!bLoadPrefab)
                {
                    arrActors.RemoveRange(0, arrActors.Count);
                    g_wndActors.LoadBSX(strBaseFolder + "\\data\\" + bsxName);
                }
                UInt32 actcnt = (UInt32)bw.ReadUInt32();
                //date fiecare actor
                for (int kk = 0; kk < actcnt; kk++)
                {
                    CActor act = new CActor();

                    act.ID = bw.ReadUInt32() + dwStartID;
                    if (arrIDs.Contains(act.ID))
                    {
                        MessageBox.Show("Actor ID conflict: " + act.ID, "Warning!");
                    }
                    arrIDs.Add(act.ID);

                    if (g_nLastID < act.ID)
                        g_nLastID = act.ID;

                    //pozitia
                    act.pos.X = (int)(bw.ReadInt32() + LOCAL_OFFSET.X * TILE_WIDTH);
                    act.pos.Y = (int)(bw.ReadInt32() + LOCAL_OFFSET.Y * TILE_HEIGHT);
                    //unghi si boolean SetAngle
                    act.bSetAngle = (bw.ReadByte() != 0) ? true : false;
                    act.fAngle = (float)bw.ReadInt16();
                    //template name
                    act.templateName = bw.ReadString();
                    //scrie nume starting AI state
                    act.strSelectedAIState = bw.ReadString();
                    //anim
                    act.bLookLeft = (bw.ReadByte() != 0) ? true : false;
                    //collision/gravity
                    act.bHasCollision = (bw.ReadByte() != 0) ? true : false;
                    act.bHasGravity = (bw.ReadByte() != 0) ? true : false;
                    //logic
                    act.logic.Load(bw, (int)dwStartID);

                    arrActors.Add(act);
                }

                //misc objects (rails samd)
                if(!bLoadPrefab)
                    arrMisc.RemoveRange(0, arrMisc.Count);

                UInt32 misccnt = bw.ReadUInt32();
                //date fiecare misc object
                for (int kk = 0; kk < misccnt; kk++)
                {
                    //datele generice:
                    //tipul
                    byte type = bw.ReadByte();
                    //datele generice
                    CMiscObjectBase mob = new CMiscObjectBase();
                    mob.ID = bw.ReadUInt32() + dwStartID;

                    if (arrIDs.Contains(mob.ID))
                    {
                        MessageBox.Show("Misc Object ID conflict: " + mob.ID, "Warning!");
                    }
                    arrIDs.Add(mob.ID);

                    if (g_nLastID < mob.ID)
                        g_nLastID = mob.ID;

                    byte u1b = bw.ReadByte(); //nr params
                    for (int i = 0; i < u1b; i++)
                    {
                        string paramname = bw.ReadString();
                        string paramval = bw.ReadString();

                        mob.listParams.Add(paramname);
                        mob.listParams.Add(paramval);
                    }
                    //datele particulare
                    switch (type)
                    {
                        case K_MISC_FRONTLAYEROBJ:
                            {
                                CMiscObject_FrontLayerObj back = new CMiscObject_FrontLayerObj();
                                back.ID = mob.ID;
                                back.listParams = mob.listParams;

                                PointF npos = new PointF();
                                npos.X = (float)bw.ReadInt32();
                                if (npos.X < 0) npos.X = 0; if (npos.X > levelw * TILE_WIDTH) npos.X = levelw * TILE_WIDTH;
                                npos.X += LOCAL_OFFSET.X * TILE_WIDTH;

                                npos.Y = (float)bw.ReadInt32();
                                if (npos.Y < 0) npos.Y = 0; if (npos.Y > levelh * TILE_HEIGHT) npos.Y = levelh * TILE_HEIGHT;
                                npos.Y += LOCAL_OFFSET.Y * TILE_HEIGHT;

                                back.pos = npos;

                                arrMisc.Add(back);
                            }
                            break;
                        case K_MISC_SCRIPT:
                            {
                                CMiscObject_Script scr = new CMiscObject_Script();
                                scr.ID = mob.ID;
                                scr.listParams = mob.listParams;

                                PointF npos = new PointF();
                                npos.X = (float)bw.ReadInt32();
                                if (npos.X < 0) npos.X = 0; if (npos.X > levelw * TILE_WIDTH) npos.X = levelw * TILE_WIDTH;
                                npos.X += LOCAL_OFFSET.X * TILE_WIDTH;

                                npos.Y = (float)bw.ReadInt32();
                                if (npos.Y < 0) npos.Y = 0; if (npos.Y > levelh * TILE_HEIGHT) npos.Y = levelh * TILE_HEIGHT;
                                npos.Y += LOCAL_OFFSET.Y * TILE_HEIGHT;


                                scr.pos = npos;

                                arrMisc.Add(scr);
                            }
                            break;
                        case K_MISC_BACKGROUND:
                            {
                                CMiscObject_Background back = new CMiscObject_Background();
                                back.ID = mob.ID;
                                back.listParams = mob.listParams;

                                PointF npos = new PointF();
                                npos.X = (float)bw.ReadInt32();
                                if (npos.X < 0) npos.X = 0; if (npos.X > levelw * TILE_WIDTH) npos.X = levelw * TILE_WIDTH;
                                npos.X += LOCAL_OFFSET.X * TILE_WIDTH;

                                npos.Y = (float)bw.ReadInt32();
                                if (npos.Y < 0) npos.Y = 0; if (npos.Y > levelh * TILE_HEIGHT) npos.Y = levelh * TILE_HEIGHT;
                                npos.Y += LOCAL_OFFSET.Y * TILE_HEIGHT;


                                back.pos = npos;

                                arrMisc.Add(back);
                            }
                            break;
                        case K_MISC_RAIL:
                            {
                                CMiscObject_Rail rail = new CMiscObject_Rail();
                                rail.ID = mob.ID;
                                rail.listParams = mob.listParams;
                                //nr de puncte
                                UInt16 ptscnt = bw.ReadUInt16();
                                //coordonate puncte
                                for (int i = 0; i < ptscnt; i++)
                                {
                                    PointF pos = new PointF();
                                    pos.X = (float)bw.ReadInt32();
                                    pos.X += LOCAL_OFFSET.X * TILE_WIDTH;

                                    pos.Y = (float)bw.ReadInt32();
                                    pos.Y += LOCAL_OFFSET.Y * TILE_HEIGHT;

                                    rail.listPoints.Add(pos);
                                }

                                //adaug in lista
                                arrMisc.Add(rail);
                            }
                            break;
                        default:
                            {
                                MessageBox.Show("Load:Misc object type not handled!");
                            }
                            break;
                    }

                }

                //close stream if reading from file
                if(pSrcStream == null)
                    pLocalStream.Dispose();

                //populate templates list in ActorsWnd
                if (!bLoadPrefab)
                {
                    g_wndActors.PopulateTemplatesList(strBaseFolder + "\\data\\actors_data.xml");

                    Undo_SetCurrent(K_UNDO_DISABLED);
                }

                //set actors animation indices
                for (int kk = 0; kk < arrActors.Count; kk++)
                {
                    CActor act = arrActors[kk] as CActor;
                    g_wndActors.SetActorAnimByTemplate(act);
                }

                //prefabs list
                if (!bLoadPrefab)
                {
                    g_wndPrefabs.SetPrefabsFolder(strBaseFolder + "\\prefabs\\");
                    //mark file as not modified
                    g_bFileModified = false;
                    //save file path if loaded from disk
                    if (pSrcStream == null)
                    {
                        g_strFilePath = strPath;
                        this.Text = Path.GetFileNameWithoutExtension(strPath);
                    }
                }
                else
                {
                    g_bFileModified = true;
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show("Level failed to load with exception:\n\r" + ex.Message);
                return false;
            }

            return true;
        }


        private void openToolStripMenuItem_Click(object sender, EventArgs e)
        {
            OpenFileDialog sfd = new OpenFileDialog();
            //fisier binar
            sfd.Filter = "BreacherSquad Area(*.area)|*.area|All Files (*.*)|*.*";
            if (sfd.ShowDialog() == DialogResult.Cancel)
                return;

            ResetLevel();

            bool bSuccess = LoadLevel_V2(sfd.FileName);

            PaintMap();

            if (bSuccess)
                MessageBox.Show("Level was loaded successfully!", "Success!");
        }

        public void ResetLevel(bool bLoadDefaultLevel = false)
        {
            g_strFilePath = "";
            g_bFileModified = false;

            zoomLevel = 2.0f;
            //se offseteaza ca sa nu ajungi in 0 niciodata
            LEVEL_OFFSET.X = 100 * BLOCK_W; LEVEL_OFFSET.Y = 100 * BLOCK_H;
            CAMERA_ORIGIN.X = LEVEL_OFFSET.X * TILE_WIDTH; CAMERA_ORIGIN.Y = LEVEL_OFFSET.Y * TILE_HEIGHT;

            cameraPos.X = CAMERA_ORIGIN.X - (pictureBox1.Width / 2) * (1.0f / zoomLevel);
            cameraPos.Y = CAMERA_ORIGIN.Y - (pictureBox1.Height / 2) * (1.0f / zoomLevel);

            gLevelOrigin.X = (int)CAMERA_ORIGIN.X;
            gLevelOrigin.Y = (int)CAMERA_ORIGIN.Y;

            gMap.ClearAllBlocks();

            arrObjects.RemoveRange(0, arrObjects.Count);
            arrCollisions.RemoveRange(0, arrCollisions.Count);
            arrLights.RemoveRange(0, arrLights.Count);
            arrMisc.RemoveRange(0, arrMisc.Count);
            arrActors.RemoveRange(0, arrActors.Count);

            g_selectedCollision = null;
            g_selectedLight = null;
            g_selectedObject = null;
            g_wndObjects.SetSelectedObject(g_selectedObject);
            g_selectedActor = null;
            g_wndActors.SetSelectedActor(null);

            g_nLastID = 0;

            this.Text = "New Level";

            if (bLoadDefaultLevel)
            {
                LoadLevel_V2("..\\media\\levels\\missions\\editor_new_area.area");
                //force file path on empty
                g_strFilePath = "";
            }
        }

        private void newToolStripMenuItem_Click(object sender, EventArgs e)
        {
            DialogResult res = MessageBox.Show(this, "This operation cannot be undone!\n\r\n\rDo you want to load the default Starter Level Kit? This will load all necessary assets to help you start making your own levels.", "Reset Level", MessageBoxButtons.YesNoCancel, MessageBoxIcon.Exclamation);
            if (res == System.Windows.Forms.DialogResult.Cancel)
                return;
            if (res == System.Windows.Forms.DialogResult.Yes)
                ResetLevel(true);
            else
                ResetLevel(false);

            PaintMap();
        }

        void moveMap(int x, int y)
        {
            /*
            ArrayList temp = new ArrayList();
            for (int ii = 0; ii < gMap.Blocks.Count; ii++)
            {
                CTileBlock tileBlock = gMap.Blocks[ii] as CTileBlock;
                for (int xx = 0; xx < BLOCK_W; xx++)
                    for (int yy = 0; yy < BLOCK_H; yy++)
                        if (tileBlock.tiles[xx, yy].level != 0) 
                        {
                            tileBlock.tiles[xx,yy].x = tileBlock.pos.X * BLOCK_W + xx + x;
                            tileBlock.tiles[xx,yy].y = tileBlock.pos.Y * BLOCK_H + yy + y;
                            temp.Add(tileBlock.tiles[xx, yy]);
                        }
            }

            gMap.Blocks.Clear();

            foreach (CTile tile in temp)
            {
                gMap.setTile(tile.x, tile.y, tile.level);
                CTile tempTile = gMap.getTile(tile.x, tile.y);
                tempTile.specialPiece = tile.specialPiece;
                tempTile.cannonDirFlags = tile.cannonDirFlags;
                tempTile.cannonRange = tile.cannonRange;
                tempTile.borderIdx = tile.borderIdx;
                tempTile.cornerIdx = tile.cornerIdx;
                tempTile.leeFactor = tile.leeFactor;
            }

            foreach (CDecoration deco in Decorations)
            {
                deco.pos.X += x * TILE_WIDTH;
                deco.pos.Y += y * TILE_HEIGHT;
            }

            if (startXtl >= 0 && startYtl >= 0)
            {
                startXtl += x;
                startYtl += y;
            }
            if (endXtl >= 0 && endXtl >= 0)
            {
                endXtl += x;
                endYtl += y;
            }
             */
        }

        private void HideToolWindows()
        {
            g_wndMaterials.Hide();
            g_wndAI.Hide();
            g_wndCollisions.Hide();
            g_wndLights.Hide();
            g_wndObjects.Hide();
            g_wndMisc.Hide();
            g_wndActors.Hide();
            g_wndPrefabs.Hide();

            g_selectedCollision = null;
            g_selectedLight = null;
            g_selectedObject = null;
            g_selectedMisc = null;
            g_selectedActor = null;

            g_wndAI.SetSelectedLogic(null);
            g_wndMisc.SetSelectedMisc(null);
            g_wndActors.SetSelectedActor(null);
            g_wndObjects.SetSelectedObject(null);
        }

        private void butWndMaterials_Click(object sender, EventArgs e)
        {
            HideToolWindows();

            g_wndMaterials.Show();
            g_wndMaterials.Left = this.Left + this.Width;
            g_wndMaterials.Top = this.Top;

            g_brushMode = BRUSH_MODE_TILES;

            UpdateStatusBarMessage();
            PaintMap();
        }

        private void invertBackgroundToolStripMenuItem_Click(object sender, EventArgs e)
        {
            invertBackground = invertBackgroundToolStripMenuItem.Checked;
            PaintMap();
        }

        /// <summary>
        /// Sets a message in the status bar
        /// </summary>
        /// <param name="strMessage">The message to be shown</param>
        private void SetStatusBarMessage(string strMessage)
        {
            toolStripStatusLabel1.Text = strMessage;
        }

        /// <summary>
        /// Updates the status bar message
        /// </summary>
        public void UpdateStatusBarMessage()
        {
            bool bSelected = false;
            if((g_selectedObject != null) || (g_selectedLight != null) || (g_selectedCollision != null) || (g_selectedActor != null) || (g_selectedMisc != null))
                bSelected = true;
            //update status bar info
            if (!bSelected)
            {
                switch (g_brushMode)
                {
                    case BRUSH_MODE_TILES:
                        SetStatusBarMessage("[RMB:clear] [S:Selection] [Ctrl+V:copy selection] [Ctrl+F - Fill Selection] [Del - Clear selection(all visible)] [Ctrl+PgUP,PgDN - move to layer] [SHIFT - act on all visible layers]");
                        break;
                    case BRUSH_MODE_OBJECTS:
                        SetStatusBarMessage("[RMB:select] [LMB:add/move] [S:Selection] [Ctrl+dir:move] [X,Y:Flip] [PgUP/DN:order] [Ctrl+PgUP/DN:layer] [C:toggle Cover]");
                        break;
                    case BRUSH_MODE_LIGHTS:
                        SetStatusBarMessage("[RMB:select] [LMB:edit] [Ctrl+dir:move] [SHIFT:keep aspect when scaling] [CTRL+V:copy selected light");
                        break;
                    case BRUSH_MODE_COLLISIONS:
                        SetStatusBarMessage("[RMB: select] [LMB: move/scale] [Ctrl+dir:move] [ALT+dir:scale] [Ctrl+V:copy selected]");
                        break;
                    case BRUSH_MODE_ACTORS:
                        SetStatusBarMessage("[RMB:select] [LMB:move] [X - flipX]");
                        break;
                    case BRUSH_MODE_MISC:
                        SetStatusBarMessage("[RMB:select] [LMB:edit/move]");
                        break;
                    case BRUSH_MODE_AI:
                        SetStatusBarMessage("[RMB:select] [Alt+RMB:select lights] [Alt+Shift+RMB:select collision] [+Ctrl: pick target ID]");
                        break;
                    case BRUSH_MODE_PREFABS:
                        SetStatusBarMessage("Select a prefab from the Prefabs window then click to add it to the scene");
                        break;
                    default:
                        SetStatusBarMessage("Nothing to see here! Keep walking!");
                        break;
                }
            }
            else
            {
                if (g_selectedObject != null)
                {
                    int objidx = arrObjects.IndexOf(g_selectedObject);
                    SetStatusBarMessage("OBJECT [ID "+ g_selectedObject.ID +"] [paint order " + objidx + "] [Layer " + g_selectedObject.layer + "] [FlipX " + (((g_selectedObject.flags & OBJFLAG_FLIPX) != 0)?"true":"false") + "] [FlipY "+ (((g_selectedObject.flags & OBJFLAG_FLIPY) != 0) ? "true" : "false") + "]");
                }
                else if (g_selectedLight != null)
                {
                    int objidx = arrLights.IndexOf(g_selectedLight);
                    SetStatusBarMessage("LIGHT [ID " + g_selectedLight.ID + "] [paint order " + objidx + "]");
                }
                else if (g_selectedCollision != null)
                {
                    SetStatusBarMessage("COLLISION [ID " + g_selectedCollision.ID + "]");
                }
                else if (g_selectedActor != null)
                {
                    int objidx = arrActors.IndexOf(g_selectedActor);
                    SetStatusBarMessage("ACTOR [ID " + g_selectedActor.ID + "] [paint order " + objidx + "]");
                }
                else if (g_selectedMisc != null)
                {
                    if (g_selectedMisc.type == K_MISC_RAIL)
                    {
                        CMiscObject_Rail rail = g_selectedMisc as CMiscObject_Rail;
                        if (rail.selectedNodeIdx >= 0)
                        {
                            PointF pt = (PointF)rail.listPoints[rail.selectedNodeIdx];
                            SetStatusBarMessage("MISC OBJECT: RAIL [ID " + g_selectedMisc.ID + "] point X:" + (pt.X - CAMERA_ORIGIN.X) + " Y:" + (pt.Y - CAMERA_ORIGIN.Y));
                        }
                        else
                        {
                            SetStatusBarMessage("MISC OBJECT: RAIL [ID " + g_selectedMisc.ID + "]");
                        }
                    }
                    else
                        SetStatusBarMessage("MISC OBJECT [ID " + g_selectedMisc.ID + "]");
                }
            }
        }

        // Gets the already set background misc element AI data
        public ArrayList GetLevelBackgroundData()
        {
            CMiscObject_Background objbg = null;
            for (int kk = 0; kk < arrMisc.Count; kk++)
            {
                CMiscObjectBase mob = arrMisc[kk] as CMiscObjectBase;
                if (mob.type == K_MISC_BACKGROUND)
                {
                    objbg = arrMisc[kk] as CMiscObject_Background;
                    break;
                }
            }
            //not found, add it now
            if (objbg == null)
            {
                objbg = new CMiscObject_Background();
                objbg.ID = GetUniqueID();
                objbg.pos = gLevelOrigin;
                objbg.listParams.Add("str_bsx"); //name
                objbg.listParams.Add(".bsx"); //val
                objbg.listParams.Add("str_anim"); //name
                objbg.listParams.Add("SET_BG_ANIM"); //val
                objbg.listParams.Add("str_water_anim"); //name
                objbg.listParams.Add("SET_WATER_ANIM"); //val
                arrMisc.Add(objbg);
            }

            return objbg.listParams;
        }

        // Sets the background AI data
        public void SetLevelBackgroundData(ArrayList arrAIParams)
        {
            CMiscObject_Background objbg = null;
            for (int kk = 0; kk < arrMisc.Count; kk++)
            {
                CMiscObjectBase mob = arrMisc[kk] as CMiscObjectBase;
                if (mob.type == K_MISC_BACKGROUND)
                {
                    objbg = arrMisc[kk] as CMiscObject_Background;
                    break;
                }
            }
            //not found, add it now
            if (objbg == null)
            {
                objbg = new CMiscObject_Background();
                objbg.ID = GetUniqueID();
                objbg.pos = gLevelOrigin;
                objbg.listParams.Add("str_bsx"); //name
                objbg.listParams.Add(".bsx"); //val
                objbg.listParams.Add("str_anim"); //name
                objbg.listParams.Add("SET_BG_ANIM"); //val
                objbg.listParams.Add("str_water_anim"); //name
                objbg.listParams.Add("SET_WATER_ANIM"); //val
                arrMisc.Add(objbg);
            }
            //copy params
            objbg.listParams.Clear();
            for (int kk = 0; kk < arrAIParams.Count; kk++)
            {
                string strval = arrAIParams[kk] as string;
                objbg.listParams.Add(strval);
            }
        }

        private void butWndLights_Click(object sender, EventArgs e)
        {
            HideToolWindows();

            g_wndLights.Show();
            g_wndLights.Left = this.Left + this.Width;
            g_wndLights.Top = this.Top;

            g_brushMode = BRUSH_MODE_LIGHTS;

            UpdateStatusBarMessage();
            PaintMap();
        }

        private void butWndCollision_Click(object sender, EventArgs e)
        {
            HideToolWindows();

            g_wndCollisions.Show();
            g_wndCollisions.Left = this.Left + this.Width;
            g_wndCollisions.Top = this.Top;

            g_brushMode = BRUSH_MODE_COLLISIONS;

            UpdateStatusBarMessage();
            PaintMap();
        }

        private void butWndObjects_Click(object sender, EventArgs e)
        {
            HideToolWindows();

            g_wndObjects.Show();
            g_wndObjects.Left = this.Left + this.Width;
            g_wndObjects.Top = this.Top;

            g_brushMode = BRUSH_MODE_OBJECTS;

            UpdateStatusBarMessage();
            PaintMap();
        }

        private void butWndAI_Click(object sender, EventArgs e)
        {
            HideToolWindows();

            g_wndAI.Show();
            g_wndAI.Left = this.Left + this.Width;
            g_wndAI.Top = this.Top;

            g_brushMode = BRUSH_MODE_AI;

            UpdateStatusBarMessage();
            PaintMap();
        }

        //cand faci hover la fereastra principala primeste focusul
        private void Form1_MouseEnter(object sender, EventArgs e)
        {
            this.BringToFront();
        }

        private void showGridToolStripMenuItem_Click(object sender, EventArgs e)
        {
            PaintMap();
        }

        private void chk_showOverlappingTiles_CheckedChanged(object sender, EventArgs e)
        {
            PaintMap();
        }

        private void butWndMisc_Click(object sender, EventArgs e)
        {
            HideToolWindows();

            g_wndMisc.Show();
            g_wndMisc.Left = this.Left + this.Width;
            g_wndMisc.Top = this.Top;

            g_brushMode = BRUSH_MODE_MISC;

            UpdateStatusBarMessage();
            PaintMap();
        }

        private void setGroundLevelToolStripMenuItem_Click(object sender, EventArgs e)
        {
            g_wndMaterials.Hide();
            g_wndLights.Hide();
            g_wndObjects.SetSelectedObject(null);
            g_wndObjects.Hide();
            g_wndCollisions.Hide();

            g_brushMode = BRUSH_MODE_GROUND_LEVEL;
        }

        private void setMissionTypeToolStripMenuItem_Click(object sender, EventArgs e)
        {
            FormMissionType wndMission = new FormMissionType(this);
            wndMission.Owner = this;
            wndMission.ShowInTaskbar = false;
            wndMission.ShowDialog();

            SetFileModified();
        }

        private void clearAllActorsToolStripMenuItem_Click(object sender, EventArgs e)
        {
            DialogResult result = MessageBox.Show(this, "Are you sure you want to erase all actors? This cannot be undone!", "Are you sure?", MessageBoxButtons.OKCancel);

            if (result == DialogResult.OK)
            {
                arrActors.Clear();
                PaintMap();
            }
        }

        private void pictureBox1_SizeChanged(object sender, EventArgs e)
        {
            //link picture box data
            pbImg = new Bitmap(pictureBox1.Width, pictureBox1.Height);
            pbGr = Graphics.FromImage(pbImg);
            pictureBox1.Image = pbImg;
            pbGr.SmoothingMode = System.Drawing.Drawing2D.SmoothingMode.None;
            pbGr.InterpolationMode = InterpolationMode.NearestNeighbor;
            pbGr.PixelOffsetMode = PixelOffsetMode.HighQuality;

            PaintMap();
        }

        private void saveToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (g_strFilePath.Length <= 0)
            {
                saveAsToolStripMenuItem_Click(sender, e);
            }
            else
            {
                SaveLevel_V2(g_strFilePath);
            }
        }

        private void SaveFileBackup()
        {
            if ((g_strFilePath.Length > 0) && (g_bFileModified == true))
            {
                string strASPath = g_strFilePath + "_auto";
                SaveLevel_V2(strASPath);
            }
        }

        //Sets the latest undo state
        public void Undo_SetCurrent(int nNewUndoState)
        {
            if (nNewUndoState == K_UNDO_DISABLED)
            {
                //clear undo levels for tiles
                gMap.Undo_ClearUndo();
                //clear file level undo
                UndoMemStream = null;

                undoToolStripMenuItem.Enabled = false;
            }
            else
            {
                undoToolStripMenuItem.Enabled = true;
            }

            gUndoStatus = nNewUndoState;
        }

        public void Undo_ExecuteCurrent()
        {
            switch (gUndoStatus)
            {
                case K_UNDO_TILES:
                    {
                        for (int kk = gMap.Blocks.Count - 1; kk >= 0; kk--)
                        {
                            CTileBlock tb = gMap.Blocks[kk] as CTileBlock;
                            tb.UndoChange();
                        }

                        gMap.ClearEmptyblocks();
                        BuildAllBlockImages();

                        PaintMap();
                    }
                    break;
                case K_UNDO_PREFAB:
                    {
                        //load previously saved file
                        LoadFileFromMemoryBackup();

                        gMap.ClearEmptyblocks();
                        BuildAllBlockImages();

                        PaintMap();
                    }
                    break;
                default:
                    {
                        System.Media.SystemSounds.Asterisk.Play();
                    }
                    break;
            }
            //clear undo
            Undo_SetCurrent(K_UNDO_DISABLED);
        }

        //saves the current file to a memory stream for undo purposes
        private void SaveFileToMemoryBackup()
        {
            UndoMemStream = new MemoryStream(200000);
            SaveLevel_V2(g_strFilePath, false, UndoMemStream);
        }

        //loads the level from the mem backup
        private void LoadFileFromMemoryBackup()
        {
            if (UndoMemStream != null)
            {
                LoadLevel_V2(g_strFilePath, false, 0, 0, UndoMemStream);
            }
        }

        //autosave timer
        private void timer_autosave_Tick(object sender, EventArgs e)
        {
            SaveFileBackup();
            //re-enable timer
            timer_autosave.Enabled = true;
            //mark file as modified
            SetFileModified();
        }

        private void Form1_FormClosing(object sender, FormClosingEventArgs e)
        {
            if (g_bFileModified)
            {
                if (MessageBox.Show("Really QUIT without saving?", "Discard changes", MessageBoxButtons.YesNo) == DialogResult.No)
                {
                    e.Cancel = true;
                    this.Activate();
                }
            }
        }

        private void findItNowToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Int32 targetID = Convert.ToInt32(menuTextBoxIDFinder.Text);
            PointF ptPos = new PointF(0.0f, 0.0f);
            int nFoundBrush = -1;

            for (int kk = 0; kk < arrObjects.Count; kk++)
            {
                CObject obj = arrObjects[kk] as CObject;
                if (obj.ID == targetID)
                {
                    if (g_lastSearchBrush == BRUSH_MODE_OBJECTS)
                        continue;

                    ptPos = new PointF(obj.pos.X, obj.pos.Y);
                    nFoundBrush = BRUSH_MODE_OBJECTS;
                }
            }
            if (nFoundBrush < 0)
            {
                for (int kk = 0; kk < arrLights.Count; kk++)
                {
                    CLight light = arrLights[kk] as CLight;
                    if (light.ID == targetID)
                    {
                        if (g_lastSearchBrush == BRUSH_MODE_LIGHTS)
                            continue;

                        ptPos = new PointF(light.pos.X, light.pos.Y);
                        nFoundBrush = BRUSH_MODE_LIGHTS;
                    }
                }
            }
            if (nFoundBrush < 0)
            {
                for (int kk = 0; kk < arrCollisions.Count; kk++)
                {
                    CCollisionElement coll = arrCollisions[kk] as CCollisionElement;
                    if (coll.ID == targetID)
                    {
                        if (g_lastSearchBrush == BRUSH_MODE_COLLISIONS)
                            continue;

                        ptPos = new PointF(coll.rect.X + coll.rect.Width / 2.0f, coll.rect.Y + coll.rect.Height / 2.0f);
                        nFoundBrush = BRUSH_MODE_COLLISIONS;
                    }
                }
            }
            if (nFoundBrush < 0)
            {
                for (int kk = 0; kk < arrMisc.Count; kk++)
                {
                    CMiscObjectBase misc = arrMisc[kk] as CMiscObjectBase;
                    if (misc.ID == targetID)
                    {
                        if (g_lastSearchBrush == BRUSH_MODE_MISC)
                            continue;

                        ptPos = misc.GetOrigin();
                        nFoundBrush = BRUSH_MODE_MISC;
                    }
                }
            }
            if (nFoundBrush < 0)
            {
                for (int kk = 0; kk < arrActors.Count; kk++)
                {
                    CActor actor = arrActors[kk] as CActor;
                    if (actor.ID == targetID)
                    {
                        if (g_lastSearchBrush == BRUSH_MODE_ACTORS)
                            continue;

                        ptPos = new PointF(actor.pos.X, actor.pos.Y);
                        nFoundBrush = BRUSH_MODE_MISC;
                    }
                }
            }
            
            if (nFoundBrush < 0)
            {
                MessageBox.Show("ID:" + targetID + " not found!");
                return;
            }

            butWndAI_Click(sender, e);
            //mark last search and redraw
            g_lastSearchID = targetID;
            g_lastSearchPos = ptPos;
            g_lastSearchBrush = nFoundBrush;
            cameraPos.X = ptPos.X - (pictureBox1.Width / 2) * (1.0f / zoomLevel);
            cameraPos.Y = ptPos.Y - (pictureBox1.Height / 2) * (1.0f / zoomLevel);
            PaintMap();
        }

        private void menuTextBoxIDFinder_KeyPress(object sender, KeyPressEventArgs e)
        {
            if (!char.IsControl(e.KeyChar) && !char.IsDigit(e.KeyChar))
            {
                e.Handled = true;
            }
        }

        private void clearResultToolStripMenuItem_Click(object sender, EventArgs e)
        {
            g_lastSearchID = -1;
            PaintMap();
        }

        private void exportPrefabToolStripMenuItem_Click(object sender, EventArgs e)
        {
            SaveFileDialog sfd = new SaveFileDialog();
            sfd.InitialDirectory = g_wndPrefabs.GetPrefabsFolder();
            //fisier binar
            sfd.Filter = "BreacherSquad Prefab (*.bs_prefab)|*.bs_prefab|All Files (*.*)|*.*";
            if (sfd.ShowDialog() == DialogResult.Cancel)
                return;

            bool bSuccess = SaveLevel_V2(sfd.FileName, true);
            if (bSuccess)
            {
                MessageBox.Show("Prefab exported correctly:\n\r" + sfd.FileName, "Prefab exported", MessageBoxButtons.OK, MessageBoxIcon.Information);
            }
        }

        private void butWndPrefabs_Click(object sender, EventArgs e)
        {
            HideToolWindows();

            g_wndPrefabs.Show();
            g_wndPrefabs.Left = this.Left + this.Width;
            g_wndPrefabs.Top = this.Top;

            g_brushMode = BRUSH_MODE_PREFABS;

            UpdateStatusBarMessage();
            PaintMap();
        }

        private void deleteOverlappingTilesToolStripMenuItem_Click(object sender, EventArgs e)
        {
            for (int kk = 0; kk < gMap.Blocks.Count; kk++)
            {
                CTileBlock tb = gMap.Blocks[kk] as CTileBlock;

                for (int yy = 0; yy < BLOCK_H; yy++)
                {
                    for (int xx = 0; xx < BLOCK_W; xx++)
                    {
                        int maxl = 0;
                        if (tb.tiles[xx, yy].tileID[1] >= 0)
                            maxl = 1;
                        if (tb.tiles[xx, yy].tileID[2] >= 0)
                            maxl = 2;

                        for (int oo = 0; oo < maxl; oo++)
                        {
                            tb.tiles[xx, yy].tileID[oo] = -1;                            
                        }
                    }
                }
            }

            PaintMap();
        }

        private void openPrefabToolStripMenuItem_Click(object sender, EventArgs e)
        {
            OpenFileDialog sfd = new OpenFileDialog();
            //fisier binar
            sfd.Filter = "BreacherSquad Prefab (*.bs_prefab)|*.bs_prefab|All Files (*.*)|*.*";
            if (sfd.ShowDialog() == DialogResult.Cancel)
                return;

            ResetLevel();

            bool bSuccess = LoadLevel_V2(sfd.FileName);

            PaintMap();

            if (bSuccess)
                MessageBox.Show("Prefab loaded successfully!", "Success!");

        }

        private void undoToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Undo_ExecuteCurrent();
        }

        private void setLevelBackgroundToolStripMenuItem_Click(object sender, EventArgs e)
        {
            BkgSelector wndMission = new BkgSelector(this);
            wndMission.Owner = this;
            wndMission.ShowInTaskbar = false;
            wndMission.ShowDialog();

            SetFileModified();
        }

        private void uploadSingleLevelToolStripMenuItem_Click(object sender, EventArgs e)
        {
            ModCreator wndMods = new ModCreator(this);
            wndMods.Owner = this;
            wndMods.ShowInTaskbar = false;
            //set mod data
            wndMods.SetModType(ModCreator.eModType.MOD_SINGLE_LEVEL);

            wndMods.ShowDialog();
        }

        private void playMapToolStripMenuItem_Click(object sender, EventArgs e)
        {
            /*
            if (g_strFilePath.Length == 0)
            {
                MessageBox.Show("Please save your level first! You should save the level in the media/levels/missions folder of the game.", "Warning");
                return;
            }

            //call on game to upload mod
            string strExePath = GetExePath();
            string strGamePath = System.IO.Path.Combine(strExePath, "..\\ActionSquad.exe");

            ProcessStartInfo startInfo = new ProcessStartInfo();
            startInfo.CreateNoWindow = false;
            startInfo.UseShellExecute = false;
            startInfo.FileName = strGamePath;
            startInfo.WindowStyle = ProcessWindowStyle.Hidden;
            startInfo.Arguments = " +map \"" + g_strFilePath + "\"";

            Process exeProcess = Process.Start(startInfo);
            */
        }

        private void radio_layer_CheckedChanged(object sender, EventArgs e)
        {
            for (int kk = 0; kk < (int)ELayer.LAYERS_CNT; kk++)
            {
                if (layers_radios[kk] == sender)
                {
                    g_selectedLayer = kk;
                    return;
                }
            }
        }

        private void chk_layer_CheckedChanged(object sender, EventArgs e)
        {
            BuildAllBlockImages();
            PaintMap();
        }

        private void butWndActors_Click(object sender, EventArgs e)
        {
            HideToolWindows();

            g_wndActors.Show();
            g_wndActors.Left = this.Left + this.Width;
            g_wndActors.Top = this.Top;

            g_brushMode = BRUSH_MODE_ACTORS;

            UpdateStatusBarMessage();
            PaintMap();
        }

        private void AddPrefab(PointF vPos, string strPrefabPath)
        {
            if (strPrefabPath == null)
            {
                MessageBox.Show("Error adding prefab: Prefab not selected!", "Warning");
                return;
            }
            //add in local level coordinates
            LoadLevel_V2(strPrefabPath, true, (int)(vPos.X / TILE_WIDTH), (int)(vPos.Y / TILE_HEIGHT));
        }


    }
}
