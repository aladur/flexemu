/*
    fpattrui.h


    FLEXplorer, An explorer for any FLEX file or disk container
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


#ifndef FPATTRUI_H
#define FPATTRUI_H

#include "misc1.h"
#include "fattrib.h"
#include "filecntb.h"
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
#include <QVector>
#include <QVarLengthArray>
#include "warnon.h"
#include <cassert>


class FileAttributesUi
{
private:
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

    static const QVarLengthArray<const char *, 4> attributeLabel;

public:
    FileAttributesUi()
    {
        assert(c_protect.size() ==
                static_cast<int>(attributeCharToFlag.size()));
        assert(c_protect.size() == attributeLabel.size());
    }

    void TransferDataToDialog(const QVector<QString> &filenames,
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
        if (state == Qt::Unchecked)
        {
            clearMask |= attributeCharToFlag.at(index).second;
            setMask &= ~attributeCharToFlag.at(index).second;
        }
        else if (state == Qt::Checked)
        {
            setMask |= attributeCharToFlag.at(index).second;
            clearMask &= ~attributeCharToFlag.at(index).second;
        }
        else if (state == Qt::PartiallyChecked)
        {
            setMask &= ~attributeCharToFlag.at(index).second;
            clearMask &= ~attributeCharToFlag.at(index).second;
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

        for (Word i = 0; i < static_cast<Word>(c_protect.size()); ++i)
        {
            c_protect[i] = new QCheckBox(&dialog);
            c_protect[i]->setObjectName(GetObjectName(attributeLabel[i]));
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
    } // setupUi

    void retranslateUi(QDialog &dialog)
    {
        dialog.setWindowTitle(
                QApplication::translate("Dialog", "File Attributes"));
        l_filename->setText(QString());
        groupBox->setTitle(QString());
        for (int i = 0; i < attributeLabel.size(); ++i)
        {
            c_protect[i]->setText(
                    QApplication::translate("Dialog", attributeLabel[i]));
        }
    } // retranslateUi
};

#endif

