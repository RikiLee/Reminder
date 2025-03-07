#include "ui/NotifyWidget.h"
#include <iomanip>
#include <sstream>
#include <QStyle>
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

		/// audio player
		m_audioPlayer = new QMediaPlayer(this);
		m_audioAudioOutput = new QAudioOutput(this);
		m_audioPlayer->setAudioOutput(m_audioAudioOutput);
		m_audioAudioOutput->setVolume(1.f);
		// play or pause
		connect(ui.pushButton_playAudio, &QPushButton::clicked,
			[this]()
			{
				if (m_audioPlayer->isPlaying())
				{
					ui.pushButton_playAudio->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::MediaPlaybackStart));
					m_audioPlayer->pause();
				}
				else
				{
					ui.pushButton_playAudio->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::MediaPlaybackPause));
					m_audioPlayer->play();
				}
			}
		);
		// volume
		connect(ui.horizontalSlider_volumeAudio, &QSlider::sliderMoved, 
			[this](int value) 
			{ 
				float volume = static_cast<float>(value) / 100.f;
				m_audioAudioOutput->setVolume(volume);
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
			[this](int value)
			{
				float v = m_audioAudioOutput->volume();
				if (v == 0.f)
				{
					ui.horizontalSlider_volumeAudio->setEnabled(true);
					float volume = static_cast<float>(ui.horizontalSlider_volumeAudio->value()) / 100.f;
					m_audioAudioOutput->setVolume(volume);
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
					m_audioAudioOutput->setVolume(0.f);
					ui.pushButton_mutexAudio->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::AudioVolumeMuted));
				}
			});
		// progress
		auto maxRangeValue = ui.horizontalSlider_progressAudio->maximum();
		connect(ui.horizontalSlider_progressAudio, &QSlider::sliderMoved,
			[this, maxRangeValue](int value)
			{
				auto pos = m_audioDuration / maxRangeValue * value;
				m_audioPlayer->setPosition(pos);

				auto timeStr = convertToTimeString(pos, m_audioDuration);
				ui.label_timeAudio->setText(QString::fromStdString(timeStr));
			}
		);
		connect(m_audioPlayer, &QMediaPlayer::positionChanged,
			[this, maxRangeValue](qint64 pos)
			{
				// update progress
				int v = pos * maxRangeValue / m_audioDuration;
				ui.horizontalSlider_progressAudio->setValue(v);
				// update play time
				auto timeStr = convertToTimeString(pos, m_audioDuration);
				ui.label_timeAudio->setText(QString::fromStdString(timeStr));
			}
		);
		connect(m_audioPlayer, &QMediaPlayer::durationChanged, this, &DialogMediaPlayer::onAudioDurationChanged);

		/// video player
		m_videoPlayer = new QMediaPlayer(this);
		m_videoPlayer->setVideoOutput(ui.widget_Video);
		m_videoAudioOutput = new QAudioOutput(this);
		m_videoPlayer->setAudioOutput(m_videoAudioOutput);
		m_videoAudioOutput->setVolume(1.f);
		// play or pause
		connect(ui.pushButton_playVideo, &QPushButton::clicked, 
			[this]()
			{
				if (m_videoPlayer->isPlaying())
				{
					ui.pushButton_playVideo->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::MediaPlaybackStart));
					m_videoPlayer->pause();
				}
				else
				{
					ui.pushButton_playVideo->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::MediaPlaybackPause));
					m_videoPlayer->play();
				}
			}
		);
		// volume
		connect(ui.horizontalSlider_volumeVideo, &QSlider::sliderMoved, 
			[this](int value) 
			{ 
				float volume = static_cast<float>(value) / 100.f;
				m_videoAudioOutput->setVolume(volume);
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
			[this](int value) 
			{ 
				float v = m_videoAudioOutput->volume();
				if (v == 0.f) 
				{
					ui.horizontalSlider_volumeVideo->setEnabled(true);
					float volume = static_cast<float>(ui.horizontalSlider_volumeVideo->value()) / 100.f;
					m_videoAudioOutput->setVolume(volume);
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
					m_videoAudioOutput->setVolume(0.f);
					ui.pushButton_mutexVideo->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::AudioVolumeMuted));
				}
			});
		// progress
		maxRangeValue = ui.horizontalSlider_progressVideo->maximum();
		connect(ui.horizontalSlider_progressVideo, &QSlider::sliderMoved, 
			[this, maxRangeValue](int value) 
			{
				auto pos = m_videoDuration / maxRangeValue * value;
				m_videoPlayer->setPosition(pos);

				auto timeStr = convertToTimeString(pos, m_videoDuration);
				ui.label_timeVideo->setText(QString::fromStdString(timeStr));
			}
		);
		connect(m_videoPlayer, &QMediaPlayer::positionChanged, 
			[this, maxRangeValue](qint64 pos)
			{
				// update progress
				int v = pos * maxRangeValue / m_videoDuration;
				ui.horizontalSlider_progressVideo->setValue(v);
				// update play time
				auto timeStr = convertToTimeString(pos, m_videoDuration);
				ui.label_timeVideo->setText(QString::fromStdString(timeStr));
			}
		);
		connect(m_videoPlayer, &QMediaPlayer::durationChanged, this, &DialogMediaPlayer::onVideoDurationChanged);

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
			m_audioPlayer->setSource(QUrl(QString::fromStdString(m_currentSch.m_mediaURL)));
			ui.pushButton_playAudio->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::MediaPlaybackPause));
			m_audioPlayer->play();
			break;
		case DisplayEffect::Video:
			m_videoPlayer->setSource(QUrl(QString::fromStdString(m_currentSch.m_mediaURL)));
			ui.pushButton_playVideo->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::MediaPlaybackPause));
			m_videoPlayer->play();
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
		auto str = convertToTimeString(m_videoPlayer->position(), m_videoDuration);
		ui.label_timeVideo->setText(QString::fromStdString(str));
	}

	void DialogMediaPlayer::onAudioDurationChanged(qint64 duration)
	{
		m_audioDuration = duration;
		auto str = convertToTimeString(m_audioPlayer->position(), m_audioDuration);
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
			m_audioPlayer->stop();
			break;
		case DisplayEffect::Video:
			m_videoPlayer->stop();
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