#include "incidencealarm.h"

#include "alarmpresets.h"
#include "ui_eventortododesktop.h"

using namespace IncidenceEditorsNG;

IncidenceAlarm::IncidenceAlarm( Ui::EventOrTodoDesktop *ui )
  : mUi( ui )
{
  mUi->mAlarmConfigureButton->setEnabled( false );
  mUi->mAlarmRemoveButton->setEnabled( false );
  mUi->mAlarmPresetCombo->insertItems( 0, AlarmPresets::availablePresets() );
  mUi->mAlarmPresetCombo->setCurrentIndex( 2 );

  mDisabledAlarms.setAutoDelete( true );
  mEnabledAlarms.setAutoDelete( true );

  connect( mUi->mAlarmPresetCombo, SIGNAL(currentIndexChanged(int)),
           SLOT(updateAlarmPreset(int)) );
  connect( mUi->mAlarmAddPresetButton, SIGNAL(clicked()),
           SLOT(addAlarm()) );
  connect( mUi->mAlarmList, SIGNAL(itemSelectionChanged ()),
           SLOT(updateButtons()) );
  connect( mUi->mAlarmRemoveButton, SIGNAL(clicked()),
           SLOT(removeCurrentAlarm()) );
}

void IncidenceAlarm::load( KCal::Incidence::ConstPtr incidence )
{
  mLoadedIncidence = incidence;

  if ( incidence->isAlarmEnabled() ) {
    foreach ( KCal::Alarm *alarm, incidence->alarms() ) {
      if ( alarm->enabled() ) {
        mEnabledAlarms.append( new KCal::Alarm( *alarm ) );
      } else {
        mDisabledAlarms.append( new KCal::Alarm( *alarm ) );
      }
    }
  } else // All alarms that are possibily there are disabled
    mDisabledAlarms = incidence->alarms();

  mWasDirty = false;
}

void IncidenceAlarm::save( KCal::Incidence::Ptr incidence )
{
  incidence->clearAlarms();
  KCal::Alarm::List::ConstIterator it;
  for ( it = mEnabledAlarms.constBegin(); it != mEnabledAlarms.constEnd(); ++it ) {
    KCal::Alarm *al = new KCal::Alarm( *(*it) );
    al->setParent( incidence.get() );
    // We need to make sure that both lists are the same in the end for isDirty.
    Q_ASSERT( *al == *(*it) );
    incidence->addAlarm( al );
  }

  for ( it = mDisabledAlarms.constBegin(); it != mDisabledAlarms.constEnd(); ++it ) {
    KCal::Alarm *al = new KCal::Alarm( *(*it) );
    al->setParent( incidence.get() );
    // We need to make sure that both lists are the same in the end for isDirty.
    Q_ASSERT( *al == *(*it) );
    incidence->addAlarm( al );
  }
}

bool IncidenceAlarm::isDirty() const
{
  if ( !mLoadedIncidence->isAlarmEnabled() && mEnabledAlarms.count() > 0 )
    return true;

  if ( mLoadedIncidence->isAlarmEnabled() && mEnabledAlarms.count() == 0 )
    return true;

  if ( mLoadedIncidence->isAlarmEnabled() ) {
    const KCal::Alarm::List initialAlarms = mLoadedIncidence->alarms();

    if ( initialAlarms.count() != ( mDisabledAlarms.count() + mEnabledAlarms.count() ) )
      return true; // The number of alarms has changed

    // Note: Not the most efficient algorithm but I'm assuming that we're only
    //       dealing with a couple, at most tens of alarms. The idea is we check
    //       if all currently enabled alarms are also in the incidence. The
    //       disabled alarms are not changed by our code at all, so we assume that
    //       they're still there.
    foreach ( const KCal::Alarm *enabledAlarm, mEnabledAlarms ) {
      bool found = false;
      foreach ( const KCal::Alarm *alarm, initialAlarms ) {
        if ( *enabledAlarm == *alarm ) {
          found  = true;
          break;
        }
      }

      if ( !found ) {
        // There was an alarm in the mLoadedIncidence->alarms() that wasn't found
        // in mLastAlarms. This means that one of the alarms was modified.
        return true;
      }
    }
  }

 return false;
}

//void IncidenceAlarm::editAlarm()
//{
//#ifndef KDEPIM_MOBILE_UI
//  QScopedPointer<EditorAlarms> dialog( new EditorAlarms( mLoadedIncidence->type(), &mLastAlarms, this ) );

//  if ( dialog->exec() == KDialog::Accepted ) {
//    mUi->mAlarmCombo->blockSignals( true );
//    if ( mLastAlarms.isEmpty() )
//      mUi->mAlarmCombo->setCurrentIndex( 0 );
//    else if ( mLastAlarms.size() > 1 )
//      mUi->mAlarmCombo->setCurrentIndex( mUi->mAlarmCombo->count() - 1 ); // Custom
//    else { // Only one alarm
//      const int index = AlarmPresets::presetIndex( *mLastAlarms.first() );
//      if ( index == -1 ) {
////        mUi->mRecurrenceCombo->setCurrentIndex( mUi->mRecurrenceCombo->count() - 1 );
//      } else {
//        // Add one to cope with the "no alarm" option in the combo, which is not
//        // in the presets.
////        mUi->mRecurrenceCombo->setCurrentIndex( index + 1 );
////        mUi->mRecurrenceEditButton->setEnabled( true );
//      }
//    }

//    mUi->mAlarmCombo->blockSignals( false );
//  }
//#endif
//}

void IncidenceAlarm::addAlarm()
{
  mEnabledAlarms.append( AlarmPresets::preset( mUi->mAlarmPresetCombo->currentText() ) );
  updateAlarmList();

  checkDirtyStatus();
}

void IncidenceAlarm::removeCurrentAlarm()
{
  Q_ASSERT( mUi->mAlarmList->selectedItems().size() == 1 );
  const int curAlarmIndex = mUi->mAlarmList->currentRow();
  delete mUi->mAlarmList->takeItem( curAlarmIndex );
  mEnabledAlarms.removeAt( curAlarmIndex );

  updateButtons();
  checkDirtyStatus();
}

void IncidenceAlarm::updateAlarmList()
{
  mUi->mAlarmList->clear();
  foreach ( KCal::Alarm *alarm, mEnabledAlarms ) {
    mUi->mAlarmList->addItem( stringForAlarm( alarm ) );
  }
}

void IncidenceAlarm::updateButtons()
{
  if ( mUi->mAlarmList->count() > 0 && mUi->mAlarmList->selectedItems().count() > 0 ) {
    mUi->mAlarmConfigureButton->setEnabled( true );
    mUi->mAlarmRemoveButton->setEnabled( true );
  } else {
    mUi->mAlarmConfigureButton->setEnabled( false );
    mUi->mAlarmRemoveButton->setEnabled( false );
  }
}


QString IncidenceAlarm::stringForAlarm( KCal::Alarm *alarm )
{
  Q_ASSERT( alarm );

  QString action;
  switch( alarm->type() ) {
  case KCal::Alarm::Display:
    action = i18n( "Display a dialog" );
    break;
  case KCal::Alarm::Procedure:
    action = i18n( "Execute a script" );
    break;
  case KCal::Alarm::Email:
    action = i18n( "Send an email" );
    break;
  case KCal::Alarm::Audio:
    action = i18n( "Play an audio file" );
    break;
  default:
    action = i18n( "Invalid Alarm." );
    return action;
  }

  QString offsetUnit = i18nc( "The alarm is set to X minutes before/after the event", "minutes" );
  const int offset = alarm->startOffset().asSeconds() / 60; // make minutes
  int useoffset = offset;

  if ( offset % ( 24 * 60 ) == 0 && offset > 0 ) { // divides evenly into days?
    useoffset =  offset / 60 / 24;
    offsetUnit = i18nc( "The alarm is set to X days before/after the event", "days" );
  } else if ( offset % 60 == 0 && offset > 0 ) { // divides evenly into hours?
    offsetUnit = i18nc( "The alarm is set to X hours before/after the event", "hours" );
    useoffset = offset / 60;
  }

  if ( useoffset > 0 )
    return i18n( "%1 %2 %3 after the event started", action, QString::number( useoffset ), offsetUnit );
  else if ( useoffset < 0 )
    return i18n( "%1 %2 %3 before the event starts", action, QString::number( qAbs( useoffset ) ), offsetUnit );

  // useoffset == 0
  return i18n( "%1 when the event starts", action );
}
