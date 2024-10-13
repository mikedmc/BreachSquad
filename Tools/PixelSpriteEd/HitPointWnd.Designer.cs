namespace InkEd3
{
    partial class HitPointWnd
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
            this.butOK = new System.Windows.Forms.Button();
            this.label1 = new System.Windows.Forms.Label();
            this.label2 = new System.Windows.Forms.Label();
            this.label3 = new System.Windows.Forms.Label();
            this.tbHitptX = new System.Windows.Forms.TextBox();
            this.tbHitptY = new System.Windows.Forms.TextBox();
            this.tbHitptFlags = new System.Windows.Forms.TextBox();
            this.SuspendLayout();
            // 
            // butOK
            // 
            this.butOK.Location = new System.Drawing.Point(69, 90);
            this.butOK.Name = "butOK";
            this.butOK.Size = new System.Drawing.Size(75, 23);
            this.butOK.TabIndex = 0;
            this.butOK.Text = "OK";
            this.butOK.UseVisualStyleBackColor = true;
            this.butOK.Click += new System.EventHandler(this.butOK_Click);
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(9, 12);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(54, 13);
            this.label1.TabIndex = 4;
            this.label1.Text = "Position X";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(9, 38);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(54, 13);
            this.label2.TabIndex = 5;
            this.label2.Text = "Position Y";
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(31, 64);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(32, 13);
            this.label3.TabIndex = 6;
            this.label3.Text = "Flags";
            // 
            // tbHitptX
            // 
            this.tbHitptX.Location = new System.Drawing.Point(69, 9);
            this.tbHitptX.Name = "tbHitptX";
            this.tbHitptX.Size = new System.Drawing.Size(125, 20);
            this.tbHitptX.TabIndex = 7;
            // 
            // tbHitptY
            // 
            this.tbHitptY.Location = new System.Drawing.Point(69, 35);
            this.tbHitptY.Name = "tbHitptY";
            this.tbHitptY.Size = new System.Drawing.Size(125, 20);
            this.tbHitptY.TabIndex = 8;
            // 
            // tbHitptFlags
            // 
            this.tbHitptFlags.Location = new System.Drawing.Point(69, 61);
            this.tbHitptFlags.Name = "tbHitptFlags";
            this.tbHitptFlags.Size = new System.Drawing.Size(125, 20);
            this.tbHitptFlags.TabIndex = 9;
            // 
            // HitPointWnd
            // 
            this.AcceptButton = this.butOK;
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(206, 121);
            this.ControlBox = false;
            this.Controls.Add(this.tbHitptFlags);
            this.Controls.Add(this.tbHitptY);
            this.Controls.Add(this.tbHitptX);
            this.Controls.Add(this.label3);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.butOK);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedToolWindow;
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "HitPointWnd";
            this.ShowIcon = false;
            this.ShowInTaskbar = false;
            this.SizeGripStyle = System.Windows.Forms.SizeGripStyle.Hide;
            this.Text = "HitPoint Settings";
            this.TopMost = true;
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Button butOK;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.TextBox tbHitptX;
        private System.Windows.Forms.TextBox tbHitptY;
        private System.Windows.Forms.TextBox tbHitptFlags;
    }
}