#include <QApplication>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include <QStyleFactory>
#include <QLocalServer>
#include <QLocalSocket>
#include "ui/MainWindow.h"
#include "Ticker.h"
#include "ui/NotifyWidget.h"
#ifdef _WIN32
	#define _CRTDBG_MAP_ALLOC
	#include <stdlib.h>
	#include <crtdbg.h>
#endif

int main(int argc, char *argv[]) 
{
#ifdef _WIN32
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF); // Enable memory leak detection
#endif

    QApplication app(argc, argv);

	static const char* const uniqueAppName = "Reminder_UniqueAppServer";

	QLocalSocket socket;
	socket.connectToServer(uniqueAppName);
	if (socket.waitForConnected(500)) 
	{
#ifdef _WIN32
		_CrtDumpMemoryLeaks(); // Dump memory leaks
#endif
		return 1;
	}

	// 创建本地服务器
	QLocalServer server;
	if (!server.listen(uniqueAppName))
	{
#ifdef _WIN32
		_CrtDumpMemoryLeaks(); // Dump memory leaks
#endif
		return 1;
	}


	app.setStyle(QStyleFactory::create("Fusion"));

    reminder::MainWindow window;
	reminder::DialogMediaPlayer notifyDialog;

    QSystemTrayIcon trayIcon;

	QIcon icon;
	icon.addFile(QString::fromUtf8(":/images/Icon_min.png"), QSize(), QIcon::Mode::Normal, QIcon::State::Off);
	trayIcon.setIcon(icon);
	trayIcon.setToolTip(QObject::tr("reminder"));

	QScopedPointer<QMenu> trayIconMenu(new QMenu());
	QAction* helpAction = trayIconMenu->addAction(QObject::tr("帮助"));
	QAction* showAction = trayIconMenu->addAction(QObject::tr("显示主界面"));
	QAction* exitAction = trayIconMenu->addAction(QObject::tr("退出"));
	trayIcon.setContextMenu(trayIconMenu.get());

	QObject::connect(helpAction, &QAction::triggered, [&window]() { QMessageBox::information(&window, "", "编辑文件用以更改设置"); });
	QObject::connect(showAction, &QAction::triggered, &window, &QMainWindow::show);
	QObject::connect(exitAction, &QAction::triggered, 
		[&app]() 
		{ 
			app.quit();  
#ifdef _WIN32
	        _CrtDumpMemoryLeaks(); // Dump memory leaks
#endif
		}
	);

	QObject::connect(&trayIcon, &QSystemTrayIcon::activated, 
		[&window](QSystemTrayIcon::ActivationReason reason) 
		{ 
			if (reason == QSystemTrayIcon::DoubleClick) { window.show(); }
		}
	);

	trayIcon.show();

    window.show();

    return app.exec();
}