namespace HexxEditor
{
    partial class ObjectsWnd
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
            this.panel1 = new System.Windows.Forms.Panel();
            this.BSXbrowserCtrl1 = new BSXAnimBrowser.BSXbrowserCtrl();
            this.tableLayoutPanel1 = new System.Windows.Forms.TableLayoutPanel();
            this.panel2 = new System.Windows.Forms.Panel();
            this.but_toBack = new System.Windows.Forms.Button();
            this.but_toTop = new System.Windows.Forms.Button();
            this.chk_useAsCover = new System.Windows.Forms.CheckBox();
            this.chk_animated = new System.Windows.Forms.CheckBox();
            this.menuStrip1 = new System.Windows.Forms.MenuStrip();
            this.fileToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.openBSXToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.chk_affectChildren = new System.Windows.Forms.CheckBox();
            this.panel1.SuspendLayout();
            this.tableLayoutPanel1.SuspendLayout();
            this.panel2.SuspendLayout();
            this.menuStrip1.SuspendLayout();
            this.SuspendLayout();
            // 
            // panel1
            // 
            this.panel1.AutoScroll = true;
            this.panel1.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.panel1.Controls.Add(this.BSXbrowserCtrl1);
            this.panel1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.panel1.Location = new System.Drawing.Point(3, 3);
            this.panel1.Name = "panel1";
            this.panel1.Size = new System.Drawing.Size(402, 327);
            this.panel1.TabIndex = 0;
            // 
            // BSXbrowserCtrl1
            // 
            this.BSXbrowserCtrl1.animBoxSize = 80;
            this.BSXbrowserCtrl1.AutoScroll = true;
            this.BSXbrowserCtrl1.AutoScrollMinSize = new System.Drawing.Size(0, 640);
            this.BSXbrowserCtrl1.canSelectFrames = true;
            this.BSXbrowserCtrl1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.BSXbrowserCtrl1.Location = new System.Drawing.Point(0, 0);
            this.BSXbrowserCtrl1.maxScaling = 2F;
            this.BSXbrowserCtrl1.Name = "BSXbrowserCtrl1";
            this.BSXbrowserCtrl1.Size = new System.Drawing.Size(400, 325);
            this.BSXbrowserCtrl1.TabIndex = 0;
            this.BSXbrowserCtrl1.thumbAnimInterval = 300;
            this.BSXbrowserCtrl1.MyAnimChangedDelegate += new System.EventHandler(this.BSXbrowserCtrl1_MyAnimChangedDelegate_1);
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
            this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Absolute, 93F));
            this.tableLayoutPanel1.Size = new System.Drawing.Size(408, 426);
            this.tableLayoutPanel1.TabIndex = 1;
            // 
            // panel2
            // 
            this.panel2.Controls.Add(this.but_toBack);
            this.panel2.Controls.Add(this.but_toTop);
            this.panel2.Controls.Add(this.chk_affectChildren);
            this.panel2.Controls.Add(this.chk_useAsCover);
            this.panel2.Controls.Add(this.chk_animated);
            this.panel2.Dock = System.Windows.Forms.DockStyle.Fill;
            this.panel2.Location = new System.Drawing.Point(3, 336);
            this.panel2.Name = "panel2";
            this.panel2.Size = new System.Drawing.Size(402, 87);
            this.panel2.TabIndex = 1;
            // 
            // but_toBack
            // 
            this.but_toBack.Location = new System.Drawing.Point(307, 32);
            this.but_toBack.Name = "but_toBack";
            this.but_toBack.Size = new System.Drawing.Size(86, 23);
            this.but_toBack.TabIndex = 3;
            this.but_toBack.Text = "Send to BACK";
            this.but_toBack.UseVisualStyleBackColor = true;
            this.but_toBack.Click += new System.EventHandler(this.but_toBack_Click);
            // 
            // but_toTop
            // 
            this.but_toTop.Location = new System.Drawing.Point(307, 3);
            this.but_toTop.Name = "but_toTop";
            this.but_toTop.Size = new System.Drawing.Size(86, 23);
            this.but_toTop.TabIndex = 3;
            this.but_toTop.Text = "Bring to TOP";
            this.but_toTop.UseVisualStyleBackColor = true;
            this.but_toTop.Click += new System.EventHandler(this.but_toTop_Click);
            // 
            // chk_useAsCover
            // 
            this.chk_useAsCover.AutoSize = true;
            this.chk_useAsCover.Location = new System.Drawing.Point(3, 26);
            this.chk_useAsCover.Name = "chk_useAsCover";
            this.chk_useAsCover.Size = new System.Drawing.Size(65, 17);
            this.chk_useAsCover.TabIndex = 2;
            this.chk_useAsCover.Text = "Is Cover";
            this.chk_useAsCover.UseVisualStyleBackColor = true;
            this.chk_useAsCover.CheckedChanged += new System.EventHandler(this.chk_useAsCover_CheckedChanged);
            // 
            // chk_animated
            // 
            this.chk_animated.AutoSize = true;
            this.chk_animated.Location = new System.Drawing.Point(3, 3);
            this.chk_animated.Name = "chk_animated";
            this.chk_animated.Size = new System.Drawing.Size(70, 17);
            this.chk_animated.TabIndex = 1;
            this.chk_animated.Text = "Animated";
            this.chk_animated.UseVisualStyleBackColor = true;
            this.chk_animated.CheckedChanged += new System.EventHandler(this.chk_animated_CheckedChanged);
            // 
            // menuStrip1
            // 
            this.menuStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.fileToolStripMenuItem});
            this.menuStrip1.Location = new System.Drawing.Point(0, 0);
            this.menuStrip1.Name = "menuStrip1";
            this.menuStrip1.Size = new System.Drawing.Size(408, 24);
            this.menuStrip1.TabIndex = 2;
            this.menuStrip1.Text = "menuStrip1";
            // 
            // fileToolStripMenuItem
            // 
            this.fileToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.openBSXToolStripMenuItem});
            this.fileToolStripMenuItem.Name = "fileToolStripMenuItem";
            this.fileToolStripMenuItem.Size = new System.Drawing.Size(37, 20);
            this.fileToolStripMenuItem.Text = "File";
            // 
            // openBSXToolStripMenuItem
            // 
            this.openBSXToolStripMenuItem.Name = "openBSXToolStripMenuItem";
            this.openBSXToolStripMenuItem.Size = new System.Drawing.Size(178, 22);
            this.openBSXToolStripMenuItem.Text = "Open Objects BSX...";
            this.openBSXToolStripMenuItem.Click += new System.EventHandler(this.openBSXToolStripMenuItem_Click_1);
            // 
            // chk_affectChildren
            // 
            this.chk_affectChildren.AutoSize = true;
            this.chk_affectChildren.Checked = true;
            this.chk_affectChildren.CheckState = System.Windows.Forms.CheckState.Checked;
            this.chk_affectChildren.Location = new System.Drawing.Point(3, 49);
            this.chk_affectChildren.Name = "chk_affectChildren";
            this.chk_affectChildren.Size = new System.Drawing.Size(171, 17);
            this.chk_affectChildren.TabIndex = 2;
            this.chk_affectChildren.Text = "Move linked solid collisions too";
            this.chk_affectChildren.UseVisualStyleBackColor = true;
            this.chk_affectChildren.CheckedChanged += new System.EventHandler(this.chk_useAsCover_CheckedChanged);
            // 
            // ObjectsWnd
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(408, 450);
            this.Controls.Add(this.tableLayoutPanel1);
            this.Controls.Add(this.menuStrip1);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.SizableToolWindow;
            this.Name = "ObjectsWnd";
            this.Text = "ObjectsWnd";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.ObjectsWnd_FormClosing);
            this.VisibleChanged += new System.EventHandler(this.ObjectsWnd_VisibleChanged);
            this.panel1.ResumeLayout(false);
            this.tableLayoutPanel1.ResumeLayout(false);
            this.panel2.ResumeLayout(false);
            this.panel2.PerformLayout();
            this.menuStrip1.ResumeLayout(false);
            this.menuStrip1.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Panel panel1;
        private System.Windows.Forms.TableLayoutPanel tableLayoutPanel1;
        private System.Windows.Forms.ToolStripMenuItem openBSXToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem fileToolStripMenuItem;
        private System.Windows.Forms.MenuStrip menuStrip1;
        private System.Windows.Forms.Panel panel2;
        private BSXAnimBrowser.BSXbrowserCtrl BSXbrowserCtrl1;
        private System.Windows.Forms.CheckBox chk_animated;
        private System.Windows.Forms.CheckBox chk_useAsCover;
        private System.Windows.Forms.Button but_toTop;
        private System.Windows.Forms.Button but_toBack;
        private System.Windows.Forms.CheckBox chk_affectChildren;
    }
}