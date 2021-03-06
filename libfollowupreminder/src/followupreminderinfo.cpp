/*
  Copyright (c) 2014-2015 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "followupreminderinfo.h"

#include <KConfigGroup>
using namespace FollowUpReminder;

FollowUpReminderInfo::FollowUpReminderInfo()
    : mOriginalMessageItemId(-1),
      mAnswerMessageItemId(-1),
      mTodoId(-1),
      mUniqueIdentifier(-1),
      mAnswerWasReceived(false)
{
}

FollowUpReminderInfo::FollowUpReminderInfo(const KConfigGroup &config)
    : mOriginalMessageItemId(-1),
      mAnswerMessageItemId(-1),
      mTodoId(-1),
      mUniqueIdentifier(-1),
      mAnswerWasReceived(false)
{
    readConfig(config);
}

FollowUpReminderInfo::FollowUpReminderInfo(const FollowUpReminderInfo &info)
{
    mFollowUpReminderDate = info.followUpReminderDate();
    mOriginalMessageItemId = info.originalMessageItemId();
    mMessageId = info.messageId();
    mTo = info.to();
    mSubject = info.subject();
    mAnswerWasReceived = info.answerWasReceived();
    mAnswerMessageItemId = info.answerMessageItemId();
    mUniqueIdentifier = info.uniqueIdentifier();
    mTodoId = info.todoId();
}

void FollowUpReminderInfo::readConfig(const KConfigGroup &config)
{
    if (config.hasKey(QStringLiteral("followUpReminderDate"))) {
        mFollowUpReminderDate = QDate::fromString(config.readEntry("followUpReminderDate"), Qt::ISODate);
    }
    mOriginalMessageItemId = config.readEntry("itemId", -1);
    mMessageId = config.readEntry("messageId", QString());
    mTo = config.readEntry("to", QString());
    mSubject = config.readEntry("subject", QString());
    mAnswerWasReceived = config.readEntry("answerWasReceived", false);
    mAnswerMessageItemId = config.readEntry("answerMessageItemId", -1);
    mTodoId = config.readEntry("todoId", -1);
    mUniqueIdentifier = config.readEntry("identifier", -1);
}

qint32 FollowUpReminderInfo::uniqueIdentifier() const
{
    return mUniqueIdentifier;
}

void FollowUpReminderInfo::setUniqueIdentifier(const qint32 &uniqueIdentifier)
{
    mUniqueIdentifier = uniqueIdentifier;
}

Akonadi::Item::Id FollowUpReminderInfo::answerMessageItemId() const
{
    return mAnswerMessageItemId;
}

void FollowUpReminderInfo::setAnswerMessageItemId(const Akonadi::Item::Id &answerMessageId)
{
    mAnswerMessageItemId = answerMessageId;
}

bool FollowUpReminderInfo::answerWasReceived() const
{
    return mAnswerWasReceived;
}

void FollowUpReminderInfo::setAnswerWasReceived(bool answerWasReceived)
{
    mAnswerWasReceived = answerWasReceived;
}

QString FollowUpReminderInfo::subject() const
{
    return mSubject;
}

void FollowUpReminderInfo::setSubject(const QString &subject)
{
    mSubject = subject;
}

void FollowUpReminderInfo::writeConfig(KConfigGroup &config, qint32 identifier)
{
    if (mFollowUpReminderDate.isValid()) {
        config.writeEntry("followUpReminderDate", mFollowUpReminderDate.toString(Qt::ISODate));
    }
    setUniqueIdentifier(identifier);
    config.writeEntry("messageId", mMessageId);
    config.writeEntry("itemId", mOriginalMessageItemId);
    config.writeEntry("to", mTo);
    config.writeEntry("subject", mSubject);
    config.writeEntry("answerWasReceived", mAnswerWasReceived);
    config.writeEntry("answerMessageItemId", mAnswerMessageItemId);
    config.writeEntry("todoId", mTodoId);
    config.writeEntry("identifier", identifier);
    config.sync();
}

Akonadi::Item::Id FollowUpReminderInfo::originalMessageItemId() const
{
    return mOriginalMessageItemId;
}

void FollowUpReminderInfo::setOriginalMessageItemId(Akonadi::Item::Id value)
{
    mOriginalMessageItemId = value;
}

Akonadi::Item::Id FollowUpReminderInfo::todoId() const
{
    return mTodoId;
}

void FollowUpReminderInfo::setTodoId(Akonadi::Item::Id value)
{
    mTodoId = value;
}

bool FollowUpReminderInfo::isValid() const
{
    return (!mMessageId.isEmpty() &&
            mFollowUpReminderDate.isValid() &&
            !mTo.isEmpty());
}

QString FollowUpReminderInfo::messageId() const
{
    return mMessageId;
}

void FollowUpReminderInfo::setMessageId(const QString &messageId)
{
    mMessageId = messageId;
}

void FollowUpReminderInfo::setTo(const QString &to)
{
    mTo = to;
}

QString FollowUpReminderInfo::to() const
{
    return mTo;
}

QDate FollowUpReminderInfo::followUpReminderDate() const
{
    return mFollowUpReminderDate;
}

void FollowUpReminderInfo::setFollowUpReminderDate(const QDate &followUpReminderDate)
{
    mFollowUpReminderDate = followUpReminderDate;
}

bool FollowUpReminderInfo::operator==(const FollowUpReminderInfo &other) const
{
    return mOriginalMessageItemId == other.originalMessageItemId()
           && mMessageId == other.messageId()
           && mTo == other.to()
           && mFollowUpReminderDate == other.followUpReminderDate()
           && mSubject == other.subject()
           && mAnswerWasReceived == other.answerWasReceived()
           && mAnswerMessageItemId == other.answerMessageItemId()
           && mUniqueIdentifier == other.uniqueIdentifier()
           && mTodoId == other.todoId();
}

