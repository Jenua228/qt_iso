#include "lefttoptree.h"

#include <QTreeWidget>
#include <QDebug>
#include <QDropEvent>
#include <QMimeData>
#include <QFileInfo>
#include <QDir>
#include <QDrag>
#include <QString>
#include <QTableWidgetItem>


 LeftTopTree::LeftTopTree(QWidget *parent) : QTreeWidget(parent) {
    setAcceptDrops(true);
    setDragDropMode(QAbstractItemView::DragOnly);
    setDragEnabled(true);
}

void LeftTopTree::startDrag(Qt::DropActions supportedActions) {
    QMimeData* mimeData = new QMimeData;
    QString itemData = currentItem()->data(0,fullPathRole).toString();
    mimeData->setText(itemData);//записали ПУТЬ для передачи
    QDrag* qDrag = new QDrag(this);
    qDrag->setMimeData(mimeData);
    qDrag->exec();
}

void LeftTopTree::dragMoveEvent(QDragMoveEvent *event) {
            event->acceptProposedAction();
}
