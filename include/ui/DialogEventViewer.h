#pragma once
#include <QDialog>
#include "ui_eventList.h"
#include "Ticker.h"

class QStandardItemModel;
namespace reminder
{
	class DialogEventViewer : public QDialog
	{
		Q_OBJECT
	public:
		explicit DialogEventViewer(QWidget* parent = nullptr);
		~DialogEventViewer();

		void setCurrentListIndex(int id) { ui.stackedWidget->setCurrentIndex(id); }

	private:
		Ui::DialogEventList ui;
		QStandardItemModel* m_historyListModel;
		QStandardItemModel* m_commonTreeViewModel;

		std::string getShowInfo(const Schedule& sch);
		std::string convertToString(Weekdays weekday)
		{
			std::string result;
			switch (weekday)
			{
			case reminder::Weekdays::Sunday:
				result = "Sunday";
				break;
			case reminder::Weekdays::Monday:
				result = "Monday";
				break;
			case reminder::Weekdays::Tuesday:
				result = "Tuesday";
				break;
			case reminder::Weekdays::Wednesday:
				result = "Wednesday";
				break;
			case reminder::Weekdays::Thursday:
				result = "Thursday";
				break;
			case reminder::Weekdays::Friday:
				result = "Friday";
				break;
			case reminder::Weekdays::Saturday:
				result = "Saturday";
				break;
			case reminder::Weekdays::None:
				break;
			default:
				break;
			}
			return result;
		}


	public slots:
		void onHistoryQueueUpdate(const std::vector<std::pair<Schedule, size_t>> &hqs);
		void onCheckQueueUpdate(const std::vector<std::pair<Schedule, size_t>>& cqs);
	};
}
