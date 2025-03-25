#include "ui/NotifyWidget.h"
#include <iomanip>
#include <sstream>
#include <QStyle>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QMediaDevices>
#include <QAudioDevice>
#ifdef WIN32
#include <Windows.h>
#endif

namespace reminder
{
	DialogMediaPlayer::DialogMediaPlayer(QWidget* parent)
		: QDialog(parent)
	{
		setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
		ui.setupUi(this);

		hide();

		m_mediaDevices = new QMediaDevices(this);
		m_mediaPlayer = new QMediaPlayer(this);
		m_audioOutput = new QAudioOutput(this);
		m_mediaPlayer->setAudioOutput(m_audioOutput);
		m_audioOutput->setVolume(1.f);

		connect(m_mediaDevices, &QMediaDevices::audioOutputsChanged,
			[this]() 
			{
				m_audioOutput->setDevice(QMediaDevices::defaultAudioOutput());
			}
		);

		/// audio player
		// play or pause
		connect(ui.pushButton_playAudio, &QPushButton::clicked,
			[this]()
			{
				if (m_mediaPlayer->isPlaying())
				{
					ui.pushButton_playAudio->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::MediaPlaybackStart));
					m_mediaPlayer->pause();
				}
				else
				{
					ui.pushButton_playAudio->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::MediaPlaybackPause));
					m_mediaPlayer->play();
				}
			}
		);
		// volume
		connect(ui.horizontalSlider_volumeAudio, &QSlider::sliderMoved, 
			[this](int value) 
			{ 
				float volume = static_cast<float>(value) / 100.f;
				m_audioOutput->setVolume(volume);
				if (volume > 0.66f)
				{
					ui.pushButton_mutexAudio->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::AudioVolumeHigh));
				}
				else if (volume > 0.33f)
				{
					ui.pushButton_mutexAudio->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::AudioVolumeMedium));
				}
				else
				{
					ui.pushButton_mutexAudio->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::AudioVolumeLow));
				}
			}
		);
		connect(ui.pushButton_mutexAudio, &QPushButton::clicked,
			[this]()
			{
				float v = m_audioOutput->volume();
				if (v == 0.f)
				{
					ui.horizontalSlider_volumeAudio->setEnabled(true);
					float volume = static_cast<float>(ui.horizontalSlider_volumeAudio->value()) / 100.f;
					m_audioOutput->setVolume(volume);
					if (volume > 0.66f)
					{
						ui.pushButton_mutexAudio->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::AudioVolumeHigh));
					}
					else if (volume > 0.33f)
					{
						ui.pushButton_mutexAudio->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::AudioVolumeMedium));
					}
					else
					{
						ui.pushButton_mutexAudio->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::AudioVolumeLow));
					}
				}
				else
				{
					ui.horizontalSlider_volumeAudio->setDisabled(true);
					m_audioOutput->setVolume(0.f);
					ui.pushButton_mutexAudio->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::AudioVolumeMuted));
				}
			});
		// progress
		auto maxRangeValue = ui.horizontalSlider_progressAudio->maximum();
		connect(ui.horizontalSlider_progressAudio, &QSlider::sliderPressed,
			[this]()
			{
				m_progressPressed = true;
			}
		);
		connect(ui.horizontalSlider_progressAudio, &QSlider::sliderReleased,
			[this, maxRangeValue]()
			{
				qint64 pos = static_cast<qint64>(
					static_cast<double>(ui.horizontalSlider_progressAudio->value() * m_videoDuration) / static_cast<double>(maxRangeValue));
				m_mediaPlayer->setPosition(pos);

				auto timeStr = convertToTimeString(pos, m_audioDuration);
				ui.label_timeAudio->setText(QString::fromStdString(timeStr));
			}
		);
		connect(m_mediaPlayer, &QMediaPlayer::positionChanged,
			[this, maxRangeValue](qint64 pos)
			{
				if (!m_progressPressed)
				{
					// update progress
					int v = pos * maxRangeValue / m_audioDuration;
					ui.horizontalSlider_progressAudio->setValue(v);
				}
				// update play time
				auto timeStr = convertToTimeString(pos, m_audioDuration);
				ui.label_timeAudio->setText(QString::fromStdString(timeStr));
			}
		);
		connect(m_mediaPlayer, &QMediaPlayer::durationChanged, this, &DialogMediaPlayer::onAudioDurationChanged);

		/// video player
		m_mediaPlayer->setVideoOutput(ui.widget_Video);
		// play or pause
		connect(ui.pushButton_playVideo, &QPushButton::clicked, 
			[this]()
			{
				if (m_mediaPlayer->isPlaying())
				{
					ui.pushButton_playVideo->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::MediaPlaybackStart));
					m_mediaPlayer->pause();
				}
				else
				{
					ui.pushButton_playVideo->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::MediaPlaybackPause));
					m_mediaPlayer->play();
				}
			}
		);
		// volume
		connect(ui.horizontalSlider_volumeVideo, &QSlider::sliderMoved, 
			[this](int value) 
			{ 
				float volume = static_cast<float>(value) / 100.f;
				m_audioOutput->setVolume(volume);
				if (volume > 0.66f)
				{
					ui.pushButton_mutexVideo->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::AudioVolumeHigh));
				}
				else if (volume > 0.33f)
				{
					ui.pushButton_mutexVideo->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::AudioVolumeMedium));
				}
				else
				{
					ui.pushButton_mutexVideo->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::AudioVolumeLow));
				}
			}
		);
		connect(ui.pushButton_mutexVideo, &QPushButton::clicked, 
			[this]() 
			{ 
				float v = m_audioOutput->volume();
				if (v == 0.f) 
				{
					ui.horizontalSlider_volumeVideo->setEnabled(true);
					float volume = static_cast<float>(ui.horizontalSlider_volumeVideo->value()) / 100.f;
					m_audioOutput->setVolume(volume);
					if (volume > 0.66f)
					{
						ui.pushButton_mutexVideo->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::AudioVolumeHigh));
					}
					else if (volume > 0.33f)
					{
						ui.pushButton_mutexVideo->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::AudioVolumeMedium));
					}
					else
					{
						ui.pushButton_mutexVideo->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::AudioVolumeLow));
					}
				}
				else
				{
					ui.horizontalSlider_volumeVideo->setDisabled(true);
					m_audioOutput->setVolume(0.f);
					ui.pushButton_mutexVideo->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::AudioVolumeMuted));
				}
			});
		// progress
		maxRangeValue = ui.horizontalSlider_progressVideo->maximum();
		connect(ui.horizontalSlider_progressVideo, &QSlider::sliderPressed,
			[this, maxRangeValue]()
			{
				m_progressPressed = true;
			}
		);
		connect(ui.horizontalSlider_progressVideo, &QSlider::sliderReleased, 
			[this, maxRangeValue]()
			{
				qint64 pos = static_cast<qint64>(
					static_cast<double>(ui.horizontalSlider_progressVideo->value() * m_videoDuration) / static_cast<double>(maxRangeValue));
				m_mediaPlayer->setPosition(pos);

				auto timeStr = convertToTimeString(pos, m_videoDuration);
				ui.label_timeVideo->setText(QString::fromStdString(timeStr));

				m_progressPressed = false;
			}
		);
		connect(m_mediaPlayer, &QMediaPlayer::positionChanged, 
			[this, maxRangeValue](qint64 pos)
			{
				if (!m_progressPressed)
				{
					// update progress
					int v = pos * maxRangeValue / m_videoDuration;
					ui.horizontalSlider_progressVideo->setValue(v);
				}
				// update play time
				auto timeStr = convertToTimeString(pos, m_videoDuration);
				ui.label_timeVideo->setText(QString::fromStdString(timeStr));
			}
		);
		connect(m_mediaPlayer, &QMediaPlayer::durationChanged, this, &DialogMediaPlayer::onVideoDurationChanged);

		auto& ticker = Ticker::instance();
		connect(&ticker, &Ticker::updateClock, this, &DialogMediaPlayer::updateDisplay);
		connect(&ticker, &Ticker::showNotification, this, &DialogMediaPlayer::showNotification);
		connect(&ticker, &Ticker::showOutOfDateNotification, this, &DialogMediaPlayer::showOutOfDateNotification);

		connect(ui.pushButton_close, &QPushButton::clicked, this, &DialogMediaPlayer::onOK);


	}

	DialogMediaPlayer::~DialogMediaPlayer()
	{
	}

	void DialogMediaPlayer::display()
	{
		m_progressPressed = false;
		ui.stackedWidget->setCurrentIndex(static_cast<int>(m_currentSch.m_displayEffect));
		ui.label_event->setText(QString::fromStdString(m_currentSch.m_event));
		switch (m_currentSch.m_displayEffect)
		{
		case DisplayEffect::Text:
			ui.label_description->setText(QString::fromStdString(m_currentSch.m_description));
			break;
		case DisplayEffect::Image:
			ui.page_Image->setImage(QString::fromStdString(m_currentSch.m_mediaURL));
			break;
		case DisplayEffect::Audio:
			m_mediaPlayer->setSource(QUrl(QString::fromStdString(m_currentSch.m_mediaURL)));
			ui.pushButton_playAudio->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::MediaPlaybackPause));
			m_mediaPlayer->play();
			break;
		case DisplayEffect::Video:
			m_mediaPlayer->setSource(QUrl(QString::fromStdString(m_currentSch.m_mediaURL)));
			ui.pushButton_playVideo->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::MediaPlaybackPause));
			m_mediaPlayer->play();
			break;
		case DisplayEffect::WebSite:
#ifdef WIN32
			ShellExecute(NULL, L"open", QString::fromStdString(m_currentSch.m_mediaURL).toStdWString().c_str(), NULL, NULL, SW_SHOWNORMAL);
#endif
			break;
		default:
			break;
		}
		show();
		m_showStartTime = std::chrono::system_clock::now();
	}

	std::string DialogMediaPlayer::convertToTimeString(qint64 position, qint64 duration)
	{
		auto totalSeconds = position / 1000;
		auto hours = totalSeconds / 3600;
		auto remainingSeconds = totalSeconds % 3600;
		auto minutes = remainingSeconds / 60;
		auto seconds = remainingSeconds % 60;

		std::ostringstream oss;
		oss << std::setfill('0') << std::setw(2) << hours << ":"
			<< std::setfill('0') << std::setw(2) << minutes << ":"
			<< std::setfill('0') << std::setw(2) << seconds << '/';

		totalSeconds = duration / 1000;
		hours = totalSeconds / 3600;
		remainingSeconds = totalSeconds % 3600;
		minutes = remainingSeconds / 60;
		seconds = remainingSeconds % 60;

		oss << std::setfill('0') << std::setw(2) << hours << ":"
			<< std::setfill('0') << std::setw(2) << minutes << ":"
			<< std::setfill('0') << std::setw(2) << seconds;

		return oss.str();
	}

	void DialogMediaPlayer::updateDisplay()
	{
		if (isHidden())
		{
			if (!m_outOfDateQueue.empty())
			{
				m_currentSch = m_outOfDateQueue.front();
				m_outOfDateQueue.pop();
				display();
			}
			else if (!m_commonQueue.empty())
			{
				m_currentSch = m_commonQueue.front();
				m_commonQueue.pop();
				display();
			}
		}
		else
		{
			if (m_currentSch.m_displayEffect == DisplayEffect::Video) 
			{
				int v = ui.horizontalSlider_progressVideo->value();
				if (v >= ui.horizontalSlider_progressVideo->maximum())
				{
					m_mediaPlayer->stop();
					hide();
					ui.progressBar->setValue(0);
				}
				else
				{
					ui.progressBar->setValue(v / 10);
				}
			}
			else if (m_currentSch.m_displayEffect == DisplayEffect::Audio)
			{
				int v = ui.horizontalSlider_progressAudio->value();
				if (v >= ui.horizontalSlider_progressAudio->maximum())
				{
					m_mediaPlayer->stop();
					hide();
					ui.progressBar->setValue(0);
				}
				else
				{
					ui.progressBar->setValue(v / 10);
				}
			}
			else
			{
				const int maxRangeValue = ui.progressBar->maximum();
				auto currentTime = std::chrono::system_clock::now();
				auto milliSec = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - m_showStartTime).count();
				int v = static_cast<int>(0.002 * milliSec);
				if (v >= maxRangeValue)
				{
					hide();
					ui.progressBar->setValue(0);
				}
				else
				{
					ui.progressBar->setValue(v);
				}
			}
		}
	}

	void DialogMediaPlayer::onVideoDurationChanged(qint64 duration)
	{
		m_videoDuration = duration;
		auto str = convertToTimeString(m_mediaPlayer->position(), m_videoDuration);
		ui.label_timeVideo->setText(QString::fromStdString(str));
	}

	void DialogMediaPlayer::onAudioDurationChanged(qint64 duration)
	{
		m_audioDuration = duration;
		auto str = convertToTimeString(m_mediaPlayer->position(), m_audioDuration);
		ui.label_timeAudio->setText(QString::fromStdString(str));
	}

	void DialogMediaPlayer::onOK()
	{
		switch (m_currentSch.m_displayEffect)
		{
		case DisplayEffect::Text:
			break;
		case DisplayEffect::Image:
			break;
		case DisplayEffect::Audio:
			m_mediaPlayer->stop();
			break;
		case DisplayEffect::Video:
			m_mediaPlayer->stop();
			break;
		case DisplayEffect::WebSite:
			//m_webView->stop();
			break;
		default:
			break;
		}
		hide();
	}

	void DialogMediaPlayer::showOutOfDateNotification(std::queue<Schedule>& schedules)
	{
		while (!schedules.empty())
		{
			Schedule sch = schedules.front();
			schedules.pop();
			m_outOfDateQueue.push(sch);
		}
	}

	void DialogMediaPlayer::showNotification(std::queue<Schedule>& schedules)
	{
		while (!schedules.empty())
		{
			Schedule sch = schedules.front();
			schedules.pop();
			m_commonQueue.push(sch);
		}
	}

}