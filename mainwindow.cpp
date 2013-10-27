#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <stdlib.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    testInit();

    //set flag to 1 indicate get list
    flag = 1;
    //initInteractive(QStringList() << "-i getinfo");
    showProjectsList();

}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::initInteractive(QStringList args)
{
    pro = new QProcess(this);
    pro->start(QString("ffbackup-restore"),args);
    QObject::connect(pro, SIGNAL(readyReadStandardOutput()), this, SLOT(readStandardOutput()));
    QObject::connect(pro, SIGNAL(finished(int)), this, SLOT(finishedHandler(int)));
    if(!pro->waitForStarted())
        return false;
    else
        return true;
}

void MainWindow::readStandardOutput()
{
    if(flag == 1)
    {
    }
    else if(flag == 2)
    {
        QByteArray info = pro->readAllStandardOutput();
        int infoSize = info.size();
        for(int i = 0; i < infoSize; i++)
            bufferCopy.push_back(info[i]);
        int cutPos = bufferCopy.size() / 4 * 4;
        char digitStr[4];
        int loop = cutPos;
        for(int i = 3; i >= 0; i--)
        {
            digitStr[i] = bufferCopy.at(loop--);
        }
        bufferCopy.erase(bufferCopy.begin(), bufferCopy.begin() + cutPos);
        progressDialog->setValue(atoi(digitStr));
    }
    else return;
}

void MainWindow::finishedHandler(int exitCode)
{
    if(flag == 1)
    {
        if(exitCode == 0)
        {
            QByteArray info = pro->readAllStandardOutput();
            char listSizeStr[4];
            int start = 4, end = 4;
            QString everyInfo;
            QByteArray tmp;
            //1.get projects list
            for(int i = 0; i < 4; i++)
                listSizeStr[i] = info[i];
            uint32_t prjListSize = atoi(listSizeStr);
            for(uint32_t i = 0; i < prjListSize; i++)
            {
                while(info[end] != '\0')
                    end++;
                tmp = info.mid(start, end - start);
                everyInfo = tmp.data();
                end++;
                start = end;
                prjsList.push_back(everyInfo.toStdString());
            }

            //2.get detail list
            IDTIME tmpID;
            for(uint32_t i = 0; i < prjListSize; i++)
            {
                for(int loop = 0; loop < 4; loop++)
                    listSizeStr[loop] = info[loop];
                uint32_t detailListSize = atoi(listSizeStr);
                start += 4;
                for(uint32_t j = 0; j < detailListSize; j++)
                {
                    //2.1 backup_id
                    tmp = info.mid(start, 4);
                    start += 4;
                    everyInfo = tmp.data();
                    tmpID.backup_id = everyInfo.toInt();
                    tmp = info.mid(start, 4);
                    everyInfo = tmp.data();
                    tmpID.finished_time = everyInfo.toInt();
                    start += 4;
                    detailList.at(i).push_back(tmpID);
                }
            }
        }
        else
        {
            qDebug() << "Error exit";
            exit(0);
        }
    }
    else if(flag == 2)
    {
        if(exitCode == 0)
            progressDialog->close();
        else
        {
            qDebug() << "Error exit";
            exit(0);
        }
    }
    else
    {
    }
}

void MainWindow::testInit()
{
    QObject::connect(ui->projectsList, SIGNAL(currentTextChanged(QString)), this, SLOT(getSpecifiedPrj(QString)));
    srand((unsigned int)time(0));
    //1.init prjsList
    prjsList.push_back("Project 1th");
    prjsList.push_back("Project 2th");
    prjsList.push_back("Project 3th");
    prjsList.push_back("Project 4th");
    prjsList.push_back("Project 5th");
    prjsList.push_back("Project 6th");
    //2.init detailList
    vector<IDTIME> vectorTmp;
    IDTIME tmp;
    for(size_t j = 0; j < prjsList.size(); j++)
    {
        for(int i = 0; i < rand() % 16; i++)
        {
            tmp.backup_id = rand();
            tmp.finished_time = rand();
            vectorTmp.push_back(tmp);
        }
        detailList.push_back(vectorTmp);
        vectorTmp.clear();
    }
}


void MainWindow::showProjectsList()
{
    QStringList strList;
    size_t listSize = prjsList.size();
    size_t i = 0;
    for(i = 0; i < listSize; i++)
    {
        strList.append(QString(prjsList.at(i).c_str()));
    }
    if(!strList.empty())
    {
        ui->projectsList->clear();
        ui->projectsList->addItems(strList);
    }
}

void MainWindow::showDetailList(string prjName)
{
    QStringList strList;
    vector<IDTIME> specPrjDetail;
    size_t i = 0;
    for(i = 0; i < prjsList.size(); i++)
    {
        if(prjsList.at(i) == prjName)
            break;
    }
    if(i == prjsList.size())
    {
        //error
    }
    else
    {
        specPrjDetail = detailList.at(i);
        for(i = 0; i < specPrjDetail.size(); i++)
        {
            IDTIME tmp;
            tmp = specPrjDetail.at(i);
            strList.append(QString("BackupID:%1  FinishedTime:%2").arg(tmp.backup_id).arg(tmp.finished_time));
        }
        if(!strList.empty())
        {
            ui->detailList->clear();
            ui->detailList->addItems(strList);
        }
    }
}

void MainWindow::on_restoreButton_clicked()
{

    QString info = QString("-i restore -n %1 ").arg(ui->projectsList->currentItem()->text());
    QString tmp = ui->detailList->currentItem()->text();
    detailIndex = ui->detailList->currentRow();
    tmp.remove(QString("BackupID:"));
    tmp = tmp.section("  ",0,0);
    info.append(QString("-o %1").arg(tmp));

    flag = 2;
    //initInteractive(QStringList() << info);
    progressDialog = new QProgressDialog(this);
    progressDialog->setLabelText(QString("Restoring now..."));
    progressDialog->setRange(0, detailList.at(detailIndex).size());
    progressDialog->setModal(true);
    progressDialog->setCancelButtonText(QString("Cancel"));
    progressDialog->exec();

    QMessageBox::information(this, "selected info", info);
}

void MainWindow::getSpecifiedPrj(QString text)
{
    //QMessageBox::information(this, "selected info", ui->projectsList->currentItem()->text());
    showDetailList(text.toStdString());
}