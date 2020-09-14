#include "DigitalCanvas.hxx"

#include <QtGui/QKeyEvent>
#include <QtGui/QWheelEvent>
#include <QtWidgets/QGraphicsPixmapItem>
#include <QtWidgets/QGraphicsSceneMouseEvent>
#include <iostream>

#include "DigitalPaintMainWindow.hxx"
#include "apps/painty_gui/ui_DigitalPaintMainWindow.h"
#include "painty/core/Color.hxx"
#include "painty/core/KubelkaMunk.hxx"
#include "painty/core/Spline.hxx"
#include "painty/renderer/Canvas.hxx"
#include "painty/renderer/Renderer.hxx"

std::shared_ptr<painty::Canvas<painty::vec3>> DigitalCanvas::getCanvas() const {
  return _canvasPtr;
}

DigitalCanvas::DigitalCanvas(const uint32_t width, const uint32_t height,
                             QObject* parent, QLabel* pickupMapLabelPtr)
    : QGraphicsScene(0., 0., static_cast<double>(width),
                     static_cast<double>(height), parent),
      _pixmapItem(nullptr),
      _canvasPtr(nullptr),
      _brushTexturePtr(std::make_unique<painty::TextureBrush<painty::vec3>>(
        "./data/sample_0")),
      _brushFootprintPtr(
        std::make_unique<painty::FootprintBrush<painty::vec3>>(256U)),
      _brushStrokePath(),
      _pickupMapLabelPtr(pickupMapLabelPtr) {
  this->setBackgroundBrush(QBrush(QColor(128, 128, 128)));
  _pixmapItem = this->addPixmap(
    QPixmap(static_cast<int32_t>(width), static_cast<int32_t>(height)));
  _pixmapItem->setFlag(QGraphicsItem::ItemIsMovable);

  _canvasPtr = std::make_shared<painty::Canvas<painty::vec3>>(height, width);

  updateCanvas();
}

DigitalCanvas::~DigitalCanvas() {}

void DigitalCanvas::setPickupMapLabelPtr(QLabel* labelPtr) {
  _pickupMapLabelPtr = labelPtr;
}

void DigitalCanvas::setColor(const QColor& Rbc, const QColor& Rwc) {
  painty::vec3 Rw(Rwc.redF(), Rwc.greenF(), Rwc.blueF());
  painty::vec3 Rb(Rbc.redF(), Rbc.greenF(), Rbc.blueF());
  painty::vec3 K, S;
  try {
    painty::ComputeScatteringAndAbsorption(Rb, Rw, K, S);
    _brushTexturePtr->dip({K, S});
    _brushFootprintPtr->dip({K, S});
  } catch (std::invalid_argument) {
    std::cerr << "invalid color" << std::endl;
  }
}

void DigitalCanvas::setBrushRadius(int radius) {
  _brushRadius = static_cast<double>(radius);
  _brushTexturePtr->setRadius(_brushRadius);
  _brushFootprintPtr->setRadius(_brushRadius);

  QPixmap cursorMap(2 * radius + 1, 2 * radius + 1);
  cursorMap.fill(QColor(255, 255, 255, 0));
  QPainter p(&cursorMap);

  p.setPen(QPen(2));
  p.drawEllipse(QPointF(cursorMap.width() / 2, cursorMap.height() / 2), radius,
                radius);
  _pixmapItem->setCursor(QCursor(cursorMap));
}

void DigitalCanvas::dryCanvas() {
  _canvasPtr->dryCanvas();

  updateCanvas();
}

void DigitalCanvas::mousePressEvent(QGraphicsSceneMouseEvent* event) {
  _brushStrokePath.clear();
  QPointF qp = event->scenePos();

  painty::vec2 p(qp.x(), qp.y());

  if (_brushStrokePath.empty() || (_brushStrokePath.back() != p)) {
    _brushStrokePath.push_back(p);
  }

  _mousePressed = true;

  event->ignore();
}

void DigitalCanvas::mouseMoveEvent(QGraphicsSceneMouseEvent* event) {
  QPointF qp = event->scenePos();

  painty::vec2 p(qp.x(), qp.y());

  if (_brushStrokePath.empty() || (_brushStrokePath.back() != p)) {
    _brushStrokePath.push_back(p);
  }

  if (_useFootprintBrush && _mousePressed && (_brushStrokePath.size() >= 2)) {
    // interpolate positions between last and this point
    const auto p0   = _brushStrokePath[static_cast<size_t>(
      std::max(0, static_cast<int32_t>(_brushStrokePath.size()) - 3))];
    const auto p1   = _brushStrokePath[static_cast<size_t>(
      std::max(0, static_cast<int32_t>(_brushStrokePath.size()) - 2))];
    const auto p2   = _brushStrokePath[static_cast<size_t>(
      std::max(0, static_cast<int32_t>(_brushStrokePath.size()) - 1))];
    const auto dist = (p2 - p1).norm();  // distance in pixel
    // don't imprint at previous point
    for (int32_t pd = 1; pd <= static_cast<int32_t>(dist); pd++) {
      const double t = static_cast<double>(pd) / dist;
      const auto dir = painty::CatmullRomDerivativeFirst(p0, p1, p2, p2, t);
      _brushFootprintPtr->imprint(painty::CatmullRom(p0, p1, p2, p2, t),
                                  std::atan2(dir[1U], dir[0U]), *_canvasPtr);
    }
    updateCanvas();
  }

  event->accept();
}

void DigitalCanvas::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
  QPointF qp = event->scenePos();
  painty::vec2 p(qp.x(), qp.y());

  if (_brushStrokePath.empty() || (_brushStrokePath.back() != p)) {
    _brushStrokePath.push_back(p);
  }

  _mousePressed = false;

  if (!_useFootprintBrush) {
    // spline
    painty::SplineEval<std::vector<painty::vec2>::const_iterator> spline(
      _brushStrokePath.cbegin(), _brushStrokePath.cend());
    // sample in radius steps
    std::vector<painty::vec2> cubicPoints;
    const auto radiusStep = 1.0 / (_brushRadius * 2.0);
    for (auto t = 0.0; t <= 1.0; t += radiusStep) {
      cubicPoints.push_back(spline.cubic(t));
    }

    _brushTexturePtr->paintStroke(cubicPoints, *_canvasPtr);
    updateCanvas();
  }

  event->ignore();
}

void DigitalCanvas::updateCanvas() {
  painty::Renderer<painty::vec3> renderer;
  painty::ColorConverter<double> converter;

  {
    painty::Mat<painty::vec3> rgb = renderer.compose(*_canvasPtr);
    // convert to srgb for display
    for (auto& v : rgb) {
      converter.rgb2srgb(v, v);
    }

    QImage qimage(rgb.cols, rgb.rows, QImage::Format_RGB32);
    for (auto i = 0; i < rgb.rows; i++) {
      for (auto j = 0; j < rgb.cols; j++) {
        qimage.setPixel(j, i,
                        qRgb(static_cast<uint8_t>(rgb(i, j)[0U] * 255.0),
                             static_cast<uint8_t>(rgb(i, j)[1U] * 255.0),
                             static_cast<uint8_t>(rgb(i, j)[2U] * 255.0)));
      }
    }
    _pixmapItem->setPixmap(QPixmap::fromImage(qimage));
  }
  {
    painty::Mat<painty::vec3> white(
      _brushFootprintPtr->getPickupMap().getRows(),
      _brushFootprintPtr->getPickupMap().getCols());
    for (auto& p : white) {
      p = painty::vec3::Ones();
    }
    painty::Mat<painty::vec3> rgb =
      renderer.compose(_brushFootprintPtr->getPickupMap(), white);

    // convert to srgb for display
    for (auto& v : rgb) {
      converter.rgb2srgb(v, v);
    }

    QImage qimage(rgb.cols, rgb.rows, QImage::Format_RGB32);
    for (auto i = 0; i < rgb.rows; i++) {
      for (auto j = 0; j < rgb.cols; j++) {
        qimage.setPixel(j, i,
                        qRgb(static_cast<uint8_t>(rgb(i, j)[0U] * 255.0),
                             static_cast<uint8_t>(rgb(i, j)[1U] * 255.0),
                             static_cast<uint8_t>(rgb(i, j)[2U] * 255.0)));
      }
    }
    _pickupMapLabelPtr->setPixmap(QPixmap::fromImage(qimage.scaled(256, 256)));
  }

  // {
  //   painty::Mat<double> fp = _brushFootprintPtr->getFootprint();

  //   QImage qimage(fp.cols, fp.rows, QImage::Format_Grayscale8);
  //   for (auto i = 0U; i < fp.rows; i++)
  //   {
  //     for (auto j = 0U; j < fp.cols; j++)
  //     {
  //       const auto g = static_cast<uint8_t>(fp(i, j) * 255.0);
  //       qimage.setPixel(j, i, qRgb(g, g, g));
  //     }
  //   }
  //   _pickupMapLabelPtr->setPixmap(QPixmap::fromImage(qimage.scaled(256, 256)));
  // }
}

void DigitalCanvas::setUseFootprintBrush(bool use) {
  _useFootprintBrush = use;
}

painty::FootprintBrush<painty::vec3>* DigitalCanvas::getFootprintBrushPtr() {
  return _brushFootprintPtr.get();
}

DigitalCanvas* DigitalCanvasView::getDigitalCanvas() {
  return _digitalCanvas.get();
}

DigitalCanvasView::DigitalCanvasView(const uint32_t width,
                                     const uint32_t height, QWidget* parent,
                                     QLabel* pickupMapLabelPtr)
    : QGraphicsView(parent) {
  _digitalCanvas =
    std::make_unique<DigitalCanvas>(width, height, this, pickupMapLabelPtr);
  this->setScene(_digitalCanvas.get());
}

DigitalCanvasView::DigitalCanvasView(QWidget* parent, QLabel* pickupMapLabelPtr)
    : QGraphicsView(parent) {
  _digitalCanvas =
    std::make_unique<DigitalCanvas>(1024U, 768U, this, pickupMapLabelPtr);
  this->setScene(_digitalCanvas.get());
}

DigitalCanvasView::~DigitalCanvasView() {}

void DigitalCanvasView::wheelEvent(QWheelEvent* event) {
  if (_currentKey == 0) {
    event->ignore();
    return;
  }

  if (_currentKey == Qt::Key_Control) {
    double numDegrees = event->angleDelta().ry();
    if (numDegrees > 0.) {
      this->scale(1.1, 1.1);
    } else if (numDegrees < 0.) {
      this->scale(0.9, 0.9);
    }
  }
  event->accept();
}

void DigitalCanvasView::keyPressEvent(QKeyEvent* e) {
  _currentKey = e->key();

  e->ignore();
}

void DigitalCanvasView::keyReleaseEvent(QKeyEvent* e) {
  _currentKey = Qt::NoButton;

  e->ignore();
}
