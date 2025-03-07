#pragma once
#include <QMainWindow>
#include "ui_MainWindow.h"
#include "Ticker.h"

class Ui::MainWindow;
class QTimer;

namespace reminder
{
	class DialogTask;
	class DialogEventViewer;
	class MainWindow : public QMainWindow
	{
		Q_OBJECT
	public:
		explicit MainWindow(QWidget* parent = nullptr);
		virtual ~MainWindow();

		void addTask();
		void modifyTask(size_t i);

	protected:
		void closeEvent(QCloseEvent* event) override;

	private:
		Ui::MainWindow ui;
		DialogTask* taskEditDialog;
		DialogEventViewer* eventViewerDialog;

	public slots:
		void updateClock();
	};
}