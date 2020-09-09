#ifndef DIGITALPAINTMAINWINDOW_H
#define DIGITALPAINTMAINWINDOW_H

#include <QtWidgets/QMainWindow>

namespace Ui {
class DigitalPaintMainWindow;
}

class DigitalCanvasView;

class DigitalPaintMainWindow : public QMainWindow {
  Q_OBJECT

 public:
  explicit DigitalPaintMainWindow(QWidget* parent = nullptr);
  ~DigitalPaintMainWindow();

  Ui::DigitalPaintMainWindow* ui;

 private:
  DigitalCanvasView* m_canvasView;

 public slots:
  void setPaintingColorRw();
};

#endif  // DIGITALPAINTMAINWINDOW_H
