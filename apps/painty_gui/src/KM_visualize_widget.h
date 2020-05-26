#ifndef KM_VISUALIZE_WIDGET_H
#define KM_VISUALIZE_WIDGET_H

#include <QWidget>

class KMVisualizeWidget : public QWidget
{
  Q_OBJECT

  QColor Rb;
  QColor Rw;

public:
  explicit KMVisualizeWidget(QWidget* parent = nullptr);

  QColor getRb() const;

  QColor getRw() const;

signals:

public slots:
  void setRw(const QColor&);
  void setRb(const QColor&);

protected:
  void paintEvent(QPaintEvent* event);
};

#endif  // KM_VISUALIZE_WIDGET_H
