/*
    findui.cpp


    Flexemu, an MC6809 emulator running FLEX
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


#include "config.h"
#include "findui.h"
#include "qtfree.h"
#include "warnoff.h"
#include "ui_find.h"
#include <QLatin1Char>
#include <QString>
#include <QValidator>
#include <QIcon>
#include <QDialog>
#include <QDialogButtonBox>
#include <QAbstractButton>
#include <QLineEdit>
#include <QMessageBox>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include "warnon.h"
#include <cassert>
#include <regex>


using StdRegexValidator = RegexValidator<std::regex>;

FindSettingsUi::FindSettingsUi() :
    Ui_FindSettings()
{
}

void FindSettingsUi::setupUi(QDialog &p_dialog)
{
    dialog = &p_dialog;
    Ui_FindSettings::setupUi(dialog);

    assert(g_searchType != nullptr);
    assert(r_ascii != nullptr);
    assert(r_regex != nullptr);
    assert(r_hex != nullptr);
    assert(c_caseSensitive != nullptr);
    assert(e_searchFor != nullptr);
    assert(b_validationCheck != nullptr);
    assert(c_buttonBox != nullptr);

    InitializeWidgets();
    ConnectSignalsWithSlots();
}

void FindSettingsUi::InitializeWidgets() const
{
    const auto findIcon = QIcon(":/resource/find.png");
    const auto checkFailedIcon = QIcon(":/resource/check-failed.png");

    dialog->setWindowIcon(findIcon);
    b_validationCheck->setIcon(checkFailedIcon);
}

void FindSettingsUi::SetData(const FindSettingsUi::Config_t &config) const
{
    switch (config.searchType)
    {
        case Find::AsciiString:
            r_ascii->click();
            break;

        case Find::RegexString:
            r_regex->click();
            break;

        case Find::HexBytes:
            r_hex->click();
            break;
    }

    c_caseSensitive->setChecked(config.isCaseSensitive);
    e_searchFor->setText("X");
    e_searchFor->setText(QString::fromStdString(config.searchFor));
    e_searchFor->setFocus(Qt::OtherFocusReason);
}

void FindSettingsUi::GetData(FindSettingsUi::Config_t &config) const
{
    config.searchType = r_ascii->isChecked() ? Find::AsciiString :
        (r_regex->isChecked() ? Find::RegexString : Find::HexBytes);
    config.isCaseSensitive = c_caseSensitive->isChecked();
    config.searchFor = e_searchFor->text().toStdString();
}


void FindSettingsUi::ConnectSignalsWithSlots() const
{
    connect(r_ascii, &QAbstractButton::clicked, this,
            &FindSettingsUi::OnAsciiClicked);
    connect(r_regex, &QAbstractButton::clicked, this,
            &FindSettingsUi::OnRegexClicked);
    connect(r_hex, &QAbstractButton::clicked, this,
            &FindSettingsUi::OnHexClicked);
    connect(e_searchFor, &QLineEdit::textChanged, this,
            &FindSettingsUi::OnSearchForChanged);
    connect(b_validationCheck, &QAbstractButton::clicked, this,
            &FindSettingsUi::OnValidationCheckClicked);
}

void FindSettingsUi::OnAsciiClicked(bool isChecked) const
{
    c_caseSensitive->setEnabled(isChecked);
    UpdateRegexValidator("^[ -~]+$", e_searchFor);
    UpdatePlaceholderText();
    validationError = tr("Enter at least one ASCII character");
    OnSearchForChanged(e_searchFor->text());
}

void FindSettingsUi::OnRegexClicked(bool isChecked) const
{
    c_caseSensitive->setEnabled(isChecked);
    auto *validator =
        new StdRegexValidator(std::regex_constants::extended, e_searchFor);
    e_searchFor->setValidator(validator);
    UpdatePlaceholderText();
    validationError = "";
    // Force a validation with current text.
    e_searchFor->selectAll();
    e_searchFor->insert(e_searchFor->text());
}

void FindSettingsUi::OnHexClicked(bool isChecked) const
{
    c_caseSensitive->setEnabled(!isChecked);
    UpdateRegexValidator("^([0-9A-Fa-f][0-9A-Fa-f])+$", e_searchFor);
    UpdatePlaceholderText();
    validationError = tr("Enter 2 hexadecimal digits for each byte, "
            "at least one byte");

    std::regex regex("^[0-9A-Fa-f]*$");
    if (!std::regex_match(e_searchFor->text().toStdString(), regex))
    {
        // Invalid input has to be removed.
        e_searchFor->setText("");
        return;
    }

    // Force a validation with current text.
    e_searchFor->selectAll();
    e_searchFor->insert(e_searchFor->text());
}

void FindSettingsUi::OnSearchForChanged(const QString &newText) const
{
    const auto *validator =
        dynamic_cast<const StdRegexValidator *>(e_searchFor->validator());
    bool isRegexOk = validator != nullptr &&
        (validator->GetState().first == QValidator::State::Acceptable);

    if (r_regex->isChecked())
    {
        validationError = e_searchFor->text().isEmpty() ?
            "Enter a regular expression" :
            ((validator == nullptr ?
                "" : QString::fromStdString(validator->GetState().second)));
    }

    const bool isValid = (!newText.isEmpty() &&
        (r_ascii->isChecked() ||
         (r_regex->isChecked() && isRegexOk) ||
         (r_hex->isChecked() && newText.size() % 2 == 0)));

    b_validationCheck->setVisible(!isValid);
    c_buttonBox->button(QDialogButtonBox::Ok)->setEnabled(isValid);
}

void FindSettingsUi::OnValidationCheckClicked() const
{
    QMessageBox::critical(dialog, PACKAGE_NAME " Error", validationError);
    e_searchFor->setFocus(Qt::OtherFocusReason);
}

void FindSettingsUi::UpdatePlaceholderText() const
{
    QString placeholderText;

    if (r_ascii->isChecked())
    {
        placeholderText = tr("e.g. Apollo");
    }
    else if (r_regex->isChecked())
    {
        placeholderText = tr("e.g. [A-Za-z_][0-9A-Za-z_]*");
    }
    else
    {
        placeholderText = tr("e.g. AA55F00F");
    }

    e_searchFor->setPlaceholderText(placeholderText);
}
