#include "righttoptable.h"

#include <QTreeWidget>
#include <QDebug>
#include <QDropEvent>
#include <QMimeData>
#include <QFileInfo>
#include <QDir>
#include <QDrag>
#include <QString>
#include <QTableWidgetItem>
#include <QMetaType>


RightTopTable::RightTopTable(QWidget *parent) : QTableWidget(parent) {
    setAcceptDrops(true);
    setDragDropMode(QAbstractItemView::DragOnly);
    setDragEnabled(true);
}

void RightTopTable::startDrag(Qt::DropActions supportedActions)  {
    QMimeData* mimeData = new QMimeData;
    QString itemData = currentItem()->data(fullPathRole).toString();
    mimeData->setText(itemData);//записали ПУТЬ для передачи
    QDrag* qDrag = new QDrag(this);
    qDrag->setMimeData(mimeData);
    qDrag->exec();
}

void RightTopTable::dragMoveEvent(QDragMoveEvent *event)  {
    event->acceptProposedAction();
}

