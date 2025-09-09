#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "righttoptable.h"
#include "lefttoptree.h"

#include <QMainWindow>
#include <QTreeWidgetItem>
#include <QGridLayout>
#include <QFileInfo>
#include <QTableWidgetItem>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public:
    void addFolderContents(const QString &path, QTreeWidgetItem *parentItem);
    bool hasSubFolders(const QString &path);
    bool hasSubFiles(const QString &path);

private slots:
    void onItemExpanded(QTreeWidgetItem *item);
    void onFolderSelected(QTreeWidgetItem *item, int column);
private:
    QString getFolderPathFromItem(QTreeWidgetItem *item);

private:
    Ui::MainWindow *ui;
    void displayFolderContents(const QString &folderPath, QTreeWidgetItem *item);
    QGridLayout* filesGridLayout;
    void clearFilesDisplay();
    void addFileToGrid(const QFileInfo& fileInfo, int row);

};
#endif // MAINWINDOW_H
