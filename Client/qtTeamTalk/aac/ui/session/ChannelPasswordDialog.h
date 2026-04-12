#pragma once

#include <QDialog>

class QLineEdit;

class ChannelPasswordDialog : public QDialog {
    Q_OBJECT
public:
    explicit ChannelPasswordDialog(QWidget* parent = nullptr);

    QString password() const;

private:
    QLineEdit* m_edit = nullptr;
};
