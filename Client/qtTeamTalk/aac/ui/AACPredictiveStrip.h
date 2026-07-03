#pragma once

#include <QWidget>
#include <QTimer>
#include <QList>
#include <vector>
#include <string>

class QHBoxLayout;
class AACAccessibilityManager;
class AACTextBar;
class AACKeyButton;

class PredictiveStrip : public QWidget
{
    Q_OBJECT
public:
    explicit PredictiveStrip(QWidget* parent = nullptr);

    void setManager(AACAccessibilityManager* mgr);
    void setTextBar(AACTextBar* tb);
    std::vector<std::string> currentSuggestionList() const;
    void clear();
    void setSemanticContext(const QString& tag);
    void clearSemanticContext();
int m_maxSuggestions = 5;
void setMaxSuggestions(int n);
    QList<QWidget*> interactiveWidgets() const;
    QList<QWidget*> primaryWidgets() const;

public slots:
    void setContext(const QString& text);
    void onCharacterTyped(const QString& ch);

signals:
    void suggestionChosen(const QString& word);

private slots:
    void debouncedUpdate();

private:
    void updateButtons(const std::vector<std::string>& suggestions);
void applyAACProperties(AACKeyButton* btn);
void applyConfidenceStyling(AACKeyButton* btn, float conf, float semanticWeight);
void applyAdaptiveSizing(AACKeyButton* btn);

    AACAccessibilityManager* m_mgr = nullptr;
    AACTextBar* m_textBar = nullptr;
    QHBoxLayout* m_layout = nullptr;

    QTimer m_debounceTimer;
    QLabel* m_semanticPreviewLabel = nullptr;
    QString m_currentSemanticTag;
    QString m_pendingContext;
QStringList m_lastSuggestionList;

QList<AACKeyButton*> m_buttons;
};
