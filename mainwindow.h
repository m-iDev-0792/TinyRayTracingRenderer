#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "renderer.h"
#include "drawwidget.h"
#include <QThread>
#include <QFileDialog>
namespace Ui {
  class MainWindow;
}

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = 0);
  ~MainWindow();
public slots:
  void getRenderState(int,int);
private slots:
  void on_renderBtn_clicked();
  void on_saveBtn_clicked();

signals:
  void run(int,int,int);
private:
  Ui::MainWindow *ui;
  DrawWidget* widget;
  Renderer tinyRenderer;
  QThread thread;
};

#endif // MAINWINDOW_H
