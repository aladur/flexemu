/*
    memwin.cpp


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


#include "typedefs.h"
#include "misc1.h"
#include "memwin.h"
#include "bintervl.h"
#include "free.h"
#include "mwtedit.h"
#include "warnoff.h"
#include <QtGlobal>
#include <QAbstractEventDispatcher>
#include <QPointer>
#include <QSize>
#include <QSizePolicy>
#include <QLatin1Char>
#include <QString>
#include <QStringList>
#include <QApplication>
#include <QGuiApplication>
#include <QTimer>
#include <QScreen>
#include <QAction>
#include <QKeySequence>
#include <QBoxLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPalette>
#include <QFont>
#include <QFontMetrics>
#include <QWidget>
#include <QFrame>
#include <QLabel>
#include <QStackedWidget>
#include <QAbstractSlider>
#include <QIcon>
#include <QMenu>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QTextEdit>
#include <QComboBox>
#include <QScrollBar>
#include <QTextCursor>
#include <QEvent>
#include <QResizeEvent>
#include <QStatusTipEvent>
#include <QCloseEvent>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QPixmap>
#include <QIODevice>
#include <QtConcurrent>
#include <fmt/format.h>
#include "warnon.h"
#include "qtfree.h"
#include <algorithm>
#include <optional>
#include <cmath>
#include <cassert>
#include <sstream>
#include <string>
#include <chrono>


/****************************
** Constructor, Destructor **
****************************/

MemoryWindow::MemoryWindow(
        bool p_isReadOnly,
        MemoryRanges_t p_availableMemoryRanges,
        Config_t p_config,
        const std::optional<QRect> &positionAndSize,
        QWidget * /*p_parent*/)
    : QWidget(nullptr, Qt::Window)
    , mainLayout(new QVBoxLayout(this))
    , toolBarLayout(new QHBoxLayout)
    , statusBarLayout(new QHBoxLayout)
    , menuBar(new QMenuBar(this))
    , e_hexDumpScale(new QTextEdit(this))
    , e_hexDump(new MemoryWindowTextEdit(this))
    , styleComboBox(new QComboBox(this))
    , availableMemoryRanges(std::move(p_availableMemoryRanges))
    , config(std::move(p_config))
    , dynamicBytesPerLine(16)
    , isReadOnly(p_isReadOnly)
    , isIgnoreResizeEvent(config.style == Style::Dynamic &&
            !positionAndSize.has_value())
    , isRequestResize(!positionAndSize.has_value())
{
    const QSize iconSize(16, 16);

    setObjectName("MemoryWindow");
    auto title = CreateWindowTitle(config.windowTitle, config.addressRange);
    title = tr("Memory") + " - " + title;
    setWindowTitle(title);

    mainLayout->setObjectName(QString::fromUtf8("mainLayout"));
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    setLayout(mainLayout);

    mainLayout->addWidget(menuBar);
    toolBarLayout->setObjectName(QString::fromUtf8("toolBarLayout"));
    toolBarLayout->setContentsMargins(4, 2, 4, 2);
    toolBarLayout->setSpacing(2);
    mainLayout->addLayout(toolBarLayout);

    e_hexDumpScale->setFocusPolicy(Qt::NoFocus);
    e_hexDumpScale->setLineWrapMode(QTextEdit::NoWrap);
    e_hexDumpScale->setAutoFillBackground(true);
    e_hexDumpScale->setBackgroundRole(QPalette::Base);
    e_hexDumpScale->setReadOnly(true);
    e_hexDumpScale->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    e_hexDumpScale->horizontalScrollBar()->setMaximum(0);
    e_hexDumpScale->setText("\n");
    e_hexDumpScale->setContextMenuPolicy(Qt::NoContextMenu);
    mainLayout->addWidget(e_hexDumpScale);

    e_hexDump->setMinimumSize(64, 48);
    e_hexDump->setLineWrapMode(QTextEdit::NoWrap);
    e_hexDump->setAutoFillBackground(true);
    e_hexDump->setBackgroundRole(QPalette::Base);
    e_hexDump->setOverwriteMode(true);
    e_hexDump->setReadOnly(isReadOnly);
    e_hexDump->setContextMenuPolicy(Qt::NoContextMenu);
    e_hexDump->setTabChangesFocus(true);
    mainLayout->addWidget(e_hexDump);
    mainLayout->setStretchFactor(e_hexDump, 1);
    e_hexDump->setFocus(Qt::OtherFocusReason);
    ConnectHexDumpCursorPositionChanged(true);
    connect(e_hexDump->horizontalScrollBar(),
#if (QT_VERSION <= QT_VERSION_CHECK(5, 7, 0))
            static_cast<void (QAbstractSlider::*)(int)>(
                &QAbstractSlider::valueChanged),
#else
            QOverload<int>::of(&QAbstractSlider::valueChanged),
#endif
            this, &MemoryWindow::OnHorizontalScrollBarValueChanged);
    // A QTextEdit widget by default handles Ctrl+A key to select all
    // if readOnly is false. This hexDump widget instead should toggle
    // displaying ASCII.
    // To support this for Ctrl+A key press a special signal is emitted.
    connect(e_hexDump, &MemoryWindowTextEdit::NotifyKeyPressed,
        this, &MemoryWindow::OnNotifyKeyPressed);
    connect(e_hexDump, &MemoryWindowTextEdit::EventTypeChanged,
        this, &MemoryWindow::OnEventTypeChanged);

    CreateActions(*toolBarLayout, iconSize);
    const auto name = QString::fromUtf8("statusBarLayout");
    statusBarLayout->setObjectName(name);
    statusBarLayout->setContentsMargins(0, 0, 0, 0);
    statusBarLayout->setSpacing(2);
    mainLayout->addLayout(statusBarLayout);
    CreateStatusBar(*statusBarLayout);
    ConnectStyleComboBoxSignalSlots();

    toolBarLayout->addStretch(1);

    QTimer::singleShot(0, this, &MemoryWindow::OnUpdateFont);

    const auto memoryIcon = QIcon(":/resource/memory.png");
    setWindowIcon(memoryIcon);

    if (positionAndSize.has_value())
    {
        setGeometry(positionAndSize.value());
    }
}

/*****************
** Public Slots **
*****************/

// Implementation may change in future.
// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
void MemoryWindow::OnFontChanged(const QFont &newFont)
{
    SetTextBrowserFont(newFont);
}

void MemoryWindow::OnUpdateFont()
{
    const auto pointSize = QApplication::font().pointSize();
    auto font = GetMonospaceFont(pointSize);
    SetTextBrowserFont(font);
}

void MemoryWindow::OnStyleChanged(int index)
{
    if (index >= 0 && index < GetStyleValues().size())
    {
        config.style = GetStyleValues()[index];
        isUpdateWindowSizeAction->setEnabled(config.style != Style::Dynamic);
        UpdateStyleCheck(index);
        UpdateStyleValue(index);
        RequestResize();
        UpdateData();
        e_hexDump->setFocus(Qt::OtherFocusReason);
    }
}

// Implementation may change in future.
// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
void MemoryWindow::OnStyleHighlighted(int index) const
{
    const auto &statusTips = GetStyleStatusTips();

    if (index >= 0 && index < statusTips.size())
    {
        const auto &message = statusTips[index];
        SetStatusMessage(message);
    }
}

void MemoryWindow::OnClose()
{
    close();
}

void MemoryWindow::OnToggleDisplayAddresses()
{
    config.withAddress = !config.withAddress;
    RequestResize();
    UpdateData();
}

void MemoryWindow::OnToggleDisplayAscii()
{
    config.withAscii = !config.withAscii;
    withAsciiAction->setChecked(config.withAscii);
    UpdateToggleHexAsciiEnabled();
    RequestResize();
    UpdateData();
}

void MemoryWindow::OnToggleUpdateWindowSize()
{
    config.isUpdateWindowSize = !config.isUpdateWindowSize;
    RequestResize();
    UpdateData();
}

void MemoryWindow::OnToggleExtraSpace()
{
    config.withExtraSpace = !config.withExtraSpace;
    RequestResize();
    UpdateData();
}

void MemoryWindow::OnToggleHexAscii()
{
    const auto rowCol = flx::get_hex_dump_position_for_address(
        currentAddress, data.size(), EstimateBytesPerLine(), config.withAscii,
        config.withAddress, currentType != flx::HexDumpType::AsciiChar,
        true, config.addressRange.lower(), CurrentExtraSpace());

    if (rowCol.has_value())
    {
        const auto position = static_cast<int>(
                (columns + 1U) *
                (rowCol.value().first ? rowCol.value().first : 0U) +
                 rowCol.value().second);

        auto cursor = e_hexDump->textCursor();
        cursor.setPosition(position);
        e_hexDump->setTextCursor(cursor);
    }
}

void MemoryWindow::OnHorizontalScrollBarValueChanged(int value) const
{
    e_hexDumpScale->horizontalScrollBar()->setValue(value);
}

void MemoryWindow::OnTextCursorPositionChanged()
{
    if (columns == 0)
    {
        return;
    }

    const auto textCursor = e_hexDump->textCursor();
    auto previousRow = currentRow;
    // QTextEdit seems to add space char. at the end of each line so
    // the columns number has to be increased by one.
    currentRow = e_hexDump->textCursor().position() / (columns + 1);
    currentColumn = e_hexDump->textCursor().columnNumber();

    const auto properties = flx::get_hex_dump_properties(
            currentRow, currentColumn, data.size(),
            EstimateBytesPerLine(), config.withAscii, config.withAddress,
            config.addressRange.lower(), CurrentExtraSpace());

    DetectAndExecuteChangedValue();
    UpdateAddressStatus(properties.address);
    UpdateValidCharacters(properties.address, properties.type);
    AdjustTextCursorPosition(properties, previousRow);

    currentType = properties.type;
    currentAddress = properties.address;
    currentIsUpperNibble = properties.isUpperNibble;
}

// Adjust the current cursor position. The cursor is only allowed
// to be positioned at a hex digit or ASCII value within the hex dump.
// All other positions are adjusted. Incorrect cursor positions are
// identified by an item type of flx::HexDumpType::NONE.
void MemoryWindow::AdjustTextCursorPosition(
        const flx::sHexDumpProperties &properties, int previousRow) const
{
    const auto col = static_cast<DWord>(currentColumn);
    auto moveOp = QTextCursor::NoMove;

    if (lastEventType == QEvent::KeyPress &&
        lastKeyPressedKey == Qt::Key_Left &&
        currentRow + 1 == previousRow)
    {
        // Roll over from column 0 to previous row
        // => Move cursor back to previous position.
        moveOp = QTextCursor::NextCharacter;
    }
    else if (properties.type != flx::HexDumpType::NONE)
    {
        // If HexDumpType is not NONE there is nothing to adjust.
        return;
    }
    else if (col < properties.beginHexCol)
    {
        moveOp = QTextCursor::NextWord;
    }
    else if (col > properties.endCol)
    {
        moveOp = QTextCursor::PreviousCharacter;
    }
    else if (lastEventType == QEvent::MouseButtonPress)
    {
        if (col > properties.endCol)
        {
            moveOp = QTextCursor::PreviousCharacter;
        }
        else
        {
            moveOp = QTextCursor::NextCharacter;
        }
    }
    else if (lastEventType == QEvent::KeyPress)
    {
        const auto key = lastKeyPressedKey;
        const auto ch = lastKeyPressedCharacter;

        if ((key == Qt::Key_Left && col < properties.beginHexCol) ||
            ((key == Qt::Key_Right || key == Qt::Key_Up ||
              key == Qt::Key_Down || (ch >= ' ' && ch <= '~')) &&
             col < properties.endCol))
        {
            moveOp = QTextCursor::NextCharacter;
        }
        else if ((key == Qt::Key_Left && col > properties.beginHexCol) ||
                ((key == Qt::Key_Right || (ch >= ' ' && ch <= '~')) &&
                 (col > properties.endCol)))
        {
            moveOp = QTextCursor::PreviousCharacter;
        }
    }

    if (moveOp != QTextCursor::NoMove)
    {
        QTimer::singleShot(0, this,
                [&, moveOp](){
                e_hexDump->moveCursor(moveOp); });
    }
}

/*****************************
** Private member functions **
*****************************/

void MemoryWindow::CreateActions(QBoxLayout &layout, const QSize &iconSize)
{
    toolBar = CreateToolBar(this, tr("ToolBar"), QStringLiteral("toolBar"),
            iconSize);
    layout.addWidget(toolBar);

    CreateFileActions(*toolBar);
    CreateViewActions(*toolBar);
}

void MemoryWindow::CreateFileActions(QToolBar &p_toolBar)
{
    fileMenu = menuBar->addMenu(tr("&File"));

    const auto closeIcon = QIcon(":/resource/window-close.png");
    auto *action = fileMenu->addAction(closeIcon, tr("&Close"));
    action->setStatusTip(tr("Close this window"));
    p_toolBar.addAction(action);
    connect(action, &QAction::triggered, this,
            &MemoryWindow::OnClose);
}

void MemoryWindow::CreateViewActions(QToolBar &p_toolBar)
{
    viewMenu = menuBar->addMenu(tr("&View"));

    p_toolBar.addSeparator();
    p_toolBar.addWidget(styleComboBox);
    auto *styleMenu = viewMenu->addMenu(tr("&Style"));
    viewMenu->addMenu(styleMenu);
    viewMenu->addSeparator();

    int index = 0;
    for (const auto &styleString : GetStyleStrings())
    {
        styleActions.append(CreateAction(nullptr, *styleMenu, styleString,
                    index,
                    GetStyleStatusTips()[index], GetStyleHotKeys()[index]));
        ++index;
    }
    InitializeStyleWidget();

    auto icon = QIcon(":/resource/hex-address.png");
    withAddressAction = viewMenu->addAction(icon,
            tr("Display A&ddress Column"));
    withAddressAction->setCheckable(true);
    withAddressAction->setShortcut(QKeySequence(tr("Ctrl+R")));
    withAddressAction->setStatusTip(tr("Toggle displaying address column"));
    withAddressAction->setChecked(config.withAddress);
    p_toolBar.addAction(withAddressAction);
    connect(withAddressAction, &QAction::triggered, this,
            &MemoryWindow::OnToggleDisplayAddresses);

    icon = QIcon(":/resource/ascii.png");
    withAsciiAction = viewMenu->addAction(icon, tr("Display &ASCII Column"));
    withAsciiAction->setCheckable(true);
    withAsciiAction->setShortcut(QKeySequence(tr("Ctrl+A")));
    withAsciiAction->setStatusTip(tr("Toggle displaying ASCII column"));
    withAsciiAction->setChecked(config.withAscii);
    p_toolBar.addAction(withAsciiAction);
    connect(withAsciiAction, &QAction::triggered, this,
            &MemoryWindow::OnToggleDisplayAscii);

    icon = QIcon(":/resource/extra-space.png");
    withExtraSpaceAction =
        viewMenu->addAction(icon, tr("Display &Extra Space"));
    withExtraSpaceAction->setCheckable(true);
    withExtraSpaceAction->setShortcut(QKeySequence(tr("Ctrl+E")));
    withExtraSpaceAction->setStatusTip(
            tr("Display extra space after each 8 byte"));
    withExtraSpaceAction->setChecked(config.withExtraSpace);
    p_toolBar.addAction(withExtraSpaceAction);
    connect(withExtraSpaceAction, &QAction::triggered, this,
            &MemoryWindow::OnToggleExtraSpace);

    icon = QIcon(":/resource/autoupdate-winsize.png");
    isUpdateWindowSizeAction = viewMenu->addAction(icon,
            tr("Automatic &Update Window Size"));
    isUpdateWindowSizeAction->setCheckable(true);
    isUpdateWindowSizeAction->setShortcut(QKeySequence(tr("Ctrl+U")));
    isUpdateWindowSizeAction->setStatusTip(
            tr("Automatic window size update"));
    isUpdateWindowSizeAction->setChecked(config.isUpdateWindowSize);
    isUpdateWindowSizeAction->setEnabled(config.style != Style::Dynamic);
    p_toolBar.addAction(isUpdateWindowSizeAction);
    connect(isUpdateWindowSizeAction, &QAction::triggered, this,
            &MemoryWindow::OnToggleUpdateWindowSize);

    p_toolBar.addSeparator();
    viewMenu->addSeparator();
    icon = QIcon(":/resource/toggle.png");
    toggleHexAsciiAction = viewMenu->addAction(icon,
            tr("&Toggle Cursor between HEX and ASCII"));
    toggleHexAsciiAction->setShortcut(QKeySequence(tr("Ctrl+T")));
    toggleHexAsciiAction->setStatusTip(
            tr("Toggle Cursor between HEX and ASCII"));
    UpdateToggleHexAsciiEnabled();
    p_toolBar.addAction(toggleHexAsciiAction);
    connect(toggleHexAsciiAction, &QAction::triggered, this,
            &MemoryWindow::OnToggleHexAscii);
}

void MemoryWindow::CreateStatusBar(QBoxLayout &layout)
{
    // Use QStackedWidget to be able to set a frame style.
    statusBarFrame = new QStackedWidget();
    statusBar = new QStatusBar();
    statusBar->setSizeGripEnabled(false);
    statusBarFrame->addWidget(statusBar);
    layout.addWidget(statusBarFrame, 1);
    statusBarFrame->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    memoryTypeFrame = new QStackedWidget();
    memoryTypeLabel = new QLabel(this);
    memoryTypeLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    memoryTypeLabel->setToolTip(tr("Memory type under cursor"));
    memoryTypeFrame->addWidget(memoryTypeLabel);
    memoryTypeFrame->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    layout.addWidget(memoryTypeFrame);
    addressFrame = new QStackedWidget();
    addressLabel = new QLabel(this);
    addressLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    addressLabel->setToolTip(tr("Address under cursor and content"));
    const auto pointSize = QApplication::font().pointSize();
    auto font = GetMonospaceFont(pointSize);
    font.setWeight(QFont::Bold);
    addressLabel->setFont(font);
    addressFrame->addWidget(addressLabel);
    addressFrame->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    layout.addWidget(addressFrame);

    updateLedFrame = new QStackedWidget(this);
    updateLedFrame->setObjectName("updateLedFrame");
    updateLedGraphics = new QGraphicsView(this);
    updateLedGraphics->setObjectName("updateLedGraphics");
    updateLedGraphics->setMaximumSize(QSize(22, 22));
    updateLedGraphics->setToolTip(
            tr("Indicator for memory update in progress"));
    updateLedGraphics->setFrameStyle(QFrame::NoFrame);
    auto *scene = new QGraphicsScene(updateLedGraphics);
    updateLedGraphics->setScene(scene);
    scene->addPixmap(QPixmap(":/resource/ledredoff.png"));
    updateLedFrame->addWidget(updateLedGraphics);
    layout.addWidget(updateLedFrame);

    dummyStatusBar = new QStatusBar(this);
    dummyStatusBar->setMaximumWidth(14);
    dummyStatusBar->setSizeGripEnabled(true);
    layout.addWidget(dummyStatusBar);

    statusBar->showMessage(tr("Ready"));
}

// Implementation may change in future.
// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
QToolBar *MemoryWindow::CreateToolBar(QWidget *parent,
                                      const QString &title,
                                      const QString &objectName,
                                      const QSize &iconSize)
{
    auto *newToolBar = new QToolBar(title, parent);
    assert(newToolBar != nullptr);
    newToolBar->setObjectName(objectName);
    newToolBar->setFloatable(false);
    newToolBar->setMovable(false);
    newToolBar->setIconSize(iconSize);
    QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    newToolBar->setSizePolicy(sizePolicy);

    return newToolBar;
}

void MemoryWindow::InitializeStyleWidget()
{
    assert(styleComboBox != nullptr);
    assert(GetStyleStrings().size() == GetStyleValues().size());
    assert(GetStyleStatusTips().size() == GetStyleValues().size());
    assert(GetStyleHotKeys().size() == GetStyleValues().size());

    styleComboBox->addItems(GetStyleStrings());
    styleComboBox->setMinimumWidth(45);

    if (auto index = GetStyleValues().indexOf(config.style); index >= 0)
    {
        styleComboBox->setCurrentText(GetStyleStrings()[index]);
        UpdateStyleCheck(index);
    }
}

void MemoryWindow::ConnectStyleComboBoxSignalSlots() const
{
    connect(styleComboBox,
#if (QT_VERSION <= QT_VERSION_CHECK(5, 7, 0))
              static_cast<void (QComboBox::*)(int)>(
                  &QComboBox::currentIndexChanged),
#else
              QOverload<int>::of(&QComboBox::currentIndexChanged),
#endif
              this, &MemoryWindow::OnStyleChanged);
      connect(styleComboBox,
#if (QT_VERSION <= QT_VERSION_CHECK(5, 7, 0))
              static_cast<void (QComboBox::*)(int)>(
                  &QComboBox::highlighted),
#else
              QOverload<int>::of(&QComboBox::highlighted),
#endif
              this, &MemoryWindow::OnStyleHighlighted);
}

void MemoryWindow::SetIconSize(const QSize &iconSize)
{
    toolBar->setIconSize(iconSize);
}

void MemoryWindow::SetTextBrowserFont(const QFont &font)
{
    e_hexDumpScale->document()->setDefaultFont(font);
    const auto height =
        static_cast<int>(ceil(e_hexDumpScale->document()->size().height()));
    e_hexDumpScale->setFixedHeight(height);
    while(e_hexDumpScale->verticalScrollBar()->isVisible())
    {
        e_hexDumpScale->setFixedHeight(e_hexDumpScale->height() + 1);
    }

    // Show overwrite cursor.
    e_hexDump->document()->setDefaultFont(font);
    QFontMetrics metrics(e_hexDump->document()->defaultFont());
    e_hexDump->setCursorWidth(metrics.averageCharWidth());

    RequestResize();
    DoResize(true);
}

/*******************
** Event handlers **
*******************/

void MemoryWindow::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::ApplicationFontChange ||
        event->type() == QEvent::FontChange)
    {
        OnUpdateFont();
    }
}

void MemoryWindow::closeEvent(QCloseEvent *event)
{
    hexDumpFuture.waitForFinished();

    emit Closed(this);
    event->accept();
}

void MemoryWindow::resizeEvent(QResizeEvent *event)
{
    // Ignore first resize event for a new window with style == Style::Dynamic.
    // This avoids recalculating dynamicBytesPerLine
    // based on the default window size and instead use the default
    // initialized value for dynamicBytesPerline as defined in ctor.
    if (isIgnoreResizeEvent)
    {
        isIgnoreResizeEvent = false;
        event->accept();
        return;
    }

    // After resizing window recalculate dynamicBytesPerLine.
    RecalculateDynamicBytesPerLine(event->size());


    if (styleComboBox->width() < 60)
    {
        int index = 0;

        for (const auto &text : GetStyleShortStrings())
        {
            styleComboBox->setItemText(index++, text);
        }
    }
    else if (styleComboBox->width() > 70)
    {
        int index = 0;

        for (const auto &text : GetStyleStrings())
        {
            styleComboBox->setItemText(index++, text);
        }
    }

    event->accept();
}

bool MemoryWindow::event(QEvent *event)
{
    if (event->type() == QEvent::StatusTip)
    {
        auto *statusTipEvent = dynamic_cast<QStatusTipEvent *>(event);
        assert(statusTipEvent != nullptr);
        statusBar->showMessage(statusTipEvent->tip());
        statusTipEvent->accept();

        return true;
    }

    return QWidget::event(event);
}

const QString &MemoryWindow::GetStartAddrLiteral()
{
    static const QString literal{"%START"};

    return literal;
}

const QString &MemoryWindow::GetEndAddrLiteral()
{
    static const QString literal{"%END"};

    return literal;
}

void MemoryWindow::UpdateData(const std::vector<Byte> &p_data)
{
    data = p_data;

    UpdateData();
}

void MemoryWindow::SetReadOnly(bool p_isReadOnly)
{
    isReadOnly = p_isReadOnly;
    e_hexDump->setReadOnly(isReadOnly);
    UpdateToggleHexAsciiEnabled();
}

void MemoryWindow::UpdateData()
{
    std::stringstream scaleStream;
    std::optional<DWord> startAddress;
    auto bytesPerLine = EstimateBytesPerLine();
    updateDataStartTime = std::chrono::system_clock::now();

    SetUpdateDataPixmap(QPixmap(":/resource/ledredon.png"));

    if (config.withAddress)
    {
        startAddress = config.addressRange.lower();
    }

    ConnectHexDumpCursorPositionChanged(false);

    flx::hex_dump_scale(scaleStream, bytesPerLine, config.withAscii,
            config.withAddress, CurrentExtraSpace());
    e_hexDumpScale->setPlainText(QString::fromStdString(scaleStream.str()));

    if (hexDumpFuture.isRunning())
    {
        hexDumpFuture.waitForFinished();
    }

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    hexDumpFuture = QtConcurrent::run(&MemoryWindow::CreateHexDump, this,
            bytesPerLine, config.withAscii, config.withAddress,
            config.addressRange.lower(), CurrentExtraSpace());
#else
    hexDumpFuture = QtConcurrent::run(this, &MemoryWindow::CreateHexDump,
            bytesPerLine, config.withAscii, config.withAddress,
            config.addressRange.lower(), CurrentExtraSpace());
#endif
    hexDumpWatcher.setFuture(hexDumpFuture);
    connect(&hexDumpWatcher, &QFutureWatcher<QString>::finished,
            this, &MemoryWindow::UpdateDataFinish);
}

QString MemoryWindow::CreateHexDump(DWord bytesPerLine, bool withAscii,
        bool isDisplayAddress, DWord startAddress,
        std::optional<DWord> extraSpace)
{
    std::stringstream hexStream;

    flx::hex_dump(hexStream, data.data(), data.size(), bytesPerLine,
            withAscii, isDisplayAddress, startAddress, extraSpace);

    return QString::fromStdString(hexStream.str());
}

void MemoryWindow::UpdateDataFinish()
{
    disconnect(&hexDumpWatcher, &QFutureWatcher<QString>::finished,
               this, &MemoryWindow::UpdateDataFinish);
    assert(hexDumpWatcher.isFinished());
    auto hexDumpString = hexDumpWatcher.result();
    auto bytesPerLine = EstimateBytesPerLine();

    int sliderPosition = 0;

    if (e_hexDump->verticalScrollBar() != nullptr)
    {
        sliderPosition = e_hexDump->verticalScrollBar()->sliderPosition();
    }
    auto value = e_hexDump->horizontalScrollBar()->value();

    e_hexDump->setPlainText(hexDumpString);

    QTextStream stream(&hexDumpString, QIODevice::ReadOnly);
    QString line;
    columns = 0;
    rows = 0;
    bool isForceMoveCursor = false;
    while (stream.readLineInto(&line))
    {
        columns = std::max(columns, cast_from_qsizetype(line.size()));
        ++rows;
    }

    const auto currentRowCol = flx::get_hex_dump_position_for_address(
            currentAddress, data.size(), bytesPerLine, config.withAscii,
            config.withAddress, currentType == flx::HexDumpType::AsciiChar,
            currentIsUpperNibble, config.addressRange.lower(),
            CurrentExtraSpace());

    if (currentRowCol.has_value())
    {
        isForceMoveCursor =
            (currentRowCol.value() == std::pair(0U, 0U)) && !config.withAddress;
        currentRow = static_cast<int>(currentRowCol.value().first);
        currentColumn = static_cast<int>(currentRowCol.value().second);
        if (!config.withAscii)
        {
            UpdateValidCharacters(currentAddress, flx::HexDumpType::HexByte);
            currentType = flx::HexDumpType::HexByte;
        }

        const auto position = static_cast<int>(
                (columns + 1U) *
                (currentRowCol.value().first ? currentRowCol.value().first : 0U) +
                 currentRowCol.value().second + (isForceMoveCursor ? 1 : 0));

        auto cursor = e_hexDump->textCursor();
        cursor.setPosition(position);
        e_hexDump->setTextCursor(cursor);
    }
    else
    {
        // row, column could not be estimated, try to position to first
        // character in first row.
        const auto moveOp = QTextCursor::NextCharacter;
        QTimer::singleShot(0, this, [&](){ e_hexDump->moveCursor(moveOp); });
    }

    if (e_hexDump->verticalScrollBar() != nullptr)
    {
        e_hexDump->verticalScrollBar()->setSliderPosition(sliderPosition);
    }

    e_hexDump->horizontalScrollBar()->setValue(value);

    ConnectHexDumpCursorPositionChanged(true);

    // Force move cursor:
    // If address is not displayed (config.withAddress == false) the default
    // position is 0 when opening the window. This resulted in not calling
    // UpdateAddressStatus (display current address, value and type).
    // This isForceMoveCursor flag forces to call OnTextCursorPositionChanged()
    // in this case. It initially sets cursor position to 1 and after
    // activating cursorPositionChanged event move cursor back one position,
    // which forces to call OnTextCursoPositionChange().
    if (isForceMoveCursor)
    {
        auto cursor = e_hexDump->textCursor();
        cursor.setPosition(0);
        e_hexDump->setTextCursor(cursor);
    }

    const auto timeDiff = std::chrono::system_clock::now() -
        updateDataStartTime;
    const auto msecDiff =
        std::chrono::duration_cast<std::chrono::milliseconds>(timeDiff).count();
    // Indicate update duration with LED switched on, for at least 200 ms.
    const int msecDelay = std::max(0, 200 - static_cast<int>(msecDiff));

    QPointer<MemoryWindow> safeThis(this);
    QTimer::singleShot(msecDelay, [safeThis](){
            if (safeThis)
            {
                const auto pixmap = QPixmap(":/resource/ledredoff.png");
                safeThis->SetUpdateDataPixmap(pixmap);
            }
    });

    DoResize(isRequestResize);
}

void MemoryWindow::DoResize(bool condition)
{
    if (condition && !data.empty())
    {
        QPointer<MemoryWindow> safeThis(this);
        const auto fctResize = [safeThis](){
                if (safeThis)
                {
                    safeThis->Resize();
                    safeThis->isRequestResize = false;
                };
            };

        if (QAbstractEventDispatcher::instance() != nullptr)
        {
            // We are already in the event loop, call function.
            fctResize();
        }
        else
        {
            // Execute in event loop to also work when opening the window.
            QTimer::singleShot(50, this, fctResize);
        }
    };
}

DWord MemoryWindow::EstimateBytesPerLine() const
{
    using T = std::underlying_type_t<MemoryWindow::Style>;

    if (config.style == MemoryWindow::Style::Dynamic)
    {
        return dynamicBytesPerLine;
    }

    return static_cast<T>(config.style);
}

void MemoryWindow::RequestResize()
{
    if (isUpdateWindowSizeAction->isEnabled() && config.isUpdateWindowSize)
    {
        isRequestResize = true;
    }
}

// Resize strategy
//
// No reason - no resize.
// Reasons for resize:
// - When opening a memory window (only if no position and size available)
//   This is done event if Automatic Update Window Size is disabled or not
//   checked.
// - Display Style has changed
// - Display Address has changed
// - Display ASCII has changed
// - Display extra space has changed
// - Automatic Update Window Size has been checked
// Prerequisites for request a resize (See RequestResize() ):
// - Automatic Update Window Size is enabled
// - Automatic Update Window Size is checked
// Prerequisites for resize (See DoResize() ):
// - Data is available
// The Resize is always executed in the event loop.
void MemoryWindow::Resize()
{
    int screenWidth = 640;
    int screenHeight = 800;
    if (!QGuiApplication::screens().isEmpty())
    {
        screenWidth =
            QGuiApplication::screens().first()->geometry().width();
        screenHeight =
            QGuiApplication::screens().first()->geometry().height();
    }

    auto size = e_hexDump->document()->size().toSize();
    // Make sure to always display the whole hex dump scale even if
    // the first row contains less than the max. byte number per line.
    const auto hdsSize = e_hexDumpScale->document()->size().toSize();
    size.setWidth(std::max(size.width(), hdsSize.width()));

    size.setHeight(std::min(size.height(), screenHeight * 3 / 4));
    size.setWidth(std::min(size.width(), screenWidth * 3 / 4));
    e_hexDump->setFixedSize(size);

    // Try to remove scroll bars.
    auto repeat = 0;
    while (repeat < 20 && e_hexDump->verticalScrollBar()->isVisible())
    {
        e_hexDump->setFixedHeight(e_hexDump->height() + 1);
        ++repeat;
    }

    while (e_hexDump->horizontalScrollBar()->isVisible())
    {
        e_hexDump->setFixedWidth(e_hexDump->width() + 1);
    }
    e_hexDump->setFixedWidth(e_hexDump->width() + 3);

    adjustSize();
    e_hexDump->setMinimumSize(64, 48);
    e_hexDump->setMaximumSize(0xFFFFFF, 0xFFFFFF);
}

void MemoryWindow::SetStatusMessage(const QString &message) const
{
    statusBar->showMessage(message, 4000);
}

void MemoryWindow::UpdateStyleCheck(int index) const
{
    for (int actionIdx = 0; actionIdx < GetStyleStrings().size(); ++actionIdx)
    {
        styleActions[actionIdx]->setChecked(actionIdx == index);
    }
}

void MemoryWindow::UpdateStyleValue(int index) const
{
    if (styleComboBox->currentIndex() != index)
    {
        styleComboBox->disconnect();
        styleComboBox->setCurrentIndex(index);
        ConnectStyleComboBoxSignalSlots();
    }
}

QAction *MemoryWindow::CreateAction(QIcon *icon, QMenu &menu,
        const QString &text, int index, const QString &statusTip,
        const QString &hotKey)
{
    auto *action = (icon == nullptr) ?
        menu.addAction(text) : menu.addAction(*icon, text);
    connect(action, &QAction::triggered,
        this, [&,index](){ OnStyleChanged(index); });
    action->setCheckable(true);

    if (!hotKey.isEmpty())
    {
        auto keySequence = QKeySequence(hotKey);
        action->setShortcut(keySequence);
    }

    if (!statusTip.isEmpty())
    {
        action->setStatusTip(statusTip);
    }

    return action;
}

/*********************
** Static functions **
*********************/

const MemoryWindow::StyleValues_t &MemoryWindow::GetStyleValues()
{
    static const MemoryWindow::StyleValues_t styleValues
    {
        MemoryWindow::Style::Dynamic,
        MemoryWindow::Style::Bytes4,
        MemoryWindow::Style::Bytes8,
        MemoryWindow::Style::Bytes16,
        MemoryWindow::Style::Bytes32,
        MemoryWindow::Style::Bytes64,
    };

    return styleValues;
}

const QStringList &MemoryWindow::GetStyleStrings()
{
    static const QStringList styleStrings
    {
        "Dynamic",
        "4 bytes per line",
        "8 bytes per line",
        "16 bytes per line",
        "32 bytes per line",
        "64 bytes per line",
    };

    return styleStrings;
}

const QStringList &MemoryWindow::GetStyleShortStrings()
{
    static const QStringList styleStrings
    {
        "Dyn",
        "4 b",
        "8 b",
        "16 b",
        "32 b",
        "64 b",
    };

    return styleStrings;
}

const QStringList &MemoryWindow::GetStyleStatusTips()
{
    static const QStringList statusTips
    {
        "Displayed byte count depends on window width",
        "Display 4 bytes per line",
        "Display 8 bytes per line",
        "Display 16 bytes per line",
        "Display 32 bytes per line",
        "Display 64 bytes per line",
    };

    return statusTips;
}

const QStringList &MemoryWindow::GetStyleHotKeys()
{
    static const QStringList hotKeys
    {
        "Ctrl+D",
        "Ctrl+4",
        "Ctrl+8",
        "Ctrl+1",
        "Ctrl+3",
        "Ctrl+6",
    };

    return hotKeys;
}

QString MemoryWindow::GetDefaultWindowTitle()
{
    return GetStartAddrLiteral() + "-" + GetEndAddrLiteral();
}

QString MemoryWindow::CreateWindowTitle(
        const std::string &sTitle,
        const BInterval<DWord> &addressRange)
{
    auto title = QString::fromStdString(sTitle);
    if (title.isEmpty())
    {
        title = GetDefaultWindowTitle();
    }

    if (auto index = title.indexOf(GetStartAddrLiteral()); index >= 0)
    {
        const auto startAddr = QString("%1").arg(addressRange.lower(), 4, 16,
            QLatin1Char('0')).toUpper();
        title.replace(GetStartAddrLiteral(), startAddr);
    }

    if (auto index = title.indexOf(GetEndAddrLiteral()); index >= 0)
    {
        const auto endAddr = QString("%1").arg(addressRange.upper(), 4, 16,
            QLatin1Char('0')).toUpper();
        title.replace(GetEndAddrLiteral(), endAddr);
    }

    return title;
}

std::optional<DWord> MemoryWindow::CurrentExtraSpace() const
{
    return config.withExtraSpace ? std::optional<DWord>(8U) : std::nullopt;
}

void MemoryWindow::DetectAndExecuteChangedValue()
{
    const auto ch = lastKeyPressedCharacter;

    // Update memory if any hex or ASCII value has changed.
    if (lastEventType == QEvent::KeyPress && ch >= ' ' && ch <= '~')
    {
        switch (currentType)
        {
            case flx::HexDumpType::NONE:
                break;

            case flx::HexDumpType::HexByte:
                HexValueChanged(ch, currentAddress, currentIsUpperNibble);
                break;

            case flx::HexDumpType::AsciiChar:
                AsciiValueChanged(ch, currentAddress);
                break;
        }
    }
}

void MemoryWindow::HexValueChanged(char ch, DWord address, bool isUpperNibble)
{
    bool isValid = false;
    const Byte operand = flx::hexval(ch, isValid) << (isUpperNibble ? 4U : 0U);
    const auto offset = address - config.addressRange.lower();

    if (isValid && offset < data.size())
    {
        const auto value = data[offset];
        const auto newValue = static_cast<Byte>(
            (value & (isUpperNibble ? 0x0FU : 0xF0U)) | operand);

        if (newValue != value)
        {
            data[offset] = newValue;
            emit MemoryModified(this, address, { newValue });

            const auto text = (newValue >= ' ' && newValue <= '~') ?
                QString(static_cast<char>(newValue)) : QString('_');
            ReplaceHexOrAsciiText(text);
        }
    }
}

void MemoryWindow::AsciiValueChanged(char ch, DWord address)
{
    const auto offset = address - config.addressRange.lower();

    if (offset < data.size())
    {
        const auto value = data[offset];
        const auto newValue = static_cast<Byte>(ch);

        if (newValue != value)
        {
            data[offset] = newValue;
            emit MemoryModified(this, address, { newValue });

            const auto text =
                QString("%1").arg(newValue, 2, 16, QLatin1Char('0')).toUpper();
            ReplaceHexOrAsciiText(text);
        }
    }
}

void MemoryWindow::UpdateValidCharacters(DWord address,
        flx::HexDumpType type) const
{
    QString validCharacters;

    for (const auto &item : availableMemoryRanges)
    {
        if (in(address, item.addressRange) && item.type != MemoryType::RAM)
        {
            e_hexDump->SetValidCharacters("");
            return;
        }
    }

    switch (type)
    {
        case flx::HexDumpType::HexByte:
            validCharacters = MemoryWindowTextEdit::GetAllHexCharacters();
            break;

        case flx::HexDumpType::AsciiChar:
            validCharacters = MemoryWindowTextEdit::GetAllAsciiCharacters();
            break;

        case flx::HexDumpType::NONE:
            break;
    }

    e_hexDump->SetValidCharacters(validCharacters);
}

BInterval<DWord> MemoryWindow::GetAddressRange() const
{
    return config.addressRange;
}

void MemoryWindow::UpdateAddressStatus(DWord address)
{
    const auto offset = address - config.addressRange.lower();
    const auto byteValue = (offset < data.size()) ? data[offset] : 0U;
    const auto statusString = fmt::format("{:04X} {:02X}", address,
            byteValue);
    std::stringstream stream;

    for (const auto &[type, memoryAddressRange] : availableMemoryRanges)
    {
        if (in(address, memoryAddressRange))
        {
            stream << type;
            memoryTypeLabel->setText(QString::fromStdString(stream.str()));
            break;
        }
    }

    addressLabel->setText(QString::fromStdString(statusString));
}

void MemoryWindow::UpdateToggleHexAsciiEnabled() const
{
    toggleHexAsciiAction->setEnabled(config.withAscii && !isReadOnly);
}

// After changing a HEX or ASCII value the corresponding ASCII or HEX
// value has to be updated too.
// This function updates the corresponding value in e_hexDump wiget.
// The data vector is unchanged.
void MemoryWindow::ReplaceHexOrAsciiText(const QString &text) const
{
    if (!config.withAscii)
    {
        return;
    }

    ConnectHexDumpCursorPositionChanged(false);

    const auto rowCol = flx::get_hex_dump_position_for_address(
        currentAddress, data.size(), EstimateBytesPerLine(), config.withAscii,
        config.withAddress, currentType != flx::HexDumpType::AsciiChar,
        true, config.addressRange.lower(), CurrentExtraSpace());

    if (rowCol.has_value())
    {
        const auto position = static_cast<int>(
                (columns + 1U) *
                (rowCol.value().first ? rowCol.value().first : 0U) +
                 rowCol.value().second);

        auto cursor = e_hexDump->textCursor();
        auto savePosition = cursor.position();
        cursor.beginEditBlock();
        cursor.setPosition(position);
        cursor.setPosition(position + text.size(), QTextCursor::KeepAnchor);
        cursor.insertText(text);
        cursor.endEditBlock();
        e_hexDump->setTextCursor(cursor);
        cursor = e_hexDump->textCursor();
        cursor.setPosition(savePosition);
        e_hexDump->setTextCursor(cursor);
    }

    ConnectHexDumpCursorPositionChanged(true);
}

void MemoryWindow::ConnectHexDumpCursorPositionChanged(bool isConnect) const
{
    if (isConnect)
    {
        connect(e_hexDump, &QTextEdit::cursorPositionChanged,
            this, &MemoryWindow::OnTextCursorPositionChanged);
        return;
    }

    disconnect(e_hexDump, &QTextEdit::cursorPositionChanged,
        this, &MemoryWindow::OnTextCursorPositionChanged);
}

void MemoryWindow::OnNotifyKeyPressed(QKeyEvent *event)
{
    assert(event != nullptr);
    if (event->key() == Qt::Key_A && event->modifiers() == Qt::ControlModifier)
    {
        OnToggleDisplayAscii();
        return;
    }

    lastKeyPressedKey = event->key();
    const bool isValidAsciiChar = event->text().size() == 1 &&
            event->text().at(0) >= ' ' && event->text().at(0) <= '~';
    lastKeyPressedCharacter = isValidAsciiChar ?
        event->text()[0].toLatin1() : '\0';
}

void MemoryWindow::OnEventTypeChanged(QEvent::Type eventType)
{
    lastEventType = eventType;
}

// Specification of configString:
// <width>,<height>,<x>,<y>,<windowTitle>,<startAddr>,<endAddr>,<style>,
// <withAddress>,<withAscii>,<withExtraSpace>,<isUpdateWindowSize>
//
// <startAddr> and <endAddr> are hexadecimal.
// for <style> the underlying enum value is used.
// for windowTitle no comma and semicolon is allowed.
void MemoryWindow::ConvertConfigString(const std::string &configString,
        Config_t &p_config, QRect &positionAndSize)
{
    Word startAddr = 0U;
    Word endAddr = 15U;
    int validCount = 0;
    int temp{};
    int height{};
    int width{};
    int x{};
    int y{};
    const auto items = flx::split(configString, ',', true);
    const decltype(items)::size_type min = 12U;

    positionAndSize = QRect(100, 100, 560, 768); // Set some default.

    auto convertToInt = [&](const std::string &str, int &value) -> bool
    {
        temp = 0;

        bool success = (flx::convert(str, temp) && temp > 0);
        if (success)
        {
            value = temp;
        }
        return success;
    };

    auto convertToBool = [&](const std::string &str, bool &value)
    {
        uint8_t u8temp = 0;

        if (flx::convert(str, u8temp) && u8temp <= 1)
        {
            value = (u8temp != 0);
        }
    };

    switch(std::min(items.size(), min))
    {
        case 12:
            convertToBool(items[11], p_config.isUpdateWindowSize);
            [[fallthrough]];
        case 11:
            convertToBool(items[10], p_config.withExtraSpace);
            [[fallthrough]];
        case 10:
            convertToBool(items[9], p_config.withAscii);
            [[fallthrough]];
        case 9:
            convertToBool(items[8], p_config.withAddress);
            [[fallthrough]];
        case 8:
            if (flx::convert(items[7], temp) && temp <= 64 &&
                flx::countSetBits(temp) <= 1 && temp != 1 && temp != 2)
            {
                p_config.style = static_cast<Style>(temp);
            }
            [[fallthrough]];
        case 7:
            validCount = flx::convert(items[6], endAddr, 16) ? 1 : 0;
            [[fallthrough]];
        case 6:
            validCount += flx::convert(items[5], startAddr, 16) ? 1 : 0;
            if (validCount == 2)
            {
                p_config.addressRange = (startAddr <= endAddr) ?
                    BInterval<DWord>(startAddr, endAddr) :
                    BInterval<DWord>(endAddr, startAddr);
            }
            [[fallthrough]];
        case 5:
            p_config.windowTitle = items[4];
            [[fallthrough]];
        case 4:
            validCount = (convertToInt(items[3], y)) ? 1 : 0;
            [[fallthrough]];
        case 3:
            validCount += (convertToInt(items[2], x)) ? 1 : 0;
            [[fallthrough]];
        case 2:
            validCount += (convertToInt(items[1], height)) ? 1 : 0;
            [[fallthrough]];
        case 1:
            validCount += (convertToInt(items[0], width)) ? 1 : 0;
            if (validCount == 4)
            {
                positionAndSize = QRect(x, y, width, height);
            }
            break;

        case 0:
        default:
            break;
    }
}

std::string MemoryWindow::GetConfigString() const
{
    using T = std::underlying_type_t<Style>;

    std::stringstream stream;
    const auto positionAndSize = geometry();
    bool isEnabled = isUpdateWindowSizeAction->isEnabled();

    stream <<
        positionAndSize.width() << ',' <<
        positionAndSize.height() << ',' <<
        positionAndSize.x() << ',' <<
        positionAndSize.y() << ',' <<
        config.windowTitle << ',' <<
        std::hex << config.addressRange.lower() << ',' <<
        std::hex << config.addressRange.upper() << ',' <<
        std::dec << static_cast<int>(static_cast<T>(config.style)) << ',' <<
        config.withAddress << ',' <<
        config.withAscii << ',' <<
        config.withExtraSpace << ',' <<
        (isEnabled && config.isUpdateWindowSize);

    return stream.str();
}

void MemoryWindow::SetUpdateDataPixmap(const QPixmap &pixmap) const
{
    assert(updateLedGraphics != nullptr);

    auto *scene = updateLedGraphics->scene();
    assert(scene != nullptr);
    auto items = scene->items();
    if (!items.isEmpty())
    {
        auto *pixmapItem = dynamic_cast<QGraphicsPixmapItem *>(items[0]);
        assert(pixmapItem != nullptr);
        pixmapItem->setPixmap(pixmap);
    }
}

void MemoryWindow::RecalculateDynamicBytesPerLine(const QSize &size)
{
    QFontMetrics metrics(e_hexDump->document()->defaultFont());
    const auto width = size.width();
    const auto cols = width / metrics.averageCharWidth();
    const auto dividend = config.withAscii ? 4U : 3U;
    const auto offset1 = (config.withAscii ? 1U : 0U) +
        (config.withAddress ? 6U : 0U);

    // First calculation without extra space.
    dynamicBytesPerLine = (cols - offset1) / dividend;
    dynamicBytesPerLine = std::max(4U,
            dynamicBytesPerLine - (dynamicBytesPerLine % 8U));

    // The extra space every 8 bytes needs an extra calculation.
    auto offset2 = (config.withExtraSpace && dynamicBytesPerLine > 8U) ?
        (config.withAscii ? 2U : 1U) * ((dynamicBytesPerLine / 8U) - 1U) : 0U;

    dynamicBytesPerLine = (cols - offset1 - offset2) / dividend;
    dynamicBytesPerLine = std::max(4U,
            dynamicBytesPerLine - (dynamicBytesPerLine % 8U));
}
