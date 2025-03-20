#include "ui/DialogTask.h"
#include "Ticker.h"
#include <QFileDialog>
#include <QMessageBox>

namespace reminder
{
	DialogTask::DialogTask(QWidget* parent)
		: QDialog(parent)
	{
		ui.setupUi(this);
		ui.dateEdit->setDate(QDate::currentDate());

		connect(ui.pushButton_ok, &QPushButton::clicked, this, &DialogTask::onOk);
		connect(ui.pushButton_cancel, &QPushButton::clicked, this, &DialogTask::onCancel);
		connect(ui.pushButton_selectFile, &QPushButton::clicked, this, &DialogTask::onSelectedFile);

		connect(ui.radioButton_eventTypeOnce, &QRadioButton::clicked, this, &DialogTask::onEventTypeChanged);
		connect(ui.radioButton_eventTypeLoop, &QRadioButton::clicked, this, &DialogTask::onEventTypeChanged);

		connect(ui.comboBox_loopType, &QComboBox::currentIndexChanged, this, &DialogTask::onLoopTypeChanged);

		connect(ui.comboBox_weekday, &QComboBox::currentIndexChanged, this, &DialogTask::onWeekdayChanged);

		connect(ui.comboBox_displayType, &QComboBox::currentIndexChanged, 
			[this]()
			{
				if (needCheck)
				{
					ui.lineEdit_mediaFile->setText("");
					if (ui.comboBox_displayType->currentIndex() == static_cast<int>(DisplayEffect::WebSite))
					{
						ui.lineEdit_mediaFile->setEnabled(true);
					}
					else
					{
						ui.lineEdit_mediaFile->setEnabled(false);
					}
				}
			}
		);
	}

	DialogTask::~DialogTask()
	{
	}

	int DialogTask::editSchedule(const Schedule& sch)
	{
		setWindowTitle(tr("修改待办"));
		m_schedule = sch;
		setDialogValue();
		return exec();
	}

	int DialogTask::addSchedule()
	{
		setWindowTitle(tr("添加待办"));
		ui.timeEdit->setTime(QTime::currentTime());
		return exec();
	}

	void DialogTask::setDialogValue()
	{
		needCheck = false;

		if (m_schedule.m_scheduleType == ScheduleType::Once)
		{
			ui.radioButton_eventTypeOnce->setChecked(true);
			ui.comboBox_loopType->setCurrentIndex(5);
			ui.comboBox_loopType->setEnabled(false);

			ui.dateEdit->setDate(QDate(m_schedule.m_timePoint.year, 
				m_schedule.m_timePoint.month, m_schedule.m_timePoint.day));

			ui.comboBox_weekday->setCurrentIndex(7);
			ui.comboBox_weekday->setEnabled(false);

			if (m_schedule.m_timePoint.hour != ScheduleTime::DoNotCheckHour)
			{
				ui.radioButton_matchTimeYes->setChecked(true);
				ui.timeEdit->setTime(QTime(m_schedule.m_timePoint.hour, m_schedule.m_timePoint.minute, 0, 0));
			}
			else
			{
				ui.radioButton_matchTimeNo->setChecked(true);
				ui.timeEdit->setEnabled(false);
			}
			ui.lineEdit_event->setText(QString::fromStdString(m_schedule.m_event));
			ui.plainTextEdit_eventDescription->setPlainText(QString::fromStdString(m_schedule.m_description));
			ui.comboBox_displayType->setCurrentIndex(static_cast<int>(m_schedule.m_displayEffect));
			ui.lineEdit_mediaFile->setText(QString::fromStdString(m_schedule.m_mediaURL));
		}
		else
		{
			ui.radioButton_eventTypeLoop->setChecked(true);
			ui.comboBox_loopType->setCurrentIndex(static_cast<int>(m_schedule.m_loopType));
			ui.comboBox_loopType->setEnabled(true);

			ui.dateEdit->setDate(QDate(m_schedule.m_timePoint.year,
				m_schedule.m_timePoint.month, m_schedule.m_timePoint.day));

			if (m_schedule.m_loopType == LoopType::EveryWeek)
			{
				ui.comboBox_weekday->setCurrentIndex(static_cast<int>(m_schedule.m_timePoint.weekday));
				ui.comboBox_weekday->setEnabled(true);
			}
			else
			{
				ui.comboBox_weekday->setCurrentIndex(7);
				ui.comboBox_weekday->setEnabled(false);
			}

			if (m_schedule.m_timePoint.hour != ScheduleTime::DoNotCheckHour)
			{
				ui.radioButton_matchTimeYes->setChecked(true);
				ui.timeEdit->setTime(QTime(m_schedule.m_timePoint.hour, m_schedule.m_timePoint.minute, 0, 0));
			}
			else
			{
				ui.radioButton_matchTimeNo->setChecked(true);
				ui.timeEdit->setEnabled(false);
			}
			ui.lineEdit_event->setText(QString::fromStdString(m_schedule.m_event));
			ui.plainTextEdit_eventDescription->setPlainText(QString::fromStdString(m_schedule.m_description));
			ui.comboBox_displayType->setCurrentIndex(static_cast<int>(m_schedule.m_displayEffect));
			ui.lineEdit_mediaFile->setText(QString::fromStdString(m_schedule.m_mediaURL));
		}

		if (ui.comboBox_displayType->currentIndex() == static_cast<int>(DisplayEffect::WebSite))
		{
			ui.lineEdit_mediaFile->setEnabled(true);
		}
		else
		{
			ui.lineEdit_mediaFile->setEnabled(false);
		}

		needCheck = true;
	}

	void DialogTask::onCancel()
	{
		reject();
	}

	void DialogTask::onEventTypeChanged()
	{
		if (!needCheck) return;
		if (ui.radioButton_eventTypeOnce->isChecked())
		{
			//schType = ScheduleType::Once;
			ui.comboBox_loopType->setCurrentIndex(5);
			ui.comboBox_loopType->setEnabled(false);
		}
		else
		{
			//schType = ScheduleType::Loop;
			ui.comboBox_loopType->setCurrentIndex(0);
			ui.comboBox_loopType->setEnabled(true);
		}
	}

	void DialogTask::onLoopTypeChanged()
	{
		if (!needCheck) return;
		if (ui.radioButton_eventTypeLoop->isChecked())
		{
			int cId = ui.comboBox_loopType->currentIndex();
			if (!ui.groupBox_matchTime->isEnabled())
			{
				ui.groupBox_matchTime->setEnabled(true);
			}
			if (cId == static_cast<int>(LoopType::None))
			{
				ui.comboBox_weekday->setEnabled(false);
				ui.comboBox_weekday->setCurrentIndex(7);

				ui.comboBox_loopType->setCurrentIndex(4);
				QMessageBox::warning(this, "警告", "循环事件必须确定循环周期");
			}
			else if (cId == static_cast<int>(LoopType::EveryHour))
			{
				ui.radioButton_matchTimeYes->setChecked(true);
				ui.groupBox_matchTime->setDisabled(true);
			}
			else if (cId == static_cast<int>(LoopType::EveryWeek))
			{
				ui.comboBox_weekday->setEnabled(true);
				ui.comboBox_weekday->setCurrentIndex(0);
			}
			else
			{
				ui.comboBox_weekday->setCurrentIndex(7);
				ui.comboBox_weekday->setEnabled(false);
			}
		}
		else
		{
			if (ui.comboBox_weekday->isEnabled())
			{
				ui.comboBox_weekday->setEnabled(false);
				ui.comboBox_weekday->setCurrentIndex(7);
			}
			if (!ui.groupBox_matchTime->isEnabled())
			{
				ui.groupBox_matchTime->setEnabled(true);
			}
		}
	}

	void DialogTask::onWeekdayChanged()
	{
		if (needCheck && ui.radioButton_eventTypeLoop->isChecked())
		{
			if (ui.comboBox_loopType->currentIndex() == 2 && ui.comboBox_weekday->currentIndex() == 7)
			{
				ui.comboBox_weekday->setCurrentIndex(6);
				QMessageBox::warning(this, "警告", "周循环事件必须确定星期");
			}
		}
	}

	void DialogTask::onSelectedFile()
	{
		const auto displayType = static_cast<DisplayEffect>(ui.comboBox_displayType->currentIndex());
		switch (displayType)
		{
		case DisplayEffect::Image:
		{
			QString file = QFileDialog::getOpenFileName(this, "选择图像文件", "", "图像文件 (*.png *.jpg *.bmp *.jpeg)");
			if (!file.isEmpty())
			{
				ui.lineEdit_mediaFile->setText(file);
			}
		}
			break;
		case DisplayEffect::Audio:
		{
			QString file = QFileDialog::getOpenFileName(this, "选择音频文件", "", "音频文件 (*.mp3 *.wav)");
			if (!file.isEmpty())
			{
				ui.lineEdit_mediaFile->setText(file);
			}
		}
			break;
		case DisplayEffect::Video:
		{
			QString file = QFileDialog::getOpenFileName(this, "选择视频文件", "", "视频文件 (*.mp4 *.mkv)");
			if (!file.isEmpty())
			{
				ui.lineEdit_mediaFile->setText(file);
			}
		}
			break;
		case DisplayEffect::WebSite:
			break;
		default:
			break;
		}
	}

	void DialogTask::onOk()
	{
		Schedule& task = m_schedule;
		task.m_description = ui.plainTextEdit_eventDescription->toPlainText().toStdString();
		task.m_event = ui.lineEdit_event->text().toStdString();

		auto date = ui.dateEdit->date();
		task.m_timePoint.year = date.year();
		task.m_timePoint.month = date.month();
		const auto loopType = static_cast<LoopType>(ui.comboBox_loopType->currentIndex());
		if (loopType == LoopType::EveryWeek)
		{
			task.m_timePoint.day = ScheduleTime::DoNotCheckDay;
			task.m_timePoint.weekday = static_cast<Weekdays>(ui.comboBox_weekday->currentIndex());
		}
		else
		{
			task.m_timePoint.day = date.day();
		}
		if (ui.radioButton_matchTimeYes->isChecked())
		{
			auto time = ui.timeEdit->time();
			task.m_timePoint.hour = time.hour();
			task.m_timePoint.minute = time.minute();
		}
		else
		{
			task.m_timePoint.hour = ScheduleTime::DoNotCheckHour;
		}
		bool eventOnce = ui.radioButton_eventTypeOnce->isChecked();
		task.m_scheduleType = eventOnce ? ScheduleType::Once : ScheduleType::Loop;
		if (!eventOnce)
		{
			task.m_loopType = loopType;
		}

		task.m_displayEffect = static_cast<DisplayEffect>(ui.comboBox_displayType->currentIndex());
		if (task.m_displayEffect != DisplayEffect::Text)
		{
			QString file = ui.lineEdit_mediaFile->text();
			if (file.isEmpty())
			{
				QMessageBox::warning(this, "错误", "请选择对应的媒体文件");
				return;
			}
			else
			{
				task.m_mediaURL = file.toStdString();
			}
		}
		
		accept();
	}
}

