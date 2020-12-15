namespace HexxEditor
{
    partial class AIwnd
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
            this.groupBehavior = new System.Windows.Forms.GroupBox();
            this.combo_AI = new System.Windows.Forms.ComboBox();
            this.text_AIparams = new System.Windows.Forms.TextBox();
            this.label4 = new System.Windows.Forms.Label();
            this.groupScript = new System.Windows.Forms.GroupBox();
            this.butHelpTimer = new System.Windows.Forms.Button();
            this.label1 = new System.Windows.Forms.Label();
            this.num_interactTimer = new System.Windows.Forms.NumericUpDown();
            this.chk_startHidden = new System.Windows.Forms.CheckBox();
            this.chk_canInteract = new System.Windows.Forms.CheckBox();
            this.text_scriptName = new System.Windows.Forms.TextBox();
            this.label2 = new System.Windows.Forms.Label();
            this.num_targetID = new System.Windows.Forms.NumericUpDown();
            this.label3 = new System.Windows.Forms.Label();
            this.splitContainer1 = new System.Windows.Forms.SplitContainer();
            this.chk_hideInteract = new System.Windows.Forms.CheckBox();
            this.groupBehavior.SuspendLayout();
            this.groupScript.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.num_interactTimer)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.num_targetID)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.splitContainer1)).BeginInit();
            this.splitContainer1.Panel1.SuspendLayout();
            this.splitContainer1.Panel2.SuspendLayout();
            this.splitContainer1.SuspendLayout();
            this.SuspendLayout();
            // 
            // groupBehavior
            // 
            this.groupBehavior.Controls.Add(this.combo_AI);
            this.groupBehavior.Controls.Add(this.text_AIparams);
            this.groupBehavior.Controls.Add(this.label4);
            this.groupBehavior.Dock = System.Windows.Forms.DockStyle.Fill;
            this.groupBehavior.Location = new System.Drawing.Point(0, 0);
            this.groupBehavior.Name = "groupBehavior";
            this.groupBehavior.Size = new System.Drawing.Size(382, 177);
            this.groupBehavior.TabIndex = 10;
            this.groupBehavior.TabStop = false;
            this.groupBehavior.Text = "Behavior / AI";
            // 
            // combo_AI
            // 
            this.combo_AI.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.combo_AI.FormattingEnabled = true;
            this.combo_AI.Location = new System.Drawing.Point(6, 19);
            this.combo_AI.Name = "combo_AI";
            this.combo_AI.Size = new System.Drawing.Size(373, 21);
            this.combo_AI.TabIndex = 2;
            this.combo_AI.SelectedIndexChanged += new System.EventHandler(this.combo_AI_SelectedIndexChanged);
            // 
            // text_AIparams
            // 
            this.text_AIparams.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.text_AIparams.Location = new System.Drawing.Point(6, 62);
            this.text_AIparams.Multiline = true;
            this.text_AIparams.Name = "text_AIparams";
            this.text_AIparams.ScrollBars = System.Windows.Forms.ScrollBars.Both;
            this.text_AIparams.Size = new System.Drawing.Size(373, 109);
            this.text_AIparams.TabIndex = 12;
            this.text_AIparams.TextChanged += new System.EventHandler(this.text_AIparams_TextChanged);
            // 
            // label4
            // 
            this.label4.AutoSize = true;
            this.label4.Location = new System.Drawing.Point(6, 46);
            this.label4.Name = "label4";
            this.label4.Size = new System.Drawing.Size(210, 13);
            this.label4.TabIndex = 7;
            this.label4.Text = "params format: a=10;b=25;str=the lazy dog;";
            // 
            // groupScript
            // 
            this.groupScript.Controls.Add(this.chk_hideInteract);
            this.groupScript.Controls.Add(this.butHelpTimer);
            this.groupScript.Controls.Add(this.label1);
            this.groupScript.Controls.Add(this.num_interactTimer);
            this.groupScript.Controls.Add(this.chk_startHidden);
            this.groupScript.Controls.Add(this.chk_canInteract);
            this.groupScript.Controls.Add(this.text_scriptName);
            this.groupScript.Controls.Add(this.label2);
            this.groupScript.Controls.Add(this.num_targetID);
            this.groupScript.Controls.Add(this.label3);
            this.groupScript.Dock = System.Windows.Forms.DockStyle.Fill;
            this.groupScript.Location = new System.Drawing.Point(0, 0);
            this.groupScript.Name = "groupScript";
            this.groupScript.Size = new System.Drawing.Size(382, 166);
            this.groupScript.TabIndex = 11;
            this.groupScript.TabStop = false;
            this.groupScript.Text = "Script";
            // 
            // butHelpTimer
            // 
            this.butHelpTimer.Location = new System.Drawing.Point(244, 69);
            this.butHelpTimer.Name = "butHelpTimer";
            this.butHelpTimer.Size = new System.Drawing.Size(21, 20);
            this.butHelpTimer.TabIndex = 11;
            this.butHelpTimer.Text = "?";
            this.butHelpTimer.UseVisualStyleBackColor = true;
            this.butHelpTimer.Click += new System.EventHandler(this.butHelpTimer_Click);
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(148, 73);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(90, 13);
            this.label1.TabIndex = 10;
            this.label1.Text = "interact timer(sec)";
            // 
            // num_interactTimer
            // 
            this.num_interactTimer.Location = new System.Drawing.Point(88, 71);
            this.num_interactTimer.Minimum = new decimal(new int[] {
            100,
            0,
            0,
            -2147483648});
            this.num_interactTimer.Name = "num_interactTimer";
            this.num_interactTimer.Size = new System.Drawing.Size(57, 20);
            this.num_interactTimer.TabIndex = 9;
            this.num_interactTimer.ValueChanged += new System.EventHandler(this.num_interactTimer_ValueChanged);
            // 
            // chk_startHidden
            // 
            this.chk_startHidden.AutoSize = true;
            this.chk_startHidden.Location = new System.Drawing.Point(151, 48);
            this.chk_startHidden.Name = "chk_startHidden";
            this.chk_startHidden.Size = new System.Drawing.Size(60, 17);
            this.chk_startHidden.TabIndex = 8;
            this.chk_startHidden.Text = "Hidden";
            this.chk_startHidden.UseVisualStyleBackColor = true;
            this.chk_startHidden.CheckedChanged += new System.EventHandler(this.chk_startHidden_CheckedChanged);
            // 
            // chk_canInteract
            // 
            this.chk_canInteract.AutoSize = true;
            this.chk_canInteract.ForeColor = System.Drawing.Color.Red;
            this.chk_canInteract.Location = new System.Drawing.Point(9, 72);
            this.chk_canInteract.Name = "chk_canInteract";
            this.chk_canInteract.Size = new System.Drawing.Size(83, 17);
            this.chk_canInteract.TabIndex = 7;
            this.chk_canInteract.Text = "Can interact";
            this.chk_canInteract.UseVisualStyleBackColor = true;
            this.chk_canInteract.CheckedChanged += new System.EventHandler(this.chk_canInteract_CheckedChanged);
            // 
            // text_scriptName
            // 
            this.text_scriptName.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.text_scriptName.Location = new System.Drawing.Point(64, 19);
            this.text_scriptName.Name = "text_scriptName";
            this.text_scriptName.Size = new System.Drawing.Size(312, 20);
            this.text_scriptName.TabIndex = 4;
            this.text_scriptName.TextChanged += new System.EventHandler(this.text_scriptName_TextChanged);
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(24, 22);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(34, 13);
            this.label2.TabIndex = 3;
            this.label2.Text = "Script";
            // 
            // num_targetID
            // 
            this.num_targetID.Location = new System.Drawing.Point(64, 45);
            this.num_targetID.Maximum = new decimal(new int[] {
            10000,
            0,
            0,
            0});
            this.num_targetID.Minimum = new decimal(new int[] {
            1,
            0,
            0,
            -2147483648});
            this.num_targetID.Name = "num_targetID";
            this.num_targetID.Size = new System.Drawing.Size(81, 20);
            this.num_targetID.TabIndex = 6;
            this.num_targetID.Value = new decimal(new int[] {
            1,
            0,
            0,
            -2147483648});
            this.num_targetID.ValueChanged += new System.EventHandler(this.num_targetID_ValueChanged);
            // 
            // label3
            // 
            this.label3.AutoSize = true;
            this.label3.Location = new System.Drawing.Point(6, 47);
            this.label3.Name = "label3";
            this.label3.Size = new System.Drawing.Size(52, 13);
            this.label3.TabIndex = 5;
            this.label3.Text = "Target ID";
            // 
            // splitContainer1
            // 
            this.splitContainer1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.splitContainer1.Location = new System.Drawing.Point(0, 0);
            this.splitContainer1.Name = "splitContainer1";
            this.splitContainer1.Orientation = System.Windows.Forms.Orientation.Horizontal;
            // 
            // splitContainer1.Panel1
            // 
            this.splitContainer1.Panel1.Controls.Add(this.groupBehavior);
            // 
            // splitContainer1.Panel2
            // 
            this.splitContainer1.Panel2.Controls.Add(this.groupScript);
            this.splitContainer1.Size = new System.Drawing.Size(382, 347);
            this.splitContainer1.SplitterDistance = 177;
            this.splitContainer1.TabIndex = 12;
            // 
            // chk_hideInteract
            // 
            this.chk_hideInteract.AutoSize = true;
            this.chk_hideInteract.Location = new System.Drawing.Point(9, 95);
            this.chk_hideInteract.Name = "chk_hideInteract";
            this.chk_hideInteract.Size = new System.Drawing.Size(111, 17);
            this.chk_hideInteract.TabIndex = 12;
            this.chk_hideInteract.Text = "Hide Interact Icon";
            this.chk_hideInteract.UseVisualStyleBackColor = true;
            this.chk_hideInteract.CheckedChanged += new System.EventHandler(this.chk_hideInteract_CheckedChanged);
            // 
            // AIwnd
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(382, 347);
            this.Controls.Add(this.splitContainer1);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.SizableToolWindow;
            this.Name = "AIwnd";
            this.Text = "AI";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.AIwnd_FormClosing);
            this.groupBehavior.ResumeLayout(false);
            this.groupBehavior.PerformLayout();
            this.groupScript.ResumeLayout(false);
            this.groupScript.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.num_interactTimer)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.num_targetID)).EndInit();
            this.splitContainer1.Panel1.ResumeLayout(false);
            this.splitContainer1.Panel2.ResumeLayout(false);
            ((System.ComponentModel.ISupportInitialize)(this.splitContainer1)).EndInit();
            this.splitContainer1.ResumeLayout(false);
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.GroupBox groupBehavior;
        private System.Windows.Forms.Label label4;
        private System.Windows.Forms.GroupBox groupScript;
        private System.Windows.Forms.TextBox text_scriptName;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.NumericUpDown num_targetID;
        private System.Windows.Forms.Label label3;
        private System.Windows.Forms.CheckBox chk_canInteract;
        private System.Windows.Forms.ComboBox combo_AI;
        private System.Windows.Forms.TextBox text_AIparams;
        private System.Windows.Forms.CheckBox chk_startHidden;
        private System.Windows.Forms.SplitContainer splitContainer1;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.NumericUpDown num_interactTimer;
        private System.Windows.Forms.Button butHelpTimer;
        private System.Windows.Forms.CheckBox chk_hideInteract;
    }
}