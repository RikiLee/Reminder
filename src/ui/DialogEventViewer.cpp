#include "ui/DialogEventViewer.h"
#include "ui/TaskItemDelegate.h"
#include "ui/MainWindow.h"
#include <sstream>
#include <iomanip>
#include <QStandardItemModel>

namespace reminder
{
	DialogEventViewer::DialogEventViewer(QWidget* parent)
		: QDialog(parent)
	{
		ui.setupUi(this);

		ListViewTaskItemDelegate* listViewDelegate = new ListViewTaskItemDelegate(this);
		m_historyListModel = new QStandardItemModel(this);
		ui.listView_history->setItemDelegate(listViewDelegate);
		ui.listView_history->setModel(m_historyListModel);
		ui.listView_history->setSelectionMode(QAbstractItemView::SingleSelection);
		ui.listView_history->setEditTriggers(QAbstractItemView::NoEditTriggers);

		assert(dynamic_cast<MainWindow*>(parent) != nullptr);
		TreeViewTaskItemDelegate* treeViewDelegate = new TreeViewTaskItemDelegate(static_cast<MainWindow*>(parent), this);
		m_commonTreeViewModel = new QStandardItemModel(this);
		ui.treeView->setItemDelegate(treeViewDelegate);
		ui.treeView->setModel(m_commonTreeViewModel);
		ui.treeView->setSelectionMode(QAbstractItemView::SingleSelection);
		ui.treeView->setEditTriggers(QAbstractItemView::NoEditTriggers);
		ui.treeView->setHeaderHidden(true);
		ui.treeView->expandAll();

		QStandardItem* onceEventRoot = new QStandardItem();
		onceEventRoot->setData(tr("一次性事件"), Qt::DisplayRole);
		m_commonTreeViewModel->appendRow(onceEventRoot);
		QStandardItem* loopEventRoot = new QStandardItem();
		loopEventRoot->setData(tr("循环事件"), Qt::DisplayRole);
		m_commonTreeViewModel->appendRow(loopEventRoot);


		auto& ticker = Ticker::instance();
		connect(&ticker, &Ticker::updateHistoryQueues, this, &DialogEventViewer::onHistoryQueueUpdate);
		connect(&ticker, &Ticker::updateCheckQueues, this, &DialogEventViewer::onCheckQueueUpdate);
		ticker.emitUpdateQueuesOnce();
	}

	DialogEventViewer::~DialogEventViewer()
	{
	}

	std::string DialogEventViewer::getShowInfo(const Schedule& sch)
	{
		std::ostringstream oss;
		oss << sch.m_event << "  ";
		if (sch.m_scheduleType == ScheduleType::Once)
		{
			oss << static_cast<uint32_t>(sch.m_timePoint.year) << "Year " << std::setfill('0') << std::setw(2) << static_cast<uint32_t>(sch.m_timePoint.month) << "Mon " << std::setfill('0') << std::setw(2) << static_cast<uint32_t>(sch.m_timePoint.day) << "Day ";
			if (sch.m_timePoint.hour != ScheduleTime::DoNotCheckHour)
			{
				oss << std::setfill('0') << std::setw(2) << static_cast<uint32_t>(sch.m_timePoint.hour) << "Hour " << std::setfill('0') << std::setw(2) << static_cast<uint32_t>(sch.m_timePoint.minute) << "Min ";
			}
		}
		else
		{
			switch (sch.m_loopType)
			{
			case LoopType::EveryYear:
				oss << "Every Year " << std::setfill('0') << std::setw(2) << static_cast<uint32_t>(sch.m_timePoint.month) << "Mon " << std::setfill('0') << std::setw(2) << static_cast<uint32_t>(sch.m_timePoint.day) << "Day ";
				if (sch.m_timePoint.hour != ScheduleTime::DoNotCheckHour)
				{
					oss << std::setfill('0') << std::setw(2) << static_cast<uint32_t>(sch.m_timePoint.hour) << "Hour " << std::setfill('0') << std::setw(2) << static_cast<uint32_t>(sch.m_timePoint.minute) << "Min ";
				}
				break;
			case LoopType::EveryMonth:
				oss << "Every Month " << std::setfill('0') << std::setw(2) << static_cast<uint32_t>(sch.m_timePoint.day) << "Day ";
				if (sch.m_timePoint.hour != ScheduleTime::DoNotCheckHour)
				{
					oss << std::setfill('0') << std::setw(2) << static_cast<uint32_t>(sch.m_timePoint.hour) << "Hour " << std::setfill('0') << std::setw(2) << static_cast<uint32_t>(sch.m_timePoint.minute) << "Min ";
				}
				break;
			case LoopType::EveryWeek:
				oss << "Every " << convertToString(sch.m_timePoint.weekday) << " ";
				if (sch.m_timePoint.hour != ScheduleTime::DoNotCheckHour)
				{
					oss << std::setfill('0') << std::setw(2) << static_cast<uint32_t>(sch.m_timePoint.hour) << "Hour " << std::setfill('0') << std::setw(2) << static_cast<uint32_t>(sch.m_timePoint.minute) << "Min ";
				}
				break;
			case LoopType::EveryDay:
				oss << "Every day ";
				if (sch.m_timePoint.hour != ScheduleTime::DoNotCheckHour)
				{
					oss << std::setfill('0') << std::setw(2) << static_cast<uint32_t>(sch.m_timePoint.hour) << "Hour " << std::setfill('0') << std::setw(2) << static_cast<uint32_t>(sch.m_timePoint.minute) << "Min ";
				}
				break;
			case LoopType::EveryHour:
				oss << "Every Hour ";
				oss << std::setfill('0') << std::setw(2) << static_cast<uint32_t>(sch.m_timePoint.minute) << "Min ";
				break;
			default:
				break;
			}
		}
		switch (sch.m_displayEffect)
		{
		case DisplayEffect::Text:
			oss << "Text";
			break;
		case DisplayEffect::Image:
			oss << "Image";
			break;
		case DisplayEffect::Audio:
			oss << "Audio";
			break;
		case DisplayEffect::Video:
			oss << "Video";
			break;
		case DisplayEffect::WebSite:
			oss << "WebSite";
			break;
		default:
			break;
		}
		return oss.str();
	}

	void DialogEventViewer::onHistoryQueueUpdate(const std::vector<std::pair<Schedule, size_t>>& hqs)
	{
		m_historyListModel->clear();
		for (const auto& [sch, id] : hqs)
		{
			auto str = getShowInfo(sch);
			QStandardItem* item = new QStandardItem();
			item->setData(QString::fromStdString(str), Qt::DisplayRole);
			item->setData(id, Qt::UserRole);
			m_historyListModel->appendRow(item);
		}
	}

	void DialogEventViewer::onCheckQueueUpdate(const std::vector<std::pair<Schedule, size_t>>& cqs)
	{
		m_commonTreeViewModel->clear();
		QStandardItem* onceEventRoot = new QStandardItem();
		onceEventRoot->setData(tr("一次性事件"), Qt::DisplayRole);
		m_commonTreeViewModel->appendRow(onceEventRoot);
		QStandardItem* loopEventRoot = new QStandardItem();
		loopEventRoot->setData(tr("循环事件"), Qt::DisplayRole);
		m_commonTreeViewModel->appendRow(loopEventRoot);
		for (const auto& [sch, id] : cqs)
		{
			auto str = getShowInfo(sch);
			if (sch.m_scheduleType == ScheduleType::Once)
			{
				QStandardItem* item = new QStandardItem();
				item->setData(QString::fromStdString(str), Qt::DisplayRole);
				item->setData(id, Qt::UserRole);
				onceEventRoot->appendRow(item);
			}
			else
			{
				QStandardItem* item = new QStandardItem();
				item->setData(QString::fromStdString(str), Qt::DisplayRole);
				item->setData(id, Qt::UserRole);
				loopEventRoot->appendRow(item);
			}
		}
		ui.treeView->expandAll();
	}

}
