/*
    qtgui.cpp  Platform independent user interface


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2020-2022  W. Schwotzer

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/


#include "misc1.h"
#include "qtgui.h"
#include "inout.h"
#include "e2floppy.h"
#include "vico1.h"
#include "vico2.h"
#include "schedule.h"
#include "csetfreq.h"
#include "clogfile.h"
#include "mc6809.h"
#include "mc6809st.h"
#include "joystick.h"
#include "keyboard.h"
#include "terminal.h"
#include "pia1.h"
#include "e2screen.h"
#include "bmp.h"
#include "brkptui.h"
#include "logfilui.h"
#include "propsui.h"
#include "fcinfo.h"
#include "sodiff.h"
#include "foptman.h"
#include "fsetupui.h"
#include "warnoff.h"
#include <QString>
#include <QVector>
#include <QPixmap>
#include <QPainter>
#include <QIODevice>
#include <QTextStream>
#include <QWidget>
#include <QDesktopWidget>
#include <QToolBar>
#include <QMainWindow>
#include <QMessageBox>
#include <QUrl>
#include <QMenu>
#include <QMenuBar>
#include <QStatusBar>
#include <QApplication>
#include <QDesktopServices>
#include <QLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QStatusTipEvent>
#include <QStackedWidget>
#include <QComboBox>
#include <QStandardItemModel>
#include <QDate>
#include <QLocale>
#include <QEvent>
#include <QShowEvent>
#include <QKeyEvent>
#include <QResizeEvent>
#include <QCloseEvent>
#include <QThread>
#include <QHash>
#include <QFont>
#include <QFontDatabase>
#include "warnon.h"
#include <cmath>

int QtGui::preferencesTabIndex = 0;

QtGui::QtGui(
    Mc6809 &x_cpu,
    Memory &x_memory,
    Scheduler &x_scheduler,
    Inout  &x_inout,
    VideoControl1 &x_vico1,
    VideoControl2 &x_vico2,
    JoystickIO &x_joystickIO,
    KeyboardIO &x_keyboardIO,
    TerminalIO &x_terminalIO,
    Pia1 &x_pia1,
    struct sOptions &x_options)
        : AbstractGui(
               x_cpu
             , x_memory
             , x_inout
             , x_terminalIO)
        , e2screen(nullptr)
        , preferencesAction(nullptr)
        , isOriginalFrequency(false)
        , isStatusBarVisible(true)
        , isRunning(true)
        , isConfirmClose(true)
        , isForceScreenUpdate(true)
        , isRestartNeeded(false)
        , timerTicks(0)
        , oldFirstRasterLine(0)
        , scheduler(x_scheduler)
        , vico1(x_vico1)
        , vico2(x_vico2)
        , joystickIO(x_joystickIO)
        , keyboardIO(x_keyboardIO)
        , fdc(nullptr)
        , options(x_options)
        , oldOptions(x_options)
{
    logfileSettings.reset();

    setObjectName("flexemuMainWindow");

    colorTable = CreateColorTable();

    mainLayout = new QVBoxLayout(this);
    mainLayout->setObjectName(QString::fromUtf8("mainLayout"));
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    menuBar = new QMenuBar;
    mainLayout->addWidget(menuBar);
    toolBarLayout = new QHBoxLayout();
    toolBarLayout->setObjectName(QString::fromUtf8("toolBarLayout"));
    toolBarLayout->setSpacing(0);
    mainLayout->addLayout(toolBarLayout);
    e2screen = new E2Screen(x_scheduler, x_joystickIO, x_keyboardIO,
                            x_pia1, x_options, colorTable.first(), this);
    mainLayout->addWidget(e2screen, 1); //, Qt::AlignCenter);
    QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    e2screen->setSizePolicy(sizePolicy);
    e2screen->setFocusPolicy(Qt::StrongFocus);

    CreateIcons();
    CreateActions(*toolBarLayout);
    CreateStatusToolBar(*mainLayout);
    CreateStatusBar(*mainLayout);

    setWindowState(Qt::WindowActive);
    const auto flexemuIcon = QIcon(":/resource/flexemu.png");
    setWindowIcon(flexemuIcon);

    connect(&timer, &QTimer::timeout, this, &QtGui::OnTimer);
    timer.start(TIME_BASE / 1000);

    setLayout(mainLayout);

    setWindowTitle(e2screen->GetTitle());

    e2screen->ReleaseMouseCapture();

    // Initialize the non-modal CPU Dialog but don't open it.
    cpuDialog = new QDialog(this);
    cpuUi.setupUi(cpuDialog);
    cpuDialog->setWindowTitle("Mc6809");
    cpuDialog->setModal(false);
    cpuDialog->setSizeGripEnabled(true);
    cpuUi.b_run->setCheckable(true);
    cpuUi.b_stop->setCheckable(true);
    ConnectCpuUiSignalsWithSlots();

    SetCpuDialogMonospaceFont(QApplication::font().pointSize());
    UpdateCpuFrequencyCheck();
    UpdateCpuUndocumentedCheck();

    OnIconSize(0);
}

QtGui::~QtGui()
{
    delete cpuDialog;
}

void QtGui::SetFloppy(E2floppy *x_fdc)
{
    if (fdc == nullptr && x_fdc != nullptr)
    {
        fdc = x_fdc; // Only Eurocom II/V7 has a floppy controller.

        AddDiskStatusButtons();
    }
}

bool QtGui::HasFloppy() const
{
    return fdc != nullptr;
}

void QtGui::output_to_graphic()
{
    AbstractGui::output_to_graphic();
    if (!isVisible())
    {
        show();
    }
}

QToolBar *QtGui::CreateToolBar(QWidget *parent, const QString &title,
                               const QString &objectName)
{
    QToolBar *toolBar = new QToolBar(title, parent);
    toolBar->setObjectName(objectName);
    toolBar->setFloatable(false);
    toolBar->setMovable(false);
    toolBar->setIconSize({16, 16});
    QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    toolBar->setSizePolicy(sizePolicy);
    toolBarLayout->addWidget(toolBar);

    return toolBar;
}

void QtGui::OnExit()
{
    bool safeFlag = isRestartNeeded;

    isRestartNeeded = false;
    close();
    isRestartNeeded = safeFlag;
}

void QtGui::SetPreferencesStatusText(bool x_isRestartNeeded) const
{
    assert(preferencesAction != nullptr);

    const auto statusText = x_isRestartNeeded ?
        tr("Edit application preferences - Needs restart") :
        tr("Edit application preferences");

    preferencesAction->setStatusTip(statusText);
}

QIcon QtGui::GetPreferencesIcon(bool x_isRestartNeeded) const
{
    return x_isRestartNeeded ?
        QIcon(":/resource/preferences-needs-restart.png") :
        QIcon(":/resource/preferences.png");
}

void QtGui::OnPreferences()
{
    QDialog dialog;
    FlexemuOptionsUi ui;

    if (!isRestartNeeded)
    {
        oldOptions = options;
    }

    ui.setupUi(&dialog);
    const auto preferencesIcon = GetPreferencesIcon(isRestartNeeded);
    dialog.setWindowIcon(preferencesIcon);
    ui.TransferDataToDialog(options);
    ui.SetTabIndex(preferencesTabIndex);
    dialog.resize({0, 0});
    dialog.setModal(true);
    dialog.setSizeGripEnabled(true);
    auto result = dialog.exec();
    preferencesTabIndex = ui.GetTabIndex();

    if (result == QDialog::Accepted)
    {
        ui.TransferDataFromDialog(options);
        FlexemuOptionsDifference optionsDiff(options, oldOptions);

        isRestartNeeded = ::IsRestartNeeded(optionsDiff);
        const auto icon = GetPreferencesIcon(isRestartNeeded);
        preferencesAction->setIcon(icon);
        SetPreferencesStatusText(isRestartNeeded);

        if (isRestartNeeded)
        {
            if (close())
            {
                // Force restart flexemu
                QApplication::exit(EXIT_RESTART);
            }
            return;
        }

        // The following preferences need no restart.
        // They are directly taken over without user interaction.
        bool isWriteOptions = false;
        for (auto id : optionsDiff.GetNotEquals())
        {
            switch (id)
            {
                case FlexemuOptionId::Frequency:
                {
                    auto f = options.frequency;
                    auto command =
                        BCommandPtr(new CSetFrequency(scheduler, f));
                    scheduler.sync_exec(std::move(command));
                    UpdateCpuFrequencyCheck();
                    isWriteOptions = true;
                }
                    break;

                case FlexemuOptionId::IsUseUndocumented:
                    cpu.set_use_undocumented(options.use_undocumented);
                    UpdateCpuUndocumentedCheck();
                    isWriteOptions = true;
                    break;

                case FlexemuOptionId::Color:
                case FlexemuOptionId::NColors:
                case FlexemuOptionId::IsInverse:
                    colorTable = CreateColorTable();
                    e2screen->SetBackgroundColor(colorTable.first());
                    isForceScreenUpdate = true;
                    isWriteOptions = true;
                    break;

                case FlexemuOptionId::PixelSize:
                    OnScreenSize(options.pixelSize - 1);
                    break;

                case FlexemuOptionId::CanFormatDrive0:
                case FlexemuOptionId::CanFormatDrive1:
                case FlexemuOptionId::CanFormatDrive2:
                case FlexemuOptionId::CanFormatDrive3:
                case FlexemuOptionId::FileTimeAccess:
                    isWriteOptions = true;
                    break;

                case FlexemuOptionId::Drive0:
                case FlexemuOptionId::Drive1:
                case FlexemuOptionId::Drive2:
                case FlexemuOptionId::Drive3:
                case FlexemuOptionId::MdcrDrive0:
                case FlexemuOptionId::MdcrDrive1:
                case FlexemuOptionId::HexFile:
                case FlexemuOptionId::DiskDirectory:
                case FlexemuOptionId::IsRamExt2x96:
                case FlexemuOptionId::IsRamExt2x288:
                case FlexemuOptionId::IsFlexibleMmu:
                case FlexemuOptionId::IsEurocom2V5:
                case FlexemuOptionId::IsUseRtc:
                    break;
            }
        }

        if (isWriteOptions)
        {
            FlexemuOptions::WriteOptions(options, false, true);
        }
    }
    else
    {
        // User pressed cancel => Cancel any preference changes.
        if (isRestartNeeded)
        {
            isRestartNeeded = false;
            const auto icon = GetPreferencesIcon(isRestartNeeded);
            preferencesAction->setIcon(icon);
            SetPreferencesStatusText(isRestartNeeded);
            options = oldOptions;
        }
    }
}

void QtGui::OnRepaintScreen()
{
    e2screen->RepaintScreen();
}

void QtGui::OnFullScreen()
{
    ToggleFullScreenMode();
}

void QtGui::OnStatusBar()
{
    ToggleStatusBarVisibility();
}

void QtGui::OnSmoothDisplay()
{
    ToggleSmoothDisplay();
}

void QtGui::OnCpuRun()
{
    scheduler.request_new_state(CpuState::Run);
    isRunning = true;
    UpdateCpuRunStopCheck();
}

void QtGui::OnCpuStop()
{
    scheduler.request_new_state(CpuState::Stop);
    isRunning = false;
    UpdateCpuRunStopCheck();
}

void QtGui::OnCpuStep()
{
    scheduler.request_new_state(CpuState::Step);
    isRunning = false;
    UpdateCpuRunStopCheck();
}

void QtGui::OnCpuNext()
{
    scheduler.request_new_state(CpuState::Next);
    isRunning = false;
    UpdateCpuRunStopCheck();
}

void QtGui::OnCpuReset()
{
    scheduler.request_new_state(CpuState::Reset);
    isRunning = false;
    UpdateCpuRunStopCheck();
}

void QtGui::OnCpuResetRun()
{
    scheduler.request_new_state(CpuState::ResetRun);
    isRunning = true;
    UpdateCpuRunStopCheck();
}

void QtGui::OnCpuDialogToggle()
{
    assert(cpuDialog != nullptr);

    if (cpuViewAction->isChecked())
    {
        // Choose the right position for the CPU Dialog
        // either above or below the main window.
        auto position = pos();
        int x, y;

        x = frameGeometry().x() +
            (frameGeometry().width() / 2) -
             (cpuDialog->frameGeometry().width() / 2);
        y = frameGeometry().y() - cpuDialog->frameGeometry().height() - 1;
        if (y >= 0)
        {
            position = QPoint(x, y);
        }
        else
        {
            auto desktop = QApplication::desktop();

            y = frameGeometry().y() + frameGeometry().height() + 1;
            if (y + cpuDialog->frameGeometry().height() < desktop->height())
            {
                position = QPoint(x, y);
            }
            else
            {
                position = QPoint(x, 0);
            }
        }
        cpuDialog->move(position);
    }

    cpuDialog->setVisible(cpuViewAction->isChecked());
}

void QtGui::OnCpuDialogClose()
{
    cpuDialog->hide();
    cpuViewAction->setChecked(false);
}

void QtGui::ConnectCpuUiSignalsWithSlots()
{
    QObject::connect(cpuUi.b_close, &QAbstractButton::clicked,
            this, &QtGui::OnCpuDialogClose);

    QObject::connect(cpuUi.b_run, &QAbstractButton::clicked,
            this, &QtGui::OnCpuRun);
    QObject::connect(cpuUi.b_stop, &QAbstractButton::clicked,
            this, &QtGui::OnCpuStop);
    QObject::connect(cpuUi.b_step, &QAbstractButton::clicked,
            this, &QtGui::OnCpuStep);
    QObject::connect(cpuUi.b_next, &QAbstractButton::clicked,
            this, &QtGui::OnCpuNext);
    QObject::connect(cpuUi.b_reset, &QAbstractButton::clicked,
            this, &QtGui::OnCpuReset);
    QObject::connect(cpuUi.b_breakpoints, &QAbstractButton::clicked,
            this, &QtGui::OnCpuBreakpoints);
    QObject::connect(cpuUi.b_logfile, &QAbstractButton::clicked,
            this, &QtGui::OnCpuLogging);
}

void QtGui::OnCpuBreakpoints()
{
    BPArray breakpoints = {};
    auto *dialog = new QDialog;
    BreakpointSettingsUi ui;

    breakpoints[0] = cpu.get_bp(0);
    breakpoints[1] = cpu.get_bp(1);
    ui.setupUi(*dialog);
    ui.SetData(breakpoints);

    auto result = dialog->exec();

    if (result == QDialog::Accepted)
    {
        breakpoints = ui.GetData();
        for (int index = 0; index < 2; ++index)
        {
            if (breakpoints[index] <= 0xFFFF)
            {
                cpu.set_bp(index, static_cast<Word>(breakpoints[index]));
            }
            else
            {
                cpu.reset_bp(index);
            }
        }
    }
}

void QtGui::OnCpuLogging()
{
    auto *dialog = new QDialog;
    LogfileSettingsUi ui;
    ui.setupUi(*dialog);
    ui.SetData(logfileSettings);

    auto result = dialog->exec();
    if (result == QDialog::Accepted)
    {
        logfileSettings = ui.GetData();
        scheduler.sync_exec(BCommandPtr(new CSetLogFile(cpu, logfileSettings)));
    }
}

void QtGui::OnCpuOriginalFrequency()
{
    ToggleCpuFrequency();
}

void QtGui::OnCpuUndocumentedInstructions()
{
    ToggleCpuUndocumented();
}

void QtGui::OnIntroduction()
{
    QString documentationDir = options.doc_dir.c_str();
    auto url = CreateDocumentationUrl(documentationDir, "flexemu.htm");

    QDesktopServices::openUrl(url);
}

void QtGui::OnAbout()
{
    auto version = QString(VERSION);

    auto title = QString::asprintf(tr("About %s").toUtf8().data(), PROGRAMNAME);

    QMessageBox::about(this, title,
        tr("<b>%1 V%2</b><p>"
           "%1 is an MC6809 emulator running "
           "<a href=\"https://en.wikipedia.org/wiki/FLEX_(operating_system)\">"
           "FLEX operating system</a>.<p>"
           "%1 comes with "
           "ABSOLUTELY NO WARRANTY. This is free software, and You "
           "are welcome to redistribute it under certain conditions.<p>"
           "Please notice that this project was developed under the "
           "terms of the "
           "<a href=\"http://flexemu.neocities.org/copying.txt\">"
           "GNU GENERAL PUBLIC LICENCE V2</a>.<p><p>"
           "Have Fun!<p><p>"
           "Copyright (C) 1997-2022 "
           "<a href=\"mailto:wolfgang.schwotzer@gmx.net\">"
           "Wolfgang Schwotzer</a><p>"
           "<a href=\"http://flexemu.neocities.org\">"
           "http://flexemu.neocities.org</a>")
        .arg(PROGRAMNAME).arg(version));
}

void QtGui::OnTimer()
{
    scheduler.timer_elapsed();

    // check every 100 ms for
    // - CPU view update
    // - Disk status update
    // - interrupt info update
    if (++timerTicks % (100000 / TIME_BASE) == 0)
    {
        timerTicks = 0;
        Word t;
        // Check for disk status update every 200 ms
        static tInterruptStatus irqStat;
        tInterruptStatus newIrqStat;
        static bool isFirstTime = true;
        static bool lastState[INT_RESET + 1];
        bool bState;

        if (HasFloppy())
        {
            static DiskStatus status[4];
            DiskStatus newStatus[4];

            fdc->get_drive_status(newStatus);

            for (t = 0; t < 4; ++t)
            {
                if ((newStatus[t] != status[t]) || isFirstTime)
                {
                    UpdateDiskStatus(t, newStatus[t]);
                    status[t] = newStatus[t];
                }
            }
        }

        scheduler.get_interrupt_status(newIrqStat);

        auto target_frequency = scheduler.get_target_frequency();
        bool newIsOriginalFrequency = (target_frequency > 0.0f) &&
            std::fabs(target_frequency - ORIGINAL_FREQUENCY) < 0.01f;
        if (newIsOriginalFrequency != isOriginalFrequency)
        {
            isOriginalFrequency = newIsOriginalFrequency;
            UpdateCpuFrequencyCheck();
        }

        if (isFirstTime)
        {
            for (t = INT_IRQ; t <= INT_RESET; ++t)
            {
                lastState[t] = false;
                irqStat.count[t] = 0;
                UpdateInterruptStatus((tIrqType)t, false);
            }

            UpdateCpuUndocumentedCheck();

            isFirstTime = false;
        }
        else
        {
            for (t = INT_IRQ; t <= INT_RESET; ++t)
            {
                bState = (newIrqStat.count[t] != irqStat.count[t]);

                if (bState != lastState[t])
                {
                    UpdateInterruptStatus((tIrqType)t, bState);
                    lastState[t] = bState;
                }
            }
        }

        memcpy(&irqStat, &newIrqStat, sizeof(tInterruptStatus));
    }

    // check every 20 ms for
    // - Update CPU status
    // - Repaint screen
    // - Mouse update
    // - Check for shutdown
    if ((timerTicks % 2) == 1)
    {
        // check if CPU view has to be updated
        Mc6809CpuStatus *status = (Mc6809CpuStatus *)scheduler.get_status();
        if (status != nullptr)
        {
            isRunning = (status->state == CpuState::Run ||
                         status->state == CpuState::Next);

            UpdateCpuRunStopCheck();

            update_cpuview(*status);

            if (status->state == CpuState::Invalid)
            {
                auto message = QString::asprintf(tr("\
    Got invalid instruction\n\
    pc=%04x instr=%02x %02x %02x %02x\n\
    Processor stopped. To\n\
    continue press Reset button").toUtf8().data(),
                        status->pc,
                        status->instruction[0],
                        status->instruction[1],
                        status->instruction[2],
                        status->instruction[3]);
                PopupMessage(message);
            }
        }

        auto firstRasterLine = vico2.get_value();
        bool isRepaintScreen = false;

        if (firstRasterLine != oldFirstRasterLine)
        {
            isRepaintScreen = true;
            oldFirstRasterLine = firstRasterLine;
        }

        short display_block;

        // update graphic display (only if display memory has changed)
        for (display_block = 0; display_block < YBLOCKS; display_block++)
        {
            if (isForceScreenUpdate || memory.has_changed(display_block))
            {
                update_block(display_block);
                isRepaintScreen = true;
            }
        }

        if (isRepaintScreen)
        {
            e2screen->RepaintScreen();
        }
        isForceScreenUpdate = false;

        e2screen->UpdateMouse();

        // If scheduler is finished this window can be unconditionally closed.
        if (scheduler.is_finished())
        {
            isConfirmClose = false;
            close();
        }
    }
}

QString QtGui::GetScreenSizeStatusTip(int index) const
{
    switch (index)
    {
        case 0:
            return tr("Resize Screen to default size (512 x 256)");

        case 1:
            return tr("Resize Screen to double size (1024 x 512)");

        case 2:
            return tr("Resize Screen to triple size (1536 x 768)");

        case 3:
            return tr("Resize Screen to quadruple size (2048 x 1024)");

        case 4:
            return tr("Resize Screen to quintuple size (2560 x 1280)");

        case 6:
            return tr("Enter Fullscreen Mode");

        default:
            return "";
    }
}

void QtGui::OnScreenSizeHighlighted(int index) const
{
    auto message = GetScreenSizeStatusTip(index);

    SetStatusMessage(message);
}

void QtGui::OnIconSize(int index)
{
    int size = 16 + 8 * index;

    SetIconSize({size, size});
}

void QtGui::OnScreenSize(int index)
{
    if (index == screenSizeComboBox->count() - 1)
    {
        if (!IsFullScreenMode())
        {
            SetFullScreenMode(true);
        }
    }
    else
    {
        if (IsFullScreenMode())
        {
            SetFullScreenMode(false);
        }
        e2screen->ResizeToFactor(index + 1);
        AdjustSize();

        oldOptions.pixelSize = index + 1;
        options.pixelSize = index + 1;
        WriteOneOption(options, FlexemuOptionId::PixelSize);
    }

    UpdateScreenSizeCheck(index);
}

void QtGui::UpdateScreenSizeCheck(int index) const
{
    for (int actionIndex = 0; actionIndex < SCREEN_SIZES; ++actionIndex)
    {
        screenSizeAction[actionIndex]->setChecked(actionIndex == index);
    }
}

void QtGui::UpdateScreenSizeValue(int index) const
{
    if (screenSizeComboBox->currentIndex() != index)
    {
        screenSizeComboBox->disconnect();
        screenSizeComboBox->setCurrentIndex(index);
        ConnectScreenSizeComboBoxSignalSlots();
    }
}

void QtGui::CreateActions(QLayout &layout)
{
    CreateFileActions(layout);
    CreateEditActions(layout);
    CreateViewActions(layout);
    CreateCpuActions(layout);
    CreateHelpActions(layout);
    CreateHorizontalSpacer(layout);
}

void QtGui::CreateFileActions(QLayout& layout)
{
    auto *fileMenu = menuBar->addMenu(tr("&File"));
    fileToolBar =
        CreateToolBar(this, tr("File"), QStringLiteral("fileToolBar"));
    QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    fileToolBar->setSizePolicy(sizePolicy);
    layout.addWidget(fileToolBar);

    const auto exitIcon = QIcon(":/resource/exit.png");
    exitAction = fileMenu->addAction(exitIcon, tr("E&xit"));
    connect(exitAction, &QAction::triggered, this, &QtGui::OnExit);
    exitAction->setShortcut(QKeySequence(Qt::SHIFT + Qt::CTRL + Qt::Key_Q));
    exitAction->setStatusTip(tr("Exit the application"));
    fileToolBar->addAction(exitAction);
}

void QtGui::CreateEditActions(QLayout& layout)
{
    auto *editMenu = menuBar->addMenu(tr("&Edit"));
    editToolBar =
        CreateToolBar(this, tr("Edit"), QStringLiteral("editToolBar"));
    QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    editToolBar->setSizePolicy(sizePolicy);
    layout.addWidget(editToolBar);

    const auto preferencesIcon = GetPreferencesIcon(isRestartNeeded);
    preferencesAction =
        editMenu->addAction(preferencesIcon, tr("&Preferences"));
    SetPreferencesStatusText(isRestartNeeded);
    connect(preferencesAction, &QAction::triggered,
            this, &QtGui::OnPreferences);
    const auto keySequence = QKeySequence(Qt::SHIFT + Qt::CTRL + Qt::Key_P);
    preferencesAction->setShortcut(keySequence);
    editToolBar->addAction(preferencesAction);
}

void QtGui::CreateViewActions(QLayout& layout)
{
    auto *viewMenu = menuBar->addMenu(tr("&View"));
    viewToolBar =
        CreateToolBar(this, tr("View"), QStringLiteral("viewToolBar"));
    QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    viewToolBar->setSizePolicy(sizePolicy);
    layout.addWidget(viewToolBar);

    auto keySequenceFullScreen = QKeySequence(QKeySequence::FullScreen);
    auto sequences = QKeySequence::keyBindings(QKeySequence::FullScreen);
    if (sequences.isEmpty())
    {
        keySequenceFullScreen = QKeySequence(Qt::Key_F11);
    }
    const auto fullScreenIcon = QIcon(":/resource/screen-full.png");
    fullScreenAction = viewMenu->addAction(fullScreenIcon, tr("&Full Screen"));
    connect(fullScreenAction, &QAction::triggered, this, &QtGui::OnFullScreen);
    fullScreenAction->setCheckable(true);
    fullScreenAction->setShortcut(keySequenceFullScreen);
    fullScreenAction->setStatusTip(tr("Enter or exit Fullscreen mode"));

    statusBarAction = viewMenu->addAction(tr("&Status Bar"));
    connect(statusBarAction, &QAction::triggered, this, &QtGui::OnStatusBar);
    statusBarAction->setCheckable(true);
    statusBarAction->setStatusTip(tr("Show or hide Status Bar"));
    viewMenu->addSeparator();

    auto *iconSizeMenu = viewMenu->addMenu(tr("&Icon Size"));

    for (uint index = 0U; index < ICON_SIZES; ++index)
    {
        iconSizeAction[index] = CreateIconSizeAction(*iconSizeMenu, index);
    }

    auto *screenSizeMenu = viewMenu->addMenu(tr("&Screen Size"));
    screenSizeComboBox = new QComboBox();

    for (uint index = 0U; index < SCREEN_SIZES; ++index)
    {
        const auto iconPath =
            QString::asprintf(":/resource/screen%u.png", index + 1);
        const auto screenSizeIcon = QIcon(iconPath);

        screenSizeAction[index] =
            CreateScreenSizeAction(screenSizeIcon, *screenSizeMenu, index);
        auto text = QString::asprintf("x%u", index + 1);
        screenSizeComboBox->addItem(screenSizeIcon, text);
    }

    const auto screenFullIcon = QIcon(":/resource/screen-full.png");
    screenSizeComboBox->addItem(screenFullIcon, "");
    screenSizeComboBox->insertSeparator(screenSizeComboBox->count() - 1);
    // E2Screen is the only widget which gets the focus.
    screenSizeComboBox->setFocusPolicy(Qt::NoFocus);
    viewToolBar->addWidget(screenSizeComboBox);
    ConnectScreenSizeComboBoxSignalSlots();

    viewMenu->addMenu(screenSizeMenu);
    viewMenu->addSeparator();

    smoothAction = viewMenu->addAction(tr("&Smooth Display"));
    connect(smoothAction, &QAction::triggered, this, &QtGui::OnSmoothDisplay);
    smoothAction->setCheckable(true);
    smoothAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_F12));
    smoothAction->setStatusTip(tr("Enter or exit smooth display mode"));
    viewMenu->addAction(smoothAction);
}

void QtGui::CreateCpuActions(QLayout& layout)
{
    auto *cpuMenu = menuBar->addMenu(tr("&CPU"));
    cpuToolBar =
        CreateToolBar(this, tr("CPU"), QStringLiteral("cpuToolBar"));
    QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    cpuToolBar->setSizePolicy(sizePolicy);
    layout.addWidget(cpuToolBar);

    const auto runIcon = QIcon(":/resource/run.png");
    cpuRunAction = cpuMenu->addAction(runIcon, tr("&Run"));
    connect(cpuRunAction, &QAction::triggered, this, &QtGui::OnCpuRun);
    cpuRunAction->setCheckable(true);
    cpuRunAction->setStatusTip(tr("Continue CPU execution"));
    cpuToolBar->addAction(cpuRunAction);

    const auto stopIcon = QIcon(":/resource/stop.png");
    cpuStopAction = cpuMenu->addAction(stopIcon, tr("&Stop"));
    connect(cpuStopAction, &QAction::triggered, this, &QtGui::OnCpuStop);
    cpuStopAction->setCheckable(true);
    cpuStopAction->setStatusTip(tr("Stop CPU execution"));
    cpuToolBar->addAction(cpuStopAction);

    const auto resetIcon = QIcon(":/resource/reset.png");
    cpuResetAction = cpuMenu->addAction(resetIcon, tr("&Reset"));
    connect(cpuResetAction, &QAction::triggered, this, &QtGui::OnCpuResetRun);
    cpuResetAction->setStatusTip(tr("Reset and continue CPU execution"));
    cpuToolBar->addAction(cpuResetAction);

    cpuMenu->addSeparator();
    cpuToolBar->addSeparator();
    const auto viewIcon = QIcon(":/resource/cpu.png");
    cpuViewAction = cpuMenu->addAction(viewIcon, tr("&View..."));
    connect(cpuViewAction, &QAction::triggered,
        this, &QtGui::OnCpuDialogToggle);
    cpuViewAction->setStatusTip(tr("Open CPU status window"));
    cpuViewAction->setCheckable(true);
    cpuToolBar->addAction(cpuViewAction);

    const auto breakpointsIcon = QIcon(":/resource/breakpoints.png");
    auto text = tr("&Breakpoints...");
    breakpointsAction = cpuMenu->addAction(breakpointsIcon, text);
    connect(breakpointsAction, &QAction::triggered,
        this, &QtGui::OnCpuBreakpoints);
    breakpointsAction->setStatusTip(tr("Open breakpoint settings"));
    cpuToolBar->addAction(breakpointsAction);

    const auto loggingIcon = QIcon(":/resource/logging.png");
    loggingAction = cpuMenu->addAction(loggingIcon, tr("&Logging..."));
    connect(loggingAction, &QAction::triggered, this, &QtGui::OnCpuLogging);
    loggingAction->setStatusTip(tr("Open logging settings"));
    cpuToolBar->addAction(loggingAction);

    cpuMenu->addSeparator();
    cpuToolBar->addSeparator();
    const auto originalFrequencyIcon =
        QIcon(":/resource/original-frequency.png");
    text = tr("&Original Frequency");
    originalFrequencyAction = cpuMenu->addAction(originalFrequencyIcon, text);
    connect(originalFrequencyAction, &QAction::triggered,
        this, &QtGui::OnCpuOriginalFrequency);
    originalFrequencyAction->setCheckable(true);
    text = tr("Set original or maximum possible CPU frequency");
    originalFrequencyAction->setStatusTip(text);
    cpuToolBar->addAction(originalFrequencyAction);

    const auto undocumentedIcon = QIcon(":/resource/cpu-undocumented.png");
    text = tr("&Undocumented Instructions");
    undocumentedAction = cpuMenu->addAction(undocumentedIcon, text);
    connect(undocumentedAction, &QAction::triggered,
        this, &QtGui::OnCpuUndocumentedInstructions);
    undocumentedAction->setCheckable(true);
    undocumentedAction->setStatusTip(
            tr("Toggle support of undocumented CPU instructions"));
}

void QtGui::CreateHelpActions(QLayout& layout)
{
    auto *helpMenu = menuBar->addMenu(tr("&Help"));
    helpToolBar =
        CreateToolBar(this, tr("Help"), QStringLiteral("helpToolBar"));
    QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    helpToolBar->setSizePolicy(sizePolicy);
    layout.addWidget(helpToolBar);

    const auto introductionIcon = QIcon(":/resource/info.png");
    auto text = tr("&Introduction");
    introductionAction = helpMenu->addAction(introductionIcon, text);
    connect(introductionAction, &QAction::triggered,
        this, &QtGui::OnIntroduction);
    introductionAction->setStatusTip(
            tr("Open an introduction to this application"));
    helpToolBar->addAction(introductionAction);

    const auto aboutIcon = QIcon(":/resource/about.png");
    aboutAction = helpMenu->addAction(aboutIcon, tr("&About"));
    connect(aboutAction, &QAction::triggered, this, &QtGui::OnAbout);
    aboutAction->setStatusTip(tr("Show the application's about box"));

    const auto aboutQtIcon = QIcon(":/resource/qt.png");
    aboutQtAction = helpMenu->addAction(aboutQtIcon, tr("&About Qt"));
    connect(aboutQtAction, &QAction::triggered, qApp, &QApplication::aboutQt);
    aboutQtAction->setStatusTip(tr("Show the Qt library's about box"));
}

void QtGui::CreateHorizontalSpacer(QLayout &layout)
{
    auto *horizontalSpacer =
        new QSpacerItem(1, 10, QSizePolicy::Expanding, QSizePolicy::Minimum);
    layout.addItem(horizontalSpacer);
}

void QtGui::CreateStatusToolBar(QLayout &layout)
{
    statusToolBar =
        CreateToolBar(this, tr("Status"), QStringLiteral("statusToolBar"));
    layout.addWidget(statusToolBar);

    auto text = tr("Interrupts");
    interruptStatusAction = statusToolBar->addAction(iconInterrupt, text);
    connect(interruptStatusAction, &QAction::triggered,
        this, [this]() { OnCpuInterruptStatus(); });
    interruptStatusAction->setStatusTip(tr("Open interrupt status"));
}

void QtGui::AddDiskStatusButtons()
{
    if (HasFloppy())
    {
        assert(statusToolBar != nullptr);

        for (Word i = 0; i < 4; ++i)
        {
            diskStatusAction[i] =
                statusToolBar->addAction(iconNoFloppy, tr("Disk #%1").arg(i));
            connect(diskStatusAction[i], &QAction::triggered,
                this, [this, i=i]() { OnDiskStatus(i); });
            auto statusTip = tr("Open disk #%1 status").arg(i);
            diskStatusAction[i]->setStatusTip(statusTip);
        }
    }
}

QAction *QtGui::CreateIconSizeAction(QMenu &menu, uint index)
{
    static const QVector<QString> menuText{
        tr("&Small"),
        tr("&Medium"),
        tr("&Large"),
    };
    static const QVector<QString> toolTipText{
        tr("Show small size Icons"),
        tr("Show medium size Icons"),
        tr("Show large size Icons"),
    };

    assert(menuText.size() == ICON_SIZES);
    assert(toolTipText.size() == ICON_SIZES);

    auto *action = menu.addAction(menuText[index]);
    connect(action, &QAction::triggered,
        this, [&,index](){ OnIconSize(index); });
    action->setCheckable(true);
    action->setStatusTip(toolTipText[index]);

    return action;
}

QAction *QtGui::CreateScreenSizeAction(
        const QIcon &icon, QMenu &menu, uint index)
{
    static const QVector<QString> menuText{
        tr("&Default"),
        tr("D&ouble"),
        tr("Tr&iple"),
        tr("&Quadruple"),
        tr("Qu&intuple")
    };

    assert (index <= 4);

    auto keySequence = QKeySequence(Qt::CTRL + (Qt::Key_1 + index));
    auto *action = menu.addAction(icon, menuText[index]);
    connect(action, &QAction::triggered,
        this, [&,index](){ OnScreenSize(index); });
    action->setShortcut(keySequence);
    action->setCheckable(true);
    action->setStatusTip(GetScreenSizeStatusTip(index));

    return action;
}

void QtGui::CreateIcons()
{
    auto pixmap = QPixmap(":/resource/floppy.png");
    QPixmap resultPixmap(pixmap.width(), pixmap.height());
    resultPixmap.fill(Qt::transparent);
    iconNoFloppy = QIcon(resultPixmap);
    iconInactiveFloppy = QIcon(":/resource/floppy.png");

    // For active floppy disk overlay icon with 50% transparent red color.
    QPainter painter(&resultPixmap);
    painter.drawPixmap(0, 0, pixmap);
    auto brush = QBrush(QColor(255, 0, 0, 127));
    painter.fillRect(0, 0, pixmap.width() - 1, pixmap.height() - 1, brush);
    iconActiveFloppy = QIcon(resultPixmap);

    iconInterrupt = QIcon(":/resource/irq-gray.png");
    iconIrq = QIcon(":/resource/irq-yellow.png");
    iconFirq = QIcon(":/resource/irq-red.png");
    iconNmi = QIcon(":/resource/irq-magenta.png");
    iconReset = QIcon(":/resource/irq-lightblue.png");
}

void QtGui::OnCpuInterruptStatus()
{
    QString text;
    auto *dialog = new QDialog(this);
    Ui::Properties ui;
    tInterruptStatus status;

    scheduler.get_interrupt_status(status);

    ui.setupUi(dialog);

    QStandardItemModel model(4, 2);
    int row = 0;
    model.setItem(row++, 0, new QStandardItem(tr("IRQ")));
    model.setItem(row++, 0, new QStandardItem(tr("FIRQ")));
    model.setItem(row++, 0, new QStandardItem(tr("NMI")));
    model.setItem(row++, 0, new QStandardItem(tr("RESET")));
    row = 0;
    text = QString::asprintf("%u", status.count[INT_IRQ]);
    model.setItem(row++, 1, new QStandardItem(text));
    text = QString::asprintf("%u", status.count[INT_FIRQ]);
    model.setItem(row++, 1, new QStandardItem(text));
    text = QString::asprintf("%u", status.count[INT_NMI]);
    model.setItem(row++, 1, new QStandardItem(text));
    text = QString::asprintf("%u", status.count[INT_RESET]);
    model.setItem(row++, 1, new QStandardItem(text));

    dialog->setWindowTitle(tr("CPU Interrupt Status"));
    dialog->setModal(true);
    dialog->setSizeGripEnabled(true);
    auto floppyPixmap = QPixmap(":/resource/irqs.png");
    ui.SetPixmap(floppyPixmap);
    ui.SetModel(model, { "Interrupt", "Count" });
    ui.SetMinimumSize(dialog);

    dialog->exec();
}

void QtGui::OnDiskStatus(Word driveNumber)
{
    if (HasFloppy() && driveNumber < 4)
    {
        QString text;
        int tracks;
        int sectors;
        auto info = fdc->drive_info(driveNumber);
        QStandardItemModel model;
        auto *dialog = new QDialog(this);
        Ui::Properties ui;
        int row = 0;

        ui.setupUi(dialog);
        ui.SetDriveInfo(driveNumber, info);

        model.setColumnCount(2);
        if (info.IsValid())
        {
            model.setRowCount(info.GetIsFlexFormat() ? 9 : 6);
            info.GetTrackSector(tracks, sectors);
            model.setItem(row++, 0, new QStandardItem(tr("Drive")));
            model.setItem(row++, 0, new QStandardItem(tr("Type")));
            model.setItem(row++, 0, new QStandardItem(tr("Path")));
            if (info.GetIsFlexFormat())
            {
                model.setItem(row++, 0, new QStandardItem(tr("Name")));
                model.setItem(row++, 0, new QStandardItem(tr("Number")));
                model.setItem(row++, 0, new QStandardItem(tr("Date")));
            }
            model.setItem(row++, 0, new QStandardItem(tr("Tracks")));
            model.setItem(row++, 0, new QStandardItem(tr("Sectors")));
            model.setItem(row++, 0, new QStandardItem(tr("Write-protect")));
            model.setItem(row++, 0, new QStandardItem(tr("FLEX format")));
            row = 0;
            text = QString::asprintf("#%u", driveNumber);
            model.setItem(row++, 1, new QStandardItem(text));
            text = info.GetTypeString().c_str();
            model.setItem(row++, 1, new QStandardItem(text));
            model.setItem(row++, 1, new QStandardItem(info.GetPath().c_str()));
            if (info.GetIsFlexFormat())
            {
                text = info.GetName().c_str();
                model.setItem(row++, 1, new QStandardItem(text));
                text = QString::number(info.GetNumber());
                model.setItem(row++, 1, new QStandardItem(text));
                auto date = info.GetDate();
                auto qdate = QDate(
                    date.GetYear(), date.GetMonth(), date.GetDay());
                text = QLocale::system().toString(qdate, QLocale::ShortFormat);
                model.setItem(row++, 1, new QStandardItem(text));
            }
            model.setItem(row++, 1, new QStandardItem(QString::number(tracks)));
            model.setItem(row++, 1, new QStandardItem(QString::number(sectors)));
            text = info.GetIsWriteProtected() ? tr("yes") : tr("no");
            model.setItem(row++, 1, new QStandardItem(text));
            text = info.GetIsFlexFormat() ? tr("yes") : tr("no");
            model.setItem(row++, 1, new QStandardItem(text));
        }
        else
        {
            model.setRowCount(2);
            model.setItem(row++, 0, new QStandardItem(tr("Drive")));
            model.setItem(row++, 0, new QStandardItem(tr("Status")));
            row = 0;
            text = QString::asprintf("#%u", driveNumber);
            model.setItem(row++, 1, new QStandardItem(text));
            model.setItem(row++, 1, new QStandardItem("Not ready"));
        }

        dialog->setWindowTitle(tr("Floppy Disk Status"));
        dialog->setModal(true);
        dialog->setSizeGripEnabled(true);
        auto floppyPixmap = QPixmap(":/resource/floppy256.png");

        ui.SetPixmap(floppyPixmap);
        ui.SetModel(model, { "Property", "Value" });
        ui.SetMinimumSize(dialog);

        dialog->exec();
    }
}

void QtGui::CreateStatusBar(QLayout &layout)
{
    // Use QStackedWidget to be able to set a frame style.
    statusBarFrame = new QStackedWidget();
    statusBar = new QStatusBar();
    statusBar->setSizeGripEnabled(true);
    statusBarFrame->addWidget(statusBar);
    layout.addWidget(statusBarFrame);
    statusBarFrame->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    statusBarAction->setChecked(true);

    SetStatusMessage(tr("Ready"));
}

void QtGui::ConnectScreenSizeComboBoxSignalSlots() const
{
    connect(screenSizeComboBox,
#if (QT_VERSION <= QT_VERSION_CHECK(5, 7, 0))
            static_cast<void (QComboBox::*)(int)>(
                &QComboBox::currentIndexChanged),
#else
            QOverload<int>::of(&QComboBox::currentIndexChanged),       
#endif
            this, &QtGui::OnScreenSize);
    connect(screenSizeComboBox,
#if (QT_VERSION <= QT_VERSION_CHECK(5, 7, 0))
            static_cast<void (QComboBox::*)(int)>(
                &QComboBox::highlighted),
#else
            QOverload<int>::of(&QComboBox::highlighted),       
#endif
            this, &QtGui::OnScreenSizeHighlighted);
}

void QtGui::SetStatusMessage(const QString &message) const
{
    statusBar->showMessage(message, 4000);
}

// Optionally confirm to close this window.
bool QtGui::IsClosingConfirmed()
{
    if (isConfirmClose)
    {
        auto message = isRestartNeeded ?
            tr("Do you want to restart %s now?") :
            tr("Do you want to close %s?");
        message = QString::asprintf(message.toUtf8().data(), PROGRAMNAME);
        auto result = QMessageBox::question(this, tr("Flexemu"), message,
                          QMessageBox::Yes | QMessageBox::No);
        return (result == QMessageBox::Yes);
    }

    return true;
}

void QtGui::redraw_cpuview_impl(const Mc6809CpuStatus &status)
{
    assert(cpuDialog != nullptr);

    int i = status.s & 7;
    text(5 + 3 * i, 10, "[");
    text(8 + 3 * i, 10, "]");

    cpuUi.e_status->setText(cpustring);
}

void QtGui::PopupMessage(const QString &message)
{
    QMessageBox::warning(this, "flexemu error", message);
}

void QtGui::SetBell(int /*percent*/)
{
    QApplication::beep();
}

void QtGui::update_block(int blockNumber)
{
    assert(blockNumber >= 0 && blockNumber < YBLOCKS);

    memory.reset_changed(blockNumber);

    Byte const *src = nullptr;
    auto video_bank = vico1.get_value();
    if (memory.is_video_bank_valid(video_bank))
    {
        // copy block from video ram into device independant bitmap
        src = memory.get_video_ram(video_bank, blockNumber);
    }

    CopyToBMPArray(BLOCKHEIGHT, dataBuffer, src, colorTable);
    e2screen->UpdateBlock(vico2.get_value(), blockNumber, dataBuffer);
}

void QtGui::UpdateDiskStatus(int floppyIndex, DiskStatus status)
{
    if (HasFloppy())
    {
        assert(static_cast<size_t>(floppyIndex) <
                (sizeof(diskStatusAction) / sizeof(diskStatusAction[0])));

        switch (status)
        {
            case DiskStatus::EMPTY:
                diskStatusAction[floppyIndex]->setIcon(iconNoFloppy);
                break;

            case DiskStatus::INACTIVE:
                diskStatusAction[floppyIndex]->setIcon(iconInactiveFloppy);
                break;

            case DiskStatus::ACTIVE:
                diskStatusAction[floppyIndex]->setIcon(iconActiveFloppy);
                break;
        }
    }
}

void QtGui::UpdateInterruptStatus(tIrqType irqType, bool status)
{
    if (status)
    {
        switch (irqType)
        {
            case INT_IRQ:
                interruptStatusAction->setIcon(iconIrq);
                break;

            case INT_FIRQ:
                interruptStatusAction->setIcon(iconFirq);
                break;

            case INT_NMI:
                interruptStatusAction->setIcon(iconNmi);
                break;

            case INT_RESET:
                interruptStatusAction->setIcon(iconReset);
                break;
        }
    }
    else
    {
        interruptStatusAction->setIcon(iconInterrupt);
    }
}

void QtGui::ToggleCpuFrequency()
{
    isOriginalFrequency = !isOriginalFrequency;
    auto frequency = isOriginalFrequency ? ORIGINAL_FREQUENCY : 0.0f;
    options.frequency = frequency;
    oldOptions.frequency = frequency;
    scheduler.sync_exec(BCommandPtr(new CSetFrequency(scheduler, frequency)));
    UpdateCpuFrequencyCheck();
    WriteOneOption(options, FlexemuOptionId::Frequency);
}

void QtGui::ToggleCpuUndocumented()
{
    options.use_undocumented = !oldOptions.use_undocumented;
    oldOptions.use_undocumented = options.use_undocumented;
    cpu.set_use_undocumented(options.use_undocumented);

    UpdateCpuUndocumentedCheck();
    WriteOneOption(options, FlexemuOptionId::IsUseUndocumented);
}

void QtGui::ToggleCpuRunStop()
{
    if (isRunning)
    {
        OnCpuStop();
    }
    else
    {
        OnCpuRun();
    }
    isRunning = !isRunning;
    UpdateCpuRunStopCheck();
}

void QtGui::ToggleFullScreenMode()
{
    SetFullScreenMode(!IsFullScreenMode());
}

void QtGui::ToggleSmoothDisplay() const
{
    e2screen->ToggleSmoothDisplay();
    UpdateSmoothDisplayCheck();
    QTimer::singleShot(0, this, &QtGui::OnRepaintScreen);
}

void QtGui::SetFullScreenMode(bool isFullScreen)
{
    if (isFullScreen)
    {
        showFullScreen();
    }
    else
    {
        showNormal();
    }

    UpdateFullScreenCheck();
}

bool QtGui::IsFullScreenMode() const
{
    return (windowState() & Qt::WindowFullScreen) == Qt::WindowFullScreen;
}

void QtGui::ToggleStatusBarVisibility()
{
    auto statusBarHeightDiff = statusBarFrame->height();
    if (statusBarFrame->isVisible())
    {
        statusBarHeightDiff = -statusBarHeightDiff;
    }
    statusBarFrame->setVisible(!statusBarFrame->isVisible());
    resize(size() + QSize(0, statusBarHeightDiff));

    UpdateStatusBarCheck();
}

void QtGui::UpdateSmoothDisplayCheck() const
{
    smoothAction->setChecked(e2screen->IsSmoothDisplay());
}

void QtGui::UpdateCpuFrequencyCheck() const
{
    originalFrequencyAction->setChecked(isOriginalFrequency);
}

void QtGui::UpdateCpuRunStopCheck() const
{
    cpuRunAction->setChecked(isRunning);
    cpuStopAction->setChecked(!isRunning);
    cpuUi.b_run->setChecked(isRunning);
    cpuUi.b_stop->setChecked(!isRunning);
    cpuUi.b_step->setDisabled(isRunning);
    cpuUi.b_next->setDisabled(isRunning);
    cpuUi.b_reset->setDisabled(isRunning);
}

void QtGui::UpdateCpuUndocumentedCheck() const
{
    undocumentedAction->setChecked(options.use_undocumented);
}

void QtGui::UpdateFullScreenCheck() const
{
    fullScreenAction->setChecked(IsFullScreenMode());
    if (IsFullScreenMode())
    {
        screenSizeComboBox->setCurrentIndex(screenSizeComboBox->count() - 1);
    }
}

void QtGui::UpdateStatusBarCheck() const
{
    statusBarAction->setChecked(statusBar->isVisible());
}

void QtGui::SetIconSize(const QSize &iconSize)
{
    auto heightDiff = 2 * iconSize.height() - fileToolBar->iconSize().height() -
        statusToolBar->iconSize().height();

    fileToolBar->setIconSize(iconSize);
    viewToolBar->setIconSize(iconSize);
    cpuToolBar->setIconSize(iconSize);
    helpToolBar->setIconSize(iconSize);
    statusToolBar->setIconSize(iconSize);

    int sizeIndex = (iconSize.width() >= 24 || iconSize.height() >= 24) ? 1 : 0;
    sizeIndex = (iconSize.width() >= 32 || iconSize.height() >= 32) ?
                2 : sizeIndex;

    for (int index = 0; index < ICON_SIZES; ++index)
    {
        auto *action = iconSizeAction[index];
        action->setChecked(index == sizeIndex);
    }

    resize(size() + QSize(0, heightDiff));
}

void QtGui::AdjustSize()
{
    if (!IsFullScreenMode())
    {
        QTimer::singleShot(0, this, &QtGui::OnResize);
    }
}

void QtGui::OnResize()
{
    // To adjust the window size the adjustSize has to be processed
    // within the event loop, e.g. with a singleShot timer.
    // See also: QtGui::AdjustSize().
    adjustSize();
}

QUrl QtGui::CreateDocumentationUrl(const QString &docDir,
                                   const QString &htmlFile)
{
    auto path = docDir + PATHSEPARATORSTRING + htmlFile;

    return QUrl::fromLocalFile(path);
}

QFont QtGui::GetMonospaceFont(int pointSize)
{
    auto isFixedPitch = [](const QFont &font) -> bool {
        const QFontInfo fontInfo(font);

        return fontInfo.fixedPitch();
    };

    QFont font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    font.setPointSize(pointSize);

    if (isFixedPitch(font))
    {
       return font;
    }

    font.setStyleHint(QFont::Monospace);
    if (isFixedPitch(font))
    {
       return font;
    }

    font.setStyleHint(QFont::TypeWriter);
    if (isFixedPitch(font))
    {
       return font;
    }

    font.setFamily("courier");

    return font;
}

void QtGui::SetCpuDialogMonospaceFont(int pointSize)
{
    auto monospaceFont = GetMonospaceFont(pointSize);
    cpuUi.e_status->setFont(monospaceFont);
    QFontMetrics monospaceFontMetrics(monospaceFont);
    int height = std::lround(monospaceFontMetrics.height() * (CPU_LINES + 0.5));
    int width;

#if (QT_VERSION >= QT_VERSION_CHECK(5, 11, 0))
    // horizontalAdvance() gives better results than averageCharWidth()
    auto text = QString(CPU_LINE_WIDTH + 1, ' ');
    width = monospaceFontMetrics.horizontalAdvance(text);
#else
    // Best value has empirically been estimated.
    width = (CPU_LINE_WIDTH + 3) * monospaceFontMetrics.averageCharWidth();
#endif

    cpuUi.e_status->setMinimumSize(width, height);
    cpuDialog->adjustSize();
}

ColorTable QtGui::CreateColorTable()
{
    using fn = std::function<QRgb(int)>;

    bool isWithColorScale = !stricmp(options.color.c_str(), "default");
    Word redBase = 255;
    Word greenBase = 255;
    Word blueBase = 255;
    colorTable.resize(MAX_COLORS);

    if (!isWithColorScale)
    {
        getRGBForName(options.color.c_str(), &redBase, &greenBase, &blueBase);
    }

    fn GetColor = [&](int index) -> QRgb
    {
        // Use same color values as Enhanced Graphics Adapter (EGA)
        // or Tandy Color Computer 3 RGB.
        // For details see:
        // https://en.wikipedia.org/wiki/Enhanced_Graphics_Adapter
        // https://exstructus.com/tags/coco/australia-colour-palette/
        static constexpr Byte colorValues[4] { 0x00, 0x55, 0xAA, 0xFF };
        int scale;

        // Create a color scale in the range of 0 - 3 based two color bits
        // <color>_HIGH and <color>_LOW. Convert the color scale into a
        // color value in the range of 0 - 255.
        scale = index & RED_HIGH ? 2 : 0;
        scale |= index & RED_LOW ? 1 : 0;
        auto red = colorValues[scale];
        scale = index & GREEN_HIGH ? 2 : 0;
        scale |= index & GREEN_LOW ? 1 : 0;
        auto green = colorValues[scale];
        scale = index & BLUE_HIGH ? 2 : 0;
        scale |= index & BLUE_LOW ? 1 : 0;
        auto blue = colorValues[scale];

        return qRgb(red, green, blue);
    };

    fn GetColorShade = [&](int index) -> QRgb
    {
        auto red = static_cast<Byte>(redBase * sqrt((double)index /
                (MAX_COLORS - 1)));
        auto green = static_cast<Byte>(greenBase * sqrt((double)index /
                (MAX_COLORS - 1)));
        auto blue = static_cast<Byte>(blueBase * sqrt((double)index /
                (MAX_COLORS - 1)));

        return qRgb(red, green, blue);
    };

    fn GetTheColor = (isWithColorScale ? GetColor : GetColorShade);

    for (int i = 0; i < colorTable.size(); ++i)
    {
        int idx = options.isInverse ? colorTable.size() - i - 1 : i;
        colorTable[idx] = GetTheColor(i);
    }

    return colorTable;
}

void QtGui::CopyToBMPArray(DWord height, QByteArray& dest,
                           Byte const *videoRam, const ColorTable& colTable)
{
    sBITMAPFILEHEADER fileHeader;
    sBITMAPINFOHEADER infoHeader;

    assert(height >= 1);
    assert(colTable.size() >= 1);

    // Size of BMP stream:
    // BITMAPFILEHEADER + BITMAPINFOHEADER + color table size + pixel data
    DWord dataOffset = sizeof(sBITMAPFILEHEADER) + sizeof(sBITMAPINFOHEADER) +
                     (colTable.size() * sizeof(sRGBQUAD));
    DWord destSize = dataOffset + (height * WINDOWWIDTH);

    dest.clear();
    dest.resize(destSize);

    auto *pData = dest.data();

    fileHeader.type[0] = 'B';
    fileHeader.type[1] = 'M';
    fileHeader.fileSize = toLittleEndian<DWord>(destSize);
    fileHeader.reserved[0] = 0U;
    fileHeader.reserved[1] = 0U;
    fileHeader.dataOffset = toLittleEndian<DWord>(dataOffset);
    memcpy(pData, &fileHeader, sizeof(fileHeader));
    pData += sizeof(fileHeader);

    infoHeader.size = toLittleEndian<DWord>(sizeof(infoHeader));
    infoHeader.width = toLittleEndian<SDWord>(WINDOWWIDTH);
    infoHeader.height = toLittleEndian<SDWord>(height);
    infoHeader.planes = toLittleEndian<Word>(1U);
    infoHeader.bitCount = toLittleEndian<Word>(8U);
    infoHeader.compression = toLittleEndian<DWord>(BI_RGB);
    infoHeader.imageSize = toLittleEndian<DWord>(height * WINDOWWIDTH);
    infoHeader.xPixelsPerMeter = 0;
    infoHeader.yPixelsPerMeter = 0;
    infoHeader.colorsUsed = toLittleEndian<DWord>(colTable.size());
    infoHeader.colorsImportant = toLittleEndian<DWord>(colTable.size());
    memcpy(pData, &infoHeader, sizeof(infoHeader));
    pData += sizeof(infoHeader);

    assert(colTable.size() <= static_cast<int>(MAX_COLORS));

    const auto size = static_cast<int>(sizeof(sRGBQUAD)) *
                      colTable.size();
    const auto hash = qHashRange(colTable.begin(), colTable.end());
    if (colorTablesCache.contains(hash))
    {
        memcpy(pData, colorTablesCache.value(hash).data(), size);
        pData += size;
    }
    else
    {
        QByteArray colorTableCache(size, '\0');
        const auto *pDataOrigin = pData;

        for (const auto rgbColor : colTable)
        {
            sRGBQUAD colorEntry;

            colorEntry.red = qRed(rgbColor);
            colorEntry.green = qGreen(rgbColor);
            colorEntry.blue = qBlue(rgbColor);
            colorEntry.reserved = '\0';
            memcpy(pData, &colorEntry, sizeof(colorEntry));
            pData += sizeof(colorEntry);
        }
        memcpy(colorTableCache.data(), pDataOrigin, size);
        colorTablesCache.insert(hash, colorTableCache);
    }
    assert(pData == dest.data() + dataOffset);

    DWord count;    /* Byte counter into video RAM          */
    Byte pixels[6]; /* One byte of video RAM for each plane */
    // Default color index: If no video source is available use highest
    // available color
    Byte colorIndex = options.isInverse ? 0x00U : 0x3FU;
    Byte colorIndexOffset = 0U;
    if (options.isInverse)
    {
        colorIndexOffset = (64U / options.nColors) - 1;
    }

    memset(pixels, '\0', sizeof(pixels));

    // The raster lines have to be filled from bottom to top.
    pData += WINDOWWIDTH * (height - 1);
    for (count = 0; count < (RASTERLINE_SIZE * height); ++count)
    {
        Byte pixelBitMask;

        if (videoRam != nullptr)
        {
            pixels[0] = videoRam[0];

            if (options.nColors > 2)
            {
                pixels[2] = videoRam[VIDEORAM_SIZE];
                pixels[4] = videoRam[VIDEORAM_SIZE * 2];

                if (options.nColors > 8)
                {
                    pixels[1] = videoRam[VIDEORAM_SIZE * 3];
                    pixels[3] = videoRam[VIDEORAM_SIZE * 4];
                    pixels[5] = videoRam[VIDEORAM_SIZE * 5];
                }
            }

            videoRam++;

            /* Loop from MSBit to LSBit */
            for (pixelBitMask = 0x80U; pixelBitMask; pixelBitMask >>= 1)
            {
                colorIndex = colorIndexOffset; /* calculated color index */

                if (pixels[0] & pixelBitMask)
                {
                    colorIndex += GREEN_HIGH;    // 0x0C, green high
                }

                if (options.nColors > 8)
                {
                    if (pixels[2] & pixelBitMask)
                    {
                        colorIndex += RED_HIGH;    // 0x0D, red high
                    }

                    if (pixels[4] & pixelBitMask)
                    {
                        colorIndex += BLUE_HIGH;    // 0x0E, blue high
                    }

                    if (pixels[1] & pixelBitMask)
                    {
                        colorIndex += GREEN_LOW;    // 0x04, green low
                    }

                    if (pixels[3] & pixelBitMask)
                    {
                        colorIndex += RED_LOW;    // 0x05, red low
                    }

                    if (pixels[5] & pixelBitMask)
                    {
                        colorIndex += BLUE_LOW;    // 0x06, blue low
                    }
                }
                else
                {
                    if (pixels[2] & pixelBitMask)
                    {
                        colorIndex += RED_HIGH;    // 0x0D, red high
                    }

                    if (pixels[4] & pixelBitMask)
                    {
                        colorIndex += BLUE_HIGH;    // 0x0E, blue high
                    }
                }
                *(pData)++ = colorIndex;
            }
        }
        else
        {
            for (pixelBitMask = 0x80U; pixelBitMask; pixelBitMask >>= 1)
            {
                *(pData)++ = colorIndex;
            }
        }
        if (count % RASTERLINE_SIZE == (RASTERLINE_SIZE - 1))
        {
            pData -= 2 * WINDOWWIDTH;
        }
    }

    assert(pData == dest.data() + dataOffset - WINDOWWIDTH);
}

bool QtGui::event(QEvent *event)
{
    if (event->type() == QEvent::StatusTip)
    {
        auto *statusTipEvent = (QStatusTipEvent *)event;
        SetStatusMessage(statusTipEvent->tip());
        statusTipEvent->accept();

        return true;
    }

    return QWidget::event(event);
}

void QtGui::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::ApplicationFontChange
        || event->type() == QEvent::FontChange)
    {
        auto font = QApplication::font();
        auto newPointSize = font.pointSize();

        QFontInfo fontInfo(cpuUi.e_status->font());

        if (!fontInfo.fixedPitch() ||
            (cpuUi.e_status->font().pointSize() != newPointSize))
        {
            SetCpuDialogMonospaceFont(newPointSize);
        }

        AdjustSize();
    }

    else if (event->type() == QEvent::StyleChange
             || event->type() == QEvent::ThemeChange)
    {
        AdjustSize();
        QWidget::update();
    }
}

void QtGui::showEvent(QShowEvent *)
{
    int index = (e2screen->GetScaledSize().width() / WINDOWWIDTH) - 1;

    index = std::min(index, SCREEN_SIZES - 1);
    UpdateScreenSizeCheck(index);
    UpdateScreenSizeValue(index);
    UpdateStatusBarCheck();

    e2screen->setFocus();
}

void QtGui::keyPressEvent(QKeyEvent *event)
{
    assert(event != nullptr);

    // Defined hotkeys:
    //
    // Hotkey           | Action
    // ----------------------------------------------------------
    // Ctrl+1           | Set default screen size
    // Ctrl+2           | Set double screen size
    // Ctrl+3           | Set triple screen size
    // Ctrl+4           | Set quadruple screen size
    // Ctrl+5           | Set quintuple screen size
    // Ctrl+F12         | Toogle smooth display
    // Pause            | Toggle CPU to stop or run
    // Shift+Pause      | Send non maskable interrupt (NMI) to CPU
    // Shift+Alt+Pause  | Reset and run CPU
    //
    // Hotkey for toggling full screen mode depends on the user interface:
    // Windows        | F11, Alt+Enter
    // macOS          | Ctrl+Meta+F
    // KDE            | F11, Ctrl+Shift+F
    // GNOME          | Ctrl+F11
    // Others         | F11

    static const auto modifiers =
        Qt::ShiftModifier | Qt::ControlModifier | Qt::AltModifier;

    switch (event->modifiers() & modifiers)
    {
        case Qt::ShiftModifier:
            switch (event->key())
            {
                case Qt::Key_Pause:
                    cpu.set_nmi();
                    event->accept();
                    return;
            }
            break;

        case Qt::ControlModifier:
            switch (event->key())
            {
                case Qt::Key_1:
                case Qt::Key_2:
                case Qt::Key_3:
                case Qt::Key_4:
                case Qt::Key_5:
                {
                    auto index = event->key() - Qt::Key_1 + 1;
                    e2screen->ResizeToFactor(index);
                    AdjustSize();
                    UpdateScreenSizeCheck(index);
                    UpdateScreenSizeValue(index);
                    event->accept();
                    return;
                }

                case Qt::Key_F12:
                    OnSmoothDisplay();
                    event->accept();
                    return;
            }
            break;

        case Qt::ShiftModifier | Qt::AltModifier:
            switch (event->key())
            {
                case Qt::Key_Pause:
                    scheduler.request_new_state(CpuState::ResetRun);
                    event->accept();
                    return;
            }
            break;

        case 0:
            switch (event->key())
            {
                case Qt::Key_Pause:
                    ToggleCpuRunStop();
                    event->accept();
                    return;
            }
            break;
    }

    event->ignore();
}

void QtGui::resizeEvent(QResizeEvent *event)
{
    if (!IsFullScreenMode())
    {
        int index = (e2screen->GetScaledSize().width() / WINDOWWIDTH) - 1;
        index = std::min(index, 4);
        UpdateScreenSizeCheck(index);
        UpdateScreenSizeValue(index);
    }

    event->ignore();
    QWidget::resizeEvent(event);
}

void QtGui::closeEvent(QCloseEvent *event)
{
    if (IsClosingConfirmed())
    {
        e2screen->ReleaseMouseCapture();
        scheduler.request_new_state(CpuState::Exit);
        while (!scheduler.is_finished())
        {
            scheduler.timer_elapsed();
            QThread::msleep(10);
        }
        timer.stop();
        FlexemuOptionsDifference optionsDiff(options, oldOptions);

        if (!optionsDiff.GetNotEquals().empty())
        {
            FlexemuOptions::WriteOptions(options, false, true);
        }
        event->accept();
    }
    else
    {
        event->ignore();
    }
}

void QtGui::WriteOneOption(sOptions p_options, FlexemuOptionId optionId) const
{
    p_options.readOnlyOptionIds = allFlexemuOptionIds;
    p_options.readOnlyOptionIds.erase(
        std::remove(p_options.readOnlyOptionIds.begin(),
                    p_options.readOnlyOptionIds.end(),
                    optionId),
        p_options.readOnlyOptionIds.end());
    FlexemuOptions::WriteOptions(p_options, false, true);
}

