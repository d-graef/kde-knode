/*
 *  datetime.cpp  -  time spinbox, and alarm time entry widget
 *  Program:  kalarm
 *  (C) 2001, 2002 by David Jarvie  software@astrojar.org.uk
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  As a special exception, permission is given to link this program
 *  with any edition of Qt, and distribute the resulting executable,
 *  without including the source code for Qt in the source distribution.
 */

#include "kalarm.h"

#include <qlayout.h>
#include <qgroupbox.h>
#include <qhbox.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qvalidator.h>
#include <qwhatsthis.h>

#include <kdialog.h>
#include <kmessagebox.h>
#include <klocale.h>

#include "dateedit.h"
#include "datetime.moc"


/******************************************************************************
*  Construct a widget with a group box and title.
*/
AlarmTimeWidget::AlarmTimeWidget(const QString& groupBoxTitle, int mode, QWidget* parent, const char* name)
	: ButtonGroup(groupBoxTitle, parent, name)
{
	init(mode);
}

/******************************************************************************
*  Construct a widget without a group box or title.
*/
AlarmTimeWidget::AlarmTimeWidget(int mode, QWidget* parent, const char* name)
	: ButtonGroup(parent, name)
{
	setFrameStyle(QFrame::NoFrame);
	init(mode);
}

void AlarmTimeWidget::init(int mode)
{
	connect(this, SIGNAL(buttonSet(int)), SLOT(slotButtonSet(int)));
	QBoxLayout* topLayout = new QVBoxLayout(this, 0, KDialog::spacingHint());
	if (!title().isEmpty())
		topLayout->setMargin(marginKDE2 + KDialog::marginHint());
	topLayout->addSpacing(fontMetrics().lineSpacing()/2);

	// At time radio button/label
	atTimeRadio = new QRadioButton(((mode & DEFER_TIME) ? i18n("&Defer to date/time:") : i18n("At &date/time:")), this, "atTimeRadio");
	atTimeRadio->setFixedSize(atTimeRadio->sizeHint());
	QWhatsThis::add(atTimeRadio,
	                ((mode & DEFER_TIME) ? i18n("Reschedule the alarm to the specified date and time.")
	                                     : i18n("Schedule the alarm at the specified date and time.")));

	// Date edit box
	dateEdit = new DateEdit(this);
	dateEdit->setFixedSize(dateEdit->sizeHint());
	QWhatsThis::add(dateEdit, i18n("Enter the date to schedule the alarm."));
	connect(dateEdit, SIGNAL(dateChanged(QDate)), this, SLOT(slotDateChanged(QDate)));

	// Time edit box and Any time checkbox
	QHBox* timeBox = new QHBox(this);
	timeBox->setSpacing(2*KDialog::spacingHint());
	timeEdit = new TimeSpinBox(timeBox);
	timeEdit->setValue(2399);
	timeEdit->setFixedSize(timeEdit->sizeHint());
	QWhatsThis::add(timeEdit, i18n("Enter the time to schedule the alarm."));
	connect(timeEdit, SIGNAL(valueChanged(int)), this, SLOT(slotTimeChanged(int)));

	if (mode & DEFER_TIME)
	{
		anyTimeAllowed = false;
		anyTimeCheckBox = 0;
	}
	else
	{
		anyTimeAllowed = true;
		anyTimeCheckBox = new QCheckBox(i18n("An&y time"), timeBox);
		anyTimeCheckBox->setFixedSize(anyTimeCheckBox->sizeHint());
		QWhatsThis::add(anyTimeCheckBox, i18n("Schedule the alarm for any time during the day"));
		connect(anyTimeCheckBox, SIGNAL(toggled(bool)), this, SLOT(anyTimeToggled(bool)));
	}

	// 'Time from now' radio button/label
	afterTimeRadio = new QRadioButton(((mode & DEFER_TIME) ? i18n("Defer for time &interval:") : i18n("Time from no&w:")),
	                                  this, "afterTimeRadio");
	afterTimeRadio->setFixedSize(afterTimeRadio->sizeHint());
	QWhatsThis::add(afterTimeRadio,
	                ((mode & DEFER_TIME) ? i18n("Reschedule the alarm for the specified time interval after now.")
	                                     : i18n("Schedule the alarm after the specified time interval from now.")));

	// Delay time spin box
	delayTime = new TimeSpinBox(1, 99*60+59, this);
	delayTime->setValue(2399);
	delayTime->setFixedSize(delayTime->sizeHint());
	QWhatsThis::add(delayTime,
	      i18n("Enter the length of time (in hours and minutes) after the current time to schedule the alarm."));
	connect(delayTime, SIGNAL(valueChanged(int)), this, SLOT(delayTimeChanged(int)));

	// Set up the layout, either narrow or wide
	if (mode & NARROW)
	{
		QGridLayout* grid = new QGridLayout(topLayout, 2, 2, KDialog::spacingHint());
		grid->addWidget(atTimeRadio, 0, 0);
		grid->addWidget(dateEdit, 0, 1, Qt::AlignLeft);
		grid->addWidget(timeBox, 1, 1, Qt::AlignLeft);
		grid->setColStretch(2, 1);
		topLayout->addStretch();
		QBoxLayout* layout = new QHBoxLayout(topLayout, KDialog::spacingHint());
		layout->addWidget(afterTimeRadio);
		layout->addWidget(delayTime);
		layout->addStretch();
	}
	else
	{
		QGridLayout* grid = new QGridLayout(topLayout, 2, 3, KDialog::spacingHint());
		grid->addWidget(atTimeRadio, 0, 0, Qt::AlignLeft);
		grid->addWidget(dateEdit, 0, 1, Qt::AlignLeft);
		grid->addWidget(timeBox, 0, 2, Qt::AlignLeft);
		grid->setRowStretch(0, 1);
		grid->addWidget(afterTimeRadio, 1, 0, Qt::AlignLeft);
		grid->addWidget(delayTime, 1, 1, Qt::AlignLeft);
		grid->setColStretch(3, 1);
		topLayout->addStretch();
	}

	// Initialise the radio button statuses
	setButton(id(atTimeRadio));

	// Timeout every minute to update alarm time fields.
	// But first synchronise to one second after the minute boundary.
	int firstInterval = 61 - QTime::currentTime().second();
	timer.start(1000 * firstInterval);
	timerSyncing = (firstInterval != 60);
	connect(&timer, SIGNAL(timeout()), this, SLOT(slotTimer()));
}

/******************************************************************************
*  Fetch the entered date/time.
*  If <= current time, and 'showErrorMessage' is true, output an error message.
*  Output: 'anyTime' is set true if no time was entered.
*  Reply = widget with the invalid value, if it is after the current time.
*/
QWidget* AlarmTimeWidget::getDateTime(QDateTime& dateTime, bool& anyTime, bool showErrorMessage) const
{
	QDateTime now = QDateTime::currentDateTime();
	if (atTimeRadio->isOn())
	{
		dateTime.setDate(dateEdit->date());
		anyTime = anyTimeAllowed && anyTimeCheckBox && anyTimeCheckBox->isChecked();
		if (anyTime)
		{
			dateTime.setTime(QTime());
			if (dateTime.date() < now.date())
			{
				if (showErrorMessage)
					KMessageBox::sorry(const_cast<AlarmTimeWidget*>(this), i18n("Alarm date has already expired"));
				return dateEdit;
			}
		}
		else
		{
			dateTime.setTime(timeEdit->time());
			int seconds = now.time().second();
			if (dateTime <= now.addSecs(1 - seconds))
			{
				if (showErrorMessage)
					KMessageBox::sorry(const_cast<AlarmTimeWidget*>(this), i18n("Alarm time has already expired"));
				return timeEdit;
			}
		}
	}
	else
	{
		dateTime = now.addSecs(delayTime->value() * 60);
		dateTime = dateTime.addSecs(-dateTime.time().second());
		anyTime = false;
	}
	return 0;
}

/******************************************************************************
*  Set the date/time.
*/
void AlarmTimeWidget::setDateTime(const QDateTime& dt, bool anyTime)
{
	timeEdit->setValue(dt.time().hour()*60 + dt.time().minute());
	dateEdit->setDate(dt.date());
	dateTimeChanged();     // update the delay time edit box
	QDate now = QDate::currentDate();
	dateEdit->setMinDate(dt.date() < now ? dt.date() : now);
	if (anyTimeCheckBox)
	{
		anyTimeAllowed = anyTime;
		anyTimeCheckBox->setChecked(anyTime);
	}
}

/******************************************************************************
*  Enable/disable the "any time" checkbox.
*/
void AlarmTimeWidget::enableAnyTime(bool enable)
{
	if (anyTimeCheckBox)
	{
		anyTimeAllowed = enable;
		bool at = atTimeRadio->isOn();
		anyTimeCheckBox->setEnabled(enable && at);
		if (at)
			timeEdit->setEnabled(!enable || !anyTimeCheckBox->isChecked());
	}
}

/******************************************************************************
*  Called every minute to update the alarm time data entry fields.
*/
void AlarmTimeWidget::slotTimer()
{
	if (timerSyncing)
	{
		// We've synced to the minute boundary. Now set timer to 1 minute intervals.
		timer.changeInterval(1000 * 60);
		timerSyncing = false;
	}
	if (atTimeRadio->isOn())
		dateTimeChanged();
	else
		delayTimeChanged(delayTime->value());
}


/******************************************************************************
*  Called when the At or After time radio button states have been set.
*  Updates the appropriate edit box.
*/
void AlarmTimeWidget::slotButtonSet(int)
{
	bool at = atTimeRadio->isOn();
	dateEdit->setEnabled(at);
	timeEdit->setEnabled(at && (!anyTimeAllowed || !anyTimeCheckBox || !anyTimeCheckBox->isChecked()));
	if (anyTimeCheckBox)
		anyTimeCheckBox->setEnabled(at && anyTimeAllowed);
	// Ensure that the value of the delay edit box is > 0.
	QDateTime dt(dateEdit->date(), timeEdit->time());
	int minutes = (QDateTime::currentDateTime().secsTo(dt) + 59) / 60;
	if (minutes <= 0)
		delayTime->setValid(true);
	delayTime->setEnabled(!at);
//setFocusProxy(at ? dateEdit : delayTime);
}

/******************************************************************************
*  Called after the anyTimeCheckBox checkbox has been toggled.
*/
void AlarmTimeWidget::anyTimeToggled(bool on)
{
	timeEdit->setEnabled((!anyTimeAllowed || !on) && atTimeRadio->isOn());
}

/******************************************************************************
*  Called when the date or time edit box values have changed.
*  Updates the time delay edit box accordingly.
*/
void AlarmTimeWidget::dateTimeChanged()
{
	QDateTime dt(dateEdit->date(), timeEdit->time());
	int minutes = (QDateTime::currentDateTime().secsTo(dt) + 59) / 60;
	bool blocked = delayTime->signalsBlocked();
	delayTime->blockSignals(true);     // prevent infinite recursion between here and delayTimeChanged()
	if (minutes <= 0  ||  minutes > delayTime->maxValue())
		delayTime->setValid(false);
	else
		delayTime->setValue(minutes);
	delayTime->blockSignals(blocked);
}

/******************************************************************************
*  Called when the delay time edit box value has changed.
*  Updates the Date and Time edit boxes accordingly.
*/
void AlarmTimeWidget::delayTimeChanged(int minutes)
{
	if (delayTime->valid())
	{
		QDateTime dt = QDateTime::currentDateTime().addSecs(minutes * 60);
		bool blockedT = timeEdit->signalsBlocked();
		bool blockedD = dateEdit->signalsBlocked();
		timeEdit->blockSignals(true);     // prevent infinite recursion between here and dateTimeChanged()
		timeEdit->blockSignals(true);
		timeEdit->setValue(dt.time().hour()*60 + dt.time().minute());
		dateEdit->setDate(dt.date());
		timeEdit->blockSignals(blockedT);
		dateEdit->blockSignals(blockedD);
	}
}


/*=============================================================================
=  class TimeSpinBox
=============================================================================*/
class TimeSpinBox::TimeValidator : public QValidator
{
	public:
		TimeValidator(int minMin, int maxMin, QWidget* parent, const char* name = 0L)
			: QValidator(parent, name), minMinute(minMin), maxMinute(maxMin) { }
		virtual State validate(QString&, int&) const;
		int  minMinute, maxMinute;
};


// Construct a wrapping 00:00 - 23:59 time spin box
TimeSpinBox::TimeSpinBox(QWidget* parent, const char* name)
	: SpinBox2(0, 1439, 1, 60, parent, name),
	  minimumValue(0),
	  invalid(false),
	  enteredSetValue(false)
{
	validator = new TimeValidator(0, 1439, this, "TimeSpinBox validator");
	setValidator(validator);
	setWrapping(true);
	setShiftSteps(5, 360);    // shift-left button increments 5 min / 6 hours
}

// Construct a non-wrapping time spin box
TimeSpinBox::TimeSpinBox(int minMinute, int maxMinute, QWidget* parent, const char* name)
	: SpinBox2(minMinute, maxMinute, 1, 60, parent, name),
	  minimumValue(minMinute),
	  invalid(false),
	  enteredSetValue(false)
{
	validator = new TimeValidator(minMinute, maxMinute, this, "TimeSpinBox validator");
	setValidator(validator);
	setShiftSteps(5, 360);    // shift-left button increments 5 min / 6 hours
}

QTime TimeSpinBox::time() const
{
	return QTime(value() / 60, value() % 60);
}

QString TimeSpinBox::mapValueToText(int v)
{
	QString s;
	s.sprintf("%02d:%02d", v/60, v%60);
	return s;
}

/******************************************************************************
 * Convert the user-entered text to a value in minutes.
 * The allowed format is [hour]:[minute], where hour and
 * minute must be non-blank.
 */
int TimeSpinBox::mapTextToValue(bool* ok)
{
	QString text = cleanText();
	int colon = text.find(':');
	if (colon >= 0)
	{
		QString hour   = text.left(colon).stripWhiteSpace();
		QString minute = text.mid(colon + 1).stripWhiteSpace();
		if (!hour.isEmpty()  &&  !minute.isEmpty())
		{
			bool okhour, okmin;
			int m = minute.toUInt(&okmin);
			int t = hour.toUInt(&okhour) * 60 + m;
			if (okhour  &&  okmin  &&  m < 60  &&  t >= minimumValue  &&  t <= maxValue())
			{
				*ok = true;
				return t;
			}
		}
	}
	*ok = false;
	return 0;
}

/******************************************************************************
 * Set the spin box as valid or invalid.
 * If newly invalid, the value is displayed as asterisks.
 * If newly valid, the value is set to the minimum value.
 */
void TimeSpinBox::setValid(bool valid)
{
	if (valid  &&  invalid)
	{
		invalid = false;
		if (value() < minimumValue)
			SpinBox2::setValue(minimumValue);
		setSpecialValueText(QString());
		setMinValue(minimumValue);
	}
	else if (!valid  &&  !invalid)
	{
		invalid = true;
		setMinValue(minimumValue - 1);
		setSpecialValueText(QString::fromLatin1("**:**"));
		SpinBox2::setValue(minimumValue - 1);
	}
}

/******************************************************************************
 * Set the spin box as valid or invalid.
 * If newly invalid, the value is displayed as asterisks.
 * If newly valid, the value is set to the minimum value.
 */
void TimeSpinBox::setValue(int value)
{
	if (!enteredSetValue)
	{
		enteredSetValue = true;
		if (invalid)
		{
			invalid = false;
			setSpecialValueText(QString());
			setMinValue(minimumValue);
		}
		SpinBox2::setValue(value);
		enteredSetValue = false;
	}
}

/******************************************************************************
 * Step the spin box value.
 * If it was invalid, set it valid and set the value to the minimum.
 */
void TimeSpinBox::stepUp()
{
	if (invalid)
		setValid(true);
	else
		SpinBox2::stepUp();
}

void TimeSpinBox::stepDown()
{
	if (invalid)
		setValid(true);
	else
		SpinBox2::stepDown();
}


/******************************************************************************
 * Validate the time spin box input.
 * The entered time must contain a colon, but hours and/or minutes may be blank.
 */
QValidator::State TimeSpinBox::TimeValidator::validate(QString& text, int& /*cursorPos*/) const
{
	QValidator::State state = QValidator::Acceptable;
	QString hour;
	int hr;
	int mn = 0;
	int colon = text.find(':');
	if (colon >= 0)
	{
		QString minute = text.mid(colon + 1).stripWhiteSpace();
		if (minute.isEmpty())
			state = QValidator::Intermediate;
		else
		{
			bool ok;
			if ((mn = minute.toUInt(&ok)) >= 60  ||  !ok)
				return QValidator::Invalid;
		}

		hour = text.left(colon).stripWhiteSpace();
	}
	else
	{
		state = QValidator::Intermediate;
		hour = text;
	}

	if (hour.isEmpty())
		return QValidator::Intermediate;
	bool ok;
	if ((hr = hour.toUInt(&ok)) > maxMinute/60  ||  !ok)
		return QValidator::Invalid;
	if (state == QValidator::Acceptable)
	{
		int t = hr * 60 + mn;
		if (t < minMinute  ||  t > maxMinute)
			return QValidator::Invalid;
	}
	return state;
}
