using System;
using System.IO;
using System.Collections;

namespace HexxEditor
{

    public partial class EditorWnd
    {
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


    }
}
