#ifndef COMMANDSCHEDULER_H
#define COMMANDSCHEDULER_H
// $Id$

#include <qobject.h>
#include <qlist.h>

#include "atcommand.h"

class Modem;

class CommandScheduler : public QObject {
    Q_OBJECT
  public:
    CommandScheduler (Modem *modem,QObject *parent = 0, const char *name = 0);

    void execute(const QString &command);
    void execute(ATCommand *command);

  signals:
    void result(const QString &);
    void commandProcessed(ATCommand *);

  private slots:
    void processOutput(const char *line);

  private:
    void sendCommand(const QString &command);
    void nextCommand();

  private:
    Modem *mModem;  

    ATCommand *mLastCommand;

    QList<ATCommand> mCommandQueue;

    enum State { WAITING, PROCESSING };
    State mState;
    
    QString mResult;
};

#endif
