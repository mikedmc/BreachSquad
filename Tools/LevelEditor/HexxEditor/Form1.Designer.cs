namespace HexxEditor
{
    partial class Form1
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
            this.components = new System.ComponentModel.Container();
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(Form1));
            this.pictureBox1 = new System.Windows.Forms.PictureBox();
            this.menuStrip1 = new System.Windows.Forms.MenuStrip();
            this.fileToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.newToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripMenuItem1 = new System.Windows.Forms.ToolStripSeparator();
            this.openToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.saveToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.saveAsToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripMenuItem6 = new System.Windows.Forms.ToolStripSeparator();
            this.openPrefabToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.exportPrefabToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripMenuItem5 = new System.Windows.Forms.ToolStripSeparator();
            this.exitToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.toolsToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.undoToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripMenuItem7 = new System.Windows.Forms.ToolStripSeparator();
            this.setMissionTypeToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.setLevelBackgroundToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.setGroundLevelToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripMenuItem3 = new System.Windows.Forms.ToolStripSeparator();
            this.findByIDToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.menuTextBoxIDFinder = new System.Windows.Forms.ToolStripTextBox();
            this.findItNowToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripMenuItem2 = new System.Windows.Forms.ToolStripSeparator();
            this.clearResultToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripMenuItem4 = new System.Windows.Forms.ToolStripSeparator();
            this.deleteOverlappingTilesToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.clearAllActorsToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.viewToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.invertBackgroundToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.showGridToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.frontLayerGridToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.workshopToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.uploadSingleLevelToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStripMenuItem8 = new System.Windows.Forms.ToolStripSeparator();
            this.playMapToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.helpToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.aboutToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.butWndMaterials = new System.Windows.Forms.Button();
            this.butWndLights = new System.Windows.Forms.Button();
            this.butWndCollision = new System.Windows.Forms.Button();
            this.butWndObjects = new System.Windows.Forms.Button();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.radio_layer0 = new System.Windows.Forms.RadioButton();
            this.chk_layer0 = new System.Windows.Forms.CheckBox();
            this.radio_layer6 = new System.Windows.Forms.RadioButton();
            this.chk_layer6 = new System.Windows.Forms.CheckBox();
            this.radio_layer5 = new System.Windows.Forms.RadioButton();
            this.chk_layer5 = new System.Windows.Forms.CheckBox();
            this.radio_layer4 = new System.Windows.Forms.RadioButton();
            this.chk_layer4 = new System.Windows.Forms.CheckBox();
            this.radio_layer3 = new System.Windows.Forms.RadioButton();
            this.chk_layer3 = new System.Windows.Forms.CheckBox();
            this.radio_layer2 = new System.Windows.Forms.RadioButton();
            this.radio_layer1 = new System.Windows.Forms.RadioButton();
            this.chk_layer2 = new System.Windows.Forms.CheckBox();
            this.chk_layer1 = new System.Windows.Forms.CheckBox();
            this.chk_showOverlappingTiles = new System.Windows.Forms.CheckBox();
            this.butWndAI = new System.Windows.Forms.Button();
            this.butWndMisc = new System.Windows.Forms.Button();
            this.butWndActors = new System.Windows.Forms.Button();
            this.statusStrip1 = new System.Windows.Forms.StatusStrip();
            this.toolStripStatusLabel1 = new System.Windows.Forms.ToolStripStatusLabel();
            this.tableLayoutPanel1 = new System.Windows.Forms.TableLayoutPanel();
            this.panel1 = new System.Windows.Forms.Panel();
            this.groupBox2 = new System.Windows.Forms.GroupBox();
            this.chk_snapToGrid = new System.Windows.Forms.CheckBox();
            this.butWndPrefabs = new System.Windows.Forms.Button();
            this.timer_autosave = new System.Windows.Forms.Timer(this.components);
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).BeginInit();
            this.menuStrip1.SuspendLayout();
            this.groupBox1.SuspendLayout();
            this.statusStrip1.SuspendLayout();
            this.tableLayoutPanel1.SuspendLayout();
            this.panel1.SuspendLayout();
            this.groupBox2.SuspendLayout();
            this.SuspendLayout();
            // 
            // pictureBox1
            // 
            this.pictureBox1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.pictureBox1.Location = new System.Drawing.Point(3, 3);
            this.pictureBox1.Name = "pictureBox1";
            this.pictureBox1.Size = new System.Drawing.Size(1021, 774);
            this.pictureBox1.TabIndex = 0;
            this.pictureBox1.TabStop = false;
            this.pictureBox1.SizeChanged += new System.EventHandler(this.pictureBox1_SizeChanged);
            this.pictureBox1.MouseDown += new System.Windows.Forms.MouseEventHandler(this.pictureBox1_MouseDown);
            this.pictureBox1.MouseMove += new System.Windows.Forms.MouseEventHandler(this.pictureBox1_MouseMove);
            this.pictureBox1.MouseUp += new System.Windows.Forms.MouseEventHandler(this.pictureBox1_MouseUp);
            this.pictureBox1.MouseWheel += new System.Windows.Forms.MouseEventHandler(this.pictureBox1_MouseWheel);
            // 
            // menuStrip1
            // 
            this.menuStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.fileToolStripMenuItem,
            this.toolsToolStripMenuItem,
            this.viewToolStripMenuItem,
            this.workshopToolStripMenuItem,
            this.helpToolStripMenuItem});
            this.menuStrip1.Location = new System.Drawing.Point(0, 0);
            this.menuStrip1.Name = "menuStrip1";
            this.menuStrip1.Size = new System.Drawing.Size(1187, 24);
            this.menuStrip1.TabIndex = 1;
            this.menuStrip1.Text = "menuStrip1";
            // 
            // fileToolStripMenuItem
            // 
            this.fileToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.newToolStripMenuItem,
            this.toolStripMenuItem1,
            this.openToolStripMenuItem,
            this.saveToolStripMenuItem,
            this.saveAsToolStripMenuItem,
            this.toolStripMenuItem6,
            this.openPrefabToolStripMenuItem,
            this.exportPrefabToolStripMenuItem,
            this.toolStripMenuItem5,
            this.exitToolStripMenuItem});
            this.fileToolStripMenuItem.Name = "fileToolStripMenuItem";
            this.fileToolStripMenuItem.Size = new System.Drawing.Size(37, 20);
            this.fileToolStripMenuItem.Text = "File";
            // 
            // newToolStripMenuItem
            // 
            this.newToolStripMenuItem.Name = "newToolStripMenuItem";
            this.newToolStripMenuItem.Size = new System.Drawing.Size(154, 22);
            this.newToolStripMenuItem.Text = "New";
            this.newToolStripMenuItem.Click += new System.EventHandler(this.newToolStripMenuItem_Click);
            // 
            // toolStripMenuItem1
            // 
            this.toolStripMenuItem1.Name = "toolStripMenuItem1";
            this.toolStripMenuItem1.Size = new System.Drawing.Size(151, 6);
            // 
            // openToolStripMenuItem
            // 
            this.openToolStripMenuItem.Name = "openToolStripMenuItem";
            this.openToolStripMenuItem.Size = new System.Drawing.Size(154, 22);
            this.openToolStripMenuItem.Text = "&Open...";
            this.openToolStripMenuItem.Click += new System.EventHandler(this.openToolStripMenuItem_Click);
            // 
            // saveToolStripMenuItem
            // 
            this.saveToolStripMenuItem.Name = "saveToolStripMenuItem";
            this.saveToolStripMenuItem.Size = new System.Drawing.Size(154, 22);
            this.saveToolStripMenuItem.Text = "&Save";
            this.saveToolStripMenuItem.Click += new System.EventHandler(this.saveToolStripMenuItem_Click);
            // 
            // saveAsToolStripMenuItem
            // 
            this.saveAsToolStripMenuItem.Name = "saveAsToolStripMenuItem";
            this.saveAsToolStripMenuItem.Size = new System.Drawing.Size(154, 22);
            this.saveAsToolStripMenuItem.Text = "Save &As...";
            this.saveAsToolStripMenuItem.Click += new System.EventHandler(this.saveAsToolStripMenuItem_Click);
            // 
            // toolStripMenuItem6
            // 
            this.toolStripMenuItem6.Name = "toolStripMenuItem6";
            this.toolStripMenuItem6.Size = new System.Drawing.Size(151, 6);
            // 
            // openPrefabToolStripMenuItem
            // 
            this.openPrefabToolStripMenuItem.Name = "openPrefabToolStripMenuItem";
            this.openPrefabToolStripMenuItem.Size = new System.Drawing.Size(154, 22);
            this.openPrefabToolStripMenuItem.Text = "Edit Prefab...";
            this.openPrefabToolStripMenuItem.Click += new System.EventHandler(this.openPrefabToolStripMenuItem_Click);
            // 
            // exportPrefabToolStripMenuItem
            // 
            this.exportPrefabToolStripMenuItem.Name = "exportPrefabToolStripMenuItem";
            this.exportPrefabToolStripMenuItem.Size = new System.Drawing.Size(154, 22);
            this.exportPrefabToolStripMenuItem.Text = "Export Prefab...";
            this.exportPrefabToolStripMenuItem.Click += new System.EventHandler(this.exportPrefabToolStripMenuItem_Click);
            // 
            // toolStripMenuItem5
            // 
            this.toolStripMenuItem5.Name = "toolStripMenuItem5";
            this.toolStripMenuItem5.Size = new System.Drawing.Size(151, 6);
            // 
            // exitToolStripMenuItem
            // 
            this.exitToolStripMenuItem.Name = "exitToolStripMenuItem";
            this.exitToolStripMenuItem.Size = new System.Drawing.Size(154, 22);
            this.exitToolStripMenuItem.Text = "E&xit";
            this.exitToolStripMenuItem.Click += new System.EventHandler(this.exitToolStripMenuItem_Click);
            // 
            // toolsToolStripMenuItem
            // 
            this.toolsToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.undoToolStripMenuItem,
            this.toolStripMenuItem7,
            this.setMissionTypeToolStripMenuItem,
            this.setLevelBackgroundToolStripMenuItem,
            this.setGroundLevelToolStripMenuItem,
            this.toolStripMenuItem3,
            this.findByIDToolStripMenuItem,
            this.toolStripMenuItem4,
            this.deleteOverlappingTilesToolStripMenuItem,
            this.clearAllActorsToolStripMenuItem});
            this.toolsToolStripMenuItem.Name = "toolsToolStripMenuItem";
            this.toolsToolStripMenuItem.Size = new System.Drawing.Size(39, 20);
            this.toolsToolStripMenuItem.Text = "Edit";
            // 
            // undoToolStripMenuItem
            // 
            this.undoToolStripMenuItem.Enabled = false;
            this.undoToolStripMenuItem.Name = "undoToolStripMenuItem";
            this.undoToolStripMenuItem.Size = new System.Drawing.Size(197, 22);
            this.undoToolStripMenuItem.Text = "Undo";
            this.undoToolStripMenuItem.Click += new System.EventHandler(this.undoToolStripMenuItem_Click);
            // 
            // toolStripMenuItem7
            // 
            this.toolStripMenuItem7.Name = "toolStripMenuItem7";
            this.toolStripMenuItem7.Size = new System.Drawing.Size(194, 6);
            // 
            // setMissionTypeToolStripMenuItem
            // 
            this.setMissionTypeToolStripMenuItem.Name = "setMissionTypeToolStripMenuItem";
            this.setMissionTypeToolStripMenuItem.Size = new System.Drawing.Size(197, 22);
            this.setMissionTypeToolStripMenuItem.Text = "Set Mission Type...";
            this.setMissionTypeToolStripMenuItem.Click += new System.EventHandler(this.setMissionTypeToolStripMenuItem_Click);
            // 
            // setLevelBackgroundToolStripMenuItem
            // 
            this.setLevelBackgroundToolStripMenuItem.Name = "setLevelBackgroundToolStripMenuItem";
            this.setLevelBackgroundToolStripMenuItem.Size = new System.Drawing.Size(197, 22);
            this.setLevelBackgroundToolStripMenuItem.Text = "Set Level Background...";
            this.setLevelBackgroundToolStripMenuItem.Click += new System.EventHandler(this.setLevelBackgroundToolStripMenuItem_Click);
            // 
            // setGroundLevelToolStripMenuItem
            // 
            this.setGroundLevelToolStripMenuItem.Name = "setGroundLevelToolStripMenuItem";
            this.setGroundLevelToolStripMenuItem.Size = new System.Drawing.Size(197, 22);
            this.setGroundLevelToolStripMenuItem.Text = "Set Level/Prefab Origin";
            this.setGroundLevelToolStripMenuItem.Click += new System.EventHandler(this.setGroundLevelToolStripMenuItem_Click);
            // 
            // toolStripMenuItem3
            // 
            this.toolStripMenuItem3.Name = "toolStripMenuItem3";
            this.toolStripMenuItem3.Size = new System.Drawing.Size(194, 6);
            // 
            // findByIDToolStripMenuItem
            // 
            this.findByIDToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.menuTextBoxIDFinder,
            this.findItNowToolStripMenuItem,
            this.toolStripMenuItem2,
            this.clearResultToolStripMenuItem});
            this.findByIDToolStripMenuItem.Name = "findByIDToolStripMenuItem";
            this.findByIDToolStripMenuItem.Size = new System.Drawing.Size(197, 22);
            this.findByIDToolStripMenuItem.Text = "Find by ID...";
            // 
            // menuTextBoxIDFinder
            // 
            this.menuTextBoxIDFinder.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.menuTextBoxIDFinder.Name = "menuTextBoxIDFinder";
            this.menuTextBoxIDFinder.Size = new System.Drawing.Size(100, 23);
            this.menuTextBoxIDFinder.Text = "0";
            this.menuTextBoxIDFinder.KeyPress += new System.Windows.Forms.KeyPressEventHandler(this.menuTextBoxIDFinder_KeyPress);
            // 
            // findItNowToolStripMenuItem
            // 
            this.findItNowToolStripMenuItem.Name = "findItNowToolStripMenuItem";
            this.findItNowToolStripMenuItem.Size = new System.Drawing.Size(160, 22);
            this.findItNowToolStripMenuItem.Text = "Find it now!";
            this.findItNowToolStripMenuItem.Click += new System.EventHandler(this.findItNowToolStripMenuItem_Click);
            // 
            // toolStripMenuItem2
            // 
            this.toolStripMenuItem2.Name = "toolStripMenuItem2";
            this.toolStripMenuItem2.Size = new System.Drawing.Size(157, 6);
            // 
            // clearResultToolStripMenuItem
            // 
            this.clearResultToolStripMenuItem.Name = "clearResultToolStripMenuItem";
            this.clearResultToolStripMenuItem.Size = new System.Drawing.Size(160, 22);
            this.clearResultToolStripMenuItem.Text = "Clear result";
            this.clearResultToolStripMenuItem.Click += new System.EventHandler(this.clearResultToolStripMenuItem_Click);
            // 
            // toolStripMenuItem4
            // 
            this.toolStripMenuItem4.Name = "toolStripMenuItem4";
            this.toolStripMenuItem4.Size = new System.Drawing.Size(194, 6);
            // 
            // deleteOverlappingTilesToolStripMenuItem
            // 
            this.deleteOverlappingTilesToolStripMenuItem.Name = "deleteOverlappingTilesToolStripMenuItem";
            this.deleteOverlappingTilesToolStripMenuItem.Size = new System.Drawing.Size(197, 22);
            this.deleteOverlappingTilesToolStripMenuItem.Text = "Delete overlapping tiles";
            this.deleteOverlappingTilesToolStripMenuItem.Click += new System.EventHandler(this.deleteOverlappingTilesToolStripMenuItem_Click);
            // 
            // clearAllActorsToolStripMenuItem
            // 
            this.clearAllActorsToolStripMenuItem.Name = "clearAllActorsToolStripMenuItem";
            this.clearAllActorsToolStripMenuItem.Size = new System.Drawing.Size(197, 22);
            this.clearAllActorsToolStripMenuItem.Text = "Clear All Actors...";
            this.clearAllActorsToolStripMenuItem.Click += new System.EventHandler(this.clearAllActorsToolStripMenuItem_Click);
            // 
            // viewToolStripMenuItem
            // 
            this.viewToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.invertBackgroundToolStripMenuItem,
            this.showGridToolStripMenuItem,
            this.frontLayerGridToolStripMenuItem});
            this.viewToolStripMenuItem.Name = "viewToolStripMenuItem";
            this.viewToolStripMenuItem.Size = new System.Drawing.Size(44, 20);
            this.viewToolStripMenuItem.Text = "View";
            // 
            // invertBackgroundToolStripMenuItem
            // 
            this.invertBackgroundToolStripMenuItem.CheckOnClick = true;
            this.invertBackgroundToolStripMenuItem.Name = "invertBackgroundToolStripMenuItem";
            this.invertBackgroundToolStripMenuItem.Size = new System.Drawing.Size(171, 22);
            this.invertBackgroundToolStripMenuItem.Text = "&Invert Background";
            this.invertBackgroundToolStripMenuItem.Click += new System.EventHandler(this.invertBackgroundToolStripMenuItem_Click);
            // 
            // showGridToolStripMenuItem
            // 
            this.showGridToolStripMenuItem.Checked = true;
            this.showGridToolStripMenuItem.CheckOnClick = true;
            this.showGridToolStripMenuItem.CheckState = System.Windows.Forms.CheckState.Checked;
            this.showGridToolStripMenuItem.Name = "showGridToolStripMenuItem";
            this.showGridToolStripMenuItem.Size = new System.Drawing.Size(171, 22);
            this.showGridToolStripMenuItem.Text = "Show Grid";
            this.showGridToolStripMenuItem.Click += new System.EventHandler(this.showGridToolStripMenuItem_Click);
            // 
            // frontLayerGridToolStripMenuItem
            // 
            this.frontLayerGridToolStripMenuItem.Name = "frontLayerGridToolStripMenuItem";
            this.frontLayerGridToolStripMenuItem.Size = new System.Drawing.Size(171, 22);
            this.frontLayerGridToolStripMenuItem.Text = "Front Layer &Grid";
            // 
            // workshopToolStripMenuItem
            // 
            this.workshopToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.uploadSingleLevelToolStripMenuItem,
            this.toolStripMenuItem8,
            this.playMapToolStripMenuItem});
            this.workshopToolStripMenuItem.Name = "workshopToolStripMenuItem";
            this.workshopToolStripMenuItem.Size = new System.Drawing.Size(73, 20);
            this.workshopToolStripMenuItem.Text = "Workshop";
            // 
            // uploadSingleLevelToolStripMenuItem
            // 
            this.uploadSingleLevelToolStripMenuItem.Name = "uploadSingleLevelToolStripMenuItem";
            this.uploadSingleLevelToolStripMenuItem.Size = new System.Drawing.Size(159, 22);
            this.uploadSingleLevelToolStripMenuItem.Text = "Mod Uploader...";
            this.uploadSingleLevelToolStripMenuItem.Click += new System.EventHandler(this.uploadSingleLevelToolStripMenuItem_Click);
            // 
            // toolStripMenuItem8
            // 
            this.toolStripMenuItem8.Name = "toolStripMenuItem8";
            this.toolStripMenuItem8.Size = new System.Drawing.Size(156, 6);
            // 
            // playMapToolStripMenuItem
            // 
            this.playMapToolStripMenuItem.Name = "playMapToolStripMenuItem";
            this.playMapToolStripMenuItem.Size = new System.Drawing.Size(159, 22);
            this.playMapToolStripMenuItem.Text = "Play Map!";
            this.playMapToolStripMenuItem.Click += new System.EventHandler(this.playMapToolStripMenuItem_Click);
            // 
            // helpToolStripMenuItem
            // 
            this.helpToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.aboutToolStripMenuItem});
            this.helpToolStripMenuItem.Name = "helpToolStripMenuItem";
            this.helpToolStripMenuItem.Size = new System.Drawing.Size(44, 20);
            this.helpToolStripMenuItem.Text = "Help";
            // 
            // aboutToolStripMenuItem
            // 
            this.aboutToolStripMenuItem.Name = "aboutToolStripMenuItem";
            this.aboutToolStripMenuItem.Size = new System.Drawing.Size(116, 22);
            this.aboutToolStripMenuItem.Text = "About...";
            this.aboutToolStripMenuItem.Click += new System.EventHandler(this.aboutToolStripMenuItem_Click);
            // 
            // butWndMaterials
            // 
            this.butWndMaterials.Location = new System.Drawing.Point(1, 3);
            this.butWndMaterials.Name = "butWndMaterials";
            this.butWndMaterials.Size = new System.Drawing.Size(76, 28);
            this.butWndMaterials.TabIndex = 31;
            this.butWndMaterials.TabStop = false;
            this.butWndMaterials.Text = "Materials";
            this.butWndMaterials.UseVisualStyleBackColor = true;
            this.butWndMaterials.Click += new System.EventHandler(this.butWndMaterials_Click);
            // 
            // butWndLights
            // 
            this.butWndLights.Location = new System.Drawing.Point(78, 3);
            this.butWndLights.Name = "butWndLights";
            this.butWndLights.Size = new System.Drawing.Size(76, 28);
            this.butWndLights.TabIndex = 33;
            this.butWndLights.TabStop = false;
            this.butWndLights.Text = "Lights";
            this.butWndLights.UseVisualStyleBackColor = true;
            this.butWndLights.Click += new System.EventHandler(this.butWndLights_Click);
            // 
            // butWndCollision
            // 
            this.butWndCollision.Location = new System.Drawing.Point(1, 32);
            this.butWndCollision.Name = "butWndCollision";
            this.butWndCollision.Size = new System.Drawing.Size(76, 28);
            this.butWndCollision.TabIndex = 31;
            this.butWndCollision.TabStop = false;
            this.butWndCollision.Text = "Collision";
            this.butWndCollision.UseVisualStyleBackColor = true;
            this.butWndCollision.Click += new System.EventHandler(this.butWndCollision_Click);
            // 
            // butWndObjects
            // 
            this.butWndObjects.Location = new System.Drawing.Point(78, 32);
            this.butWndObjects.Name = "butWndObjects";
            this.butWndObjects.Size = new System.Drawing.Size(76, 28);
            this.butWndObjects.TabIndex = 33;
            this.butWndObjects.TabStop = false;
            this.butWndObjects.Text = "Objects";
            this.butWndObjects.UseVisualStyleBackColor = true;
            this.butWndObjects.Click += new System.EventHandler(this.butWndObjects_Click);
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.radio_layer0);
            this.groupBox1.Controls.Add(this.chk_layer0);
            this.groupBox1.Controls.Add(this.radio_layer6);
            this.groupBox1.Controls.Add(this.chk_layer6);
            this.groupBox1.Controls.Add(this.radio_layer5);
            this.groupBox1.Controls.Add(this.chk_layer5);
            this.groupBox1.Controls.Add(this.radio_layer4);
            this.groupBox1.Controls.Add(this.chk_layer4);
            this.groupBox1.Controls.Add(this.radio_layer3);
            this.groupBox1.Controls.Add(this.chk_layer3);
            this.groupBox1.Controls.Add(this.radio_layer2);
            this.groupBox1.Controls.Add(this.radio_layer1);
            this.groupBox1.Controls.Add(this.chk_layer2);
            this.groupBox1.Controls.Add(this.chk_layer1);
            this.groupBox1.Location = new System.Drawing.Point(3, 121);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(148, 184);
            this.groupBox1.TabIndex = 35;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "Layers";
            // 
            // radio_layer0
            // 
            this.radio_layer0.AutoSize = true;
            this.radio_layer0.Location = new System.Drawing.Point(25, 17);
            this.radio_layer0.Name = "radio_layer0";
            this.radio_layer0.Size = new System.Drawing.Size(80, 17);
            this.radio_layer0.TabIndex = 6;
            this.radio_layer0.Text = "Below Floor";
            this.radio_layer0.UseVisualStyleBackColor = true;
            this.radio_layer0.CheckedChanged += new System.EventHandler(this.radio_layer_CheckedChanged);
            // 
            // chk_layer0
            // 
            this.chk_layer0.AutoSize = true;
            this.chk_layer0.Checked = true;
            this.chk_layer0.CheckState = System.Windows.Forms.CheckState.Checked;
            this.chk_layer0.Location = new System.Drawing.Point(8, 19);
            this.chk_layer0.Name = "chk_layer0";
            this.chk_layer0.Size = new System.Drawing.Size(15, 14);
            this.chk_layer0.TabIndex = 5;
            this.chk_layer0.TabStop = false;
            this.chk_layer0.UseVisualStyleBackColor = true;
            this.chk_layer0.CheckedChanged += new System.EventHandler(this.chk_layer_CheckedChanged);
            // 
            // radio_layer6
            // 
            this.radio_layer6.AutoSize = true;
            this.radio_layer6.Location = new System.Drawing.Point(25, 155);
            this.radio_layer6.Name = "radio_layer6";
            this.radio_layer6.Size = new System.Drawing.Size(78, 17);
            this.radio_layer6.TabIndex = 6;
            this.radio_layer6.Text = "Ceiling Top";
            this.radio_layer6.UseVisualStyleBackColor = true;
            this.radio_layer6.CheckedChanged += new System.EventHandler(this.radio_layer_CheckedChanged);
            // 
            // chk_layer6
            // 
            this.chk_layer6.AutoSize = true;
            this.chk_layer6.Checked = true;
            this.chk_layer6.CheckState = System.Windows.Forms.CheckState.Checked;
            this.chk_layer6.Location = new System.Drawing.Point(8, 158);
            this.chk_layer6.Name = "chk_layer6";
            this.chk_layer6.Size = new System.Drawing.Size(15, 14);
            this.chk_layer6.TabIndex = 5;
            this.chk_layer6.TabStop = false;
            this.chk_layer6.UseVisualStyleBackColor = true;
            this.chk_layer6.CheckedChanged += new System.EventHandler(this.chk_layer_CheckedChanged);
            // 
            // radio_layer5
            // 
            this.radio_layer5.AutoSize = true;
            this.radio_layer5.Location = new System.Drawing.Point(25, 132);
            this.radio_layer5.Name = "radio_layer5";
            this.radio_layer5.Size = new System.Drawing.Size(114, 17);
            this.radio_layer5.TabIndex = 6;
            this.radio_layer5.Text = "Ceiling Deco (OBJ)";
            this.radio_layer5.UseVisualStyleBackColor = true;
            this.radio_layer5.CheckedChanged += new System.EventHandler(this.radio_layer_CheckedChanged);
            // 
            // chk_layer5
            // 
            this.chk_layer5.AutoSize = true;
            this.chk_layer5.Checked = true;
            this.chk_layer5.CheckState = System.Windows.Forms.CheckState.Checked;
            this.chk_layer5.Location = new System.Drawing.Point(8, 135);
            this.chk_layer5.Name = "chk_layer5";
            this.chk_layer5.Size = new System.Drawing.Size(15, 14);
            this.chk_layer5.TabIndex = 5;
            this.chk_layer5.TabStop = false;
            this.chk_layer5.UseVisualStyleBackColor = true;
            this.chk_layer5.CheckedChanged += new System.EventHandler(this.chk_layer_CheckedChanged);
            // 
            // radio_layer4
            // 
            this.radio_layer4.AutoSize = true;
            this.radio_layer4.Location = new System.Drawing.Point(25, 109);
            this.radio_layer4.Name = "radio_layer4";
            this.radio_layer4.Size = new System.Drawing.Size(51, 17);
            this.radio_layer4.TabIndex = 6;
            this.radio_layer4.Text = "Walls";
            this.radio_layer4.UseVisualStyleBackColor = true;
            this.radio_layer4.CheckedChanged += new System.EventHandler(this.radio_layer_CheckedChanged);
            // 
            // chk_layer4
            // 
            this.chk_layer4.AutoSize = true;
            this.chk_layer4.Checked = true;
            this.chk_layer4.CheckState = System.Windows.Forms.CheckState.Checked;
            this.chk_layer4.Location = new System.Drawing.Point(8, 112);
            this.chk_layer4.Name = "chk_layer4";
            this.chk_layer4.Size = new System.Drawing.Size(15, 14);
            this.chk_layer4.TabIndex = 5;
            this.chk_layer4.TabStop = false;
            this.chk_layer4.UseVisualStyleBackColor = true;
            this.chk_layer4.CheckedChanged += new System.EventHandler(this.chk_layer_CheckedChanged);
            // 
            // radio_layer3
            // 
            this.radio_layer3.AutoSize = true;
            this.radio_layer3.Location = new System.Drawing.Point(25, 86);
            this.radio_layer3.Name = "radio_layer3";
            this.radio_layer3.Size = new System.Drawing.Size(84, 17);
            this.radio_layer3.TabIndex = 6;
            this.radio_layer3.Text = "Floor deco 2";
            this.radio_layer3.UseVisualStyleBackColor = true;
            this.radio_layer3.CheckedChanged += new System.EventHandler(this.radio_layer_CheckedChanged);
            // 
            // chk_layer3
            // 
            this.chk_layer3.AutoSize = true;
            this.chk_layer3.Checked = true;
            this.chk_layer3.CheckState = System.Windows.Forms.CheckState.Checked;
            this.chk_layer3.Location = new System.Drawing.Point(8, 89);
            this.chk_layer3.Name = "chk_layer3";
            this.chk_layer3.Size = new System.Drawing.Size(15, 14);
            this.chk_layer3.TabIndex = 5;
            this.chk_layer3.TabStop = false;
            this.chk_layer3.UseVisualStyleBackColor = true;
            this.chk_layer3.CheckedChanged += new System.EventHandler(this.chk_layer_CheckedChanged);
            // 
            // radio_layer2
            // 
            this.radio_layer2.AutoSize = true;
            this.radio_layer2.Location = new System.Drawing.Point(25, 63);
            this.radio_layer2.Name = "radio_layer2";
            this.radio_layer2.Size = new System.Drawing.Size(84, 17);
            this.radio_layer2.TabIndex = 4;
            this.radio_layer2.Text = "Floor deco 1";
            this.radio_layer2.UseVisualStyleBackColor = true;
            this.radio_layer2.CheckedChanged += new System.EventHandler(this.radio_layer_CheckedChanged);
            // 
            // radio_layer1
            // 
            this.radio_layer1.AutoSize = true;
            this.radio_layer1.Checked = true;
            this.radio_layer1.Location = new System.Drawing.Point(25, 40);
            this.radio_layer1.Name = "radio_layer1";
            this.radio_layer1.Size = new System.Drawing.Size(77, 17);
            this.radio_layer1.TabIndex = 3;
            this.radio_layer1.TabStop = true;
            this.radio_layer1.Text = "Floor (OBJ)";
            this.radio_layer1.UseVisualStyleBackColor = true;
            this.radio_layer1.CheckedChanged += new System.EventHandler(this.radio_layer_CheckedChanged);
            // 
            // chk_layer2
            // 
            this.chk_layer2.AutoSize = true;
            this.chk_layer2.Checked = true;
            this.chk_layer2.CheckState = System.Windows.Forms.CheckState.Checked;
            this.chk_layer2.Location = new System.Drawing.Point(8, 65);
            this.chk_layer2.Name = "chk_layer2";
            this.chk_layer2.Size = new System.Drawing.Size(15, 14);
            this.chk_layer2.TabIndex = 1;
            this.chk_layer2.TabStop = false;
            this.chk_layer2.UseVisualStyleBackColor = true;
            this.chk_layer2.CheckedChanged += new System.EventHandler(this.chk_layer_CheckedChanged);
            // 
            // chk_layer1
            // 
            this.chk_layer1.AutoSize = true;
            this.chk_layer1.Checked = true;
            this.chk_layer1.CheckState = System.Windows.Forms.CheckState.Checked;
            this.chk_layer1.Location = new System.Drawing.Point(8, 42);
            this.chk_layer1.Name = "chk_layer1";
            this.chk_layer1.Size = new System.Drawing.Size(15, 14);
            this.chk_layer1.TabIndex = 0;
            this.chk_layer1.TabStop = false;
            this.chk_layer1.UseVisualStyleBackColor = true;
            this.chk_layer1.CheckedChanged += new System.EventHandler(this.chk_layer_CheckedChanged);
            // 
            // chk_showOverlappingTiles
            // 
            this.chk_showOverlappingTiles.AutoSize = true;
            this.chk_showOverlappingTiles.Location = new System.Drawing.Point(6, 18);
            this.chk_showOverlappingTiles.Name = "chk_showOverlappingTiles";
            this.chk_showOverlappingTiles.Size = new System.Drawing.Size(132, 17);
            this.chk_showOverlappingTiles.TabIndex = 36;
            this.chk_showOverlappingTiles.TabStop = false;
            this.chk_showOverlappingTiles.Text = "Show overlapping tiles";
            this.chk_showOverlappingTiles.UseVisualStyleBackColor = true;
            this.chk_showOverlappingTiles.CheckedChanged += new System.EventHandler(this.chk_showOverlappingTiles_CheckedChanged);
            // 
            // butWndAI
            // 
            this.butWndAI.Location = new System.Drawing.Point(1, 61);
            this.butWndAI.Name = "butWndAI";
            this.butWndAI.Size = new System.Drawing.Size(76, 28);
            this.butWndAI.TabIndex = 33;
            this.butWndAI.TabStop = false;
            this.butWndAI.Text = "AI";
            this.butWndAI.UseVisualStyleBackColor = true;
            this.butWndAI.Click += new System.EventHandler(this.butWndAI_Click);
            // 
            // butWndMisc
            // 
            this.butWndMisc.Location = new System.Drawing.Point(78, 61);
            this.butWndMisc.Name = "butWndMisc";
            this.butWndMisc.Size = new System.Drawing.Size(76, 28);
            this.butWndMisc.TabIndex = 37;
            this.butWndMisc.TabStop = false;
            this.butWndMisc.Text = "Misc";
            this.butWndMisc.UseVisualStyleBackColor = true;
            this.butWndMisc.Click += new System.EventHandler(this.butWndMisc_Click);
            // 
            // butWndActors
            // 
            this.butWndActors.Location = new System.Drawing.Point(1, 91);
            this.butWndActors.Name = "butWndActors";
            this.butWndActors.Size = new System.Drawing.Size(76, 28);
            this.butWndActors.TabIndex = 38;
            this.butWndActors.TabStop = false;
            this.butWndActors.Text = "Actors";
            this.butWndActors.UseVisualStyleBackColor = true;
            this.butWndActors.Click += new System.EventHandler(this.butWndActors_Click);
            // 
            // statusStrip1
            // 
            this.statusStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.toolStripStatusLabel1});
            this.statusStrip1.Location = new System.Drawing.Point(0, 804);
            this.statusStrip1.Name = "statusStrip1";
            this.statusStrip1.Size = new System.Drawing.Size(1187, 22);
            this.statusStrip1.TabIndex = 39;
            this.statusStrip1.Text = "statusStrip1";
            // 
            // toolStripStatusLabel1
            // 
            this.toolStripStatusLabel1.Name = "toolStripStatusLabel1";
            this.toolStripStatusLabel1.Size = new System.Drawing.Size(118, 17);
            this.toolStripStatusLabel1.Text = "toolStripStatusLabel1";
            // 
            // tableLayoutPanel1
            // 
            this.tableLayoutPanel1.ColumnCount = 2;
            this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 100F));
            this.tableLayoutPanel1.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Absolute, 160F));
            this.tableLayoutPanel1.Controls.Add(this.panel1, 1, 0);
            this.tableLayoutPanel1.Controls.Add(this.pictureBox1, 0, 0);
            this.tableLayoutPanel1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.tableLayoutPanel1.Location = new System.Drawing.Point(0, 24);
            this.tableLayoutPanel1.Name = "tableLayoutPanel1";
            this.tableLayoutPanel1.RowCount = 1;
            this.tableLayoutPanel1.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 100F));
            this.tableLayoutPanel1.Size = new System.Drawing.Size(1187, 780);
            this.tableLayoutPanel1.TabIndex = 40;
            // 
            // panel1
            // 
            this.panel1.Controls.Add(this.groupBox2);
            this.panel1.Controls.Add(this.butWndMaterials);
            this.panel1.Controls.Add(this.butWndPrefabs);
            this.panel1.Controls.Add(this.butWndActors);
            this.panel1.Controls.Add(this.butWndMisc);
            this.panel1.Controls.Add(this.groupBox1);
            this.panel1.Controls.Add(this.butWndCollision);
            this.panel1.Controls.Add(this.butWndAI);
            this.panel1.Controls.Add(this.butWndLights);
            this.panel1.Controls.Add(this.butWndObjects);
            this.panel1.Dock = System.Windows.Forms.DockStyle.Fill;
            this.panel1.Location = new System.Drawing.Point(1030, 3);
            this.panel1.Name = "panel1";
            this.panel1.Size = new System.Drawing.Size(154, 774);
            this.panel1.TabIndex = 0;
            // 
            // groupBox2
            // 
            this.groupBox2.Controls.Add(this.chk_snapToGrid);
            this.groupBox2.Controls.Add(this.chk_showOverlappingTiles);
            this.groupBox2.Location = new System.Drawing.Point(3, 311);
            this.groupBox2.Name = "groupBox2";
            this.groupBox2.Size = new System.Drawing.Size(148, 66);
            this.groupBox2.TabIndex = 40;
            this.groupBox2.TabStop = false;
            this.groupBox2.Text = "Options";
            // 
            // chk_snapToGrid
            // 
            this.chk_snapToGrid.AutoSize = true;
            this.chk_snapToGrid.Checked = true;
            this.chk_snapToGrid.CheckState = System.Windows.Forms.CheckState.Checked;
            this.chk_snapToGrid.Location = new System.Drawing.Point(6, 41);
            this.chk_snapToGrid.Name = "chk_snapToGrid";
            this.chk_snapToGrid.Size = new System.Drawing.Size(85, 17);
            this.chk_snapToGrid.TabIndex = 39;
            this.chk_snapToGrid.Text = "Snap to Grid";
            this.chk_snapToGrid.UseVisualStyleBackColor = true;
            // 
            // butWndPrefabs
            // 
            this.butWndPrefabs.Location = new System.Drawing.Point(78, 91);
            this.butWndPrefabs.Name = "butWndPrefabs";
            this.butWndPrefabs.Size = new System.Drawing.Size(76, 28);
            this.butWndPrefabs.TabIndex = 38;
            this.butWndPrefabs.TabStop = false;
            this.butWndPrefabs.Text = "Prefabs";
            this.butWndPrefabs.UseVisualStyleBackColor = true;
            this.butWndPrefabs.Click += new System.EventHandler(this.butWndPrefabs_Click);
            // 
            // timer_autosave
            // 
            this.timer_autosave.Enabled = true;
            this.timer_autosave.Interval = 300000;
            this.timer_autosave.Tick += new System.EventHandler(this.timer_autosave_Tick);
            // 
            // Form1
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(1187, 826);
            this.Controls.Add(this.tableLayoutPanel1);
            this.Controls.Add(this.statusStrip1);
            this.Controls.Add(this.menuStrip1);
            this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
            this.KeyPreview = true;
            this.MainMenuStrip = this.menuStrip1;
            this.Name = "Form1";
            this.SizeGripStyle = System.Windows.Forms.SizeGripStyle.Hide;
            this.Text = "KnockEd";
            this.FormClosing += new System.Windows.Forms.FormClosingEventHandler(this.Form1_FormClosing);
            this.MouseEnter += new System.EventHandler(this.Form1_MouseEnter);
            this.MouseWheel += new System.Windows.Forms.MouseEventHandler(this.pictureBox1_MouseWheel);
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).EndInit();
            this.menuStrip1.ResumeLayout(false);
            this.menuStrip1.PerformLayout();
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            this.statusStrip1.ResumeLayout(false);
            this.statusStrip1.PerformLayout();
            this.tableLayoutPanel1.ResumeLayout(false);
            this.panel1.ResumeLayout(false);
            this.groupBox2.ResumeLayout(false);
            this.groupBox2.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }
        #endregion

        private System.Windows.Forms.PictureBox pictureBox1;
        private System.Windows.Forms.MenuStrip menuStrip1;
        private System.Windows.Forms.ToolStripMenuItem fileToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem helpToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem aboutToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem newToolStripMenuItem;
        private System.Windows.Forms.ToolStripSeparator toolStripMenuItem1;
        private System.Windows.Forms.ToolStripMenuItem openToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem saveAsToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem exitToolStripMenuItem;
        private System.Windows.Forms.Button butWndMaterials;
        private System.Windows.Forms.ToolStripMenuItem viewToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem invertBackgroundToolStripMenuItem;
        private System.Windows.Forms.Button butWndLights;
        private System.Windows.Forms.Button butWndCollision;
        private System.Windows.Forms.Button butWndObjects;
        private System.Windows.Forms.ToolStripMenuItem showGridToolStripMenuItem;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.RadioButton radio_layer2;
        private System.Windows.Forms.RadioButton radio_layer1;
        private System.Windows.Forms.CheckBox chk_layer2;
        private System.Windows.Forms.CheckBox chk_layer1;
        private System.Windows.Forms.CheckBox chk_showOverlappingTiles;
        private System.Windows.Forms.ToolStripMenuItem toolsToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem deleteOverlappingTilesToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem setGroundLevelToolStripMenuItem;
        private System.Windows.Forms.Button butWndAI;
        private System.Windows.Forms.Button butWndMisc;
        private System.Windows.Forms.Button butWndActors;
        private System.Windows.Forms.StatusStrip statusStrip1;
        private System.Windows.Forms.ToolStripStatusLabel toolStripStatusLabel1;
        private System.Windows.Forms.ToolStripMenuItem setMissionTypeToolStripMenuItem;
        private System.Windows.Forms.ToolStripSeparator toolStripMenuItem3;
        private System.Windows.Forms.ToolStripSeparator toolStripMenuItem4;
        private System.Windows.Forms.ToolStripMenuItem clearAllActorsToolStripMenuItem;
        private System.Windows.Forms.TableLayoutPanel tableLayoutPanel1;
        private System.Windows.Forms.Panel panel1;
        private System.Windows.Forms.ToolStripMenuItem saveToolStripMenuItem;
        private System.Windows.Forms.Timer timer_autosave;
        private System.Windows.Forms.ToolStripSeparator toolStripMenuItem5;
        private System.Windows.Forms.ToolStripMenuItem findByIDToolStripMenuItem;
        private System.Windows.Forms.ToolStripTextBox menuTextBoxIDFinder;
        private System.Windows.Forms.ToolStripMenuItem findItNowToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem clearResultToolStripMenuItem;
        private System.Windows.Forms.ToolStripSeparator toolStripMenuItem2;
        private System.Windows.Forms.CheckBox chk_snapToGrid;
        private System.Windows.Forms.RadioButton radio_layer3;
        private System.Windows.Forms.CheckBox chk_layer3;
        private System.Windows.Forms.GroupBox groupBox2;
        private System.Windows.Forms.Button butWndPrefabs;
        private System.Windows.Forms.ToolStripSeparator toolStripMenuItem6;
        private System.Windows.Forms.ToolStripMenuItem exportPrefabToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem openPrefabToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem undoToolStripMenuItem;
        private System.Windows.Forms.ToolStripSeparator toolStripMenuItem7;
        private System.Windows.Forms.ToolStripMenuItem setLevelBackgroundToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem workshopToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem uploadSingleLevelToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem frontLayerGridToolStripMenuItem;
        private System.Windows.Forms.ToolStripSeparator toolStripMenuItem8;
        private System.Windows.Forms.ToolStripMenuItem playMapToolStripMenuItem;
        private System.Windows.Forms.RadioButton radio_layer0;
        private System.Windows.Forms.CheckBox chk_layer0;
        private System.Windows.Forms.RadioButton radio_layer6;
        private System.Windows.Forms.CheckBox chk_layer6;
        private System.Windows.Forms.RadioButton radio_layer5;
        private System.Windows.Forms.CheckBox chk_layer5;
        private System.Windows.Forms.RadioButton radio_layer4;
        private System.Windows.Forms.CheckBox chk_layer4;
    }
}

