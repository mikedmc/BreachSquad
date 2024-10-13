using Newtonsoft.Json.Linq;
using System;
using System.Collections.Generic;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace InkEditor
{
    class AsepriteTag
    {
        public string name;
        public Int32 framesCount;
    }

    class AsepriteLayer
    {
        public string name;
        public Int32 opacity;
    }

    public class AsepriteFrame
    {
        public string strLayerName;
        public string strTagName;
        public Int32 tagFrame;
        public Rectangle moduleRect;
        public Int32 posX;
        public Int32 posY;
        public Int32 duration;
        // index of unique module in unique modules list
        public Int32 uniqueModuleIndex;

        public AsepriteFrame()
        {
            strLayerName = "";
            strTagName = "";
            tagFrame = -1;
            
            moduleRect = new Rectangle();
            posX = 0;
            posY = 0;
            duration = 0;

            uniqueModuleIndex = -1;
        }
    }

    class AsepriteImport
    {
        // global list for loaded json frames. Only used when importing from json
        public List<AsepriteFrame> arrFrames = new List<AsepriteFrame>();
        // list of layers in JSON in paint order (0 less visible to Count most on top)
        public List<AsepriteLayer> arrLayers = new List<AsepriteLayer>();
        // list of tags containing frame references
        public Dictionary<string, AsepriteTag> mapTags = new Dictionary<string, AsepriteTag>();
        // list of unique modules so we can have frame indexes to them
        public List<Rectangle> arrUniqueModules = new List<Rectangle>();

        // public name of the loaded json
        public string strJSONfilename = "";
        // image name and path #TODO: could load many images too
        public string strImgName = "";
        public string strImgPath = "";

        // fills the arrUniqueModules with unique modules
        void FindUniqueModulesFromFrames()
        {
            for (int ll = 0; ll < arrFrames.Count(); ll++)
            {
                int modidx = arrUniqueModules.IndexOf(arrFrames[ll].moduleRect);
                if (modidx < 0)
                {
                    arrUniqueModules.Add(arrFrames[ll].moduleRect);
                    modidx = arrUniqueModules.Count() - 1;
                }
                arrFrames[ll].uniqueModuleIndex = modidx;
            }
        }

        // Returns number of frames for specified tag counting them in the frames
        int GetMaxFrameForTag(string tagName)
        {
            int nMaxFrame = -1;
            foreach (var frame in arrFrames)
            {
                if ((frame.strTagName == tagName) && (frame.tagFrame > nMaxFrame))
                {
                    nMaxFrame = frame.tagFrame;
                }
            }
            // zero based frames so increase max frame with 1
            return nMaxFrame + 1;
        }

        AsepriteFrame GetFrame(string tagName, string layerName, int tagFrame)
        {
            int retidx = arrFrames.FindIndex(x => x.strLayerName == layerName && x.strTagName == tagName && x.tagFrame == tagFrame);
            return retidx < 0 ? null : arrFrames[retidx];
        }

        /*
         * \brief returns all frames (by layer) in layer paint order, with null where layer doesn't have frame
         */
        public AsepriteFrame[] GetTagFrameLayers(string tagName, int frame)
        {
            try
            {
                int nFramesCnt = mapTags[tagName].framesCount;
                if ((nFramesCnt <= 0) || (frame < 0) || (frame >= nFramesCnt))
                    return null;
                // fill the return array with all visible frames, in paint order (layer order)
                AsepriteFrame[] retArr = new AsepriteFrame[arrLayers.Count()];
                for (int ll = 0; ll < arrLayers.Count(); ll++)
                {
                    //#WARNING: array will contain null values where we don't have frames on a layer
                    retArr[ll] = GetFrame(tagName, arrLayers[ll].name, frame);
                }

                return retArr;
            }
            catch (Exception ex)
            {
                MessageBox.Show("GetTagFrame failed: " + ex.Message);
                return null;
            }
        }

        // Imports modules from JSON exported by Aseprite
        // should export frames as Array and item filename as {frame}:{layer}
        // \returns TRUE on success
        public bool ImportModulesFromJSON(string strJSONpath)
        {
            bool bRotationNotAllowed = false;

            // clear imported frames list
            arrFrames.Clear();
            arrLayers.Clear();
            mapTags.Clear();
            strJSONfilename = "";

            // load json data
            try
            {
                string jsontext = File.ReadAllText(strJSONpath);
                JObject rss = JObject.Parse(jsontext);

                ///--- PARSE META ---
                // get image paths
                strImgName = (string)rss["meta"]["image"];
                strImgPath = Path.GetDirectoryName(strJSONpath) + "\\" + strImgName;
                // get layers
                int layersCnt = rss["meta"]["layers"].Count();
                for (int ll = 0; ll < layersCnt; ll++)
                {
                    AsepriteLayer nl = new AsepriteLayer();
                    nl.name = (string)rss["meta"]["layers"][ll]["name"];
                    nl.opacity = (int)rss["meta"]["layers"][ll]["opacity"];
                    arrLayers.Add(nl);
                }
                // get tags
                int tagsCnt = rss["meta"]["frameTags"].Count();
                for (int ll = 0; ll < tagsCnt; ll++)
                {
                    AsepriteTag nt = new AsepriteTag();
                    nt.name = (string)rss["meta"]["frameTags"][ll]["name"];
                    nt.framesCount = 0;
                    // write in map
                    mapTags[nt.name] = nt;
                }
                
                ///--- PARSE FRAMES ---
                int framesCnt = rss["frames"].Count();
                for (int ll = 0; ll < framesCnt; ll++)
                {
                    AsepriteFrame nf = new AsepriteFrame();
                    var frame = rss["frames"][ll];
                    ///--- get important data from frame filename ---
                    string strName = (string)frame["filename"];
                    // unpack custom frame filename
                    // export as: {frame}:{layer}:{tag}:{tagframe}
                    string[] elements = strName.Split(':');
                    // globalFrame = elements[0];
                    nf.strLayerName = elements[1];
                    nf.strTagName = elements[2];
                    nf.tagFrame = Int32.Parse(elements[3]);

                    ///--- read module rect ---
                    nf.moduleRect.X = (int)(frame["frame"]["x"]);
                    nf.moduleRect.Y = (int)(frame["frame"]["y"]);
                    nf.moduleRect.Width = (int)(frame["frame"]["w"]);
                    nf.moduleRect.Height = (int)(frame["frame"]["h"]);

                    ///--- read position in frame ---
                    nf.posX = (int)(frame["spriteSourceSize"]["x"]);
                    nf.posY = (int)(frame["spriteSourceSize"]["y"]);

                    ///--- read duration for processing later ---
                    nf.duration = (int)(frame["duration"]);

                    // rotation not supported yet so make sure
                    if((bool)(frame["rotated"]) == true)
                        bRotationNotAllowed = true;

                    arrFrames.Add(nf);
                }

            }
            catch (Exception ex)
            {
                MessageBox.Show("JSON File failed to open!\n" + ex.Message, "ERROR !", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return false;
            }

            // do some preprocessing
            foreach (var tag in mapTags)
            {
                tag.Value.framesCount = GetMaxFrameForTag(tag.Value.name);
            }
            FindUniqueModulesFromFrames();

            //keep loaded filename for export
            strJSONfilename = strJSONpath;


            if (bRotationNotAllowed)
            {
                MessageBox.Show("Rotated sprites are not supported yet. They are loaded but output might be broken! Export the source JSON without 'allow rotation'", "Warning!", MessageBoxButtons.OK, MessageBoxIcon.Information);
            }

            return true;
        }
    }
}
