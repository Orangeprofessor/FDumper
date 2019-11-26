/********************************************************************************
** Form generated from reading UI file 'FDumperv2.ui'
**
** Created by: Qt User Interface Compiler version 5.12.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_FDUMPERV2_H
#define UI_FDUMPERV2_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_FDumperv2Class
{
public:
    QWidget *centralWidget;
    QListView *ImageListtxt;
    QTabWidget *galleryTabs;
    QWidget *tab;
    QWidget *tab_2;
    QWidget *tab_3;
    QListView *queueList;
    QGroupBox *groupBox;
    QWidget *layoutWidget;
    QHBoxLayout *horizontalLayout;
    QLineEdit *queueLineInput;
    QPushButton *addToQueue;
    QGroupBox *groupBox_2;
    QGroupBox *groupBox_3;
    QGroupBox *groupBox_4;
    QWidget *layoutWidget1;
    QGridLayout *gridLayout;
    QCheckBox *ctfilterMain;
    QCheckBox *ctfilterScraps;
    QCheckBox *ctfilterFavs;
    QRadioButton *allRatings;
    QRadioButton *SFWonly;
    QRadioButton *NSFWonly;
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *FDumperv2Class)
    {
        if (FDumperv2Class->objectName().isEmpty())
            FDumperv2Class->setObjectName(QString::fromUtf8("FDumperv2Class"));
        FDumperv2Class->resize(689, 562);
        centralWidget = new QWidget(FDumperv2Class);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        ImageListtxt = new QListView(centralWidget);
        ImageListtxt->setObjectName(QString::fromUtf8("ImageListtxt"));
        ImageListtxt->setGeometry(QRect(10, 20, 306, 176));
        galleryTabs = new QTabWidget(centralWidget);
        galleryTabs->setObjectName(QString::fromUtf8("galleryTabs"));
        galleryTabs->setGeometry(QRect(345, 20, 331, 476));
        tab = new QWidget();
        tab->setObjectName(QString::fromUtf8("tab"));
        galleryTabs->addTab(tab, QString());
        tab_2 = new QWidget();
        tab_2->setObjectName(QString::fromUtf8("tab_2"));
        galleryTabs->addTab(tab_2, QString());
        tab_3 = new QWidget();
        tab_3->setObjectName(QString::fromUtf8("tab_3"));
        galleryTabs->addTab(tab_3, QString());
        queueList = new QListView(centralWidget);
        queueList->setObjectName(QString::fromUtf8("queueList"));
        queueList->setGeometry(QRect(10, 365, 306, 131));
        groupBox = new QGroupBox(centralWidget);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        groupBox->setGeometry(QRect(5, 315, 321, 191));
        layoutWidget = new QWidget(groupBox);
        layoutWidget->setObjectName(QString::fromUtf8("layoutWidget"));
        layoutWidget->setGeometry(QRect(5, 25, 306, 25));
        horizontalLayout = new QHBoxLayout(layoutWidget);
        horizontalLayout->setSpacing(6);
        horizontalLayout->setContentsMargins(11, 11, 11, 11);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
        queueLineInput = new QLineEdit(layoutWidget);
        queueLineInput->setObjectName(QString::fromUtf8("queueLineInput"));

        horizontalLayout->addWidget(queueLineInput);

        addToQueue = new QPushButton(layoutWidget);
        addToQueue->setObjectName(QString::fromUtf8("addToQueue"));

        horizontalLayout->addWidget(addToQueue);

        groupBox_2 = new QGroupBox(centralWidget);
        groupBox_2->setObjectName(QString::fromUtf8("groupBox_2"));
        groupBox_2->setGeometry(QRect(5, 210, 321, 101));
        groupBox_3 = new QGroupBox(centralWidget);
        groupBox_3->setObjectName(QString::fromUtf8("groupBox_3"));
        groupBox_3->setGeometry(QRect(5, 0, 321, 206));
        groupBox_4 = new QGroupBox(centralWidget);
        groupBox_4->setObjectName(QString::fromUtf8("groupBox_4"));
        groupBox_4->setGeometry(QRect(340, 0, 341, 506));
        layoutWidget1 = new QWidget(centralWidget);
        layoutWidget1->setObjectName(QString::fromUtf8("layoutWidget1"));
        layoutWidget1->setGeometry(QRect(10, 230, 306, 71));
        gridLayout = new QGridLayout(layoutWidget1);
        gridLayout->setSpacing(6);
        gridLayout->setContentsMargins(11, 11, 11, 11);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        gridLayout->setContentsMargins(0, 0, 0, 0);
        ctfilterMain = new QCheckBox(layoutWidget1);
        ctfilterMain->setObjectName(QString::fromUtf8("ctfilterMain"));
        ctfilterMain->setChecked(false);

        gridLayout->addWidget(ctfilterMain, 0, 0, 1, 1);

        ctfilterScraps = new QCheckBox(layoutWidget1);
        ctfilterScraps->setObjectName(QString::fromUtf8("ctfilterScraps"));

        gridLayout->addWidget(ctfilterScraps, 0, 1, 1, 1);

        ctfilterFavs = new QCheckBox(layoutWidget1);
        ctfilterFavs->setObjectName(QString::fromUtf8("ctfilterFavs"));

        gridLayout->addWidget(ctfilterFavs, 0, 2, 1, 1);

        allRatings = new QRadioButton(layoutWidget1);
        allRatings->setObjectName(QString::fromUtf8("allRatings"));

        gridLayout->addWidget(allRatings, 1, 0, 1, 1);

        SFWonly = new QRadioButton(layoutWidget1);
        SFWonly->setObjectName(QString::fromUtf8("SFWonly"));

        gridLayout->addWidget(SFWonly, 1, 1, 1, 1);

        NSFWonly = new QRadioButton(layoutWidget1);
        NSFWonly->setObjectName(QString::fromUtf8("NSFWonly"));

        gridLayout->addWidget(NSFWonly, 1, 2, 1, 1);

        ctfilterMain->raise();
        ctfilterScraps->raise();
        allRatings->raise();
        SFWonly->raise();
        NSFWonly->raise();
        ctfilterFavs->raise();
        FDumperv2Class->setCentralWidget(centralWidget);
        groupBox->raise();
        groupBox_2->raise();
        layoutWidget->raise();
        groupBox_4->raise();
        groupBox_3->raise();
        queueList->raise();
        ImageListtxt->raise();
        galleryTabs->raise();
        menuBar = new QMenuBar(FDumperv2Class);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 689, 21));
        FDumperv2Class->setMenuBar(menuBar);
        mainToolBar = new QToolBar(FDumperv2Class);
        mainToolBar->setObjectName(QString::fromUtf8("mainToolBar"));
        FDumperv2Class->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(FDumperv2Class);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        FDumperv2Class->setStatusBar(statusBar);

        retranslateUi(FDumperv2Class);

        galleryTabs->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(FDumperv2Class);
    } // setupUi

    void retranslateUi(QMainWindow *FDumperv2Class)
    {
        FDumperv2Class->setWindowTitle(QApplication::translate("FDumperv2Class", "FDumperv2", nullptr));
        galleryTabs->setTabText(galleryTabs->indexOf(tab), QApplication::translate("FDumperv2Class", "Tab 1", nullptr));
        galleryTabs->setTabText(galleryTabs->indexOf(tab_2), QApplication::translate("FDumperv2Class", "Tab 2", nullptr));
        galleryTabs->setTabText(galleryTabs->indexOf(tab_3), QApplication::translate("FDumperv2Class", "Page", nullptr));
        groupBox->setTitle(QApplication::translate("FDumperv2Class", "Download Queue", nullptr));
        addToQueue->setText(QApplication::translate("FDumperv2Class", "Queue", nullptr));
        groupBox_2->setTitle(QApplication::translate("FDumperv2Class", "Content Filtering", nullptr));
        groupBox_3->setTitle(QApplication::translate("FDumperv2Class", "Image List", nullptr));
        groupBox_4->setTitle(QApplication::translate("FDumperv2Class", "Preview Gallery", nullptr));
        ctfilterMain->setText(QApplication::translate("FDumperv2Class", "Main Gallery", nullptr));
        ctfilterScraps->setText(QApplication::translate("FDumperv2Class", "Scraps", nullptr));
        ctfilterFavs->setText(QApplication::translate("FDumperv2Class", "Favorites", nullptr));
        allRatings->setText(QApplication::translate("FDumperv2Class", "All Ratings", nullptr));
        SFWonly->setText(QApplication::translate("FDumperv2Class", "SFW Only", nullptr));
        NSFWonly->setText(QApplication::translate("FDumperv2Class", "NSFW Only", nullptr));
    } // retranslateUi

};

namespace Ui {
    class FDumperv2Class: public Ui_FDumperv2Class {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_FDUMPERV2_H
