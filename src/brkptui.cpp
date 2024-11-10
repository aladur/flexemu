/*
    brkptui.cpp


    Flexemu, an MC6809 emulator running FLEX
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


#include "misc1.h"
#include "bui.h"
#include "brkptui.h"
#include "efslctle.h"
#include <stdexcept>
#include <cassert>
#include "warnoff.h"
#include <QAbstractButton>
#include "warnon.h"

using OptionalWord = std::optional<Word>;

BreakpointSettingsUi::BreakpointSettingsUi() :
    Ui_BreakpointSettings()
{
}

void BreakpointSettingsUi::setupUi(QDialog &p_dialog)
{
    dialog = &p_dialog;
    Ui_BreakpointSettings::setupUi(dialog);

    assert(e_breakpoint1 != nullptr);
    assert(e_breakpoint2 != nullptr);

    InitializeWidgets();
    ConnectSignalsWithSlots();
}

void BreakpointSettingsUi::InitializeWidgets()
{
    ::InstallSelectionEventFilter(*e_breakpoint1, this);
    ::InstallSelectionEventFilter(*e_breakpoint2, this);

    QString inputMask(">hhhh");
    e_breakpoint1->setInputMask(inputMask);
    e_breakpoint2->setInputMask(inputMask);
}

void BreakpointSettingsUi::SetData(const BPArray &breakpoints)
{
    if (dialog == nullptr)
    {
        throw std::logic_error("setupUi(dialog) has to be called before.");
    }

    ::SetData<OptionalWord>(breakpoints[0], *e_breakpoint1);
    ::SetData<OptionalWord>(breakpoints[1], *e_breakpoint2);
}

BPArray BreakpointSettingsUi::GetData() const
{
    BPArray breakpoints;

    breakpoints[0] = ::GetData<OptionalWord>(*e_breakpoint1);
    breakpoints[1] = ::GetData<OptionalWord>(*e_breakpoint2);

    return breakpoints;
}

void BreakpointSettingsUi::ConnectSignalsWithSlots()
{
    connect(c_buttonBox, &QDialogButtonBox::accepted,
            this, &BreakpointSettingsUi::OnAccepted);
    connect(c_buttonBox, &QDialogButtonBox::rejected,
            this, &BreakpointSettingsUi::OnRejected);
    connect(c_buttonBox, &QDialogButtonBox::clicked,
            this, &BreakpointSettingsUi::OnClicked);
}

void BreakpointSettingsUi::OnAccepted()
{
    dialog->done(QDialog::Accepted);
}

void BreakpointSettingsUi::OnRejected()
{
    dialog->done(QDialog::Rejected);
}

void BreakpointSettingsUi::OnClicked(QAbstractButton *button)
{
    assert(button != nullptr);

    if (button->text() == "Reset")
    {
        e_breakpoint1->clear();
        e_breakpoint2->clear();
    }
}

