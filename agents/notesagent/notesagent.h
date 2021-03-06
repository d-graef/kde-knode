/*
  Copyright (c) 2013-2015 Montel Laurent <montel@kde.org>

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

#ifndef NOTESAGENT_H
#define NOTESAGENT_H

#include <AkonadiAgentBase/agentbase.h>
class NotesManager;
class NotesAgent : public Akonadi::AgentBase, public Akonadi::AgentBase::ObserverV3
{
    Q_OBJECT
public:
    explicit NotesAgent(const QString &id);
    ~NotesAgent();

    void showConfigureDialog(qlonglong windowId = 0);

    void setEnableAgent(bool b);
    bool enabledAgent() const;

    void reload();

    bool receiveNotes() const;
    void setReceiveNotes(bool b);

    void setPort(int value);
    int port() const;

    int alarmCheckInterval() const;
    void setAlarmCheckInterval(int value);

    void configurationChanged();

Q_SIGNALS:
    void needUpdateConfigDialogBox();

public Q_SLOTS:
    void configure(WId windowId) Q_DECL_OVERRIDE;

protected:
    void doSetOnline(bool online) Q_DECL_OVERRIDE;

private Q_SLOTS:
    void slotStartAgent();

private:
    bool mAgentInitialized;
    NotesManager *mNotesManager;
};

#endif // NOTESAGENT_H
