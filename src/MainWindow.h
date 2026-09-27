#ifndef _MAIN_WINDOW_H
#define _MAIN_WINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QTimer>
#include "anglerhiwidget.h"

class MainWindow : public QMainWindow {
Q_OBJECT
public:
	MainWindow();
	~MainWindow();

	void Init();

protected:
    void closeEvent(QCloseEvent *event);
    
private:
	ANGLERhiWidget * mainWindowWidget_;
	QTimer * Time;
    bool running_;
private slots:
	void Render();
};

#endif
