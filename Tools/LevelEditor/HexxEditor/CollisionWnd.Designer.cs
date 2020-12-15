namespace HexxEditor
{
    partial class CollisionWnd
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
            this.groupBox_collisions = new System.Windows.Forms.GroupBox();
            this.label3 = new System.Windows.Forms.Label();
            this.combo_CollType = new System.Windows.Forms.ComboBox();
            this.chk_castShadows = new System.Windows.Forms.CheckBox();
            this.chk_shrinkByAxis = new System.Windows.Forms.CheckBox();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.chk_hideTriggers = new System.Windows.Forms.CheckBox();
            this.chk_hideWater = new System.Windows.Forms.CheckBox();
            this.chk_hideFOW = new System.Windows.Forms.CheckBox();
            this.groupBox_collisions.SuspendLayout();
            this.groupBox1.SuspendLayout();
            this.SuspendLayout();
            // 
            // groupBox_collisions
            // 
            this.groupBox_collisions.Controls.Add(this.label3);
            this.groupBox_collisions.Controls.Add(this.combo_CollType);
            this.groupBox_collisions.Controls.Add(this.chk_castShadows);
            this.groupBox_collisions.Location = new System.Drawing.Point(12, 12);
            this.groupBox_collisions.Name = "groupBox_collisions";
            this.groupBox_collisions.Size = new System.Drawing.Size(260, 74);
            this.groupBox_collisions.TabIndex = 0;
            this.groupBox_collisions.TabStop = false;
            this.groupBox_collisions.Text = "Collision Data";
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(30, 20);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(31, 13);
            this.label3.TabIndex = 11;
            this.label3.Text = "Type";
            // 
            // combo_CollType
            // 
            this.combo_CollType.FormattingEnabled = true;
            this.combo_CollType.Items.AddRange(new object[] {
            "Solid",
            "Stairs",
            "Water",
            "Ladder",
            "Box",
            "Trigger",
            "Particle System",
            "Room Occluder",
            "Cover",
            "Moving Platform",
            "Ledge (solid box)"});
            this.combo_CollType.Location = new System.Drawing.Point(67, 17);
            this.combo_CollType.Name = "combo_CollType";
            this.combo_CollType.Size = new System.Drawing.Size(187, 21);
            this.combo_CollType.TabIndex = 10;
            this.combo_CollType.SelectedIndexChanged += new System.EventHandler(this.combo_CollType_SelectedIndexChanged);
            this.combo_CollType.SelectionChangeCommitted += new System.EventHandler(this.combo_CollType_SelectionChangeCommitted);
            // 
            // chk_castShadows
            // 
            this.chk_castShadows.AutoSize = true;
            this.chk_castShadows.Location = new System.Drawing.Point(6, 44);
            this.chk_castShadows.Name = "chk_castShadows";
            this.chk_castShadows.Size = new System.Drawing.Size(94, 17);
            this.chk_castShadows.TabIndex = 2;
            this.chk_castShadows.Text = "Cast Shadows";
            this.chk_castShadows.UseVisualStyleBackColor = true;
            this.chk_castShadows.CheckedChanged += new System.EventHandler(this.chk_castShadows_CheckedChanged);
            // 
            // chk_shrinkByAxis
            // 
            this.chk_shrinkByAxis.AutoSize = true;
            this.chk_shrinkByAxis.Checked = true;
            this.chk_shrinkByAxis.CheckState = System.Windows.Forms.CheckState.Checked;
            this.chk_shrinkByAxis.Location = new System.Drawing.Point(17, 224);
            this.chk_shrinkByAxis.Name = "chk_shrinkByAxis";
            this.chk_shrinkByAxis.Size = new System.Drawing.Size(97, 17);
            this.chk_shrinkByAxis.TabIndex = 3;
            this.chk_shrinkByAxis.Text = "Auto fit to walls";
            this.chk_shrinkByAxis.UseVisualStyleBackColor = true;
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.chk_hideTriggers);
            this.groupBox1.Controls.Add(this.chk_hideWater);
            this.groupBox1.Location = new System.Drawing.Point(12, 92);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(260, 89);
            this.groupBox1.TabIndex = 4;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "Hide";
            // 
            // chk_hideTriggers
            // 
            this.chk_hideTriggers.AutoSize = true;
            this.chk_hideTriggers.Location = new System.Drawing.Point(5, 65);
            this.chk_hideTriggers.Name = "chk_hideTriggers";
            this.chk_hideTriggers.Size = new System.Drawing.Size(64, 17);
            this.chk_hideTriggers.TabIndex = 6;
            this.chk_hideTriggers.Text = "Triggers";
            this.chk_hideTriggers.UseVisualStyleBackColor = true;
            // 
            // chk_hideWater
            // 
            this.chk_hideWater.AutoSize = true;
            this.chk_hideWater.Location = new System.Drawing.Point(5, 42);
            this.chk_hideWater.Name = "chk_hideWater";
            this.chk_hideWater.Size = new System.Drawing.Size(55, 17);
            this.chk_hideWater.TabIndex = 5;
            this.chk_hideWater.Text = "Water";
            this.chk_hideWater.UseVisualStyleBackColor = true;
            // 
            // chk_hideFOW
            // 
            this.chk_hideFOW.AutoSize = true;
            this.chk_hideFOW.Location = new System.Drawing.Point(17, 111);
            this.chk_hideFOW.Name = "chk_hideFOW";
            this.chk_hideFOW.Size = new System.Drawing.Size(79, 17);
            this.chk_hideFOW.TabIndex = 0;
            this.chk_hideFOW.Text = "Fog of War";
            this.chk_hideFOW.UseVisualStyleBackColor = true;
            // 
            // CollisionWnd
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(284, 253);
            this.Controls.Add(this.chk_hideFOW);
            this.Controls.Add(this.groupBox1);
            this.Controls.Add(this.chk_shrinkByAxis);
            this.Controls.Add(this.groupBox_collisions);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedToolWindow;
            this.Name = "CollisionWnd";
            this.Text = "Collisions";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.CollisionWnd_FormClosing);
            this.groupBox_collisions.ResumeLayout(false);
            this.groupBox_collisions.PerformLayout();
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.GroupBox groupBox_collisions;
        private System.Windows.Forms.CheckBox chk_castShadows;
        private System.Windows.Forms.ComboBox combo_CollType;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.CheckBox chk_shrinkByAxis;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.CheckBox chk_hideWater;
        private System.Windows.Forms.CheckBox chk_hideFOW;
        private System.Windows.Forms.CheckBox chk_hideTriggers;
    }
}