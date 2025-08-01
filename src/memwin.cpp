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
#include <fmt/format.h>
#include "warnon.h"
#include "qtfree.h"
#include <algorithm>
#include <optional>
#include <cmath>
#include <cassert>
#include <sstream>
#include <string>


/****************************
** Constructor, Destructor **
****************************/

MemoryWindow::MemoryWindow(
        bool p_isReadOnly,
        const BInterval<DWord> &p_addressRange,
        MemoryRanges_t p_availableMemoryRanges,
        QString p_windowTitle,
        MemoryWindow::Style p_style,
        bool p_withAddress,
        bool p_withAscii,
        bool p_withExtraSpace,
        bool p_isUpdateWindowSize)
    : mainLayout(new QVBoxLayout(this))
    , toolBarLayout(new QHBoxLayout)
    , statusBarLayout(new QHBoxLayout)
    , menuBar(new QMenuBar(this))
    , e_hexDumpScale(new QTextEdit(this))
    , e_hexDump(new MemoryWindowTextEdit(this))
    , styleComboBox(new QComboBox(this))
    , availableMemoryRanges(std::move(p_availableMemoryRanges))
    , addressRange(p_addressRange)
    , windowTitle(std::move(p_windowTitle))
    , style(p_style)
    , dynamicBytesPerLine(16)
    , withAddress(p_withAddress)
    , withAscii(p_withAscii)
    , withExtraSpace(p_withExtraSpace)
    , isUpdateWindowSize(p_isUpdateWindowSize)
    , isReadOnly(p_isReadOnly)
    , isFirstResizeEvent(true)
    , isRequestResize(true)
{
    const QSize iconSize(16, 16);

    setObjectName("MemoryWindow");
    auto title = CreateDefaultWindowTitle(addressRange);
    if (!windowTitle.isEmpty())
    {
        title = windowTitle.contains("%1") ?
            windowTitle.arg(title) : windowTitle;
    }
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
}

/*********************
** Static functions **
*********************/

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
        style = GetStyleValues()[index];
        UpdateStyleCheck(index);
        UpdateStyleValue(index);
        UpdateData();
        RequestResize();
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
    withAddress = !withAddress;
    UpdateData();
    RequestResize();
}

void MemoryWindow::OnToggleDisplayAscii()
{
    withAscii = !withAscii;
    withAsciiAction->setChecked(withAscii);
    UpdateToggleHexAsciiEnabled();
    UpdateData();
    RequestResize();
}

void MemoryWindow::OnToggleUpdateWindowSize()
{
    isUpdateWindowSize = !isUpdateWindowSize;
    UpdateData();
    RequestResize();
}

void MemoryWindow::OnToggleExtraSpace()
{
    withExtraSpace = !withExtraSpace;
    UpdateData();
    RequestResize();
}

void MemoryWindow::OnToggleHexAscii()
{
    const auto rowCol = flx::get_hex_dump_position_for_address(
        currentAddress, data.size(), EstimateBytesPerLine(), withAscii,
        withAddress, currentType != flx::HexDumpType::AsciiChar,
        true, addressRange.lower(), CurrentExtraSpace());

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
            EstimateBytesPerLine(), withAscii, withAddress,
            addressRange.lower(), CurrentExtraSpace());

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
    withAddressAction->setChecked(withAddress);
    p_toolBar.addAction(withAddressAction);
    connect(withAddressAction, &QAction::triggered, this,
            &MemoryWindow::OnToggleDisplayAddresses);

    icon = QIcon(":/resource/ascii.png");
    withAsciiAction = viewMenu->addAction(icon, tr("Display &ASCII Column"));
    withAsciiAction->setCheckable(true);
    withAsciiAction->setShortcut(QKeySequence(tr("Ctrl+A")));
    withAsciiAction->setStatusTip(tr("Toggle displaying ASCII column"));
    withAsciiAction->setChecked(withAscii);
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
    withExtraSpaceAction->setChecked(withExtraSpace);
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
    isUpdateWindowSizeAction->setChecked(isUpdateWindowSize);
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

    if (auto index = GetStyleValues().indexOf(style); index >= 0)
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
    emit Closed(this);
    event->accept();
}

void MemoryWindow::resizeEvent(QResizeEvent *event)
{
    // Ignore first resize event, to avoid recalculating dynamicBytesPerLine
    // based on the default window size and instead use the default
    // initialized value for dynamicBytesPerline as defined in ctor.
    if (isFirstResizeEvent)
    {
        isFirstResizeEvent = false;
        event->accept();
        return;
    }

    // After resizing window recalculate dynamicBytesPerLine.
    QFontMetrics metrics(e_hexDump->document()->defaultFont());
    auto width = event->size().width();
    auto cols = width / metrics.averageCharWidth();
    auto dividend = withAscii ? 4U : 3U;
    auto offset1 = (withAscii ? 1U : 0U) + (withAddress ? 6U : 0U);

    // First calculation without extra space.
    dynamicBytesPerLine = (cols - offset1) / dividend;
    dynamicBytesPerLine = std::max(4U,
            dynamicBytesPerLine - (dynamicBytesPerLine % 8U));

    // The extra space every 8 bytes needs an extra calculation.
    auto offset2 = (withExtraSpace && dynamicBytesPerLine > 8U) ?
        (withAscii ? 2U : 1U) * ((dynamicBytesPerLine / 8U) - 1U) : 0U;

    dynamicBytesPerLine = (cols - offset1 - offset2) / dividend;
    dynamicBytesPerLine = std::max(4U,
            dynamicBytesPerLine - (dynamicBytesPerLine % 8U));

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

void MemoryWindow::UpdateData(const std::vector<Byte> &p_data)
{
    data = p_data;

    UpdateData();

    if (isUpdateWindowSizeAction->isEnabled() && isUpdateWindowSize &&
        isRequestResize)
    {
        Resize();
        isRequestResize = false;
    }
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
    std::stringstream hexStream;
    int sliderPosition = 0;
    std::optional<DWord> startAddress;
    auto bytesPerLine = EstimateBytesPerLine();

    if (withAddress)
    {
        startAddress = addressRange.lower();
    }

    ConnectHexDumpCursorPositionChanged(false);

    const auto rowCol = flx::get_hex_dump_position_for_address(
            currentAddress, data.size(), bytesPerLine, withAscii,
            withAddress, currentType == flx::HexDumpType::AsciiChar,
            currentIsUpperNibble, addressRange.lower(), CurrentExtraSpace());
    flx::hex_dump_scale(scaleStream, bytesPerLine, withAscii, withAddress,
            CurrentExtraSpace());
    flx::hex_dump(hexStream, data.data(), data.size(), bytesPerLine,
            withAscii, withAddress, addressRange.lower(), CurrentExtraSpace());

    if (e_hexDump->verticalScrollBar() != nullptr)
    {
        sliderPosition = e_hexDump->verticalScrollBar()->sliderPosition();
    }
    const auto value = e_hexDump->horizontalScrollBar()->value();

    e_hexDumpScale->setPlainText(QString::fromStdString(scaleStream.str()));
    e_hexDump->setPlainText(QString::fromStdString(hexStream.str()));

    std::string line;
    columns = 0;
    rows = 0;
    while (std::getline(hexStream, line))
    {
        columns = std::max(columns, static_cast<int>(line.size()));
        ++rows;
    }

    if (rowCol.has_value())
    {
        currentRow = static_cast<int>(rowCol.value().first);
        currentColumn = static_cast<int>(rowCol.value().second);
        if (!withAscii)
        {
            UpdateValidCharacters(currentAddress, flx::HexDumpType::HexByte);
            currentType = flx::HexDumpType::HexByte;
        }

        const auto position = static_cast<int>(
                (columns + 1U) *
                (rowCol.value().first ? rowCol.value().first : 0U) +
                 rowCol.value().second);

        auto cursor = e_hexDump->textCursor();
        cursor.setPosition(position);
        e_hexDump->setTextCursor(cursor);
    }
    else
    {
        // row, column could not be estimated, try to position to first
        // character in first row.
        QTimer::singleShot(0, this,
                [&](){ e_hexDump->moveCursor(QTextCursor::NextCharacter); });
    }

    if (e_hexDump->verticalScrollBar() != nullptr)
    {
        e_hexDump->verticalScrollBar()->setSliderPosition(sliderPosition);
    }

    e_hexDump->horizontalScrollBar()->setValue(value);

    ConnectHexDumpCursorPositionChanged(true);
}

DWord MemoryWindow::EstimateBytesPerLine() const
{
    using T = std::underlying_type_t<MemoryWindow::Style>;

    if (style == MemoryWindow::Style::Dynamic)
    {
        return dynamicBytesPerLine;
    }

    return static_cast<T>(style);
}

void MemoryWindow::RequestResize()
{
    if (!isReadOnly)
    {
        if (isUpdateWindowSizeAction->isEnabled() && isUpdateWindowSize)
        {
            // Execute in event loop to also work when opening the window.
            QTimer::singleShot(100, this, [&](){
                Resize();
                isRequestResize = false;
            });
        };
        return;
    }

    isRequestResize = true;
}

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

QString MemoryWindow::CreateDefaultWindowTitle(
        const BInterval<DWord> &addressRange)
{
    const auto lowerStr = QString("%1").arg(addressRange.lower(), 4, 16,
            QLatin1Char('0')).toUpper();
    const auto upperStr = QString("%1").arg(addressRange.upper(), 4, 16,
            QLatin1Char('0')).toUpper();
    return QString("%1-%2").arg(lowerStr).arg(upperStr);
}

std::optional<DWord> MemoryWindow::CurrentExtraSpace() const
{
    return withExtraSpace ? std::optional<DWord>(8U) : std::nullopt;
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
    const auto offset = address - addressRange.lower();

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
    const auto offset = address - addressRange.lower();

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

    if (type == currentType)
    {
        return;
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
    return addressRange;
}

void MemoryWindow::UpdateAddressStatus(DWord address)
{
    const auto offset = address - addressRange.lower();
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
    toggleHexAsciiAction->setEnabled(withAscii & !isReadOnly);
}

// After changing a HEX or ASCII value the corresponding ASCII or HEX
// value has to be updated too.
// This function updates the corresponding value in e_hexDump wiget.
// The data vector is unchanged.
void MemoryWindow::ReplaceHexOrAsciiText(const QString &text) const
{
    if (!withAscii)
    {
        return;
    }

    ConnectHexDumpCursorPositionChanged(false);

    const auto rowCol = flx::get_hex_dump_position_for_address(
        currentAddress, data.size(), EstimateBytesPerLine(), withAscii,
        withAddress, currentType != flx::HexDumpType::AsciiChar,
        true, addressRange.lower(), CurrentExtraSpace());

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
