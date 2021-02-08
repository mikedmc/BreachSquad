namespace circleEnvelope
{
    partial class Form1
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
            this.pictureBox1 = new System.Windows.Forms.PictureBox();
            this.butClear = new System.Windows.Forms.Button();
            this.butGenerate = new System.Windows.Forms.Button();
            this.lbAreas = new System.Windows.Forms.ListBox();
            this.tabControl1 = new System.Windows.Forms.TabControl();
            this.tabAreas = new System.Windows.Forms.TabPage();
            this.label2 = new System.Windows.Forms.Label();
            this.tb_areaName = new System.Windows.Forms.TextBox();
            this.butSaveArea = new System.Windows.Forms.Button();
            this.tabGenerator = new System.Windows.Forms.TabPage();
            this.generations = new System.Windows.Forms.Label();
            this.numGenerations = new System.Windows.Forms.NumericUpDown();
            this.but_DelArea = new System.Windows.Forms.Button();
            this.menuStrip1 = new System.Windows.Forms.MenuStrip();
            this.fileToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.loadAreasToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.saveAreasToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.label1 = new System.Windows.Forms.Label();
            this.but_ComputeFlags = new System.Windows.Forms.Button();
            this.butCloneArea = new System.Windows.Forms.Button();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).BeginInit();
            this.tabControl1.SuspendLayout();
            this.tabAreas.SuspendLayout();
            this.tabGenerator.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.numGenerations)).BeginInit();
            this.menuStrip1.SuspendLayout();
            this.SuspendLayout();
            // 
            // pictureBox1
            // 
            this.pictureBox1.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.pictureBox1.Location = new System.Drawing.Point(12, 27);
            this.pictureBox1.Name = "pictureBox1";
            this.pictureBox1.Size = new System.Drawing.Size(762, 585);
            this.pictureBox1.TabIndex = 0;
            this.pictureBox1.TabStop = false;
            this.pictureBox1.MouseDown += new System.Windows.Forms.MouseEventHandler(this.pictureBox1_MouseDown);
            this.pictureBox1.MouseMove += new System.Windows.Forms.MouseEventHandler(this.pictureBox1_MouseMove);
            this.pictureBox1.MouseUp += new System.Windows.Forms.MouseEventHandler(this.pictureBox1_MouseUp);
            // 
            // butClear
            // 
            this.butClear.Location = new System.Drawing.Point(6, 165);
            this.butClear.Name = "butClear";
            this.butClear.Size = new System.Drawing.Size(166, 25);
            this.butClear.TabIndex = 1;
            this.butClear.Text = "Clear";
            this.butClear.UseVisualStyleBackColor = true;
            this.butClear.Click += new System.EventHandler(this.butClear_Click);
            // 
            // butGenerate
            // 
            this.butGenerate.Location = new System.Drawing.Point(6, 6);
            this.butGenerate.Name = "butGenerate";
            this.butGenerate.Size = new System.Drawing.Size(166, 25);
            this.butGenerate.TabIndex = 2;
            this.butGenerate.Text = "Generate";
            this.butGenerate.UseVisualStyleBackColor = true;
            this.butGenerate.Click += new System.EventHandler(this.butGenerate_Click);
            // 
            // lbAreas
            // 
            this.lbAreas.FormattingEnabled = true;
            this.lbAreas.Location = new System.Drawing.Point(780, 371);
            this.lbAreas.Name = "lbAreas";
            this.lbAreas.Size = new System.Drawing.Size(186, 238);
            this.lbAreas.TabIndex = 5;
            this.lbAreas.SelectedIndexChanged += new System.EventHandler(this.lbAreas_SelectedIndexChanged);
            this.lbAreas.MouseDoubleClick += new System.Windows.Forms.MouseEventHandler(this.lbAreas_MouseDoubleClick);
            // 
            // tabControl1
            // 
            this.tabControl1.Controls.Add(this.tabAreas);
            this.tabControl1.Controls.Add(this.tabGenerator);
            this.tabControl1.Location = new System.Drawing.Point(780, 27);
            this.tabControl1.Name = "tabControl1";
            this.tabControl1.SelectedIndex = 0;
            this.tabControl1.Size = new System.Drawing.Size(186, 261);
            this.tabControl1.TabIndex = 6;
            this.tabControl1.SelectedIndexChanged += new System.EventHandler(this.tabControl1_SelectedIndexChanged);
            // 
            // tabAreas
            // 
            this.tabAreas.Controls.Add(this.butCloneArea);
            this.tabAreas.Controls.Add(this.label2);
            this.tabAreas.Controls.Add(this.tb_areaName);
            this.tabAreas.Controls.Add(this.butSaveArea);
            this.tabAreas.Controls.Add(this.butClear);
            this.tabAreas.Location = new System.Drawing.Point(4, 22);
            this.tabAreas.Name = "tabAreas";
            this.tabAreas.Padding = new System.Windows.Forms.Padding(3);
            this.tabAreas.Size = new System.Drawing.Size(178, 235);
            this.tabAreas.TabIndex = 0;
            this.tabAreas.Text = "Areas";
            this.tabAreas.UseVisualStyleBackColor = true;
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(6, 193);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(58, 13);
            this.label2.TabIndex = 4;
            this.label2.Text = "Area name";
            // 
            // tb_areaName
            // 
            this.tb_areaName.Location = new System.Drawing.Point(6, 209);
            this.tb_areaName.Name = "tb_areaName";
            this.tb_areaName.Size = new System.Drawing.Size(166, 20);
            this.tb_areaName.TabIndex = 3;
            this.tb_areaName.TextChanged += new System.EventHandler(this.tb_areaName_TextChanged);
            // 
            // butSaveArea
            // 
            this.butSaveArea.Location = new System.Drawing.Point(6, 6);
            this.butSaveArea.Name = "butSaveArea";
            this.butSaveArea.Size = new System.Drawing.Size(166, 25);
            this.butSaveArea.TabIndex = 2;
            this.butSaveArea.Text = "Add New Area";
            this.butSaveArea.UseVisualStyleBackColor = true;
            this.butSaveArea.Click += new System.EventHandler(this.butSaveArea_Click);
            // 
            // tabGenerator
            // 
            this.tabGenerator.Controls.Add(this.generations);
            this.tabGenerator.Controls.Add(this.numGenerations);
            this.tabGenerator.Controls.Add(this.butGenerate);
            this.tabGenerator.Location = new System.Drawing.Point(4, 22);
            this.tabGenerator.Name = "tabGenerator";
            this.tabGenerator.Padding = new System.Windows.Forms.Padding(3);
            this.tabGenerator.Size = new System.Drawing.Size(178, 235);
            this.tabGenerator.TabIndex = 1;
            this.tabGenerator.Text = "Generator";
            this.tabGenerator.UseVisualStyleBackColor = true;
            // 
            // generations
            // 
            this.generations.AutoSize = true;
            this.generations.Location = new System.Drawing.Point(6, 39);
            this.generations.Name = "generations";
            this.generations.Size = new System.Drawing.Size(62, 13);
            this.generations.TabIndex = 4;
            this.generations.Text = "generations";
            // 
            // numGenerations
            // 
            this.numGenerations.Location = new System.Drawing.Point(98, 37);
            this.numGenerations.Minimum = new decimal(new int[] {
            1,
            0,
            0,
            0});
            this.numGenerations.Name = "numGenerations";
            this.numGenerations.Size = new System.Drawing.Size(74, 20);
            this.numGenerations.TabIndex = 3;
            this.numGenerations.Value = new decimal(new int[] {
            6,
            0,
            0,
            0});
            // 
            // but_DelArea
            // 
            this.but_DelArea.Location = new System.Drawing.Point(780, 615);
            this.but_DelArea.Name = "but_DelArea";
            this.but_DelArea.Size = new System.Drawing.Size(85, 25);
            this.but_DelArea.TabIndex = 7;
            this.but_DelArea.Text = "Delete Area";
            this.but_DelArea.UseVisualStyleBackColor = true;
            this.but_DelArea.Click += new System.EventHandler(this.but_DelArea_Click);
            // 
            // menuStrip1
            // 
            this.menuStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.fileToolStripMenuItem});
            this.menuStrip1.Location = new System.Drawing.Point(0, 0);
            this.menuStrip1.Name = "menuStrip1";
            this.menuStrip1.Size = new System.Drawing.Size(972, 24);
            this.menuStrip1.TabIndex = 8;
            this.menuStrip1.Text = "menuStrip1";
            // 
            // fileToolStripMenuItem
            // 
            this.fileToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.loadAreasToolStripMenuItem,
            this.saveAreasToolStripMenuItem});
            this.fileToolStripMenuItem.Name = "fileToolStripMenuItem";
            this.fileToolStripMenuItem.Size = new System.Drawing.Size(37, 20);
            this.fileToolStripMenuItem.Text = "File";
            // 
            // loadAreasToolStripMenuItem
            // 
            this.loadAreasToolStripMenuItem.Name = "loadAreasToolStripMenuItem";
            this.loadAreasToolStripMenuItem.Size = new System.Drawing.Size(141, 22);
            this.loadAreasToolStripMenuItem.Text = "Load Areas...";
            this.loadAreasToolStripMenuItem.Click += new System.EventHandler(this.loadAreasToolStripMenuItem_Click);
            // 
            // saveAreasToolStripMenuItem
            // 
            this.saveAreasToolStripMenuItem.Name = "saveAreasToolStripMenuItem";
            this.saveAreasToolStripMenuItem.Size = new System.Drawing.Size(141, 22);
            this.saveAreasToolStripMenuItem.Text = "Save Areas...";
            this.saveAreasToolStripMenuItem.Click += new System.EventHandler(this.saveAreasToolStripMenuItem_Click);
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(780, 355);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(34, 13);
            this.label1.TabIndex = 9;
            this.label1.Text = "Areas";
            // 
            // but_ComputeFlags
            // 
            this.but_ComputeFlags.Location = new System.Drawing.Point(882, 615);
            this.but_ComputeFlags.Name = "but_ComputeFlags";
            this.but_ComputeFlags.Size = new System.Drawing.Size(85, 25);
            this.but_ComputeFlags.TabIndex = 10;
            this.but_ComputeFlags.Text = "Compute Flags";
            this.but_ComputeFlags.UseVisualStyleBackColor = true;
            this.but_ComputeFlags.Click += new System.EventHandler(this.but_ComputeFlags_Click);
            // 
            // butCloneArea
            // 
            this.butCloneArea.Location = new System.Drawing.Point(6, 37);
            this.butCloneArea.Name = "butCloneArea";
            this.butCloneArea.Size = new System.Drawing.Size(166, 25);
            this.butCloneArea.TabIndex = 5;
            this.butCloneArea.Text = "Clone Area";
            this.butCloneArea.UseVisualStyleBackColor = true;
            this.butCloneArea.Click += new System.EventHandler(this.butCloneArea_Click);
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(972, 679);
            this.Controls.Add(this.but_ComputeFlags);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.but_DelArea);
            this.Controls.Add(this.tabControl1);
            this.Controls.Add(this.lbAreas);
            this.Controls.Add(this.pictureBox1);
            this.Controls.Add(this.menuStrip1);
            this.MainMenuStrip = this.menuStrip1;
            this.Name = "Form1";
            this.Text = "Random Level Generator";
            this.KeyDown += new System.Windows.Forms.KeyEventHandler(this.Form1_KeyDown);
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).EndInit();
            this.tabControl1.ResumeLayout(false);
            this.tabAreas.ResumeLayout(false);
            this.tabAreas.PerformLayout();
            this.tabGenerator.ResumeLayout(false);
            this.tabGenerator.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.numGenerations)).EndInit();
            this.menuStrip1.ResumeLayout(false);
            this.menuStrip1.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.PictureBox pictureBox1;
        private System.Windows.Forms.Button butClear;
        private System.Windows.Forms.Button butGenerate;
        private System.Windows.Forms.ListBox lbAreas;
        private System.Windows.Forms.TabControl tabControl1;
        private System.Windows.Forms.TabPage tabAreas;
        private System.Windows.Forms.Button butSaveArea;
        private System.Windows.Forms.TabPage tabGenerator;
        private System.Windows.Forms.Button but_DelArea;
        private System.Windows.Forms.MenuStrip menuStrip1;
        private System.Windows.Forms.ToolStripMenuItem fileToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem loadAreasToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem saveAreasToolStripMenuItem;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.TextBox tb_areaName;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Button but_ComputeFlags;
        private System.Windows.Forms.Label generations;
        private System.Windows.Forms.NumericUpDown numGenerations;
        private System.Windows.Forms.Button butCloneArea;
    }
}

