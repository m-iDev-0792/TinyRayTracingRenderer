#ifndef DRAWWIDGET_H
#define DRAWWIDGET_H

#include <QWidget>
#include <QPainter>
#include <QPaintEvent>
class DrawWidget : public QWidget
{
  Q_OBJECT
public:
  explicit DrawWidget(QWidget *parent = nullptr);
public slots:
  void updateImg(QImage newImg);
private:
  QImage img;
signals:
protected:
  virtual void paintEvent(QPaintEvent *event);
};

#endif // DRAWWIDGET_H
