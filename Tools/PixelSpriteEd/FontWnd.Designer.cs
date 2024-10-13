namespace InkEditor
{
    partial class FontWnd
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
            this.button1 = new System.Windows.Forms.Button();
            this.IDBox = new System.Windows.Forms.TextBox();
            this.label1 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.LetterSpacing = new System.Windows.Forms.NumericUpDown();
            this.RowSpacing = new System.Windows.Forms.NumericUpDown();
            this.label3 = new System.Windows.Forms.Label();
            this.SpaceSize = new System.Windows.Forms.NumericUpDown();
            this.label4 = new System.Windows.Forms.Label();
            this.RowHeight = new System.Windows.Forms.NumericUpDown();
            this.label5 = new System.Windows.Forms.Label();
            this.button2 = new System.Windows.Forms.Button();
            this.checkBox1 = new System.Windows.Forms.CheckBox();
            this.chk_baseline = new System.Windows.Forms.CheckBox();
            this.chk_invert = new System.Windows.Forms.CheckBox();
            this.splitContainer1 = new System.Windows.Forms.SplitContainer();
            this.pbSample = new System.Windows.Forms.PictureBox();
            ((System.ComponentModel.ISupportInitialize)(this.LetterSpacing)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.RowSpacing)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.SpaceSize)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.RowHeight)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.splitContainer1)).BeginInit();
            this.splitContainer1.Panel1.SuspendLayout();
            this.splitContainer1.Panel2.SuspendLayout();
            this.splitContainer1.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.pbSample)).BeginInit();
            this.SuspendLayout();
            // 
            // button1
            // 
            this.button1.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.button1.Location = new System.Drawing.Point(9, 481);
            this.button1.Name = "button1";
            this.button1.Size = new System.Drawing.Size(75, 23);
            this.button1.TabIndex = 0;
            this.button1.Text = "OK";
            this.button1.UseVisualStyleBackColor = true;
            this.button1.Click += new System.EventHandler(this.button1_Click);
            // 
            // IDBox
            // 
            this.IDBox.Location = new System.Drawing.Point(85, 4);
            this.IDBox.Name = "IDBox";
            this.IDBox.Size = new System.Drawing.Size(100, 20);
            this.IDBox.TabIndex = 1;
            this.IDBox.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(61, 7);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(18, 13);
            this.label1.TabIndex = 2;
            this.label1.Text = "ID";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(6, 32);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(73, 13);
            this.label2.TabIndex = 4;
            this.label2.Text = "LetterSpacing";
            // 
            // LetterSpacing
            // 
            this.LetterSpacing.Location = new System.Drawing.Point(85, 30);
            this.LetterSpacing.Minimum = new decimal(new int[] {
            100,
            0,
            0,
            -2147483648});
            this.LetterSpacing.Name = "LetterSpacing";
            this.LetterSpacing.Size = new System.Drawing.Size(100, 20);
            this.LetterSpacing.TabIndex = 5;
            this.LetterSpacing.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            this.LetterSpacing.ValueChanged += new System.EventHandler(this.textBox1_TextChanged);
            // 
            // RowSpacing
            // 
            this.RowSpacing.Location = new System.Drawing.Point(85, 56);
            this.RowSpacing.Minimum = new decimal(new int[] {
            100,
            0,
            0,
            -2147483648});
            this.RowSpacing.Name = "RowSpacing";
            this.RowSpacing.Size = new System.Drawing.Size(100, 20);
            this.RowSpacing.TabIndex = 7;
            this.RowSpacing.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            this.RowSpacing.ValueChanged += new System.EventHandler(this.textBox1_TextChanged);
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(6, 58);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(68, 13);
            this.label3.TabIndex = 6;
            this.label3.Text = "RowSpacing";
            // 
            // SpaceSize
            // 
            this.SpaceSize.Location = new System.Drawing.Point(85, 108);
            this.SpaceSize.Minimum = new decimal(new int[] {
            100,
            0,
            0,
            -2147483648});
            this.SpaceSize.Name = "SpaceSize";
            this.SpaceSize.Size = new System.Drawing.Size(100, 20);
            this.SpaceSize.TabIndex = 9;
            this.SpaceSize.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            this.SpaceSize.ValueChanged += new System.EventHandler(this.textBox1_TextChanged);
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(6, 110);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(58, 13);
            this.label4.TabIndex = 8;
            this.label4.Text = "SpaceSize";
            // 
            // RowHeight
            // 
            this.RowHeight.Location = new System.Drawing.Point(85, 82);
            this.RowHeight.Minimum = new decimal(new int[] {
            100,
            0,
            0,
            -2147483648});
            this.RowHeight.Name = "RowHeight";
            this.RowHeight.Size = new System.Drawing.Size(100, 20);
            this.RowHeight.TabIndex = 11;
            this.RowHeight.TextAlign = System.Windows.Forms.HorizontalAlignment.Center;
            this.RowHeight.ValueChanged += new System.EventHandler(this.textBox1_TextChanged);
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Location = new System.Drawing.Point(6, 84);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(60, 13);
            this.label5.TabIndex = 10;
            this.label5.Text = "RowHeight";
            // 
            // button2
            // 
            this.button2.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.button2.Location = new System.Drawing.Point(9, 452);
            this.button2.Name = "button2";
            this.button2.Size = new System.Drawing.Size(75, 23);
            this.button2.TabIndex = 12;
            this.button2.Text = "Randomize";
            this.button2.UseVisualStyleBackColor = true;
            this.button2.Click += new System.EventHandler(this.button2_Click);
            // 
            // checkBox1
            // 
            this.checkBox1.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Left)));
            this.checkBox1.AutoSize = true;
            this.checkBox1.Location = new System.Drawing.Point(9, 429);
            this.checkBox1.Name = "checkBox1";
            this.checkBox1.Size = new System.Drawing.Size(90, 17);
            this.checkBox1.TabIndex = 13;
            this.checkBox1.Text = "Insert Header";
            this.checkBox1.UseVisualStyleBackColor = true;
            // 
            // chk_baseline
            // 
            this.chk_baseline.AutoSize = true;
            this.chk_baseline.Location = new System.Drawing.Point(9, 146);
            this.chk_baseline.Name = "chk_baseline";
            this.chk_baseline.Size = new System.Drawing.Size(118, 17);
            this.chk_baseline.TabIndex = 14;
            this.chk_baseline.Text = "Draw Font Baseline";
            this.chk_baseline.UseVisualStyleBackColor = true;
            this.chk_baseline.CheckedChanged += new System.EventHandler(this.chk_baseline_CheckedChanged);
            // 
            // chk_invert
            // 
            this.chk_invert.AutoSize = true;
            this.chk_invert.Location = new System.Drawing.Point(9, 169);
            this.chk_invert.Name = "chk_invert";
            this.chk_invert.Size = new System.Drawing.Size(114, 17);
            this.chk_invert.TabIndex = 14;
            this.chk_invert.Text = "Invert Background";
            this.chk_invert.UseVisualStyleBackColor = true;
            this.chk_invert.CheckedChanged += new System.EventHandler(this.chk_invert_CheckedChanged);
            // 
            // splitContainer1
            // 
            this.splitContainer1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.splitContainer1.FixedPanel = System.Windows.Forms.FixedPanel.Panel1;
            this.splitContainer1.IsSplitterFixed = true;
            this.splitContainer1.Location = new System.Drawing.Point(0, 0);
            this.splitContainer1.Name = "splitContainer1";
            // 
            // splitContainer1.Panel1
            // 
            this.splitContainer1.Panel1.Controls.Add(this.RowSpacing);
            this.splitContainer1.Panel1.Controls.Add(this.checkBox1);
            this.splitContainer1.Panel1.Controls.Add(this.chk_invert);
            this.splitContainer1.Panel1.Controls.Add(this.button2);
            this.splitContainer1.Panel1.Controls.Add(this.IDBox);
            this.splitContainer1.Panel1.Controls.Add(this.button1);
            this.splitContainer1.Panel1.Controls.Add(this.chk_baseline);
            this.splitContainer1.Panel1.Controls.Add(this.label1);
            this.splitContainer1.Panel1.Controls.Add(this.label2);
            this.splitContainer1.Panel1.Controls.Add(this.LetterSpacing);
            this.splitContainer1.Panel1.Controls.Add(this.RowHeight);
            this.splitContainer1.Panel1.Controls.Add(this.label3);
            this.splitContainer1.Panel1.Controls.Add(this.label5);
            this.splitContainer1.Panel1.Controls.Add(this.label4);
            this.splitContainer1.Panel1.Controls.Add(this.SpaceSize);
            // 
            // splitContainer1.Panel2
            // 
            this.splitContainer1.Panel2.Controls.Add(this.pbSample);
            this.splitContainer1.Size = new System.Drawing.Size(656, 516);
            this.splitContainer1.SplitterDistance = 224;
            this.splitContainer1.TabIndex = 15;
            // 
            // pbSample
            // 
            this.pbSample.Dock = System.Windows.Forms.DockStyle.Fill;
            this.pbSample.Location = new System.Drawing.Point(0, 0);
            this.pbSample.Name = "pbSample";
            this.pbSample.Size = new System.Drawing.Size(428, 516);
            this.pbSample.TabIndex = 0;
            this.pbSample.TabStop = false;
            this.pbSample.SizeChanged += new System.EventHandler(this.pbSample_SizeChanged);
            // 
            // FontWnd
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(656, 516);
            this.Controls.Add(this.splitContainer1);
            this.DoubleBuffered = true;
            this.Name = "FontWnd";
            this.ShowIcon = false;
            this.SizeGripStyle = System.Windows.Forms.SizeGripStyle.Hide;
            this.Text = "Font Tool";
            this.Activated += new System.EventHandler(this.textBox1_TextChanged);
            this.Load += new System.EventHandler(this.FontWnd_Load);
            this.Paint += new System.Windows.Forms.PaintEventHandler(this.FontWnd_Paint);
            this.Resize += new System.EventHandler(this.textBox1_TextChanged);
            ((System.ComponentModel.ISupportInitialize)(this.LetterSpacing)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.RowSpacing)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.SpaceSize)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.RowHeight)).EndInit();
            this.splitContainer1.Panel1.ResumeLayout(false);
            this.splitContainer1.Panel1.PerformLayout();
            this.splitContainer1.Panel2.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.splitContainer1)).EndInit();
            this.splitContainer1.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.pbSample)).EndInit();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.Button button1;
        private System.Windows.Forms.TextBox IDBox;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.NumericUpDown LetterSpacing;
        private System.Windows.Forms.NumericUpDown RowSpacing;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.NumericUpDown SpaceSize;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.NumericUpDown RowHeight;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.Button button2;
        private System.Windows.Forms.CheckBox checkBox1;
        private System.Windows.Forms.CheckBox chk_baseline;
        private System.Windows.Forms.CheckBox chk_invert;
        private System.Windows.Forms.SplitContainer splitContainer1;
        private System.Windows.Forms.PictureBox pbSample;
    }
}