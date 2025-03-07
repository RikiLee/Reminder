#include "ui/TaskItemDelegate.h"
#include "ui/MainWindow.h"
#include "Ticker.h"
#include <QPainter>
#include <QMouseEvent>
#include <QApplication>

namespace reminder
{
	TreeViewTaskItemDelegate::TreeViewTaskItemDelegate(MainWindow* mainWindow, QWidget* parent)
		: QStyledItemDelegate(parent), m_mainWindow(mainWindow)
	{
	}

	void TreeViewTaskItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
	{
		if (index.parent().isValid()) 
		{
			painter->save();
			painter->setRenderHint(QPainter::Antialiasing, true);
			painter->fillRect(option.rect, option.backgroundBrush);

			QRect labelRect = option.rect;
			labelRect.setLeft(45);
			labelRect.setWidth(option.rect.width() - 160);
			painter->drawText(labelRect, Qt::AlignLeft | Qt::AlignVCenter, index.data(Qt::DisplayRole).toString());

			QRect buttonModifyRect = option.rect;
			buttonModifyRect.setLeft(labelRect.right() + 5);
			buttonModifyRect.setWidth(50);
			QStyleOptionButton buttonModifyOption;
			buttonModifyOption.rect = buttonModifyRect;
			buttonModifyOption.text = tr("修改");
			buttonModifyOption.state = QStyle::State_Enabled;
			QApplication::style()->drawControl(QStyle::CE_PushButton, &buttonModifyOption, painter);

			QRect buttonDelRect = option.rect;
			buttonDelRect.setLeft(option.rect.width() - 55);
			buttonDelRect.setWidth(50);
			QStyleOptionButton buttonDelOption;
			buttonDelOption.rect = buttonDelRect;
			buttonDelOption.text = tr("删除");
			buttonDelOption.state = QStyle::State_Enabled;
			QApplication::style()->drawControl(QStyle::CE_PushButton, &buttonDelOption, painter);

			painter->restore();
		}
		else
		{
			QStyledItemDelegate::paint(painter, option, index);
		}
	}

	bool TreeViewTaskItemDelegate::editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index)
	{
		if (event->type() == QEvent::MouseButtonPress && index.parent().isValid()) {
			QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
			QRect deleteRect = QRect(option.rect.width() - 55, option.rect.top(), 50, option.rect.height());
			QRect editRect = QRect(option.rect.width() - 110, option.rect.top(), 50, option.rect.height());

			if (editRect.contains(mouseEvent->pos())) 
			{
				assert(m_mainWindow != nullptr);
				m_mainWindow->modifyTask(index.data(Qt::UserRole).toULongLong());
				return true;
			}
			else if (deleteRect.contains(mouseEvent->pos())) 
			{
				Ticker::instance().removeTask(index.data(Qt::UserRole).toULongLong());
				return true;
			}
		}
		return QStyledItemDelegate::editorEvent(event, model, option, index);
	}


	ListViewTaskItemDelegate::ListViewTaskItemDelegate(QWidget* parent)
		: QStyledItemDelegate(parent)
	{
	}


	void ListViewTaskItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
	{
		painter->save();
		painter->setRenderHint(QPainter::Antialiasing, true);
		painter->fillRect(option.rect, option.backgroundBrush);

		QRect labelRect = option.rect;
		labelRect.setLeft(5);
		labelRect.setWidth(option.rect.width() - 65);
		painter->drawText(labelRect, Qt::AlignLeft | Qt::AlignVCenter, index.data(Qt::DisplayRole).toString());

		QRect buttonDelRect = option.rect;
		buttonDelRect.setLeft(option.rect.width() - 55);
		buttonDelRect.setWidth(50);
		QStyleOptionButton buttonDelOption;
		buttonDelOption.rect = buttonDelRect;
		buttonDelOption.text = tr("删除");
		buttonDelOption.state = QStyle::State_Enabled;
		QApplication::style()->drawControl(QStyle::CE_PushButton, &buttonDelOption, painter);

		painter->restore();
	}

	bool ListViewTaskItemDelegate::editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index)
	{
		if (event->type() == QEvent::MouseButtonPress)
		{
			QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);

			QRect pushButtonDelRect = option.rect;
			pushButtonDelRect.setLeft(option.rect.width() - 55);
			pushButtonDelRect.setWidth(50);
			if (pushButtonDelRect.contains(mouseEvent->pos()))
			{
				Ticker::instance().removeHistory(index.data(Qt::UserRole).toULongLong());
				return true;
			}
		}
		return QStyledItemDelegate::editorEvent(event, model, option, index);
	}
}


