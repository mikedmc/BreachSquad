using System;

namespace HexxEditor
{
    public class CTile
    {
        public UInt16[] tlXY; //2 bytes: X:Y; 0xffff - not set - represents position in tileset, in tile coordinates

        public bool IsEmpty()
        {
            for (int kk = 0; kk < (int)ELayer.LAYERS_CNT; kk++)
            {
                if (tlXY[kk] != 0xffff)
                    return false;
            }
            return true;
        }

        public CTile()
        {
            tlXY = new UInt16[(int)ELayer.LAYERS_CNT];
            for (int kk = 0; kk < (int)ELayer.LAYERS_CNT; kk++)
            {
                tlXY[kk] = 0xffff; // empty tile
            }
        }
    }
}
