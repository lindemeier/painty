#include "digital_paint_main_window.h"
#include "ui_digital_paint_main_window.h"

#include "digital_canvas.h"

#include <painty/image_io.h>
#include <painty/renderer.h>

#include <QMainWindow>
#include <QProgressBar>
#include <QFileDialog>
#include <QMessageBox>

DigitalPaintMainWindow::DigitalPaintMainWindow(QWidget* parent)
  : QMainWindow(parent), ui(new Ui::DigitalPaintMainWindow)
{
  ui->setupUi(this);
  m_canvasView = new DigitalCanvasView(1024U, 768U, this, ui->pickupMapLabel);

  this->setCentralWidget(m_canvasView);

  KMVisualizeWidget* km = ui->kmWidget;

  QColorDialog* Rw = ui->colorDialog_Rw;
  Rw->setWindowFlags(Qt::Widget);
  Rw->setMinimumSize(Rw->sizeHint());
  Rw->setWindowTitle("Select reflectance on white surface");
  Rw->setWindowFlags(Qt::Widget);
  Rw->setOptions(QColorDialog::DontUseNativeDialog | QColorDialog::NoButtons);
  Rw->setCurrentColor(km->getRw());
  connect(Rw, SIGNAL(currentColorChanged(const QColor&)), this, SLOT(setPaintingColorRw()));

  connect(ui->rb_slider, SIGNAL(valueChanged(int)), this, SLOT(setPaintingColorRw()));
  connect(ui->brushRadiusSlider, SIGNAL(valueChanged(int)), m_canvasView->getDigitalCanvas(),
          SLOT(setBrushRadius(int)));
  connect(ui->dryCanvasButton, SIGNAL(pressed()), m_canvasView->getDigitalCanvas(), SLOT(dryCanvas()));

  m_canvasView->getDigitalCanvas()->setBrushRadius(ui->brushRadiusSlider->value());
  setPaintingColorRw();

  QAction* newAct = new QAction(tr("&New"), this);
  newAct->setShortcuts(QKeySequence::New);
  newAct->setStatusTip(tr("Create a new canvas"));
  QMenu* fileMenu = menuBar()->addMenu(tr("&File"));
  fileMenu->addAction(newAct);
  connect(newAct, &QAction::triggered, [=]() {
    m_canvasView->getDigitalCanvas()->getCanvas()->clear();
    m_canvasView->getDigitalCanvas()->updateCanvas();
  });

  QAction* saveAct = new QAction(tr("&Save As"), this);
  saveAct->setShortcuts(QKeySequence::SaveAs);
  saveAct->setStatusTip(tr("Save as image file"));
  fileMenu->addAction(saveAct);
  connect(saveAct, &QAction::triggered, [=]() {
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Image"), ".", tr("Images (*.png)"));
    if (fileName.isEmpty())
    {
      return;
    }
    painty::Renderer<painty::vec3> renderer;
    const auto image = renderer.compose(*m_canvasView->getDigitalCanvas()->getCanvas());
    painty::io::imSave(fileName.toStdString(), image);
  });

  QAction* aboutAct = new QAction(tr("&about"), this);
  aboutAct->setStatusTip(tr("About..."));
  QMenu* helpMenu = menuBar()->addMenu(tr("&About"));
  helpMenu->addAction(aboutAct);
  connect(aboutAct, &QAction::triggered, [=]() {
    QMessageBox::about(this, tr("About Application"),
                       tr("Created by:\t\tThomas "
                          "Lindemeier\nDate:\t\t2017\n\nEmail:\t\tthomas.lindemeier@gmail.com\n\nSimple program "
                          "demonstrating the creation of brush strokes."));
  });

  ui->spinBoxPickupRate->setValue(m_canvasView->getDigitalCanvas()->getFootprintBrushPtr()->getPickupRate());
  ui->spinBoxDepositionRate->setValue(m_canvasView->getDigitalCanvas()->getFootprintBrushPtr()->getDepositionRate());
  connect(ui->spinBoxPickupRate, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
          [=](double v) { m_canvasView->getDigitalCanvas()->getFootprintBrushPtr()->setPickupRate(v); });
  connect(ui->spinBoxDepositionRate, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
          [=](double v) { m_canvasView->getDigitalCanvas()->getFootprintBrushPtr()->setDepositionRate(v); });

  ui->spinBoxDryingTime->setValue(
      static_cast<double>(m_canvasView->getDigitalCanvas()->getCanvas()->getDryingTime().count()));
  connect(ui->spinBoxDryingTime, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=](double v) {
    m_canvasView->getDigitalCanvas()->getCanvas()->setDryingTime(
        std::chrono::milliseconds(static_cast<uint32_t>(v * 1000.0)));
  });

  ui->checkBoxUseSnapshotBuffer->setChecked(
      m_canvasView->getDigitalCanvas()->getFootprintBrushPtr()->getUseSnapshotBuffer());
  connect(ui->checkBoxUseSnapshotBuffer, QOverload<int>::of(&QCheckBox::stateChanged), [=](int state) {
    const auto checked = (state == Qt::Checked);
    m_canvasView->getDigitalCanvas()->getFootprintBrushPtr()->setUseSnapshotBuffer(checked);
  });

  connect(ui->radioButtonFootprintBrush, QOverload<bool>::of(&QRadioButton::clicked),
          [=](bool clicked) { m_canvasView->getDigitalCanvas()->setUseFootprintBrush(clicked); });
  connect(ui->radioButtonTextureBrush, QOverload<bool>::of(&QRadioButton::clicked),
          [=](bool clicked) { m_canvasView->getDigitalCanvas()->setUseFootprintBrush(!clicked); });
}

DigitalPaintMainWindow::~DigitalPaintMainWindow()
{
  delete ui;
}

void DigitalPaintMainWindow::setPaintingColorRw()
{
  QColor rw = ui->colorDialog_Rw->currentColor();
  QColor rb = rw.lighter(ui->rb_slider->value());

  const auto clamp = [](int32_t x, int32_t min, int32_t max) { return std::min(max, std::max(x, min)); };

  QColor Rwc_clamped(clamp(rw.red(), 2, 254), clamp(rw.green(), 2, 254), clamp(rw.blue(), 2, 254));
  QColor Rbc_clamped(clamp(rb.red(), 1, Rwc_clamped.red() - 1), clamp(rb.green(), 1, Rwc_clamped.green() - 1),
                     clamp(rb.blue(), 1, Rwc_clamped.blue() - 1));

  ui->kmWidget->setRb(Rbc_clamped);
  ui->kmWidget->setRw(Rwc_clamped);
  m_canvasView->getDigitalCanvas()->setColor(Rbc_clamped, Rwc_clamped);
  ui->colorDialog_Rw->setCurrentColor(Rwc_clamped);
}
