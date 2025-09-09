#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "righttoptable.h"
#include "lefttoptree.h"
#include "leftdowntree.h"

#include <QMessageBox>
#include <QTreeWidgetItem>
#include <QDir>
#include <QFileInfo>
#include <QFileInfoList>
#include <QDateTime>
#include <QDebug>
#include <QDrag>
#include <QDropEvent>
#include <QMimeData>
#include <QTableWidget>
#include <QSplitter>
#include <QStyle>
#include <QInputDialog>
#include <QCursor>
#include <QPushButton>
#include <QListWidget>
const int fullPathRole = Qt::UserRole;
const int itemTypeRole = fullPathRole + 1;
//const int diskFilePathRole = fullPathRole + 2;
const int fromWhichObject = fullPathRole + 3;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QSplitter *mainSplitter = new QSplitter(Qt::Vertical, this);
    QSplitter *topSplitter = new QSplitter(Qt::Horizontal, this);

    QWidget *treeContainer = new QWidget();
    QVBoxLayout *treeLayout = new QVBoxLayout(treeContainer);

    treeLayout->addWidget(ui->treeWidgetNew);

    topSplitter->addWidget(ui->twg);
    topSplitter->addWidget(ui->tableContent);

    mainSplitter->addWidget(topSplitter);
    mainSplitter->addWidget(treeContainer);

    topSplitter->setChildrenCollapsible(false);
    mainSplitter->setChildrenCollapsible(false);

    setCentralWidget(mainSplitter);

    topSplitter->setSizes({width()/2, width()/2});
    mainSplitter->setSizes({height()*2/3, height()/3});

    qRegisterMetaType<QFileInfo>("QFileInfo");
    connect(ui->twg, &LeftTopTree::itemExpanded, this, &MainWindow::onItemExpanded);
    connect(ui->treeWidgetNew, &LeftDownTree::itemExpanded, this, &MainWindow::onItemExpanded);
    connect(ui->twg, &LeftTopTree::itemPressed, this, &MainWindow::onFolderSelected);


    LeftTopTree *twg = ui->twg;

    QStringList headers;
    headers << "Имя папки";
    twg->setHeaderLabels(headers);
    twg->setSortingEnabled(false);

    QString rootPath = "/";

    QTreeWidgetItem* rootItem = new QTreeWidgetItem(twg);
    rootItem->setText(0, rootPath);
    rootItem->setIcon(0, style()->standardIcon(QStyle::SP_DirIcon));
    rootItem->setExpanded(true);

    if (hasSubFolders(rootPath)) {
        addFolderContents(rootPath, rootItem);}

       LeftDownTree *treeWidgetNew = ui->treeWidgetNew;
       treeWidgetNew->setHeaderLabels(headers);
       treeWidgetNew->setSortingEnabled(false);

       QTreeWidgetItem* rootItemNew = new QTreeWidgetItem(treeWidgetNew);
       rootItemNew->setText(0, rootPath);
       //rootItemNew->setData(0, diskFilePathRole, "");
       rootItemNew->setIcon(0, style()->standardIcon(QStyle::SP_DirIcon));
       rootItemNew->setData(0, itemTypeRole, "Folder");
       rootItemNew->setData(0, fullPathRole, "/");
       rootItem->setData(0, fromWhichObject, "treeWidgetNew");

       RightTopTable *tableWidget = ui->tableContent;
       QStringList tableHeaders;
       tableHeaders << "Имя" << "Размер";
       tableWidget->setColumnCount(2);
       tableWidget->setHorizontalHeaderLabels(tableHeaders);
       tableWidget->setSortingEnabled(true);
       tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::addFolderContents(const QString &path, QTreeWidgetItem *parentItem)
{
    QDir dir(path);
    if (!dir.exists() && path!="[VIRTUAL]") {
        QMessageBox::warning(this, "Ошибка", "Папка не найдена: " + path);
        return;
    }

    if (parentItem->childCount() >= 1 && parentItem->child(0)->text(0) == "Загрузка...") {
        delete parentItem->takeChild(0);
    }

    QFileInfoList list = dir.entryInfoList(QDir::NoDotAndDotDot | QDir::Files | QDir::Dirs | QDir::NoSymLinks, QDir::DirsFirst | QDir::Name);//НЕЛЬЗЯ МЕНЯТЬ, НЕ ОТОБРАЖАЕТСЯ bin, либы и др.
    QGuiApplication::setOverrideCursor(Qt::WaitCursor);


    for (const QFileInfo &info : list) {
        // Проверка по подпапкам
        if (info.isDir()){
            QTreeWidgetItem* item = new QTreeWidgetItem(parentItem);
            item->setText(0, info.fileName());
            item->setData(0, fullPathRole, info.absoluteFilePath());
            item->setData(0, itemTypeRole, "Folder");
            //item->setData(0, diskFilePathRole, parentItem->data(0, diskFilePathRole).toString() + "/" + item->text(0));

            item->setIcon(0, style()->standardIcon(QStyle::SP_DirIcon));

            if (!hasSubFolders(info.absoluteFilePath())){
                item->setChildIndicatorPolicy(QTreeWidgetItem::DontShowIndicator);
            }

            QTreeWidgetItem* dummy = new QTreeWidgetItem();
            dummy->setText(0, "Загрузка...");

            item->addChild(dummy);
        }
    }


    QGuiApplication::setOverrideCursor(Qt::ArrowCursor);
}

void MainWindow::onItemExpanded(QTreeWidgetItem *item)
{

    if (item ->data(0, fromWhichObject)=="treeWidgetNew"){
        if (item->child(0)->text(0) == "Загрузка...") {
            delete item->takeChild(0);
        }
        return;
    }
    if (item->childCount() == 1 && item->child(0)->text(0) == "Загрузка...") {

        QString folderPath = getFolderPathFromItem(item);
        if (folderPath.isEmpty()) return;
        addFolderContents(folderPath, item);
    }
}

QString MainWindow::getFolderPathFromItem(QTreeWidgetItem *item)
{
    QVariant v = item->data(0, fullPathRole);
    if (v.isValid()) {
        return v.toString();
    }
    return QString();
}

bool MainWindow::hasSubFolders(const QString &path)
{
    QDir dir(path);
    if (!dir.exists()) return false;

    QFileInfoList list = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot  | QDir::NoSymLinks);
    return !list.isEmpty();
}

void MainWindow::onFolderSelected(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column);
    QString path = getFolderPathFromItem(item);
    if (path.isEmpty()) return;
    if (item->data(0, itemTypeRole)  == "Folder"){
        displayFolderContents(path, item);
    }
}

void MainWindow::displayFolderContents(const QString &folderPath, QTreeWidgetItem *item)
{
    QDir dir(folderPath);
    if (!dir.exists()) return;

    QFileInfoList list = dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot  | QDir::NoSymLinks);

    int row = 0;
    ui->tableContent->setRowCount(list.size());

    for (const QFileInfo &info : list) {
        QTableWidgetItem *item = new QTableWidgetItem(info.fileName());
        if (info.isDir())
        {
            item->setIcon(style()->standardIcon(QStyle::SP_DirIcon));
        }
        else{
            item->setIcon(style()->standardIcon(QStyle::SP_FileIcon));
        }
        item->setData(fullPathRole, info.absoluteFilePath());
        ui->tableContent->setItem(row, 0, item);
        QString sizeStr;
        if (info.isFile()) {
            double sizeMB = static_cast<double>(info.size()) / (1024 * 1024);
            sizeStr = QString::number(sizeMB, 'f', 2) + " MB";
        } else {
            sizeStr = "";
        }
        ui->tableContent->setItem(row, 1, new QTableWidgetItem(sizeStr));
        row++;
    }
}




