using System;
using System.Collections.Generic;
using System.Text;
using System.Drawing;
using System.Drawing.Imaging;
using System.Xml;
using System.Windows.Forms;
using System.IO;

//----------------------------------------------------------	
//FORMAT EXPORT
//    - version (1ub)
//    - export flags (6ub = 2ub ce exporta + 4ub pe ce dimensiuni)
//    - nr imagini (1ub)
//        - numele imagine [*]
//    - MODULES Count (2ub)  [*]
//        - MODULE imgID (1ub)[*]
//        - x, y, w, h (*b)
//    - FMODULES Count (2ub)[*]
//        - moduleIDx (*b)
//        - OX, OY (*b)
//        - flipFlags	(*b)[*]
//    - FRAMES Count   (2ub)[*]
//        - FModules Count (1ub)
//            - FModule IDx (*ub)
//        - BBox X, Y, W, H (*b)[*]
//        - HitPts Count (1b)[*]
//            - X, Y   (*b)
//            - flags	 (*b) [*]
//    - AFRAMES Count (2ub)[*]
//        - Frame IDx   (*ub)
//        - Duration	 (*ub)	[*]
//        - MoveX, MoveY (*b) [*]
//        - flags		   (*ub)[*]
//    - ANIMS Count  (2ub)[*]
//        - Anim Flags (*ub)
//        - Aframes Count	(1ub)
//            - Aframes IDx  (*ub)
//EXPLICATII EXPORT
//    (2b) = 2 bytes; (*b) = select din form export pe cati bytes			
//    (2ub) = 2 bytes fara semn
//    [*] = checkbox suprimare in export form
//-------------------------------------------------------------------------	


namespace InkEd3
{
    //pt fn de callback din mainwnd
    public enum CallbackCommands { UpdateParams, UpdateHitpoint };

    public class FontData
    {
        public string ID;
        public int LetterSpacing;
        public int RowSpacing;
        public int RowHeight;
        public int SpaceSize;
    }

    public class Entity
    {
        public int index = 0; //indexul sub care va aparea in fisierul exportat
        public int ID = 0; //identificator unic, nemodificabil = cheie primara
    }

    public class Module : Entity
    {
        public int type = 0; //la ce foloseste? (nu-l salvez inca)
        public int imageID = 0; //renuntam si aici la index
        public int x = 0, y = 0, w = 0, h = 0;
        public Image image;
        public string strName = ""; //name of module image (image source)

        public Module()
        {
            type = 0;
            imageID = 0;
            x = 0; y = 0; w = 0; h = 0;
            image = null;
            strName = "";
        }

        public void Paint(Graphics g, int x, int y, RotateFlipType FlipType)
        {
            g.SmoothingMode = System.Drawing.Drawing2D.SmoothingMode.None;
            g.CompositingQuality = System.Drawing.Drawing2D.CompositingQuality.HighSpeed;
            g.InterpolationMode = System.Drawing.Drawing2D.InterpolationMode.NearestNeighbor;
            if (image != null)
            {
                image.RotateFlip(FlipType);
                g.DrawImage(image, x - 0.5f, y - 0.5f, new Rectangle(-1, -1, image.Width + 2, image.Height + 2), GraphicsUnit.Pixel);
                image.RotateFlip(FlipType);
            }
        }
        
        public void Paint(Graphics g, int x, int y)
        {
            g.SmoothingMode = System.Drawing.Drawing2D.SmoothingMode.None;
            g.CompositingQuality = System.Drawing.Drawing2D.CompositingQuality.HighSpeed;
            g.InterpolationMode = System.Drawing.Drawing2D.InterpolationMode.NearestNeighbor;
            
            if (image != null)
            {
                g.DrawImage(image, x - 0.5f, y - 0.5f, new Rectangle(-1, -1, image.Width + 2, image.Height + 2), GraphicsUnit.Pixel);
            }
        }

        public void Paint(Graphics g, int x, int y, float alpha)
        {
            g.SmoothingMode = System.Drawing.Drawing2D.SmoothingMode.None;
            g.CompositingQuality = System.Drawing.Drawing2D.CompositingQuality.HighSpeed;
            g.InterpolationMode = System.Drawing.Drawing2D.InterpolationMode.NearestNeighbor;
            
            if (image != null)
            {
                //create a color matrix object  
                ColorMatrix matrix = new ColorMatrix();
                //set the opacity  
                matrix.Matrix33 = alpha;
                //create image attributes  
                ImageAttributes attributes = new ImageAttributes();
                //set the color(opacity) of the image  
                attributes.SetColorMatrix(matrix, ColorMatrixFlag.Default, ColorAdjustType.Bitmap);

                PointF[] points = {
                        new PointF(x - 0.5f, y - 0.5f),
                        new PointF(x - 0.5f + image.Width + 2, y - 0.5f),
                        new PointF(x - 0.5f, y - 0.5f + image.Height + 2),
                        };
                g.DrawImage(image, points, new Rectangle(-1, -1, image.Width + 2, image.Height + 2), GraphicsUnit.Pixel, attributes);
            }
        }
    }

    public class FrameModule: Entity
    {
        public Module module = null;
        public int moduleID = 0; //cheie externa (in caz ca se pierde cumva referinta module)
        public int ox = 0, oy = 0;
        public int flags = 0;

        public Rectangle GetRect()
        {
            Rectangle rect = new Rectangle();
            rect.X = ox;
            rect.Y = oy;
            rect.Width = module.w;
            rect.Height = module.h;
            return rect;
        }
    }

    public class HitPoint
    {
        public int X;
        public int Y;
        public int flags;

        public HitPoint()
        {
            X = 0;
            Y = 0;
            flags = 0;
        }

        public HitPoint(int nX, int nY)
        {
            X = nX;
            Y = nY;
            flags = 0;
        }

    }

    public class Frame: Entity
    {
        public List<FrameModule> fmodules = new List<FrameModule>();
        public List<HitPoint> hitPoints = new List<HitPoint>();
        public Rectangle BBox = new Rectangle(); //bounding box

        public FrameModule AddFModule(Module mod, int flags)
        {
            FrameModule fm = new FrameModule();
            int max = 0;
            foreach (FrameModule fmi in fmodules)
                if (fmi.ID > max)
                    max = fmi.ID;
            fm.ID = max+1;
            fm.module = mod;
            fm.moduleID = mod.ID;
            fm.flags = flags;
            fmodules.Add(fm);
            return fm;
        }

        public FrameModule GetFModuleByIndex(int index)
        {
            foreach (FrameModule fm in fmodules)
                if (fm.index == index)
                    return fm;
            return null;
        }

        public void DeleteFModule(int index)
        {
            fmodules.Remove(GetFModuleByIndex(index));
            foreach (FrameModule fm in fmodules)
                if (fm.index > index)
                    fm.index--;
        }

        public void DeleteFModule(FrameModule fmod)
        {
            fmodules.Remove(fmod);
            int index = fmod.index;
            foreach (FrameModule fm in fmodules)
                if (fm.index > index)
                    fm.index--;
        }

        public Rectangle GetRect()
        {
            Rectangle rect = new Rectangle();
            foreach (FrameModule fm in fmodules)
            {
                if (rect.IsEmpty)
                    rect = fm.GetRect();
                else
                    rect = Rectangle.Union(rect, fm.GetRect());
            }
            return rect;
        }

        public void SortFModules()
        {            
            fmodules.Sort(Sprite.CompareEntities);
        }

        public void PaintWithAlpha(Graphics g, int x, int y, float alpha)
        {
            foreach (FrameModule fm in fmodules)
            {
                fm.module.Paint(g, fm.ox + x, fm.oy + y, alpha);
            }
        }

        public void Paint(Graphics g, int x, int y)
        {
            foreach (FrameModule fm in fmodules)
            {
                if ((fm.flags & 3) == 0)
                {
                    fm.module.Paint(g, fm.ox + x, fm.oy + y);
                }
                else if((fm.flags & 3) == 3)
                {
                    fm.module.Paint(g, fm.ox + x, fm.oy + y, RotateFlipType.RotateNoneFlipXY);
                }
                else if((fm.flags & 2) != 0)
                {
                    fm.module.Paint(g, fm.ox + x, fm.oy + y, RotateFlipType.RotateNoneFlipY);
                }
                else if ((fm.flags & 1) != 0)
                {
                    fm.module.Paint(g, fm.ox + x, fm.oy + y, RotateFlipType.RotateNoneFlipX);
                }
            }
        }

        public void Paint(Graphics g)
        {
            Paint(g, 0, 0);
        }
    }

    public class AnimFrame: Entity
    {
        public Frame frame = null;
        public int frameID = 0;
        public int ox = 0, oy = 0;
        public int time = 1;
        public string strFlags = "";
    }

    public class Animation : Entity
    {
        public List<AnimFrame> aframes = new List<AnimFrame>();
        public int flags = 0;
        public string name = "";

        public AnimFrame AddAFrame(Frame f)
        {
            AnimFrame af = new AnimFrame();
            int max = 0;
            foreach (AnimFrame afi in aframes)
                if (afi.ID > max)
                    max = afi.ID;
            af.ID = max+1;
            af.frame = f;
            af.frameID = f.ID;
            aframes.Add(af);
            return af;
        }

        public AnimFrame GetAFrameByIndex(int index)
        {
            foreach (AnimFrame af in aframes)
                if (af.index == index)
                    return af;
            return null;
        }


        public void DeleteAFrame(int index)
        {
            aframes.Remove(GetAFrameByIndex(index));
            foreach (AnimFrame af in aframes)
                if (af.index > index)
                    af.index--;
        }
    }

    public class Sprite
    {
        #region Constants

        public const byte VERSION = 0x30;
        //ce exporta
		public const UInt16 MASK_EXPORT_IMAGENAMES				= 1;
		public const UInt16 MASK_EXPORT_MODULES				    = 1 << 1;
		public const UInt16 MASK_EXPORT_MODULE_IMAGEIDX		    = 1 << 2;
		public const UInt16 MASK_EXPORT_FRAMEMODULES			= 1 << 3;
        public const UInt16 MASK_EXPORT_FRAMEMODULES_FLAGS      = 1 << 4;
        public const UInt16 MASK_EXPORT_FRAMES                  = 1 << 5;
        public const UInt16 MASK_EXPORT_FRAMEBBOX               = 1 << 6;
        public const UInt16 MASK_EXPORT_FRAMEHITPOINTS          = 1 << 7;
        public const UInt16 MASK_EXPORT_FRAMEHITPOINT_FLAGS     = 1 << 8;
        public const UInt16 MASK_EXPORT_AFRAMES                 = 1 << 9;
        public const UInt16 MASK_EXPORT_AFRAMES_DURATION        = 1 << 10;
        public const UInt16 MASK_EXPORT_AFRAMES_OFFSETS         = 1 << 11;
        public const UInt16 MASK_EXPORT_AFRAMES_FLAGS           = 1 << 12;
        public const UInt16 MASK_EXPORT_ANIMATIONS              = 1 << 13;
        //pe ce dimensiuni exporta
        //avem doua variabile pt marimi m_b1, m_b2; 
        //daca din m_b1|mask si m_b2|mask formam 2 biti, valoarea reprezinta numarul de bytes pe care se exporta variabila
        public const UInt16 MASK_EXPORTSZ_MODULEXYWH            = 1;
        public const UInt16 MASK_EXPORTSZ_FMODULE_MODULEINDEX   = 1<<1;
        public const UInt16 MASK_EXPORTSZ_FMODULE_OFFSETS       = 1<<2;
        public const UInt16 MASK_EXPORTSZ_FMODULE_FLAGS         = 1<<3;
        public const UInt16 MASK_EXPORTSZ_FRAME_FMODULEINDEX    = 1<<4;
        public const UInt16 MASK_EXPORTSZ_FRAME_BBOX            = 1<<5;
        public const UInt16 MASK_EXPORTSZ_FRAME_HITPOINTS_XY    = 1<<6;
        public const UInt16 MASK_EXPORTSZ_FRAME_HITPOINTS_FLAGS = 1<<7;
        public const UInt16 MASK_EXPORTSZ_AFRAME_FRAMEINDEX     = 1<<8;
        public const UInt16 MASK_EXPORTSZ_AFRAME_FRAMEDURATION  = 1<<9;
        public const UInt16 MASK_EXPORTSZ_AFRAME_OFFSETS        = 1<<10;
        public const UInt16 MASK_EXPORTSZ_AFRAME_FLAGS          = 1<<11;
        public const UInt16 MASK_EXPORTSZ_ANIMATION_AFRAMEINDEX = 1<<12;
        public const UInt16 MASK_EXPORTSZ_ANIMATION_FLAGS       = 1<<13;

        #endregion

        public string strAtlasName = "";

        public List<Image> images = new List<Image>();
        public List<string> imgPath = new List<string>();

        public List<Module> modules = new List<Module>();
        public List<Frame> frames = new List<Frame>();
        public List<Animation> anims = new List<Animation>();

        public static FontData fontData;
        public static bool saveFontData;

        #region Modules

        public bool IsModuleCorrect(Module m)
        {
            if (m == null) return true;
            if ((m.imageID >= images.Count) || (m.imageID < 0)) return false;
            if ((m.w <= 0) || (m.h <= 0) || (m.x < 0) || (m.y < 0) || (m.w + m.x > images[m.imageID].Width) || (m.y + m.h > images[m.imageID].Height)) return false;
            return true;
        }

        public Module AddModule()
        {
            Module m = new Module();
            int max = 0;
            for (int i = 0; i < modules.Count; i++)
                if (modules[i].ID > max)
                    max = modules[i].ID;
            m.ID = max+1; 
            modules.Add(m);
            return m;
        }

        public void DeleteModule(int index)
        {
            Module mod = GetModuleByIndex(index);
            if (mod == null)
                return;
            modules.Remove(mod);
            foreach (Module m in modules)
                if (m != null && m.index > index)
                    m.index--;
            //sterge si toate frame modules care faceau referinta la el            
            foreach (Frame f in frames){
                for(int i=0; i<f.fmodules.Count; i++)
                {
                    FrameModule fm = f.fmodules[i];
                    if (fm.module == mod || fm.moduleID == mod.ID) //daca cumva se pierde referinta la obiect, ramane ID-ul
                    {
                        i--;
                        f.DeleteFModule(fm);                    
                    }
                }
            }
        }

        public Module GetModuleByPoint(Point pt, int imageIdx)
        {
            for (int kk = 0; kk < modules.Count; kk++)
            {
                Module mod = modules[kk];
                if ((modules[kk].imageID == imageIdx) && (pt.X >= mod.x) && (pt.Y >= mod.y) && (pt.X <= mod.x + mod.w) && (pt.Y <= mod.y + mod.h))
                {
                    return mod;
                }
            }
            return null;
        }

        Module GetModuleByID(int ID)
        {
            foreach (Module mod in modules)
                if (mod.ID == ID)
                    return mod;
            return null;
        }

        public Module GetModuleByIndex(int index)
        {
            foreach (Module mod in modules)
                if (mod.index == index)
                    return mod;
            return null;
        }

        public Module GetModuleByName(string sName)
        {
            foreach (Module mod in modules)
                if (mod.strName == sName)
                    return mod;
            return null;
        }

        #endregion

        #region Frames

        public Frame AddFrame()
        {
            Frame f = new Frame();
            int max = 0;
            foreach (Frame fi in frames)
                if (fi.ID > max)
                    max = fi.ID;
            f.ID = max+1;
            frames.Add(f);
            return f;
        }



        public void DeleteFrame(int index)
        {            
            Frame frame = GetFrameByIndex(index);
            frames.Remove(frame);
            foreach (Frame f in frames)
                if (f.index > index)
                    f.index--;
            //sterge anime frames care au referinta spre el
            foreach(Animation a in anims)
                for (int i = 0; i < a.aframes.Count; i++)
                {
                    AnimFrame af = a.aframes[i];
                    if (af.frame == frame || af.frameID == frame.ID)
                    {
                        a.DeleteAFrame(af.index); //TD: delete(af)
                        i--;
                    }
                }
        }

        Frame GetFrameByID(int ID)
        {
            foreach (Frame f in frames)
                if (f.ID == ID)
                    return f;
            return null;
        }

        public Frame GetFrameByIndex(int index)
        {
            foreach (Frame f in frames)
                if (f.index == index)
                    return f;
            return null;
        }

        public List<FrameModule> ExportFModules()
        {            
            List<FrameModule> fmodules = new List<FrameModule>();
            frames.Sort(CompareEntities);
            foreach (Frame f in frames)
            {
                f.SortFModules();
                fmodules.AddRange(f.fmodules);
            }
            return fmodules;
        }

        #endregion

        #region Animations

        public Animation AddAnim()
        {
            Animation a = new Animation();
            int max = 0;
            foreach (Animation ai in anims)
                if (ai.ID > max)
                    max = ai.ID;
            a.ID = max+1;
            anims.Add(a);
            return a;
        }

        public void DeleteAnim(int index)
        {
            Animation anim = GetAnimByIndex(index);
            anims.Remove(anim);
            foreach (Animation an in anims)
                if (an.index > index)
                    an.index--;
        }

        public Animation GetAnimByIndex(int index)
        {
            foreach (Animation a in anims)
                if (a.index == index)
                    return a;
            return null;
        }

        public List<AnimFrame> ExportAFrames()
        {
            List<AnimFrame> aframes = new List<AnimFrame>();
            anims.Sort(CompareEntities);
            foreach (Animation a in anims)
            {
                a.aframes.Sort(CompareEntities);
                aframes.AddRange(a.aframes);
            }
            return aframes;
        }

        #endregion

        public static int CompareEntities(Entity a, Entity b)
        {
            if (a.index == b.index)
                return 0;
            if (a.index < b.index)
                return -1;
            else
                return 1;
        }

        public void PaintImage(Graphics g, int idx, Rectangle dest)
        {
            if (idx < 0 || idx >= images.Count)
                return;
            Rectangle rect = new Rectangle(0, 0, images[idx].Width, images[idx].Height);
            g.SmoothingMode = System.Drawing.Drawing2D.SmoothingMode.None;
            g.CompositingQuality = System.Drawing.Drawing2D.CompositingQuality.HighSpeed;
            g.InterpolationMode = System.Drawing.Drawing2D.InterpolationMode.NearestNeighbor;
            g.DrawImage(images[idx], dest, rect, GraphicsUnit.Pixel);
        }

        public void PaintImageSmooth(Graphics g, int idx, Rectangle dest)
        {
            if (idx < 0 || idx >= images.Count)
                return;
            Rectangle rect = new Rectangle(0, 0, images[idx].Width, images[idx].Height);
            g.SmoothingMode = System.Drawing.Drawing2D.SmoothingMode.AntiAlias;
            g.CompositingQuality = System.Drawing.Drawing2D.CompositingQuality.HighQuality;
            g.InterpolationMode = System.Drawing.Drawing2D.InterpolationMode.Bilinear;
            g.DrawImage(images[idx], dest, rect, GraphicsUnit.Pixel);
        }

        //public void PaintModule(Graphics g, Module mod, int x, int y)
        //{
        //    Rectangle rect = new Rectangle(mod.x, mod.y, mod.w, mod.h);
        //    g.SmoothingMode = System.Drawing.Drawing2D.SmoothingMode.None;
        //    g.CompositingQuality = System.Drawing.Drawing2D.CompositingQuality.HighSpeed;
        //    g.InterpolationMode = System.Drawing.Drawing2D.InterpolationMode.NearestNeighbor;
        //    //g.DrawImage(images[mod.imageID], x, y, rect, GraphicsUnit.Pixel);
        //    DrawImage(g, moduleImages[mod.ID], x, y);
        //}

        //public void PaintModule(Graphics g, int idx, int x, int y)
        //{
        //    if (idx < 0 || idx >= modules.Count)
        //        return;
        //    //PaintModule(g, modules[idx], x, y);            
        //    DrawImage(g, moduleImages[idx], x, y);
        //}

        public void PaintModule(Graphics g, int idx, Rectangle dest)
        {
            if (idx < 0 || idx >= modules.Count)
                return;
            //Rectangle rect = new Rectangle(modules[idx].x, modules[idx].y, modules[idx].w, modules[idx].h);
            g.SmoothingMode = System.Drawing.Drawing2D.SmoothingMode.None;
            g.CompositingQuality = System.Drawing.Drawing2D.CompositingQuality.HighSpeed;
            g.InterpolationMode = System.Drawing.Drawing2D.InterpolationMode.NearestNeighbor;
            //g.DrawImage(images[modules[idx].imageID], dest, rect, GraphicsUnit.Pixel);
            g.DrawImage(modules[idx].image, dest);
        }

        public static void DrawImage(Graphics g, Image img, int x, int y)
        {
            if (img != null)
            {
                g.DrawImage(img, x - 0.5f, y - 0.5f, new Rectangle(-1, -1, img.Width + 2, img.Height + 2), GraphicsUnit.Pixel);
            }
        }

        public void MakeModuleImages()
        {
            foreach(Module mod in modules) {
                try
                {
                    mod.image = new Bitmap(mod.w, mod.h);
                    Graphics g = Graphics.FromImage(mod.image);
                    g.DrawImage(images[mod.imageID], 0, 0, new Rectangle(mod.x, mod.y, mod.w, mod.h), GraphicsUnit.Pixel);
                }
                catch (Exception)
                {
                    mod.image = new Bitmap(50, 50);
                    Graphics g = Graphics.FromImage(mod.image);
                    g.DrawString("ERR", new Font("Arial", 12, FontStyle.Bold), Brushes.Red, 0, 0);
                }
            }
        }

        public void WriteFontData(XmlTextWriter xw, FontData fd)
        {
            if (xw == null || fd == null) return;
            if (saveFontData == false) return;

            xw.WriteStartElement("FontData");

            xw.WriteStartAttribute("ID");
            xw.WriteValue(fd.ID);
            xw.WriteEndAttribute();

            xw.WriteStartAttribute("LetterSpacing");
            xw.WriteValue(fd.LetterSpacing);
            xw.WriteEndAttribute();

            xw.WriteStartAttribute("RowSpacing");
            xw.WriteValue(fd.RowSpacing);
            xw.WriteEndAttribute();

            xw.WriteStartAttribute("RowHeight");
            xw.WriteValue(fd.RowHeight);
            xw.WriteEndAttribute();

            xw.WriteStartAttribute("SpaceSize");
            xw.WriteValue(fd.SpaceSize);
            xw.WriteEndAttribute();

            xw.WriteEndElement();
        }

        public FontData GetFontData(XmlDocument xdoc)
        {
            if (xdoc == null) return null;

            XmlNodeList nl = xdoc.GetElementsByTagName("FontData");
            if (nl.Count == 0) return null;

            XmlNode n = nl[0];
            if (n == null) return null;

            FontData fd = new FontData();
            fd.ID = n.Attributes["ID"].Value;
            fd.LetterSpacing = Convert.ToInt32(n.Attributes["LetterSpacing"].Value);
            fd.RowSpacing = Convert.ToInt32(n.Attributes["RowSpacing"].Value);
            fd.RowHeight = Convert.ToInt32(n.Attributes["RowHeight"].Value);
            fd.SpaceSize = Convert.ToInt32(n.Attributes["SpaceSize"].Value);
            return fd;
        }


        public void Open(string path)
        {
            XmlDocument xdoc = new XmlDocument();
            xdoc.Load(path);
            //verifica versiunea
            XmlNodeList ver = xdoc.GetElementsByTagName("SpriteCollection");
            if ((ver.Count < 1) || (ver[0].Attributes["Version"].Value != "2.0"))
            {
                MessageBox.Show("Wrong XML version or illegal file!", "Open File Error!");
                return;
            }

            // load paired atlas name (if available)
            strAtlasName = "";
            if (ver[0].Attributes["AtlasJSON"] != null)
            {
                strAtlasName = ver[0].Attributes["AtlasJSON"].Value;
            }

            fontData = GetFontData(xdoc);
            if (fontData != null) saveFontData = true;

            XmlNodeList nodes = xdoc.GetElementsByTagName("Image");
            foreach (XmlNode imgNode in nodes)
            {
                string txt = imgNode.InnerText;
                imgPath.Add(txt);
                string dir = Path.GetDirectoryName(path);

                Image img = null;
                using (var bmpTemp = new Bitmap(dir + "\\" + txt))
                {
                    img = new Bitmap(bmpTemp);
                }
                images.Add(img);
            }
            nodes = xdoc.GetElementsByTagName("Module");
            int idx = 0;
            foreach (XmlNode node in nodes)
            {
                Module mod = new Module();
                mod.index = Convert.ToInt32(idx);
                mod.ID = Convert.ToInt32(idx);
                mod.imageID = Convert.ToInt32(node.Attributes["ImageIdx"].Value);
                mod.x = Convert.ToInt32(node.Attributes["X"].Value);
                mod.y = Convert.ToInt32(node.Attributes["Y"].Value);
                mod.w = Convert.ToInt32(node.Attributes["W"].Value);
                mod.h = Convert.ToInt32(node.Attributes["H"].Value);
                mod.strName = "";
                if (node.Attributes["strName"] != null)
                {
                    mod.strName = node.Attributes["strName"].Value;
                }
                modules.Add(mod);
                idx++;
            }
            //MakeModuleImages();

            XmlNodeList frameModulesNodes = xdoc.GetElementsByTagName("FrameModule");
            
            nodes = xdoc.GetElementsByTagName("Frame");
            idx = 0;
            foreach (XmlNode node in nodes)
            {
                Frame f = new Frame();
                f.index = Convert.ToInt32(idx);
                f.ID = Convert.ToInt32(idx);
                idx++;
                f.BBox.X = Convert.ToInt32(node.Attributes["BBoxX"].Value);
                f.BBox.Y = Convert.ToInt32(node.Attributes["BBoxY"].Value);
                f.BBox.Width = Convert.ToInt32(node.Attributes["BBoxW"].Value);
                f.BBox.Height = Convert.ToInt32(node.Attributes["BBoxH"].Value);
                //aici e cam hardcodare, dar stiu sigur ca am un singur not FModules per frame
                XmlNodeList fnodes = node.SelectNodes("FModules")[0].ChildNodes;

                int localIdx = 0;
                foreach (XmlNode fnode in fnodes)
                {
                    FrameModule fm = new FrameModule();
                    int fmodidx = Convert.ToInt32(fnode.Attributes["Idx"].Value);
                    fm.index = localIdx++;
                    fm.ID = Convert.ToInt32(fmodidx);
                    fm.moduleID = Convert.ToInt32(frameModulesNodes[fmodidx].Attributes["ModuleIdx"].Value);
                    fm.ox = Convert.ToInt32(frameModulesNodes[fmodidx].Attributes["OX"].Value);
                    fm.oy = Convert.ToInt32(frameModulesNodes[fmodidx].Attributes["OY"].Value);
                    fm.flags = Convert.ToInt32(frameModulesNodes[fmodidx].Attributes["Flags"].Value);
                    fm.module = modules[fm.moduleID];// GetModuleByID(fm.moduleID);
                    f.fmodules.Add(fm);
                }
                XmlNodeList hpnodes = node.SelectNodes("Points")[0].ChildNodes;
                foreach (XmlNode fnode in hpnodes)
                {
                    HitPoint nhp = new HitPoint();
                    nhp.X = Convert.ToInt32(fnode.Attributes["X"].Value);
                    nhp.Y = Convert.ToInt32(fnode.Attributes["Y"].Value);
                    nhp.flags = Convert.ToInt32(fnode.Attributes["Flags"].Value);
                    f.hitPoints.Add(nhp);
                }
                frames.Add(f);
            }

            //citeste animation frames
            XmlNodeList animFramesNodes = xdoc.GetElementsByTagName("AnimationFrame");

            nodes = xdoc.GetElementsByTagName("Animation");
            idx = 0;
            foreach (XmlNode node in nodes)
            {
                Animation a = new Animation();
                a.index = Convert.ToInt32(idx);
                a.ID = Convert.ToInt32(idx);
                idx++;
                a.flags = Convert.ToInt32(node.Attributes["Flags"].Value);
                a.name = node.Attributes["ID"].Value;
                XmlNodeList anodes = node.SelectNodes("AFrame");

                int localIdx = 0;
                foreach (XmlNode anode in anodes)
                {
                    AnimFrame af = new AnimFrame();
                    int afidx = Convert.ToInt32(anode.Attributes["Idx"].Value);

                    af.index = localIdx++;
                    af.ID = afidx;
                    af.frameID = Convert.ToInt32(animFramesNodes[afidx].Attributes["FrameIdx"].Value);
                    af.time = Convert.ToInt32(animFramesNodes[afidx].Attributes["Duration"].Value);
                    af.ox = Convert.ToInt32(animFramesNodes[afidx].Attributes["DX"].Value);
                    af.oy = Convert.ToInt32(animFramesNodes[afidx].Attributes["DY"].Value);
                    af.strFlags = animFramesNodes[afidx].Attributes["Flags"].Value.ToString();
                    af.frame = frames[af.frameID]; //GetFrameByID(af.frameID);
                    a.aframes.Add(af);
                }
                anims.Add(a);
            }

            xdoc = null;
        }

        public void SaveCHeader(string path)
        {
            string sifPath = Path.GetFileNameWithoutExtension(path);
            string exportFolder = Path.GetDirectoryName(path);

            string flname = sifPath.Trim().Replace(' ', '_') + "_SPR";

            if (File.Exists(flname))
                File.Delete(flname);

            StreamWriter animstr = new StreamWriter(exportFolder + "\\" + flname + ".h");
            int count = 0;
            anims.Sort(Sprite.CompareEntities);

            animstr.WriteLine("\n//Animation indexes (Idx)");
            foreach (Animation anm in anims)
            {
                string line;
                if ((anm.name.Length == 0) || (anm.name == null))
                {
                    line = "ANM_" + flname.ToUpper() + "_ANIMATION_" + count;
                }
                else
                {
                    line = anm.name.ToUpper();
                    line = line.Trim();
                    line = line.Replace(' ', '_');
                    line = "ANM_" + flname.ToUpper() + "_" + line;
                }
                animstr.WriteLine("#define \t" + line + "   " + count);
                count++;
            }

            animstr.WriteLine("\n//Animation names (ID)");
            //scrie si numele
            foreach (Animation anm in anims)
            {
                string line;
                if ((anm.name.Length == 0) || (anm.name == null))
                {
                    line = "ANMID_" + flname.ToUpper() + "_ANIMATION_" + count;
                }
                else
                {
                    line = anm.name.ToUpper();
                    line = line.Trim();
                    line = line.Replace(' ', '_');
                    line = "ANMID_" + flname.ToUpper() + "_" + line;
                }
                animstr.WriteLine("#define \t" + line + "   \"" + anm.name.ToUpper().Trim().Replace(' ', '_') + "\"");
                count++;
            }
            
            animstr.Flush();
            animstr.Close();
        }
        
        public void Save(string path)
        {
            //presortare
            anims.Sort(Sprite.CompareEntities);
            frames.Sort(Sprite.CompareEntities);
            modules.Sort(Sprite.CompareEntities);
            // saves header
            SaveCHeader(path);

            //TODO: try-catch
            XmlTextWriter xw = new XmlTextWriter(path, null);
            xw.Formatting = Formatting.Indented;
            xw.WriteStartDocument();
            xw.WriteStartElement("SpriteCollection");
            xw.WriteStartAttribute("Version");
            xw.WriteValue("2.0");
            xw.WriteEndAttribute();
            //write accompanying JSON name
            xw.WriteStartAttribute("AtlasJSON");
            xw.WriteValue(strAtlasName);
            xw.WriteEndAttribute();

            WriteFontData(xw, fontData);

            //sa mai fac Images/Modules etc?
            foreach(string txt in imgPath)
                xw.WriteElementString("Image", txt);

            xw.WriteStartElement("Modules");
            foreach (Module mod in modules)
            {
                xw.WriteStartElement("Module");

                xw.WriteStartAttribute("ImageIdx"); xw.WriteValue(mod.imageID.ToString()); xw.WriteEndAttribute();
                xw.WriteStartAttribute("X"); xw.WriteValue(mod.x.ToString()); xw.WriteEndAttribute();
                xw.WriteStartAttribute("Y"); xw.WriteValue(mod.y.ToString()); xw.WriteEndAttribute();
                xw.WriteStartAttribute("W"); xw.WriteValue(mod.w.ToString()); xw.WriteEndAttribute();
                xw.WriteStartAttribute("H"); xw.WriteValue(mod.h.ToString()); xw.WriteEndAttribute();
                xw.WriteStartAttribute("strName"); xw.WriteValue(mod.strName); xw.WriteEndAttribute();

                xw.WriteEndElement();
            }
            xw.WriteEndElement();

            xw.WriteStartElement("FrameModules");
            ///--- write fmodules ---
            List<FrameModule> tempFmodules = new List<FrameModule>();
            //count fmodules
            foreach (Frame frm in frames)
            {
                frm.fmodules.Sort(Sprite.CompareEntities);
                foreach (FrameModule fmd in frm.fmodules)
                {
                    tempFmodules.Add(fmd);
                }
            }

            foreach (Frame frm in frames)
            {
                frm.fmodules.Sort(Sprite.CompareEntities);
                foreach (FrameModule fmd in frm.fmodules)
                {
                    Int32 idx = modules.IndexOf(fmd.module);

                    xw.WriteStartElement("FrameModule");
                    xw.WriteStartAttribute("ModuleIdx"); xw.WriteValue(idx.ToString()); xw.WriteEndAttribute();
                    xw.WriteStartAttribute("OX"); xw.WriteValue(fmd.ox.ToString()); xw.WriteEndAttribute();
                    xw.WriteStartAttribute("OY"); xw.WriteValue(fmd.oy.ToString()); xw.WriteEndAttribute();
                    xw.WriteStartAttribute("Flags"); xw.WriteValue(fmd.flags.ToString()); xw.WriteEndAttribute();
                    xw.WriteEndElement();
                }
            }
            xw.WriteEndElement();

            
            ///--- animFrames ---
            List<AnimFrame> tempAframes = new List<AnimFrame>();
            //count aframes
            foreach (Animation anm in anims)
            {
                anm.aframes.Sort(Sprite.CompareEntities);
                foreach (AnimFrame afm in anm.aframes)
                {
                    tempAframes.Add(afm);
                }
            }

            //frames
            xw.WriteStartElement("Frames");
            foreach (Frame f in frames)
            {
                xw.WriteStartElement("Frame");

                xw.WriteStartAttribute("BBoxX"); xw.WriteValue(f.BBox.X.ToString()); xw.WriteEndAttribute();
                xw.WriteStartAttribute("BBoxY"); xw.WriteValue(f.BBox.Y.ToString()); xw.WriteEndAttribute();
                xw.WriteStartAttribute("BBoxW"); xw.WriteValue(f.BBox.Width.ToString()); xw.WriteEndAttribute();
                xw.WriteStartAttribute("BBoxH"); xw.WriteValue(f.BBox.Height.ToString()); xw.WriteEndAttribute();

                f.fmodules.Sort(Sprite.CompareEntities);

                xw.WriteStartElement("FModules");
                foreach (FrameModule fm in f.fmodules)
                {
                    Int32 idx = tempFmodules.IndexOf(fm);
                    xw.WriteStartElement("FModule");

                    xw.WriteStartAttribute("Idx"); xw.WriteValue(idx.ToString()); xw.WriteEndAttribute();

                    xw.WriteEndElement();
                }
                xw.WriteEndElement();

                xw.WriteStartElement("Points");
                foreach (HitPoint hp in f.hitPoints)
                {
                    xw.WriteStartElement("Point");
                    xw.WriteStartAttribute("X"); xw.WriteValue(hp.X.ToString()); xw.WriteEndAttribute();
                    xw.WriteStartAttribute("Y"); xw.WriteValue(hp.Y.ToString()); xw.WriteEndAttribute();
                    xw.WriteStartAttribute("Flags"); xw.WriteValue(hp.flags.ToString()); xw.WriteEndAttribute();
                    xw.WriteEndElement();
                }
                xw.WriteEndElement();
                //end frame
                xw.WriteEndElement();
            }
            xw.WriteEndElement();

            ///--- animation frames ---
            xw.WriteStartElement("AnimationFrames");
            foreach (Animation anm in anims)
            {
                anm.aframes.Sort(Sprite.CompareEntities);
                int relMovex = 0, relMovey = 0;

                foreach (AnimFrame afm in anm.aframes)
                {
                    Int32 idx = frames.IndexOf(afm.frame);

                    xw.WriteStartElement("AnimationFrame");
                    xw.WriteStartAttribute("FrameIdx"); xw.WriteValue(idx.ToString()); xw.WriteEndAttribute();
                    xw.WriteStartAttribute("Duration"); xw.WriteValue(afm.time.ToString()); xw.WriteEndAttribute();
                    //scrie miscarile relative fata de frame-ul anterior
                    int dx, dy;
                    dx = afm.ox;// -relMovex;
                    dy = afm.oy;// -relMovey;
                    xw.WriteStartAttribute("DX"); xw.WriteValue(dx.ToString()); xw.WriteEndAttribute();
                    xw.WriteStartAttribute("DY"); xw.WriteValue(dy.ToString()); xw.WriteEndAttribute();
                    relMovex = afm.ox;
                    relMovey = afm.oy;
                    //scrie flags 
                    xw.WriteStartAttribute("Flags"); xw.WriteValue(afm.strFlags.ToString()); xw.WriteEndAttribute();
                    xw.WriteEndElement();
                }
            }
            xw.WriteEndElement();
            

            tempFmodules.Clear();
            tempFmodules = null;


            ///--- animations ---
            xw.WriteStartElement("Animations");
            foreach (Animation anm in anims)
            {
                xw.WriteStartElement("Animation");
                //write name
                xw.WriteStartAttribute("ID"); xw.WriteValue(anm.name.ToUpper()); xw.WriteEndAttribute();
                //write flags
                xw.WriteStartAttribute("Flags"); xw.WriteValue(anm.flags.ToString()); xw.WriteEndAttribute();
                //write aframes
                foreach (AnimFrame afm in anm.aframes)
                {
                    Int32 idx = tempAframes.IndexOf(afm);
                    xw.WriteStartElement("AFrame");
                    xw.WriteStartAttribute("Idx"); xw.WriteValue(idx.ToString()); xw.WriteEndAttribute();
                    xw.WriteEndElement();
                }
                xw.WriteEndElement();
            }
            xw.WriteEndElement();

            tempAframes.Clear();
            tempAframes = null;

            xw.WriteEndElement();
            xw.WriteEndDocument();
            xw.Flush();
            xw.Close();
        }

        public void WriteVariableFormat(BinaryWriter bw, UInt16 h_b1, UInt16 h_b2, UInt16 SizeMask, Int32 value, bool signed)
        {
            if (((h_b1 & SizeMask) == 0) && ((h_b2 & SizeMask) != 0)) //daca e pe 1byte
            {
                byte val = (byte)value;
                bw.Write(val);
            }
            else if (((h_b1 & SizeMask) != 0) && ((h_b2 & SizeMask) == 0)) //daca e pe 2bytes
            {
                if (signed)
                {
                    Int16 val = (Int16)value;
                    bw.Write(val);
                }
                else
                {
                    UInt16 val = (UInt16)value;
                    bw.Write(val);
                }
            }
            else if (((h_b1 & SizeMask) != 0) && ((h_b2 & SizeMask) != 0)) //daca e pe 4bytes
            {
                if (signed)
                {
                    Int32 val = (Int32)value;
                    bw.Write(val);
                }
                else
                {
                    UInt32 val = (UInt32)value;
                    bw.Write(val);
                }
            }
            else
            {
                MessageBox.Show("WriteVariableFormat -> Illegal parameter combination !");
            }
        }

        
        public void Export(string path, bool definitionsFileInCFormat, byte h_ver, UInt16 h_a, UInt16 h_b1, UInt16 h_b2)
        {
            Stream str = File.OpenWrite(path);
            BinaryWriter bw = new BinaryWriter(str);
            string sifPath = Path.GetFileNameWithoutExtension(path);
            string exportFolder = Path.GetDirectoryName(path);

            //////////////////anim consts/////////////////
            if(!definitionsFileInCFormat)
            {
                string flname = sifPath.Trim().Replace(' ', '_') + "_SPR";
                StreamWriter animstr = new StreamWriter(exportFolder + "\\" + flname + ".java");
                anims.Sort(Sprite.CompareEntities);
                animstr.WriteLine("interface "+flname+" {");
                int count = 0;
                foreach (Animation anm in anims)
                {
                    string line;
                    if ((anm.name.Length == 0) || (anm.name == null))
                    {
                        line = "ANM_"+flname.ToUpper()+"_ANIMATION_" + count;
                    }
                    else
                    {
                        line = anm.name.ToUpper();
                        line = line.Trim();
                        line = line.Replace(' ', '_');
                        line = "ANM_" + flname.ToUpper() + "_" + line;
                    }
                    animstr.WriteLine("\tpublic static final int " + line + " = " + count + ";");
                    count++;
                }
                animstr.WriteLine("}\n\n");
                animstr.Flush();
                animstr.Close();
            }
            else
            {
                string flname = sifPath.Trim().Replace(' ', '_') + "_SPR";
                StreamWriter animstr = new StreamWriter(exportFolder + "\\" + flname + ".h");
                int count = 0;
                anims.Sort(Sprite.CompareEntities);
                foreach (Animation anm in anims)
                {
                    string line;
                    if ((anm.name.Length == 0) || (anm.name == null))
                    {
                        line = "ANM_" + flname.ToUpper() + "_ANIMATION_" + count;
                    }
                    else
                    {
                        line = anm.name.ToUpper();
                        line = line.Trim();
                        line = line.Replace(' ', '_');
                        line = "ANM_" + flname.ToUpper() + "_" + line;
                    }
                    animstr.WriteLine("#define \t" + line + "   " + count);
                    count++;
                }
                animstr.Flush();
                animstr.Close();
            }
            //--- scriere fisier ---
            byte OutByte;
            ushort OutUShort;

            //versiune 1ub
            OutByte = h_ver;
            bw.Write(OutByte);
            //flags 2b + 4b
            bw.Write(h_a);
            bw.Write(h_b1);
            bw.Write(h_b2);
            // images
            //numar imagini (1b)
            OutByte = (byte)images.Count;
            bw.Write(OutByte);
            if ((h_a & MASK_EXPORT_IMAGENAMES) != 0)
            {
                //nume imagini
                for (int i = 0; i < images.Count; i++)
                {
                    bw.Write(imgPath[i]);
                    //flaguri procesare imagine onload
                    //OutByte = (byte)((CImage)images[i]).m_flips;
                    //bw.Write(OutByte);
                }
            }

            anims.Sort(Sprite.CompareEntities);
            frames.Sort(Sprite.CompareEntities);
            modules.Sort(Sprite.CompareEntities);

            //write modules
            if ((h_a & MASK_EXPORT_MODULES) != 0)
            {
                //numar module (2b)
                OutUShort = (UInt16)modules.Count;
                bw.Write(OutUShort);
                //module data
                foreach (Module mod in modules)
                {
                    if ((h_a & MASK_EXPORT_MODULE_IMAGEIDX) != 0)
                    {
                        OutByte = (byte)mod.imageID;
                        bw.Write(OutByte);
                    }
                    //module x,y,w,h
                    WriteVariableFormat(bw, h_b1, h_b2, MASK_EXPORTSZ_MODULEXYWH, mod.x, true);
                    WriteVariableFormat(bw, h_b1, h_b2, MASK_EXPORTSZ_MODULEXYWH, mod.y, true);
                    WriteVariableFormat(bw, h_b1, h_b2, MASK_EXPORTSZ_MODULEXYWH, mod.w, false);
                    WriteVariableFormat(bw, h_b1, h_b2, MASK_EXPORTSZ_MODULEXYWH, mod.h, false);
                }
            }
            //write fmodules
            List<FrameModule> tempFmodules = new List<FrameModule>();
            //count fmodules
            foreach (Frame frm in frames)
            {
                frm.fmodules.Sort(Sprite.CompareEntities);
                foreach (FrameModule fmd in frm.fmodules)
                {
                    tempFmodules.Add(fmd);
                }
            }
            //write count (2ub)
            if ((h_a & MASK_EXPORT_FRAMEMODULES) != 0)
            {
                OutUShort = (UInt16)tempFmodules.Count;
                bw.Write(OutUShort);

                foreach (Frame frm in frames)
                {
                    frm.fmodules.Sort(Sprite.CompareEntities);
                    foreach (FrameModule fmd in frm.fmodules)
                    {
                        Int32 idx = modules.IndexOf(fmd.module);
                        WriteVariableFormat(bw, h_b1, h_b2, MASK_EXPORTSZ_FMODULE_MODULEINDEX, idx, false);
                        WriteVariableFormat(bw, h_b1, h_b2, MASK_EXPORTSZ_FMODULE_OFFSETS, fmd.ox, true);
                        WriteVariableFormat(bw, h_b1, h_b2, MASK_EXPORTSZ_FMODULE_OFFSETS, fmd.oy, true);
                        if ((h_a & MASK_EXPORT_FRAMEMODULES_FLAGS) != 0)
                        {
                            WriteVariableFormat(bw, h_b1, h_b2, MASK_EXPORTSZ_FMODULE_FLAGS, fmd.flags, false);
                        }
                    }
                }
            }
            //write frames
            if ((h_a & MASK_EXPORT_FRAMES) != 0)
            {
                //write count
                OutUShort = (UInt16)frames.Count;
                bw.Write(OutUShort);

                foreach (Frame frm in frames)
                {
                    //frame fmodules count (1ub)
                    OutByte = (byte)frm.fmodules.Count;
                    bw.Write(OutByte);

                    frm.fmodules.Sort(Sprite.CompareEntities);
                    foreach (FrameModule fmd in frm.fmodules)
                    {
                        Int32 idx = tempFmodules.IndexOf(fmd);
                        //fmodule IDX (*ub)
                        WriteVariableFormat(bw, h_b1, h_b2, MASK_EXPORTSZ_FRAME_FMODULEINDEX, idx, false);
                    }
                    //daca se exporta bbox
                    if ((h_a & MASK_EXPORT_FRAMEBBOX) != 0)
                    {
                        WriteVariableFormat(bw, h_b1, h_b2, MASK_EXPORTSZ_FRAME_BBOX, frm.BBox.X, true);
                        WriteVariableFormat(bw, h_b1, h_b2, MASK_EXPORTSZ_FRAME_BBOX, frm.BBox.Y, true);
                        WriteVariableFormat(bw, h_b1, h_b2, MASK_EXPORTSZ_FRAME_BBOX, frm.BBox.Width, false);
                        WriteVariableFormat(bw, h_b1, h_b2, MASK_EXPORTSZ_FRAME_BBOX, frm.BBox.Height, false);
                    }
                    //daca exporta hitpts
                    if ((h_a & MASK_EXPORT_FRAMEHITPOINTS) != 0)
                    {
                        //scrie nr pe un octet
                        OutByte = (byte)frm.hitPoints.Count;
                        bw.Write(OutByte);
                        for (int kk = 0; kk < frm.hitPoints.Count; kk++)
                        {
                            WriteVariableFormat(bw, h_b1, h_b2, MASK_EXPORTSZ_FRAME_HITPOINTS_XY, frm.hitPoints[kk].X, true);
                            WriteVariableFormat(bw, h_b1, h_b2, MASK_EXPORTSZ_FRAME_HITPOINTS_XY, frm.hitPoints[kk].Y, true);
                            if ((h_a & MASK_EXPORT_FRAMEHITPOINT_FLAGS) != 0)
                            {
                                WriteVariableFormat(bw, h_b1, h_b2, MASK_EXPORTSZ_FRAME_HITPOINTS_FLAGS, frm.hitPoints[kk].flags, false);
                            }
                        }
                    }
                }
            }
            tempFmodules.Clear();
            tempFmodules = null;

            //write animFrames
            List<AnimFrame> tempAframes = new List<AnimFrame>();
            //count aframes
            foreach (Animation anm in anims)
            {
                anm.aframes.Sort(Sprite.CompareEntities);
                foreach (AnimFrame afm in anm.aframes)
                {
                    tempAframes.Add(afm);
                }
            }


            if ((h_a & MASK_EXPORT_AFRAMES) != 0)
            {
                //scrie nr aframes (2ub)
                OutUShort = (UInt16)tempAframes.Count;
                bw.Write(OutUShort);

                foreach (Animation anm in anims)
                {
                    anm.aframes.Sort(Sprite.CompareEntities);
                    int relMovex = 0, relMovey = 0;
                    foreach (AnimFrame afm in anm.aframes)
                    {
                        Int32 idx = frames.IndexOf(afm.frame);

                        //frame IDX
                        WriteVariableFormat(bw, h_b1, h_b2, MASK_EXPORTSZ_AFRAME_FRAMEINDEX, idx, false);
                        //daca nu exporta durata aframe-0ului se considera 1 la toate
                        if ((h_a & MASK_EXPORT_AFRAMES_DURATION) != 0)
                        {
                            WriteVariableFormat(bw, h_b1, h_b2, MASK_EXPORTSZ_AFRAME_FRAMEDURATION, afm.time, false);
                        }
                        //scrie miscarile frameului
                        if ((h_a & MASK_EXPORT_AFRAMES_OFFSETS) != 0)
                        {                        
                            //scrie miscarile relative fata de frame-ul anterior
                            WriteVariableFormat(bw, h_b1, h_b2, MASK_EXPORTSZ_AFRAME_OFFSETS, afm.ox - relMovex, true);
                            WriteVariableFormat(bw, h_b1, h_b2, MASK_EXPORTSZ_AFRAME_OFFSETS, afm.oy - relMovey, true);
                            relMovex = afm.ox;
                            relMovey = afm.oy;
                        }
                        //scrie flags daca e selectat
                        //if ((h_a & MASK_EXPORT_AFRAMES_FLAGS) != 0)
                        //{
                        //    WriteVariableFormat(bw, h_b1, h_b2, MASK_EXPORTSZ_AFRAME_FLAGS, afm.strFlags, false);
                        //}
                    }
                }
                
            }
            
            //write animations
            if ((h_a & MASK_EXPORT_ANIMATIONS) != 0)
            {
                //count (2ub)
                OutUShort = (UInt16)anims.Count;
                bw.Write(OutUShort);
                foreach (Animation anm in anims)
                {
                    //write flags
                    WriteVariableFormat(bw, h_b1, h_b2, MASK_EXPORTSZ_ANIMATION_FLAGS, anm.flags, false);
                    //write aframes count (1ub)
                    OutByte = (byte)anm.aframes.Count;
                    bw.Write(OutByte);
                    foreach (AnimFrame afm in anm.aframes)
                    {
                        Int32 idx = tempAframes.IndexOf(afm);
                        WriteVariableFormat(bw, h_b1, h_b2, MASK_EXPORTSZ_ANIMATION_AFRAMEINDEX, idx, false);
                    }
                }
            }
            tempAframes.Clear();
            tempAframes = null;

            bw.Flush();
            str.Flush();
            bw.Close();
            str.Close();
        }
    }
}
