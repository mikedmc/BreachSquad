namespace InkEd3
{
    partial class ViewOptionsWnd
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
            this.label1 = new System.Windows.Forms.Label();
            this.modNumWTreshold = new System.Windows.Forms.NumericUpDown();
            this.FramesCBShowLinkedHitpts = new System.Windows.Forms.CheckBox();
            this.AnimCBShowPath = new System.Windows.Forms.CheckBox();
            this.OptionsNumMajorGrid = new System.Windows.Forms.NumericUpDown();
            this.OptionsNumGridSize = new System.Windows.Forms.NumericUpDown();
            this.label4 = new System.Windows.Forms.Label();
            this.label3 = new System.Windows.Forms.Label();
            this.butOK = new System.Windows.Forms.Button();
            this.chk_optionsExportSmallJSON = new System.Windows.Forms.CheckBox();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            ((System.ComponentModel.ISupportInitialize)(this.modNumWTreshold)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.OptionsNumMajorGrid)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.OptionsNumGridSize)).BeginInit();
            this.groupBox1.SuspendLayout();
            this.SuspendLayout();
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(12, 14);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(80, 13);
            this.label1.TabIndex = 1;
            this.label1.Text = "Wand Treshold";
            // 
            // modNumWTreshold
            // 
            this.modNumWTreshold.Location = new System.Drawing.Point(98, 12);
            this.modNumWTreshold.Maximum = new decimal(new int[] {
            50,
            0,
            0,
            0});
            this.modNumWTreshold.Name = "modNumWTreshold";
            this.modNumWTreshold.Size = new System.Drawing.Size(68, 20);
            this.modNumWTreshold.TabIndex = 0;
            // 
            // FramesCBShowLinkedHitpts
            // 
            this.FramesCBShowLinkedHitpts.AutoSize = true;
            this.FramesCBShowLinkedHitpts.Location = new System.Drawing.Point(7, 19);
            this.FramesCBShowLinkedHitpts.Name = "FramesCBShowLinkedHitpts";
            this.FramesCBShowLinkedHitpts.Size = new System.Drawing.Size(144, 17);
            this.FramesCBShowLinkedHitpts.TabIndex = 0;
            this.FramesCBShowLinkedHitpts.Text = "Link HitPoints With Lines";
            this.FramesCBShowLinkedHitpts.UseVisualStyleBackColor = true;
            // 
            // AnimCBShowPath
            // 
            this.AnimCBShowPath.AutoSize = true;
            this.AnimCBShowPath.Location = new System.Drawing.Point(7, 42);
            this.AnimCBShowPath.Name = "AnimCBShowPath";
            this.AnimCBShowPath.Size = new System.Drawing.Size(125, 17);
            this.AnimCBShowPath.TabIndex = 0;
            this.AnimCBShowPath.Text = "Show animation path";
            this.AnimCBShowPath.UseVisualStyleBackColor = true;
            // 
            // OptionsNumMajorGrid
            // 
            this.OptionsNumMajorGrid.Location = new System.Drawing.Point(98, 60);
            this.OptionsNumMajorGrid.Minimum = new decimal(new int[] {
            2,
            0,
            0,
            0});
            this.OptionsNumMajorGrid.Name = "OptionsNumMajorGrid";
            this.OptionsNumMajorGrid.Size = new System.Drawing.Size(68, 20);
            this.OptionsNumMajorGrid.TabIndex = 3;
            this.OptionsNumMajorGrid.Value = new decimal(new int[] {
            10,
            0,
            0,
            0});
            // 
            // OptionsNumGridSize
            // 
            this.OptionsNumGridSize.Location = new System.Drawing.Point(98, 36);
            this.OptionsNumGridSize.Maximum = new decimal(new int[] {
            1000,
            0,
            0,
            0});
            this.OptionsNumGridSize.Minimum = new decimal(new int[] {
            1,
            0,
            0,
            0});
            this.OptionsNumGridSize.Name = "OptionsNumGridSize";
            this.OptionsNumGridSize.Size = new System.Drawing.Size(68, 20);
            this.OptionsNumGridSize.TabIndex = 2;
            this.OptionsNumGridSize.Value = new decimal(new int[] {
            16,
            0,
            0,
            0});
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(16, 62);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(76, 13);
            this.label4.TabIndex = 1;
            this.label4.Text = "Major Gridlines";
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(43, 38);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(49, 13);
            this.label3.TabIndex = 0;
            this.label3.Text = "Grid Size";
            // 
            // butOK
            // 
            this.butOK.Location = new System.Drawing.Point(149, 192);
            this.butOK.Name = "butOK";
            this.butOK.Size = new System.Drawing.Size(97, 28);
            this.butOK.TabIndex = 1;
            this.butOK.Text = "OK";
            this.butOK.UseVisualStyleBackColor = true;
            this.butOK.Click += new System.EventHandler(this.butOK_Click);
            // 
            // chk_optionsExportSmallJSON
            // 
            this.chk_optionsExportSmallJSON.AutoSize = true;
            this.chk_optionsExportSmallJSON.Checked = true;
            this.chk_optionsExportSmallJSON.CheckState = System.Windows.Forms.CheckState.Checked;
            this.chk_optionsExportSmallJSON.Location = new System.Drawing.Point(7, 65);
            this.chk_optionsExportSmallJSON.Name = "chk_optionsExportSmallJSON";
            this.chk_optionsExportSmallJSON.Size = new System.Drawing.Size(152, 17);
            this.chk_optionsExportSmallJSON.TabIndex = 4;
            this.chk_optionsExportSmallJSON.Text = "Export non-indented JSON";
            this.chk_optionsExportSmallJSON.UseVisualStyleBackColor = true;
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.chk_optionsExportSmallJSON);
            this.groupBox1.Controls.Add(this.AnimCBShowPath);
            this.groupBox1.Controls.Add(this.FramesCBShowLinkedHitpts);
            this.groupBox1.Location = new System.Drawing.Point(15, 86);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(388, 100);
            this.groupBox1.TabIndex = 5;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "groupBox1";
            // 
            // ViewOptionsWnd
            // 
            this.AcceptButton = this.butOK;
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(420, 230);
            this.ControlBox = false;
            this.Controls.Add(this.groupBox1);
            this.Controls.Add(this.OptionsNumMajorGrid);
            this.Controls.Add(this.OptionsNumGridSize);
            this.Controls.Add(this.label4);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.modNumWTreshold);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.butOK);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedToolWindow;
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "ViewOptionsWnd";
            this.ShowIcon = false;
            this.ShowInTaskbar = false;
            this.SizeGripStyle = System.Windows.Forms.SizeGripStyle.Hide;
            this.Text = "Options";
            this.TopMost = true;
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.ViewOptionsWnd_FormClosing);
            ((System.ComponentModel.ISupportInitialize)(this.modNumWTreshold)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.OptionsNumMajorGrid)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.OptionsNumGridSize)).EndInit();
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion
        private System.Windows.Forms.Button butOK;
        private System.Windows.Forms.CheckBox AnimCBShowPath;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.NumericUpDown modNumWTreshold;
        private System.Windows.Forms.CheckBox FramesCBShowLinkedHitpts;
        private System.Windows.Forms.NumericUpDown OptionsNumMajorGrid;
        private System.Windows.Forms.NumericUpDown OptionsNumGridSize;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.CheckBox chk_optionsExportSmallJSON;
        private System.Windows.Forms.GroupBox groupBox1;
    }
}