namespace HexxEditor
{
    partial class FormMissionType
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
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.radio_bomb = new System.Windows.Forms.RadioButton();
            this.radio_hostages = new System.Windows.Forms.RadioButton();
            this.radio_eliminate = new System.Windows.Forms.RadioButton();
            this.radio_arrest = new System.Windows.Forms.RadioButton();
            this.groupBox1.SuspendLayout();
            this.SuspendLayout();
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.radio_arrest);
            this.groupBox1.Controls.Add(this.radio_bomb);
            this.groupBox1.Controls.Add(this.radio_hostages);
            this.groupBox1.Controls.Add(this.radio_eliminate);
            this.groupBox1.Location = new System.Drawing.Point(12, 12);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(260, 114);
            this.groupBox1.TabIndex = 0;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "Mission Type";
            // 
            // radio_bomb
            // 
            this.radio_bomb.AutoSize = true;
            this.radio_bomb.Location = new System.Drawing.Point(6, 65);
            this.radio_bomb.Name = "radio_bomb";
            this.radio_bomb.Size = new System.Drawing.Size(89, 17);
            this.radio_bomb.TabIndex = 2;
            this.radio_bomb.TabStop = true;
            this.radio_bomb.Text = "Defuse Bomb";
            this.radio_bomb.UseVisualStyleBackColor = true;
            // 
            // radio_hostages
            // 
            this.radio_hostages.AutoSize = true;
            this.radio_hostages.Location = new System.Drawing.Point(6, 42);
            this.radio_hostages.Name = "radio_hostages";
            this.radio_hostages.Size = new System.Drawing.Size(116, 17);
            this.radio_hostages.TabIndex = 1;
            this.radio_hostages.TabStop = true;
            this.radio_hostages.Text = "Save the Hostages";
            this.radio_hostages.UseVisualStyleBackColor = true;
            // 
            // radio_eliminate
            // 
            this.radio_eliminate.AutoSize = true;
            this.radio_eliminate.Location = new System.Drawing.Point(6, 19);
            this.radio_eliminate.Name = "radio_eliminate";
            this.radio_eliminate.Size = new System.Drawing.Size(68, 17);
            this.radio_eliminate.TabIndex = 0;
            this.radio_eliminate.TabStop = true;
            this.radio_eliminate.Text = "Kill\'em All";
            this.radio_eliminate.UseVisualStyleBackColor = true;
            // 
            // radio_arrest
            // 
            this.radio_arrest.AutoSize = true;
            this.radio_arrest.Location = new System.Drawing.Point(6, 88);
            this.radio_arrest.Name = "radio_arrest";
            this.radio_arrest.Size = new System.Drawing.Size(93, 17);
            this.radio_arrest.TabIndex = 3;
            this.radio_arrest.TabStop = true;
            this.radio_arrest.Text = "Arrest Warrant";
            this.radio_arrest.UseVisualStyleBackColor = true;
            // 
            // FormMissionType
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(284, 138);
            this.Controls.Add(this.groupBox1);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedToolWindow;
            this.Name = "FormMissionType";
            this.Text = "Set Mission Type";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.FormMissionType_FormClosing);
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.RadioButton radio_bomb;
        private System.Windows.Forms.RadioButton radio_hostages;
        private System.Windows.Forms.RadioButton radio_eliminate;
        private System.Windows.Forms.RadioButton radio_arrest;
    }
}