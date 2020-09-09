#ifndef EDAVID_DIGITAL_canvasPtr_H
#define EDAVID_DIGITAL_canvasPtr_H

#include <QtWidgets/QGraphicsScene>
#include <QtWidgets/QGraphicsView>
#include <QtWidgets/QLabel>
#include <memory>

#include "painty/renderer/Canvas.h"
#include "painty/renderer/FootprintBrush.h"
#include "painty/renderer/TextureBrush.h"

namespace edavid {
namespace renderer {
class RenderCanvas;
}
}  // namespace edavid

class DigitalCanvas : public QGraphicsScene {
  Q_OBJECT

 public:
  DigitalCanvas(const uint32_t width, const uint32_t height, QObject* parent,
                QLabel* pickupMapLabelPtr);
  virtual ~DigitalCanvas();

  std::shared_ptr<painty::Canvas<painty::vec3>> getCanvas() const;

  void setPickupMapLabelPtr(QLabel* labelPtr);

 public slots:
  void setColor(const QColor& Rbc, const QColor& Rwc);
  void setBrushRadius(int radius);

  void dryCanvas();
  void updateCanvas();
  void setUseFootprintBrush(bool);

  painty::FootprintBrush<painty::vec3>* getFootprintBrushPtr();

 protected:
  void mousePressEvent(QGraphicsSceneMouseEvent* event);
  void mouseMoveEvent(QGraphicsSceneMouseEvent* event);
  void mouseReleaseEvent(QGraphicsSceneMouseEvent* event);

 private:
  QGraphicsPixmapItem* _pixmapItem;

  std::shared_ptr<painty::Canvas<painty::vec3>> _canvasPtr;

  std::unique_ptr<painty::TextureBrush<painty::vec3>> _brushTexturePtr;
  std::unique_ptr<painty::FootprintBrush<painty::vec3>> _brushFootprintPtr;

  bool _useFootprintBrush = true;

  std::vector<painty::vec2> _brushStrokePath;

  double _brushRadius = 15.0;

  bool _mousePressed = false;

  QLabel* _pickupMapLabelPtr = nullptr;
};

class DigitalCanvasView : public QGraphicsView {
  std::unique_ptr<DigitalCanvas> _digitalCanvas;
  int32_t _currentKey;

 public:
  DigitalCanvasView(const uint32_t width, const uint32_t height,
                    QWidget* parent, QLabel* pickupMapLabelPtr);
  DigitalCanvasView(QWidget* parent, QLabel* pickupMapLabelPtr);
  virtual ~DigitalCanvasView();

  DigitalCanvas* getDigitalCanvas();

 protected:
  void wheelEvent(QWheelEvent* event);
  void keyPressEvent(QKeyEvent* e);
  void keyReleaseEvent(QKeyEvent* e);
};

#endif  // EDAVID_DIGITAL_canvasPtr_H
