namespace HexxEditor
{
    partial class MetaWnd
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
            this.tb_labels = new System.Windows.Forms.TextBox();
            this.label1 = new System.Windows.Forms.Label();
            this.SuspendLayout();
            // 
            // tb_labels
            // 
            this.tb_labels.HideSelection = false;
            this.tb_labels.Location = new System.Drawing.Point(12, 29);
            this.tb_labels.Multiline = true;
            this.tb_labels.Name = "tb_labels";
            this.tb_labels.ScrollBars = System.Windows.Forms.ScrollBars.Vertical;
            this.tb_labels.Size = new System.Drawing.Size(359, 60);
            this.tb_labels.TabIndex = 0;
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(13, 13);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(154, 13);
            this.label1.TabIndex = 1;
            this.label1.Text = "Area TAGS (comma separated)";
            // 
            // MetaWnd
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(383, 304);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.tb_labels);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedToolWindow;
            this.Name = "MetaWnd";
            this.Text = "Level Meta Data";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.BkgSelector_FormClosing);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.TextBox tb_labels;
        private System.Windows.Forms.Label label1;
    }
}