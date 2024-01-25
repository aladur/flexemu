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
#include "filecntb.h"
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
#include <cassert>


class FileAttributesUi
{
private:
    QVBoxLayout *verticalLayout_1;
    QLabel *l_filename;
    QGroupBox *groupBox;
    QVBoxLayout *verticalLayout_2;
    QCheckBox *c_protect[4];
    QDialogButtonBox *buttonBox;
    QVector<Qt::CheckState> initialState;
    Byte clearMask;
    Byte setMask;

    static QString protectChars;
    static QVector<const char *> protectText;
    static const QVector<Byte> flags;

public:
    FileAttributesUi() :
        verticalLayout_1(nullptr), l_filename(nullptr), groupBox(nullptr),
        verticalLayout_2(nullptr), c_protect{ },
        buttonBox(nullptr), clearMask(0), setMask(0)
    {
        assert(sizeof(c_protect) / sizeof(c_protect[0]) == flags.size());
        assert(protectText.size() == flags.size());
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

        bool isMultiFile = (filenames.size() > 1);
        l_filename->setText(isMultiFile ? "multiple files" : filenames.first());

        for (auto attribute : attributes)
        {
            for (int i = 0; i < flags.size(); ++i)
            {
                auto state = (attribute & flags.at(i)) ?
                    Qt::Checked : Qt::Unchecked;

                if (initialState.size() < flags.size())
                {
                    initialState.append(state);
                }
                else if (initialState.at(i) != state)
                {
                    initialState.replace(i, Qt::PartiallyChecked);
                }
            }
        }

        assert(sizeof(c_protect) / sizeof(c_protect[0]) == initialState.size());

        for (Word i = 0; i < (sizeof(c_protect) / sizeof(c_protect[0])); ++i)
        {
            bool isEnabled = !isWriteProtected && i < protectChars.size() &&
                             supportedAttributes.contains(protectChars[i]);
                             
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
            clearMask |= flags.at(index);
            setMask &= ~flags.at(index);
        }
        else if (state == Qt::Checked)
        {
            setMask |= flags.at(index);
            clearMask &= ~flags.at(index);
        }
        else if (state == Qt::PartiallyChecked)
        {
            setMask &= ~flags.at(index);
            clearMask &= ~flags.at(index);
        }
    }

    QString GetObjectName(QString text)
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

        for (Word i = 0; i < (sizeof(c_protect) / sizeof(c_protect[0])); ++i)
        {
            c_protect[i] = new QCheckBox(&dialog);
            c_protect[i]->setObjectName(GetObjectName(protectText[i]));
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
        for (Word i = 0; i < (sizeof(c_protect) / sizeof(c_protect[0])); ++i)
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
        for (int i = 0; i < protectText.size(); ++i)
        {
            c_protect[i]->setText(
                    QApplication::translate("Dialog", protectText[i]));
        }
    } // retranslateUi
};

QString FileAttributesUi::protectChars{"WDRC"};

QVector<const char *> FileAttributesUi::protectText{
    "&Write Protect", "&Delete Protect", "&Read Protect", "&Catalog Protect"
};

const QVector<Byte> FileAttributesUi::flags{
    WRITE_PROTECT, DELETE_PROTECT, READ_PROTECT, CATALOG_PROTECT
};
#endif

