#include "leftdowntree.h"
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
#include <QInputDialog>
#include <libisofs/libisofs.h>
#include <QChar>
#include <stdio.h>
#include <QTextCodec>

using namespace burn;

LeftDownTree::LeftDownTree(QWidget *parent) : QTreeWidget(parent) {
    setAcceptDrops(true);
    setDragDropMode(QAbstractItemView::DragDrop);
    setDragEnabled(true);
    setDropIndicatorShown(true);
    setDefaultDropAction(Qt::MoveAction);

    connect(btnCreateFolder, &QPushButton::clicked, this, &LeftDownTree::onCreateFolderClicked);
    connect(btnDeleteFolder, &QPushButton::clicked, this, &LeftDownTree::onDeleteFolderClicked);
    connect(btnEditFolder, &QPushButton::clicked, this, &LeftDownTree::onEditFolderClicked);
    connect(btnFormIso, &QPushButton::clicked, this, &LeftDownTree::onFormIsoClicked);
}

QString LeftDownTree::getFolderPathFromItem(QTreeWidgetItem *item)
{
    QVariant v = item->data(0, fullPathRole);
    if (v.isValid()) {
        return v.toString();
    }
    return QString();
}

void LeftDownTree::clearHighlight() {
    if (previousHighlightedItem) {
                resetItemAppearance(previousHighlightedItem);
                previousHighlightedItem = nullptr;
            }
}

void LeftDownTree::resetItemAppearance(QTreeWidgetItem *item) {
        if (item) {
            item->setBackground(0, originalBackground);
            item->setForeground(0, originalForeground);
        }
    }

void LeftDownTree::dragEnterEvent(QDragEnterEvent *event) {
     clearHighlight();
     event->acceptProposedAction();
}

void LeftDownTree::dragLeaveEvent(QDragLeaveEvent *event) {
    clearHighlight();
    QTreeWidget::dragLeaveEvent(event);
}

void LeftDownTree::dragMoveEvent(QDragMoveEvent *event) {
    QTreeWidgetItem *currentItem = itemAt(event->pos());

    if (previousHighlightedItem && previousHighlightedItem != currentItem) {
        if (previousHighlightedItem && originalStyles.contains(previousHighlightedItem)) {
            previousHighlightedItem->setBackground(0, originalStyles[previousHighlightedItem].first);
            originalStyles.remove(previousHighlightedItem); // Удаляем из хранилища после восстановления
        }
    }
    if (currentItem) {
        if (!originalStyles.contains(currentItem)) {
                        originalStyles[currentItem] = qMakePair(currentItem->background(0),currentItem->foreground(0));
                    }

         currentItem->setBackground(0, QColor("#dedede"));
         currentItem->setForeground(0, Qt::black);

         previousHighlightedItem = currentItem;
        }
    event->acceptProposedAction();
}

void LeftDownTree::dropEvent(QDropEvent *event) {
    clearHighlight();
               // Если не попали ни на один элемент или это не корневой элемент
    QTreeWidgetItem *targetItem0 = itemAt(event->pos());
       if (!targetItem0) {
           QMessageBox::warning(this, "Ошибка", "Необходимо положить элемент в каталог");
           event->ignore();
           return;
       }

    const QMimeData *mimeData = event->mimeData();
    QString folderPath;
    folderPath = event->mimeData()->text();

    // Проверяем перетаскивание из другого виджета Qt
    bool isFromSameWidget = false;
    if (mimeData->hasFormat("application/x-qabstractitemmodeldatalist")) {

        QByteArray encoded = mimeData->data("application/x-qabstractitemmodeldatalist");
        QDataStream stream(&encoded, QIODevice::ReadOnly);

        int row, col;
        QMap<int, QVariant> roleDataMap;
        stream >> row >> col >> roleDataMap;

        if (roleDataMap.contains(fullPathRole)) {
            folderPath = roleDataMap[fullPathRole].toString();

       // Проверяем, существует ли такой элемент в текущем виджете
               if (findItemByPath(folderPath)) {
                   isFromSameWidget = true;
               }
        }
    }

    QFileInfo fi(folderPath);
    if (!fi.exists()) {
        QMessageBox::warning(this, "Ошибка", "Недействительная папка");
        event->ignore();
        return;
    }
    QTreeWidgetItem *folderItem = new QTreeWidgetItem();
    QPoint pos = event->pos();
    QTreeWidgetItem *targetItem = itemAt(pos);
    QTreeWidgetItem* draggedItem = findItemByPath(folderPath);

    if (targetItem){
    QString targetPath = getFolderPathFromItem(targetItem);
    if (targetItem->data(0,itemTypeRole) != "Folder"){
        QMessageBox::warning(this, "Ошибка", "Это не папка!");
        event->ignore();
        return;
    }

    QString sourcePath;

    // Проверка на перемещение в себя/потомка
    if (draggedItem && (isAncestor(draggedItem, targetItem))) {
        QMessageBox::warning(this, "Ошибка", "Нельзя перемещать папку в её подпапки!");
        event->ignore();
        return;
    }

    if (draggedItem && isFromSameWidget){
    QTreeWidgetItem *oldParent = draggedItem->parent();
    targetItem->setData(0, fromWhichObject, "treeWidgetNew");

    if (oldParent) {
        oldParent->removeChild(draggedItem);
    } else {
        takeTopLevelItem(indexOfTopLevelItem(draggedItem));
    }}

    targetItem->setExpanded(true);

    if (draggedItem && isFromSameWidget){

        if (targetItem){
        //    draggedItem->setData(0, diskFilePathRole, targetItem->data(0, diskFilePathRole).toString() + "/" + draggedItem->text(0));
            targetItem->addChild(draggedItem);
        }
        else{
            this->invisibleRootItem()->addChild(draggedItem);
      //      draggedItem->setData(0, diskFilePathRole, "/" + draggedItem->text(0));
        }
    //updateFolderPath(draggedItem, draggedItem->data(0, diskFilePathRole).toString());
    ///qDebug()<<"path"<<draggedItem->data(0, diskFilePathRole);
    }
}

    if (!isFromSameWidget) {
        folderItem->setText(0, fi.fileName());
        folderItem->setData(0, fullPathRole, fi.absoluteFilePath());
    if (fi.isDir()){
            //qint64 folderSizeBytes = addFolderContentAndCount(fi.absoluteFilePath(), folderItem);
            folderSizeBytes += addFolderContentAndCount(fi.absoluteFilePath(), folderItem);
            QGuiApplication::setOverrideCursor(Qt::ArrowCursor);
            //qDebug() << "abs" <<fi.absoluteFilePath();
            double folderSizeMB = static_cast<double>(folderSizeBytes) / (1024 * 1024);
            if (folderSizeMB>=1024){
                double folderSizeGB = static_cast<double>(folderSizeBytes) / (1024 * 1024 * 1024);
                this->sizeList->item(0)->setText("Размер диска: " + QString::number(folderSizeGB, 'f', 2) + "GB");
            }
            else{
//            qDebug() << "Размер папки" << fi.fileName() << ":"
//                     << QString::number(folderSizeMB, 'f', 2) << "MB ("<< folderSizeBytes << "байт)";
            this->sizeList->item(0)->setText("Размер диска: " + QString::number(folderSizeMB, 'f', 2) + "МВ");}

        folderItem->setIcon(0, style()->standardIcon(QStyle::SP_DirIcon));
        folderItem->setData(0, itemTypeRole, "Folder");
    }
    else{
        folderItem->setIcon(0, style()->standardIcon(QStyle::SP_FileIcon));
        folderItem->setData(0, itemTypeRole, fi.suffix());
    }
    if (targetItem){
        this->invisibleRootItem()->addChild(targetItem);
        //folderItem->setData(0, diskFilePathRole, targetItem->data(0, diskFilePathRole).toString() + "/" + folderItem->text(0));
        targetItem->addChild(folderItem);
    }
    else{
        this->invisibleRootItem()->addChild(folderItem);
        //folderItem->setData(0, diskFilePathRole, "/" + folderItem->text(0));
    }

    //updateFolderPath(folderItem, folderItem->data(0, diskFilePathRole).toString());
   // qDebug()<<"path"<<folderItem->data(0, diskFilePathRole);
    }

    QDir dir(folderPath);

    if (targetItem){
        targetItem->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
        }
    if (!targetItem) {
        event->ignore();
        return;
    }

    event->acceptProposedAction();
}

qint64 LeftDownTree::addFolderContentAndCount(const QString &path, QTreeWidgetItem *folderItem) {
    qint64 size = 0;
    QDir dir(path);

    // Получаем список всех файлов и папок
    QFileInfoList list = dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks);
    for (const QFileInfo &fileInfo : list) {
        QGuiApplication::setOverrideCursor(Qt::WaitCursor);
        QTreeWidgetItem *subFile = new QTreeWidgetItem(folderItem);
        subFile->setText(0, fileInfo.fileName());
        subFile->setData(0, fullPathRole, fileInfo.absoluteFilePath());

        //subFile->setData(0, diskFilePathRole, folderItem->data(0, diskFilePathRole).toString() + "/" + fileInfo.fileName());
        if (fileInfo.isFile()) {
            subFile->setData(0, itemTypeRole, fileInfo.suffix());
            subFile->setData(0, fileSize, fileInfo.size());
            subFile->setIcon(0, style()->standardIcon(QStyle::SP_FileIcon));
            size += fileInfo.size();

        } else if (fileInfo.isDir()) {
            subFile->setData(0, itemTypeRole, "Folder");
            subFile->setIcon(0, style()->standardIcon(QStyle::SP_DirIcon));
            size += addFolderContentAndCount(fileInfo.filePath(), subFile); // Рекурсия для подпапки
        }
    }
    return size;
}

qint64 LeftDownTree::countAfterDelete(QTreeWidgetItem *folderItem) {
    qint64 size = 0;
    for (int i = 0; i < folderItem->childCount(); i++){
         QGuiApplication::setOverrideCursor(Qt::WaitCursor);
         if (folderItem->child(i)->data(0, itemTypeRole) != "Folder"){
             size +=  (folderItem->child(i)->data(0, fileSize).toInt());
         }
         else{
             size += countAfterDelete(folderItem->child(i));
         }
    }
    return size;
}

//void MyTreeWidget2::updateFolderPath(QTreeWidgetItem *item, const QString &newPath) {
//    //item->setData(0, diskFilePathRole, newPath);
//    for (int i = 0; i < item->childCount(); ++i) {
//        QString childName = item->child(i)->text(0);
//        QString childPath = newPath + "/" + childName;
//        updateFolderPath(item->child(i), childPath);
//    }
//}

bool LeftDownTree::isAncestor(QTreeWidgetItem *ancestor, QTreeWidgetItem *child) {
    QTreeWidgetItem *parent = child->parent();
    while (parent) {
        if (parent == ancestor)
            return true;
        parent = parent->parent();
    }
    return false;
}

QTreeWidgetItem* LeftDownTree::findItemByPath(const QString& path) {
    for (int i = 0; i < topLevelItemCount(); ++i) {
        QTreeWidgetItem* result = findItemByPathRecursive(topLevelItem(i), path);
        if (result) return result;
    }
    return nullptr;
}

QTreeWidgetItem* LeftDownTree::findItemByPathRecursive(QTreeWidgetItem* item, const QString& path) {
    if (item->data(0, fullPathRole).toString() == path) {
        return item;
    }
    for (int i = 0; i < item->childCount(); ++i) {
        QTreeWidgetItem* result = findItemByPathRecursive(item->child(i), path);
        if (result) return result;
    }
    return nullptr;
}

// QWidget interface
void LeftDownTree::resizeEvent(QResizeEvent *event)
{
    btnCreateFolder->resize(131, 31);
    btnCreateFolder->move(this->width()-btnCreateFolder->width()-20, 0);
    btnCreateFolder->setText("Создать папку");
    btnCreateFolder->setLayoutDirection(Qt::LayoutDirectionAuto);

    btnDeleteFolder->resize(131, 31);
    btnDeleteFolder->move(this->width()-2*btnDeleteFolder->width()-20, 0);
    btnDeleteFolder->setText("Удалить элемент");
    btnDeleteFolder->setLayoutDirection(Qt::LayoutDirectionAuto);

    btnEditFolder->resize(131, 31);
    btnEditFolder->move(this->width()-3*btnEditFolder->width()-20, 0);
    btnEditFolder->setText("Переименовать");
    btnEditFolder->setLayoutDirection(Qt::LayoutDirectionAuto);

    btnFormIso->resize(180, 31);
    btnFormIso->move(this->width()-btnFormIso->width()-20, 148);
    btnFormIso->setText("Сформировать iso образ");
    btnFormIso->setLayoutDirection(Qt::LayoutDirectionAuto);

    sizeList->resize(200, 31);
    sizeList->move(this->width()-sizeList->width()-btnFormIso->width()-20, 148);
    sizeList->addItem("Размер диска: 0 MB");
    sizeList->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    sizeList->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
}

void LeftDownTree::onCreateFolderClicked() {
    QTreeWidgetItem* currentItem = this->currentItem();
    if (!currentItem || currentItem->data(0, itemTypeRole).toString() != "Folder") {
        QMessageBox::warning(this, "Ошибка", "Выберите папку, в которой нужно создать новую папку");
        return;
    }
    bool ok;
    QString folderName = QInputDialog::getText(this, "Создать папку","Введите имя папки:",QLineEdit::Normal,"Новая папка",&ok);
    if (!ok || folderName.isEmpty()) return;
    //QString currentPath = currentItem->data(0, diskFilePathRole).toString();
    //qDebug()<<"cur" <<currentPath;
    //QString newFolderPath = currentPath + "/" + folderName;

    QTreeWidgetItem* newFolderItem = new QTreeWidgetItem();
    newFolderItem->setText(0, folderName);
    newFolderItem->setIcon(0, style()->standardIcon(QStyle::SP_DirIcon));
    newFolderItem->setData(0, fullPathRole, "[VIRTUAL]");
    newFolderItem->setData(0, itemTypeRole, "Folder");
    //newFolderItem->setData(0, diskFilePathRole, currentItem->data(0, diskFilePathRole).toString() + "/" + folderName);
    currentItem->setData(0, fromWhichObject, "treeWidgetNew");
    currentItem->setExpanded(true);
    currentItem->addChild(newFolderItem);

    QTreeWidgetItem* dummy = new QTreeWidgetItem();
    dummy->setText(0, "Загрузка...");
    newFolderItem->addChild(dummy);
}

void LeftDownTree::onDeleteFolderClicked()
{
    QTreeWidgetItem* currentItem = this->currentItem();
    if (!currentItem) {
        QMessageBox::warning(this, "Ошибка", "Выберите элемент для удаления");
        return;
    }

    if (currentItem->parent() == nullptr && currentItem == this->topLevelItem(0)) {
        QMessageBox::warning(this, "Ошибка", "Нельзя удалить корневую папку");
        return;
    }

    int answer = QMessageBox::question(this, "Подтверждение",
                                     "Удалить элемент '" + currentItem->text(0) + "'?",
                                     QMessageBox::Yes | QMessageBox::No);

    if (answer == QMessageBox::Yes) {
        QTreeWidgetItem* parent = currentItem->parent();
        if (parent) {
            parent->removeChild(currentItem);
        } else {
            this->takeTopLevelItem(this->indexOfTopLevelItem(currentItem));
        }
        folderSizeBytes-=countAfterDelete(currentItem);
        QGuiApplication::setOverrideCursor(Qt::ArrowCursor);
        double folderSizeMB = static_cast<double>(folderSizeBytes) / (1024 * 1024);
        if (folderSizeMB>=1024){
            double folderSizeGB = static_cast<double>(folderSizeBytes) / (1024 * 1024 * 1024);
            this->sizeList->item(0)->setText("Размер диска: " + QString::number(folderSizeGB, 'f', 2) + "GB");
        }
        else{
//        qDebug() << "Размер папки" << fi.fileName() << ":"
//                 << QString::number(folderSizeMB, 'f', 2) << "MB ("<< folderSizeBytes << "байт)";
        this->sizeList->item(0)->setText("Размер диска: " + QString::number(folderSizeMB, 'f', 2) + "МВ");}

        delete currentItem;
    }

}

void LeftDownTree::onEditFolderClicked(){
    QTreeWidgetItem* currentItem = this->currentItem();
    if (!currentItem) {
        QMessageBox::warning(this, "Ошибка", "Выберите элемент для переименования");
        return;
    }

    if (currentItem->parent() == nullptr && currentItem == this->topLevelItem(0)) {
        QMessageBox::warning(this, "Ошибка", "Нельзя переименовать корневую папку");
        return;
    }

    bool ok;
    QString newFolderName = QInputDialog::getText(this, "Переименовать папку","Введите новое имя папки:",QLineEdit::Normal, currentItem->text(0) ,&ok);

    if (!ok || newFolderName.isEmpty()) return;

    int answer = QMessageBox::question(this, "Подтверждение",
                                     "Переименовать элемент '" + currentItem->text(0) + "'?",
                                     QMessageBox::Yes | QMessageBox::No);

 if (answer == QMessageBox::Yes) {
    for (int i = 0; i < currentItem->childCount(); ++i) {
        //QString oldPath;
        //oldPath = currentItem->child(i)->data(0, diskFilePathRole).toString();
        //QString newPath = oldPath.replace(currentItem->text(0), newFolderName); //ЗАМЕНА ПУТЕЙ ПОДПАПОК И ПОДФАЙЛОВ
        //currentItem->child(i)->setData(0, diskFilePathRole, newPath);
    }

    //QString oldPath2;
    //oldPath2 = currentItem->data(0, diskFilePathRole).toString();
    //QString newPath = oldPath2.replace(currentItem->text(0), newFolderName); //ЗАМЕНА ПУТи самой папки
    //currentItem->setData(0, diskFilePathRole, newPath);

        currentItem->setText(0, newFolderName);
    }
}

bool LeftDownTree::onFormIsoClicked(){
    int ret;
    IsoImage* image = nullptr;
    IsoDataSource* src = nullptr;
    IsoFileSource* fileSrc = nullptr;
    IsoWriteOpts *opts = nullptr;
    IsoReadImageFeatures *features = nullptr;
    IsoDir* rootDir = nullptr;
QGuiApplication::setOverrideCursor(Qt::WaitCursor);
    iso_init();

    // Создание нового образа
    ret = iso_image_new("volume_id", &image);

    if (ret < 0) {
        qDebug() << "Error creating image:" << iso_error_to_msg(ret);
        //return false;
    }

    iso_image_set_volset_id(image, "UTF-8");

    // Получаем корневой каталог образа
    rootDir = iso_image_get_root(image);

    // Рекурсивное добавление файлов из дерева
    QTreeWidgetItem* rootItem = this->topLevelItem(0);
    if (!processTreeItems(rootItem, rootDir, image)) {
        iso_image_unref(image);
        return false;
    }

    int op = iso_write_opts_new(&opts, 0);
    if (op < 0){
        qDebug()<<"Erro opts" << iso_error_to_msg(op);
        return false;
    }

    int jj = iso_write_opts_set_rockridge(opts, 1);

    if (jj < 0){
        qDebug()<<"Ebhghgh" << iso_error_to_msg(jj);
        return false;
    }

//    int kk = iso_write_opts_set_joliet(opts, 1);
//    if (kk < 0){
//        qDebug()<<"ggggggg" << iso_error_to_msg(kk);
//        return false;
//    }
    struct burn_source *burnSrc = nullptr;

    int ret2 = iso_image_create_burn_source(image, opts, &burnSrc);
    if (ret2 < 0 || burnSrc == nullptr){
        qDebug() << "Error creating burn:" << iso_error_to_msg(ret2);
        return false;
    }

    FILE *iso_file = fopen("output.iso", "wb");
    //qDebug() <<"ffffff";
    if (!iso_file){
        qDebug() << "Ошибка открытия output.iso";
    }
    unsigned char buffer[32 * 1024];
    int read_bytes = 0;

    while ((read_bytes = burnSrc->read_xt(burnSrc, buffer, sizeof(buffer)))) {

        if (read_bytes < 0)
        {
            burnSrc->cancel(burnSrc);
            break;
        }

        fwrite(buffer, 1, read_bytes, iso_file);
    }
    QGuiApplication::setOverrideCursor(Qt::ArrowCursor);

    return true;
}

bool LeftDownTree::processTreeItems(QTreeWidgetItem* parentItem, IsoDir* parentDir, IsoImage* image) {
    QTextCodec *codec = QTextCodec::codecForName("UTF-8");

    for (int i = 0; i < parentItem->childCount(); ++i) {
        qDebug()<<parentItem->child(i)->text(0);
        QTreeWidgetItem* item = parentItem->child(i);
        QString itemName = item->text(0);
        QString realPath = item->data(0, fullPathRole).toString();
        IsoDir* newDir = nullptr;

        if (item->data(0, itemTypeRole).toString() == "Folder") {
            // 1. Создаем узел директории
            int ret = iso_image_add_new_dir(image, parentDir, codec->fromUnicode(itemName).constData(), &newDir);

            if (ret < 0) {
                qDebug() << "Failed to add directory"<<iso_error_to_msg(ret);
                return false;
            }

                if (!processTreeItems(item, newDir, image)) {
                    return false;
                }
        }
        else {
            // Обработка файла
             const char* filePath =  codec->fromUnicode(item->data(0, fullPathRole).toString()).constData(); // Полный путь к файлу на диске
             IsoNode *node = nullptr;
             int ret = iso_tree_add_node(image, parentDir, filePath, &node);
             if (ret < 0){
                 qDebug()<<"Ошибка в создании файла"<<iso_error_to_msg(ret);
             }
        }
    }
    return true;
}
