#ifndef LEFTDOWNTREE_H
#define LEFTDOWNTREE_H
#include <QTreeWidget>
#include <QDebug>
#include <QDropEvent>
#include <QMimeData>
#include <QFileInfo>
#include <QDir>
#include <QDrag>
#include <QString>
#include <QUrl>
#include <QTreeWidgetItem>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QMainWindow>
#include <QMessageBox>
#include <QCursor>
#include <QGuiApplication>
#include <QPushButton>
#include <QListWidget>

namespace burn {
#include <libisofs/libisofs.h>
}
#include <libburn/libburn.h>

using namespace burn;

class LeftDownTree : public QTreeWidget {
    const int fullPathRole = Qt::UserRole;
    const int itemTypeRole = fullPathRole + 1;
    const int diskFilePathRole = fullPathRole + 2;
    const int fromWhichObject = fullPathRole + 3;
    const int fileSize = fullPathRole + 4;
    QTreeWidgetItem* previousHighlightedItem = nullptr;
    QMap<QTreeWidgetItem*, QPair<QBrush, QBrush>> originalStyles;
    QBrush originalBackground;
    QBrush originalForeground;
    Q_OBJECT

public:
    explicit LeftDownTree(QWidget *parent = nullptr);
    QString getFolderPathFromItem(QTreeWidgetItem *item);

protected:
    void clearHighlight();
    void resetItemAppearance(QTreeWidgetItem *item);
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    qint64 addFolderContentAndCount(const QString &path, QTreeWidgetItem *folderItem);
    qint64 countAfterDelete(QTreeWidgetItem *folderItem);
    void updateFolderPath(QTreeWidgetItem *item, const QString &newPath);
    bool isAncestor(QTreeWidgetItem *ancestor, QTreeWidgetItem *child);
    QTreeWidgetItem* findItemByPath(const QString& path);
    QTreeWidgetItem* findItemByPathRecursive(QTreeWidgetItem* item, const QString& path);
    void resizeEvent(QResizeEvent *event) override;
private:
    QPushButton *btnCreateFolder = new QPushButton(this);
    QPushButton *btnDeleteFolder = new QPushButton(this);
    QPushButton *btnEditFolder = new QPushButton(this);
    QPushButton *btnFormIso = new QPushButton(this);
    QListWidget *sizeList = new QListWidget(this);
    void onDeleteFolderClicked();
    void onEditFolderClicked();
    void onCreateFolderClicked();
    bool onFormIsoClicked();
    qint64 folderSizeBytes = 0;
    QString isoPath;
    bool processTreeItems(QTreeWidgetItem* parentItem, IsoDir* parentDir, IsoImage* image);
};

#endif // LEFTDOWNTREE_H
