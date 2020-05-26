#include "KM_visualize_widget.h"

#include <QPainter>
#include <QColorDialog>

QColor KMVisualizeWidget::getRb() const
{
  return Rb;
}

QColor KMVisualizeWidget::getRw() const
{
  return Rw;
}

KMVisualizeWidget::KMVisualizeWidget(QWidget* parent) : QWidget(parent), Rb(120, 120, 120), Rw(121, 121, 121)
{
  this->setMouseTracking(true);
}

void KMVisualizeWidget::setRw(const QColor& c)
{
  Rw = c;
  this->update();
}

void KMVisualizeWidget::setRb(const QColor& c)
{
  Rb = c;
  this->update();
}

void KMVisualizeWidget::paintEvent(QPaintEvent* event)
{
  QPainter p(this);

  p.fillRect(0, 0, this->width(), this->height(), Qt::white);
  p.fillRect(this->width() / 3, 0, this->width() / 3, this->height(), Qt::black);

  p.fillRect(0, this->height() / 3, this->width() / 3, this->height() / 3, Rw);
  p.fillRect(this->width() / 3, this->height() / 3, this->width() / 3, this->height() / 3, Rb);
  p.fillRect(2 * (this->width() / 3), this->height() / 3, this->width() / 3, this->height() / 3, Rw);
}
