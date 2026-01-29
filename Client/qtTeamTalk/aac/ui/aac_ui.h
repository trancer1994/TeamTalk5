#pragma once

#include <QWidget>

namespace AAC {

class AACGridModel;

class AACUI : public QWidget
{
    Q_OBJECT

public:
    explicit AACUI(QWidget* parent = nullptr);

    void setModel(AACGridModel* model);

signals:
    void symbolActivated(const QString& label);
    void categorySelected(const QString& categoryId);

private:
    AACGridModel* m_model = nullptr;
};

} // namespace AAC
