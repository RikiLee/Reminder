#pragma once
#include <QStyledItemDelegate>

namespace reminder
{
    class MainWindow;
    class TreeViewTaskItemDelegate : public QStyledItemDelegate
    {
        Q_OBJECT
    public:
        TreeViewTaskItemDelegate(MainWindow* mainWindow, QWidget* parent = nullptr);

        void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

        bool editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index) override;

    private:
        MainWindow* m_mainWindow;
    };

    class ListViewTaskItemDelegate : public QStyledItemDelegate
    {
        Q_OBJECT
    public:
        ListViewTaskItemDelegate(QWidget* parent = nullptr);

        void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

        bool editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option, const QModelIndex& index) override;

    private:

    };
}