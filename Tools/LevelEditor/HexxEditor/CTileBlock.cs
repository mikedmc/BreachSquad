using System.Drawing;
using System.Windows.Forms;

namespace HexxEditor
{
    public class CTileBlock
    {
        public const int BLOCK_W = 8;
        public const int BLOCK_H = 8;

        public Image layerImg; //image that contains bottom layers
        public Graphics gr; //graphics to image
        public Image layerImgCeil; //image that contains ceiling objects
        public Graphics grCeil; //graphics to image

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

            gr = null; grCeil = null;
            layerImg = null; layerImgCeil = null;
            if (nTileSize > 0)
            {
                layerImg = new Bitmap(nTileSize * BLOCK_W, nTileSize * BLOCK_H, System.Drawing.Imaging.PixelFormat.Format32bppArgb);
                gr = Graphics.FromImage(layerImg);
                gr.Clear(Color.Transparent);

                layerImgCeil = new Bitmap(nTileSize * BLOCK_W, nTileSize * BLOCK_H, System.Drawing.Imaging.PixelFormat.Format32bppArgb);
                grCeil = Graphics.FromImage(layerImgCeil);
                grCeil.Clear(Color.Transparent);
            }
            else
            {
                MessageBox.Show("Error!", "Could not create Tile Block image! Tile size is 0!", MessageBoxButtons.OK);
            }

            bHasUndo = false;
        }

        // returns true if it  has walkable tiles on specified extremity
        public bool HasConnectionOnSide(EDir eDirection)
        {
            //#TODO: sa ia in considerare toate layerele de floor nu doar primul (pentru cand ai tranzitii)
            switch (eDirection)
            {
                case EDir.K_DIR_UP:
                    for (int kk = 0; kk < BLOCK_W; kk++)
                    {
                        if (tiles[kk, 0].tlXY[(int)ELayer.FLOOR] != 0xffff)
                            return true;
                    }
                    break;
                case EDir.K_DIR_DOWN:
                    for (int kk = 0; kk < BLOCK_W; kk++)
                    {
                        if (tiles[kk, BLOCK_H - 1].tlXY[(int)ELayer.FLOOR] != 0xffff)
                            return true;
                    }
                    break;
                case EDir.K_DIR_LEFT:
                    for (int kk = 0; kk < BLOCK_W; kk++)
                    {
                        if (tiles[0, kk].tlXY[(int)ELayer.FLOOR] != 0xffff)
                            return true;
                    }
                    break;
                case EDir.K_DIR_RIGHT:
                    for (int kk = 0; kk < BLOCK_W; kk++)
                    {
                        if (tiles[BLOCK_W - 1, kk].tlXY[(int)ELayer.FLOOR] != 0xffff)
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
                            tiles_undo[kk, ll].tlXY[mm] = tiles[kk, ll].tlXY[mm];
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
                            tiles[kk, ll].tlXY[mm] = tiles_undo[kk, ll].tlXY[mm];
                        }
                    }
                }
            }
        }
    }


}
