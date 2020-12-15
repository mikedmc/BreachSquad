using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Drawing;
using System.Xml;
using System.Windows.Forms;
using System.IO;

namespace HexxEditor
{
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
            //Sprite.DrawImage(g, image, x, y); //asa era inainte
            if (image != null)
            {
                g.DrawImage(image, x - 0.5f, y - 0.5f, new Rectangle(-1, -1, image.Width + 2, image.Height + 2), GraphicsUnit.Pixel);
            }
        }
    }

    public class FrameModule : Entity
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

    public class Frame : Entity
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
            fm.ID = max + 1;
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
            fmodules.Sort(SpriteLoader.CompareEntities);
        }

        public void Paint(Graphics g, int x, int y)
        {
            foreach (FrameModule fm in fmodules)
            {
                if ((fm.flags & 3) == 0)
                {
                    fm.module.Paint(g, fm.ox + x, fm.oy + y);
                }
                else if ((fm.flags & 3) == 3)
                {
                    fm.module.Paint(g, fm.ox + x, fm.oy + y, RotateFlipType.RotateNoneFlipXY);
                }
                else if ((fm.flags & 2) != 0)
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

    public class AnimFrame : Entity
    {
        public Frame frame = null;
        public int frameID = 0;
        public int ox = 0, oy = 0;
        public int time = 1;
        public int flags = 0;
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
            af.ID = max + 1;
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

    public class SpriteLoader
    {
        #region Constants

        public const byte VERSION = 0x30;
        //ce exporta
        public const UInt16 MASK_EXPORT_IMAGENAMES = 1;
        public const UInt16 MASK_EXPORT_MODULES = 1 << 1;
        public const UInt16 MASK_EXPORT_MODULE_IMAGEIDX = 1 << 2;
        public const UInt16 MASK_EXPORT_FRAMEMODULES = 1 << 3;
        public const UInt16 MASK_EXPORT_FRAMEMODULES_FLAGS = 1 << 4;
        public const UInt16 MASK_EXPORT_FRAMES = 1 << 5;
        public const UInt16 MASK_EXPORT_FRAMEBBOX = 1 << 6;
        public const UInt16 MASK_EXPORT_FRAMEHITPOINTS = 1 << 7;
        public const UInt16 MASK_EXPORT_FRAMEHITPOINT_FLAGS = 1 << 8;
        public const UInt16 MASK_EXPORT_AFRAMES = 1 << 9;
        public const UInt16 MASK_EXPORT_AFRAMES_DURATION = 1 << 10;
        public const UInt16 MASK_EXPORT_AFRAMES_OFFSETS = 1 << 11;
        public const UInt16 MASK_EXPORT_AFRAMES_FLAGS = 1 << 12;
        public const UInt16 MASK_EXPORT_ANIMATIONS = 1 << 13;
        //pe ce dimensiuni exporta
        //avem doua variabile pt marimi m_b1, m_b2; 
        //daca din m_b1|mask si m_b2|mask formam 2 biti, valoarea reprezinta numarul de bytes pe care se exporta variabila
        public const UInt16 MASK_EXPORTSZ_MODULEXYWH = 1;
        public const UInt16 MASK_EXPORTSZ_FMODULE_MODULEINDEX = 1 << 1;
        public const UInt16 MASK_EXPORTSZ_FMODULE_OFFSETS = 1 << 2;
        public const UInt16 MASK_EXPORTSZ_FMODULE_FLAGS = 1 << 3;
        public const UInt16 MASK_EXPORTSZ_FRAME_FMODULEINDEX = 1 << 4;
        public const UInt16 MASK_EXPORTSZ_FRAME_BBOX = 1 << 5;
        public const UInt16 MASK_EXPORTSZ_FRAME_HITPOINTS_XY = 1 << 6;
        public const UInt16 MASK_EXPORTSZ_FRAME_HITPOINTS_FLAGS = 1 << 7;
        public const UInt16 MASK_EXPORTSZ_AFRAME_FRAMEINDEX = 1 << 8;
        public const UInt16 MASK_EXPORTSZ_AFRAME_FRAMEDURATION = 1 << 9;
        public const UInt16 MASK_EXPORTSZ_AFRAME_OFFSETS = 1 << 10;
        public const UInt16 MASK_EXPORTSZ_AFRAME_FLAGS = 1 << 11;
        public const UInt16 MASK_EXPORTSZ_ANIMATION_AFRAMEINDEX = 1 << 12;
        public const UInt16 MASK_EXPORTSZ_ANIMATION_FLAGS = 1 << 13;

        #endregion

        public List<Image> images = new List<Image>();
        public List<string> imgPath = new List<string>();

        public List<Module> modules = new List<Module>();
        public List<Frame> frames = new List<Frame>();
        public List<Animation> anims = new List<Animation>();

        #region Modules

        public bool IsModuleCorrect(Module m)
        {
            if (m == null) return true;
            if ((m.imageID >= images.Count) || (m.imageID < 0)) return false;
            if ((m.w <= 0) || (m.h <= 0) || (m.x < 0) || (m.y < 0) || (m.w + m.x > images[m.imageID].Width) || (m.y + m.h > images[m.imageID].Height)) return false;
            return true;
        }

        public void Release()
        {
            anims.RemoveRange(0, anims.Count);
            frames.RemoveRange(0, frames.Count);
            modules.RemoveRange(0, modules.Count);
            images.RemoveRange(0, images.Count);
            imgPath.RemoveRange(0, imgPath.Count);
        }

        public Module AddModule()
        {
            Module m = new Module();
            int max = 0;
            for (int i = 0; i < modules.Count; i++)
                if (modules[i].ID > max)
                    max = modules[i].ID;
            m.ID = max + 1;
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
            foreach (Frame f in frames)
            {
                for (int i = 0; i < f.fmodules.Count; i++)
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

        #endregion

        #region Frames

        public Frame AddFrame()
        {
            Frame f = new Frame();
            int max = 0;
            foreach (Frame fi in frames)
                if (fi.ID > max)
                    max = fi.ID;
            f.ID = max + 1;
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
            foreach (Animation a in anims)
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
            a.ID = max + 1;
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
            foreach (Module mod in modules)
            {
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

        public void Open(string path)
        {
            XmlDocument xdoc = new XmlDocument();
            xdoc.Load(path);
            //verifica versiunea
            XmlNodeList ver = xdoc.GetElementsByTagName("SpriteCollection");
            if ((ver.Count < 1) || (ver[0].Attributes["Version"].Value != "2.0"))
            {
                DialogResult dr = MessageBox.Show("Wrong XML version or illegal file!\nDo you want to try to load the old format anyway?", "Open File Error!", MessageBoxButtons.YesNo);
                if (dr != DialogResult.Yes) return;
                OpenOldFormat(path);
                return;
            }

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
                modules.Add(mod);
                idx++;
            }
            MakeModuleImages();

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
                    af.flags = Convert.ToInt32(animFramesNodes[afidx].Attributes["Flags"].Value);
                    af.frame = frames[af.frameID]; //GetFrameByID(af.frameID);
                    a.aframes.Add(af);
                }
                anims.Add(a);
            }

            xdoc = null;

           // loadedFile = Path.GetFileName(path);
        }

        public void OpenOldFormat(string path)
        {
            XmlDocument xdoc = new XmlDocument();
            xdoc.Load(path);
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
            foreach (XmlNode node in nodes)
            {
                Module mod = new Module();
                int i = 0;
                mod.index = Convert.ToInt32(node.ChildNodes[i++].InnerText);
                mod.ID = Convert.ToInt32(node.ChildNodes[i++].InnerText);
                mod.imageID = Convert.ToInt32(node.ChildNodes[i++].InnerText);
                mod.x = Convert.ToInt32(node.ChildNodes[i++].InnerText);
                mod.y = Convert.ToInt32(node.ChildNodes[i++].InnerText);
                mod.w = Convert.ToInt32(node.ChildNodes[i++].InnerText);
                mod.h = Convert.ToInt32(node.ChildNodes[i++].InnerText);
                modules.Add(mod);
            }
            //creeaza imaginile pt fiecare modul
            MakeModuleImages();

            nodes = xdoc.GetElementsByTagName("Frame");
            foreach (XmlNode node in nodes)
            {
                Frame f = new Frame();
                int i = 0;
                f.index = Convert.ToInt32(node.ChildNodes[i++].InnerText);
                f.ID = Convert.ToInt32(node.ChildNodes[i++].InnerText);
                XmlNode bbnode = node.SelectSingleNode("BoundingBox");
                f.BBox.X = Convert.ToInt32(bbnode.ChildNodes[0].InnerText);
                f.BBox.Y = Convert.ToInt32(bbnode.ChildNodes[1].InnerText);
                f.BBox.Width = Convert.ToInt32(bbnode.ChildNodes[2].InnerText);
                f.BBox.Height = Convert.ToInt32(bbnode.ChildNodes[3].InnerText);
                XmlNodeList fnodes = node.SelectNodes("FrameModule");
                foreach (XmlNode fnode in fnodes)
                {
                    FrameModule fm = new FrameModule();
                    fm.index = Convert.ToInt32(fnode.SelectSingleNode("Index").InnerText);
                    fm.ID = Convert.ToInt32(fnode.SelectSingleNode("ID").InnerText);
                    fm.moduleID = Convert.ToInt32(fnode.SelectSingleNode("ModuleID").InnerText);
                    fm.ox = Convert.ToInt32(fnode.SelectSingleNode("OX").InnerText);
                    fm.oy = Convert.ToInt32(fnode.SelectSingleNode("OY").InnerText);
                    fm.flags = Convert.ToInt32(fnode.SelectSingleNode("flags").InnerText);
                    fm.module = GetModuleByID(fm.moduleID);
                    f.fmodules.Add(fm);
                }
                XmlNodeList hpnodes = node.SelectNodes("HitPoint");
                foreach (XmlNode fnode in hpnodes)
                {
                    HitPoint nhp = new HitPoint();
                    nhp.X = Convert.ToInt32(fnode.SelectSingleNode("X").InnerText);
                    nhp.Y = Convert.ToInt32(fnode.SelectSingleNode("Y").InnerText);
                    nhp.flags = Convert.ToInt32(fnode.SelectSingleNode("Flags").InnerText);
                    f.hitPoints.Add(nhp);
                }
                frames.Add(f);
            }
            nodes = xdoc.GetElementsByTagName("Animation");
            foreach (XmlNode node in nodes)
            {
                Animation a = new Animation();
                a.index = Convert.ToInt32(node.SelectSingleNode("Index").InnerText);
                a.ID = Convert.ToInt32(node.SelectSingleNode("ID").InnerText);
                a.flags = Convert.ToInt32(node.SelectSingleNode("Flags").InnerText);
                a.name = node.SelectSingleNode("Name").InnerText;
                XmlNodeList anodes = node.SelectNodes("AnimFrame");
                foreach (XmlNode anode in anodes)
                {
                    AnimFrame af = new AnimFrame();
                    af.index = Convert.ToInt32(anode.SelectSingleNode("Index").InnerText);
                    af.ID = Convert.ToInt32(anode.SelectSingleNode("ID").InnerText);
                    af.frameID = Convert.ToInt32(anode.SelectSingleNode("FrameID").InnerText);
                    af.time = Convert.ToInt32(anode.SelectSingleNode("Time").InnerText);
                    af.ox = Convert.ToInt32(anode.SelectSingleNode("MoveX").InnerText);
                    af.oy = Convert.ToInt32(anode.SelectSingleNode("MoveY").InnerText);
                    af.flags = Convert.ToInt32(anode.SelectSingleNode("Flags").InnerText);
                    af.frame = GetFrameByID(af.frameID);
                    a.aframes.Add(af);
                }
                anims.Add(a);
            }

            xdoc = null;
        }

    }
}
