/*
    qtgui.cpp  Platform independent user interface


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2020-2025  W. Schwotzer

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
#include "ccopymem.h"
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
#include "colors.h"
#include "poutwin.h"
#include "fversion.h"
#include "free.h"
#include "qtfree.h"
#include "bintervl.h"
#include "warnoff.h"
#ifdef USE_CMAKE
#include "ui_about.h"
#else
#include "about_ui.h"
#endif
#include <QString>
#include <QStringList>
#include <QPixmap>
#include <QPainter>
#include <QIODevice>
#include <QTextStream>
#include <QFrame>
#include <QLabel>
#include <QWidget>
#include <QScreen>
#include <QToolBar>
#include <QAbstractButton>
#include <QMainWindow>
#include <QMessageBox>
#include <QDir>
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
#include <QTimer>
#include <QHash>
#include <QFont>
#include <QFontDatabase>
#include <QTextBrowser>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <fmt/format.h>
#include "warnon.h"
#include <cmath>
#include <cassert>
#include <array>
#include <vector>
#include <regex>

int QtGui::preferencesTabIndex = 0;

QtGui::QtGui(
    Mc6809 &p_cpu,
    Memory &p_memory,
    Scheduler &p_scheduler,
    Inout &p_inout,
    VideoControl1 &p_vico1,
    VideoControl2 &p_vico2,
    JoystickIO &p_joystickIO,
    KeyboardIO &p_keyboardIO,
    TerminalIO &p_terminalIO,
    Pia1 &p_pia1,
    struct sOptions &p_options)
        : AbstractGui(
               p_cpu
             , p_memory
             , p_inout
             , p_terminalIO)
        , mainLayout(new QVBoxLayout(this))
        , statusBarLayout(new QHBoxLayout)
        , menuBar(new QMenuBar)
        , cpuDialog(new QDialog(this))
        , isRunning(true)
        , isConfirmClose(true)
        , isForceScreenUpdate(true)
        , cpuState(CpuState::NONE)
        , scheduler(p_scheduler)
        , vico1(p_vico1)
        , vico2(p_vico2)
        , joystickIO(p_joystickIO)
        , keyboardIO(p_keyboardIO)
        , options(p_options)
        , oldOptions(p_options)
{
    const QSize iconSize(options.iconSize, options.iconSize);

    cpuLoggerConfig.reset();

    setObjectName("flexemuMainWindow");

    colorTable = CreateColorTable();

    mainLayout->setObjectName(QString::fromUtf8("mainLayout"));
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(menuBar);

    CreateActions(*mainLayout, iconSize);

    e2screen = new E2Screen(p_scheduler, p_joystickIO, p_keyboardIO,
                            p_pia1, p_options, colorTable.first(), this);
    mainLayout->addWidget(e2screen, 1); //, Qt::AlignCenter);
    e2screen->setFocusPolicy(Qt::StrongFocus);

    CreateIcons();
    CreateStatusToolBar(*mainLayout, iconSize);
    const auto name = QString::fromUtf8("statusBarLayout");
    statusBarLayout->setObjectName(name);
    statusBarLayout->setContentsMargins(0, 0, 0, 0);
    statusBarLayout->setSpacing(2);
    mainLayout->addLayout(statusBarLayout);
    CreateStatusBar(*statusBarLayout);

    setWindowState(Qt::WindowActive);
    // Resize needed here to overwrite previous adjustSize()
    // with 2/3 screen limit.
    resize(sizeHint());
    const auto flexemuIcon = QIcon(":/resource/flexemu.png");
    setWindowIcon(flexemuIcon);

    connect(&timer, &QTimer::timeout, this, &QtGui::OnTimer);
    timer.start(TIME_BASE / 1000);

    setLayout(mainLayout);

    setWindowTitle(e2screen->GetTitle());

    e2screen->ReleaseMouseCapture();
    e2screen->Attach(*this);

    // Initialize the non-modal CPU Dialog but don't open it.
    cpuUi.setupUi(cpuDialog);
    const QString title = QString("%1 %2")
        .arg(QString::fromStdString(p_cpu.get_vendor()))
        .arg(QString::fromStdString(p_cpu.get_name()));
    cpuDialog->setWindowTitle(title);
    cpuDialog->setModal(false);
    cpuDialog->setSizeGripEnabled(true);
    cpuUi.b_run->setCheckable(true);
    cpuUi.b_stop->setCheckable(true);
    ConnectCpuUiSignalsWithSlots();

    printOutputWindow = new PrintOutputWindow(options);

    SetCpuDialogMonospaceFont(QApplication::font().pointSize());
    UpdateCpuFrequencyCheck();
    UpdateCpuUndocumentedCheck();
    if (options.isSmooth)
    {
        ToggleSmoothDisplay();
    }

    SetIconSizeCheck(iconSize);

    if (!options.isStatusBarVisible)
    {
        SetStatusBarVisibility(false);
    }
    UpdateStatusBarCheck();
}

QtGui::~QtGui()
{
    delete cpuDialog;
}

void QtGui::SetFloppy(E2floppy *p_fdc)
{
    if (fdc == nullptr && p_fdc != nullptr)
    {
        fdc = p_fdc; // Only Eurocom II/V7 has a floppy controller.

        AddDiskStatusButtons();
    }
}

bool QtGui::HasFloppy() const
{
    return fdc != nullptr;
}

bool QtGui::output_to_graphic()
{
    const auto result = AbstractGui::output_to_graphic();
    if (!isVisible())
    {
        show();
    }

    return result;
}

// Implementation may change in future.
// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
FlexemuToolBar *QtGui::CreateToolBar(QWidget *parent, const QString &title,
        const QString &objectName, const QSize &iconSize) const
{
    auto *newToolBar = new FlexemuToolBar(title, parent);
    assert(newToolBar != nullptr);
    newToolBar->setObjectName(objectName);
    newToolBar->setFloatable(false);
    newToolBar->setMovable(false);
    newToolBar->setIconSize(iconSize);
    newToolBar->SetPixelSize(options.pixelSize);

    return newToolBar;
}

void QtGui::OnPrinterOutput()
{
    printOutputWindow->show();
    printOutputWindow->raise();
}

void QtGui::OnExit()
{
    bool safeFlag = isRestartNeeded;

    e2screen->Detach(*this);
    isRestartNeeded = false;
    close();
    isRestartNeeded = safeFlag;
}

void QtGui::SetPreferencesStatusText(bool p_isRestartNeeded) const
{
    assert(preferencesAction != nullptr);

    const auto statusText = p_isRestartNeeded ?
        tr("Edit application preferences - Needs restart") :
        tr("Edit application preferences");

    preferencesAction->setStatusTip(statusText);
}

QIcon QtGui::GetPreferencesIcon(bool p_isRestartNeeded)
{
    return p_isRestartNeeded ?
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
            ForceRestart();
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
                        BCommandSPtr(new CSetFrequency(scheduler, f));
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

                case FlexemuOptionId::IsDisplaySmooth:
                    ToggleSmoothDisplay();
                    break;

                case FlexemuOptionId::CanFormatDrive0:
                case FlexemuOptionId::CanFormatDrive1:
                case FlexemuOptionId::CanFormatDrive2:
                case FlexemuOptionId::CanFormatDrive3:
                case FlexemuOptionId::FileTimeAccess:
                case FlexemuOptionId::IsTerminalIgnoreESC:
                case FlexemuOptionId::IsTerminalIgnoreNUL:
                case FlexemuOptionId::PrintFont:
                case FlexemuOptionId::IsPrintPageBreakDetected:
                case FlexemuOptionId::PrintOrientation:
                case FlexemuOptionId::PrintPageSize:
                case FlexemuOptionId::PrintUnit:
                case FlexemuOptionId::PrintOutputWindowGeometry:
                case FlexemuOptionId::PrintPreviewDialogGeometry:
                case FlexemuOptionId::PrintConfigs:
                case FlexemuOptionId::IsDirectoryDiskActive:
                case FlexemuOptionId::DirectoryDiskTrkSec:
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
                case FlexemuOptionId::IsRamExt2x384:
                case FlexemuOptionId::IsFlexibleMmu:
                case FlexemuOptionId::IsEurocom2V5:
                case FlexemuOptionId::IsUseRtc:
                case FlexemuOptionId::IconSize:
                case FlexemuOptionId::TerminalType:
                case FlexemuOptionId::IsStatusBarVisible:
                case FlexemuOptionId::IsConfirmExit:
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
    memoryWindowMgr.SetReadOnly(true);
}

void QtGui::OnCpuStop()
{
    scheduler.request_new_state(CpuState::Stop);
    isRunning = false;
    UpdateCpuRunStopCheck();
    memoryWindowMgr.SetReadOnly(false);
}

void QtGui::OnCpuStep()
{
    scheduler.request_new_state(CpuState::Step);
    isRunning = false;
    UpdateCpuRunStopCheck();
    memoryWindowMgr.SetReadOnly(true);
}

void QtGui::OnCpuNext()
{
    scheduler.request_new_state(CpuState::Next);
    isRunning = false;
    UpdateCpuRunStopCheck();
    memoryWindowMgr.SetReadOnly(true);
}

void QtGui::OnCpuReset()
{
    osName.clear();
    scheduler.request_new_state(CpuState::Reset);
    isRunning = false;
    UpdateCpuRunStopCheck();
    memoryWindowMgr.SetReadOnly(false);
}

void QtGui::OnCpuResetRun()
{
    osName.clear();
    scheduler.request_new_state(CpuState::ResetRun);
    isRunning = true;
    UpdateCpuRunStopCheck();
    memoryWindowMgr.SetReadOnly(true);
}

void QtGui::OnCpuDialogToggle()
{
    assert(cpuDialog != nullptr);

    if (cpuViewAction->isChecked())
    {
        // Choose the right position for the CPU Dialog
        // either above or below the main window.
        auto position = pos();
        int x;
        int y;

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
            int screenHeight = 0;

            if (!QGuiApplication::screens().isEmpty())
            {
                screenHeight =
                    QGuiApplication::screens().first()->geometry().height();
            }

            y = frameGeometry().y() + frameGeometry().height() + 1;
            if (y + cpuDialog->frameGeometry().height() < screenHeight)
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

void QtGui::OnOpenMemoryWindow()
{
    const auto isReadOnly = (cpuState != CpuState::Stop);

    memoryWindowMgr.OpenMemoryWindow(isReadOnly, options, memory, scheduler);
}

void QtGui::ConnectCpuUiSignalsWithSlots()
{
    connect(cpuUi.b_close, &QAbstractButton::clicked,
            this, &QtGui::OnCpuDialogClose);
    connect(cpuUi.b_run, &QAbstractButton::clicked,
            this, &QtGui::OnCpuRun);
    connect(cpuUi.b_stop, &QAbstractButton::clicked, this, &QtGui::OnCpuStop);
    connect(cpuUi.b_step, &QAbstractButton::clicked, this, &QtGui::OnCpuStep);
    connect(cpuUi.b_next, &QAbstractButton::clicked, this, &QtGui::OnCpuNext);
    connect(cpuUi.b_reset, &QAbstractButton::clicked, this, &QtGui::OnCpuReset);
    connect(cpuUi.b_breakpoints, &QAbstractButton::clicked,
            this, &QtGui::OnCpuBreakpoints);
    connect(cpuUi.b_logfile, &QAbstractButton::clicked,
            this, &QtGui::OnCpuLogging);
}

void QtGui::OnCpuBreakpoints()
{
    BPArray breakpoints = { cpu.get_bp(0), cpu.get_bp(1) };
    auto *dialog = new QDialog;
    BreakpointSettingsUi ui;

    ui.setupUi(*dialog);
    ui.SetData(breakpoints);

    auto result = dialog->exec();

    if (result == QDialog::Accepted)
    {
        int index = 0;

        breakpoints = ui.GetData();
        for (const auto &breakpoint : breakpoints)
        {
            if (breakpoint.has_value())
            {
                cpu.set_bp(index, breakpoint.value());
            }
            else
            {
                cpu.reset_bp(index);
            }
            ++index;
        }
    }
}

void QtGui::OnCpuLogging()
{
    auto *dialog = new QDialog;
    Mc6809LoggerConfigUi ui;
    ui.setupUi(*dialog);
    ui.SetData(cpuLoggerConfig);

    auto result = dialog->exec();
    if (result == QDialog::Accepted)
    {
        cpuLoggerConfig = ui.GetData();
        scheduler.sync_exec(BCommandSPtr(new CmdSetMc6809LoggerConfig(
                        cpu, cpuLoggerConfig)));
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

void QtGui::OnIntroduction() const
{
    auto documentationDir = QString::fromStdString(options.doc_dir.u8string());
    auto url = CreateDocumentationUrl(documentationDir, "flexemu.htm");

    QDesktopServices::openUrl(url);
}

// Implementation may change in future.
// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
void QtGui::OnAbout()
{
    QDialog dialog;
    Ui_AboutDialog ui{};

    ui.setupUi(&dialog);
    const auto aboutIcon = QIcon(":/resource/about.png");
    dialog.setWindowIcon(aboutIcon);
    dialog.resize({0, 0});
    dialog.setModal(true);
    dialog.setSizeGripEnabled(true);
    auto title = tr("About %1").arg(PROGRAMNAME);
    dialog.setWindowTitle(title);
    auto *scene = new QGraphicsScene(ui.w_icon);
    scene->setSceneRect(0, 0, 32, 32);
    ui.w_icon->setScene(scene);
    scene->addPixmap(QPixmap(":/resource/flexemu.png"))->setPos(0, 0);
    connect(ui.c_tabWidget, &QTabWidget::currentChanged, this,
            [&](int index){
                const std::vector<QTextBrowser *>browsers{
                    ui.e_about,
                    ui.e_versions,
                    ui.e_configuration,
                };
                if (index >= 0 && index < static_cast<int>(browsers.size()))
                {
                    AboutTabChanged(browsers[index]);
                }
            });
    ui.c_tabWidget->setCurrentIndex(0);
    ui.e_about->setOpenExternalLinks(true);
    ui.e_versions->setOpenExternalLinks(true);
    ui.e_configuration->setOpenExternalLinks(true);

    dialog.exec();
}

// Implementation may change in future.
// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
void QtGui::AboutTabChanged(QTextBrowser *browser) const
{
    assert(browser != nullptr);

    if (browser->objectName() == "e_about")
    {
        browser->setHtml(GetAboutHtmlText());
    }
    else if (browser->objectName() == "e_versions")
    {
        browser->setHtml(GetVersionsHtmlText(PROGRAMNAME));
    }
    else if (browser->objectName() == "e_configuration")
    {
        browser->setHtml(GetConfigurationHtmlText());
    }
}

// Implementation may change in future.
// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
QString QtGui::GetAboutHtmlText() const
{
    return tr("<b>%1 V%2</b><p>"
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
       "Copyright (C) 1997-2025 "
       "<a href=\"mailto:wolfgang.schwotzer@gmx.net\">"
       "Wolfgang Schwotzer</a><p>"
       "<a href=\"http://flexemu.neocities.org\">"
       "http://flexemu.neocities.org</a>")
    .arg(PROGRAMNAME).arg(VERSION);
}

QString QtGui::GetConfigurationHtmlText() const
{
    auto result = tr("<b>%1 V%2</b><p>Guest Configuration:\n")
        .arg(PROGRAMNAME).arg(VERSION);

    result.append(ConvertItemPairListToHtml(GetConfiguration()));

    return result;
}

void QtGui::OnTimer()
{
    scheduler.timer_elapsed();

    // check every 1 second for
    // - Parse name of Boot ROM
    // - Parse name of loaded operating system.
    if (++timerTicks % (1000000 / TIME_BASE) == 0)
    {
        timerTicks = 0;
        ParseRomName();
        ParseOsName();
        RequestMemoryUpdate();
    }

    // check every 100 ms for
    // - CPU view update
    // - Disk status update
    // - interrupt info update
    // - CPU frequency update (if newFrequency has value)
    if (timerTicks % (100000 / TIME_BASE) == 0)
    {
        Word t;
        // Check for disk status update every 200 ms
        static tInterruptStatus irqStat;
        tInterruptStatus newIrqStat;
        static std::array<bool, INT_RESET + 1> lastState{};
        bool bState;

        if (HasFloppy())
        {
            static std::array<DiskStatus, MAX_DRIVES> status{};
            std::array<DiskStatus, MAX_DRIVES> newStatus{};

            if (isTimerFirstTime)
            {
                status = {};
            }

            fdc->get_drive_status(newStatus);

            for (t = 0; t < MAX_DRIVES; ++t)
            {
                if (newStatus[t] != status[t])
                {
                    UpdateDiskStatus(t, status[t], newStatus[t]);
                    status[t] = newStatus[t];
                }
            }
        }

        scheduler.get_interrupt_status(newIrqStat);

        auto target_frequency = scheduler.get_target_frequency();
        bool newIsOriginalFrequency = (target_frequency > 0.0F) &&
            std::fabs(target_frequency - ORIGINAL_FREQUENCY) < 0.01F;
        if (newIsOriginalFrequency != isOriginalFrequency)
        {
            isOriginalFrequency = newIsOriginalFrequency;
            UpdateCpuFrequencyCheck();
        }
        {
            std::lock_guard<std::mutex> guard(newFrequencyMutex);
            if (newFrequency.has_value())
            {
                SetCpuFrequency(*newFrequency);
                newFrequency.reset();
            }
        }

        std::vector<Byte> newKeysCopy;
        {
            std::lock_guard<std::mutex> guard(newKeysMutex);
            if (!newKeys.empty())
            {
                newKeysCopy = std::move(newKeys);
            }
        }

        for (auto key : newKeysCopy)
        {
            const auto newKeyString = GetKeyString(key);
            newKeyLabel->setText(QString::fromStdString(newKeyString));
        }

        if (isTimerFirstTime)
        {
            for (t = INT_IRQ; t <= INT_RESET; ++t)
            {
                lastState[t] = false;
                irqStat.count[t] = 0;
                UpdateInterruptStatus(static_cast<tIrqType>(t), false);
            }

            UpdateCpuUndocumentedCheck();

            isTimerFirstTime = false;
        }
        else
        {
            for (t = INT_IRQ; t <= INT_RESET; ++t)
            {
                bState = (newIrqStat.count[t] != irqStat.count[t]);

                if (bState != lastState[t])
                {
                    UpdateInterruptStatus(static_cast<tIrqType>(t), bState);
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
        const auto *status =
            dynamic_cast<Mc6809CpuStatus *>(scheduler.get_status());

        if (status != nullptr)
        {
            isRunning = (status->state == CpuState::Run ||
                         status->state == CpuState::Next);

            UpdateCpuRunStopCheck();

            update_cpuview(*status);

            if (cpuState != CpuState::Invalid &&
                status->state == CpuState::Invalid)
            {
                GotIllegalInstruction(*status);
            }
            cpuState = status->state;
        }

        memoryWindowMgr.UpdateData();

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

QString QtGui::GetScreenSizeStatusTip(int index)
{
    static const QStringList statusTips{
        tr("Resize Screen to default size (512 x 256)"),
        tr("Resize Screen to double size (1024 x 512)"),
        tr("Resize Screen to triple size (1536 x 768)"),
        tr("Resize Screen to quadruple size (2048 x 1024)"),
        tr("Resize Screen to quintuple size (2560 x 1280)"),
        tr("Enter Fullscreen Mode"),
    };

    if (index >= 0 && index < statusTips.size())
    {
        return statusTips[index];
    }

    return "";
}

void QtGui::OnScreenSizeHighlighted(int index) const
{
    auto message = GetScreenSizeStatusTip(index);

    SetStatusMessage(message);
}

void QtGui::OnIconSize(int index)
{
    const int size = 16 + 8 * index;
    const QSize iconSize({size, size});

    SetIconSize(iconSize);
    SetIconSizeCheck(iconSize);

    options.iconSize = size;
    oldOptions.iconSize = options.iconSize;
    WriteOneOption(options, FlexemuOptionId::IconSize);
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
        toolBar->SetPixelSize(index + 1);
        toolBar->updateGeometry();
        statusToolBar->SetPixelSize(index + 1);
        statusToolBar->updateGeometry();
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

void QtGui::CreateActions(QLayout &layout, const QSize &iconSize)
{
    toolBar = CreateToolBar(this, tr("ToolBar"), QStringLiteral("toolBar"),
            iconSize);
    assert(toolBar != nullptr);
    layout.addWidget(toolBar);

    CreateFileActions(*toolBar);
    CreateEditActions(*toolBar);
    CreateViewActions(*toolBar);
    CreateCpuActions(*toolBar);
    CreateHelpActions(*toolBar);
}

void QtGui::CreateFileActions(QToolBar &p_toolBar)
{
    auto *fileMenu = menuBar->addMenu(tr("&File"));

    const auto printerIcon = QIcon(":/resource/print-output.png");
    printOutputAction =
        fileMenu->addAction(printerIcon, tr("Open &Printer Output Window"));
    connect(printOutputAction, &QAction::triggered, this,
            &QtGui::OnPrinterOutput);
    printOutputAction->setStatusTip(tr("Open the printer output window"));

    const auto memoryIcon = QIcon(":/resource/memory.png");
    memoryWindowAction = fileMenu->addAction(memoryIcon,
            tr("Open &Memory Window"));
    connect(memoryWindowAction, &QAction::triggered,
            this, &QtGui::OnOpenMemoryWindow);
    memoryWindowAction->setStatusTip(tr("Open a new memory window"));

    fileMenu->addSeparator();

    const auto exitIcon = QIcon(":/resource/exit.png");
    exitAction = fileMenu->addAction(exitIcon, tr("E&xit"));
    connect(exitAction, &QAction::triggered, this, &QtGui::OnExit);
    exitAction->setShortcut(QKeySequence(tr("Shift+Ctrl+Q")));
    exitAction->setStatusTip(tr("Exit the application"));
    p_toolBar.addAction(exitAction);

    p_toolBar.addAction(printOutputAction);
    p_toolBar.addAction(memoryWindowAction);
}

void QtGui::CreateEditActions(QToolBar &p_toolBar)
{
    auto *editMenu = menuBar->addMenu(tr("&Edit"));

    p_toolBar.addSeparator();
    const auto preferencesIcon = GetPreferencesIcon(isRestartNeeded);
    preferencesAction =
        editMenu->addAction(preferencesIcon, tr("&Preferences"));
    SetPreferencesStatusText(isRestartNeeded);
    connect(preferencesAction, &QAction::triggered,
            this, &QtGui::OnPreferences);
    const auto keySequence = QKeySequence(tr("Shift+Ctrl+P"));
    preferencesAction->setShortcut(keySequence);
    p_toolBar.addAction(preferencesAction);
}

void QtGui::CreateViewActions(QToolBar &p_toolBar)
{
    auto *viewMenu = menuBar->addMenu(tr("&View"));

    p_toolBar.addSeparator();
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

    for (uint16_t index = 0U; index < ICON_SIZES; ++index)
    {
        iconSizeAction[index] = CreateIconSizeAction(*iconSizeMenu, index);
        connect(iconSizeAction[index], &QAction::triggered,
            this, [&,index](){ OnIconSize(index); });
    }

    auto *screenSizeMenu = viewMenu->addMenu(tr("&Screen Size"));
    screenSizeComboBox = new QComboBox();

    for (uint16_t index = 0; index < SCREEN_SIZES; ++index)
    {
        const auto iconPath = QString(":/resource/screen%1.png").arg(index + 1);
        const auto screenSizeIcon = QIcon(iconPath);

        screenSizeAction[index] =
            CreateScreenSizeAction(screenSizeIcon, *screenSizeMenu, index);
        auto text = QString("x%1").arg(index + 1);
        screenSizeComboBox->addItem(screenSizeIcon, text);
        screenSizeComboBox->setMinimumContentsLength(
                cast_from_qsizetype(text.size()));
    }

    const auto screenFullIcon = QIcon(":/resource/screen-full.png");
    screenSizeComboBox->addItem(screenFullIcon, "");
    screenSizeComboBox->insertSeparator(screenSizeComboBox->count() - 1);
    // E2Screen is the only widget which gets the focus.
    screenSizeComboBox->setFocusPolicy(Qt::NoFocus);
    p_toolBar.addWidget(screenSizeComboBox);
    ConnectScreenSizeComboBoxSignalSlots();

    viewMenu->addMenu(screenSizeMenu);
    viewMenu->addSeparator();

    smoothAction = viewMenu->addAction(tr("&Smooth Display"));
    connect(smoothAction, &QAction::triggered, this, &QtGui::OnSmoothDisplay);
    smoothAction->setCheckable(true);
    smoothAction->setShortcut(QKeySequence(tr("Ctrl+F12")));
    smoothAction->setStatusTip(tr("Enter or exit smooth display mode"));
    viewMenu->addAction(smoothAction);
}

void QtGui::CreateCpuActions(QToolBar &p_toolBar)
{
    auto *cpuMenu = menuBar->addMenu(tr("&CPU"));

    p_toolBar.addSeparator();
    const auto runIcon = QIcon(":/resource/run.png");
    cpuRunAction = cpuMenu->addAction(runIcon, tr("&Run"));
    connect(cpuRunAction, &QAction::triggered, this, &QtGui::OnCpuRun);
    cpuRunAction->setCheckable(true);
    cpuRunAction->setStatusTip(tr("Continue CPU execution"));
    p_toolBar.addAction(cpuRunAction);

    const auto stopIcon = QIcon(":/resource/stop.png");
    cpuStopAction = cpuMenu->addAction(stopIcon, tr("&Stop"));
    connect(cpuStopAction, &QAction::triggered, this, &QtGui::OnCpuStop);
    cpuStopAction->setCheckable(true);
    cpuStopAction->setStatusTip(tr("Stop CPU execution"));
    p_toolBar.addAction(cpuStopAction);

    const auto resetIcon = QIcon(":/resource/reset.png");
    cpuResetAction = cpuMenu->addAction(resetIcon, tr("&Reset"));
    connect(cpuResetAction, &QAction::triggered, this, &QtGui::OnCpuResetRun);
    cpuResetAction->setStatusTip(tr("Reset and continue CPU execution"));
    p_toolBar.addAction(cpuResetAction);

    cpuMenu->addSeparator();
    p_toolBar.addSeparator();
    const auto viewIcon = QIcon(":/resource/cpu.png");
    cpuViewAction = cpuMenu->addAction(viewIcon, tr("&View..."));
    connect(cpuViewAction, &QAction::triggered,
        this, &QtGui::OnCpuDialogToggle);
    cpuViewAction->setStatusTip(tr("Open CPU status window"));
    cpuViewAction->setCheckable(true);
    p_toolBar.addAction(cpuViewAction);

    const auto breakpointsIcon = QIcon(":/resource/breakpoints.png");
    auto text = tr("&Breakpoints...");
    breakpointsAction = cpuMenu->addAction(breakpointsIcon, text);
    connect(breakpointsAction, &QAction::triggered,
        this, &QtGui::OnCpuBreakpoints);
    breakpointsAction->setStatusTip(tr("Open breakpoint settings"));
    p_toolBar.addAction(breakpointsAction);

    const auto loggingIcon = QIcon(":/resource/logging.png");
    loggingAction = cpuMenu->addAction(loggingIcon, tr("&Logging..."));
    connect(loggingAction, &QAction::triggered, this, &QtGui::OnCpuLogging);
    loggingAction->setStatusTip(tr("Open logging settings"));
    p_toolBar.addAction(loggingAction);

    cpuMenu->addSeparator();
    p_toolBar.addSeparator();
    const auto originalFrequencyIcon =
        QIcon(":/resource/original-frequency.png");
    text = tr("&Original Frequency");
    originalFrequencyAction = cpuMenu->addAction(originalFrequencyIcon, text);
    connect(originalFrequencyAction, &QAction::triggered,
        this, &QtGui::OnCpuOriginalFrequency);
    originalFrequencyAction->setCheckable(true);
    text = tr("Set original or maximum possible CPU frequency");
    originalFrequencyAction->setStatusTip(text);
    p_toolBar.addAction(originalFrequencyAction);

    const auto undocumentedIcon = QIcon(":/resource/cpu-undocumented.png");
    text = tr("&Undocumented Instructions");
    undocumentedAction = cpuMenu->addAction(undocumentedIcon, text);
    connect(undocumentedAction, &QAction::triggered,
        this, &QtGui::OnCpuUndocumentedInstructions);
    undocumentedAction->setCheckable(true);
    undocumentedAction->setStatusTip(
            tr("Toggle support of undocumented CPU instructions"));
}

void QtGui::CreateHelpActions(QToolBar &p_toolBar)
{
    auto *helpMenu = menuBar->addMenu(tr("&Help"));

    p_toolBar.addSeparator();
    const auto introductionIcon = QIcon(":/resource/info.png");
    auto text = tr("&Introduction");
    introductionAction = helpMenu->addAction(introductionIcon, text);
    connect(introductionAction, &QAction::triggered,
        this, &QtGui::OnIntroduction);
    introductionAction->setStatusTip(
            tr("Open an introduction to this application"));
    p_toolBar.addAction(introductionAction);

    const auto aboutIcon = QIcon(":/resource/about.png");
    aboutAction = helpMenu->addAction(aboutIcon, tr("&About"));
    connect(aboutAction, &QAction::triggered, this, &QtGui::OnAbout);
    aboutAction->setStatusTip(tr("Show the application's about box"));

    const auto aboutQtIcon = QIcon(":/resource/qt.png");
    aboutQtAction = helpMenu->addAction(aboutQtIcon, tr("&About Qt"));
    connect(aboutQtAction, &QAction::triggered, qApp, &QApplication::aboutQt);
    aboutQtAction->setStatusTip(tr("Show the Qt library's about box"));
}

void QtGui::CreateStatusToolBar(QLayout &layout, const QSize &iconSize)
{
    statusToolBar =
        CreateToolBar(this, tr("Status"), QStringLiteral("statusToolBar"),
                iconSize);
    assert(statusToolBar != nullptr);
    statusToolBar->setIconSize(iconSize);
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

        for (Word i = 0; i < MAX_DRIVES; ++i)
        {
            const auto text = tr("Disk #%1 not ready").arg(i);
            diskStatusAction[i] = statusToolBar->addAction(iconNoFloppy, text);
            connect(diskStatusAction[i], &QAction::triggered,
                this, [this, i=i]() { OnDiskStatus(i); });
            auto statusTip = tr("Open disk #%1 status").arg(i);
            diskStatusAction[i]->setStatusTip(statusTip);
        }
    }
}

QAction *QtGui::CreateScreenSizeAction(
        const QIcon &icon, QMenu &menu, uint16_t index)
{
    static const QStringList menuText{
        tr("&Default"),
        tr("D&ouble"),
        tr("Tr&iple"),
        tr("&Quadruple"),
        tr("Qu&intuple")
    };

    assert(menuText.size() == SCREEN_SIZES);
    assert (index < SCREEN_SIZES);

    auto keySequence = QKeySequence(tr("Ctrl+%1").arg(index + 1));
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
    text = QString("%1").arg(status.count[INT_IRQ]);
    model.setItem(row++, 1, new QStandardItem(text));
    text = QString("%1").arg(status.count[INT_FIRQ]);
    model.setItem(row++, 1, new QStandardItem(text));
    text = QString("%1").arg(status.count[INT_NMI]);
    model.setItem(row++, 1, new QStandardItem(text));
    text = QString("%1").arg(status.count[INT_RESET]);
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

        const auto diskAttributes = fdc->drive_attributes(driveNumber);
        const auto title = tr("Floppy Disk Status");

        OpenDiskStatusDialog(this, title, diskAttributes, driveNumber);
    }
}

void QtGui::CreateStatusBar(QBoxLayout &layout)
{
    // Use QStackedWidget to be able to set a frame style.
    statusBarFrame = new QStackedWidget();
    statusBar = new QStatusBar();
    statusBar->setSizeGripEnabled(false);
    statusBarFrame->addWidget(statusBar);
    layout.addWidget(statusBarFrame, 1);
    statusBarFrame->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    statusBarAction->setChecked(true);
    newKeyFrame = new QStackedWidget();
    newKeyLabel = new QLabel(this);
    newKeyLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    newKeyLabel->setToolTip(tr("Last key entered"));
    const auto defaultFont = QApplication::font();
    const auto pointSize = defaultFont.pointSize();
    auto font = GetMonospaceFont(pointSize);
    font.setWeight(QFont::Bold);
    newKeyLabel->setFont(font);
    newKeyLabel->setText("00  ");
    newKeyFrame->addWidget(newKeyLabel);
    newKeyFrame->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    layout.addWidget(newKeyFrame);
    dummyStatusBar = new QStatusBar(this);
    dummyStatusBar->setMaximumWidth(14);
    dummyStatusBar->setSizeGripEnabled(true);
    layout.addWidget(dummyStatusBar);

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
    if ((options.isConfirmExit && isConfirmClose) || isRestartNeeded)
    {
        auto message = isRestartNeeded ?
            tr("Do you want to restart %1 now?") :
            tr("Do you want to close %1?");
        message = message.arg(PROGRAMNAME);
        auto result = QMessageBox::question(this, tr("Flexemu"), message,
                          QMessageBox::Yes | QMessageBox::No);
        return (result == QMessageBox::Yes);
    }

    return true;
}

void QtGui::redraw_cpuview_impl(const Mc6809CpuStatus &/*status*/)
{
    assert(cpuDialog != nullptr);

    cpuUi.e_status->setText(QString::fromStdString(cpustring));
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

void QtGui::UpdateDiskStatus(Word floppyIndex, DiskStatus oldStatus,
        DiskStatus newStatus)
{
    auto fct_getStatusText = [&](Word idx){
        const auto diskAttributes = fdc->drive_attributes(idx);
        return tr("Disk #%1 %2")
            .arg(idx)
            .arg(diskAttributes.GetIsFlexFormat() ?
                QString::fromStdString(diskAttributes.GetDiskname()) :
                QString::fromStdString(diskAttributes.GetPath().filename()
                    .u8string()));
    };

    if (HasFloppy())
    {
        QString text;
        assert(static_cast<size_t>(floppyIndex) < diskStatusAction.size());

        switch (newStatus)
        {
            case DiskStatus::EMPTY:
                text = tr("Disk #%1 not ready").arg(floppyIndex);
                diskStatusAction[floppyIndex]->setText(text);
                diskStatusAction[floppyIndex]->setIcon(iconNoFloppy);
                break;

            case DiskStatus::INACTIVE:
                if (oldStatus == DiskStatus::EMPTY)
                {
                    text = fct_getStatusText(floppyIndex);
                    diskStatusAction[floppyIndex]->setText(text);
                }
                diskStatusAction[floppyIndex]->setIcon(iconInactiveFloppy);
                break;

            case DiskStatus::ACTIVE:
                if (oldStatus == DiskStatus::EMPTY)
                {
                    text = fct_getStatusText(floppyIndex);
                    diskStatusAction[floppyIndex]->setText(text);
                }
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
    auto frequency = isOriginalFrequency ? ORIGINAL_FREQUENCY : 0.0F;
    SetCpuFrequency(frequency);
}

void QtGui::SetCpuFrequency(float frequency)
{
    options.frequency = frequency;
    oldOptions.frequency = frequency;
    scheduler.sync_exec(BCommandSPtr(new CSetFrequency(scheduler, frequency)));
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

void QtGui::ToggleSmoothDisplay()
{
    e2screen->ToggleSmoothDisplay();
    UpdateSmoothDisplayCheck();
    QTimer::singleShot(0, this, &QtGui::OnRepaintScreen);

    oldOptions.isSmooth = e2screen->IsSmoothDisplay();
    options.isSmooth = e2screen->IsSmoothDisplay();
    WriteOneOption(options, FlexemuOptionId::IsDisplaySmooth);
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
    SetStatusBarVisibility(!statusBarFrame->isVisible());
    UpdateStatusBarCheck();

    oldOptions.isStatusBarVisible = statusBarFrame->isVisible();
    options.isStatusBarVisible = statusBarFrame->isVisible();
    WriteOneOption(options, FlexemuOptionId::IsStatusBarVisible);

    QTimer::singleShot(0, this, &QtGui::OnResize);
}

void QtGui::SetStatusBarVisibility(bool isVisible)
{
    statusBarFrame->setVisible(isVisible);
    newKeyFrame->setVisible(isVisible);
    dummyStatusBar->setVisible(isVisible);
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
    auto heightDiff = 2 * iconSize.height() - toolBar->iconSize().height() -
        statusToolBar->iconSize().height();

    toolBar->setIconSize(iconSize);
    statusToolBar->setIconSize(iconSize);
    printOutputWindow->SetIconSize(iconSize);
    memoryWindowMgr.SetIconSize(iconSize);

    resize(size() + QSize(0, heightDiff));
}

void QtGui::SetIconSizeCheck(const QSize &iconSize)
{
    const int sizeIndex = IconSizeToIndex(iconSize);

    for (int index = 0; index < ICON_SIZES; ++index)
    {
        auto *action = iconSizeAction[index];
        assert(action != nullptr);
        action->setChecked(index == sizeIndex);
    }

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
    // To adjust the window size resize() has to be processed
    // within the event loop, e.g. with a singleShot timer.
    // See also: QtGui::AdjustSize().
    resize(sizeHint());
}

QUrl QtGui::CreateDocumentationUrl(const QString &docDir,
                                   const QString &htmlFile)
{
    auto path = QDir(docDir).filePath(htmlFile);

    return QUrl::fromLocalFile(path);
}

void QtGui::SetCpuDialogMonospaceFont(int pointSize)
{
    auto monospaceFont = GetMonospaceFont(pointSize);
    cpuUi.e_status->setFont(monospaceFont);
    QFontMetrics monospaceFontMetrics(monospaceFont);
    auto fHeight = monospaceFontMetrics.height() * (CPU_LINES + 0.5);
    auto height = static_cast<int>(std::lround(fHeight));
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
    using fn = std::function<QRgb(Byte)>;

    const auto optionsColor = flx::tolower(options.color);
    bool isWithColorScale = !optionsColor.compare("default");
    Word redBase = 255;
    Word greenBase = 255;
    Word blueBase = 255;
    colorTable.resize(MAX_COLORS);

    if (!isWithColorScale)
    {
        flx::getRGBForName(options.color, redBase, greenBase, blueBase);
    }

    fn GetColor = [&](Byte index) -> QRgb
    {
        // Use same color values as Enhanced Graphics Adapter (EGA)
        // or Tandy Color Computer 3 RGB.
        // For details see:
        // https://en.wikipedia.org/wiki/Enhanced_Graphics_Adapter
        // https://exstructus.com/tags/coco/australia-colour-palette/
        constexpr static std::array<Byte, 4> colorValues{
            0x00, 0x55, 0xAA, 0xFF
        };
        unsigned scale;

        // Create a color scale in the range of 0 - 3 based two color bits
        // <color>_HIGH and <color>_LOW. Convert the color scale into a
        // color value in the range of 0 - 255.
        scale = index & RED_HIGH ? 2U : 0U;
        scale |= index & RED_LOW ? 1U : 0U;
        auto red = colorValues[scale];
        scale = index & GREEN_HIGH ? 2U : 0U;
        scale |= index & GREEN_LOW ? 1U : 0U;
        auto green = colorValues[scale];
        scale = index & BLUE_HIGH ? 2U : 0U;
        scale |= index & BLUE_LOW ? 1U : 0U;
        auto blue = colorValues[scale];

        return qRgb(red, green, blue);
    };

    fn GetColorShade = [&](Byte index) -> QRgb
    {
        auto dIndex = static_cast<double>(index);
        auto red = static_cast<Byte>(redBase * sqrt(dIndex / (MAX_COLORS - 1)));
        auto green = static_cast<Byte>(greenBase * sqrt(dIndex /
                (MAX_COLORS - 1)));
        auto blue = static_cast<Byte>(blueBase * sqrt(dIndex /
                (MAX_COLORS - 1)));

        return qRgb(red, green, blue);
    };

    fn GetTheColor = (isWithColorScale ? GetColor : GetColorShade);

    for (Byte i = 0; i < static_cast<Byte>(colorTable.size()); ++i)
    {
        int idx = options.isInverse ?
                  cast_from_qsizetype(colorTable.size()) - i - 1 : i;
        colorTable[idx] = GetTheColor(i);
    }

    return colorTable;
}

void QtGui::CopyToBMPArray(Word height, QByteArray& dest,
                           Byte const *videoRam,
                           const ColorTable& p_colorTable)
{
    sBITMAPFILEHEADER fileHeader{};
    sBITMAPINFOHEADER infoHeader{};

    assert(height >= 1);
    assert(!p_colorTable.empty());

    // Size of BMP stream:
    // BITMAPFILEHEADER + BITMAPINFOHEADER + color table size + pixel data
    DWord dataOffset = sizeof(fileHeader) + sizeof(infoHeader) +
                     (static_cast<DWord>(p_colorTable.size()) *
                      sizeof(sRGBQUAD));
    auto destSize = static_cast<SDWord>(dataOffset + (height * WINDOWWIDTH));

    dest.clear();
    dest.resize(destSize);

    auto *pData = dest.data();

    fileHeader.type[0] = 'B';
    fileHeader.type[1] = 'M';
    fileHeader.fileSize = flx::toLittleEndian<DWord>(destSize);
    fileHeader.reserved[0] = 0U;
    fileHeader.reserved[1] = 0U;
    fileHeader.dataOffset = flx::toLittleEndian<DWord>(dataOffset);
    memcpy(pData, &fileHeader, sizeof(fileHeader));
    pData += sizeof(fileHeader);

    infoHeader.size = flx::toLittleEndian<DWord>(sizeof(infoHeader));
    infoHeader.width = flx::toLittleEndian<SDWord>(WINDOWWIDTH);
    infoHeader.height = flx::toLittleEndian<SDWord>(height);
    infoHeader.planes = flx::toLittleEndian<Word>(1U);
    infoHeader.bitCount = flx::toLittleEndian<Word>(8U);
    infoHeader.compression = flx::toLittleEndian<DWord>(BI_RGB);
    infoHeader.imageSize = flx::toLittleEndian<DWord>(height * WINDOWWIDTH);
    infoHeader.xPixelsPerMeter = 0;
    infoHeader.yPixelsPerMeter = 0;
    infoHeader.colorsUsed =
        flx::toLittleEndian<DWord>(static_cast<DWord>(p_colorTable.size()));
    infoHeader.colorsImportant =
        flx::toLittleEndian<DWord>(static_cast<DWord>(p_colorTable.size()));
    memcpy(pData, &infoHeader, sizeof(infoHeader));
    pData += sizeof(infoHeader);

    assert(p_colorTable.size() <=
           static_cast<ColorTable::size_type>(MAX_COLORS));

    const auto size = static_cast<int>(sizeof(sRGBQUAD)) *
                      p_colorTable.size();
    const auto hash = qHashRange(p_colorTable.begin(), p_colorTable.end());
    if (colorTablesCache.contains(hash))
    {
        memcpy(pData, colorTablesCache.value(hash).data(), size);
        pData += size;
    }
    else
    {
        QByteArray colorTableCache(size, '\0');
        sRGBQUAD colorEntry{};
        const auto *pDataOrigin = pData;

        for (const auto rgbColor : p_colorTable)
        {
            colorEntry.red = static_cast<Byte>(qRed(rgbColor));
            colorEntry.green = static_cast<Byte>(qGreen(rgbColor));
            colorEntry.blue = static_cast<Byte>(qBlue(rgbColor));
            colorEntry.reserved = '\0';
            memcpy(pData, &colorEntry, sizeof(colorEntry));
            pData += sizeof(colorEntry);
        }
        memcpy(colorTableCache.data(), pDataOrigin, size);
        colorTablesCache.insert(hash, colorTableCache);
    }
    assert(pData == dest.data() + dataOffset);

    std::array<Byte, 6> pixels{}; /* One byte of video RAM for each plane */
    // Default color index: If no video source is available use highest
    // available color
    Byte colorIndex = options.isInverse ? 0x00U : 0x3FU;
    Byte colorIndexOffset = 0U;
    if (options.isInverse)
    {
        colorIndexOffset = static_cast<Byte>((64U / options.nColors) - 1U);
    }

    // The raster lines have to be filled from bottom to top.
    pData += WINDOWWIDTH * (height - 1);
    for (auto count = 0; count < (RASTERLINE_SIZE * height); ++count)
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
            for (pixelBitMask = 0x80U; pixelBitMask; pixelBitMask >>= 1U)
            {
                colorIndex = colorIndexOffset; /* calculated color index */

                if (pixels[0] & pixelBitMask)
                {
                    colorIndex += GREEN_HIGH; // 0x0C, green high
                }

                if (options.nColors > 8)
                {
                    if (pixels[2] & pixelBitMask)
                    {
                        colorIndex += RED_HIGH; // 0x0D, red high
                    }

                    if (pixels[4] & pixelBitMask)
                    {
                        colorIndex += BLUE_HIGH; // 0x0E, blue high
                    }

                    if (pixels[1] & pixelBitMask)
                    {
                        colorIndex += GREEN_LOW; // 0x04, green low
                    }

                    if (pixels[3] & pixelBitMask)
                    {
                        colorIndex += RED_LOW; // 0x05, red low
                    }

                    if (pixels[5] & pixelBitMask)
                    {
                        colorIndex += BLUE_LOW; // 0x06, blue low
                    }
                }
                else
                {
                    if (pixels[2] & pixelBitMask)
                    {
                        colorIndex += RED_HIGH; // 0x0D, red high
                    }

                    if (pixels[4] & pixelBitMask)
                    {
                        colorIndex += BLUE_HIGH; // 0x0E, blue high
                    }
                }
                *(pData)++ = static_cast<char>(colorIndex);
            }
        }
        else
        {
            for (pixelBitMask = 0x80U; pixelBitMask; pixelBitMask >>= 1U)
            {
                *(pData)++ = static_cast<char>(colorIndex);
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
        auto *statusTipEvent = dynamic_cast<QStatusTipEvent *>(event);
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
        auto defaultFont = QApplication::font();
        auto newPointSize = defaultFont.pointSize();

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

void QtGui::showEvent(QShowEvent * /*event*/)
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
                    osName.clear();
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
        e2screen->Detach(*this);
        scheduler.request_new_state(CpuState::Exit);
        while (!scheduler.is_finished())
        {
            scheduler.timer_elapsed();
            QThread::msleep(10);
        }
        timer.stop();
        if (printOutputWindow != nullptr)
        {
            printOutputWindow->close();
            printOutputWindow = nullptr;
        }
        memoryWindowMgr.CloseAllWindows();
        FlexemuOptionsDifference optionsDiff(options, oldOptions);

        if (!optionsDiff.GetNotEquals().empty())
        {
            FlexemuOptions::WriteOptions(options, false, true);
        }
        event->accept();
        emit CloseApplication();
    }
    else
    {
        event->ignore();
    }
}

// Implementation may change in future.
// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
void QtGui::WriteOneOption(sOptions p_options, FlexemuOptionId optionId) const
{
    p_options.readOnlyOptionIds = GetAllFlexemuOptionIds();
    p_options.readOnlyOptionIds.erase(
        std::remove(p_options.readOnlyOptionIds.begin(),
                    p_options.readOnlyOptionIds.end(),
                    optionId),
        p_options.readOnlyOptionIds.end());
    FlexemuOptions::WriteOptions(p_options, false, true);
}

void QtGui::write_char_serial(Byte value)
{
    printOutputWindow->write_char_serial(value);
}

// Implementation may change in future.
// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
std::string QtGui::GetKeyString(Byte key)
{
    const std::array<const char *, 32> code{
        "NUL", "SOH", "STX", "ETX", "EOT", "ENQ", "ACK", "BEL", "BS",
        "HT", "LF", "VT", "FF", "CR", "SO", "SI", "DLE", "DC1", "DC2",
        "DC3", "DC4", "NAK", "SYN", "ETB", "CAN", "EM", "SUB", "ESC",
        "FS", "GS", "RS", "US",
    };

    auto cKey = static_cast<char>(key);
    auto ctrlCh = "CTRL-" + std::string(1U, static_cast<char>(cKey + '@'));
    auto ch = (key < ' ') ? ctrlCh : "";
    ch = (ch.empty() && key <= '~') ? std::string(1U, cKey) : ch;
    ch = ch.empty() ? std::string(1U, ' ') : ch;
    std::string sCode = (key < ' ') ? code[key] :
        ((key == '\x7F') ? "DEL" : "");

    return (!sCode.empty()) ?
        fmt::format("{:02X} {:6} {:3}", key, ch, sCode) :
        fmt::format("{:02X} {}", key, ch);
}

void QtGui::UpdateFrom(NotifyId id, void *param)
{
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch-enum"
#endif
    if (param != nullptr)
    {
        switch (id)
        {
            case NotifyId::SetFrequency:
                {
                    std::lock_guard<std::mutex> guard(newFrequencyMutex);
                    newFrequency = *static_cast<float *>(param);
                }
                break;

            case NotifyId::KeyPressed:
                {
                    const auto bKey = *static_cast<Byte *>(param);
                    const auto text = GetKeyString(bKey);
                    newKeyLabel->setText(QString::fromStdString(text));
                }
                break;

            case NotifyId::KeyPressedOnCPU:
                {
                    std::lock_guard<std::mutex> guard(newKeysMutex);
                    newKeys.push_back(*static_cast<Byte *>(param));
                }
                break;

            default:
                break;
        }
    }
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
}

void QtGui::ParseRomName()
{
    static const auto flags =
        std::regex_constants::extended | std::regex_constants::icase;
    static const BInterval<DWord> romAddrRange(0xF000U, 0xFFFFU);

    if (!flx::is_range_in_ranges(romAddrRange, memory.GetAddressRanges()) ||
        !isParseRomName || !romName.empty())
    {
        return;
    }

    if (!copyRomCommand)
    {
        copyRomCommand = std::make_shared<CCopyMemory>(memory, romAddrRange);
        scheduler.sync_exec(
            std::dynamic_pointer_cast<BCommand>(copyRomCommand));
    }
    else
    {
        static const std::regex rom_name_regex(".*(eurocom.*)", flags);
        romName = flx::find_regex_string(rom_name_regex, '\x04',
            copyRomCommand->GetData());
        isParseRomName = false; // Parse ROM name only once.
    }
}

void QtGui::ParseOsName()
{
    static const auto flags =
        std::regex_constants::extended | std::regex_constants::icase;
    static const BInterval<DWord> flexAddrRange(0xC700, 0xDFFF);

    if (!flx::is_range_in_ranges(flexAddrRange, memory.GetAddressRanges()) ||
        !osName.empty())
    {
        return;
    }

    if (!copyOsCommand)
    {
        copyOsCommand = std::make_shared<CCopyMemory>(memory, flexAddrRange);
        scheduler.sync_exec(std::dynamic_pointer_cast<BCommand>(copyOsCommand));
    }
    else
    {
        static const std::regex os_name_regex(".*(flex.*)", flags);
        osName = flx::find_regex_string(os_name_regex, '\x04',
            copyOsCommand->GetData());
        if (osName.empty())
        {
            scheduler.sync_exec(
                std::dynamic_pointer_cast<BCommand>(copyOsCommand));
        }
    }
}

void QtGui::RequestMemoryUpdate()
{
    memoryWindowMgr.RequestMemoryUpdate(scheduler);
}

ItemPairList_t QtGui::GetConfiguration() const
{
    ItemPairList_t result;
    std::vector<std::string> values;

    result.emplace_back("Mainboard", std::vector(1U, GetMainboardName()));
    values = { cpu.get_vendor() + " " + cpu.get_name() };
    result.emplace_back("CPU", values);
    values = { std::to_string(memory.get_ram_size()) + " KByte" };
    result.emplace_back("RAM", values);
    values = { "none" };
    if (memory.get_ram_extension_size() > 0U)
    {
        values = { std::to_string(memory.get_ram_extension_boards()) + " x " +
            std::to_string(memory.get_ram_extension_size()) + " KByte" };
    }
    result.emplace_back("RAM extension", values);
    result.emplace_back("Boot ROM", std::vector(1U, romName));
    if (!osName.empty())
    {
        result.emplace_back("Operating system", std::vector(1U, osName));
    }
    const auto size = e2screen->baseSize();
    values = { std::to_string(size.width()) + " x " +
        std::to_string(size.height()) + " pixels, " +
        std::to_string(options.nColors) + " colors" };
    result.emplace_back("Graphics", values);
    values.clear();
    for (const auto &device_props : memory.get_devices_properties())
    {
        std::stringstream strdevice;

        strdevice << std::uppercase <<
            std::setw(4) << std::hex << device_props.addressRange.lower();
        if (!singleton(device_props.addressRange))
        {
            strdevice << "-" << std::setw(4) << std::hex <<
                device_props.addressRange.upper();
        }
        strdevice << " " << device_props.name;
        values.emplace_back(strdevice.str());
    }
    result.emplace_back("Devices", values);

    return result;
}

std::string QtGui::GetMainboardName() const
{
    return options.isEurocom2V5 ? "Eurocom II/V5" : "Eurocom II/V7";
}

void QtGui::ForceRestart()
{
    e2screen->Detach(*this);

    if (close())
    {
        // Force restart flexemu
        QApplication::exit(EXIT_RESTART);
    }
}

void QtGui::GotIllegalInstruction(const Mc6809CpuStatus &status)
{
    auto pc = QString("%1").arg(status.pc, 4, 16, QChar('0')).toUpper();
    QStringList hexBytes;
    for (Byte byte : status.instruction)
    {
        auto hexString = QString("%1").arg(byte, 2, 16, QChar('0'));
        hexBytes.append(hexString.toUpper());
    }
    auto buttons = QMessageBox::Close | QMessageBox::Reset;
    auto message = tr(
        "CPU got an invalid instruction and has stopped.<br>"
        "PC=%1 opcode=%2.<br>"
        "<br>&#x2022; <b>Close</b> will close flexemu."
        "<br>&#x2022; <b>Reset</b> will execute a reset and reboot.")
            .arg(pc).arg(hexBytes.join(QChar(' ')));

    if (!FlexemuOptions::AreAllBootOptionsReadOnly(options))
    {
        message += tr("<br>&#x2022; <b>Restore Defaults</b> will restore "
                      "boot ROM file and/or boot disk to default values and "
                      "restart flexemu.");
        buttons |= QMessageBox::RestoreDefaults;
    }

    if (isVisible())
    {
        message += tr("<br>&#x2022; <b>Ok</b> will close this dialog.");
        buttons |= QMessageBox::Ok;
    }


    QTimer::singleShot(0, this, [&, message, buttons]()
    {
        const auto answer =
            QMessageBox::critical(this, "flexemu error", message, buttons,
                QMessageBox::Close);

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch-enum"
#pragma GCC diagnostic ignored "-Wswitch"
#endif
        switch (answer)
        {
            case QMessageBox::RestoreDefaults:
                FlexemuOptions::InitBootOptions(options);
                FlexemuOptions::WriteOptions(options, false, true);
                isConfirmClose = false;
                ForceRestart();
                break;

            case QMessageBox::Reset:
                OnCpuResetRun();
                break;

            case QMessageBox::Close:
                isConfirmClose = false;
                close();
                break;

            case QMessageBox::Ok:
                break;
        };
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
    });
}
