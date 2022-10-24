#ifndef _MAIN_WINDOW_H
#define _MAIN_WINDOW_H

#include <qmainwindow.h>
#include <qwidget.h>
#include <qtimer.h>

class MainWindow : public QMainWindow {
Q_OBJECT
public:
	MainWindow();
	~MainWindow();

	void EGLInit();
private:
	QWidget * MainWindowWidget;
	QTimer * Time;
	int position;
	int dir;
private slots:
	void Render();
};

#endif
