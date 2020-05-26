#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <memory>

#include <QApplication>
#include <QGraphicsView>

#include "digital_paint_main_window.h"

int main(int argc, char** args)
{
  auto* app = new QApplication(argc, args);

  DigitalPaintMainWindow view(nullptr);
  view.showMaximized();

  return app->exec();
}
