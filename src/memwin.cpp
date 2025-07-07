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
#include "memwin.h"
#include "bintervl.h"
#include "warnoff.h"
#include <QString>
#include <QStringList>
#include <QApplication>
#include <QGuiApplication>
#include <QTimer>
#include <QScreen>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFontMetrics>
#include <QWidget>
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
#include <QCloseEvent>
#include <QDebug>
#include "warnon.h"
#include "qtfree.h"
#include <algorithm>
#include <optional>
#include <cmath>
#include <cassert>
#include <sstream>


/****************************
** Constructor, Destructor **
****************************/

MemoryWindow::MemoryWindow(
        bool p_isReadOnly,
        const BInterval<DWord> &p_addressRange,
        QString p_windowTitle,
        MemoryWindow::Style p_style,
        bool p_withAddress,
        bool p_withAscii,
        bool p_withExtraSpace,
        bool p_isUpdateWindowSize)
    : mainLayout(new QVBoxLayout(this))
    , toolBarLayout(new QHBoxLayout)
    , statusBarFrame(nullptr)
    , menuBar(new QMenuBar(this))
    , toolBar(nullptr)
    , fileMenu(nullptr)
    , viewMenu(nullptr)
    , statusBar(nullptr)
    , e_hexDumpScale(new QTextEdit(this))
    , e_hexDump(new QTextEdit(this))
    , styleComboBox(new QComboBox(this))
    , withAddressAction(nullptr)
    , withAsciiAction(nullptr)
    , isUpdateWindowSizeAction(nullptr)
    , withExtraSpaceAction(nullptr)
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
    , isFirstUpdate(true)
    , columns(0)
    , rows(0)
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
    e_hexDump->setTextInteractionFlags(Qt::NoTextInteraction);
    mainLayout->addWidget(e_hexDump);
    mainLayout->setStretchFactor(e_hexDump, 1);
    connect(e_hexDump, &QTextEdit::cursorPositionChanged,
            this, &MemoryWindow::OnTextCursorPositionChanged);
    connect(e_hexDump->horizontalScrollBar(),
#if (QT_VERSION <= QT_VERSION_CHECK(5, 7, 0))
            static_cast<void (QAbstractSlider::*)(int)>(
                &QAbstractSlider::valueChanged),
#else
            QOverload<int>::of(&QAbstractSlider::valueChanged),
#endif
            this, &MemoryWindow::OnHorizontalScrollBarValueChanged);

    CreateActions(*toolBarLayout, iconSize);
    CreateStatusBar(*mainLayout);
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
void MemoryWindow::OnFontChanged(const QFont &newFont) const
{
    SetTextBrowserFont(newFont);
}

void MemoryWindow::OnUpdateFont() const
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

void MemoryWindow::OnHorizontalScrollBarValueChanged(int value) const
{
    e_hexDumpScale->horizontalScrollBar()->setValue(value);
}

void MemoryWindow::OnTextCursorPositionChanged() const
{
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
    connect(action, &QAction::triggered, this,
            &MemoryWindow::OnClose);
    action->setStatusTip(tr("Close this window"));
    p_toolBar.addAction(action);
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
        styleAction.push_back(CreateAction(nullptr, *styleMenu, styleString,
                    index,
                    GetStyleStatusTips()[index], GetStyleHotKeys()[index]));
        ++index;
    }
    InitializeStyleWidget();

    auto icon = QIcon(":/resource/hex-address.png");
    withAddressAction = viewMenu->addAction(icon,
            tr("Display A&ddress Column"));
    connect(withAddressAction, &QAction::triggered, this,
            &MemoryWindow::OnToggleDisplayAddresses);
    withAddressAction->setCheckable(true);
    withAddressAction->setShortcut(QKeySequence(tr("Ctrl+R")));
    withAddressAction->setStatusTip(tr("Toggle displaying address column"));
    withAddressAction->setChecked(withAddress);
    p_toolBar.addAction(withAddressAction);

    icon = QIcon(":/resource/ascii.png");
    withAsciiAction = viewMenu->addAction(icon, tr("Display &ASCII Column"));
    connect(withAsciiAction, &QAction::triggered, this,
            &MemoryWindow::OnToggleDisplayAscii);
    withAsciiAction->setCheckable(true);
    withAsciiAction->setShortcut(QKeySequence(tr("Ctrl+A")));
    withAsciiAction->setStatusTip(tr("Toggle displaying ASCII column"));
    withAsciiAction->setChecked(withAscii);
    p_toolBar.addAction(withAsciiAction);

    icon = QIcon(":/resource/extra-space.png");
    withExtraSpaceAction =
        viewMenu->addAction(icon, tr("Display &Extra Space"));
    connect(withExtraSpaceAction, &QAction::triggered, this,
            &MemoryWindow::OnToggleExtraSpace);
    withExtraSpaceAction->setCheckable(true);
    withExtraSpaceAction->setShortcut(QKeySequence(tr("Ctrl+E")));
    withExtraSpaceAction->setStatusTip(
            tr("Display extra space after each 8 byte"));
    withExtraSpaceAction->setChecked(withExtraSpace);
    p_toolBar.addAction(withExtraSpaceAction);

    icon = QIcon(":/resource/autoupdate-winsize.png");
    isUpdateWindowSizeAction = viewMenu->addAction(icon,
            tr("Automatic &Update Window Size"));
    connect(isUpdateWindowSizeAction, &QAction::triggered, this,
            &MemoryWindow::OnToggleUpdateWindowSize);
    isUpdateWindowSizeAction->setCheckable(true);
    isUpdateWindowSizeAction->setShortcut(QKeySequence(tr("Ctrl+U")));
    isUpdateWindowSizeAction->setStatusTip(
            tr("Automatic window size update"));
    isUpdateWindowSizeAction->setChecked(isUpdateWindowSize);
    p_toolBar.addAction(isUpdateWindowSizeAction);
}

void MemoryWindow::CreateStatusBar(QBoxLayout &layout)
{
    // Use QStackedWidget to be able to set a frame style.
    statusBarFrame = new QStackedWidget();
    statusBar = new QStatusBar();
    statusBar->setSizeGripEnabled(true);
    statusBarFrame->addWidget(statusBar);
    statusBarFrame->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    layout.addWidget(statusBarFrame);
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

void MemoryWindow::SetTextBrowserFont(const QFont &font) const
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
    if (isReadOnly || isFirstUpdate)
    {
        UpdateData();
        isFirstUpdate = false;
    }

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
}

void MemoryWindow::UpdateData()
{
    std::stringstream scaleStream;
    std::stringstream hexStream;
    int sliderPosition = 0;
    std::optional<DWord> startAddress;
    std::optional<DWord> extraSpace;
    auto bytesPerLine = EstimateBytesPerLine();

    if (withAddress)
    {
        startAddress = addressRange.lower();
    }

    if (withExtraSpace)
    {
        extraSpace = 8U;
    }

    flx::hex_dump_scale(scaleStream, bytesPerLine, withAscii, withAddress,
            extraSpace);
    flx::hex_dump(hexStream, data.data(), data.size(), bytesPerLine,
            withAscii, withAddress, addressRange.lower(), extraSpace);

    const auto positionStart = e_hexDump->textCursor().selectionStart();
    const auto positionEnd = e_hexDump->textCursor().selectionEnd();
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

    if (positionStart != 0 || positionEnd != 0)
    {
        auto cursor = e_hexDump->textCursor();
        cursor.setPosition(positionStart);
        cursor.setPosition(positionEnd, QTextCursor::KeepAnchor);
        e_hexDump->setTextCursor(cursor);
    }

    if (e_hexDump->verticalScrollBar() != nullptr)
    {
        e_hexDump->verticalScrollBar()->setSliderPosition(sliderPosition);
    }

    e_hexDump->horizontalScrollBar()->setValue(value);
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

void MemoryWindow::RequestResize() const
{
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
        styleAction[actionIdx]->setChecked(actionIdx == index);
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
