#include <QApplication>

#include "mainwindow.h"

int main(int argc, char *argv[])
{

   QApplication app(argc, argv);
   MainWindow window;
   window.show();
   return app.exec();
   
}


/*

#include <QApplication>
#include <QWidget>
#include <QLabel>
#include <QPushButton>

class MyButton : public QWidget {

 public:
     MyButton(QWidget *parent = nullptr);
};

MyButton::MyButton(QWidget *parent)
    : QWidget(parent) {

  auto *quitBtn = new QPushButton("Quit", this);
  quitBtn->setGeometry(50, 40, 75, 30);

  connect(quitBtn, &QPushButton::clicked, qApp, &QApplication::quit);
}

int main(int argc, char *argv[]) {

  QApplication app(argc, argv);
  QLabel label("Hello William QT!");

  MyButton window;

  window.resize(250, 150);
  window.setWindowTitle("QPushButton");
  label.show();
  window.show();

  return app.exec();
}
*/
