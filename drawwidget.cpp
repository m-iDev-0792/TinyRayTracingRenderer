#include "drawwidget.h"

DrawWidget::DrawWidget(QWidget *parent) : QWidget(parent)
{

}
void DrawWidget::paintEvent(QPaintEvent *event){
  QPainter p(this);
  if(img.isNull()){
      p.fillRect(this->rect(),Qt::black);
      return;
    }
  p.drawImage(0,0,img);
}
void DrawWidget::updateImg(QImage newImg){
  img=newImg;
  repaint();
}
