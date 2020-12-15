namespace HexxEditor
{
    partial class MaterialsWnd
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
            this.splitContainer1 = new System.Windows.Forms.SplitContainer();
            this.pbTileset = new System.Windows.Forms.PictureBox();
            this.menuStrip1 = new System.Windows.Forms.MenuStrip();
            this.fileToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.loadTilesetToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.chk_Invert = new System.Windows.Forms.CheckBox();
            this.chk_Grid = new System.Windows.Forms.CheckBox();
            this.but_resetView = new System.Windows.Forms.Button();
            ((System.ComponentModel.ISupportInitialize)(this.splitContainer1)).BeginInit();
            this.splitContainer1.Panel1.SuspendLayout();
            this.splitContainer1.Panel2.SuspendLayout();
            this.splitContainer1.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.pbTileset)).BeginInit();
            this.menuStrip1.SuspendLayout();
            this.SuspendLayout();
            // 
            // splitContainer1
            // 
            this.splitContainer1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.splitContainer1.FixedPanel = System.Windows.Forms.FixedPanel.Panel2;
            this.splitContainer1.IsSplitterFixed = true;
            this.splitContainer1.Location = new System.Drawing.Point(0, 0);
            this.splitContainer1.Name = "splitContainer1";
            this.splitContainer1.Orientation = System.Windows.Forms.Orientation.Horizontal;
            // 
            // splitContainer1.Panel1
            // 
            this.splitContainer1.Panel1.Controls.Add(this.pbTileset);
            this.splitContainer1.Panel1.Controls.Add(this.menuStrip1);
            // 
            // splitContainer1.Panel2
            // 
            this.splitContainer1.Panel2.Controls.Add(this.but_resetView);
            this.splitContainer1.Panel2.Controls.Add(this.chk_Invert);
            this.splitContainer1.Panel2.Controls.Add(this.chk_Grid);
            this.splitContainer1.Size = new System.Drawing.Size(447, 429);
            this.splitContainer1.SplitterDistance = 345;
            this.splitContainer1.TabIndex = 0;
            // 
            // pbTileset
            // 
            this.pbTileset.BackColor = System.Drawing.SystemColors.AppWorkspace;
            this.pbTileset.Dock = System.Windows.Forms.DockStyle.Fill;
            this.pbTileset.Location = new System.Drawing.Point(0, 24);
            this.pbTileset.Name = "pbTileset";
            this.pbTileset.Size = new System.Drawing.Size(447, 321);
            this.pbTileset.SizeMode = System.Windows.Forms.PictureBoxSizeMode.AutoSize;
            this.pbTileset.TabIndex = 0;
            this.pbTileset.TabStop = false;
            this.pbTileset.MouseDown += new System.Windows.Forms.MouseEventHandler(this.pbTileset_MouseDown);
            this.pbTileset.MouseMove += new System.Windows.Forms.MouseEventHandler(this.pbTileset_MouseMove);
            this.pbTileset.MouseUp += new System.Windows.Forms.MouseEventHandler(this.pbTileset_MouseUp);
            this.pbTileset.MouseWheel += new System.Windows.Forms.MouseEventHandler(this.pbTileset_MouseWheel);
            this.pbTileset.Resize += new System.EventHandler(this.pbTileset_Resize);
            // 
            // menuStrip1
            // 
            this.menuStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.fileToolStripMenuItem});
            this.menuStrip1.Location = new System.Drawing.Point(0, 0);
            this.menuStrip1.Name = "menuStrip1";
            this.menuStrip1.Size = new System.Drawing.Size(447, 24);
            this.menuStrip1.TabIndex = 1;
            this.menuStrip1.Text = "menuStrip1";
            // 
            // fileToolStripMenuItem
            // 
            this.fileToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.loadTilesetToolStripMenuItem});
            this.fileToolStripMenuItem.Name = "fileToolStripMenuItem";
            this.fileToolStripMenuItem.Size = new System.Drawing.Size(37, 20);
            this.fileToolStripMenuItem.Text = "File";
            // 
            // loadTilesetToolStripMenuItem
            // 
            this.loadTilesetToolStripMenuItem.Name = "loadTilesetToolStripMenuItem";
            this.loadTilesetToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
            this.loadTilesetToolStripMenuItem.Text = "Load Tileset...";
            this.loadTilesetToolStripMenuItem.Click += new System.EventHandler(this.loadTilesetToolStripMenuItem_Click);
            // 
            // chk_Invert
            // 
            this.chk_Invert.AutoSize = true;
            this.chk_Invert.Location = new System.Drawing.Point(3, 26);
            this.chk_Invert.Name = "chk_Invert";
            this.chk_Invert.Size = new System.Drawing.Size(114, 17);
            this.chk_Invert.TabIndex = 1;
            this.chk_Invert.Text = "Invert Background";
            this.chk_Invert.UseVisualStyleBackColor = true;
            this.chk_Invert.CheckedChanged += new System.EventHandler(this.chk_Invert_CheckedChanged);
            // 
            // chk_Grid
            // 
            this.chk_Grid.AutoSize = true;
            this.chk_Grid.Location = new System.Drawing.Point(3, 3);
            this.chk_Grid.Name = "chk_Grid";
            this.chk_Grid.Size = new System.Drawing.Size(75, 17);
            this.chk_Grid.TabIndex = 0;
            this.chk_Grid.Text = "Show Grid";
            this.chk_Grid.UseVisualStyleBackColor = true;
            this.chk_Grid.CheckedChanged += new System.EventHandler(this.chk_Grid_CheckedChanged);
            // 
            // but_resetView
            // 
            this.but_resetView.Location = new System.Drawing.Point(3, 49);
            this.but_resetView.Name = "but_resetView";
            this.but_resetView.Size = new System.Drawing.Size(75, 23);
            this.but_resetView.TabIndex = 2;
            this.but_resetView.Text = "Reset View";
            this.but_resetView.UseVisualStyleBackColor = true;
            this.but_resetView.Click += new System.EventHandler(this.but_resetView_Click);
            // 
            // MaterialsWnd
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(447, 429);
            this.Controls.Add(this.splitContainer1);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.SizableToolWindow;
            this.MainMenuStrip = this.menuStrip1;
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "MaterialsWnd";
            this.ShowIcon = false;
            this.Text = "Materials";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.MaterialsWnd_FormClosing);
            this.MouseWheel += new System.Windows.Forms.MouseEventHandler(this.pbTileset_MouseWheel);
            this.splitContainer1.Panel1.ResumeLayout(false);
            this.splitContainer1.Panel1.PerformLayout();
            this.splitContainer1.Panel2.ResumeLayout(false);
            this.splitContainer1.Panel2.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.splitContainer1)).EndInit();
            this.splitContainer1.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.pbTileset)).EndInit();
            this.menuStrip1.ResumeLayout(false);
            this.menuStrip1.PerformLayout();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.SplitContainer splitContainer1;
        private System.Windows.Forms.PictureBox pbTileset;
        private System.Windows.Forms.MenuStrip menuStrip1;
        private System.Windows.Forms.ToolStripMenuItem fileToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem loadTilesetToolStripMenuItem;
        private System.Windows.Forms.CheckBox chk_Grid;
        private System.Windows.Forms.CheckBox chk_Invert;
        private System.Windows.Forms.Button but_resetView;
    }
}