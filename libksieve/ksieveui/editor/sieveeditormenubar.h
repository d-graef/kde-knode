/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

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

#ifndef SIEVEEDITORMENUBAR_H
#define SIEVEEDITORMENUBAR_H

#include <QMenuBar>
class QAction;
namespace KSieveUi {
class SieveEditorMenuBar : public QMenuBar
{
    Q_OBJECT
public:
    explicit SieveEditorMenuBar(QWidget *parent = 0);
    ~SieveEditorMenuBar();

    QAction *goToLine() const;
    QAction *findAction() const;
    QAction *replaceAction() const;
    QAction *undoAction() const;
    QAction *redoAction() const;
    QAction *copyAction() const;
    QAction *pasteAction() const;
    QAction *cutAction() const;
    QAction *selectAllAction() const;

Q_SIGNALS:
    void gotoLine();
    void find();
    void replace();
    void undo();
    void redo();
    void copy();
    void paste();
    void cut();
    void selectAll();

private:
    void initActions();
    QAction *mGoToLine;
    QAction *mFindAction;
    QAction *mReplaceAction;
    QAction *mUndoAction;
    QAction *mRedoAction;
    QAction *mCopyAction;
    QAction *mPasteAction;
    QAction *mCutAction;
    QAction *mSelectAllAction;
};
}

#endif // SIEVEEDITORMENUBAR_H
