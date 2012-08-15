/****************************************************************************
** Form implementation generated from reading ui file 'cutecommdlg.ui'
**
** Created: Fri Oct 10 11:26:18 2008
**      by: The User Interface Compiler ($Id: qt/main.cpp   3.3.3   edited Nov 24 2003 $)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#include "cutecommdlg.h"

#include <qvariant.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qsplitter.h>
#include <qframe.h>
#include <qtextbrowser.h>
#include <qlineedit.h>
#include <qlistbox.h>
#include <qspinbox.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qimage.h>
#include <qpixmap.h>

/*
 *  Constructs a CuteCommDlg as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 */
CuteCommDlg::CuteCommDlg( QWidget* parent, const char* name, WFlags fl )
    : QWidget( parent, name, fl )
{
    if ( !name )
	setName( "CuteCommDlg" );
    CuteCommDlgLayout = new QVBoxLayout( this, 11, 6, "CuteCommDlgLayout"); 

    layout11 = new QGridLayout( 0, 1, 1, 0, 6, "layout11"); 

    textLabel5 = new QLabel( this, "textLabel5" );

    layout11->addWidget( textLabel5, 3, 1 );

    m_quitPb = new QPushButton( this, "m_quitPb" );

    layout11->addWidget( m_quitPb, 3, 0 );

    textLabel3 = new QLabel( this, "textLabel3" );

    layout11->addWidget( textLabel3, 2, 1 );

    m_aboutPb = new QPushButton( this, "m_aboutPb" );

    layout11->addWidget( m_aboutPb, 2, 0 );

    m_dataBitsCb = new QComboBox( FALSE, this, "m_dataBitsCb" );

    layout11->addWidget( m_dataBitsCb, 2, 2 );

    textLabel1 = new QLabel( this, "textLabel1" );

    layout11->addWidget( textLabel1, 0, 1 );

    m_connectPb = new QPushButton( this, "m_connectPb" );

    layout11->addWidget( m_connectPb, 0, 0 );

    m_closePb = new QPushButton( this, "m_closePb" );

    layout11->addWidget( m_closePb, 1, 0 );

    m_stopCb = new QComboBox( FALSE, this, "m_stopCb" );

    layout11->addWidget( m_stopCb, 3, 2 );

    textLabel2 = new QLabel( this, "textLabel2" );

    layout11->addWidget( textLabel2, 1, 1 );

    m_deviceCb = new QComboBox( FALSE, this, "m_deviceCb" );
    m_deviceCb->setEditable( TRUE );

    layout11->addWidget( m_deviceCb, 0, 2 );

    m_baudCb = new QComboBox( FALSE, this, "m_baudCb" );

    layout11->addWidget( m_baudCb, 1, 2 );

    layout10 = new QVBoxLayout( 0, 0, 6, "layout10"); 

    layout9 = new QHBoxLayout( 0, 0, 6, "layout9"); 

    textLabel4 = new QLabel( this, "textLabel4" );
    layout9->addWidget( textLabel4 );

    m_parityCb = new QComboBox( FALSE, this, "m_parityCb" );
    layout9->addWidget( m_parityCb );
    layout10->addLayout( layout9 );

    layout9_2 = new QGridLayout( 0, 1, 1, 0, 6, "layout9_2"); 

    m_hardwareCb = new QCheckBox( this, "m_hardwareCb" );

    layout9_2->addWidget( m_hardwareCb, 0, 2 );

    m_softwareCb = new QCheckBox( this, "m_softwareCb" );

    layout9_2->addWidget( m_softwareCb, 0, 1 );

    m_writeCb = new QCheckBox( this, "m_writeCb" );

    layout9_2->addWidget( m_writeCb, 1, 2 );

    m_readCb = new QCheckBox( this, "m_readCb" );

    layout9_2->addWidget( m_readCb, 1, 1 );

    textLabel1_3 = new QLabel( this, "textLabel1_3" );

    layout9_2->addWidget( textLabel1_3, 1, 0 );

    textLabel2_2 = new QLabel( this, "textLabel2_2" );

    layout9_2->addWidget( textLabel2_2, 0, 0 );
    layout10->addLayout( layout9_2 );

    m_applyCb = new QCheckBox( this, "m_applyCb" );
    layout10->addWidget( m_applyCb );

    layout11->addMultiCellLayout( layout10, 0, 3, 3, 3 );
    CuteCommDlgLayout->addLayout( layout11 );

    splitter2 = new QSplitter( this, "splitter2" );
    splitter2->setOrientation( QSplitter::Vertical );

    frame3 = new QFrame( splitter2, "frame3" );
    frame3->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)5, 0, 2, frame3->sizePolicy().hasHeightForWidth() ) );
    frame3->setFrameShape( QFrame::StyledPanel );
    frame3->setFrameShadow( QFrame::Raised );
    frame3Layout = new QVBoxLayout( frame3, 11, 6, "frame3Layout"); 

    m_outputView = new QTextBrowser( frame3, "m_outputView" );
    QFont m_outputView_font(  m_outputView->font() );
    m_outputView_font.setFamily( "Courier" );
    m_outputView->setFont( m_outputView_font ); 
    m_outputView->setTextFormat( QTextBrowser::PlainText );
    m_outputView->setWrapPolicy( QTextBrowser::Anywhere );
    frame3Layout->addWidget( m_outputView );

    layout10_2 = new QHBoxLayout( 0, 0, 6, "layout10_2"); 

    m_clearOutputPb = new QPushButton( frame3, "m_clearOutputPb" );
    layout10_2->addWidget( m_clearOutputPb );

    m_hexOutputCb = new QCheckBox( frame3, "m_hexOutputCb" );
    m_hexOutputCb->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)1, (QSizePolicy::SizeType)0, 1, 0, m_hexOutputCb->sizePolicy().hasHeightForWidth() ) );
    layout10_2->addWidget( m_hexOutputCb );

    m_enableLoggingCb = new QCheckBox( frame3, "m_enableLoggingCb" );
    layout10_2->addWidget( m_enableLoggingCb );

    m_logAppendCb = new QComboBox( FALSE, frame3, "m_logAppendCb" );
    layout10_2->addWidget( m_logAppendCb );

    m_logFileLe = new QLineEdit( frame3, "m_logFileLe" );
    m_logFileLe->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)7, (QSizePolicy::SizeType)0, 2, 0, m_logFileLe->sizePolicy().hasHeightForWidth() ) );
    layout10_2->addWidget( m_logFileLe );

    m_logFileFileDialog = new QPushButton( frame3, "m_logFileFileDialog" );
    layout10_2->addWidget( m_logFileFileDialog );
    frame3Layout->addLayout( layout10_2 );

    frame4 = new QFrame( splitter2, "frame4" );
    frame4->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)5, 0, 1, frame4->sizePolicy().hasHeightForWidth() ) );
    frame4->setFrameShape( QFrame::StyledPanel );
    frame4->setFrameShadow( QFrame::Raised );
    frame4Layout = new QVBoxLayout( frame4, 11, 6, "frame4Layout"); 

    m_oldCmdsLb = new QListBox( frame4, "m_oldCmdsLb" );
    m_oldCmdsLb->setFocusPolicy( QListBox::NoFocus );
    frame4Layout->addWidget( m_oldCmdsLb );

    layout9_3 = new QHBoxLayout( 0, 0, 6, "layout9_3"); 

    textLabel1_2 = new QLabel( frame4, "textLabel1_2" );
    layout9_3->addWidget( textLabel1_2 );

    m_cmdLe = new QLineEdit( frame4, "m_cmdLe" );
    layout9_3->addWidget( m_cmdLe );
    frame4Layout->addLayout( layout9_3 );

    layout10_3 = new QHBoxLayout( 0, 0, 6, "layout10_3"); 

    m_sendPb = new QPushButton( frame4, "m_sendPb" );
    m_sendPb->setAutoDefault( FALSE );
    layout10_3->addWidget( m_sendPb );

    m_protoPb = new QComboBox( FALSE, frame4, "m_protoPb" );
    layout10_3->addWidget( m_protoPb );
    spacer3 = new QSpacerItem( 40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    layout10_3->addItem( spacer3 );

    m_inputModeCb = new QComboBox( FALSE, frame4, "m_inputModeCb" );
    layout10_3->addWidget( m_inputModeCb );

    textLabel1_4 = new QLabel( frame4, "textLabel1_4" );
    layout10_3->addWidget( textLabel1_4 );

    m_charDelaySb = new QSpinBox( frame4, "m_charDelaySb" );
    m_charDelaySb->setMaxValue( 250 );
    m_charDelaySb->setValue( 1 );
    layout10_3->addWidget( m_charDelaySb );
    frame4Layout->addLayout( layout10_3 );
    CuteCommDlgLayout->addWidget( splitter2 );
    languageChange();
    resize( QSize(756, 648).expandedTo(minimumSizeHint()) );
    clearWState( WState_Polished );

    // tab order
    setTabOrder( m_connectPb, m_aboutPb );
    setTabOrder( m_aboutPb, m_quitPb );
    setTabOrder( m_quitPb, m_deviceCb );
    setTabOrder( m_deviceCb, m_baudCb );
    setTabOrder( m_baudCb, m_dataBitsCb );
    setTabOrder( m_dataBitsCb, m_stopCb );
    setTabOrder( m_stopCb, m_parityCb );
    setTabOrder( m_parityCb, m_softwareCb );
    setTabOrder( m_softwareCb, m_hardwareCb );
    setTabOrder( m_hardwareCb, m_readCb );
    setTabOrder( m_readCb, m_writeCb );
    setTabOrder( m_writeCb, m_applyCb );
    setTabOrder( m_applyCb, m_outputView );
    setTabOrder( m_outputView, m_clearOutputPb );
    setTabOrder( m_clearOutputPb, m_hexOutputCb );
    setTabOrder( m_hexOutputCb, m_cmdLe );
    setTabOrder( m_cmdLe, m_inputModeCb );
    setTabOrder( m_inputModeCb, m_sendPb );
    setTabOrder( m_sendPb, m_protoPb );
    setTabOrder( m_protoPb, m_closePb );

    // buddies
    textLabel5->setBuddy( m_stopCb );
    textLabel3->setBuddy( m_dataBitsCb );
    textLabel1->setBuddy( m_deviceCb );
    textLabel2->setBuddy( m_baudCb );
    textLabel4->setBuddy( m_parityCb );
    textLabel1_2->setBuddy( m_cmdLe );
}

/*
 *  Destroys the object and frees any allocated resources
 */
CuteCommDlg::~CuteCommDlg()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void CuteCommDlg::languageChange()
{
    setCaption( tr( "CuteCom" ) );
    textLabel5->setText( tr( "Stop bits:" ) );
    m_quitPb->setText( tr( "&Quit" ) );
    m_quitPb->setAccel( QKeySequence( tr( "Alt+Q" ) ) );
    textLabel3->setText( tr( "Data bits:" ) );
    m_aboutPb->setText( tr( "&About" ) );
    m_aboutPb->setAccel( QKeySequence( tr( "Alt+A" ) ) );
    m_dataBitsCb->clear();
    m_dataBitsCb->insertItem( tr( "5" ) );
    m_dataBitsCb->insertItem( tr( "6" ) );
    m_dataBitsCb->insertItem( tr( "7" ) );
    m_dataBitsCb->insertItem( tr( "8" ) );
    m_dataBitsCb->setCurrentItem( 3 );
    textLabel1->setText( tr( "Device:" ) );
    m_connectPb->setText( tr( "&Open device" ) );
    m_connectPb->setAccel( QKeySequence( tr( "Alt+O" ) ) );
    QToolTip::add( m_connectPb, QString::null );
    m_closePb->setText( tr( "Cl&ose device" ) );
    m_closePb->setAccel( QKeySequence( tr( "Alt+O" ) ) );
    m_stopCb->clear();
    m_stopCb->insertItem( tr( "1" ) );
    m_stopCb->insertItem( tr( "2" ) );
    textLabel2->setText( tr( "Baud rate:" ) );
    m_baudCb->clear();
    m_baudCb->insertItem( tr( "600" ) );
    m_baudCb->insertItem( tr( "1200" ) );
    m_baudCb->insertItem( tr( "2400" ) );
    m_baudCb->insertItem( tr( "4800" ) );
    m_baudCb->insertItem( tr( "9600" ) );
    m_baudCb->insertItem( tr( "19200" ) );
    m_baudCb->insertItem( tr( "38400" ) );
    m_baudCb->insertItem( tr( "57600" ) );
    m_baudCb->insertItem( tr( "115200" ) );
    m_baudCb->insertItem( tr( "230400" ) );
    m_baudCb->insertItem( tr( "460800" ) );
    m_baudCb->insertItem( tr( "576000" ) );
    m_baudCb->insertItem( tr( "921600" ) );
    m_baudCb->setCurrentItem( 7 );
    textLabel4->setText( tr( "Parity:" ) );
    m_parityCb->clear();
    m_parityCb->insertItem( tr( "None" ) );
    m_parityCb->insertItem( tr( "Odd" ) );
    m_parityCb->insertItem( tr( "Even" ) );
    m_parityCb->insertItem( tr( "Mark" ) );
    m_parityCb->insertItem( tr( "Space" ) );
    m_hardwareCb->setText( tr( "Hardware" ) );
    QToolTip::add( m_hardwareCb, tr( "Enable Hardware Handshake" ) );
    m_softwareCb->setText( tr( "Software" ) );
    QToolTip::add( m_softwareCb, tr( "Enable Software Handshake (XON/XOFF)" ) );
    m_writeCb->setText( tr( "Writing" ) );
    QToolTip::add( m_writeCb, tr( "Usually you want to use read and write" ) );
    m_readCb->setText( tr( "Reading" ) );
    QToolTip::add( m_readCb, tr( "Usually you want to use read and write" ) );
    textLabel1_3->setText( tr( "Open for:" ) );
    textLabel2_2->setText( tr( "Handshake:" ) );
    m_applyCb->setText( tr( "Apply settings when opening" ) );
    m_clearOutputPb->setText( tr( "&Clear" ) );
    m_clearOutputPb->setAccel( QKeySequence( tr( "Alt+C" ) ) );
    QToolTip::add( m_clearOutputPb, tr( "Clear the output window" ) );
    m_hexOutputCb->setText( tr( "&Hex output" ) );
    m_hexOutputCb->setAccel( QKeySequence( tr( "Alt+H" ) ) );
    QToolTip::add( m_hexOutputCb, tr( "Show the output in hexadecimal format" ) );
    m_enableLoggingCb->setText( QString::null );
    QToolTip::add( m_enableLoggingCb, tr( "Enable logging the data to a file. Chose \"Append to\" to append instead overwrite to an existing file." ) );
    m_logAppendCb->clear();
    m_logAppendCb->insertItem( tr( "Log to:" ) );
    m_logAppendCb->insertItem( tr( "Append to:" ) );
    QToolTip::add( m_logAppendCb, tr( "Enable logging the data to a file. Chose \"Append to\" to append instead overwrite to an existing file." ) );
    QToolTip::add( m_logFileLe, tr( "The logfile" ) );
    m_logFileFileDialog->setText( tr( "..." ) );
    QToolTip::add( m_logFileFileDialog, tr( "Open file dialog" ) );
    QToolTip::add( m_oldCmdsLb, tr( "The input history" ) );
    textLabel1_2->setText( tr( "&Input:" ) );
    QToolTip::add( m_cmdLe, tr( "Enter commands here, Ctrl+C, Ctrl+S, Ctrl+Q also work" ) );
    m_sendPb->setText( tr( "Send file..." ) );
    QToolTip::add( m_sendPb, tr( "Select a file to be sent using the specified protocol" ) );
    m_protoPb->clear();
    m_protoPb->insertItem( tr( "Plain" ) );
    m_protoPb->insertItem( tr( "XModem" ) );
    m_protoPb->insertItem( tr( "ZModem" ) );
    m_protoPb->insertItem( tr( "YModem" ) );
    m_protoPb->insertItem( tr( "1kXModem" ) );
    m_protoPb->insertItem( tr( "Script" ) );
    QToolTip::add( m_protoPb, tr( "Select the transfer protocol" ) );
    m_inputModeCb->clear();
    m_inputModeCb->insertItem( tr( "LF line end" ) );
    m_inputModeCb->insertItem( tr( "CR line end" ) );
    m_inputModeCb->insertItem( tr( "CR,LF line end" ) );
    m_inputModeCb->insertItem( tr( "No line end" ) );
    m_inputModeCb->insertItem( tr( "Hex input" ) );
    QToolTip::add( m_inputModeCb, tr( "Select the line end termination" ) );
    textLabel1_4->setText( tr( "Char delay:" ) );
    m_charDelaySb->setSuffix( tr( " ms" ) );
    QToolTip::add( m_charDelaySb, tr( "Delay between single characters" ) );
}

