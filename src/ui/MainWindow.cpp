#include "ui/MainWindow.h"
#include "ui/DialogTask.h"
#include "ui/DialogEventViewer.h"
#include <chrono>
#include <QTimer>
#include <QDateTime>
#include <QCloseEvent>
#include <QMessageBox>

namespace reminder
{
	MainWindow::MainWindow(QWidget* parent)
		:QMainWindow(parent)
	{
		Qt::WindowFlags flags = windowFlags();
		flags &= ~(Qt::WindowMaximizeButtonHint | Qt::WindowMinimizeButtonHint);
		setWindowFlags(flags);

		ui.setupUi(this);

		auto& ticker = Ticker::instance();
		connect(&ticker, &Ticker::updateClock, this, &MainWindow::updateClock);

		taskEditDialog = new DialogTask(this);
		connect(ui.action_newTask, &QAction::triggered, [this]() { addTask(); });

		eventViewerDialog = new DialogEventViewer(this);
		connect(ui.action_viewHistroyQueue, &QAction::triggered, 
			[this]()
			{
				eventViewerDialog->setCurrentListIndex(1);
				eventViewerDialog->show();
			}
		);
		connect(ui.action_viewCheckQueue, &QAction::triggered, 
			[this]()
			{
				eventViewerDialog->setCurrentListIndex(0);
				eventViewerDialog->show();
			}
		);
	}

	MainWindow::~MainWindow()
	{
	}

	void MainWindow::addTask()
	{
		if (taskEditDialog->addSchedule() == QDialog::Accepted)
		{
			auto& ticker = Ticker::instance();
			ticker.addTask(taskEditDialog->getSchedule());
		}
	}

	void MainWindow::modifyTask(size_t i)
	{
		auto& ticker = Ticker::instance();
		if (taskEditDialog->editSchedule(ticker.getCommonSchedule(i)) == QDialog::Accepted)
		{
			ticker.updateTask(i, taskEditDialog->getSchedule());
		}
	}

	void MainWindow::closeEvent(QCloseEvent* event)
	{
		event->ignore();
		hide();
	}

	void MainWindow::updateClock()
	{
		if (!isHidden())
		{
			QString timeString = QDateTime::currentDateTime().toString("HH:mm:ss");

			ui.lcdNumber->display(timeString);
		}
	}
}
