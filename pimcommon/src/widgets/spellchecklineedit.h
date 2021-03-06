/*
 * Copyright (c) 2011-2015 Montel Laurent <montel@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; version 2 of the License
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *  In addition, as a special exception, the copyright holders give
 *  permission to link the code of this program with any edition of
 *  the Qt library by Trolltech AS, Norway (or with modified versions
 *  of Qt that use the same license as Qt), and distribute linked
 *  combinations including the two.  You must obey the GNU General
 *  Public License in all respects for all of the code used other than
 *  Qt.  If you modify this file, you may extend this exception to
 *  your version of the file, but you are not obligated to do so.  If
 *  you do not wish to do so, delete this exception statement from
 *  your version.
 */

#ifndef SpellCheckLineEdit_H
#define SpellCheckLineEdit_H
#include "pimcommon_export.h"
#include "kpimtextedit/richtexteditor.h"

class QMimeData;
namespace PimCommon
{

class PIMCOMMON_EXPORT SpellCheckLineEdit : public KPIMTextEdit::RichTextEditor
{
    Q_OBJECT

public:
    /**
    * Constructs a SpellCheckLineEdit object.
    * @param parent of widget
    * @param configFile config file name for spell checking
    */
    explicit SpellCheckLineEdit(QWidget *parent, const QString &configFile);
    /**
    * Destructor
    */
    ~SpellCheckLineEdit();

protected:
    QSize sizeHint() const Q_DECL_OVERRIDE;
    QSize minimumSizeHint() const Q_DECL_OVERRIDE;
    void keyPressEvent(QKeyEvent *) Q_DECL_OVERRIDE;
    void insertFromMimeData(const QMimeData *source) Q_DECL_OVERRIDE;

Q_SIGNALS:
    /**
    * Emitted when the user uses the up arrow in the first line. The application
    * should then put the focus on the widget above the text edit.
    */
    void focusUp();

    void focusDown();
};
}
#endif /* SpellCheckLineEdit_H */

