#pragma once
#include <QDialog>
#include "ui_mediaPlayer.h"
#include "Ticker.h"

class QMediaPlayer;
class QAudioOutput;
class QMediaDevices;
namespace reminder
{
	class DialogMediaPlayer : public QDialog
	{
		Q_OBJECT
	public:
		explicit DialogMediaPlayer(QWidget* parent = nullptr);
		~DialogMediaPlayer();

	private:
		Ui::DialogMediaPlayer ui;
		QMediaDevices* m_mediaDevices;
		QMediaPlayer* m_mediaPlayer;
		QAudioOutput* m_audioOutput;
		qint64 m_videoDuration = 10ll * 60 * 1000;
		qint64 m_audioDuration = 10ll * 60 * 1000;
		std::queue<Schedule> m_outOfDateQueue;
		std::queue<Schedule> m_commonQueue;
		TimePoint m_showStartTime;
		Schedule m_currentSch;
		bool m_progressPressed = false;

		void display();
		inline std::string convertToTimeString(qint64 position, qint64 duration);
		void hideAndReset();


	public slots:
		void updateDisplay();
		void onVideoDurationChanged(qint64 duration);
		void onAudioDurationChanged(qint64 duration);
		void onOK();
		void showOutOfDateNotification(std::queue<Schedule>& schedules);
		void showNotification(std::queue<Schedule>& schedules);
	};
}
