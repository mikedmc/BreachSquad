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
            this.but_ComputeFlags = new System.Windows.Forms.Button();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.tb_areaTags = new System.Windows.Forms.TextBox();
            this.tb_areaName = new System.Windows.Forms.TextBox();
            this.label4 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.but_DelArea = new System.Windows.Forms.Button();
            this.butCloneArea = new System.Windows.Forms.Button();
            this.butSaveArea = new System.Windows.Forms.Button();
            this.tabGenerator = new System.Windows.Forms.TabPage();
            this.but_GenFromStory = new System.Windows.Forms.Button();
            this.generations = new System.Windows.Forms.Label();
            this.numGenerations = new System.Windows.Forms.NumericUpDown();
            this.but_GenCorridors = new System.Windows.Forms.Button();
            this.tabStory = new System.Windows.Forms.TabPage();
            this.wa_butDelEntry = new System.Windows.Forms.Button();
            this.wa_butAddArea = new System.Windows.Forms.Button();
            this.but_generateFromStory = new System.Windows.Forms.Button();
            this.but_addStoryGen = new System.Windows.Forms.Button();
            this.groupBox_storyarea = new System.Windows.Forms.GroupBox();
            this.wa_tbTags = new System.Windows.Forms.TextBox();
            this.wa_numChildren = new System.Windows.Forms.NumericUpDown();
            this.label6 = new System.Windows.Forms.Label();
            this.label5 = new System.Windows.Forms.Label();
            this.label3 = new System.Windows.Forms.Label();
            this.menuStrip1 = new System.Windows.Forms.MenuStrip();
            this.fileToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.loadAreasToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.saveAreasToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripMenuItem1 = new System.Windows.Forms.ToolStripSeparator();
            this.loadStoryToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.saveStoryToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.but_GenCorridorsNeeded = new System.Windows.Forms.Button();
            this.but_storyComputeIDs = new System.Windows.Forms.Button();
            this.chk_ShowIDs = new System.Windows.Forms.CheckBox();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).BeginInit();
            this.tabControl1.SuspendLayout();
            this.tabAreas.SuspendLayout();
            this.groupBox1.SuspendLayout();
            this.tabGenerator.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.numGenerations)).BeginInit();
            this.tabStory.SuspendLayout();
            this.groupBox_storyarea.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.wa_numChildren)).BeginInit();
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
            this.butClear.Location = new System.Drawing.Point(6, 352);
            this.butClear.Name = "butClear";
            this.butClear.Size = new System.Drawing.Size(203, 25);
            this.butClear.TabIndex = 1;
            this.butClear.Text = "Clear Area";
            this.butClear.UseVisualStyleBackColor = true;
            this.butClear.Click += new System.EventHandler(this.butClear_Click);
            // 
            // butGenerate
            // 
            this.butGenerate.Location = new System.Drawing.Point(8, 501);
            this.butGenerate.Name = "butGenerate";
            this.butGenerate.Size = new System.Drawing.Size(201, 25);
            this.butGenerate.TabIndex = 2;
            this.butGenerate.Text = "Generate Randomly";
            this.butGenerate.UseVisualStyleBackColor = true;
            this.butGenerate.Click += new System.EventHandler(this.butGenerate_Click);
            // 
            // lbAreas
            // 
            this.lbAreas.FormattingEnabled = true;
            this.lbAreas.Location = new System.Drawing.Point(6, 6);
            this.lbAreas.Name = "lbAreas";
            this.lbAreas.Size = new System.Drawing.Size(203, 277);
            this.lbAreas.TabIndex = 5;
            this.lbAreas.SelectedIndexChanged += new System.EventHandler(this.lbAreas_SelectedIndexChanged);
            this.lbAreas.MouseDoubleClick += new System.Windows.Forms.MouseEventHandler(this.lbAreas_MouseDoubleClick);
            // 
            // tabControl1
            // 
            this.tabControl1.Controls.Add(this.tabAreas);
            this.tabControl1.Controls.Add(this.tabGenerator);
            this.tabControl1.Controls.Add(this.tabStory);
            this.tabControl1.Location = new System.Drawing.Point(780, 27);
            this.tabControl1.Name = "tabControl1";
            this.tabControl1.SelectedIndex = 0;
            this.tabControl1.Size = new System.Drawing.Size(223, 585);
            this.tabControl1.TabIndex = 6;
            this.tabControl1.SelectedIndexChanged += new System.EventHandler(this.tabControl1_SelectedIndexChanged);
            // 
            // tabAreas
            // 
            this.tabAreas.Controls.Add(this.but_ComputeFlags);
            this.tabAreas.Controls.Add(this.groupBox1);
            this.tabAreas.Controls.Add(this.but_DelArea);
            this.tabAreas.Controls.Add(this.butCloneArea);
            this.tabAreas.Controls.Add(this.butSaveArea);
            this.tabAreas.Controls.Add(this.lbAreas);
            this.tabAreas.Controls.Add(this.butClear);
            this.tabAreas.Location = new System.Drawing.Point(4, 22);
            this.tabAreas.Name = "tabAreas";
            this.tabAreas.Padding = new System.Windows.Forms.Padding(3);
            this.tabAreas.Size = new System.Drawing.Size(215, 559);
            this.tabAreas.TabIndex = 0;
            this.tabAreas.Text = "Areas";
            this.tabAreas.UseVisualStyleBackColor = true;
            // 
            // but_ComputeFlags
            // 
            this.but_ComputeFlags.Location = new System.Drawing.Point(108, 325);
            this.but_ComputeFlags.Name = "but_ComputeFlags";
            this.but_ComputeFlags.Size = new System.Drawing.Size(101, 25);
            this.but_ComputeFlags.TabIndex = 10;
            this.but_ComputeFlags.Text = "Compute Flags";
            this.but_ComputeFlags.UseVisualStyleBackColor = true;
            this.but_ComputeFlags.Click += new System.EventHandler(this.but_ComputeFlags_Click);
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.tb_areaTags);
            this.groupBox1.Controls.Add(this.tb_areaName);
            this.groupBox1.Controls.Add(this.label4);
            this.groupBox1.Controls.Add(this.label2);
            this.groupBox1.Location = new System.Drawing.Point(6, 445);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(200, 108);
            this.groupBox1.TabIndex = 10;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "Area Properties";
            // 
            // tb_areaTags
            // 
            this.tb_areaTags.Location = new System.Drawing.Point(9, 71);
            this.tb_areaTags.Name = "tb_areaTags";
            this.tb_areaTags.Size = new System.Drawing.Size(185, 20);
            this.tb_areaTags.TabIndex = 3;
            this.tb_areaTags.TextChanged += new System.EventHandler(this.tb_areaTags_TextChanged);
            // 
            // tb_areaName
            // 
            this.tb_areaName.Location = new System.Drawing.Point(9, 32);
            this.tb_areaName.Name = "tb_areaName";
            this.tb_areaName.Size = new System.Drawing.Size(185, 20);
            this.tb_areaName.TabIndex = 3;
            this.tb_areaName.TextChanged += new System.EventHandler(this.tb_areaName_TextChanged);
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(6, 55);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(52, 13);
            this.label4.TabIndex = 4;
            this.label4.Text = "Area tags";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(6, 16);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(58, 13);
            this.label2.TabIndex = 4;
            this.label2.Text = "Area name";
            // 
            // but_DelArea
            // 
            this.but_DelArea.Location = new System.Drawing.Point(6, 325);
            this.but_DelArea.Name = "but_DelArea";
            this.but_DelArea.Size = new System.Drawing.Size(101, 25);
            this.but_DelArea.TabIndex = 7;
            this.but_DelArea.Text = "Delete Area";
            this.but_DelArea.UseVisualStyleBackColor = true;
            this.but_DelArea.Click += new System.EventHandler(this.but_DelArea_Click);
            // 
            // butCloneArea
            // 
            this.butCloneArea.Location = new System.Drawing.Point(108, 297);
            this.butCloneArea.Name = "butCloneArea";
            this.butCloneArea.Size = new System.Drawing.Size(101, 25);
            this.butCloneArea.TabIndex = 5;
            this.butCloneArea.Text = "Clone Area";
            this.butCloneArea.UseVisualStyleBackColor = true;
            this.butCloneArea.Click += new System.EventHandler(this.butCloneArea_Click);
            // 
            // butSaveArea
            // 
            this.butSaveArea.Location = new System.Drawing.Point(6, 297);
            this.butSaveArea.Name = "butSaveArea";
            this.butSaveArea.Size = new System.Drawing.Size(101, 25);
            this.butSaveArea.TabIndex = 2;
            this.butSaveArea.Text = "Add New Area";
            this.butSaveArea.UseVisualStyleBackColor = true;
            this.butSaveArea.Click += new System.EventHandler(this.butSaveArea_Click);
            // 
            // tabGenerator
            // 
            this.tabGenerator.Controls.Add(this.but_GenFromStory);
            this.tabGenerator.Controls.Add(this.generations);
            this.tabGenerator.Controls.Add(this.numGenerations);
            this.tabGenerator.Controls.Add(this.but_GenCorridorsNeeded);
            this.tabGenerator.Controls.Add(this.but_GenCorridors);
            this.tabGenerator.Controls.Add(this.butGenerate);
            this.tabGenerator.Location = new System.Drawing.Point(4, 22);
            this.tabGenerator.Name = "tabGenerator";
            this.tabGenerator.Padding = new System.Windows.Forms.Padding(3);
            this.tabGenerator.Size = new System.Drawing.Size(215, 559);
            this.tabGenerator.TabIndex = 1;
            this.tabGenerator.Text = "Generator";
            this.tabGenerator.UseVisualStyleBackColor = true;
            // 
            // but_GenFromStory
            // 
            this.but_GenFromStory.Location = new System.Drawing.Point(8, 6);
            this.but_GenFromStory.Name = "but_GenFromStory";
            this.but_GenFromStory.Size = new System.Drawing.Size(201, 23);
            this.but_GenFromStory.TabIndex = 5;
            this.but_GenFromStory.Text = "Generate from Story";
            this.but_GenFromStory.UseVisualStyleBackColor = true;
            this.but_GenFromStory.Click += new System.EventHandler(this.but_GenFromStory_Click);
            // 
            // generations
            // 
            this.generations.AutoSize = true;
            this.generations.Location = new System.Drawing.Point(8, 534);
            this.generations.Name = "generations";
            this.generations.Size = new System.Drawing.Size(62, 13);
            this.generations.TabIndex = 4;
            this.generations.Text = "generations";
            // 
            // numGenerations
            // 
            this.numGenerations.Location = new System.Drawing.Point(135, 532);
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
            // but_GenCorridors
            // 
            this.but_GenCorridors.Location = new System.Drawing.Point(8, 470);
            this.but_GenCorridors.Name = "but_GenCorridors";
            this.but_GenCorridors.Size = new System.Drawing.Size(201, 25);
            this.but_GenCorridors.TabIndex = 2;
            this.but_GenCorridors.Text = "Generate with corridors";
            this.but_GenCorridors.UseVisualStyleBackColor = true;
            this.but_GenCorridors.Click += new System.EventHandler(this.but_GenCorridors_Click);
            // 
            // tabStory
            // 
            this.tabStory.Controls.Add(this.chk_ShowIDs);
            this.tabStory.Controls.Add(this.wa_butDelEntry);
            this.tabStory.Controls.Add(this.wa_butAddArea);
            this.tabStory.Controls.Add(this.but_generateFromStory);
            this.tabStory.Controls.Add(this.but_storyComputeIDs);
            this.tabStory.Controls.Add(this.but_addStoryGen);
            this.tabStory.Controls.Add(this.groupBox_storyarea);
            this.tabStory.Controls.Add(this.label3);
            this.tabStory.Location = new System.Drawing.Point(4, 22);
            this.tabStory.Name = "tabStory";
            this.tabStory.Size = new System.Drawing.Size(215, 559);
            this.tabStory.TabIndex = 2;
            this.tabStory.Text = "Story";
            this.tabStory.UseVisualStyleBackColor = true;
            // 
            // wa_butDelEntry
            // 
            this.wa_butDelEntry.Location = new System.Drawing.Point(109, 49);
            this.wa_butDelEntry.Name = "wa_butDelEntry";
            this.wa_butDelEntry.Size = new System.Drawing.Size(103, 23);
            this.wa_butDelEntry.TabIndex = 7;
            this.wa_butDelEntry.Text = "Delete Entry";
            this.wa_butDelEntry.UseVisualStyleBackColor = true;
            this.wa_butDelEntry.Click += new System.EventHandler(this.wa_butDelEntry_Click);
            // 
            // wa_butAddArea
            // 
            this.wa_butAddArea.Location = new System.Drawing.Point(3, 49);
            this.wa_butAddArea.Name = "wa_butAddArea";
            this.wa_butAddArea.Size = new System.Drawing.Size(103, 23);
            this.wa_butAddArea.TabIndex = 7;
            this.wa_butAddArea.Text = "Add Entry";
            this.wa_butAddArea.UseVisualStyleBackColor = true;
            this.wa_butAddArea.Click += new System.EventHandler(this.wa_butAddArea_Click);
            // 
            // but_generateFromStory
            // 
            this.but_generateFromStory.Location = new System.Drawing.Point(3, 533);
            this.but_generateFromStory.Name = "but_generateFromStory";
            this.but_generateFromStory.Size = new System.Drawing.Size(103, 23);
            this.but_generateFromStory.TabIndex = 4;
            this.but_generateFromStory.Text = "Sort Visually";
            this.but_generateFromStory.UseVisualStyleBackColor = true;
            this.but_generateFromStory.Click += new System.EventHandler(this.but_generateFromStory_Click);
            // 
            // but_addStoryGen
            // 
            this.but_addStoryGen.Location = new System.Drawing.Point(3, 20);
            this.but_addStoryGen.Name = "but_addStoryGen";
            this.but_addStoryGen.Size = new System.Drawing.Size(103, 23);
            this.but_addStoryGen.TabIndex = 3;
            this.but_addStoryGen.Text = "Add Generation";
            this.but_addStoryGen.UseVisualStyleBackColor = true;
            this.but_addStoryGen.Click += new System.EventHandler(this.but_addStoryGen_Click);
            // 
            // groupBox_storyarea
            // 
            this.groupBox_storyarea.Controls.Add(this.wa_tbTags);
            this.groupBox_storyarea.Controls.Add(this.wa_numChildren);
            this.groupBox_storyarea.Controls.Add(this.label6);
            this.groupBox_storyarea.Controls.Add(this.label5);
            this.groupBox_storyarea.Location = new System.Drawing.Point(3, 78);
            this.groupBox_storyarea.Name = "groupBox_storyarea";
            this.groupBox_storyarea.Size = new System.Drawing.Size(209, 84);
            this.groupBox_storyarea.TabIndex = 2;
            this.groupBox_storyarea.TabStop = false;
            this.groupBox_storyarea.Text = "Entry Properties";
            // 
            // wa_tbTags
            // 
            this.wa_tbTags.Location = new System.Drawing.Point(74, 50);
            this.wa_tbTags.Name = "wa_tbTags";
            this.wa_tbTags.Size = new System.Drawing.Size(120, 20);
            this.wa_tbTags.TabIndex = 6;
            this.wa_tbTags.TextChanged += new System.EventHandler(this.wa_tbTags_TextChanged);
            // 
            // wa_numChildren
            // 
            this.wa_numChildren.Location = new System.Drawing.Point(74, 23);
            this.wa_numChildren.Maximum = new decimal(new int[] {
            5,
            0,
            0,
            0});
            this.wa_numChildren.Name = "wa_numChildren";
            this.wa_numChildren.Size = new System.Drawing.Size(120, 20);
            this.wa_numChildren.TabIndex = 5;
            this.wa_numChildren.ValueChanged += new System.EventHandler(this.wa_numChildren_ValueChanged);
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Location = new System.Drawing.Point(41, 53);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(27, 13);
            this.label6.TabIndex = 3;
            this.label6.Text = "tags";
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Location = new System.Drawing.Point(24, 25);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(44, 13);
            this.label5.TabIndex = 2;
            this.label5.Text = "children";
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(3, 4);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(91, 13);
            this.label3.TabIndex = 1;
            this.label3.Text = "Story Generations";
            // 
            // menuStrip1
            // 
            this.menuStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.fileToolStripMenuItem});
            this.menuStrip1.Location = new System.Drawing.Point(0, 0);
            this.menuStrip1.Name = "menuStrip1";
            this.menuStrip1.Size = new System.Drawing.Size(1012, 24);
            this.menuStrip1.TabIndex = 8;
            this.menuStrip1.Text = "menuStrip1";
            // 
            // fileToolStripMenuItem
            // 
            this.fileToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.loadAreasToolStripMenuItem,
            this.saveAreasToolStripMenuItem,
            this.toolStripMenuItem1,
            this.loadStoryToolStripMenuItem,
            this.saveStoryToolStripMenuItem});
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
            // toolStripMenuItem1
            // 
            this.toolStripMenuItem1.Name = "toolStripMenuItem1";
            this.toolStripMenuItem1.Size = new System.Drawing.Size(138, 6);
            // 
            // loadStoryToolStripMenuItem
            // 
            this.loadStoryToolStripMenuItem.Name = "loadStoryToolStripMenuItem";
            this.loadStoryToolStripMenuItem.Size = new System.Drawing.Size(141, 22);
            this.loadStoryToolStripMenuItem.Text = "Load Story...";
            // 
            // saveStoryToolStripMenuItem
            // 
            this.saveStoryToolStripMenuItem.Name = "saveStoryToolStripMenuItem";
            this.saveStoryToolStripMenuItem.Size = new System.Drawing.Size(141, 22);
            this.saveStoryToolStripMenuItem.Text = "Save Story...";
            this.saveStoryToolStripMenuItem.Click += new System.EventHandler(this.saveStoryToolStripMenuItem_Click);
            // 
            // but_GenCorridorsNeeded
            // 
            this.but_GenCorridorsNeeded.Location = new System.Drawing.Point(8, 439);
            this.but_GenCorridorsNeeded.Name = "but_GenCorridorsNeeded";
            this.but_GenCorridorsNeeded.Size = new System.Drawing.Size(201, 25);
            this.but_GenCorridorsNeeded.TabIndex = 2;
            this.but_GenCorridorsNeeded.Text = "Generate corridors when needed";
            this.but_GenCorridorsNeeded.UseVisualStyleBackColor = true;
            this.but_GenCorridorsNeeded.Click += new System.EventHandler(this.but_GenCorridorsNeeded_Click);
            // 
            // but_storyComputeIDs
            // 
            this.but_storyComputeIDs.Location = new System.Drawing.Point(109, 533);
            this.but_storyComputeIDs.Name = "but_storyComputeIDs";
            this.but_storyComputeIDs.Size = new System.Drawing.Size(103, 23);
            this.but_storyComputeIDs.TabIndex = 3;
            this.but_storyComputeIDs.Text = "Compute IDs";
            this.but_storyComputeIDs.UseVisualStyleBackColor = true;
            this.but_storyComputeIDs.Click += new System.EventHandler(this.but_storyComputeIDs_Click);
            // 
            // chk_ShowIDs
            // 
            this.chk_ShowIDs.AutoSize = true;
            this.chk_ShowIDs.Location = new System.Drawing.Point(3, 510);
            this.chk_ShowIDs.Name = "chk_ShowIDs";
            this.chk_ShowIDs.Size = new System.Drawing.Size(72, 17);
            this.chk_ShowIDs.TabIndex = 8;
            this.chk_ShowIDs.Text = "Show IDs";
            this.chk_ShowIDs.UseVisualStyleBackColor = true;
            this.chk_ShowIDs.CheckedChanged += new System.EventHandler(this.chk_ShowIDs_CheckedChanged);
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(1012, 624);
            this.Controls.Add(this.tabControl1);
            this.Controls.Add(this.pictureBox1);
            this.Controls.Add(this.menuStrip1);
            this.MainMenuStrip = this.menuStrip1;
            this.Name = "Form1";
            this.Text = "Random Level Generator";
            this.KeyDown += new System.Windows.Forms.KeyEventHandler(this.Form1_KeyDown);
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).EndInit();
            this.tabControl1.ResumeLayout(false);
            this.tabAreas.ResumeLayout(false);
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            this.tabGenerator.ResumeLayout(false);
            this.tabGenerator.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.numGenerations)).EndInit();
            this.tabStory.ResumeLayout(false);
            this.tabStory.PerformLayout();
            this.groupBox_storyarea.ResumeLayout(false);
            this.groupBox_storyarea.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.wa_numChildren)).EndInit();
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
        private System.Windows.Forms.Button but_ComputeFlags;
        private System.Windows.Forms.Label generations;
        private System.Windows.Forms.NumericUpDown numGenerations;
        private System.Windows.Forms.Button butCloneArea;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.TabPage tabStory;
        private System.Windows.Forms.GroupBox groupBox_storyarea;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.TextBox wa_tbTags;
        private System.Windows.Forms.NumericUpDown wa_numChildren;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.Button but_addStoryGen;
        private System.Windows.Forms.Button wa_butAddArea;
        private System.Windows.Forms.Button wa_butDelEntry;
        private System.Windows.Forms.ToolStripSeparator toolStripMenuItem1;
        private System.Windows.Forms.ToolStripMenuItem loadStoryToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem saveStoryToolStripMenuItem;
        private System.Windows.Forms.Button but_generateFromStory;
        private System.Windows.Forms.Button but_GenFromStory;
        private System.Windows.Forms.TextBox tb_areaTags;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.Button but_GenCorridors;
        private System.Windows.Forms.Button but_GenCorridorsNeeded;
        private System.Windows.Forms.Button but_storyComputeIDs;
        private System.Windows.Forms.CheckBox chk_ShowIDs;
    }
}

