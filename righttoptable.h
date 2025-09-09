#ifndef RIGHTTOPTABLE_H
#define RIGHTTOPTABLE_H
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

class RightTopTable : public QTableWidget {
    const int fullPathRole = Qt::UserRole;
    Q_OBJECT
public:
    explicit RightTopTable(QWidget *parent = nullptr);

protected:
    void startDrag(Qt::DropActions supportedActions) override;

    void dragMoveEvent(QDragMoveEvent *event) override;
};
#endif // RIGHTTOPTABLE_H
