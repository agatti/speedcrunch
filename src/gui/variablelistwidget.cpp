// This file is part of the SpeedCrunch project
// Copyright (C) 2009 Andreas Scherer <andreas_coder@freenet.de>
// Copyright (C) 2009, 2011, 2013 @heldercorreia
// Copyright (C) 2012 Roger Lamb <rlamb1id@gmail.com>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; see the file COPYING.  If not, write to
// the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
// Boston, MA 02110-1301, USA.

#include "gui/variablelistwidget.h"

#include "core/evaluator.h"
#include "core/settings.h"
#include "core/numberformatter.h"

#include <QEvent>
#include <QTimer>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QTreeWidget>
#include <QVBoxLayout>

static QString formatValue(const Quantity &value);

VariableListWidget::VariableListWidget(QWidget* parent)
    : QWidget(parent)
    , m_filterTimer(new QTimer(this))
    , m_variables(new QTreeWidget(this))
    , m_noMatchLabel(new QLabel(m_variables))
    , m_searchFilter(new QLineEdit(this))
    , m_searchLabel(new QLabel(this))
{
    m_filterTimer->setInterval(500);
    m_filterTimer->setSingleShot(true);

    m_variables->setAlternatingRowColors(true);
    m_variables->setAutoScroll(true);
    m_variables->setColumnCount(2);
    m_variables->setEditTriggers(QTreeWidget::NoEditTriggers);
    m_variables->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_variables->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    m_variables->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    m_variables->setRootIsDecorated(false);
    m_variables->setSelectionBehavior(QTreeWidget::SelectRows);
    m_variables->setCursor(QCursor(Qt::PointingHandCursor));

    m_noMatchLabel->setAlignment(Qt::AlignCenter);
    m_noMatchLabel->adjustSize();
    m_noMatchLabel->hide();

    QWidget* searchBox = new QWidget(this);
    QHBoxLayout* searchLayout = new QHBoxLayout;
    searchLayout->addWidget(m_searchLabel);
    searchLayout->addWidget(m_searchFilter);
    searchLayout->setContentsMargins(0, 0, 0, 0);
    searchBox->setLayout(searchLayout);

    QVBoxLayout* layout = new QVBoxLayout;
    layout->setContentsMargins(3, 3, 3, 3);
    layout->addWidget(searchBox);
    layout->addWidget(m_variables);
    setLayout(layout);

    QMenu* contextMenu = new QMenu(m_variables);
    m_insertAction = new QAction("", contextMenu);
    m_deleteAction = new QAction("", contextMenu);
    m_deleteAllAction = new QAction("", contextMenu);
    m_variables->setContextMenuPolicy(Qt::ActionsContextMenu);
    m_variables->addAction(m_insertAction);
    m_variables->addAction(m_deleteAction);
    m_variables->addAction(m_deleteAllAction);

    QWidget::setTabOrder(m_searchFilter, m_variables);
    setFocusProxy(m_searchFilter);

    retranslateText();

    connect(m_filterTimer, SIGNAL(timeout()), SLOT(updateList()));
    connect(m_searchFilter, SIGNAL(textChanged(const QString&)), SLOT(triggerFilter()));
    connect(m_variables, SIGNAL(itemActivated(QTreeWidgetItem*, int)), SLOT(activateItem()));
    connect(m_insertAction, SIGNAL(triggered()), SLOT(activateItem()));
    connect(m_deleteAction, SIGNAL(triggered()), SLOT(deleteItem()));
    connect(m_deleteAllAction, SIGNAL(triggered()), SLOT(deleteAllItems()));

    updateList();
}

VariableListWidget::~VariableListWidget()
{
    m_filterTimer->stop();
}

void VariableListWidget::updateList()
{
    setUpdatesEnabled(false);

    m_filterTimer->stop();
    m_variables->clear();
    QString term = m_searchFilter->text();
    QList<Variable> variables = Evaluator::instance()->getUserDefinedVariables();

    for (int i = 0; i < variables.count(); ++i) {
        QString varName = variables.at(i).identifier();

        QStringList namesAndValues;
        namesAndValues << varName << formatValue(variables.at(i).value());

        if (term.isEmpty()
            || namesAndValues.at(0).contains(term, Qt::CaseInsensitive)
            || namesAndValues.at(1).contains(term, Qt::CaseInsensitive))
        {
            QTreeWidgetItem* item = new QTreeWidgetItem(m_variables, namesAndValues);
            item->setTextAlignment(0, Qt::AlignLeft | Qt::AlignVCenter);
            item->setTextAlignment(1, Qt::AlignLeft | Qt::AlignVCenter);
        }
    }

    m_variables->resizeColumnToContents(0);
    m_variables->resizeColumnToContents(1);

    if (m_variables->topLevelItemCount() > 0) {
        m_noMatchLabel->hide();
        m_variables->sortItems(0, Qt::AscendingOrder);
    } else {
        m_noMatchLabel->setGeometry(m_variables->geometry());
        m_noMatchLabel->show();
        m_noMatchLabel->raise();
    }

    setUpdatesEnabled(true);
}

void VariableListWidget::retranslateText()
{
    QStringList titles;
    titles << tr("Name") << tr("Value");
    m_variables->setHeaderLabels(titles);

    m_searchLabel->setText(tr("Search"));
    m_noMatchLabel->setText(tr("No match found"));

    m_insertAction->setText(tr("Insert"));
    m_deleteAction->setText(tr("Delete"));
    m_deleteAllAction->setText(tr("Delete All"));

    QTimer::singleShot(0, this, SLOT(updateList()));
}

QTreeWidgetItem* VariableListWidget::currentItem() const
{
    return m_variables->currentItem();
}

void VariableListWidget::activateItem()
{
    if (!currentItem() || m_variables->selectedItems().isEmpty())
        return;
    emit variableSelected(currentItem()->text(0));
}

void VariableListWidget::deleteItem()
{
    if (!currentItem() || m_variables->selectedItems().isEmpty())
        return;
    Evaluator::instance()->unsetVariable(currentItem()->text(0));
    updateList();
}

void VariableListWidget::deleteAllItems()
{
    Evaluator::instance()->unsetAllUserDefinedVariables();
    updateList();
}

void VariableListWidget::triggerFilter()
{
    m_filterTimer->stop();
    m_filterTimer->start();
}

void VariableListWidget::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange) {
        setLayoutDirection(Qt::LeftToRight);
        retranslateText();
        return;
    }
    QWidget::changeEvent(event);
}

void VariableListWidget::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Delete) {
        deleteItem();
        event->accept();
        return;
    }
    QWidget::keyPressEvent(event);
}

static QString formatValue(const Quantity& value)
{
    return NumberFormatter::format(value);
}
