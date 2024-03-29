/*
    poutwin.h


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2023-2024  W. Schwotzer

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

#ifndef PRINTOUTPUTWINDOW_INCLUDED
#define PRINTOUTPUTWINDOW_INCLUDED

#include "misc1.h"
#include "warnoff.h"
#include <QString>
#include <QTimer>
#include <QWidget>
#include <QPageLayout>
#include <QPageSize>
#include <QPrinter>
#include <QMarginsF>
#include "warnon.h"
#include <vector>
#include <deque>
#include <mutex>
#include "poverhlp.h"


class QStackedWidget;
class QTextEdit;
class QMenu;
class QMenuBar;
class QToolBar;
class QBoxLayout;
class QHBoxLayout;
class QVBoxLayout;
class QComboBox;
class QFile;
class QTextStream;
class QCloseEvent;
class QFontComboBox;
class QFont;
class QStatusBar;
class QAction;
namespace Ui {
class PrintPreview;
}
struct sOptions;


class PrintOutputWindow : public QWidget
{
    Q_OBJECT

public:
    PrintOutputWindow() = delete;
    PrintOutputWindow(sOptions &x_options);
    ~PrintOutputWindow() override;

    // Static functions
    static float GetSizeFactorFromPercent(int percent);
    static float GetDisplayedSizeFactorFromPercent(int percent);
    static int GetPercentFromSizeFactor(float sizeFactor);
    static QMarginsF GetDefaultMarginsFor(QPageSize::PageSizeId id);

    // Static constants
    static const char separator; // Separator for print config values

    // Receive flexemu print output
    void write_char_serial(Byte value);

public slots:
    void OnClearTextBrowser();
    void OnCyclicTimer();
    void OnFontChanged(const QFont &newFont) const;
    void OnHideWindow();
    void OnInitializePrintPreview();
    void OnPageBreakDetectionToggled(bool checked) const;
    void OnLandscapeClicked(bool checked);
    void OnMarginBottomChanged(double newValue);
    void OnMarginLeftChanged(double newValue);
    void OnMarginRightChanged(double newValue);
    void OnMarginTopChanged(double newValue);
    void OnOpenPrintPreview();
    void OnPageSizeChanged(int index);
    void OnPaintRequested(QPrinter *printer) const;
    void OnPortraitClicked(bool checked);
    void OnPreviewChanged();
    void OnPrint(bool checked);
    void OnSaveAsHtml();
    void OnSaveAsMarkdown();
    void OnSaveAsPlainText();
    void OnSizeFactorChanged(int value);
    void OnSizeFactorDecrement();
    void OnSizeFactorIncrement();
    void OnUpdateGeometry();
    void OnUnitChanged(int index);

private:
    // Private member functions
    void CreateEditActions(QBoxLayout &layout);
    void CreateFileActions(QBoxLayout &layout);
    void CreateStatusBar(QBoxLayout &layout);
    QToolBar *CreateToolBar(QWidget *parent, const QString &title,
                            const QString &objectName);
    void DetectPageBreaks();
    std::string CreatePrintConfigKey() const;
    bool HasSerialInput() const;
    void InitializeOrientation();
    void InitializePageSizeWidget();
    void InitializePageWidthAndHeightWidgets();
    void InitializeSizeAdjustmentWidget();
    void InitializeUnitWidget();
    void OpenPrintDialog(QPrinter *printer);
    void ProcessSerialInput();
    void RestorePrintConfig();
    void SavePrintConfig();
    void SetMarginsInfo(bool isInvalid) const;
    void SetTextBrowserFont(const QFont &font) const;
    double ToMillimeter(double displayValue) const;
    void UpdateMarginWidgets();
    void UpdatePageWidthAndHeightWidgets();
    void UpdateSizeAdjustmentWidget() const;
    void UpdateSpinBoxUnit(const QString &unit);

    // Process flexemu print output
    void PrintLine(const RichLine &richLine, bool isPageBreak = false) const;

    // Event handlers
    void closeEvent(QCloseEvent *event) override;
    bool event(QEvent *event) override;

    // Private member variables
    QTimer cyclicTimer;
    QVBoxLayout *mainLayout;
    QHBoxLayout *toolBarLayout;
    QStackedWidget *statusBarFrame;
    QMenuBar *menuBar;
    QToolBar *fileToolBar;
    QToolBar *editToolBar;
    QMenu *fileMenu;
    QMenu *editMenu;
    QStatusBar *statusBar;
    QAction *printPreviewAction;
    QAction *pageBreakDetectorAction;
    QFontComboBox *fontComboBox;
    QTextEdit *textBrowser;
    QPrinter *printer;
    QPrinter *previewPrinter;
    Ui::PrintPreview *ui;

    bool hasFixedFont;
    PrintOverlayHelper overlayHelper;
    RichLines richLines;
    std::vector<bool> isEmptyLine;
    size_t linesPerPage;
    bool hasPageStructure;
    bool isPageBreakDetectionFinished;
    float sizeFactor;
    enum QPrinter::Unit unit;
    enum QPageLayout::Orientation orientation;
    enum QPageSize::PageSizeId pageSizeId;
    QMarginsF margins; // Unit: Millimeter, displayed in Centimeter
    mutable int characterCount;
    mutable int lineCount;
    mutable std::mutex serial_mutex;
    std::deque<Byte> serial_buffer;
    sOptions &options;
    bool isDeferPreviewUpdate;
};

#endif

