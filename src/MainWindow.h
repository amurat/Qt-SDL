#ifndef _MAIN_WINDOW_H
#define _MAIN_WINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QTimer>
#include "glesrhiwidget.h"

class MainWindow : public QMainWindow {
Q_OBJECT
public:
	MainWindow();
	~MainWindow();

	void Init();

protected:
    void closeEvent(QCloseEvent *event);
    
private:
	GLESRhiWidget * mainWindowWidget_;
	QTimer * Time;
    bool running_;
private slots:
	void Render();
};

#endif
