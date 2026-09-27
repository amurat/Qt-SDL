#include <QApplication>

#include "MainWindow.h"

int main(int argc, char * argv[]) {
	QApplication a(argc, argv); // Basics of QT, start an application

	MainWindow mainWinExample;
	//mainWinExample.EGLInit();
#if defined(Q_OS_IOS)
	mainWinExample.showMaximized();
#else
	mainWinExample.show();
#endif
	
	int RetVal = a.exec();	// Most examples have this on the return, we
							// need to have it return to a variable cause:
    return RetVal;
}
