#include "digital_canvas.h"

#include <QGraphicsPixmapItem>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QGraphicsSceneMouseEvent>

#include <painty/canvas.h>
#include <painty/kubelka_munk.h>
#include <painty/renderer.h>
#include <painty/spline.h>

#include "digital_paint_main_window.h"
#include "ui_digital_paint_main_window.h"

#include <iostream>

std::shared_ptr<painty::Canvas<painty::vec3>> DigitalCanvas::getCanvas() const
{
  return _canvasPtr;
}

DigitalCanvas::DigitalCanvas(const uint32_t width, const uint32_t height, QObject* parent)
  : QGraphicsScene(0., 0., static_cast<double>(width), static_cast<double>(height), parent)
  , _pixmapItem(nullptr)
  , _canvasPtr(nullptr)
  , _brushStrokePath()
  , _brushPtr(std::make_unique<painty::TextureBrush<painty::vec3>>("/home/tsl/development/painty/data/sample_0"))
{
  this->setBackgroundBrush(QBrush(QColor(128, 128, 128)));
  _pixmapItem = this->addPixmap(QPixmap(static_cast<int32_t>(width), static_cast<int32_t>(height)));
  _pixmapItem->setFlag(QGraphicsItem::ItemIsMovable);

  _canvasPtr = std::make_shared<painty::Canvas<painty::vec3>>(height, width);

  updateCanvas();
}

DigitalCanvas::~DigitalCanvas()
{
}

void DigitalCanvas::setColor(const QColor& Rbc, const QColor& Rwc)
{
  painty::vec3 Rw(Rwc.redF(), Rwc.greenF(), Rwc.blueF());
  painty::vec3 Rb(Rbc.redF(), Rbc.greenF(), Rbc.blueF());
  painty::vec3 K, S;
  try
  {
    painty::ComputeScatteringAndAbsorption(Rb, Rw, K, S);
    _brushPtr->dip({ K, S });
  }
  catch (std::invalid_argument)
  {
    std::cerr << "invalid color" << std::endl;
  }
}

void DigitalCanvas::setBrushRadius(int radius)
{
  _brushPtr->setRadius(radius);
  _brushRadius = static_cast<double>(radius);

  QPixmap cursorMap(2 * radius + 1, 2 * radius + 1);
  cursorMap.fill(QColor(255, 255, 255, 0));
  QPainter p(&cursorMap);

  p.setPen(QPen(2));
  p.drawEllipse(QPointF(cursorMap.width() / 2, cursorMap.height() / 2), radius, radius);
  _pixmapItem->setCursor(QCursor(cursorMap));
}

void DigitalCanvas::dryCanvas()
{
  _canvasPtr->dryCanvas();

  updateCanvas();
}

void DigitalCanvas::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
  _brushStrokePath.clear();
  QPointF qp = event->scenePos();

  painty::vec2 p(qp.x(), qp.y());

  if (_brushStrokePath.empty() || (_brushStrokePath.back() != p))
  {
    _brushStrokePath.push_back(p);
  }
  event->ignore();
}

void DigitalCanvas::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
  QPointF qp = event->scenePos();

  painty::vec2 p(qp.x(), qp.y());

  if (_brushStrokePath.empty() || (_brushStrokePath.back() != p))
  {
    _brushStrokePath.push_back(p);
  }
  event->accept();
}

void DigitalCanvas::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
  QPointF qp = event->scenePos();
  painty::vec2 p(qp.x(), qp.y());

  if (_brushStrokePath.empty() || (_brushStrokePath.back() != p))
  {
    _brushStrokePath.push_back(p);
  }

  // spline
  painty::SplineEval<std::vector<painty::vec2>::const_iterator> spline(_brushStrokePath.cbegin(),
                                                                       _brushStrokePath.cend());
  // sample in radius steps
  std::vector<painty::vec2> cubicPoints;
  const auto radiusStep = 1.0 / (_brushRadius * 2.0);
  for (auto t = 0.0; t <= 1.0; t += radiusStep)
  {
    cubicPoints.push_back(spline.cubic(t));
  }

  _brushPtr->applyTo(cubicPoints, *_canvasPtr);

  event->ignore();
  updateCanvas();
}

void DigitalCanvas::updateCanvas()
{
  painty::Renderer<painty::vec3> renderer;
  painty::Mat<painty::vec3> rgb = renderer.compose(*_canvasPtr);
  painty::Mat<painty::vec<uchar, 3UL>> rgb_8(rgb.getRows(), rgb.getCols());

  const auto& fdata = rgb.getData();
  auto& bdata = rgb_8.getData();
  for (auto i = 0UL; i < fdata.size(); i++)
  {
    bdata[i][0] = static_cast<uchar>(fdata[i][0] * 255.0);
    bdata[i][1] = static_cast<uchar>(fdata[i][1] * 255.0);
    bdata[i][2] = static_cast<uchar>(fdata[i][2] * 255.0);
  }
  _pixmapItem->setPixmap(
      QPixmap::fromImage(QImage(&(bdata.front()[0]), rgb_8.getCols(), rgb_8.getRows(), QImage::Format_RGB888)));
}

DigitalCanvas* DigitalCanvasView::getDigitalCanvas()
{
  return _digitalCanvas.get();
}

DigitalCanvasView::DigitalCanvasView(const uint32_t width, const uint32_t height, QWidget* parent)
  : QGraphicsView(parent)
{
  _digitalCanvas = std::make_unique<DigitalCanvas>(width, height, this);
  this->setScene(_digitalCanvas.get());
}

DigitalCanvasView::DigitalCanvasView(QWidget* parent) : QGraphicsView(parent)
{
  _digitalCanvas = std::make_unique<DigitalCanvas>(1024U, 768U, this);
  this->setScene(_digitalCanvas.get());
}

DigitalCanvasView::~DigitalCanvasView()
{
}

void DigitalCanvasView::wheelEvent(QWheelEvent* event)
{
  if (_currentKey == 0)
  {
    event->ignore();
    return;
  }

  if (_currentKey == Qt::Key_Control)
  {
    double numDegrees = event->angleDelta().ry();
    if (numDegrees > 0.)
    {
      this->scale(1.1, 1.1);
    }
    else if (numDegrees < 0.)
    {
      this->scale(0.9, 0.9);
    }
  }
  event->accept();
}

void DigitalCanvasView::keyPressEvent(QKeyEvent* e)
{
  _currentKey = e->key();

  e->ignore();
}

void DigitalCanvasView::keyReleaseEvent(QKeyEvent* e)
{
  _currentKey = Qt::NoButton;

  e->ignore();
}
