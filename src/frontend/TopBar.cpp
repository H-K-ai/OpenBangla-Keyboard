#include <QSystemTrayIcon>
#include <QMessageBox>
#include <QMouseEvent>
#include <QFileDialog>
#include <QMenu>
#include <QAction>
#include <QDebug>
#include "TopBar.h"
#include "Layout.h"
#include "Settings.h"
#include "LayoutViewer.h"
#include "AboutFile.h"
#include "SettingsDialog.h"
#include "LayoutConverter.h"
#include "ui_TopBar.h"

TopBar::TopBar(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::TopBar)
{
    ui->setupUi(this);

    gLayout = new Layout();
    gSettings = new Settings();

    /* Dialogs */
    layoutViewer = new LayoutViewer(this);
    settingsDialog = new SettingsDialog(this);

    SetupTopBar();
    SetupPopupMenus();
    SetupTrayIcon();
}

TopBar::~TopBar()
{
    /* Dialogs */
    delete layoutViewer;
    delete settingsDialog;

    delete ui;
}

void TopBar::SetupTopBar() {
  this->setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
  this->setFixedSize(QSize(this->width(), this->height()));

  move(gSettings->getTopBarWindowPosition());
}

void TopBar::SetupPopupMenus() {
  // Layout Popup Menu
  layoutMenu = new QMenu("Select keyboard layout", this);
  layoutMenu->setIcon(QIcon(":/images/keyboard_layout.png"));
  layoutMenuInstall = new QAction("Install a layout", this);
  layoutMenuLayoutsGroup = new QActionGroup(this);
  for (int i = 0; i < MaxLayoutFiles; ++i) {
    layoutMenuLayouts[i] = new QAction(this);
    layoutMenuLayouts[i]->setVisible(false);
    layoutMenuLayouts[i]->setCheckable(true);
    layoutMenuLayoutsGroup->addAction(layoutMenuLayouts[i]);
    connect(layoutMenuLayouts[i], SIGNAL(triggered()), this, SLOT(layoutMenuLayouts_clicked()));
  }
  RefreshLayouts();
  connect(layoutMenuInstall, SIGNAL(triggered()), this, SLOT(layoutMenuInstall_clicked()));

  // Settings Popup Menu
  settingsMenuFixedLayoutAutoVForm = new QAction("Enable \"Automatic Vowel Forming\"", this);
  settingsMenuFixedLayoutAutoVForm->setCheckable(true);
  settingsMenuFixedLayoutAutoVForm->setChecked(gSettings->getAutoVowelFormFixed());
  connect(settingsMenuFixedLayoutAutoVForm, SIGNAL(triggered()), this, SLOT(settingsMenuFixedLayoutAutoVForm_clicked()));

  settingsMenuFixedLayoutAutoChandra = new QAction("Automatically fix \"Chandrabindu\" position", this);
  settingsMenuFixedLayoutAutoChandra->setCheckable(true);
  settingsMenuFixedLayoutAutoChandra->setChecked(gSettings->getAutoChandraPosFixed());
  connect(settingsMenuFixedLayoutAutoChandra, SIGNAL(triggered()), this, SLOT(settingsMenuFixedLayoutAutoChandra_clicked()));
  /*
  settingsMenuFixedLayoutOldReph = new QAction("Old Style Reph", this);
  settingsMenuFixedLayoutOldReph->setCheckable(true);
  settingsMenuFixedLayoutOldReph->setChecked(gSettings->getOldReph());
  connect(settingsMenuFixedLayoutOldReph, SIGNAL(triggered()), this, SLOT(settingsMenuFixedLayoutOldReph_clicked()));
  */
  settingsMenuFixedLayoutTraditionalKar = new QAction("Enable \"Traditional Kar Joining\"", this);
  settingsMenuFixedLayoutTraditionalKar->setCheckable(true);
  settingsMenuFixedLayoutTraditionalKar->setChecked(gSettings->getTraditionalKarFixed());
  connect(settingsMenuFixedLayoutTraditionalKar, SIGNAL(triggered()), this, SLOT(settingsMenuFixedLayoutTraditionalKar_clicked()));

  settingsMenuFixedLayoutNumberPad = new QAction("Enable Bengali in NumberPad", this);
  settingsMenuFixedLayoutNumberPad->setCheckable(true);
  settingsMenuFixedLayoutNumberPad->setChecked(gSettings->getNumberPadFixed());
  connect(settingsMenuFixedLayoutNumberPad, SIGNAL(triggered()), this, SLOT(settingsMenuFixedLayoutNumberPad_clicked()));

  settingsMenuFixedLayout = new QMenu("Fixed Keyboard Layout Options", this);
  settingsMenuFixedLayout->addAction(settingsMenuFixedLayoutAutoVForm);
  settingsMenuFixedLayout->addAction(settingsMenuFixedLayoutAutoChandra);
  //settingsMenuFixedLayout->addAction(settingsMenuFixedLayoutOldReph);
  settingsMenuFixedLayout->addAction(settingsMenuFixedLayoutTraditionalKar);
  settingsMenuFixedLayout->addAction(settingsMenuFixedLayoutNumberPad);

  settingsMenuShowDialog = new QAction("Settings", this);
  connect(settingsMenuShowDialog, SIGNAL(triggered()), this, SLOT(settingsMenuShowDialog_clicked()));

  settingsMenu = new QMenu(this);
  settingsMenu->addMenu(settingsMenuFixedLayout);
  settingsMenu->addSeparator();
  settingsMenu->addAction(settingsMenuShowDialog);

  // About Popup Menu
  aboutMenuLayout = new QAction("About current keyboard layout...", this);
  connect(aboutMenuLayout, SIGNAL(triggered()), this, SLOT(aboutMenuLayout_clicked()));

  aboutMenuAbout = new QAction("About OpenBangla Keyboard...", this);
  connect(aboutMenuAbout, SIGNAL(triggered()), this, SLOT(aboutMenuAbout_clicked()));

  aboutMenu = new QMenu(this);
  aboutMenu->addAction(aboutMenuLayout);
  aboutMenu->addAction(aboutMenuAbout);

  // Quit Popup Menu
  quitMenuQuit = new QAction("Quit", this);
  connect(quitMenuQuit, SIGNAL(triggered()), this, SLOT(quitMenuQuit_clicked()));

  quitMenuOnTray = new QAction("Jump to system tray", this);
  connect(quitMenuOnTray, SIGNAL(triggered()), this, SLOT(quitMenuOnTray_clicked()));

  quitMenu = new QMenu(this);
  quitMenu->addAction(quitMenuOnTray);
  quitMenu->addAction(quitMenuQuit);
}

void TopBar::SetupTrayIcon() {
  /* TODO: Fix Crash... */
  tray = new QSystemTrayIcon(QIcon(":/images/keyboard_layout_viewer.png"), this);
  tray->setToolTip("OpenBangla Keyboard");

  /* Tray Menu */
  trayMenuRestore = new QAction("Restore TopBar", this);
  connect(trayMenuRestore, SIGNAL(triggered()), this, SLOT(trayMenuRestore_clicked()));

  trayMenu = new QMenu(this);
  trayMenu->addAction(trayMenuRestore);
  trayMenu->addMenu(layoutMenu); // Layout Menu
  /*
  trayMenu->addSeparator();
  trayMenu->addAction(aboutMenuAbout);*/
  trayMenu->addSeparator();
  trayMenu->addAction(quitMenuQuit);

  tray->setContextMenu(trayMenu);
}

void TopBar::RefreshLayouts() {
  LayoutList list;
  list = gLayout->searchLayouts();

  for(int k = 0; k < MaxLayoutFiles; ++k) {
    if(k < list.count()) {
      layoutMenuLayouts[k]->setText(list[k]);
      layoutMenuLayouts[k]->setVisible(true);
    } else {
      layoutMenuLayouts[k]->setVisible(false);
    }
    layoutMenu->addAction(layoutMenuLayouts[k]);
    // Select previous selected layout
    if(layoutMenuLayouts[k]->text() == gSettings->getLayoutName()) {
      layoutMenuLayouts[k]->setChecked(true);
      gLayout->setLayout(layoutMenuLayouts[k]->text());
    }
  }
  layoutMenu->addSeparator();
  layoutMenu->addAction(layoutMenuInstall);
}

void TopBar::layoutMenuLayouts_clicked() {
  QAction *action = qobject_cast<QAction *>(sender());
  gLayout->setLayout(action->text());
  action->setChecked(true);
  layoutViewer->refreshLayoutViewer();
}

void TopBar::layoutMenuInstall_clicked() {
  QString fileName = QFileDialog::getOpenFileName(this, tr("Select Keyboard Layout"), QDir::homePath(), tr("Avro Keyboard 5 Keyboard Layout (*.avrolayout)"));
  LayoutConverter conv;
  if(fileName.contains(".avrolayout") && fileName != "") {
    conv.convertLayout(fileName);
    QMessageBox::information(this, tr("OpenBangla Keyboard"), tr("Layout Installed Successfully"), QMessageBox::Ok);
  }
  RefreshLayouts();
}

void TopBar::settingsMenuFixedLayoutAutoVForm_clicked() {
    gSettings->setAutoVowelFormFixed(settingsMenuFixedLayoutAutoVForm->isChecked());
}

void TopBar::settingsMenuFixedLayoutAutoChandra_clicked() {
  gSettings->setAutoChandraPosFixed(settingsMenuFixedLayoutAutoChandra->isChecked());
}

void TopBar::settingsMenuFixedLayoutOldReph_clicked() {
  gSettings->setOldReph(settingsMenuFixedLayoutOldReph->isChecked());
}

void TopBar::settingsMenuFixedLayoutTraditionalKar_clicked() {
  gSettings->setTraditionalKarFixed(settingsMenuFixedLayoutTraditionalKar->isChecked());
}

void TopBar::settingsMenuFixedLayoutNumberPad_clicked() {
  gSettings->setNumberPadFixed(settingsMenuFixedLayoutNumberPad->isChecked());
}

void TopBar::settingsMenuShowDialog_clicked() {
    settingsDialog->updateSettings();
    settingsDialog->show();
}

void TopBar::aboutMenuLayout_clicked() {
  layoutViewer->showLayoutInfoDialog();
}

void TopBar::aboutMenuAbout_clicked() {
  QMessageBox::about(this, "About OpenBangla Keyboard",
      tr("<h2>OpenBangla Keyboard 0.1</h2>"
      "<p>Copyright &copy; 2015-2016 Muhammad Mominul Huque & OpenBangla Team."
      "<p>An OpenSource, Cross-Platform, Unicode Compliant Bengali Input Method."));
}

void TopBar::quitMenuOnTray_clicked() {
  this->setVisible(false);
  tray->setVisible(true);
  tray->showMessage("OpenBangla Keyboard", "OpenBangla Keyboard is now running on system tray");
}

void TopBar::quitMenuQuit_clicked() {
  TopBar::close();
}

void TopBar::trayMenuRestore_clicked() {
  tray->setVisible(false);
  this->setVisible(true);
}

void TopBar::on_buttonAbout_clicked()
{
  QPoint point;
  point = this->pos();
  point.setX(point.x() + ui->buttonAbout->geometry().x());
  point.setY(point.y() + this->height());
  aboutMenu->exec(point);
}

void TopBar::closeEvent(QCloseEvent *event) {
  gSettings->setTopBarWindowPosition(this->pos());
  event->accept();
}

void TopBar::mouseMoveEvent(QMouseEvent *event) {
  if(canMoveTopbar) {
    this->setCursor(Qt::ClosedHandCursor);
    move(event->globalX() - pressedMouseX, event->globalY() - pressedMouseY);
  }
}

void TopBar::mousePressEvent(QMouseEvent *event) {
  canMoveTopbar = true;
  pressedMouseX = event->x();
  pressedMouseY = event->y();
  event->accept();
}

void TopBar::mouseReleaseEvent(QMouseEvent *event) {
  canMoveTopbar = false;
  this->unsetCursor();
  event->accept();
}

void TopBar::on_buttonSetLayout_clicked()
{
  QPoint point;
  point = this->pos();
  point.setX(point.x() + ui->buttonSetLayout->geometry().x());
  point.setY(point.y() + this->height());
  layoutMenu->exec(point);
}

void TopBar::on_buttonShutdown_clicked()
{
    QPoint point;
    point = this->pos();
    point.setX(point.x() + ui->buttonShutdown->geometry().x());
    point.setY(point.y() + this->height());
    quitMenu->exec(point);
}

void TopBar::on_buttonViewLayout_clicked()
{
  layoutViewer->refreshLayoutViewer();
  layoutViewer->show();
}

void TopBar::on_buttonSettings_clicked()
{
    QPoint point;
    point = this->pos();
    point.setX(point.x() + ui->buttonSettings->geometry().x());
    point.setY(point.y() + this->height());
    settingsMenu->exec(point);
}
