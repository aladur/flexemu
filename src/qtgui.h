/*
    qtgui.h  Platform independent user interface


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2020-2024  W. Schwotzer

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

#ifndef QTGUI_INCLUDED
#define QTGUI_INCLUDED

#include "misc1.h"
#include "flexemu.h"
#include "absgui.h"
#include "schedcpu.h"
#include "scpulog.h"
#include "soptions.h"
#include "e2.h"
#include <vector>
#include <string>
#include <memory>
#include "warnoff.h"
#include "cpustat_ui.h"
#include <QWidget>
#include <QIcon>
#include <QTimer>
#include <QString>
#include <QByteArray>
#include <QMap>
#include "warnon.h"

class Mc6809;
class Memory;
class Scheduler;
class Inout;
class VideoControl1;
class VideoControl2;
struct Mc6809CpuStatus;
class JoystickIO;
class KeyboardIO;
class TerminalIO;
class Pia1;
class E2floppy;
class E2Screen;
class FlexDiskAttributes;
class FlexemuOptionsDifference;
class PrintOutputWindow;
class QAction;
class QSize;
class QString;
class QWidget;
class QEvent;
class QKeyEvent;
class QShowEvent;
class QResizeEvent;
class QCloseEvent;
class QLayout;
class QVBoxLayout;
class QHBoxLayout;
class QMenuBar;
class QToolBar;
class QStatusBar;
class QStackedWidget;
class QComboBox;
struct sOptions;

using ColorTable = QVector<QRgb>;

// Cache containing color tables identified by a hash value.
// The hash value is the key which depends on the Qt version.
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
using ColorTablesCache = QHash<std::size_t, QByteArray>;
#else
using ColorTablesCache = QHash<uint, QByteArray>;
#endif

class QtGui : public QWidget, public AbstractGui
{
    Q_OBJECT

public:
    QtGui() = delete;
    QtGui(
        Mc6809 & p_cpu,
        Memory & p_memory,
        Scheduler & p_scheduler,
        Inout & p_inout,
        VideoControl1 & p_vico1,
        VideoControl2 & p_vico2,
        JoystickIO & p_joystickIO,
        KeyboardIO & p_keyboardIO,
        TerminalIO & p_terminalIO,
        Pia1 & p_pia1,
        struct sOptions & p_options);
    ~QtGui() override;
    QtGui(const QtGui &src) = delete;
    QtGui(QtGui &&src) = delete;
    QtGui &operator=(const QtGui &&src) = delete;
    QtGui &operator=(QtGui &&src) = delete;

    void SetFloppy(E2floppy *fdc);
    bool HasFloppy() const;
    void output_to_graphic() override;
    void write_char_serial(Byte value) override;

signals:
    void CloseApplication();

protected:
    void redraw_cpuview_impl(const Mc6809CpuStatus &status) override;

private slots:
    void OnPrinterOutput();
    void OnExit();
    void OnPreferences();
    void OnFullScreen();
    void OnRepaintScreen();
    void OnStatusBar();
    void OnSmoothDisplay();
    void OnCpuRun();
    void OnCpuStop();
    void OnCpuReset();
    void OnCpuStep();
    void OnCpuNext();
    void OnCpuResetRun();
    void OnCpuBreakpoints();
    void OnCpuLogging();
    void OnCpuDialogToggle();
    void OnCpuDialogClose();
    void OnCpuOriginalFrequency();
    void OnCpuUndocumentedInstructions();
    void OnCpuInterruptStatus();
    void OnIntroduction() const;
    void OnDiskStatus(Word driveNumber);
    void OnAbout();
    void OnTimer();
    void OnIconSize(int index);
    void OnScreenSize(int index);
    void OnScreenSizeHighlighted(int index) const;
    void AdjustSize();
    void OnResize();

private:
    QToolBar *CreateToolBar(QWidget *parent, const QString &title,
                            const QString &objectName);
    void CreateIcons();
    void CreateActions(QLayout &layout);
    void CreateFileActions(QLayout &layout);
    void CreateEditActions(QLayout &layout);
    void CreateViewActions(QLayout &layout);
    void CreateCpuActions(QLayout &layout);
    void CreateHelpActions(QLayout &layout);
    static void CreateHorizontalSpacer(QLayout &layout);
    QAction *CreateScreenSizeAction(const QIcon &icon, QMenu &menu,
                                    uint16_t index);
    QAction *CreateIconSizeAction(QMenu &menu, uint16_t index);
    void CreateStatusToolBar(QLayout &layout);
    void CreateStatusBar(QLayout &layout);
    void AddDiskStatusButtons();
    void ConnectCpuUiSignalsWithSlots();
    void SetStatusMessage(const QString &message) const;
    bool IsClosingConfirmed();
    void PopupMessage(const QString &message);
    static void SetBell(int percent);
    void update_block(int blockNumber);
    void UpdateDiskStatus(int floppyIndex, DiskStatus status);
    void UpdateInterruptStatus(tIrqType irqType, bool status);
    void ToggleSmoothDisplay();
    void ToggleCpuFrequency();
    void ToggleCpuUndocumented();
    void ToggleFullScreenMode();
    void ToggleStatusBarVisibility();
    void ToggleCpuRunStop();
    void SetFullScreenMode(bool isFullScreen);
    bool IsFullScreenMode() const;
    void UpdateFullScreenCheck() const;
    void UpdateSmoothDisplayCheck() const;
    void UpdateStatusBarCheck() const;
    void UpdateCpuFrequencyCheck() const;
    void UpdateCpuRunStopCheck() const;
    void UpdateCpuUndocumentedCheck() const;
    void UpdateScreenSizeCheck(int index) const;
    void UpdateScreenSizeValue(int index) const;
    void SetIconSize(const QSize &size);
    static QUrl CreateDocumentationUrl(const QString &docDir,
                                       const QString &htmlFile);
    ColorTable CreateColorTable();
    void CopyToBMPArray(Word height, QByteArray& dest,
                        Byte const *videoRam, const ColorTable& colorTable);
    int TranslateToAscii(QKeyEvent *event);
    static QFont GetMonospaceFont(int pointSize = -1);
    void SetCpuDialogMonospaceFont(int pointSize);
    void ConnectScreenSizeComboBoxSignalSlots() const;
    static QString GetScreenSizeStatusTip(int index);
    static QString AsString(Word driveNumber, const FlexDiskAttributes &info);
    static QIcon GetPreferencesIcon(bool isRestartNeeded);
    void SetPreferencesStatusText(bool isRestartNeeded) const;
    void WriteOneOption(sOptions options, FlexemuOptionId optionId) const;

    // QWidget Overrides
    bool event(QEvent *event) override;
    void changeEvent(QEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

    std::vector<QIcon> icons;
    QTimer timer;
    QVBoxLayout *mainLayout{};
    QHBoxLayout *toolBarLayout{};
    QHBoxLayout *e2screenLayout{};
    QStackedWidget *statusBarFrame{};
    E2Screen *e2screen{};
    QMenuBar *menuBar{};
    QToolBar *fileToolBar{};
    QToolBar *editToolBar{};
    QToolBar *viewToolBar{};
    QToolBar *cpuToolBar{};
    QToolBar *helpToolBar{};
    QToolBar *statusToolBar{};
    QStatusBar *statusBar{};
    QComboBox *screenSizeComboBox{};
    QDialog *cpuDialog{};
    Ui::CpuStatus cpuUi{};
    QAction *printOutputAction{};
    QAction *exitAction{};
    QAction *preferencesAction{};
    QAction *fullScreenAction{};
    QAction *smoothAction{};
    QAction *statusBarAction{};
    QAction *cpuRunAction{};
    QAction *cpuStopAction{};
    QAction *cpuResetAction{};
    QAction *cpuViewAction{};
    QAction *breakpointsAction{};
    QAction *loggingAction{};
    QAction *originalFrequencyAction{};
    QAction *undocumentedAction{};
    QAction *introductionAction{};
    QAction *aboutAction{};
    QAction *aboutQtAction{};
    std::array<QAction *, MAX_DRIVES> diskStatusAction{};
    std::array<QAction *, ICON_SIZES> iconSizeAction{};
    std::array<QAction *, SCREEN_SIZES> screenSizeAction{};
    QAction *interruptStatusAction{};
    PrintOutputWindow *printOutputWindow{};
    QIcon iconNoFloppy;
    QIcon iconInactiveFloppy;
    QIcon iconActiveFloppy;
    QIcon iconInterrupt;
    QIcon iconIrq;
    QIcon iconFirq;
    QIcon iconNmi;
    QIcon iconReset;
    ColorTable colorTable;
    ColorTablesCache colorTablesCache;
    QByteArray dataBuffer;

    bool isOriginalFrequency{};
    bool isStatusBarVisible{};
    bool isRunning{};
    bool isConfirmClose{};
    bool isForceScreenUpdate{};
    bool isRestartNeeded{};
    int timerTicks{0};
    Byte oldFirstRasterLine{0U};
    Mc6809LoggerConfig cpuLoggerConfig;

    Scheduler &scheduler;
    VideoControl1 &vico1;
    VideoControl2 &vico2;
    JoystickIO &joystickIO;
    KeyboardIO &keyboardIO;
    E2floppy *fdc{};
    sOptions &options;
    sOptions oldOptions{};

    static int preferencesTabIndex;
};
#endif

