/*                                                                              
    poutwin.cpp


    flexemu, an MC6809 emulator running FLEX
    Copyright (C) 2023  W. Schwotzer

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


#include "poutwin.h"
#include "warnoff.h"
#include "pprev_ui.h"
#include <QIODevice>
#include <QPageSize>
#include <QPageLayout>
#include <QFile>
#include <QFileInfo>
#include <QPrinter>
#include <QTextStream>
#include <QTextDocument>
#include <QWidget>
#include <QStackedWidget>
#include <QMenu>
#include <QMenuBar>
#include <QToolBar>
#include <QPushButton>
#include <QCheckBox>
#include <QComboBox>
#include <QTextBrowser>
#include <QMessageBox>
#include <QInputDialog>
#include <QFileDialog>
#include <QProgressDialog>
#include <QCloseEvent>
#include <QStatusTipEvent>
#include <QFont>
#include <QFontDatabase>
#include <QFontComboBox>
#include <QDirIterator>
#include <QPrintDialog>
#include <QStatusBar>
#include <QEventLoop>
#include "warnon.h"
#include <algorithm>
#include <iostream>
#include <cmath>
#include <set>
#include "qtfree.h"
#include "pagedet.h"
#include "soptions.h"

const QStringList pageSizeStrings{
    "DIN A3", "DIN A4", "DIN A5", "DIN A6",
    "DIN B3", "DIN B4", "DIN B5", "DIN B6",
    "Envelope C3", "Envelope C4", "Envelope C5", "Envelope C6",
    "#10 Envelope", "Envelope DL", "Envelope Monarch",
    "JIS B4", "JIS B5", "JIS B6",
    "Tabloid", "Legal", "Letter", "Executive", "Half Letter"
};
const QList<enum QPageSize::PageSizeId> pageSizeValues{
    QPageSize::A3, QPageSize::A4, QPageSize::A5, QPageSize::A6,
    QPageSize::B3, QPageSize::B4, QPageSize::B5, QPageSize::B6,
    QPageSize::EnvelopeC3, QPageSize::EnvelopeC4, QPageSize::EnvelopeC5,
    QPageSize::EnvelopeC6,
    QPageSize::Envelope10, QPageSize::EnvelopeDL, QPageSize::EnvelopeMonarch,
    QPageSize::JisB4, QPageSize::JisB5, QPageSize::JisB6,
    QPageSize::Tabloid, QPageSize::Legal, QPageSize::Letter,
    QPageSize::ExecutiveStandard, QPageSize::Statement
};
const QStringList unitShortStrings{ "mm", "in" };
const QStringList unitStrings{ "Millimeter (mm)", "Inches (in)" };
const QList<enum QPrinter::Unit> unitValues{
    QPrinter::Millimeter, QPrinter::Inch
};

/****************************
** Constructor, Destructor **
****************************/

PrintOutputWindow::PrintOutputWindow(sOptions &x_options) :
      mainLayout(nullptr)
    , toolBarLayout(nullptr)
    , statusBarFrame(nullptr)
    , menuBar(nullptr)
    , fileToolBar(nullptr)
    , editToolBar(nullptr)
    , fileMenu(nullptr)
    , editMenu(nullptr)
    , statusBar(nullptr)
    , printPreviewAction(nullptr)
    , pageBreakDetectorAction(nullptr)
    , fontComboBox(nullptr)
    , textBrowser(nullptr)
    , printer(nullptr)
    , ui(nullptr)
    , hasFixedFont(false)
    , linesPerPage(0U)
    , hasPageStructure(false)
    , isPageBreakDetectionFinished(false)
    , sizeFactor(1.4f)
    , unit(QPrinter::Millimeter)
    , orientation(QPageLayout::Portrait)
    , pageSizeId(QPageSize::A4)
    , characterCount(0)
    , lineCount(0)
    , options(x_options)
{
    setObjectName("PrintOutputWindow");
    setWindowTitle("FLEX Print Output");

    mainLayout = new QVBoxLayout(this);
    mainLayout->setObjectName(QString::fromUtf8("mainLayout"));
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    menuBar = new QMenuBar(this);
    mainLayout->addWidget(menuBar);
    toolBarLayout = new QHBoxLayout();
    toolBarLayout->setObjectName(QString::fromUtf8("toolBarLayout"));
    toolBarLayout->setContentsMargins(4, 2, 4, 2);
    toolBarLayout->setSpacing(2);
    mainLayout->addLayout(toolBarLayout);

    textBrowser = new QTextEdit(this);
    textBrowser->setMinimumSize(640, 800);
    textBrowser->setAutoFillBackground(true);
    textBrowser->setBackgroundRole(QPalette::Base);
    textBrowser->setReadOnly(true);
    mainLayout->addWidget(textBrowser);
    mainLayout->setStretchFactor(textBrowser, 1);

    CreateFileActions(*toolBarLayout);
    CreateEditActions(*toolBarLayout);
    CreateStatusBar(*mainLayout);

    toolBarLayout->addStretch(1);
    auto font = GetFont(options.printFont.c_str());
    fontComboBox->setCurrentFont(font);

    const auto printOutputIcon = QIcon(":/resource/print-output.png");
    setWindowIcon(printOutputIcon);

    connect(&cyclicTimer, &QTimer::timeout, this,
            &PrintOutputWindow::OnCyclicTimer);

    cyclicTimer.start(100);
}

PrintOutputWindow::~PrintOutputWindow()
{
    //QObject::dumpObjectTree();
    delete toolBarLayout;
    delete mainLayout;
    delete editMenu;
    delete fileMenu;
}

/*********************
** Static functions **
*********************/

/*
 Conversion from percent (0 ... 99) to the size factor
 used to scale the printer output.
 Precent range:      [0 ... 100[
 Size factor range:  [2.0 ... 0.8[
*/
float PrintOutputWindow::GetSizeFactorFromPercent(int percent)
{
    return 0.8f + (100 - percent) * 1.2f / 100.0f;
}

/*
 Conversion from percent (0 ... 99) to the display size in percent.
 Precent range:      [0 ... 100[
 Displaysize percent range:  [40 ... 160[
*/
float PrintOutputWindow::GetDisplayedSizeFactorFromPercent(int percent)
{
    return 40.0f + percent * 120.0f / 100.0f;
}

/*
 Conversion from size factor (2.0 ... 0.8) to percent value.
 Size factor range:  [2.0 ... 0.8[
 Precent range:      [0 ... 100[
*/
int PrintOutputWindow::GetPercentFromSizeFactor(float sizeFactor)
{
    auto temp = std::round((sizeFactor - 0.8) * 100.0f / 1.2f);
    return 100 - static_cast<int>(temp);
}

QMarginsF PrintOutputWindow::GetDefaultMarginsFor(QPageSize::PageSizeId)
{
    return QMarginsF(0.394, 0.394, 0.394, 0.394); // In Inch
}

/*****************
** Public Slots **
*****************/

void PrintOutputWindow::OnClearTextBrowser()
{
    textBrowser->clear();
    richLines.clear();
    isEmptyLine.clear();
    hasPageStructure = false;
    isPageBreakDetectionFinished = false;
}

void PrintOutputWindow::OnFontChanged(const QFont &newFont) const
{
    auto font = fontComboBox->currentFont();

    options.printFont = std::string(font.toString().toUtf8().data());
    SetTextBrowserFont(newFont);
}

void PrintOutputWindow::OnPageBreakDetectionToggled(bool checked) const
{
    options.isPrintPageBreakDetected = checked;
}

void PrintOutputWindow::OnLandscapeClicked(bool checked)
{
    if (ui != nullptr && checked)
    {
        orientation = QPageLayout::Landscape;
        previewPrinter->setPageOrientation(orientation);
        UpdatePaperWidthAndHeight(true);
    }
}

void PrintOutputWindow::OnMarginBottomChanged(double newValue)
{
    margins.setBottom(ToInches(newValue));
    ui->w_printPreview->updatePreview();
}

void PrintOutputWindow::OnMarginLeftChanged(double newValue)
{
    margins.setLeft(ToInches(newValue));
    ui->w_printPreview->updatePreview();
}

void PrintOutputWindow::OnMarginRightChanged(double newValue)
{
    margins.setRight(ToInches(newValue));
    ui->w_printPreview->updatePreview();
}

void PrintOutputWindow::OnMarginTopChanged(double newValue)
{
    margins.setTop(ToInches(newValue));
    ui->w_printPreview->updatePreview();
}

void PrintOutputWindow::OnOpenPrintPreview()
{
    previewPrinter = new QPrinter(QPrinter::ScreenResolution);
    auto *dialog = new QDialog(this);

    DetectPageBreaks();

    ui = new Ui::PrintPreview;
    ui->setupUi(dialog);

    ui->cb_pageSize->addItems(pageSizeStrings) ;
    ui->cb_unit->addItems(unitStrings);
    ui->ds_pageWidth->setMinimum(1.00);
    ui->ds_pageHeight->setMinimum(1.00);
    ui->ds_pageWidth->setMaximum(999.99);
    ui->ds_pageHeight->setMaximum(999.99);
    ui->ds_pageWidth->setReadOnly(true);
    ui->ds_pageHeight->setReadOnly(true);
    ui->ds_marginTop->setSingleStep(0.1);
    ui->ds_marginBottom->setSingleStep(0.1);
    ui->ds_marginLeft->setSingleStep(0.1);
    ui->ds_marginRight->setSingleStep(0.1);

    InitializeUnit();
    InitializeOrientation();
    InitializePageSize();
    margins = GetMarginsFor(pageSizeId);
    UpdateMargins();
    UpdatePaperWidthAndHeight(false);

    connect(ui->cb_unit,
#if (QT_VERSION <= QT_VERSION_CHECK(5, 7, 0))
            static_cast<void (QComboBox::*)(int)>(
            &QComboBox::currentIndexChanged),
#else
            QOverload<int>::of(&QComboBox::currentIndexChanged),
#endif
            this, &PrintOutputWindow::OnUnitChanged);
    connect(ui->cb_pageSize,
#if (QT_VERSION <= QT_VERSION_CHECK(5, 7, 0))
            static_cast<void (QComboBox::*)(int)>(
            &QComboBox::currentIndexChanged),
#else
            QOverload<int>::of(&QComboBox::currentIndexChanged),
#endif
            this, &PrintOutputWindow::OnPageSizeChanged);
    connect(ui->s_sizeAdjustment, &QSlider::valueChanged,
            this, &PrintOutputWindow::OnSizeFactorChanged);
    connect(ui->p_minus, &QPushButton::clicked,
            this, &PrintOutputWindow::OnSizeFactorDecrement);
    connect(ui->p_plus, &QPushButton::clicked,
            this, &PrintOutputWindow::OnSizeFactorIncrement);
    connect(ui->rb_portrait, &QRadioButton::clicked,
            this, &PrintOutputWindow::OnPortraitClicked);
    connect(ui->rb_landscape, &QRadioButton::clicked,
            this, &PrintOutputWindow::OnLandscapeClicked);
    connect(ui->ds_marginBottom,
#if (QT_VERSION <= QT_VERSION_CHECK(5, 7, 0))
            static_cast<void (QDoubleSpinBox::*)(double)>(
            &QDoubleSpinBox::valueChanged),
#else
            QOverload<double>::of(&QDoubleSpinBox::valueChanged),
#endif
            this, &PrintOutputWindow::OnMarginBottomChanged);
    connect(ui->ds_marginLeft,
#if (QT_VERSION <= QT_VERSION_CHECK(5, 7, 0))
            static_cast<void (QDoubleSpinBox::*)(double)>(
            &QDoubleSpinBox::valueChanged),
#else
            QOverload<double>::of(&QDoubleSpinBox::valueChanged),
#endif
            this, &PrintOutputWindow::OnMarginLeftChanged);
    connect(ui->ds_marginRight,
#if (QT_VERSION <= QT_VERSION_CHECK(5, 7, 0))
            static_cast<void (QDoubleSpinBox::*)(double)>(
            &QDoubleSpinBox::valueChanged),
#else
            QOverload<double>::of(&QDoubleSpinBox::valueChanged),
#endif
            this, &PrintOutputWindow::OnMarginRightChanged);
    connect(ui->ds_marginTop,
#if (QT_VERSION <= QT_VERSION_CHECK(5, 7, 0))
            static_cast<void (QDoubleSpinBox::*)(double)>(
            &QDoubleSpinBox::valueChanged),
#else
            QOverload<double>::of(&QDoubleSpinBox::valueChanged),
#endif
            this, &PrintOutputWindow::OnMarginTopChanged);
    connect(ui->b_print, &QPushButton::clicked,
            this, &PrintOutputWindow::OnPrint);
    connect(ui->w_printPreview, &QPrintPreviewWidget::paintRequested,
            this, &PrintOutputWindow::OnPaintRequested);
    connect(ui->w_printPreview, &QPrintPreviewWidget::previewChanged,
            this, &PrintOutputWindow::OnPreviewChanged);

    auto value = GetPercentFromSizeFactor(sizeFactor);
    if (!value)
    {
        // This forces a valueChanged() event if value == 0.
        ui->s_sizeAdjustment->setValue(value + 1);
    }
    ui->s_sizeAdjustment->setTickInterval(10);
    ui->s_sizeAdjustment->setTracking(true);
    ui->s_sizeAdjustment->setValue(value);
    ui->s_sizeAdjustment->setTickPosition(QSlider::TicksAbove);

    dialog->exec();

    SetMarginsFor(pageSizeId, margins);
    delete previewPrinter;
    previewPrinter = nullptr;
    delete(ui);
    ui = nullptr;
}

void PrintOutputWindow::OnPageSizeChanged(int index)
{
    if (ui != nullptr && index >= 0)
    {
        SetMarginsFor(pageSizeId, margins);
        pageSizeId = pageSizeValues[index];
        previewPrinter->setPageSize(QPageSize(pageSizeId));
        margins = GetMarginsFor(pageSizeId);
        UpdateMargins();
        UpdatePaperWidthAndHeight(true);
    }
}

void PrintOutputWindow::OnPortraitClicked(bool checked)
{
    if (ui != nullptr && checked)
    {
        orientation = QPageLayout::Portrait;
        previewPrinter->setPageOrientation(orientation);
        UpdatePaperWidthAndHeight(true);
    }
}

void PrintOutputWindow::OnPaintRequested(QPrinter *p) const
{
    QTextDocument doc;
    QPageLayout layout;

    layout.setMode(QPageLayout::StandardMode);
    layout.setOrientation(orientation);
    layout.setUnits(QPageLayout::Inch);
    layout.setPageSize(QPageSize(pageSizeId));
    layout.setMargins(margins);
    p->setPageLayout(layout);
    bool isInvalid = !p->setPageMargins(margins, QPageLayout::Inch);
    SetMarginsInfo(isInvalid);

    const auto pageSize = p->pageLayout().paintRect(QPageLayout::Point).size();
    doc.setPageSize(pageSize * sizeFactor);
    doc.setHtml(textBrowser->toHtml());
    doc.print(p);

    characterCount = doc.characterCount();
    lineCount = doc.lineCount();
}

void PrintOutputWindow::OnPreviewChanged()
{
    if (ui != nullptr)
    {
        auto currentPage = ui->w_printPreview->currentPage();
        auto pageCount = ui->w_printPreview->pageCount();
        auto text = tr("Page %1 of %2").arg(currentPage).arg(pageCount);
        ui->l_pages->setText(text);
        text = tr("%1 Characters, %2 Lines").arg(characterCount).arg(lineCount);
        ui->l_counts->setText(text);
        if (hasPageStructure)
        {
            text = tr("%1 Lines per page").arg(linesPerPage);
            ui->l_linesPerPage->setText(text);
        }
    }
}

void PrintOutputWindow::OnPrint(bool)
{
    printer = new QPrinter(QPrinter::HighResolution);

    OpenPrintDialog(printer);
    delete printer;
    printer = nullptr;
}

void PrintOutputWindow::OnSaveAsHtml()
{
    QString title("Save to HTML File");
    auto filename =
        QFileDialog::getSaveFileName(this, title, QString(), "*.html");

    if (!filename.isEmpty())
    {
        if (QFileInfo(filename).suffix().isEmpty())
        {
            filename.append(".html");
        }

        QFile file(filename);

        auto mode = QIODevice::WriteOnly | QIODevice::Truncate |
                    QIODevice::Text;
        if (!file.open(mode))
        {
            auto message = QString("Error opening file ") +
                file.fileName().toUtf8().data() + " for writing";
            QMessageBox::warning(this, "Warning", message);
            return;
        }

        QTextStream outStream(&file);
        outStream << textBrowser->toHtml();
    }
}

void PrintOutputWindow::OnSaveAsMarkdown()
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    QString title("Save to Markdown File");
    auto filename =
        QFileDialog::getSaveFileName(this, title, QString(), "*.md");

    if (!filename.isEmpty())
    {
        if (QFileInfo(filename).suffix().isEmpty())
        {
            filename.append(".md");
        }

        QFile file(filename);

        auto mode = QIODevice::WriteOnly | QIODevice::Truncate |
                    QIODevice::Text;
        if (!file.open(mode))
        {
            auto message = QString("Error opening file ") +
                file.fileName().toUtf8().data() + " for writing";
            QMessageBox::warning(this, "Warning", message);
            return;
        }

        QTextStream outStream(&file);
        outStream << textBrowser->toMarkdown();
    }
#endif
}

void PrintOutputWindow::OnSaveAsPlainText()
{
    QString title("Save to Text File");
    auto filename =
        QFileDialog::getSaveFileName(this, title, QString(), "*.txt");

    if (!filename.isEmpty())
    {
        if (QFileInfo(filename).suffix().isEmpty())
        {
            filename.append(".txt");
        }

        QFile file(filename);

        auto mode = QIODevice::WriteOnly | QIODevice::Truncate |
                    QIODevice::Text;
        if (!file.open(mode))
        {
            auto message = QString("Error opening file ") +
                file.fileName().toUtf8().data() + " for writing";
            QMessageBox::warning(this, "Warning", message);
            return;
        }

        QTextStream outStream(&file);
        outStream << textBrowser->toPlainText();
    }
}

void PrintOutputWindow::OnSizeFactorChanged(int value)
{
    if (ui != nullptr)
    {
        sizeFactor = GetSizeFactorFromPercent(value);
        auto displayValue = GetDisplayedSizeFactorFromPercent(value);
        auto displayValueString = QString("%1 %").arg(displayValue, 0, 'f', 0);
        ui->l_sizeAdjustment->setText(displayValueString);
        ui->w_printPreview->updatePreview();
    }
}

void PrintOutputWindow::OnSizeFactorDecrement()
{
    if (ui != nullptr)
    {
        ui->s_sizeAdjustment->triggerAction(
                QAbstractSlider::SliderSingleStepSub);
    }
}

void PrintOutputWindow::OnSizeFactorIncrement()
{
    if (ui != nullptr)
    {
        ui->s_sizeAdjustment->triggerAction(
                QAbstractSlider::SliderSingleStepAdd);
    }
}

void PrintOutputWindow::OnCyclicTimer()
{
    ProcessSerialInput();

    cyclicTimer.start(20);
}

void PrintOutputWindow::OnUnitChanged(int index)
{
    if (ui != nullptr && index >= 0)
    {
        unit = unitValues[index];
        UpdateMargins();
        SetSpinBoxUnit(unitShortStrings[index]);
        UpdatePaperWidthAndHeight(true);
    }
}

/*****************************
** Private member functions **
*****************************/

void PrintOutputWindow::CreateEditActions(QBoxLayout &layout)
{
    QAction *action;
    QToolBar *toolBar;

    editMenu = menuBar->addMenu(tr("&Edit"));
    toolBar = CreateToolBar(this, tr("Edit"), QStringLiteral("editToolBar"));
    layout.addWidget(toolBar);

    const auto clearIcon = QIcon(":/resource/print-clear.png");
    action = editMenu->addAction(clearIcon, tr("&Clear"));
    connect(action, &QAction::triggered, this,
            &PrintOutputWindow::OnClearTextBrowser);
    action->setStatusTip(tr("Clear print output"));
    toolBar->addAction(action);

    const auto detectIcon = QIcon(":/resource/detect-page-break.png");
    action = editMenu->addAction(detectIcon, tr("&Autodetect page breaks"));
    action->setStatusTip(tr(
                "Automatically detect page breaks in print output"));
    action->setCheckable(true);
    action->setChecked(options.isPrintPageBreakDetected);
    toolBar->addAction(action);
    pageBreakDetectorAction = action;

    fontComboBox = new QFontComboBox();
    fontComboBox->setEditable(false);
    fontComboBox->setFontFilters(QFontComboBox::MonospacedFonts);
    toolBar->addWidget(fontComboBox);

    // Add signal/slot actions
    connect(fontComboBox, &QFontComboBox::currentFontChanged,
            this, &PrintOutputWindow::OnFontChanged);
    connect(pageBreakDetectorAction, &QAction::toggled,
            this, &PrintOutputWindow::OnPageBreakDetectionToggled);

    editToolBar = toolBar;
}

void PrintOutputWindow::CreateFileActions(QBoxLayout &layout)
{
    QAction *action;
    QToolBar *toolBar;

    fileMenu = menuBar->addMenu(tr("&File"));
    toolBar = CreateToolBar(this, tr("File"), QStringLiteral("fileToolBar"));
    layout.addWidget(toolBar);

    const auto previewIcon = QIcon(":/resource/print-preview.png");
    action = fileMenu->addAction(previewIcon, tr("&Print Preview ..."));
    connect(action, &QAction::triggered, this,
            &PrintOutputWindow::OnOpenPrintPreview);
    action->setStatusTip(tr("Open print preview for print output"));
    printPreviewAction = action;

    action = fileMenu->addAction(tr("Save as &HTML ..."));
    connect(action, &QAction::triggered, this,
            &PrintOutputWindow::OnSaveAsHtml);
    action->setStatusTip(tr("Save print output to a HTML file"));
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    action = fileMenu->addAction(tr("Save as &Markdown ..."));
    connect(action, &QAction::triggered, this,
            &PrintOutputWindow::OnSaveAsMarkdown);
    action->setStatusTip(tr("Save print output to a markdown file"));
#endif
    action = fileMenu->addAction(tr("Save as &Text ..."));
    connect(action, &QAction::triggered, this,
            &PrintOutputWindow::OnSaveAsPlainText);
    action->setStatusTip(tr("Save print output to a plain text file"));

    fileMenu->addSeparator();

    const auto closeIcon = QIcon(":/resource/window-close.png");
    action = fileMenu->addAction(closeIcon, tr("&Close"));
    connect(action, &QAction::triggered, this, &PrintOutputWindow::hide);
    action->setStatusTip(tr("Close this window"));
    toolBar->addAction(action);
    toolBar->addAction(printPreviewAction);
    fileToolBar = toolBar;
}

void PrintOutputWindow::CreateStatusBar(QBoxLayout &layout)
{
    // Use QStackedWidget to be able to set a frame style.
    statusBarFrame = new QStackedWidget();
    statusBar = new QStatusBar();
    statusBar->setSizeGripEnabled(true);
    statusBarFrame->addWidget(statusBar);
    layout.addWidget(statusBarFrame);
    statusBarFrame->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    statusBar->showMessage(tr("Ready"));
}

void PrintOutputWindow::DetectPageBreaks()
{
    std::set<size_t> ignored;
    std::set<size_t> pageBreak;
    size_t index;
    PageDetector detector(richLines);

    if (!isPageBreakDetectionFinished &&
        pageBreakDetectorAction->isChecked() &&
        detector.HasLinesPerPageDetected())
    {
        hasPageStructure = true;
        linesPerPage = detector.GetLinesPerPage();

        for (index = linesPerPage - 6U; index < richLines.size();
             index += linesPerPage)
        {
            size_t highestOffset = 0U;

            // For empty lines set up to last 5 lines of a page to ignored.
            for (size_t offset = 5U; offset > 0U; --offset)
            {
                if ((index + offset) < richLines.size() &&
                    richLines[index + offset].empty())
                {
                    if (highestOffset == 0U)
                    {
                        highestOffset = offset;
                    }
                    ignored.emplace(index + offset);
                }
            }

            // Set empty line with highest offset to page break.
            if (highestOffset > 0U)
            {
                pageBreak.emplace(index + highestOffset);
            }
        }

        // Remove all empty lines backwards from the end. Also remove last
        // page break.
        bool pageBreakRemoved = false;
        index = richLines.size() - 1U;
        while (index > 0U && richLines[index].empty())
        {
            ignored.emplace(index);
            const auto iter = pageBreak.find(index);
            if (!pageBreakRemoved && (iter != pageBreak.end()))
            {
                pageBreak.erase(iter);
                pageBreakRemoved = true;
            }
            --index;
        }

        textBrowser->clear();
        auto progress =
            QProgressDialog(tr("Reformatting ..."), tr("Cancel"), 0, 100, this);
        progress.setCancelButton(nullptr);

        // Print lines which are not ignored. Also print page breaks.
        for (index = 0U; index < richLines.size(); ++index)
        {
            if (ignored.find(index) == ignored.end())
            {
                const auto iter = pageBreak.find(index);
                bool isPageBreak = (iter != pageBreak.end());

                progress.setValue(index * 100 / richLines.size());
                PrintLine(richLines[index], isPageBreak);
                QGuiApplication::processEvents(
                        QEventLoop::ExcludeUserInputEvents);
            }
        }
    }

    isPageBreakDetectionFinished = true;
}

// Return the margins for a given page size ID.
// The margins have unit Inches.
QMarginsF PrintOutputWindow::GetMarginsFor(QPageSize::PageSizeId id) const
{
    const auto iter = marginsForPageSize.find(id);
    if (iter != marginsForPageSize.cend())
    {
        return iter->second;
    }
    else
    {
        return GetDefaultMarginsFor(pageSizeId);
    }
}

QToolBar *PrintOutputWindow::CreateToolBar(QWidget *parent,
                                           const QString &title,
                                           const QString &objectName)
{
    QToolBar *toolBar = new QToolBar(title, parent);
    toolBar->setObjectName(objectName);
    toolBar->setFloatable(false);
    toolBar->setMovable(false);
    toolBar->setIconSize({32, 32});
    QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    toolBar->setSizePolicy(sizePolicy);
    toolBarLayout->addWidget(toolBar, 1, Qt::AlignLeft);

    return toolBar;
}

void PrintOutputWindow::InitializeOrientation()
{
    if (orientation == QPageLayout::Portrait)
    {
        ui->rb_portrait->click();
    }
    else if (orientation == QPageLayout::Landscape)
    {
        ui->rb_landscape->click();
    }
}

void PrintOutputWindow::InitializePageSize()
{
    auto index = pageSizeValues.indexOf(pageSizeId);
   
    if (index >= 0)
    { 
        ui->cb_pageSize->setCurrentText(pageSizeStrings[index]);
    }
}

void PrintOutputWindow::InitializeUnit()
{
    auto index = unitValues.indexOf(unit);
   
    if (index >= 0)
    { 
        ui->cb_unit->setCurrentText(unitStrings[index]);
        SetSpinBoxUnit(unitShortStrings[index]);
    }
}

void PrintOutputWindow::OpenPrintDialog(QPrinter *p)
{
    QPrintDialog printDialog(p, this);

    if (printDialog.exec() == QDialog::Accepted)
    {
        OnPaintRequested(p);
    }
}

// Set the margins for a given page size ID.
// The margins have unit Inches.
void PrintOutputWindow::SetMarginsFor(QPageSize::PageSizeId id,
        const QMarginsF &newMargins)
{
    marginsForPageSize[id] = newMargins;
}

void PrintOutputWindow::SetMarginsInfo(bool isInvalid) const
{
    if (ui != nullptr)
    {
        ui->l_marginInfo->setStyleSheet("color: red; font-weight: bold");
        ui->l_marginInfo->setText(isInvalid ? "!!!" : "");
    }
}

void PrintOutputWindow::SetSpinBoxUnit(const QString &p_unit)
{
    ui->ds_pageWidth->setSuffix(p_unit);
    ui->ds_pageHeight->setSuffix(p_unit);
    ui->ds_marginTop->setSuffix(p_unit);
    ui->ds_marginBottom->setSuffix(p_unit);
    ui->ds_marginLeft->setSuffix(p_unit);
    ui->ds_marginRight->setSuffix(p_unit);
}

void PrintOutputWindow::SetTextBrowserFont(const QFont &font) const
{
    auto *doc = textBrowser->document();

    doc->setDefaultFont(font);
}

double PrintOutputWindow::ToInches(double value) const
{
    if (unit == QPrinter::Millimeter)
    {
        return value / 2.54; // Convert millimeters into inches
    }

    return value;
}

void PrintOutputWindow::UpdateMargins()
{
    double factor = 1.0;

    if (unit == QPrinter::Millimeter)
    {
        factor = 2.54;
    }

    ui->ds_marginTop->setValue(margins.top() * factor);
    ui->ds_marginBottom->setValue(margins.bottom() * factor);
    ui->ds_marginLeft->setValue(margins.left() * factor);
    ui->ds_marginRight->setValue(margins.right() * factor);
}

void PrintOutputWindow::UpdatePaperWidthAndHeight(bool doUpdate)
{
    auto rect = previewPrinter->paperRect(unit);

    ui->ds_pageWidth->setValue(rect.width());
    ui->ds_pageHeight->setValue(rect.height());
    if (doUpdate)
    {
        ui->w_printPreview->updatePreview();
    }
}

/*********************************
** Receive flexemu print output **
*********************************/

void PrintOutputWindow::PrintLine(const RichLine &richLine,
                                  bool isPageBreak) const
{
    auto cursor = textBrowser->textCursor();
    auto previousProperty = CharProperty::Normal;
    bool isFirst = true;

    textBrowser->setUpdatesEnabled(false);
    for (auto &richChar : richLine)
    {
        isFirst ? cursor.beginEditBlock() : cursor.joinPreviousEditBlock();
        isFirst = false;

        if (richChar.properties != previousProperty)
        {
            auto charFormat = cursor.charFormat();
            auto isUnderlined = ((richChar.properties & CharProperty::Underlined)
                                 == CharProperty::Underlined);
            // DoubleStrike is displayed as italic
            auto isItalic = ((richChar.properties & CharProperty::DoubleStrike)
                             == CharProperty::DoubleStrike);
            auto isBold = ((richChar.properties & CharProperty::Bold)
                           == CharProperty::Bold);
            charFormat.setFontUnderline(isUnderlined);
            charFormat.setFontItalic(isItalic);
            auto fontWeight = QFont::Normal;
            fontWeight = isBold ? QFont::Bold : fontWeight;
            charFormat.setFontWeight(fontWeight);
            cursor.setCharFormat(charFormat);

            previousProperty = richChar.properties;
        }

        cursor.insertText(QString(richChar.character));
        cursor.endEditBlock();
    }

    isFirst ? cursor.beginEditBlock() : cursor.joinPreviousEditBlock();
    auto charFormat = cursor.charFormat();
    charFormat.setFontUnderline(false);
    charFormat.setFontItalic(false);
    charFormat.setFontWeight(QFont::Normal);
    cursor.setCharFormat(charFormat);
    cursor.insertText(QString("\n"));
    QTextFormat::PageBreakFlags flags = isPageBreak ?
        QTextFormat::PageBreak_AlwaysAfter : QTextFormat::PageBreak_Auto;
    auto blockFormat = cursor.blockFormat();
    blockFormat.setPageBreakPolicy(flags);
    cursor.setBlockFormat(blockFormat);
    cursor.endEditBlock();
    textBrowser->setTextCursor(cursor);
    textBrowser->setUpdatesEnabled(true);
    textBrowser->repaint();
}

void PrintOutputWindow::write_char_serial(Byte value)
{
    // The received serial values are recieved in the CPU-thread
    // but have to be processed in the UI-thread.
    std::lock_guard<std::mutex> guard(serial_mutex);

    serial_buffer.push_back(value);
}

bool PrintOutputWindow::HasSerialInput() const
{
    std::lock_guard<std::mutex> guard(serial_mutex);

    return !serial_buffer.empty();
}

void PrintOutputWindow::ProcessSerialInput()
{
    Byte value = '\0';

    while (HasSerialInput())
    {
        {
            std::lock_guard<std::mutex> guard(serial_mutex);

            value = serial_buffer.front();
            serial_buffer.pop_front();
        }

        bool isLineFinished = overlayHelper.AddCharacter(value);
        if (isLineFinished)
        {
            isEmptyLine.push_back(overlayHelper.IsRichLineEmpty());
            richLines.push_back(overlayHelper.GetRichLine());
            PrintLine(overlayHelper.GetRichLine());
            QGuiApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
        }

        if (!isVisible())
        {
            show();
            raise();
        }
    }
}

/*******************
** Event handlers **
*******************/

void PrintOutputWindow::closeEvent(QCloseEvent *event)
{
    event->accept();
}

bool PrintOutputWindow::event(QEvent *event)
{
    if (event->type() == QEvent::StatusTip)
    {
        auto *statusTipEvent = static_cast<QStatusTipEvent *>(event);
        statusBar->showMessage(statusTipEvent->tip());
        statusTipEvent->accept();

        return true;
    }

    return QWidget::event(event);
}

