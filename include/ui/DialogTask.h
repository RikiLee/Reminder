#pragma once
#include <QDialog>
#include "ui_task.h"
#include "Ticker.h"

namespace reminder
{
	class DialogTask :public QDialog
	{
		Q_OBJECT
	public:
		explicit DialogTask(QWidget* parent = nullptr);
		~DialogTask();

		const Schedule& getSchedule() const noexcept { return m_schedule; }

		int editSchedule(const Schedule& sch);
		int addSchedule() 
		{
			setWindowTitle(tr("添加待办"));
			return exec(); 
		}

	private:
		Ui::Dialog ui;
		Schedule m_schedule;
		bool needCheck = true;

		void setDialogValue();

	public slots:
		void onOk();
		void onCancel();
		void onEventTypeChanged();
		void onLoopTypeChanged();
		void onWeekdayChanged();
		void onSelectedFile();
	};
}
