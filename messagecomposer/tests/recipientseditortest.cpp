/*
    This file is part of KMail.

    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "recipientseditortest.h"
#include <MessageComposer/RecipientsEditor>

#include <qdebug.h>
#include <KLocalizedString>

#include <kmessagebox.h>
#include <KAboutData>

#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QGridLayout>
#include <QApplication>
#include <QCommandLineParser>

using namespace MessageComposer;

Composer::Composer(QWidget *parent)
    : QWidget(parent)
{
    QGridLayout *topLayout = new QGridLayout(this);
    topLayout->setMargin(4);
    topLayout->setSpacing(4);

    QLabel *label = new QLabel(QStringLiteral("From:"), this);
    topLayout->addWidget(label, 0, 0);
    QLineEdit *edit = new QLineEdit(this);
    topLayout->addWidget(edit, 0, 1);

    mRecipients = new RecipientsEditor(this);
    topLayout->addWidget(mRecipients, 1, 0, 1, 2);

    qDebug() << "SIZEHINT:" << mRecipients->sizeHint();

//  mRecipients->setFixedHeight( 10 );

    QTextEdit *editor = new QTextEdit(this);
    topLayout->addWidget(editor, 2, 0, 1, 2);
    topLayout->setRowStretch(2, 1);

    QPushButton *button = new QPushButton(QStringLiteral("&Close"), this);
    topLayout->addWidget(button, 3, 0, 1, 2);
    connect(button, &QPushButton::clicked, this, &Composer::slotClose);
}

void Composer::slotClose()
{
#if 0
    QString text;

    text += "<qt>";

    Recipient::List recipients = mRecipients->recipients();
    Recipient::List::ConstIterator it;
    for (it = recipients.begin(); it != recipients.end(); ++it) {
        text += "<b>" + (*it).typeLabel() + ":</b> " + (*it).email() + "<br/>";
    }

    text += "</qt>";

    KMessageBox::information(this, text);
#endif

    close();
}

int main(int argc, char **argv)
{
    KAboutData aboutData(QStringLiteral("testrecipienteditor"),
                         i18n("Test Recipient Editor"), QStringLiteral("0.1"));
    QApplication app(argc, argv);
    QCommandLineParser parser;
    KAboutData::setApplicationData(aboutData);
    parser.addVersionOption();
    parser.addHelpOption();
    aboutData.setupCommandLine(&parser);
    parser.process(app);
    aboutData.processCommandLine(&parser);

    QObject::connect(&app, &QApplication::lastWindowClosed, &app, &QApplication::quit);

    QWidget *wid = new Composer(Q_NULLPTR);

    wid->show();

    int ret = app.exec();

    delete wid;

    return ret;
}

