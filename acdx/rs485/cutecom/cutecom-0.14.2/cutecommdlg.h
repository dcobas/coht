/****************************************************************************
** Form interface generated from reading ui file 'cutecommdlg.ui'
**
** Created: Fri Oct 10 11:26:11 2008
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.3.3   edited Nov 24 2003 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef CUTECOMMDLG_H
#define CUTECOMMDLG_H

#include <qvariant.h>
#include <qwidget.h>

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class QLabel;
class QPushButton;
class QComboBox;
class QCheckBox;
class QSplitter;
class QFrame;
class QTextBrowser;
class QLineEdit;
class QListBox;
class QListBoxItem;
class QSpinBox;

class CuteCommDlg : public QWidget
{
    Q_OBJECT

public:
    CuteCommDlg( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~CuteCommDlg();

    QLabel* textLabel5;
    QPushButton* m_quitPb;
    QLabel* textLabel3;
    QPushButton* m_aboutPb;
    QComboBox* m_dataBitsCb;
    QLabel* textLabel1;
    QPushButton* m_connectPb;
    QPushButton* m_closePb;
    QComboBox* m_stopCb;
    QLabel* textLabel2;
    QComboBox* m_deviceCb;
    QComboBox* m_baudCb;
    QLabel* textLabel4;
    QComboBox* m_parityCb;
    QCheckBox* m_hardwareCb;
    QCheckBox* m_softwareCb;
    QCheckBox* m_writeCb;
    QCheckBox* m_readCb;
    QLabel* textLabel1_3;
    QLabel* textLabel2_2;
    QCheckBox* m_applyCb;
    QSplitter* splitter2;
    QFrame* frame3;
    QTextBrowser* m_outputView;
    QPushButton* m_clearOutputPb;
    QCheckBox* m_hexOutputCb;
    QCheckBox* m_enableLoggingCb;
    QComboBox* m_logAppendCb;
    QLineEdit* m_logFileLe;
    QPushButton* m_logFileFileDialog;
    QFrame* frame4;
    QListBox* m_oldCmdsLb;
    QLabel* textLabel1_2;
    QLineEdit* m_cmdLe;
    QPushButton* m_sendPb;
    QComboBox* m_protoPb;
    QComboBox* m_inputModeCb;
    QLabel* textLabel1_4;
    QSpinBox* m_charDelaySb;

protected:
    QVBoxLayout* CuteCommDlgLayout;
    QGridLayout* layout11;
    QVBoxLayout* layout10;
    QHBoxLayout* layout9;
    QGridLayout* layout9_2;
    QVBoxLayout* frame3Layout;
    QHBoxLayout* layout10_2;
    QVBoxLayout* frame4Layout;
    QHBoxLayout* layout9_3;
    QHBoxLayout* layout10_3;
    QSpacerItem* spacer3;

protected slots:
    virtual void languageChange();

};

#endif // CUTECOMMDLG_H
