#ifndef _MAIN_WINDOW_H
#define _MAIN_WINDOW_H

#include <qmainwindow.h>
#include <qwidget.h>
#include <qtimer.h>
#include "rendergl.h"
#include "gleswidget.h"

class MainWindow : public QMainWindow {
Q_OBJECT
public:
	MainWindow();
	~MainWindow();

	void Init();
    
private:
	GLESWidget * mainWindowWidget_;
	QTimer * Time;
    bool bGL2Render;
    RenderGL* rendergl;
private slots:
	void Render();
};

#endif
