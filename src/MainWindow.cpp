#include "MainWindow.h"
#include "glesrhiwidget.h"

MainWindow::MainWindow() : mainWindowWidget_(0), running_(false) {
    mainWindowWidget_ = new GLESRhiWidget();
	setWindowTitle("QMainWindow EGL Rendering Example");
	setCentralWidget(mainWindowWidget_);	// Basic setup, ensuring that the window has a widget
	setBaseSize(640, 480);				// inside of it that we can render to
	resize(640, 480);

	/*
		The timer requests a new frame; GLESRhiWidget renders it
		(into its QRhi color texture) from its render() override.
	*/
	Time = new QTimer(this);
	connect(Time, SIGNAL(timeout()), this, SLOT(Render()));
	Time->start(1000 / 60);
    Init();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    running_ = false;
    //std::cout << "closeEvent" << std::endl;
}

MainWindow::~MainWindow() {

	delete Time;
	Time = 0;
}

void MainWindow::Init() {
    // renderer setup happens in GLESRhiWidget::initialize()
    running_ = true;
}

void MainWindow::Render()
{
    if (!running_)
        return;

    mainWindowWidget_->update();
}
