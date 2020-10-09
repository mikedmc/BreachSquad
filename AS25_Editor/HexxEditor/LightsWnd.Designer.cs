namespace HexxEditor
{
    partial class LightsWnd
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
            this.groupLightData = new System.Windows.Forms.GroupBox();
            this.num_lightIntensity = new System.Windows.Forms.NumericUpDown();
            this.label5 = new System.Windows.Forms.Label();
            this.label1 = new System.Windows.Forms.Label();
            this.numZCoord = new System.Windows.Forms.NumericUpDown();
            this.hScrollBarAlpha = new System.Windows.Forms.HScrollBar();
            this.butColor = new System.Windows.Forms.Button();
            this.label3 = new System.Windows.Forms.Label();
            this.chk_castShadows = new System.Windows.Forms.CheckBox();
            this.label2 = new System.Windows.Forms.Label();
            this.combo_lightType = new System.Windows.Forms.ComboBox();
            this.label4 = new System.Windows.Forms.Label();
            this.num_Angle = new System.Windows.Forms.NumericUpDown();
            this.tableLayoutPanel1 = new System.Windows.Forms.TableLayoutPanel();
            this.panel1 = new System.Windows.Forms.Panel();
            this.chk_showSpotImg = new System.Windows.Forms.CheckBox();
            this.panel2 = new System.Windows.Forms.Panel();
            this.bsXbrowserCtrl1 = new BSXAnimBrowser.BSXbrowserCtrl();
            this.menuStrip1 = new System.Windows.Forms.MenuStrip();
            this.fileToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.openLightsBSXToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.num_atmoAttenuation = new System.Windows.Forms.NumericUpDown();
            this.label6 = new System.Windows.Forms.Label();
            this.groupLightData.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.num_lightIntensity)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.numZCoord)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.num_Angle)).BeginInit();
            this.tableLayoutPanel1.SuspendLayout();
            this.panel1.SuspendLayout();
            this.panel2.SuspendLayout();
            this.menuStrip1.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.num_atmoAttenuation)).BeginInit();
            this.SuspendLayout();
            // 
            // groupLightData
            // 
            this.groupLightData.Controls.Add(this.num_atmoAttenuation);
            this.groupLightData.Controls.Add(this.num_lightIntensity);
            this.groupLightData.Controls.Add(this.label6);
            this.groupLightData.Controls.Add(this.label5);
            this.groupLightData.Controls.Add(this.label1);
            this.groupLightData.Controls.Add(this.numZCoord);
            this.groupLightData.Controls.Add(this.hScrollBarAlpha);
            this.groupLightData.Controls.Add(this.butColor);
            this.groupLightData.Controls.Add(this.label3);
            this.groupLightData.Controls.Add(this.chk_castShadows);
            this.groupLightData.Controls.Add(this.label2);
            this.groupLightData.Controls.Add(this.combo_lightType);
            this.groupLightData.Controls.Add(this.label4);
            this.groupLightData.Controls.Add(this.num_Angle);
            this.groupLightData.Location = new System.Drawing.Point(3, 3);
            this.groupLightData.Name = "groupLightData";
            this.groupLightData.Size = new System.Drawing.Size(378, 141);
            this.groupLightData.TabIndex = 0;
            this.groupLightData.TabStop = false;
            this.groupLightData.Text = "Light Data";
            // 
            // num_lightIntensity
            // 
            this.num_lightIntensity.DecimalPlaces = 2;
            this.num_lightIntensity.Increment = new decimal(new int[] {
            1,
            0,
            0,
            65536});
            this.num_lightIntensity.Location = new System.Drawing.Point(299, 69);
            this.num_lightIntensity.Name = "num_lightIntensity";
            this.num_lightIntensity.Size = new System.Drawing.Size(73, 20);
            this.num_lightIntensity.TabIndex = 12;
            this.num_lightIntensity.Value = new decimal(new int[] {
            1,
            0,
            0,
            0});
            this.num_lightIntensity.ValueChanged += new System.EventHandler(this.num_lightIntensity_ValueChanged);
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Location = new System.Drawing.Point(249, 71);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(46, 13);
            this.label5.TabIndex = 11;
            this.label5.Text = "Intensity";
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(124, 71);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(44, 13);
            this.label1.TabIndex = 10;
            this.label1.Text = "Z coord";
            // 
            // numZCoord
            // 
            this.numZCoord.Location = new System.Drawing.Point(170, 68);
            this.numZCoord.Maximum = new decimal(new int[] {
            1000,
            0,
            0,
            0});
            this.numZCoord.Minimum = new decimal(new int[] {
            1000,
            0,
            0,
            -2147483648});
            this.numZCoord.Name = "numZCoord";
            this.numZCoord.Size = new System.Drawing.Size(73, 20);
            this.numZCoord.TabIndex = 9;
            this.numZCoord.Value = new decimal(new int[] {
            100,
            0,
            0,
            0});
            this.numZCoord.ValueChanged += new System.EventHandler(this.numZCoord_ValueChanged);
            // 
            // hScrollBarAlpha
            // 
            this.hScrollBarAlpha.LargeChange = 1;
            this.hScrollBarAlpha.Location = new System.Drawing.Point(139, 45);
            this.hScrollBarAlpha.Maximum = 255;
            this.hScrollBarAlpha.Name = "hScrollBarAlpha";
            this.hScrollBarAlpha.Size = new System.Drawing.Size(104, 20);
            this.hScrollBarAlpha.TabIndex = 3;
            this.hScrollBarAlpha.Value = 255;
            this.hScrollBarAlpha.ValueChanged += new System.EventHandler(this.hScrollBarAlpha_ValueChanged);
            // 
            // butColor
            // 
            this.butColor.BackColor = System.Drawing.SystemColors.ActiveCaption;
            this.butColor.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            this.butColor.Location = new System.Drawing.Point(55, 44);
            this.butColor.Name = "butColor";
            this.butColor.Size = new System.Drawing.Size(81, 23);
            this.butColor.TabIndex = 8;
            this.butColor.TabStop = false;
            this.butColor.UseVisualStyleBackColor = false;
            this.butColor.Click += new System.EventHandler(this.butColor_Click);
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(9, 49);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(31, 13);
            this.label3.TabIndex = 5;
            this.label3.Text = "Color";
            // 
            // chk_castShadows
            // 
            this.chk_castShadows.AutoSize = true;
            this.chk_castShadows.Location = new System.Drawing.Point(55, 96);
            this.chk_castShadows.Name = "chk_castShadows";
            this.chk_castShadows.Size = new System.Drawing.Size(94, 17);
            this.chk_castShadows.TabIndex = 4;
            this.chk_castShadows.Text = "Cast Shadows";
            this.chk_castShadows.UseVisualStyleBackColor = true;
            this.chk_castShadows.CheckedChanged += new System.EventHandler(this.chk_castShadows_CheckedChanged);
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(9, 24);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(31, 13);
            this.label2.TabIndex = 3;
            this.label2.Text = "Type";
            // 
            // combo_lightType
            // 
            this.combo_lightType.FormattingEnabled = true;
            this.combo_lightType.Items.AddRange(new object[] {
            "Ambiental",
            "Area Light",
            "Point Light",
            "Directional"});
            this.combo_lightType.Location = new System.Drawing.Point(55, 21);
            this.combo_lightType.Name = "combo_lightType";
            this.combo_lightType.Size = new System.Drawing.Size(188, 21);
            this.combo_lightType.TabIndex = 2;
            this.combo_lightType.SelectedIndexChanged += new System.EventHandler(this.combo_lightType_SelectedIndexChanged);
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(6, 71);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(34, 13);
            this.label4.TabIndex = 1;
            this.label4.Text = "Angle";
            // 
            // num_Angle
            // 
            this.num_Angle.Location = new System.Drawing.Point(55, 69);
            this.num_Angle.Maximum = new decimal(new int[] {
            360,
            0,
            0,
            0});
            this.num_Angle.Name = "num_Angle";
            this.num_Angle.Size = new System.Drawing.Size(63, 20);
            this.num_Angle.TabIndex = 0;
            this.num_Angle.ValueChanged += new System.EventHandler(this.num_angle_ValueChanged);
            // 
            // tableLayoutPanel1
            // 
            this.tableLayoutPanel1.ColumnCount = 1;
            this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
            this.tableLayoutPanel1.Controls.Add(this.panel1, 0, 1);
            this.tableLayoutPanel1.Controls.Add(this.panel2, 0, 0);
            this.tableLayoutPanel1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.tableLayoutPanel1.Location = new System.Drawing.Point(0, 24);
            this.tableLayoutPanel1.Name = "tableLayoutPanel1";
            this.tableLayoutPanel1.RowCount = 2;
            this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
            this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Absolute, 200F));
            this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Absolute, 20F));
            this.tableLayoutPanel1.Size = new System.Drawing.Size(390, 524);
            this.tableLayoutPanel1.TabIndex = 2;
            // 
            // panel1
            // 
            this.panel1.Controls.Add(this.chk_showSpotImg);
            this.panel1.Controls.Add(this.groupLightData);
            this.panel1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.panel1.Location = new System.Drawing.Point(3, 327);
            this.panel1.Name = "panel1";
            this.panel1.Size = new System.Drawing.Size(384, 194);
            this.panel1.TabIndex = 3;
            // 
            // chk_showSpotImg
            // 
            this.chk_showSpotImg.AutoSize = true;
            this.chk_showSpotImg.Location = new System.Drawing.Point(3, 174);
            this.chk_showSpotImg.Name = "chk_showSpotImg";
            this.chk_showSpotImg.Size = new System.Drawing.Size(106, 17);
            this.chk_showSpotImg.TabIndex = 2;
            this.chk_showSpotImg.Text = "Show light image";
            this.chk_showSpotImg.UseVisualStyleBackColor = true;
            this.chk_showSpotImg.CheckedChanged += new System.EventHandler(this.chk_showSpotImg_CheckedChanged);
            // 
            // panel2
            // 
            this.panel2.AutoScroll = true;
            this.panel2.Controls.Add(this.bsXbrowserCtrl1);
            this.panel2.Dock = System.Windows.Forms.DockStyle.Fill;
            this.panel2.Location = new System.Drawing.Point(3, 3);
            this.panel2.Name = "panel2";
            this.panel2.Size = new System.Drawing.Size(384, 318);
            this.panel2.TabIndex = 4;
            // 
            // bsXbrowserCtrl1
            // 
            this.bsXbrowserCtrl1.animBoxSize = 80;
            this.bsXbrowserCtrl1.AutoScroll = true;
            this.bsXbrowserCtrl1.AutoScrollMinSize = new System.Drawing.Size(0, 720);
            this.bsXbrowserCtrl1.canSelectFrames = false;
            this.bsXbrowserCtrl1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.bsXbrowserCtrl1.Location = new System.Drawing.Point(0, 0);
            this.bsXbrowserCtrl1.maxScaling = 2F;
            this.bsXbrowserCtrl1.Name = "bsXbrowserCtrl1";
            this.bsXbrowserCtrl1.Size = new System.Drawing.Size(384, 318);
            this.bsXbrowserCtrl1.TabIndex = 0;
            this.bsXbrowserCtrl1.thumbAnimInterval = 500;
            this.bsXbrowserCtrl1.MyAnimChangedDelegate += new System.EventHandler(this.bsXbrowserCtrl1_MyAnimChangedDelegate);
            // 
            // menuStrip1
            // 
            this.menuStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.fileToolStripMenuItem});
            this.menuStrip1.Location = new System.Drawing.Point(0, 0);
            this.menuStrip1.Name = "menuStrip1";
            this.menuStrip1.Size = new System.Drawing.Size(390, 24);
            this.menuStrip1.TabIndex = 3;
            this.menuStrip1.Text = "menuStrip1";
            // 
            // fileToolStripMenuItem
            // 
            this.fileToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.openLightsBSXToolStripMenuItem});
            this.fileToolStripMenuItem.Name = "fileToolStripMenuItem";
            this.fileToolStripMenuItem.Size = new System.Drawing.Size(37, 20);
            this.fileToolStripMenuItem.Text = "File";
            // 
            // openLightsBSXToolStripMenuItem
            // 
            this.openLightsBSXToolStripMenuItem.Name = "openLightsBSXToolStripMenuItem";
            this.openLightsBSXToolStripMenuItem.Size = new System.Drawing.Size(170, 22);
            this.openLightsBSXToolStripMenuItem.Text = "Open Lights BSX...";
            this.openLightsBSXToolStripMenuItem.Click += new System.EventHandler(this.openLightsBSXToolStripMenuItem_Click);
            // 
            // num_atmoAttenuation
            // 
            this.num_atmoAttenuation.Increment = new decimal(new int[] {
            10,
            0,
            0,
            0});
            this.num_atmoAttenuation.Location = new System.Drawing.Point(299, 95);
            this.num_atmoAttenuation.Name = "num_atmoAttenuation";
            this.num_atmoAttenuation.Size = new System.Drawing.Size(73, 20);
            this.num_atmoAttenuation.TabIndex = 13;
            this.num_atmoAttenuation.ValueChanged += new System.EventHandler(this.num_atmoAtten_ValueChanged);
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Location = new System.Drawing.Point(174, 97);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(121, 13);
            this.label6.TabIndex = 11;
            this.label6.Text = "Atmospheric attenuation";
            // 
            // LightsWnd
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(390, 548);
            this.Controls.Add(this.tableLayoutPanel1);
            this.Controls.Add(this.menuStrip1);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.SizableToolWindow;
            this.MainMenuStrip = this.menuStrip1;
            this.Name = "LightsWnd";
            this.Text = "Lights";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.LightsWnd_FormClosing);
            this.groupLightData.ResumeLayout(false);
            this.groupLightData.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.num_lightIntensity)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.numZCoord)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.num_Angle)).EndInit();
            this.tableLayoutPanel1.ResumeLayout(false);
            this.panel1.ResumeLayout(false);
            this.panel1.PerformLayout();
            this.panel2.ResumeLayout(false);
            this.menuStrip1.ResumeLayout(false);
            this.menuStrip1.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.num_atmoAttenuation)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.GroupBox groupLightData;
        private System.Windows.Forms.ComboBox combo_lightType;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.CheckBox chk_castShadows;
        private System.Windows.Forms.Button butColor;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.NumericUpDown num_Angle;
        private System.Windows.Forms.TableLayoutPanel tableLayoutPanel1;
        private System.Windows.Forms.Panel panel1;
        private System.Windows.Forms.Panel panel2;
        private BSXAnimBrowser.BSXbrowserCtrl bsXbrowserCtrl1;
        private System.Windows.Forms.MenuStrip menuStrip1;
        private System.Windows.Forms.ToolStripMenuItem fileToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem openLightsBSXToolStripMenuItem;
        private System.Windows.Forms.CheckBox chk_showSpotImg;
        private System.Windows.Forms.HScrollBar hScrollBarAlpha;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.NumericUpDown numZCoord;
        private System.Windows.Forms.NumericUpDown num_lightIntensity;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.NumericUpDown num_atmoAttenuation;
        private System.Windows.Forms.Label label6;
    }
}