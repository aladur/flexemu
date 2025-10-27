/*
    fpattrui.h


    FLEXplorer, An explorer for FLEX disk image files and directory disks.
    Copyright (C) 2020-2025  W. Schwotzer

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


#ifndef FPATTRUI_H
#define FPATTRUI_H

#include "typedefs.h"
#include "fattrib.h"
#include "warnoff.h"
#include <QApplication>
#include <QCheckBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QButtonGroup>
#include <QVBoxLayout>
#include <QLabel>
#include <QString>
#include <QStringList>
#include <QVector>
#include <QVarLengthArray>
#include "warnon.h"
#include <cassert>


class FileAttributesUi
{
private:
    using AttributeLabels_t = QVarLengthArray<const char *, 4>;

    QVBoxLayout *verticalLayout_1{nullptr};
    QLabel *l_filename{nullptr};
    QGroupBox *groupBox{nullptr};
    QVBoxLayout *verticalLayout_2{nullptr};
    QVarLengthArray<QCheckBox *, 4> c_protect{nullptr, nullptr,
                                              nullptr, nullptr};
    QDialogButtonBox *buttonBox{nullptr};
    QVector<Qt::CheckState> initialState;
    Byte clearMask{0};
    Byte setMask{0};

    static const AttributeLabels_t &GetAttributeLabels();

public:
    FileAttributesUi()
    {
        assert(c_protect.size() ==
                static_cast<int>(GetAttributeCharToFlag().size()));
        assert(c_protect.size() == GetAttributeLabels().size());
    }

    void TransferDataToDialog(const QStringList &filenames,
                              const QVector<Byte> &attributes,
                              const QString &supportedAttributes,
                              bool isWriteProtected)
    {
        if (l_filename == nullptr)
        {
            throw std::logic_error("setupUi(dialog) has to be called before.");
        }

        assert(filenames.size() == attributes.size());

        initialState.clear();
        bool isMultiFile = (filenames.size() > 1);
        l_filename->setText(isMultiFile ? "multiple files" : filenames.first());

        const auto &attributeCharToFlag = GetAttributeCharToFlag();
        for (auto attribute : attributes)
        {
            auto i = 0;
            for (const auto &iter : attributeCharToFlag)
            {
                auto state = (attribute & iter.second) ?
                    Qt::Checked : Qt::Unchecked;

                if (initialState.size() <
                        static_cast<int>(attributeCharToFlag.size()))
                {
                    // Initially set checked or unchecked state.
                    initialState.append(state);
                }
                else if (initialState.at(i) != state)
                {
                    // Multiple states detected, set partially checked.
                    initialState.replace(i, Qt::PartiallyChecked);
                }
                ++i;
            }
        }

        assert(c_protect.size() == initialState.size());

        for (Word i = 0; i < static_cast<Word>(c_protect.size()); ++i)
        {
            bool isEnabled =
                !isWriteProtected && i < attributeCharToFlag.size() &&
                supportedAttributes.contains(attributeCharToFlag[i].first);

            auto isTristate =
                     isMultiFile && initialState.at(i) == Qt::PartiallyChecked;
            c_protect[i]->setTristate(isTristate);
            c_protect[i]->setCheckState(initialState.at(i));
            c_protect[i]->setEnabled(isEnabled);
        }
    }

    Byte GetSetMask() const
    {
        return setMask;
    }

    Byte GetClearMask() const
    {
        return clearMask;
    }

    void CheckBoxStateChanged(int state, int index)
    {
        const auto &attributeCharToFlag = GetAttributeCharToFlag();
        if (state == Qt::Unchecked)
        {
            clearMask |= attributeCharToFlag.at(index).second;
            setMask &= static_cast<Byte>(~attributeCharToFlag.at(index).second);
        }
        else if (state == Qt::Checked)
        {
            setMask |= attributeCharToFlag.at(index).second;
            clearMask &=
                static_cast<Byte>(~attributeCharToFlag.at(index).second);
        }
        else if (state == Qt::PartiallyChecked)
        {
            setMask &= static_cast<Byte>(~attributeCharToFlag.at(index).second);
            clearMask &=
                static_cast<Byte>(~attributeCharToFlag.at(index).second);
        }
    }

    static QString GetObjectName(QString text)
    {
        return text.remove(' ').remove('&').prepend("c_");
    }

    void setupUi(QDialog &dialog)
    {
        if (dialog.objectName().isEmpty())
        {
            dialog.setObjectName(QStringLiteral("Dialog"));
        }
        dialog.resize({0, 0});
        dialog.setSizeGripEnabled(true);
        dialog.setModal(true);

        verticalLayout_1 = new QVBoxLayout(&dialog);
        verticalLayout_1->setObjectName(QStringLiteral("verticalLayout_1"));

        l_filename = new QLabel(&dialog);
        l_filename->setObjectName(QStringLiteral("l_filename"));
        verticalLayout_1->addWidget(l_filename);

        groupBox = new QGroupBox(&dialog);
        groupBox->setObjectName(QStringLiteral("groupBox"));
        verticalLayout_2 = new QVBoxLayout();
        verticalLayout_2->setSpacing(0);
        verticalLayout_2->setObjectName(QStringLiteral("verticalLayout_2"));
        verticalLayout_2->setContentsMargins(0, 0, 0, 0);
        const auto &attributeLabels = GetAttributeLabels();

        for (Word i = 0; i < static_cast<Word>(c_protect.size()); ++i)
        {
            c_protect[i] = new QCheckBox(&dialog);
            c_protect[i]->setObjectName(GetObjectName(attributeLabels[i]));
            verticalLayout_2->addWidget(c_protect[i]);
        }

        groupBox->setLayout(verticalLayout_2);
        verticalLayout_1->addWidget(groupBox);

        buttonBox = new QDialogButtonBox(&dialog);
        buttonBox->setObjectName(QStringLiteral("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        auto buttons = QDialogButtonBox::Cancel | QDialogButtonBox::Ok;
        buttonBox->setStandardButtons(buttons);
        verticalLayout_1->addWidget(buttonBox);
        verticalLayout_1->setStretch(0,1);
        verticalLayout_1->setStretch(1,5);

        retranslateUi(dialog);

        QObject::connect(buttonBox, &QDialogButtonBox::accepted,
                         &dialog, &QDialog::accept);
        QObject::connect(buttonBox, &QDialogButtonBox::rejected,
                         &dialog, &QDialog::reject);
        for (Word i = 0; i < static_cast<Word>(c_protect.size()); ++i)
        {
            QObject::connect(c_protect[i], &QCheckBox::stateChanged,
                [&, i](int state){ CheckBoxStateChanged(state, i); });
        }

        QMetaObject::connectSlotsByName(&dialog);
    }

    void retranslateUi(QDialog &dialog)
    {
        dialog.setWindowTitle(
                QApplication::translate("Dialog", "File Attributes"));
        l_filename->setText(QString());
        groupBox->setTitle(QString());
        const auto &attributeLabels = GetAttributeLabels();
        for (int i = 0; i < attributeLabels.size(); ++i)
        {
            c_protect[i]->setText(
                    QApplication::translate("Dialog", attributeLabels[i]));
        }
    }
};

#endif

