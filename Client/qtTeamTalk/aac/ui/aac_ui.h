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

private:
    AACGridModel* m_model = nullptr;
};

} // namespace AAC
