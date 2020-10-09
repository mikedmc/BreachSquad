namespace HexxEditor
{
    partial class ActorsWnd
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
            this.chk_lookLeft = new System.Windows.Forms.CheckBox();
            this.menuStrip1 = new System.Windows.Forms.MenuStrip();
            this.fileToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.openActorsTemplateToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.openActorsBSXToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.chk_bCollisions = new System.Windows.Forms.CheckBox();
            this.chk_Gravity = new System.Windows.Forms.CheckBox();
            this.groupProperties = new System.Windows.Forms.GroupBox();
            this.label1 = new System.Windows.Forms.Label();
            this.comboStates = new System.Windows.Forms.ComboBox();
            this.num_Angle = new System.Windows.Forms.NumericUpDown();
            this.chk_setAngle = new System.Windows.Forms.CheckBox();
            this.chk_hideActors = new System.Windows.Forms.CheckBox();
            this.bsXbrowserCtrl1 = new BSXAnimBrowser.BSXbrowserCtrl();
            this.tableLayoutPanel1 = new System.Windows.Forms.TableLayoutPanel();
            this.panel1 = new System.Windows.Forms.Panel();
            this.lvActors = new System.Windows.Forms.ListView();
            this.panel2 = new System.Windows.Forms.Panel();
            this.menuStrip1.SuspendLayout();
            this.groupProperties.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.num_Angle)).BeginInit();
            this.tableLayoutPanel1.SuspendLayout();
            this.panel1.SuspendLayout();
            this.panel2.SuspendLayout();
            this.SuspendLayout();
            // 
            // chk_lookLeft
            // 
            this.chk_lookLeft.AutoSize = true;
            this.chk_lookLeft.Location = new System.Drawing.Point(6, 19);
            this.chk_lookLeft.Name = "chk_lookLeft";
            this.chk_lookLeft.Size = new System.Drawing.Size(49, 17);
            this.chk_lookLeft.TabIndex = 2;
            this.chk_lookLeft.Text = "FlipX";
            this.chk_lookLeft.UseVisualStyleBackColor = true;
            this.chk_lookLeft.CheckedChanged += new System.EventHandler(this.chk_lookLeft_CheckedChanged);
            // 
            // menuStrip1
            // 
            this.menuStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.fileToolStripMenuItem});
            this.menuStrip1.Location = new System.Drawing.Point(0, 0);
            this.menuStrip1.Name = "menuStrip1";
            this.menuStrip1.Size = new System.Drawing.Size(453, 24);
            this.menuStrip1.TabIndex = 4;
            this.menuStrip1.Text = "menuStrip1";
            // 
            // fileToolStripMenuItem
            // 
            this.fileToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.openActorsTemplateToolStripMenuItem,
            this.openActorsBSXToolStripMenuItem});
            this.fileToolStripMenuItem.Name = "fileToolStripMenuItem";
            this.fileToolStripMenuItem.Size = new System.Drawing.Size(37, 20);
            this.fileToolStripMenuItem.Text = "File";
            // 
            // openActorsTemplateToolStripMenuItem
            // 
            this.openActorsTemplateToolStripMenuItem.Name = "openActorsTemplateToolStripMenuItem";
            this.openActorsTemplateToolStripMenuItem.Size = new System.Drawing.Size(201, 22);
            this.openActorsTemplateToolStripMenuItem.Text = "Open Actors Template...";
            this.openActorsTemplateToolStripMenuItem.Click += new System.EventHandler(this.openActorsTemplateToolStripMenuItem_Click);
            // 
            // openActorsBSXToolStripMenuItem
            // 
            this.openActorsBSXToolStripMenuItem.Name = "openActorsBSXToolStripMenuItem";
            this.openActorsBSXToolStripMenuItem.Size = new System.Drawing.Size(201, 22);
            this.openActorsBSXToolStripMenuItem.Text = "Open Actors BSX...";
            this.openActorsBSXToolStripMenuItem.Click += new System.EventHandler(this.openActorsBSXToolStripMenuItem_Click);
            // 
            // chk_bCollisions
            // 
            this.chk_bCollisions.AutoSize = true;
            this.chk_bCollisions.Checked = true;
            this.chk_bCollisions.CheckState = System.Windows.Forms.CheckState.Checked;
            this.chk_bCollisions.Location = new System.Drawing.Point(61, 19);
            this.chk_bCollisions.Name = "chk_bCollisions";
            this.chk_bCollisions.Size = new System.Drawing.Size(86, 17);
            this.chk_bCollisions.TabIndex = 5;
            this.chk_bCollisions.Text = "hasCollisions";
            this.chk_bCollisions.UseVisualStyleBackColor = true;
            this.chk_bCollisions.CheckedChanged += new System.EventHandler(this.chk_bCollisions_CheckedChanged);
            // 
            // chk_Gravity
            // 
            this.chk_Gravity.AutoSize = true;
            this.chk_Gravity.Checked = true;
            this.chk_Gravity.CheckState = System.Windows.Forms.CheckState.Checked;
            this.chk_Gravity.Location = new System.Drawing.Point(153, 19);
            this.chk_Gravity.Name = "chk_Gravity";
            this.chk_Gravity.Size = new System.Drawing.Size(76, 17);
            this.chk_Gravity.TabIndex = 6;
            this.chk_Gravity.Text = "hasGravity";
            this.chk_Gravity.UseVisualStyleBackColor = true;
            this.chk_Gravity.CheckedChanged += new System.EventHandler(this.chk_Gravity_CheckedChanged);
            // 
            // groupProperties
            // 
            this.groupProperties.Controls.Add(this.label1);
            this.groupProperties.Controls.Add(this.comboStates);
            this.groupProperties.Controls.Add(this.num_Angle);
            this.groupProperties.Controls.Add(this.chk_setAngle);
            this.groupProperties.Controls.Add(this.chk_lookLeft);
            this.groupProperties.Controls.Add(this.chk_Gravity);
            this.groupProperties.Controls.Add(this.chk_bCollisions);
            this.groupProperties.Location = new System.Drawing.Point(9, 14);
            this.groupProperties.Name = "groupProperties";
            this.groupProperties.Size = new System.Drawing.Size(322, 100);
            this.groupProperties.TabIndex = 7;
            this.groupProperties.TabStop = false;
            this.groupProperties.Text = "Properties";
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(13, 68);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(62, 13);
            this.label1.TabIndex = 10;
            this.label1.Text = "Set AI state";
            // 
            // comboStates
            // 
            this.comboStates.FormattingEnabled = true;
            this.comboStates.Location = new System.Drawing.Point(84, 65);
            this.comboStates.Name = "comboStates";
            this.comboStates.Size = new System.Drawing.Size(232, 21);
            this.comboStates.TabIndex = 9;
            this.comboStates.SelectedIndexChanged += new System.EventHandler(this.comboStates_SelectedIndexChanged);
            // 
            // num_Angle
            // 
            this.num_Angle.Enabled = false;
            this.num_Angle.Location = new System.Drawing.Point(84, 39);
            this.num_Angle.Maximum = new decimal(new int[] {
            360,
            0,
            0,
            0});
            this.num_Angle.Name = "num_Angle";
            this.num_Angle.Size = new System.Drawing.Size(91, 20);
            this.num_Angle.TabIndex = 8;
            this.num_Angle.ValueChanged += new System.EventHandler(this.num_Angle_ValueChanged);
            // 
            // chk_setAngle
            // 
            this.chk_setAngle.AutoSize = true;
            this.chk_setAngle.Location = new System.Drawing.Point(6, 42);
            this.chk_setAngle.Name = "chk_setAngle";
            this.chk_setAngle.Size = new System.Drawing.Size(72, 17);
            this.chk_setAngle.TabIndex = 7;
            this.chk_setAngle.Text = "Set Angle";
            this.chk_setAngle.UseVisualStyleBackColor = true;
            this.chk_setAngle.CheckedChanged += new System.EventHandler(this.chk_setAngle_CheckedChanged);
            // 
            // chk_hideActors
            // 
            this.chk_hideActors.AutoSize = true;
            this.chk_hideActors.Location = new System.Drawing.Point(9, 118);
            this.chk_hideActors.Name = "chk_hideActors";
            this.chk_hideActors.Size = new System.Drawing.Size(81, 17);
            this.chk_hideActors.TabIndex = 8;
            this.chk_hideActors.Text = "Hide Actors";
            this.chk_hideActors.UseVisualStyleBackColor = true;
            this.chk_hideActors.CheckedChanged += new System.EventHandler(this.chk_hideActors_CheckedChanged);
            // 
            // bsXbrowserCtrl1
            // 
            this.bsXbrowserCtrl1.animBoxSize = 60;
            this.bsXbrowserCtrl1.AutoScroll = true;
            this.bsXbrowserCtrl1.AutoScrollMinSize = new System.Drawing.Size(0, 720);
            this.bsXbrowserCtrl1.canSelectFrames = false;
            this.bsXbrowserCtrl1.Location = new System.Drawing.Point(337, 19);
            this.bsXbrowserCtrl1.maxScaling = 2F;
            this.bsXbrowserCtrl1.Name = "bsXbrowserCtrl1";
            this.bsXbrowserCtrl1.Size = new System.Drawing.Size(100, 76);
            this.bsXbrowserCtrl1.TabIndex = 1;
            this.bsXbrowserCtrl1.thumbAnimInterval = 500;
            this.bsXbrowserCtrl1.Visible = false;
            // 
            // tableLayoutPanel1
            // 
            this.tableLayoutPanel1.ColumnCount = 1;
            this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
            this.tableLayoutPanel1.Controls.Add(this.panel1, 0, 0);
            this.tableLayoutPanel1.Controls.Add(this.panel2, 0, 1);
            this.tableLayoutPanel1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.tableLayoutPanel1.Location = new System.Drawing.Point(0, 24);
            this.tableLayoutPanel1.Name = "tableLayoutPanel1";
            this.tableLayoutPanel1.RowCount = 2;
            this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
            this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Absolute, 150F));
            this.tableLayoutPanel1.Size = new System.Drawing.Size(453, 506);
            this.tableLayoutPanel1.TabIndex = 10;
            // 
            // panel1
            // 
            this.panel1.Controls.Add(this.lvActors);
            this.panel1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.panel1.Location = new System.Drawing.Point(3, 3);
            this.panel1.Name = "panel1";
            this.panel1.Size = new System.Drawing.Size(447, 350);
            this.panel1.TabIndex = 2;
            // 
            // lvActors
            // 
            this.lvActors.BackColor = System.Drawing.SystemColors.ControlDark;
            this.lvActors.Dock = System.Windows.Forms.DockStyle.Fill;
            this.lvActors.HeaderStyle = System.Windows.Forms.ColumnHeaderStyle.None;
            this.lvActors.HideSelection = false;
            this.lvActors.Location = new System.Drawing.Point(0, 0);
            this.lvActors.MultiSelect = false;
            this.lvActors.Name = "lvActors";
            this.lvActors.ShowItemToolTips = true;
            this.lvActors.Size = new System.Drawing.Size(447, 350);
            this.lvActors.TabIndex = 9;
            this.lvActors.UseCompatibleStateImageBehavior = false;
            this.lvActors.SelectedIndexChanged += new System.EventHandler(this.lvActors_SelectedIndexChanged);
            // 
            // panel2
            // 
            this.panel2.Controls.Add(this.groupProperties);
            this.panel2.Controls.Add(this.chk_hideActors);
            this.panel2.Controls.Add(this.bsXbrowserCtrl1);
            this.panel2.Dock = System.Windows.Forms.DockStyle.Fill;
            this.panel2.Location = new System.Drawing.Point(3, 359);
            this.panel2.Name = "panel2";
            this.panel2.Size = new System.Drawing.Size(447, 144);
            this.panel2.TabIndex = 3;
            // 
            // ActorsWnd
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(453, 530);
            this.Controls.Add(this.tableLayoutPanel1);
            this.Controls.Add(this.menuStrip1);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.SizableToolWindow;
            this.MainMenuStrip = this.menuStrip1;
            this.Name = "ActorsWnd";
            this.Text = "ActorsWnd";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.Actors_FormClosing);
            this.menuStrip1.ResumeLayout(false);
            this.menuStrip1.PerformLayout();
            this.groupProperties.ResumeLayout(false);
            this.groupProperties.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.num_Angle)).EndInit();
            this.tableLayoutPanel1.ResumeLayout(false);
            this.panel1.ResumeLayout(false);
            this.panel2.ResumeLayout(false);
            this.panel2.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion
        private System.Windows.Forms.CheckBox chk_lookLeft;
        private System.Windows.Forms.MenuStrip menuStrip1;
        private System.Windows.Forms.ToolStripMenuItem fileToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem openActorsTemplateToolStripMenuItem;
        private System.Windows.Forms.CheckBox chk_bCollisions;
        private System.Windows.Forms.CheckBox chk_Gravity;
        private System.Windows.Forms.GroupBox groupProperties;
        private System.Windows.Forms.NumericUpDown num_Angle;
        private System.Windows.Forms.CheckBox chk_setAngle;
        private System.Windows.Forms.ComboBox comboStates;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.CheckBox chk_hideActors;
        private BSXAnimBrowser.BSXbrowserCtrl bsXbrowserCtrl1;
        private System.Windows.Forms.ToolStripMenuItem openActorsBSXToolStripMenuItem;
        private System.Windows.Forms.TableLayoutPanel tableLayoutPanel1;
        private System.Windows.Forms.Panel panel1;
        private System.Windows.Forms.ListView lvActors;
        private System.Windows.Forms.Panel panel2;
    }
}