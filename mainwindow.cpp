#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::MainWindow)
{
  ui->setupUi(this);
  widget=ui->widget;

  thread.setStackSize(50*1024*1024);
  tinyRenderer.moveToThread(&thread);
  thread.start();
  connect(&tinyRenderer,SIGNAL(newLineRendered(QImage)),widget,SLOT(updateImg(QImage)));
  connect(this,SIGNAL(run(int,int,int)),&tinyRenderer,SLOT(renderScene(int,int,int)));
  connect(&tinyRenderer,SIGNAL(renderState(int,int)),this,SLOT(getRenderState(int,int)));

  ui->sampleList->addItem(QString("4"));
  ui->sampleList->addItem(QString("16"));
  ui->sampleList->addItem(QString("32"));
  ui->sampleList->addItem(QString("64"));
  ui->sampleList->addItem(QString("128"));
  ui->sampleList->addItem(QString("256"));
  ui->sampleList->addItem(QString("512"));
  ui->sampleList->addItem(QString("1024"));
}

MainWindow::~MainWindow()
{
  thread.quit();
  thread.wait();
  delete ui;
}
void MainWindow::getRenderState(int percentage, int remainTime){
  ui->progressBar->setValue(percentage);
  QString state=QString("progress %1 %\nreamin %2 s").arg(percentage).arg(remainTime);
  ui->stateLabel->setText(state);
}

void MainWindow::on_renderBtn_clicked()
{
  int sample=ui->sampleList->currentText().toInt();
  ui->progressBar->setValue(0);
  emit run(sample,ui->widget->width(),ui->widget->height());
}

void MainWindow::on_saveBtn_clicked()
{
    QString path=QFileDialog::getSaveFileName(nullptr,"Save image","image.png");
    if(path.isNull())return;
    ui->widget->grab().save(path);
}
