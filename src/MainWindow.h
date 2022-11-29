#ifndef _MAIN_WINDOW_H
#define _MAIN_WINDOW_H

#include <qmainwindow.h>
#include <qwidget.h>
#include <qtimer.h>
#include "rendergl.h"

class MainWindow : public QMainWindow {
Q_OBJECT
public:
	MainWindow();
	~MainWindow();

	void EGLInit();
    void EGLTerminate();
    
private:
	QWidget * MainWindowWidget;
	QTimer * Time;
	int position;
	int dir;
    bool bGL2Render;
    RenderGL* rendergl;
private slots:
	void Render();
};

#endif
