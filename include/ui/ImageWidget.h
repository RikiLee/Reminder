#pragma once
#include <QWidget>
#include <QPainter>
#include <QPixmap>
#include <QPaintEvent>

class ImageWidget : public QWidget {
public:
    ImageWidget(QWidget* parent = nullptr)
        : QWidget(parent)
    {
    }

    ~ImageWidget() {}

    void setImage(const QString& imagePath) {
        m_pixmap
            = QPixmap(imagePath);
        if (!m_pixmap.isNull()) {
            resize(m_pixmap.size());
        }
    }

protected:
    void paintEvent(QPaintEvent* event) override {
        QPainter painter(this);
        if (!m_pixmap.isNull()) {
            painter.drawPixmap(0, 0, m_pixmap);
        }
    }

private:
    QPixmap m_pixmap;
};
