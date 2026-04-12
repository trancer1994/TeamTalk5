#pragma once

#include <QDialog>

class QLineEdit;

class NicknameDialog : public QDialog {
    Q_OBJECT
public:
    explicit NicknameDialog(QWidget* parent = nullptr);

    QString nickname() const;

private:
    QLineEdit* m_edit = nullptr;
};
