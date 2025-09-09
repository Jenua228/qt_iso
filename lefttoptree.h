#ifndef LEFTTOPTREE_H
#define LEFTTOPTREE_H
#include <QTreeWidget>
#include <QDebug>
#include <QDropEvent>
#include <QMimeData>
#include <QFileInfo>
#include <QDir>
#include <QDrag>
#include <QString>
#include <QTableWidgetItem>


class LeftTopTree : public QTreeWidget {
    const int fullPathRole = Qt::UserRole;
    Q_OBJECT
public:
    explicit LeftTopTree(QWidget *parent = nullptr);
protected:
    void startDrag(Qt::DropActions supportedActions) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
};

#endif // LEFTTOPTREE_H
