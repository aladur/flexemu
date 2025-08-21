/*
    memwin.h


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2025  W. Schwotzer

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

#ifndef MEMORYWINDOW_INCLUDED
#define MEMORYWINDOW_INCLUDED

#include "typedefs.h"
#include "bintervl.h"
#include "free.h"
#include "memtype.h"
#include "warnoff.h"
#include <QObject>
#include <QString>
#include <QEvent>
#include <QWidget>
#include <QList>
#include <QStringList>
#include <QFuture>
#include <QFutureWatcher>
#include <memory>
#include "warnon.h"
#include <vector>
#include <optional>
#include <chrono>


class QSize;
class QCloseEvent;
class QResizeEvent;
class QBoxLayout;
class QVBoxLayout;
class QHBoxLayout;
class QFont;
class QLabel;
class QAction;
class QStackedWidget;
class QMenu;
class QMenuBar;
class QToolBar;
class QStatusBar;
class QIcon;
class QTextEdit;
class QComboBox;
class QPixmap;
class QGraphicsView;
class MemoryWindowTextEdit;

class MemoryWindow : public QWidget
{
    Q_OBJECT

public:

    enum class Style : uint8_t
    {
        Dynamic = 0U, // Depending on window width dynamically set bytes
                      // per line
        Bytes4 = 4U, // 4 bytes per line
        Bytes8 = 8U, // 8 bytes per line
        Bytes16 = 16U, // 16 bytes per line
        Bytes32 = 32U, // 32 bytes per line
        Bytes64 = 64U, // 64 bytes per line
    };

    using StyleValues_t = QList<enum MemoryWindow::Style>;

    struct sConfig
    {
        std::string windowTitle;
        BInterval<DWord> addressRange;
        MemoryWindow::Style style{};
        bool withAddress{};
        bool withAscii{};
        bool withExtraSpace{};
        bool isUpdateWindowSize{};
    };

    using Config_t = struct sConfig;

    MemoryWindow() = delete;
    explicit MemoryWindow(
            bool p_isReadOnly,
            MemoryRanges_t p_availableMemoryRanges,
            Config_t p_config,
            const std::optional<QRect> &positionAndSize = std::nullopt,
            QWidget *p_parent = nullptr);
    MemoryWindow(const MemoryWindow &src) = delete;
    MemoryWindow(MemoryWindow &&src) = delete;
    MemoryWindow &operator=(const MemoryWindow &src) = delete;
    MemoryWindow &operator=(MemoryWindow &&src) = delete;

    // Static functions
    static const StyleValues_t &GetStyleValues();
    static const QStringList &GetStyleStrings();
    static const QStringList &GetStyleShortStrings();
    static const QStringList &GetStyleStatusTips();
    static const QStringList &GetStyleHotKeys();
    static QString GetDefaultWindowTitle();
    static QString CreateWindowTitle(
            const std::string &title,
            const BInterval<DWord> &addressRange);
    static void ConvertConfigString(const std::string &configString,
            Config_t &config, QRect &positionAndSize);

    void SetIconSize(const QSize &iconSize);
    void UpdateData(const std::vector<Byte> &p_data);
    void SetReadOnly(bool p_isReadOnly);
    BInterval<DWord> GetAddressRange() const;
    std::string GetConfigString() const;

public slots:
    void OnClose();
    void OnFontChanged(const QFont &newFont);
    void OnUpdateFont();
    void OnStyleChanged(int index);
    void OnStyleHighlighted(int index) const;
    void OnToggleDisplayAddresses();
    void OnToggleDisplayAscii();
    void OnToggleUpdateWindowSize();
    void OnToggleExtraSpace();
    void OnToggleHexAscii();
    void OnHorizontalScrollBarValueChanged(int value) const;
    void OnTextCursorPositionChanged();
    void OnNotifyKeyPressed(QKeyEvent *event);
    void OnEventTypeChanged(QEvent::Type type);

signals:
    void Closed(MemoryWindow *p_win);
    void MemoryModified(const MemoryWindow *p_win, Word address,
            const std::vector<Byte> &data) const;

protected:
    void CreateActions(QBoxLayout &layout, const QSize &iconSize);
    void CreateFileActions(QToolBar &p_toolBar);
    void CreateViewActions(QToolBar &p_toolBar);
    void CreateStatusBar(QBoxLayout &layout);
    QToolBar *CreateToolBar(QWidget *parent, const QString &title,
                            const QString &objectName, const QSize &iconSize);
    QAction *CreateAction(QIcon *icon, QMenu &menu, const QString &text,
            int index, const QString &statusTip = "",
            const QString &hotKey = "");
    void InitializeStyleWidget();
    void ConnectStyleComboBoxSignalSlots() const;
    void SetTextBrowserFont(const QFont &font);
    void ConnectHexDumpCursorPositionChanged(bool isConnect) const;
    void SetStatusMessage(const QString &message) const;
    DWord EstimateBytesPerLine() const;
    std::optional<DWord> CurrentExtraSpace() const;
    void UpdateData();
    void UpdateDataFinish();
    QString CreateHexDump(DWord bytesPerLine, bool withAscii,
        bool isDisplayAddress, DWord startAddress,
        std::optional<DWord> extraSpace);
    void UpdateStyleCheck(int index) const;
    void UpdateStyleValue(int index) const;
    void RequestResize();
    void DoResize(bool condition);
    void Resize();
    void AdjustTextCursorPosition(
            const flx::sHexDumpProperties &properties, int previousRow) const;
    void DetectAndExecuteChangedValue();
    void HexValueChanged(char ch, DWord address, bool isUpperNibble);
    void AsciiValueChanged(char ch, DWord address);
    void UpdateValidCharacters(DWord address, flx::HexDumpType type) const;
    void UpdateAddressStatus(DWord address);
    void UpdateToggleHexAsciiEnabled() const;
    void ReplaceHexOrAsciiText(const QString &text) const;
    void SetUpdateDataPixmap(const QPixmap &pixmap) const;
    void RecalculateDynamicBytesPerLine(const QSize &size);

    // Event handlers
    void changeEvent(QEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    bool event(QEvent *event) override;

    static const QString &GetStartAddrLiteral();
    static const QString &GetEndAddrLiteral();

private:
    QVBoxLayout *mainLayout{};
    QHBoxLayout *toolBarLayout{};
    QHBoxLayout *statusBarLayout{};
    QStackedWidget *statusBarFrame{};
    QMenuBar *menuBar{};
    QToolBar *toolBar{};
    QMenu *fileMenu{};
    QMenu *viewMenu{};
    QStatusBar *statusBar{};
    QStatusBar *dummyStatusBar{};
    QStackedWidget *updateLedFrame{};
    QStackedWidget *memoryTypeFrame{};
    QStackedWidget *addressFrame{};
    QGraphicsView *updateLedGraphics{};
    QLabel *memoryTypeLabel{};
    QLabel *addressLabel{};
    QTextEdit *e_hexDumpScale{};
    MemoryWindowTextEdit *e_hexDump{};
    QComboBox *styleComboBox{};
    QAction *withAddressAction{};
    QAction *withAsciiAction{};
    QAction *isUpdateWindowSizeAction{};
    QAction *withExtraSpaceAction{};
    QAction *toggleHexAsciiAction{};
    QList<QAction *> styleActions;
    QFuture<QString> hexDumpFuture;
    QFutureWatcher<QString> hexDumpWatcher;
    const MemoryRanges_t availableMemoryRanges;
    Config_t config;
    DWord dynamicBytesPerLine{};
    bool isReadOnly{};
    bool isIgnoreResizeEvent{};
    QEvent::Type lastEventType{QEvent::None};
    char lastKeyPressedCharacter{'\0'};
    int lastKeyPressedKey{Qt::Key_unknown};
    int columns{};
    int rows{};
    int currentColumn{};
    int currentRow{};
    DWord currentAddress{};
    std::chrono::time_point<std::chrono::system_clock> updateDataStartTime;
    flx::HexDumpType currentType{};
    bool currentIsUpperNibble{true};
    bool isRequestResize{};
    std::vector<Byte> data;
};

using MemoryWindowSPtr = std::shared_ptr<MemoryWindow>;
#endif
