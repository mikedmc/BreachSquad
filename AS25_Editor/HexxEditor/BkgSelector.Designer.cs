namespace HexxEditor
{
    partial class BkgSelector
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
            System.Windows.Forms.ListViewItem listViewItem4 = new System.Windows.Forms.ListViewItem("Background Name");
            this.lv_backgrounds = new System.Windows.Forms.ListView();
            this.columnHeader1 = ((System.Windows.Forms.ColumnHeader)(new System.Windows.Forms.ColumnHeader()));
            this.SuspendLayout();
            // 
            // lv_backgrounds
            // 
            this.lv_backgrounds.Columns.AddRange(new System.Windows.Forms.ColumnHeader[] {
            this.columnHeader1});
            this.lv_backgrounds.FullRowSelect = true;
            this.lv_backgrounds.GridLines = true;
            this.lv_backgrounds.HeaderStyle = System.Windows.Forms.ColumnHeaderStyle.None;
            this.lv_backgrounds.HideSelection = false;
            this.lv_backgrounds.Items.AddRange(new System.Windows.Forms.ListViewItem[] {
            listViewItem4});
            this.lv_backgrounds.Location = new System.Drawing.Point(8, 8);
            this.lv_backgrounds.MultiSelect = false;
            this.lv_backgrounds.Name = "lv_backgrounds";
            this.lv_backgrounds.Size = new System.Drawing.Size(367, 288);
            this.lv_backgrounds.TabIndex = 1;
            this.lv_backgrounds.UseCompatibleStateImageBehavior = false;
            this.lv_backgrounds.View = System.Windows.Forms.View.Details;
            this.lv_backgrounds.SizeChanged += new System.EventHandler(this.lv_backgrounds_SizeChanged);
            // 
            // columnHeader1
            // 
            this.columnHeader1.Text = "Prefab name";
            this.columnHeader1.Width = 300;
            // 
            // BkgSelector
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(383, 304);
            this.Controls.Add(this.lv_backgrounds);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedToolWindow;
            this.Name = "BkgSelector";
            this.Text = "BkgSelector";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.BkgSelector_FormClosing);
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.ListView lv_backgrounds;
        private System.Windows.Forms.ColumnHeader columnHeader1;
    }
}