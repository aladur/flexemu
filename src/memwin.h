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
#include "warnoff.h"
#include <QObject>
#include <QString>
#include <QWidget>
#include <QList>
#include <QStringList>
#include <memory>
#include "warnon.h"
#include <vector>


class QSize;
class QEvent;
class QCloseEvent;
class QResizeEvent;
class QBoxLayout;
class QVBoxLayout;
class QHBoxLayout;
class QFont;
class QStackedWidget;
class QMenu;
class QMenuBar;
class QToolBar;
class QStatusBar;
class QMenu;
class QTextEdit;
class QComboBox;

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

    MemoryWindow() = delete;
    explicit MemoryWindow(
            bool p_isReadOnly,
            const BInterval<DWord> &p_addressRange,
            QString p_windowTitle = "",
            MemoryWindow::Style p_style = MemoryWindow::Style::Dynamic,
            bool p_withAddress = true,
            bool p_withAscii = true,
            bool p_withExtraSpace = false,
            bool p_isUpdateWindowSize = true);

    // Static functions
    static const StyleValues_t &GetStyleValues();
    static const QStringList &GetStyleStrings();
    static const QStringList &GetStyleShortStrings();
    static const QStringList &GetStyleStatusTips();
    static const QStringList &GetStyleHotKeys();
    static DWord GetBytesPerLine(MemoryWindow::Style style);
    static QString CreateDefaultWindowTitle(
            const BInterval<DWord> &addressRange);

    void SetIconSize(const QSize &iconSize);
    void UpdateData(const std::vector<Byte> &p_data);
    void SetReadOnly(bool p_isReadOnly);

public slots:
    void OnClose();
    void OnFontChanged(const QFont &newFont) const;
    void OnUpdateFont() const;
    void OnStyleChanged(int index);
    void OnStyleHighlighted(int index) const;
    void OnToggleDisplayAddresses();
    void OnToggleDisplayAscii();
    void OnToggleUpdateWindowSize();
    void OnToggleExtraSpace();
    void OnHorizontalScrollBarValueChanged(int value) const;
    void OnTextCursorPositionChanged() const;

signals:
    void Closed(MemoryWindow *p_win);

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
    void SetTextBrowserFont(const QFont &font) const;
    void SetStatusMessage(const QString &message) const;
    DWord EstimateBytesPerLine() const;
    void UpdateData();
    void UpdateStyleCheck(int index) const;
    void UpdateStyleValue(int index) const;
    void RequestResize() const;
    void Resize();

    // Event handlers
    void changeEvent(QEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    bool event(QEvent *event) override;

    // Protected member variables
    QVBoxLayout *mainLayout;
    QHBoxLayout *toolBarLayout;
    QStackedWidget *statusBarFrame;
    QMenuBar *menuBar;
    QToolBar *toolBar;
    QMenu *fileMenu;
    QMenu *viewMenu;
    QStatusBar *statusBar;
    QTextEdit *e_hexDumpScale;
    QTextEdit *e_hexDump;
    QComboBox *styleComboBox;
    QAction *withAddressAction;
    QAction *withAsciiAction;
    QAction *isUpdateWindowSizeAction;
    QAction *withExtraSpaceAction;
    std::vector<QAction *> styleAction;
    BInterval<DWord> addressRange;
    QString windowTitle;
    MemoryWindow::Style style;
    DWord dynamicBytesPerLine;
    bool withAddress;
    bool withAscii;
    bool withExtraSpace;
    bool isUpdateWindowSize;
    bool isReadOnly;
    bool isFirstResizeEvent;
    int columns;
    int rows;
    mutable bool isRequestResize;
    std::vector<Byte> data;
};

using MemoryWindowSPtr = std::shared_ptr<MemoryWindow>;
#endif
