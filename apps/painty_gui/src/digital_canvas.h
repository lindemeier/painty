#ifndef EDAVID_DIGITAL_canvasPtr_H
#define EDAVID_DIGITAL_canvasPtr_H

#include <QGraphicsScene>
#include <QGraphicsView>

#include <memory>

#include <painty/canvas.h>
#include <painty/texture_brush.h>
#include <painty/footprint_brush.h>

namespace edavid
{
namespace renderer
{
class RenderCanvas;
}
}  // namespace edavid

class DigitalCanvas : public QGraphicsScene
{
  Q_OBJECT

public:
  DigitalCanvas(const uint32_t width, const uint32_t height, QObject* parent);
  virtual ~DigitalCanvas();

  std::shared_ptr<painty::Canvas<painty::vec3>> getCanvas() const;

public slots:
  void setColor(const QColor& Rbc, const QColor& Rwc);
  void setBrushRadius(int radius);

  void dryCanvas();
  void updateCanvas();

protected:
  void mousePressEvent(QGraphicsSceneMouseEvent* event);
  void mouseMoveEvent(QGraphicsSceneMouseEvent* event);
  void mouseReleaseEvent(QGraphicsSceneMouseEvent* event);

private:
  QGraphicsPixmapItem* _pixmapItem;

  std::shared_ptr<painty::Canvas<painty::vec3>> _canvasPtr;

  std::unique_ptr<painty::TextureBrush<painty::vec3>> _brushPtr;
  std::unique_ptr<painty::FootprintBrush<painty::vec3>> _brushPtr2;

  std::vector<painty::vec2> _brushStrokePath;

  double _brushRadius = 15.0;
};

class DigitalCanvasView : public QGraphicsView
{
  std::unique_ptr<DigitalCanvas> _digitalCanvas;
  int32_t _currentKey;

public:
  DigitalCanvasView(const uint32_t width, const uint32_t height, QWidget* parent);
  DigitalCanvasView(QWidget* parent);
  virtual ~DigitalCanvasView();

  DigitalCanvas* getDigitalCanvas();

protected:
  void wheelEvent(QWheelEvent* event);
  void keyPressEvent(QKeyEvent* e);
  void keyReleaseEvent(QKeyEvent* e);
};

#endif  // EDAVID_DIGITAL_canvasPtr_H
