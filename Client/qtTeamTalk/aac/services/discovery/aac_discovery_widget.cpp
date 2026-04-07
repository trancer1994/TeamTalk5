#include "aac_discovery_widget.h"

#include <QListView>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

#include "Client/qtTeamTalk/aac/services/serverservice.h"
#include "Client/qtTeamTalk/aac/services/aac_server_discovery.h"
#include "Client/qtTeamTalk/aac/services/aac_server_discovery_model.h"
#include "Client/qtTeamTalk/aac/models/serverinfo.h"

AACDiscoveryWidget::AACDiscoveryWidget(ServerService* service,
                                       QWidget* parent)
    : QWidget(parent)
    , m_service(service)
{
    m_discovery = new AACServerDiscovery(m_service, this);
    m_model = new AACServerDiscoveryModel(this);

    setupUi();
    wireSignals();
}

void AACDiscoveryWidget::setupUi()
{
    m_listView = new QListView(this);
    m_listView->setModel(m_model);
    m_listView->setSelectionMode(QAbstractItemView::SingleSelection);

    m_startButton = new QPushButton(tr("Start discovery"), this);
    m_refreshButton = new QPushButton(tr("Refresh"), this);
    m_selectButton = new QPushButton(tr("Select"), this);
    m_backButton = new QPushButton(tr("Back"), this);
    m_statusLabel = new QLabel(tr("Idle"), this);

    auto* buttonRow = new QHBoxLayout;
    buttonRow->addWidget(m_startButton);
    buttonRow->addWidget(m_refreshButton);
    buttonRow->addWidget(m_selectButton);
    buttonRow->addStretch();
    buttonRow->addWidget(m_backButton);

    auto* layout = new QVBoxLayout;
    layout->addWidget(m_statusLabel);
    layout->addWidget(m_listView);
    layout->addLayout(buttonRow);

    setLayout(layout);
}

void AACDiscoveryWidget::wireSignals()
{
    connect(m_startButton, &QPushButton::clicked,
            this, &AACDiscoveryWidget::onStartDiscovery);
    connect(m_refreshButton, &QPushButton::clicked,
            this, &AACDiscoveryWidget::onRefresh);
    connect(m_selectButton, &QPushButton::clicked,
            this, &AACDiscoveryWidget::onSelect);
    connect(m_backButton, &QPushButton::clicked,
            this, &AACDiscoveryWidget::backRequested);

    connect(m_discovery, &AACServerDiscovery::stateChanged,
            this, &AACDiscoveryWidget::onStateChanged);
    connect(m_discovery, &AACServerDiscovery::serverListCleared,
            m_model, &AACServerDiscoveryModel::clear);
    connect(m_discovery, &AACServerDiscovery::serverFound,
            m_model, &AACServerDiscoveryModel::addServer);
    connect(m_discovery, &AACServerDiscovery::errorOccurred,
            this, &AACDiscoveryWidget::onError);

    connect(m_model, &AACServerDiscoveryModel::selectionChanged,
            this, &AACDiscoveryWidget::onSelectionChanged);

    connect(m_listView, &QListView::clicked,
            this, [this](const QModelIndex& idx) {
        m_model->toggleSelection(idx.row());
    });
}

void AACDiscoveryWidget::onStartDiscovery()
{
    m_model->clear();
    m_discovery->startDiscovery();
}

void AACDiscoveryWidget::onRefresh()
{
    m_discovery->restart();
}

void AACDiscoveryWidget::onSelect()
{
    ServerInfo info = m_model->selectedServer();
    if (!info.isValid())
        return;

    m_discovery->selectServer(info);
    emit serverChosen(info);
}

void AACDiscoveryWidget::onSelectionChanged(const ServerInfo& info)
{
    // Optional: update status label
    m_statusLabel->setText(tr("Selected: %1 (%2)")
                           .arg(info.name.isEmpty() ? info.host : info.name)
                           .arg(info.host));
}

void AACDiscoveryWidget::onStateChanged(int state)
{
    using State = AACServerDiscovery::State;
    switch (static_cast<State>(state)) {
    case State::Idle:
        m_statusLabel->setText(tr("Idle"));
        break;
    case State::Discovering:
        m_statusLabel->setText(tr("Discovering servers..."));
        break;
    case State::Resolved:
        m_statusLabel->setText(tr("Server resolved"));
        break;
    }
}

void AACDiscoveryWidget::onError(const QString& message)
{
    m_statusLabel->setText(tr("Error: %1").arg(message));
}
