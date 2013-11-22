#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtGui>
#include <stdint.h>
#include <string>
#include <vector>

using namespace std;

typedef struct IDTIME
{
    uint32_t backup_id;
    uint32_t finished_time;
}IDTIME;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void showProjectsList();
    void showDetailList(string prjName);
    void testInit();
    bool initInteractive(QStringList args);
private slots:
    void on_restoreButton_clicked();
    void getSpecifiedPrj(QString);
    void readStandardOutput();
    void finishedHandler(int exitCode);
private:
    Ui::MainWindow *ui;
    vector<string> prjsList;
    vector< vector<IDTIME> > detailList;
    vector<char> bufferCopy;
    QProcess *pro;
    QProgressDialog *progressDialog;
    bool firstReadStd;
    int detailIndex;
    int prjIndex;
    int flag;
};

#endif // MAINWINDOW_H
