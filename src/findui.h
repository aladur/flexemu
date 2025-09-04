/*
    findui.h


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


#ifndef FINDSETTINGSUI_INCLUDE
#define FINDSETTINGSUI_INCLUDE

#include "warnoff.h"
#ifdef USE_CMAKE
#include "ui_find.h"
#else
#include "find_ui.h"
#endif
#include <QObject>
#include <QString>
#include "warnon.h"
#include <string>

class QDialog;
class QAbstractButton;

class FindSettingsUi : public QObject, public Ui_FindSettings
{
    Q_OBJECT

public:
    enum class Find : uint8_t
    {
        AsciiString,
        RegexString,
        HexBytes,
    };

    struct sConfig
    {
        FindSettingsUi::Find searchType{};
        bool isCaseSensitive{true};
        std::string searchFor;
    };

    using Config_t = struct sConfig;

    FindSettingsUi();
    ~FindSettingsUi() override = default;
    void setupUi(QDialog &dialog);
    void SetData(const FindSettingsUi::Config_t &config) const;
    void GetData(FindSettingsUi::Config_t &config) const;

protected:
    void OnAsciiClicked(bool isChecked) const;
    void OnHexClicked(bool isChecked) const;
    void OnRegexClicked(bool isChecked) const;
    void OnSearchForChanged(const QString &newText) const;
    void OnValidationCheckClicked() const;

    void UpdatePlaceholderText() const;
    void InitializeWidgets() const;
    void ConnectSignalsWithSlots() const;

private:
    QDialog *dialog{nullptr};
    mutable QString validationError;
};

#endif
