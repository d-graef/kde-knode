/*
  Copyright (c) 2007 Volker Krause <vkrause@kde.org>
  Copyright (C) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.net
  Author: Sergio Martins <sergio.martins@kdab.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/
#include "multiagendaview.h"
#include "configdialoginterface.h"
#include "prefs.h"

#include "agenda/agenda.h"
#include "agenda/agendaview.h"
#include "agenda/timelabelszone.h"

#include <entitytreemodel.h>
#include <Akonadi/Calendar/ETMCalendar>
#include <CalendarSupport/CollectionSelection>
#include <CalendarSupport/Utils>

#include <ETMViewStateSaver>

#include <KCalCore/Event>

#include <KCheckableProxyModel>
#include <KLocalizedString>
#include <QVBoxLayout>
#include <KViewStateMaintainer>
#include "calendarview_debug.h"

#include <QHBoxLayout>
#include <QItemSelectionModel>
#include <QLabel>
#include <QResizeEvent>
#include <QScrollArea>
#include <QScrollBar>
#include <QSplitter>
#include <QSortFilterProxyModel>
#include <QTimer>
#include <QPainter>
#include <KRearrangeColumnsProxyModel>

using namespace Akonadi;
using namespace EventViews;

/**
   Function for debugging purposes:
   prints an object's sizeHint()/minimumSizeHint()/policy
   and it's children's too, recursively
*/
/*
static void printObject( QObject *o, int level = 0 )
{
  QMap<int,QString> map;
  map.insert( 0, "Fixed" );
  map.insert( 1, "Minimum" );
  map.insert( 4, "Maximum" );
  map.insert( 5, "Preferred" );
  map.insert( 7, "Expanding" );
  map.insert( 3, "MinimumExpaning" );
  map.insert( 13, "Ignored" );

  QWidget *w = qobject_cast<QWidget*>( o );

  if ( w ) {
    qCDebug(CALENDARVIEW_LOG) << QString( level*2, '-' ) << o
             << w->sizeHint() << "/" << map[w->sizePolicy().verticalPolicy()]
             << "; minimumSize = " << w->minimumSize()
             << "; minimumSizeHint = " << w->minimumSizeHint();
  } else {
    qCDebug(CALENDARVIEW_LOG) << QString( level*2, '-' ) << o ;
  }

  foreach( QObject *child, o->children() ) {
    printObject( child, level + 1 );
  }
}
*/

static QString generateColumnLabel(int c)
{
    return i18n("Agenda %1", c + 1);
}

class Q_DECL_HIDDEN MultiAgendaView::Private
{
private:
    class ElidedLabel: public QFrame
    {
    public:
        ElidedLabel(const QString &text): mText(text) {}

        QSize minimumSizeHint() const Q_DECL_OVERRIDE;

    protected:
        void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;

    private:
        QString mText;
    };

public:
    Private(MultiAgendaView *qq) :
        q(qq),
        mUpdateOnShow(true),
        mPendingChanges(true),
        mCustomColumnSetupUsed(false),
        mCustomNumberOfColumns(2)
    {
    }

    ~Private()
    {
        qDeleteAll(mSelectionSavers);
    }

    void addView(const Akonadi::Collection &collection);
    void addView(KCheckableProxyModel *selectionProxy, const QString &title);
    AgendaView *createView(const QString &title);
    void deleteViews();
    void setupViews();
    void resizeScrollView(const QSize &size);

    MultiAgendaView *q;
    QList<AgendaView *> mAgendaViews;
    QList<QWidget *> mAgendaWidgets;
    QWidget *mTopBox;
    QScrollArea *mScrollArea;
    TimeLabelsZone *mTimeLabelsZone;
    QSplitter *mLeftSplitter, *mRightSplitter;
    QScrollBar *mScrollBar;
    QWidget *mLeftBottomSpacer, *mRightBottomSpacer;
    QDate mStartDate, mEndDate;
    bool mUpdateOnShow;
    bool mPendingChanges;
    bool mCustomColumnSetupUsed;
    QVector<KCheckableProxyModel *> mCollectionSelectionModels;
    QStringList mCustomColumnTitles;
    int mCustomNumberOfColumns;
    QLabel *mLabel;
    QWidget *mRightDummyWidget;
    QHash<QString, KViewStateMaintainer<ETMViewStateSaver>* > mSelectionSavers;
};

MultiAgendaView::MultiAgendaView(QWidget *parent)
    : EventView(parent), d(new Private(this))

{
    QHBoxLayout *topLevelLayout = new QHBoxLayout(this);
    topLevelLayout->setSpacing(0);
    topLevelLayout->setMargin(0);

    QFontMetrics fm(font());
    int topLabelHeight = 2 * fm.height() + fm.lineSpacing();

    QWidget *topSideBox = new QWidget(this);
    QVBoxLayout *topSideBoxVBoxLayout = new QVBoxLayout(topSideBox);
    topSideBoxVBoxLayout->setMargin(0);

    QWidget *topSideSpacer = new QWidget(topSideBox);
    topSideBoxVBoxLayout->addWidget(topSideSpacer);
    topSideSpacer->setFixedHeight(topLabelHeight);

    d->mLeftSplitter = new QSplitter(Qt::Vertical, topSideBox);
    topSideBoxVBoxLayout->addWidget(d->mLeftSplitter);

    d->mLabel = new QLabel(i18n("All Day"), d->mLeftSplitter);
    d->mLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    d->mLabel->setWordWrap(true);

    QWidget *sideBox = new QWidget(d->mLeftSplitter);
    QVBoxLayout *sideBoxVBoxLayout = new QVBoxLayout(sideBox);
    sideBoxVBoxLayout->setMargin(0);

    // compensate for the frame the agenda views but not the timelabels have
    QWidget *timeLabelTopAlignmentSpacer = new QWidget(sideBox);
    sideBoxVBoxLayout->addWidget(timeLabelTopAlignmentSpacer);

    d->mTimeLabelsZone = new TimeLabelsZone(sideBox, PrefsPtr(new Prefs()));

    QWidget *timeLabelBotAlignmentSpacer = new QWidget(sideBox);
    sideBoxVBoxLayout->addWidget(timeLabelBotAlignmentSpacer);

    d->mLeftBottomSpacer = new QWidget(topSideBox);
    topSideBoxVBoxLayout->addWidget(d->mLeftBottomSpacer);

    topLevelLayout->addWidget(topSideBox);

    d->mScrollArea = new QScrollArea(this);
    d->mScrollArea->setWidgetResizable(true);

    d->mScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // BUG: timelabels aren't aligned with the agenda's grid, 2 or 3 pixels of offset.
    // asymetric since the timelabels
    timeLabelTopAlignmentSpacer->setFixedHeight(d->mScrollArea->frameWidth() - 1);
    // have 25 horizontal lines
    timeLabelBotAlignmentSpacer->setFixedHeight(d->mScrollArea->frameWidth() - 2);

    d->mScrollArea->setFrameShape(QFrame::NoFrame);
    topLevelLayout->addWidget(d->mScrollArea, 100);
    d->mTopBox = new QWidget(d->mScrollArea->viewport());
    QHBoxLayout *mTopBoxHBoxLayout = new QHBoxLayout(d->mTopBox);
    mTopBoxHBoxLayout->setMargin(0);
    d->mScrollArea->setWidget(d->mTopBox);

    topSideBox = new QWidget(this);
    topSideBoxVBoxLayout = new QVBoxLayout(topSideBox);
    topSideBoxVBoxLayout->setMargin(0);

    topSideSpacer = new QWidget(topSideBox);
    topSideBoxVBoxLayout->addWidget(topSideSpacer);
    topSideSpacer->setFixedHeight(topLabelHeight);

    d->mRightSplitter = new QSplitter(Qt::Vertical, topSideBox);
    topSideBoxVBoxLayout->addWidget(d->mRightSplitter);

    connect(d->mLeftSplitter, &QSplitter::splitterMoved, this, &MultiAgendaView::resizeSplitters);
    connect(d->mRightSplitter, &QSplitter::splitterMoved, this, &MultiAgendaView::resizeSplitters);

    d->mRightDummyWidget = new QWidget(d->mRightSplitter);

    d->mScrollBar = new QScrollBar(Qt::Vertical, d->mRightSplitter);

    d->mRightBottomSpacer = new QWidget(topSideBox);
    topSideBoxVBoxLayout->addWidget(d->mRightBottomSpacer);
    topLevelLayout->addWidget(topSideBox);
}

void MultiAgendaView::setCalendar(const Akonadi::ETMCalendar::Ptr &calendar)
{
    EventView::setCalendar(calendar);
    Q_FOREACH (KCheckableProxyModel *proxy, d->mCollectionSelectionModels) {
        proxy->setSourceModel(calendar->entityTreeModel());
    }

    disconnect(Q_NULLPTR, SIGNAL(selectionChanged(Akonadi::Collection::List,Akonadi::Collection::List)),
               this, SLOT(forceRecreateViews()));

    connect(collectionSelection(), &CalendarSupport::CollectionSelection::selectionChanged,
            this, &MultiAgendaView::forceRecreateViews);

    recreateViews();
}

void MultiAgendaView::recreateViews()
{
    if (!d->mPendingChanges) {
        return;
    }
    d->mPendingChanges = false;

    d->deleteViews();

    if (d->mCustomColumnSetupUsed) {
        Q_ASSERT(d->mCollectionSelectionModels.size() == d->mCustomNumberOfColumns);
        for (int i = 0; i < d->mCustomNumberOfColumns; ++i) {
            d->addView(d->mCollectionSelectionModels[i], d->mCustomColumnTitles[i]);
        }
    } else {
        Q_FOREACH (const Akonadi::Collection &i, collectionSelection()->selectedCollections()) {
            if (i.contentMimeTypes().contains(KCalCore::Event::eventMimeType())) {
                d->addView(i);
            }
        }
    }
    // no resources activated, so stop here to avoid crashing somewhere down the line
    // TODO: show a nice message instead
    if (d->mAgendaViews.isEmpty()) {
        return;
    }

    d->setupViews();
    QTimer::singleShot(0, this, &MultiAgendaView::slotResizeScrollView);
    d->mTimeLabelsZone->updateAll();

    QScrollArea *timeLabel = d->mTimeLabelsZone->timeLabels().first();
    connect(timeLabel->verticalScrollBar(), &QAbstractSlider::valueChanged,
            d->mScrollBar, &QAbstractSlider::setValue);
    connect(d->mScrollBar, &QAbstractSlider::valueChanged,
            timeLabel->verticalScrollBar(), &QAbstractSlider::setValue);

    resizeSplitters();
    QTimer::singleShot(0, this, &MultiAgendaView::setupScrollBar);

    d->mTimeLabelsZone->updateTimeLabelsPosition();
}

void MultiAgendaView::forceRecreateViews()
{
    d->mPendingChanges = true;
    recreateViews();
}

void MultiAgendaView::Private::deleteViews()
{
    Q_FOREACH (AgendaView *const i, mAgendaViews) {
        KCheckableProxyModel *proxy = i->takeCustomCollectionSelectionProxyModel();
        if (proxy && !mCollectionSelectionModels.contains(proxy)) {
            delete proxy;
        }
        delete i;
    }

    mAgendaViews.clear();
    mTimeLabelsZone->setAgendaView(Q_NULLPTR);
    qDeleteAll(mAgendaWidgets);
    mAgendaWidgets.clear();
}

void MultiAgendaView::Private::setupViews()
{
    foreach (AgendaView *agenda, mAgendaViews) {
        q->connect(agenda, SIGNAL(newEventSignal()),
                   q, SIGNAL(newEventSignal()));
        q->connect(agenda, SIGNAL(newEventSignal(QDate)),
                   q, SIGNAL(newEventSignal(QDate)));
        q->connect(agenda, SIGNAL(newEventSignal(QDateTime)),
                   q, SIGNAL(newEventSignal(QDateTime)));
        q->connect(agenda,
                   SIGNAL(newEventSignal(QDateTime,QDateTime)),
                   q, SIGNAL(newEventSignal(QDateTime,QDateTime)));

        q->connect(agenda, &EventView::editIncidenceSignal,
                   q, &EventView::editIncidenceSignal);
        q->connect(agenda, &EventView::showIncidenceSignal,
                   q, &EventView::showIncidenceSignal);
        q->connect(agenda, &EventView::deleteIncidenceSignal,
                   q, &EventView::deleteIncidenceSignal);

        q->connect(agenda, &EventView::incidenceSelected,
                   q, &EventView::incidenceSelected);

        q->connect(agenda, &EventView::cutIncidenceSignal,
                   q, &EventView::cutIncidenceSignal);
        q->connect(agenda, &EventView::copyIncidenceSignal,
                   q, &EventView::copyIncidenceSignal);
        q->connect(agenda, &EventView::pasteIncidenceSignal,
                   q, &EventView::pasteIncidenceSignal);
        q->connect(agenda, &EventView::toggleAlarmSignal,
                   q, &EventView::toggleAlarmSignal);
        q->connect(agenda, &EventView::dissociateOccurrencesSignal,
                   q, &EventView::dissociateOccurrencesSignal);

        q->connect(agenda, &EventView::newTodoSignal,
                   q, &EventView::newTodoSignal);

        q->connect(agenda, &EventView::incidenceSelected,
                   q, &MultiAgendaView::slotSelectionChanged);

        q->connect(agenda, &AgendaView::timeSpanSelectionChanged,
                   q, &MultiAgendaView::slotClearTimeSpanSelection);

        q->disconnect(agenda->agenda(),
                      SIGNAL(zoomView(int,QPoint,Qt::Orientation)),
                      agenda, Q_NULLPTR);
        q->connect(agenda->agenda(),
                   &Agenda::zoomView,
                   q, &MultiAgendaView::zoomView);
    }

    AgendaView *lastView = mAgendaViews.last();
    foreach (AgendaView *agenda, mAgendaViews) {
        if (agenda != lastView) {
            connect(agenda->agenda()->verticalScrollBar(), &QAbstractSlider::valueChanged,
                    lastView->agenda()->verticalScrollBar(), &QAbstractSlider::setValue);
        }
    }

    foreach (AgendaView *agenda, mAgendaViews) {
        agenda->readSettings();
    }
}

MultiAgendaView::~MultiAgendaView()
{
    delete d;
}

Akonadi::Item::List MultiAgendaView::selectedIncidences() const
{
    Akonadi::Item::List list;
    foreach (AgendaView *agendaView, d->mAgendaViews) {
        list += agendaView->selectedIncidences();
    }
    return list;
}

KCalCore::DateList MultiAgendaView::selectedIncidenceDates() const
{
    KCalCore::DateList list;
    foreach (AgendaView *agendaView, d->mAgendaViews) {
        list += agendaView->selectedIncidenceDates();
    }
    return list;
}

int MultiAgendaView::currentDateCount() const
{
    foreach (AgendaView *agendaView, d->mAgendaViews) {
        return agendaView->currentDateCount();
    }
    return 0;
}

void MultiAgendaView::showDates(const QDate &start, const QDate &end, const QDate &preferredMonth)
{
    Q_UNUSED(preferredMonth);
    d->mStartDate = start;
    d->mEndDate = end;
    slotResizeScrollView();
    d->mTimeLabelsZone->updateAll();
    foreach (AgendaView *agendaView, d->mAgendaViews) {
        agendaView->showDates(start, end);
    }
}

void MultiAgendaView::showIncidences(const Akonadi::Item::List &incidenceList, const QDate &date)
{
    foreach (AgendaView *agendaView, d->mAgendaViews) {
        agendaView->showIncidences(incidenceList, date);
    }
}

void MultiAgendaView::updateView()
{
    recreateViews();
    foreach (AgendaView *agendaView, d->mAgendaViews) {
        agendaView->updateView();
    }
}

int MultiAgendaView::maxDatesHint() const
{
    // these maxDatesHint functions aren't used
    return AgendaView::MAX_DAY_COUNT;
}

void MultiAgendaView::slotSelectionChanged()
{
    foreach (AgendaView *agenda, d->mAgendaViews) {
        if (agenda != sender()) {
            agenda->clearSelection();
        }
    }
}

bool MultiAgendaView::eventDurationHint(QDateTime &startDt, QDateTime &endDt,
                                        bool &allDay) const
{
    foreach (AgendaView *agenda, d->mAgendaViews) {
        bool valid = agenda->eventDurationHint(startDt, endDt, allDay);
        if (valid) {
            return true;
        }
    }
    return false;
}

void MultiAgendaView::slotClearTimeSpanSelection()
{
    foreach (AgendaView *agenda, d->mAgendaViews) {
        if (agenda != sender()) {
            agenda->clearTimeSpanSelection();
        } else {
            setCollectionId(agenda->collectionId());
        }
    }
}

AgendaView *MultiAgendaView::Private::createView(const QString &title)
{
    QWidget *box = new QWidget(mTopBox);
    mTopBox->layout()->addWidget(box);
    QVBoxLayout *layout = new QVBoxLayout(box);
    layout->setMargin(0);
    layout->addWidget(new ElidedLabel(title));
    AgendaView *av = new AgendaView(q->preferences(),
                                    q->startDateTime().date(),
                                    q->endDateTime().date(),
                                    true,
                                    true, q);
    layout->addWidget(av);
    av->setCalendar(q->calendar());
    av->setIncidenceChanger(q->changer());
    av->agenda()->scrollArea()->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    mAgendaViews.append(av);
    mAgendaWidgets.append(box);
    box->show();
    mTimeLabelsZone->setAgendaView(av);

    q->connect(mScrollBar, &QAbstractSlider::valueChanged,
               av->agenda()->verticalScrollBar(), &QAbstractSlider::setValue);

    q->connect(av->splitter(), &QSplitter::splitterMoved,
               q, &MultiAgendaView::resizeSplitters);
    q->connect(av, &AgendaView::showIncidencePopupSignal,
               q, &MultiAgendaView::showIncidencePopupSignal);

    q->connect(av, &AgendaView::showNewEventPopupSignal,
               q, &MultiAgendaView::showNewEventPopupSignal);

    const QSize minHint = av->allDayAgenda()->scrollArea()->minimumSizeHint();

    if (minHint.isValid()) {
        mLabel->setMinimumHeight(minHint.height());
        mRightDummyWidget->setMinimumHeight(minHint.height());
    }

    return av;
}

void MultiAgendaView::Private::addView(const Akonadi::Collection &collection)
{
    AgendaView *av = createView(CalendarSupport::displayName(q->calendar().data(), collection));
    av->setCollectionId(collection.id());
}

void MultiAgendaView::Private::addView(KCheckableProxyModel *sm, const QString &title)
{
    AgendaView *av = createView(title);
    av->setCustomCollectionSelectionProxyModel(sm);
}

void MultiAgendaView::resizeEvent(QResizeEvent *ev)
{
    d->resizeScrollView(ev->size());
    EventView::resizeEvent(ev);
}

void MultiAgendaView::Private::resizeScrollView(const QSize &size)
{
    const int widgetWidth = size.width() - mTimeLabelsZone->width() -
                            mScrollBar->width();

    int height = size.height();
    if (mScrollArea->horizontalScrollBar()->isVisible()) {
        const int sbHeight = mScrollArea->horizontalScrollBar()->height();
        height -= sbHeight;
        mLeftBottomSpacer->setFixedHeight(sbHeight);
        mRightBottomSpacer->setFixedHeight(sbHeight);
    } else {
        mLeftBottomSpacer->setFixedHeight(0);
        mRightBottomSpacer->setFixedHeight(0);
    }

    mTopBox->resize(widgetWidth, height);
}

void MultiAgendaView::setIncidenceChanger(Akonadi::IncidenceChanger *changer)
{
    EventView::setIncidenceChanger(changer);
    foreach (AgendaView *agenda, d->mAgendaViews) {
        agenda->setIncidenceChanger(changer);
    }
}

void MultiAgendaView::setPreferences(const PrefsPtr &prefs)
{
    foreach (AgendaView *agenda, d->mAgendaViews) {
        agenda->setPreferences(prefs);
    }
    EventView::setPreferences(prefs);
}

void MultiAgendaView::updateConfig()
{
    EventView::updateConfig();
    d->mTimeLabelsZone->setPreferences(preferences());
    d->mTimeLabelsZone->updateAll();
    foreach (AgendaView *agenda, d->mAgendaViews) {
        agenda->updateConfig();
    }
}

void MultiAgendaView::resizeSplitters()
{
    if (d->mAgendaViews.isEmpty()) {
        return;
    }

    QSplitter *lastMovedSplitter = qobject_cast<QSplitter *>(sender());
    if (!lastMovedSplitter) {
        lastMovedSplitter = d->mLeftSplitter;
    }
    foreach (AgendaView *agenda, d->mAgendaViews) {
        if (agenda->splitter() == lastMovedSplitter) {
            continue;
        }
        agenda->splitter()->setSizes(lastMovedSplitter->sizes());
    }
    if (lastMovedSplitter != d->mLeftSplitter) {
        d->mLeftSplitter->setSizes(lastMovedSplitter->sizes());
    }
    if (lastMovedSplitter != d->mRightSplitter) {
        d->mRightSplitter->setSizes(lastMovedSplitter->sizes());
    }
}

void MultiAgendaView::zoomView(const int delta, const QPoint &pos, const Qt::Orientation ori)
{
    const int hourSz = preferences()->hourSize();
    if (ori == Qt::Vertical) {
        if (delta > 0) {
            if (hourSz > 4) {
                preferences()->setHourSize(hourSz - 1);
            }
        } else {
            preferences()->setHourSize(hourSz + 1);
        }
    }

    foreach (AgendaView *agenda, d->mAgendaViews) {
        agenda->zoomView(delta, pos, ori);
    }

    d->mTimeLabelsZone->updateAll();
}

void MultiAgendaView::slotResizeScrollView()
{
    d->resizeScrollView(size());
}

void MultiAgendaView::showEvent(QShowEvent *event)
{
    EventView::showEvent(event);
    if (d->mUpdateOnShow) {
        d->mUpdateOnShow = false;
        d->mPendingChanges = true; // force a full view recreation
        showDates(d->mStartDate, d->mEndDate);
    }
}

void MultiAgendaView::setChanges(Changes changes)
{
    EventView::setChanges(changes);
    foreach (AgendaView *agenda, d->mAgendaViews) {
        agenda->setChanges(changes);
    }
}

void MultiAgendaView::setupScrollBar()
{
    if (!d->mAgendaViews.isEmpty() && d->mAgendaViews.first()->agenda()) {
        QScrollBar *scrollBar = d->mAgendaViews.first()->agenda()->verticalScrollBar();
        d->mScrollBar->setMinimum(scrollBar->minimum());
        d->mScrollBar->setMaximum(scrollBar->maximum());
        d->mScrollBar->setSingleStep(scrollBar->singleStep());
        d->mScrollBar->setPageStep(scrollBar->pageStep());
        d->mScrollBar->setValue(scrollBar->value());
    }
}

void MultiAgendaView::collectionSelectionChanged()
{
    qCDebug(CALENDARVIEW_LOG);
    d->mPendingChanges = true;
    recreateViews();
}

bool MultiAgendaView::hasConfigurationDialog() const
{

    /** The wrapper in korg has the dialog. Too complicated to move to CalendarViews.
        Depends on korg/AkonadiCollectionView, and will be refactored some day
        to get rid of CollectionSelectionProxyModel/EntityStateSaver */
    return false;
}

void MultiAgendaView::doRestoreConfig(const KConfigGroup &configGroup)
{
    if (!calendar()) {
        qCCritical(CALENDARVIEW_LOG) << "Calendar is not set.";
        Q_ASSERT(false);
        return;
    }

    d->mCustomColumnSetupUsed = configGroup.readEntry("UseCustomColumnSetup", false);
    d->mCustomNumberOfColumns = configGroup.readEntry("CustomNumberOfColumns", 2);
    d->mCustomColumnTitles =  configGroup.readEntry("ColumnTitles", QStringList());
    if (d->mCustomColumnTitles.size() != d->mCustomNumberOfColumns) {
        const int orig = d->mCustomColumnTitles.size();
        d->mCustomColumnTitles.reserve(d->mCustomNumberOfColumns);
        for (int i = orig; i < d->mCustomNumberOfColumns; ++i) {
            d->mCustomColumnTitles.push_back(generateColumnLabel(i));
        }
    }

    QVector<KCheckableProxyModel *> oldModels = d->mCollectionSelectionModels;
    d->mCollectionSelectionModels.clear();

    if (d->mCustomColumnSetupUsed) {
        d->mCollectionSelectionModels.resize(d->mCustomNumberOfColumns);
        for (int i = 0; i < d->mCustomNumberOfColumns; ++i) {
            // Sort the calanders by name
            QSortFilterProxyModel *sortProxy = new QSortFilterProxyModel(this);
            sortProxy->setDynamicSortFilter(true);

            sortProxy->setSourceModel(calendar()->entityTreeModel());

            // Only show the first column
            KRearrangeColumnsProxyModel *columnFilterProxy = new KRearrangeColumnsProxyModel(this);
            columnFilterProxy->setSourceColumns(QVector<int>() << Akonadi::ETMCalendar::CollectionTitle);
            columnFilterProxy->setSourceModel(sortProxy);

            // Keep track of selection.
            QItemSelectionModel *qsm = new QItemSelectionModel(columnFilterProxy);

            // Make the model checkable.
            KCheckableProxyModel *checkableProxy = new KCheckableProxyModel(this);
            checkableProxy->setSourceModel(columnFilterProxy);
            checkableProxy->setSelectionModel(qsm);
            const QString groupName = configGroup.name() + QLatin1String("_subView_") + QString::number(i);
            const KConfigGroup group = configGroup.config()->group(groupName);

            if (!d->mSelectionSavers.contains(groupName)) {
                d->mSelectionSavers.insert(groupName, new KViewStateMaintainer<ETMViewStateSaver>(group));
                d->mSelectionSavers[groupName]->setSelectionModel(checkableProxy->selectionModel());
            }

            d->mSelectionSavers[groupName]->restoreState();
            d->mCollectionSelectionModels[i] = checkableProxy;
        }
    }

    d->mPendingChanges = true;
    recreateViews();
    qDeleteAll(oldModels);
}

void MultiAgendaView::doSaveConfig(KConfigGroup &configGroup)
{
    configGroup.writeEntry("UseCustomColumnSetup", d->mCustomColumnSetupUsed);
    configGroup.writeEntry("CustomNumberOfColumns", d->mCustomNumberOfColumns);
    configGroup.writeEntry("ColumnTitles", d->mCustomColumnTitles);
    int idx = 0;
    foreach (KCheckableProxyModel *checkableProxyModel, d->mCollectionSelectionModels) {
        const QString groupName = configGroup.name() + QLatin1String("_subView_") + QString::number(idx);
        KConfigGroup group = configGroup.config()->group(groupName);
        ++idx;
        //TODO never used ?
        KViewStateMaintainer<ETMViewStateSaver> saver(group);
        if (!d->mSelectionSavers.contains(groupName)) {
            d->mSelectionSavers.insert(groupName, new KViewStateMaintainer<ETMViewStateSaver>(group));
            d->mSelectionSavers[groupName]->setSelectionModel(checkableProxyModel->selectionModel());
        }
        d->mSelectionSavers[groupName]->saveState();
    }
}

void MultiAgendaView::customCollectionsChanged(ConfigDialogInterface *dlg)
{
    if (!d->mCustomColumnSetupUsed && !dlg->useCustomColumns()) {
        // Config didn't change, no need to recreate views
        return;
    }

    d->mCustomColumnSetupUsed = dlg->useCustomColumns();
    d->mCustomNumberOfColumns = dlg->numberOfColumns();
    QVector<KCheckableProxyModel *> newModels;
    newModels.resize(d->mCustomNumberOfColumns);
    d->mCustomColumnTitles.clear();
    d->mCustomColumnTitles.reserve(d->mCustomNumberOfColumns);
    for (int i = 0; i < d->mCustomNumberOfColumns; ++i) {
        newModels[i] = dlg->takeSelectionModel(i);
        d->mCustomColumnTitles.push_back(dlg->columnTitle(i));
    }
    d->mCollectionSelectionModels = newModels;
    d->mPendingChanges = true;
    recreateViews();
}

bool MultiAgendaView::customColumnSetupUsed() const
{
    return d->mCustomColumnSetupUsed;
}

int MultiAgendaView::customNumberOfColumns() const
{
    return d->mCustomNumberOfColumns;
}

QVector<KCheckableProxyModel *> MultiAgendaView::collectionSelectionModels() const
{
    return d->mCollectionSelectionModels;
}

QStringList MultiAgendaView::customColumnTitles() const
{
    return d->mCustomColumnTitles;
}

void MultiAgendaView::Private::ElidedLabel::paintEvent(QPaintEvent *event)
{
    QFrame::paintEvent(event);
    QPainter p(this);
    QRect r = contentsRect();
    const QString elidedText = fontMetrics().elidedText(mText, Qt::ElideMiddle, r.width());
    p.drawText(r, Qt::AlignHCenter | Qt::AlignVCenter, elidedText);
}

QSize MultiAgendaView::Private::ElidedLabel::minimumSizeHint() const
{
    const QFontMetrics &fm = fontMetrics();
    return QSize(fm.width(QStringLiteral("...")), fm.height());
}
