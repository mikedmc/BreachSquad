namespace HexxEditor
{
    partial class ModCreator
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(ModCreator));
            this.combo_modType = new System.Windows.Forms.ComboBox();
            this.text_modName = new System.Windows.Forms.TextBox();
            this.label1 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.but_Upload = new System.Windows.Forms.Button();
            this.menuStrip1 = new System.Windows.Forms.MenuStrip();
            this.fileToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.saveMODDescriptorToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.openMODDescriptorToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripMenuItem1 = new System.Windows.Forms.ToolStripSeparator();
            this.exitToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.text_description = new System.Windows.Forms.TextBox();
            this.label3 = new System.Windows.Forms.Label();
            this.text_author = new System.Windows.Forms.TextBox();
            this.label4 = new System.Windows.Forms.Label();
            this.text_tags = new System.Windows.Forms.TextBox();
            this.label5 = new System.Windows.Forms.Label();
            this.text_changenotes = new System.Windows.Forms.TextBox();
            this.label6 = new System.Windows.Forms.Label();
            this.text_gameVer = new System.Windows.Forms.TextBox();
            this.label7 = new System.Windows.Forms.Label();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.groupBox2 = new System.Windows.Forms.GroupBox();
            this.but_selectImage = new System.Windows.Forms.Button();
            this.pb_Image = new System.Windows.Forms.PictureBox();
            this.but_cancel = new System.Windows.Forms.Button();
            this.list_files = new System.Windows.Forms.ListBox();
            this.groupBox3 = new System.Windows.Forms.GroupBox();
            this.but_filesRemove = new System.Windows.Forms.Button();
            this.but_filesAdd = new System.Windows.Forms.Button();
            this.menuStrip1.SuspendLayout();
            this.groupBox1.SuspendLayout();
            this.groupBox2.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.pb_Image)).BeginInit();
            this.groupBox3.SuspendLayout();
            this.SuspendLayout();
            // 
            // combo_modType
            // 
            this.combo_modType.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
            this.combo_modType.FormattingEnabled = true;
            this.combo_modType.Items.AddRange(new object[] {
            "Single Level",
            "Game Mod"});
            this.combo_modType.Location = new System.Drawing.Point(87, 19);
            this.combo_modType.Name = "combo_modType";
            this.combo_modType.Size = new System.Drawing.Size(134, 21);
            this.combo_modType.TabIndex = 0;
            this.combo_modType.SelectedIndexChanged += new System.EventHandler(this.combo_modType_SelectedIndexChanged);
            // 
            // text_modName
            // 
            this.text_modName.Location = new System.Drawing.Point(87, 46);
            this.text_modName.Name = "text_modName";
            this.text_modName.Size = new System.Drawing.Size(134, 20);
            this.text_modName.TabIndex = 1;
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(50, 22);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(31, 13);
            this.label1.TabIndex = 2;
            this.label1.Text = "Type";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(22, 49);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(59, 13);
            this.label2.TabIndex = 2;
            this.label2.Text = "Mod Name";
            // 
            // but_Upload
            // 
            this.but_Upload.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(255)))), ((int)(((byte)(192)))), ((int)(((byte)(192)))));
            this.but_Upload.Location = new System.Drawing.Point(12, 394);
            this.but_Upload.Name = "but_Upload";
            this.but_Upload.Size = new System.Drawing.Size(88, 28);
            this.but_Upload.TabIndex = 3;
            this.but_Upload.Text = "Upload Now";
            this.but_Upload.UseVisualStyleBackColor = false;
            this.but_Upload.Click += new System.EventHandler(this.but_Upload_Click);
            // 
            // menuStrip1
            // 
            this.menuStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.fileToolStripMenuItem});
            this.menuStrip1.Location = new System.Drawing.Point(0, 0);
            this.menuStrip1.Name = "menuStrip1";
            this.menuStrip1.Size = new System.Drawing.Size(647, 24);
            this.menuStrip1.TabIndex = 4;
            this.menuStrip1.Text = "menuStrip1";
            // 
            // fileToolStripMenuItem
            // 
            this.fileToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.saveMODDescriptorToolStripMenuItem,
            this.openMODDescriptorToolStripMenuItem,
            this.toolStripMenuItem1,
            this.exitToolStripMenuItem});
            this.fileToolStripMenuItem.Name = "fileToolStripMenuItem";
            this.fileToolStripMenuItem.Size = new System.Drawing.Size(37, 20);
            this.fileToolStripMenuItem.Text = "File";
            // 
            // saveMODDescriptorToolStripMenuItem
            // 
            this.saveMODDescriptorToolStripMenuItem.Name = "saveMODDescriptorToolStripMenuItem";
            this.saveMODDescriptorToolStripMenuItem.Size = new System.Drawing.Size(199, 22);
            this.saveMODDescriptorToolStripMenuItem.Text = "Save MOD descriptor...";
            this.saveMODDescriptorToolStripMenuItem.Click += new System.EventHandler(this.saveMODDescriptorToolStripMenuItem_Click);
            // 
            // openMODDescriptorToolStripMenuItem
            // 
            this.openMODDescriptorToolStripMenuItem.Name = "openMODDescriptorToolStripMenuItem";
            this.openMODDescriptorToolStripMenuItem.Size = new System.Drawing.Size(199, 22);
            this.openMODDescriptorToolStripMenuItem.Text = "Open MOD descriptor...";
            this.openMODDescriptorToolStripMenuItem.Click += new System.EventHandler(this.openMODDescriptorToolStripMenuItem_Click);
            // 
            // toolStripMenuItem1
            // 
            this.toolStripMenuItem1.Name = "toolStripMenuItem1";
            this.toolStripMenuItem1.Size = new System.Drawing.Size(196, 6);
            // 
            // exitToolStripMenuItem
            // 
            this.exitToolStripMenuItem.Name = "exitToolStripMenuItem";
            this.exitToolStripMenuItem.Size = new System.Drawing.Size(199, 22);
            this.exitToolStripMenuItem.Text = "Exit";
            this.exitToolStripMenuItem.Click += new System.EventHandler(this.exitToolStripMenuItem_Click);
            // 
            // text_description
            // 
            this.text_description.Location = new System.Drawing.Point(87, 72);
            this.text_description.Name = "text_description";
            this.text_description.Size = new System.Drawing.Size(351, 20);
            this.text_description.TabIndex = 1;
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(21, 75);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(60, 13);
            this.label3.TabIndex = 2;
            this.label3.Text = "Description";
            // 
            // text_author
            // 
            this.text_author.Location = new System.Drawing.Point(304, 46);
            this.text_author.Name = "text_author";
            this.text_author.Size = new System.Drawing.Size(134, 20);
            this.text_author.TabIndex = 1;
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(260, 49);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(38, 13);
            this.label4.TabIndex = 2;
            this.label4.Text = "Author";
            // 
            // text_tags
            // 
            this.text_tags.Location = new System.Drawing.Point(87, 98);
            this.text_tags.Name = "text_tags";
            this.text_tags.Size = new System.Drawing.Size(351, 20);
            this.text_tags.TabIndex = 1;
            // 
            // label5
            // 
            this.label5.AutoSize = true;
            this.label5.Location = new System.Drawing.Point(50, 101);
            this.label5.Name = "label5";
            this.label5.Size = new System.Drawing.Size(31, 13);
            this.label5.TabIndex = 2;
            this.label5.Text = "Tags";
            // 
            // text_changenotes
            // 
            this.text_changenotes.Location = new System.Drawing.Point(87, 124);
            this.text_changenotes.Name = "text_changenotes";
            this.text_changenotes.Size = new System.Drawing.Size(351, 20);
            this.text_changenotes.TabIndex = 1;
            // 
            // label6
            // 
            this.label6.AutoSize = true;
            this.label6.Location = new System.Drawing.Point(6, 127);
            this.label6.Name = "label6";
            this.label6.Size = new System.Drawing.Size(75, 13);
            this.label6.TabIndex = 2;
            this.label6.Text = "Change Notes";
            // 
            // text_gameVer
            // 
            this.text_gameVer.Location = new System.Drawing.Point(304, 20);
            this.text_gameVer.Name = "text_gameVer";
            this.text_gameVer.Size = new System.Drawing.Size(134, 20);
            this.text_gameVer.TabIndex = 1;
            // 
            // label7
            // 
            this.label7.AutoSize = true;
            this.label7.Location = new System.Drawing.Point(226, 23);
            this.label7.Name = "label7";
            this.label7.Size = new System.Drawing.Size(72, 13);
            this.label7.TabIndex = 2;
            this.label7.Text = "Game version";
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.combo_modType);
            this.groupBox1.Controls.Add(this.text_modName);
            this.groupBox1.Controls.Add(this.label6);
            this.groupBox1.Controls.Add(this.text_author);
            this.groupBox1.Controls.Add(this.label5);
            this.groupBox1.Controls.Add(this.text_gameVer);
            this.groupBox1.Controls.Add(this.label3);
            this.groupBox1.Controls.Add(this.text_description);
            this.groupBox1.Controls.Add(this.label7);
            this.groupBox1.Controls.Add(this.text_tags);
            this.groupBox1.Controls.Add(this.label4);
            this.groupBox1.Controls.Add(this.text_changenotes);
            this.groupBox1.Controls.Add(this.label2);
            this.groupBox1.Controls.Add(this.label1);
            this.groupBox1.Location = new System.Drawing.Point(12, 27);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(446, 156);
            this.groupBox1.TabIndex = 5;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "Mod Data";
            // 
            // groupBox2
            // 
            this.groupBox2.Controls.Add(this.but_selectImage);
            this.groupBox2.Controls.Add(this.pb_Image);
            this.groupBox2.Location = new System.Drawing.Point(464, 27);
            this.groupBox2.Name = "groupBox2";
            this.groupBox2.Size = new System.Drawing.Size(172, 156);
            this.groupBox2.TabIndex = 6;
            this.groupBox2.TabStop = false;
            this.groupBox2.Text = "Mod Image";
            // 
            // but_selectImage
            // 
            this.but_selectImage.Location = new System.Drawing.Point(6, 122);
            this.but_selectImage.Name = "but_selectImage";
            this.but_selectImage.Size = new System.Drawing.Size(160, 23);
            this.but_selectImage.TabIndex = 1;
            this.but_selectImage.Text = "Select Image";
            this.but_selectImage.UseVisualStyleBackColor = true;
            this.but_selectImage.Click += new System.EventHandler(this.but_selectImage_Click);
            // 
            // pb_Image
            // 
            this.pb_Image.BackColor = System.Drawing.Color.FromArgb(((int)(((byte)(255)))), ((int)(((byte)(128)))), ((int)(((byte)(128)))));
            this.pb_Image.Location = new System.Drawing.Point(6, 20);
            this.pb_Image.Name = "pb_Image";
            this.pb_Image.Size = new System.Drawing.Size(160, 90);
            this.pb_Image.TabIndex = 0;
            this.pb_Image.TabStop = false;
            // 
            // but_cancel
            // 
            this.but_cancel.Location = new System.Drawing.Point(106, 394);
            this.but_cancel.Name = "but_cancel";
            this.but_cancel.Size = new System.Drawing.Size(88, 28);
            this.but_cancel.TabIndex = 3;
            this.but_cancel.Text = "Cancel";
            this.but_cancel.UseVisualStyleBackColor = true;
            this.but_cancel.Click += new System.EventHandler(this.but_cancel_Click);
            // 
            // list_files
            // 
            this.list_files.FormattingEnabled = true;
            this.list_files.Location = new System.Drawing.Point(6, 19);
            this.list_files.Name = "list_files";
            this.list_files.Size = new System.Drawing.Size(612, 147);
            this.list_files.TabIndex = 0;
            // 
            // groupBox3
            // 
            this.groupBox3.Controls.Add(this.but_filesRemove);
            this.groupBox3.Controls.Add(this.but_filesAdd);
            this.groupBox3.Controls.Add(this.list_files);
            this.groupBox3.Location = new System.Drawing.Point(12, 189);
            this.groupBox3.Name = "groupBox3";
            this.groupBox3.Size = new System.Drawing.Size(624, 199);
            this.groupBox3.TabIndex = 7;
            this.groupBox3.TabStop = false;
            this.groupBox3.Text = "Affected Files";
            // 
            // but_filesRemove
            // 
            this.but_filesRemove.Location = new System.Drawing.Point(117, 170);
            this.but_filesRemove.Name = "but_filesRemove";
            this.but_filesRemove.Size = new System.Drawing.Size(105, 23);
            this.but_filesRemove.TabIndex = 1;
            this.but_filesRemove.Text = "Remove Selected";
            this.but_filesRemove.UseVisualStyleBackColor = true;
            this.but_filesRemove.Click += new System.EventHandler(this.but_filesRemove_Click);
            // 
            // but_filesAdd
            // 
            this.but_filesAdd.Location = new System.Drawing.Point(6, 170);
            this.but_filesAdd.Name = "but_filesAdd";
            this.but_filesAdd.Size = new System.Drawing.Size(105, 23);
            this.but_filesAdd.TabIndex = 1;
            this.but_filesAdd.Text = "Add File...";
            this.but_filesAdd.UseVisualStyleBackColor = true;
            this.but_filesAdd.Click += new System.EventHandler(this.but_filesAdd_Click);
            // 
            // ModCreator
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(647, 431);
            this.Controls.Add(this.groupBox3);
            this.Controls.Add(this.groupBox2);
            this.Controls.Add(this.groupBox1);
            this.Controls.Add(this.but_cancel);
            this.Controls.Add(this.but_Upload);
            this.Controls.Add(this.menuStrip1);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedToolWindow;
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.MainMenuStrip = this.menuStrip1;
            this.Name = "ModCreator";
            this.Text = "ModCreator";
            this.menuStrip1.ResumeLayout(false);
            this.menuStrip1.PerformLayout();
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            this.groupBox2.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.pb_Image)).EndInit();
            this.groupBox3.ResumeLayout(false);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.ComboBox combo_modType;
        private System.Windows.Forms.TextBox text_modName;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Button but_Upload;
        private System.Windows.Forms.MenuStrip menuStrip1;
        private System.Windows.Forms.ToolStripMenuItem fileToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem openMODDescriptorToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem saveMODDescriptorToolStripMenuItem;
        private System.Windows.Forms.TextBox text_description;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.TextBox text_author;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.TextBox text_tags;
        private System.Windows.Forms.Label label5;
        private System.Windows.Forms.TextBox text_changenotes;
        private System.Windows.Forms.Label label6;
        private System.Windows.Forms.TextBox text_gameVer;
        private System.Windows.Forms.Label label7;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.GroupBox groupBox2;
        private System.Windows.Forms.Button but_selectImage;
        private System.Windows.Forms.PictureBox pb_Image;
        private System.Windows.Forms.Button but_cancel;
        private System.Windows.Forms.ToolStripSeparator toolStripMenuItem1;
        private System.Windows.Forms.ToolStripMenuItem exitToolStripMenuItem;
        private System.Windows.Forms.ListBox list_files;
        private System.Windows.Forms.GroupBox groupBox3;
        private System.Windows.Forms.Button but_filesRemove;
        private System.Windows.Forms.Button but_filesAdd;
    }
}