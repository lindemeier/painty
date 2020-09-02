#ifndef DIGITALPAINTMAINWINDOW_H
#define DIGITALPAINTMAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class DigitalPaintMainWindow;
}

class DigitalCanvasView;

class DigitalPaintMainWindow : public QMainWindow {
  Q_OBJECT

 public:
  explicit DigitalPaintMainWindow(QWidget* parent = 0);
  ~DigitalPaintMainWindow();

  Ui::DigitalPaintMainWindow* ui;

 private:
  DigitalCanvasView* m_canvasView;

 public slots:
  void setPaintingColorRw();
};

#endif  // DIGITALPAINTMAINWINDOW_H
