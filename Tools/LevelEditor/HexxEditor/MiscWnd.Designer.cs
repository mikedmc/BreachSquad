namespace HexxEditor
{
    partial class MiscWnd
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.butTrails = new System.Windows.Forms.Button();
            this.text_MiscParams = new System.Windows.Forms.TextBox();
            this.label4 = new System.Windows.Forms.Label();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.groupBox2 = new System.Windows.Forms.GroupBox();
            this.but_windowHoriz = new System.Windows.Forms.Button();
            this.button1 = new System.Windows.Forms.Button();
            this.but_doorLocked = new System.Windows.Forms.Button();
            this.but_soloDoor = new System.Windows.Forms.Button();
            this.but_FrontStairs = new System.Windows.Forms.Button();
            this.but_FrontDoor = new System.Windows.Forms.Button();
            this.but_keycardGold = new System.Windows.Forms.Button();
            this.but_MetalDoorUnlocker = new System.Windows.Forms.Button();
            this.but_doorMetallic = new System.Windows.Forms.Button();
            this.but_doorUnlocked = new System.Windows.Forms.Button();
            this.but_Checkpoint = new System.Windows.Forms.Button();
            this.but_Sound = new System.Windows.Forms.Button();
            this.groupBox1.SuspendLayout();
            this.groupBox2.SuspendLayout();
            this.SuspendLayout();
            // 
            // butTrails
            // 
            this.butTrails.Location = new System.Drawing.Point(110, 136);
            this.butTrails.Name = "butTrails";
            this.butTrails.Size = new System.Drawing.Size(85, 38);
            this.butTrails.TabIndex = 34;
            this.butTrails.TabStop = false;
            this.butTrails.Text = "Rails";
            this.butTrails.UseVisualStyleBackColor = true;
            this.butTrails.Click += new System.EventHandler(this.butTrails_Click);
            // 
            // text_MiscParams
            // 
            this.text_MiscParams.Location = new System.Drawing.Point(6, 32);
            this.text_MiscParams.Multiline = true;
            this.text_MiscParams.Name = "text_MiscParams";
            this.text_MiscParams.ScrollBars = System.Windows.Forms.ScrollBars.Both;
            this.text_MiscParams.Size = new System.Drawing.Size(361, 74);
            this.text_MiscParams.TabIndex = 36;
            this.text_MiscParams.TextChanged += new System.EventHandler(this.text_MiscParams_TextChanged);
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(6, 16);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(210, 13);
            this.label4.TabIndex = 35;
            this.label4.Text = "params format: a=10;b=25;str=the lazy dog;";
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.text_MiscParams);
            this.groupBox1.Controls.Add(this.label4);
            this.groupBox1.Location = new System.Drawing.Point(12, 12);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(375, 118);
            this.groupBox1.TabIndex = 38;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "Misc ID:";
            // 
            // groupBox2
            // 
            this.groupBox2.Controls.Add(this.but_windowHoriz);
            this.groupBox2.Controls.Add(this.button1);
            this.groupBox2.Controls.Add(this.but_doorLocked);
            this.groupBox2.Controls.Add(this.but_soloDoor);
            this.groupBox2.Controls.Add(this.but_FrontStairs);
            this.groupBox2.Controls.Add(this.but_FrontDoor);
            this.groupBox2.Controls.Add(this.but_keycardGold);
            this.groupBox2.Controls.Add(this.but_MetalDoorUnlocker);
            this.groupBox2.Controls.Add(this.but_doorMetallic);
            this.groupBox2.Controls.Add(this.but_doorUnlocked);
            this.groupBox2.Location = new System.Drawing.Point(12, 180);
            this.groupBox2.Name = "groupBox2";
            this.groupBox2.Size = new System.Drawing.Size(375, 159);
            this.groupBox2.TabIndex = 40;
            this.groupBox2.TabStop = false;
            this.groupBox2.Text = "Macros";
            // 
            // but_windowHoriz
            // 
            this.but_windowHoriz.Location = new System.Drawing.Point(282, 63);
            this.but_windowHoriz.Name = "but_windowHoriz";
            this.but_windowHoriz.Size = new System.Drawing.Size(85, 38);
            this.but_windowHoriz.TabIndex = 35;
            this.but_windowHoriz.TabStop = false;
            this.but_windowHoriz.Text = "Window Horiz 3 Tiles";
            this.but_windowHoriz.UseVisualStyleBackColor = true;
            this.but_windowHoriz.Click += new System.EventHandler(this.but_windowHoriz_Click);
            // 
            // button1
            // 
            this.button1.Location = new System.Drawing.Point(282, 19);
            this.button1.Name = "button1";
            this.button1.Size = new System.Drawing.Size(85, 38);
            this.button1.TabIndex = 34;
            this.button1.TabStop = false;
            this.button1.Text = "Window Vertical";
            this.button1.UseVisualStyleBackColor = true;
            this.button1.Click += new System.EventHandler(this.but_windProfile_Click);
            // 
            // but_doorLocked
            // 
            this.but_doorLocked.Location = new System.Drawing.Point(9, 19);
            this.but_doorLocked.Name = "but_doorLocked";
            this.but_doorLocked.Size = new System.Drawing.Size(85, 38);
            this.but_doorLocked.TabIndex = 34;
            this.but_doorLocked.TabStop = false;
            this.but_doorLocked.Text = "Door Section Locked";
            this.but_doorLocked.UseVisualStyleBackColor = true;
            this.but_doorLocked.Click += new System.EventHandler(this.but_doorLocked_Click);
            // 
            // but_soloDoor
            // 
            this.but_soloDoor.Location = new System.Drawing.Point(100, 63);
            this.but_soloDoor.Name = "but_soloDoor";
            this.but_soloDoor.Size = new System.Drawing.Size(85, 38);
            this.but_soloDoor.TabIndex = 34;
            this.but_soloDoor.TabStop = false;
            this.but_soloDoor.Text = "Front Solo Door";
            this.but_soloDoor.UseVisualStyleBackColor = true;
            this.but_soloDoor.Click += new System.EventHandler(this.but_soloDoor_Click);
            // 
            // but_FrontStairs
            // 
            this.but_FrontStairs.Location = new System.Drawing.Point(9, 63);
            this.but_FrontStairs.Name = "but_FrontStairs";
            this.but_FrontStairs.Size = new System.Drawing.Size(85, 38);
            this.but_FrontStairs.TabIndex = 34;
            this.but_FrontStairs.TabStop = false;
            this.but_FrontStairs.Text = "Front Solo Stairs";
            this.but_FrontStairs.UseVisualStyleBackColor = true;
            this.but_FrontStairs.Click += new System.EventHandler(this.but_FrontStairs_Click);
            // 
            // but_FrontDoor
            // 
            this.but_FrontDoor.Location = new System.Drawing.Point(191, 63);
            this.but_FrontDoor.Name = "but_FrontDoor";
            this.but_FrontDoor.Size = new System.Drawing.Size(85, 38);
            this.but_FrontDoor.TabIndex = 34;
            this.but_FrontDoor.TabStop = false;
            this.but_FrontDoor.Text = "Front Team Door";
            this.but_FrontDoor.UseVisualStyleBackColor = true;
            this.but_FrontDoor.Click += new System.EventHandler(this.but_FrontDoor_Click);
            // 
            // but_keycardGold
            // 
            this.but_keycardGold.Location = new System.Drawing.Point(100, 107);
            this.but_keycardGold.Name = "but_keycardGold";
            this.but_keycardGold.Size = new System.Drawing.Size(85, 38);
            this.but_keycardGold.TabIndex = 34;
            this.but_keycardGold.TabStop = false;
            this.but_keycardGold.Text = "Keycard Gold";
            this.but_keycardGold.UseVisualStyleBackColor = true;
            this.but_keycardGold.Click += new System.EventHandler(this.but_keycardGold_Click);
            // 
            // but_MetalDoorUnlocker
            // 
            this.but_MetalDoorUnlocker.Location = new System.Drawing.Point(9, 107);
            this.but_MetalDoorUnlocker.Name = "but_MetalDoorUnlocker";
            this.but_MetalDoorUnlocker.Size = new System.Drawing.Size(85, 38);
            this.but_MetalDoorUnlocker.TabIndex = 34;
            this.but_MetalDoorUnlocker.TabStop = false;
            this.but_MetalDoorUnlocker.Text = "Keycard Red";
            this.but_MetalDoorUnlocker.UseVisualStyleBackColor = true;
            this.but_MetalDoorUnlocker.Click += new System.EventHandler(this.but_MetalDoorUnlocker_Click);
            // 
            // but_doorMetallic
            // 
            this.but_doorMetallic.Location = new System.Drawing.Point(191, 19);
            this.but_doorMetallic.Name = "but_doorMetallic";
            this.but_doorMetallic.Size = new System.Drawing.Size(85, 38);
            this.but_doorMetallic.TabIndex = 34;
            this.but_doorMetallic.TabStop = false;
            this.but_doorMetallic.Text = "Door Section Keycard";
            this.but_doorMetallic.UseVisualStyleBackColor = true;
            this.but_doorMetallic.Click += new System.EventHandler(this.but_doorMetallic_Click);
            // 
            // but_doorUnlocked
            // 
            this.but_doorUnlocked.Location = new System.Drawing.Point(100, 19);
            this.but_doorUnlocked.Name = "but_doorUnlocked";
            this.but_doorUnlocked.Size = new System.Drawing.Size(85, 38);
            this.but_doorUnlocked.TabIndex = 34;
            this.but_doorUnlocked.TabStop = false;
            this.but_doorUnlocked.Text = "Door Section Unlocked";
            this.but_doorUnlocked.UseVisualStyleBackColor = true;
            this.but_doorUnlocked.Click += new System.EventHandler(this.but_doorUnlocked_Click);
            // 
            // but_Checkpoint
            // 
            this.but_Checkpoint.Location = new System.Drawing.Point(18, 136);
            this.but_Checkpoint.Name = "but_Checkpoint";
            this.but_Checkpoint.Size = new System.Drawing.Size(85, 38);
            this.but_Checkpoint.TabIndex = 34;
            this.but_Checkpoint.TabStop = false;
            this.but_Checkpoint.Text = "Spawn Point";
            this.but_Checkpoint.UseVisualStyleBackColor = true;
            this.but_Checkpoint.Click += new System.EventHandler(this.but_Checkpoint_Click);
            // 
            // but_Sound
            // 
            this.but_Sound.Location = new System.Drawing.Point(203, 136);
            this.but_Sound.Name = "but_Sound";
            this.but_Sound.Size = new System.Drawing.Size(85, 38);
            this.but_Sound.TabIndex = 34;
            this.but_Sound.TabStop = false;
            this.but_Sound.Text = "Script";
            this.but_Sound.UseVisualStyleBackColor = true;
            this.but_Sound.Click += new System.EventHandler(this.but_Script_Click);
            // 
            // MiscWnd
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(399, 348);
            this.Controls.Add(this.groupBox2);
            this.Controls.Add(this.groupBox1);
            this.Controls.Add(this.but_Sound);
            this.Controls.Add(this.butTrails);
            this.Controls.Add(this.but_Checkpoint);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedToolWindow;
            this.Name = "MiscWnd";
            this.Text = "Misc Objects";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.MiscWnd_FormClosing);
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            this.groupBox2.ResumeLayout(false);
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.Button butTrails;
        private System.Windows.Forms.TextBox text_MiscParams;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.GroupBox groupBox2;
        private System.Windows.Forms.Button but_doorLocked;
        private System.Windows.Forms.Button but_FrontDoor;
        private System.Windows.Forms.Button but_doorMetallic;
        private System.Windows.Forms.Button but_doorUnlocked;
        private System.Windows.Forms.Button but_FrontStairs;
        private System.Windows.Forms.Button but_MetalDoorUnlocker;
        private System.Windows.Forms.Button but_Checkpoint;
        private System.Windows.Forms.Button but_Sound;
        private System.Windows.Forms.Button button1;
        private System.Windows.Forms.Button but_windowHoriz;
        private System.Windows.Forms.Button but_soloDoor;
        private System.Windows.Forms.Button but_keycardGold;
    }
}