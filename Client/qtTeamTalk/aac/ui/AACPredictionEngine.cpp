#include "AACPredictionEngine.h"
#include "AACFramework.h"

#include <QRegularExpression>
#include <QFile>
#include <QTextStream>

#include <algorithm>
#include <cctype>
#include <limits>
#include <cmath>

AACPredictionEngine::AACPredictionEngine(AACAccessibilityManager* mgr,
                                         QObject* parent)
    : QObject(parent)
    , m_mgr(mgr)
{
    loadDefaultSemanticPhrases();
}

// -------------------------
// Semantic helpers
// -------------------------

bool AACPredictionEngine::semanticActive() const
{
    if (m_currentSemanticTag.isEmpty())
        return false;

    int age = m_recencyCounter - m_semanticSetTick;
    return age >= 0 && age <= m_semanticDecayWindow;
}

void AACPredictionEngine::decaySemanticIfNeeded() const
{
    if (!semanticActive()) {
        // semantic context expired; tag remains but stops contributing
        return;
    }
}

float AACPredictionEngine::semanticWeightFor(const std::string& candidate) const
{
    if (!semanticActive())
        return 0.0f;

    float score = 0.0f;

    // Strong boost for last symbol word
    if (!m_lastSymbolWord.empty() && candidate == m_lastSymbolWord)
        score += 2.5f;

    // Boost words that appear in symbol phrases for current tag
    auto it = m_symbolPhrases.find(m_currentSemanticTag.toLower().toStdString());
    if (it != m_symbolPhrases.end()) {
        for (const auto& phrase : it->second) {
            QString qPhrase = QString::fromStdString(phrase);
            QStringList parts = qPhrase.split(QRegularExpression("\\s+"),
                                              Qt::SkipEmptyParts);
            for (const QString& p : parts) {
                if (p.toLower().toStdString() == candidate) {
                    score += 1.0f;
                    break;
                }
            }
        }
    }

    // Light boost for category presence
    if (!m_currentCategory.isEmpty())
        score += 0.3f;

    // Category blending (emotion + social, needs + emotion, etc.)
    const QString cat = m_currentCategory.toLower();
    if (cat == "emotion" || cat == "social")
        score += 0.2f;
    else if (cat == "needs")
        score += 0.15f;

    // Decay with age
    int age = m_recencyCounter - m_semanticSetTick;
    if (age > 0) {
        float factor = std::max(0.2f, 1.0f - (age / float(m_semanticDecayWindow * 2)));
        score *= factor;
    }

    return score;
}

float AACPredictionEngine::semanticWeightForToken(const std::string& token) const
{
    return semanticWeightFor(token);
}
void AACPredictionEngine::setSemanticContext(const QString& tag)
{
    if (tag.isEmpty())
    m_currentSemanticTag.clear();
    m_currentCategory.clear();
    m_lastSymbolWord.clear();
    emit semanticContextChanged(QString());
        return;

    m_currentSemanticTag = tag.trimmed();
    m_semanticSetTick = m_recencyCounter;

    const QString lower = m_currentSemanticTag.toLower();

    // Category + last-symbol-word mapping
    if (lower.startsWith("emotion_")) {
        setCurrentCategory("emotion");
        setLastSymbolWord(lower.mid(QString("emotion_").length()));
    }
    else if (lower.startsWith("need_")) {
        setCurrentCategory("needs");
        setLastSymbolWord(lower.mid(QString("need_").length()));
    }
    else if (lower.startsWith("social_")) {
        setCurrentCategory("social");
        setLastSymbolWord(lower.mid(QString("social_").length()));
    }
    else if (lower == "yes" || lower == "no") {
        setCurrentCategory("response");
        setLastSymbolWord(lower);
    }
    else if (lower == "question") {
        setCurrentCategory("question");
        setLastSymbolWord("question");
    }
    else {
        // Fallback: treat tag itself as the last symbol word
        setLastSymbolWord(lower);
    }

    emit semanticContextChanged(m_currentSemanticTag);
}

// -------------------------
// Tokenization helpers (2,3)
// -------------------------

QString AACPredictionEngine::normalizePunctuation(const QString& text) const
{
    QString cleaned = text;
    cleaned.replace(".", " . ");
    cleaned.replace("!", " ! ");
    cleaned.replace("?", " ? ");
    cleaned.replace(",", " , "); // for afterComma context
    return cleaned;
}

std::vector<std::string> AACPredictionEngine::tokenize(const QString& text) const
{
    QString cleaned = normalizePunctuation(text);

    QStringList parts = cleaned.split(QRegularExpression("\\s+"),
                                      Qt::SkipEmptyParts);

    std::vector<std::string> out;
    out.reserve(parts.size());
    for (const auto& p : parts)
        out.push_back(p.toLower().toStdString());

    return out;
}

// -------------------------
// Stage 2 + 7 + 8 + 10: learning
// -------------------------

void AACPredictionEngine::learnUtterance(const QString& text)
{
    auto tokens = tokenize(text);
    if (tokens.empty())
        return;

    // Unigrams + word seen counts (Stage 2 + 10)
    for (const auto& w : tokens) {
        ++m_unigram[w];
        m_cachedTotalUnigrams = -1; // invalidate cache

        int& seen = m_wordSeenCount[w];
        ++seen;
        // Stage 10: add to custom dictionary on second occurrence
        if (seen == 2)
            m_customWords.insert(w);
    }

    // Bigrams (Stage 2)
    for (size_t i = 1; i < tokens.size(); ++i) {
        const std::string& prev = tokens[i - 1];
        const std::string& next = tokens[i];
        ++m_bigram[prev][next];
    }

    // Trigrams (Stage 8)
    if (tokens.size() >= 3) {
        for (size_t i = 2; i < tokens.size(); ++i) {
            const std::string& w1 = tokens[i - 2];
            const std::string& w2 = tokens[i - 1];
            const std::string& w3 = tokens[i];
            ++m_trigram[w1][w2][w3];
        }
    }

    // Stage 7: phrase memory
    const QString trimmed = text.trimmed();
    if (!trimmed.isEmpty()) {
        std::string key = trimmed.toLower().toStdString();
        auto& entry = m_phrases[key];
        entry.phrase = key;
        entry.count += 1;
        entry.recencyTick = ++m_recencyCounter;
    }

    // Symbol → next-word bigram seeding
    if (semanticActive() && !m_lastSymbolWord.empty() && !tokens.empty()) {
        const std::string& firstWord = tokens.front();
        ++m_bigram[m_lastSymbolWord][firstWord];
        m_sessionBigramBoost[m_lastSymbolWord][firstWord] += 1.0f;
    }

    // Symbol → next-phrase reinforcement + per-user semantic phrase learning
    if (semanticActive() && !trimmed.isEmpty()) {
        std::string tagKey = m_currentSemanticTag.toLower().toStdString();
        auto& vec = m_symbolPhrases[tagKey];

        std::string phraseKey = trimmed.toLower().toStdString();
        bool exists = std::find(vec.begin(), vec.end(), phraseKey) != vec.end();
        if (!exists)
            vec.push_back(phraseKey);
    }

    // Reverse semantic mapping from utterance
    inferSemanticFromUtterance(text);
}

// -------------------------
// Stage 5: vocabulary boosting
// -------------------------

void AACPredictionEngine::boostToken(const QString& token)
{
    std::string w = token.trimmed().toLower().toStdString();
    if (w.empty())
        return;

    m_unigram[w] += 10;
    m_cachedTotalUnigrams = -1;
}

// -------------------------
// Stage 11: fuzzy matching
// -------------------------

int AACPredictionEngine::fuzzyDistance(const std::string& a,
                                       const std::string& b) const
{
    int mismatches = 0;
    size_t len = std::min(a.size(), b.size());
    for (size_t i = 0; i < len; ++i) {
        if (a[i] != b[i])
            ++mismatches;
    }
    mismatches += static_cast<int>(std::max(a.size(), b.size()) - len);
    return mismatches;
}

bool AACPredictionEngine::fuzzyCloseEnough(const std::string& typed,
                                           const std::string& candidate) const
{
    if (typed.empty())
        return false;
    int d = fuzzyDistance(typed, candidate);
    return d <= 2;
}

// -------------------------
// Semantic API
// -------------------------

void AACPredictionEngine::setSemanticContext(const QString& tag)
{
    if (tag.isEmpty())
        return;

    m_currentSemanticTag = tag.trimmed();
    m_semanticSetTick = m_recencyCounter;

    const QString lower = m_currentSemanticTag.toLower();

    if (lower.startsWith("emotion_")) {
        setCurrentCategory("emotion");
        setLastSymbolWord(lower.mid(QString("emotion_").length()));
    } else if (lower.startsWith("need_")) {
        setCurrentCategory("needs");
        setLastSymbolWord(lower.mid(QString("need_").length()));
    } else if (lower.startsWith("social_")) {
        setCurrentCategory("social");
        setLastSymbolWord(lower.mid(QString("social_").length()));
    } else if (lower == "yes" || lower == "no") {
        setCurrentCategory("response");
        setLastSymbolWord(lower);
    } else if (lower == "question") {
        setCurrentCategory("question");
        setLastSymbolWord("question");
    } else {
        // Fallback: treat tag itself as last symbol word
        setLastSymbolWord(lower);
    }
}

void AACPredictionEngine::registerSymbolPhrases(const QString& tag,
                                                const QStringList& phrases)
{
    if (tag.isEmpty() || phrases.isEmpty())
        return;

    std::string key = tag.trimmed().toLower().toStdString();
    auto& vec = m_symbolPhrases[key];
    vec.clear();
    vec.reserve(phrases.size());

    for (const QString& p : phrases) {
        QString trimmed = p.trimmed();
        if (!trimmed.isEmpty())
            vec.push_back(trimmed.toLower().toStdString());
    }
}

void AACPredictionEngine::inferSemanticFromUtterance(const QString& text)
{
    QString lower = text.trimmed().toLower();
    if (lower.isEmpty())
        return;

    for (const auto& kv : m_symbolPhrases) {
        const std::string& tag = kv.first;
        const auto& phrases = kv.second;

        for (const auto& phrase : phrases) {
            QString qPhrase = QString::fromStdString(phrase);
            if (lower.startsWith(qPhrase) || lower.contains(qPhrase)) {
                setSemanticContext(QString::fromStdString(tag));
                return;
            }
        }
    }
}

// Starter semantic phrase-bank
void AACPredictionEngine::loadDefaultSemanticPhrases()
{
    // Emotion
    registerSymbolPhrases("emotion_happy", {
        QObject::tr("I am happy"),
        QObject::tr("This is good"),
        QObject::tr("I like this"),
        QObject::tr("That makes me smile")
    });

    registerSymbolPhrases("emotion_sad", {
        QObject::tr("I feel sad"),
        QObject::tr("This is hard"),
        QObject::tr("I am upset"),
        QObject::tr("I want comfort")
    });

    registerSymbolPhrases("emotion_angry", {
        QObject::tr("I am angry"),
        QObject::tr("I do not like this"),
        QObject::tr("Please stop"),
        QObject::tr("This is not okay")
    });

    registerSymbolPhrases("emotion_scared", {
        QObject::tr("I am scared"),
        QObject::tr("I feel unsafe"),
        QObject::tr("Stay with me"),
        QObject::tr("Please help me feel safe")
    });

    // Needs
    registerSymbolPhrases("need_water", {
        QObject::tr("I am thirsty"),
        QObject::tr("I need a drink"),
        QObject::tr("Can I have some water?")
    });

    registerSymbolPhrases("need_food", {
        QObject::tr("I am hungry"),
        QObject::tr("I need food"),
        QObject::tr("Can we eat now?")
    });

    registerSymbolPhrases("need_rest", {
        QObject::tr("I am tired"),
        QObject::tr("I need to rest"),
        QObject::tr("Can I lie down?")
    });

    registerSymbolPhrases("need_help", {
        QObject::tr("I need help"),
        QObject::tr("Please help me"),
        QObject::tr("Something is wrong")
    });

    // Social / conversation
    registerSymbolPhrases("hello", {
        QObject::tr("Hello"),
        QObject::tr("Hi"),
        QObject::tr("Good to see you")
    });

    registerSymbolPhrases("please", {
        QObject::tr("Please"),
        QObject::tr("Could you please"),
        QObject::tr("I would like")
    });

    registerSymbolPhrases("sorry", {
        QObject::tr("I am sorry"),
        QObject::tr("I did not mean to"),
        QObject::tr("Please forgive me")
    });

    registerSymbolPhrases("thank_you", {
        QObject::tr("Thank you"),
        QObject::tr("Thanks a lot"),
        QObject::tr("I appreciate that")
    });

    registerSymbolPhrases("yes", {
        QObject::tr("Yes"),
        QObject::tr("That is okay"),
        QObject::tr("I agree")
    });

    registerSymbolPhrases("no", {
        QObject::tr("No"),
        QObject::tr("I do not want that"),
        QObject::tr("That is not okay")
    });

    registerSymbolPhrases("question", {
        QObject::tr("I have a question"),
        QObject::tr("Can I ask something?"),
        QObject::tr("I do not understand")
    });
}

// -------------------------
// Stage 7: phrase suggestions (recency + frequency + semantic)
// -------------------------

std::vector<std::string> AACPredictionEngine::phraseSuggestions(const std::string& prefix,
                                                                int maxSuggestions) const
{
    std::vector<std::string> out;
    if (maxSuggestions <= 0)
        return out;

    const std::string lowerPrefix = QString::fromStdString(prefix)
                                        .trimmed()
                                        .toLower()
                                        .toStdString();

    if (lowerPrefix.empty() && !semanticActive())
        return out;

    struct Scored {
        std::string phrase;
        int score;
        int recency;
    };

    std::vector<Scored> candidates;
    candidates.reserve(m_phrases.size());

    // Phrase memory
    for (const auto& kv : m_phrases) {
        const auto& entry = kv.second;
        if (!lowerPrefix.empty()) {
            if (entry.phrase.rfind(lowerPrefix, 0) != 0)
                continue;
        }
        int score = entry.count * 10 + entry.recencyTick;
        candidates.push_back({ entry.phrase, score, entry.recencyTick });
    }

    // Symbol phrase bank (semantic)
    if (semanticActive()) {
        auto it = m_symbolPhrases.find(m_currentSemanticTag.toLower().toStdString());
        if (it != m_symbolPhrases.end()) {
            for (const auto& phrase : it->second) {
                if (!lowerPrefix.empty()) {
                    if (phrase.rfind(lowerPrefix, 0) != 0)
                        continue;
                }
                int base = 1000;
                candidates.push_back({ phrase, base, m_recencyCounter });
            }
        }
    }

    std::sort(candidates.begin(), candidates.end(),
              [](const Scored& a, const Scored& b) {
                  return a.score > b.score;
              });

    for (const auto& c : candidates) {
        out.push_back(c.phrase);
        if ((int)out.size() >= maxSuggestions)
            break;
    }

    return out;
}

// -------------------------
// Stage 17: build context
// -------------------------

AACPredictionEngine::Context
AACPredictionEngine::buildContext(const QStringList& parts) const
{
    Context ctx;

    if (parts.isEmpty()) {
        ctx.sentenceStart = true;
        return ctx;
    }

    QString last = parts.last();
    if (last == "." || last == "!" || last == "?") {
        ctx.sentenceStart = true;
    } else {
        ctx.lastWord = last.toLower().toStdString();
    }

    if (parts.size() >= 2) {
        QString prev = parts[parts.size() - 2];
        ctx.prevWord = prev.toLower().toStdString();
        if (prev == ",")
            ctx.afterComma = true;
        if (prev == "?")
            ctx.afterQuestion = true;
    }

    return ctx;
}

// -------------------------
// Stage 12+17+18+14: scoring helper
// -------------------------

float AACPredictionEngine::scoreCandidate(const Context& ctx,
                                          const std::string& candidate) const
{
    float score = 0.0f;

    // Unigram weight
    auto itU = m_unigram.find(candidate);
    if (itU != m_unigram.end())
        score += itU->second * 1.0f;

    // Bigram weight (lastWord -> candidate)
    if (!ctx.lastWord.empty()) {
        auto itB = m_bigram.find(ctx.lastWord);
        if (itB != m_bigram.end()) {
            auto it2 = itB->second.find(candidate);
            if (it2 != itB->second.end())
                score += it2->second * 2.0f;
        }
    }

    // Trigram weight (prevWord, lastWord -> candidate)
    if (!ctx.prevWord.empty() && !ctx.lastWord.empty()) {
        auto it1 = m_trigram.find(ctx.prevWord);
        if (it1 != m_trigram.end()) {
            auto it2 = it1->second.find(ctx.lastWord);
            if (it2 != it1->second.end()) {
                auto it3 = it2->second.find(candidate);
                if (it3 != it2->second.end())
                    score += it3->second * 3.0f;
            }
        }
    }

    // Session boosts (Stage 15)
    auto itSB = m_sessionBoost.find(candidate);
    if (itSB != m_sessionBoost.end())
        score += itSB->second;

    if (!ctx.lastWord.empty()) {
        auto itSBB = m_sessionBigramBoost.find(ctx.lastWord);
        if (itSBB != m_sessionBigramBoost.end()) {
            auto it2 = itSBB->second.find(candidate);
            if (it2 != itSBB->second.end())
                score += it2->second;
        }
    }

    // Stage 18: AAC category weighting
    if (!m_currentCategory.isEmpty()) {
        auto itP = m_phrases.find(m_currentCategory.toLower().toStdString());
        if (itP != m_phrases.end()) {
            score += 0.5f;
        }
    }

    // Stage 18: last symbol word weighting
    if (!m_lastSymbolWord.empty() && candidate == m_lastSymbolWord) {
        score += 2.0f;
    }

    // Semantic weighting (symbol + category blending)
    score += semanticWeightFor(candidate);

    // Punctuation context: after question, boost "yes/no"
    if (ctx.afterQuestion) {
        if (candidate == "yes" || candidate == "no")
            score += 1.5f;
    }

    // Stage 14: negative reinforcement penalty
    auto negIt = m_negativeCount.find(candidate);
    if (negIt != m_negativeCount.end()) {
        float penalty = m_penaltyCfg.globalPenaltyFactor * negIt->second;
        score -= penalty;
    }

    return score;
}

// -------------------------
// Stage 2,3,7,8,9,10,11,12,17,18,19: prediction
// -------------------------

std::vector<std::string> AACPredictionEngine::Predict(const std::string& prefix,
                                                      int maxSuggestions) const
{
    std::vector<std::string> result;
    if (maxSuggestions <= 0)
        return result;

    // Stage 19: freeze
    if (m_predictionsFrozen && !m_lastStable.empty())
        return m_lastStable;

    // Semantic decay
    decaySemanticIfNeeded();

    // Stage 7: phrase-level suggestions (includes symbol phrases)
    auto phraseRes = phraseSuggestions(prefix, maxSuggestions);
    for (auto& p : phraseRes)
        result.push_back(p);
    if ((int)result.size() >= maxSuggestions) {
        if (!result.empty())
            m_lastTopPrediction = result.front();
        m_lastStable = result;
        return result;
    }

    if (prefix.empty()) {
        // No context: top unigrams
        std::vector<std::pair<std::string,int>> uni(m_unigram.begin(), m_unigram.end());
        std::sort(uni.begin(), uni.end(),
                  [](auto& a, auto& b){ return a.second > b.second; });

        for (auto& u : uni) {
            result.push_back(u.first);
            if ((int)result.size() >= maxSuggestions)
                break;
        }
        if (!result.empty())
            m_lastTopPrediction = result.front();
        m_lastStable = result;
        return result;
    }

    // Punctuation-aware context
    QString qPrefix = QString::fromStdString(prefix);
    QString cleaned = normalizePunctuation(qPrefix);

    QStringList parts = cleaned.split(QRegularExpression("\\s+"),
                                      Qt::SkipEmptyParts);

    Context ctx = buildContext(parts);

    // Stage 8: Trigram context
    if (!ctx.prevWord.empty() && !ctx.lastWord.empty()) {
        auto it1 = m_trigram.find(ctx.prevWord);
        if (it1 != m_trigram.end()) {
            auto it2 = it1->second.find(ctx.lastWord);
            if (it2 != it1->second.end()) {
                std::vector<std::pair<std::string,int>> triCandidates(
                    it2->second.begin(), it2->second.end());

                std::sort(triCandidates.begin(), triCandidates.end(),
                          [](auto& a, auto& b){ return a.second > b.second; });

                for (auto& c : triCandidates) {
                    if (std::find(result.begin(), result.end(), c.first) == result.end()) {
                        result.push_back(c.first);
                        if ((int)result.size() >= maxSuggestions)
                            break;
                    }
                }
            }
        }

        if ((int)result.size() >= maxSuggestions) {
            if (!result.empty())
                m_lastTopPrediction = result.front();
            m_lastStable = result;
            return result;
        }
    }

    // 1) Bigram context
    if (!ctx.lastWord.empty()) {
        auto itBig = m_bigram.find(ctx.lastWord);
        if (itBig != m_bigram.end()) {
            std::vector<std::pair<std::string,int>> candidates(
                itBig->second.begin(), itBig->second.end());

            std::sort(candidates.begin(), candidates.end(),
                      [](auto& a, auto& b){ return a.second > b.second; });

            for (auto& c : candidates) {
                if (std::find(result.begin(), result.end(), c.first) == result.end()) {
                    result.push_back(c.first);
                    if ((int)result.size() >= maxSuggestions)
                        break;
                }
            }
        }
    }

    // 2) Personal dictionary + fuzzy matching
    if (!ctx.lastWord.empty() && (int)result.size() < maxSuggestions) {
        for (const auto& w : m_customWords) {
            if (fuzzyCloseEnough(ctx.lastWord, w)) {
                if (std::find(result.begin(), result.end(), w) == result.end()) {
                    result.push_back(w);
                    if ((int)result.size() >= maxSuggestions)
                        break;
                }
            }
        }
    }

    // 3) Fill with top unigrams (scored)
    if ((int)result.size() < maxSuggestions) {
        std::vector<std::pair<std::string,int>> uni(m_unigram.begin(), m_unigram.end());
        std::sort(uni.begin(), uni.end(),
                  [this, &ctx](auto& a, auto& b){
                      float sa = scoreCandidate(ctx, a.first);
                      float sb = scoreCandidate(ctx, b.first);
                      if (sa == sb)
                          return a.second > b.second;
                      return sa > sb;
                  });

        for (auto& u : uni) {
            if (std::find(result.begin(), result.end(), u.first) == result.end()) {
                result.push_back(u.first);
                if ((int)result.size() >= maxSuggestions)
                    break;
            }
        }
    }

    // Stage 9: Capitalize at sentence start
    if (ctx.sentenceStart) {
        for (auto& s : result) {
            if (!s.empty())
                s[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(s[0])));
        }
    }

    // Stage 19: stability layer
    if (!m_lastStable.empty()) {
        int overlap = 0;
        for (const auto& s : result) {
            if (std::find(m_lastStable.begin(), m_lastStable.end(), s) != m_lastStable.end())
                ++overlap;
        }
        float overlapRatio = 0.0f;
        int denom = std::max((int)result.size(), (int)m_lastStable.size());
        if (denom > 0)
            overlapRatio = static_cast<float>(overlap) / static_cast<float>(denom);

        if (overlapRatio >= m_stabilityThreshold) {
            return m_lastStable;
        }
    }

    if (!result.empty())
        m_lastTopPrediction = result.front();
    m_lastStable = result;
    return result;
}

// -------------------------
// Stage 12: confidence API
// -------------------------

float AACPredictionEngine::confidenceFor(const std::string& token) const
{
    if (token.empty())
        return 0.0f;

    auto it = m_unigram.find(token);
    if (it == m_unigram.end())
        return 0.0f;

    if (m_cachedTotalUnigrams < 0) {
        int total = 0;
        for (const auto& kv : m_unigram)
            total += kv.second;
        m_cachedTotalUnigrams = std::max(total, 1);
    }

    float freq = static_cast<float>(it->second) /
                 static_cast<float>(m_cachedTotalUnigrams);

    if (freq > 0.2f) freq = 0.2f;
    return freq / 0.2f;
}

// -------------------------
// Stage 13: positive reinforcement
// -------------------------

void AACPredictionEngine::reinforceChoice(const std::string& prev,
                                          const std::string& chosen)
{
    if (chosen.empty())
        return;

    m_unigram[chosen] += 3;
    m_cachedTotalUnigrams = -1;

    if (!prev.empty())
        m_bigram[prev][chosen] += 5;

    m_sessionBoost[chosen] += 1.0f;
    if (!prev.empty())
        m_sessionBigramBoost[prev][chosen] += 1.5f;

    ++m_recencyCounter;
}

// -------------------------
// Stage 14: negative reinforcement
// -------------------------

void AACPredictionEngine::applyPenalty(const std::string& token, float basePenalty)
{
    int inc = static_cast<int>(std::ceil(basePenalty));
    m_negativeCount[token] += inc;

    if (m_negativeCount[token] > 50)
        m_negativeCount[token] = 50;
}

void AACPredictionEngine::penalizeIgnored(const std::string& prev,
                                          const std::vector<std::string>& shown,
                                          const std::string& actualTyped)
{
    Q_UNUSED(prev);

    if (actualTyped.empty())
        return;

    bool actualWasShown =
        std::find(shown.begin(), shown.end(), actualTyped) != shown.end();

    if (!actualWasShown) {
        for (const auto& s : shown)
            applyPenalty(s, 1.0f);
    }
}

void AACPredictionEngine::onUserSelected(const std::string& prev,
                                         const std::string& chosen,
                                         const std::vector<std::string>& shown)
{
    bool wasShown =
        std::find(shown.begin(), shown.end(), chosen) != shown.end();

    if (wasShown) {
        m_positiveCount[chosen] += 1;

        m_sessionBoost[chosen] += 0.3f;
        if (!prev.empty())
            m_sessionBigramBoost[prev][chosen] += 0.3f;

        auto it = m_negativeCount.find(chosen);
        if (it != m_negativeCount.end() && it->second > 0)
            it->second -= 1;
    }
}

void AACPredictionEngine::onUserDeletedAutocompleted(const std::string& token)
{
    applyPenalty(token, 2.0f);
}

void AACPredictionEngine::onPredictionBarShown()
{
    m_predictionBarVisible = true;
    m_predictionBarShown = std::chrono::steady_clock::now();
}

void AACPredictionEngine::tick()
{
    if (!m_predictionBarVisible)
        return;

    auto now = std::chrono::steady_clock::now();
    auto elapsedMs =
        std::chrono::duration_cast<std::chrono::milliseconds>(
            now - m_predictionBarShown).count();

    if (elapsedMs >= m_ignoreThresholdMs) {

        if (!m_lastTopPrediction.empty())
            applyPenalty(m_lastTopPrediction, 1.0f);

        for (auto& kv : m_negativeCount) {
            kv.second = static_cast<int>(
                std::floor(kv.second * m_penaltyCfg.decayFactor));
        }

        m_predictionBarVisible = false;
    }
}

// -------------------------
// Stage 20: dwell reinforcement
// -------------------------

void AACPredictionEngine::reinforceDwellChoice(const std::string& prev,
                                               const std::string& chosen)
{
    m_positiveCount[chosen] += 2;
    m_sessionBoost[chosen]  += 0.6f;

    if (!prev.empty())
        m_sessionBigramBoost[prev][chosen] += 0.6f;

    auto it = m_negativeCount.find(chosen);
    if (it != m_negativeCount.end() && it->second > 0) {
        it->second -= 2;
        if (it->second < 0)
            it->second = 0;
    }
}

// -------------------------
// Stage 18: AAC context hooks
// -------------------------

void AACPredictionEngine::setCurrentCategory(const QString& category)
{
    m_currentCategory = category;
}

void AACPredictionEngine::setLastSymbolWord(const QString& word)
{
    m_lastSymbolWord = word.trimmed().toLower().toStdString();
}

// -------------------------
// Stage 19: freeze control
// -------------------------

void AACPredictionEngine::freezePredictions()
{
    m_predictionsFrozen = true;
}

void AACPredictionEngine::unfreezePredictions()
{
    m_predictionsFrozen = false;
}

// -------------------------
// Stage 4: persistence
// -------------------------

bool AACPredictionEngine::saveToFile(const QString& path) const
{
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;

    QTextStream out(&f);

    // Unigrams
    out << "UNIGRAM\n";
    for (const auto& kv : m_unigram)
        out << QString::fromStdString(kv.first) << " " << kv.second << "\n";

    // Bigrams
    out << "BIGRAM\n";
    for (const auto& pkv : m_bigram) {
        QString prev = QString::fromStdString(pkv.first);
        for (const auto& nkv : pkv.second) {
            QString next = QString::fromStdString(nkv.first);
            out << prev << " " << next << " " << nkv.second << "\n";
        }
    }

    // Trigrams
    out << "TRIGRAM\n";
    for (const auto& w1kv : m_trigram) {
        QString w1 = QString::fromStdString(w1kv.first);
        for (const auto& w2kv : w1kv.second) {
            QString w2 = QString::fromStdString(w2kv.first);
            for (const auto& w3kv : w2kv.second) {
                QString w3 = QString::fromStdString(w3kv.first);
                out << w1 << " " << w2 << " " << w3 << " " << w3kv.second << "\n";
            }
        }
    }

    // Phrases
    out << "PHRASE\n";
    for (const auto& kv : m_phrases) {
        const auto& e = kv.second;
        out << QString::fromStdString(e.phrase)
            << " " << e.count
            << " " << e.recencyTick << "\n";
    }

    // Personal dictionary
    out << "CUSTOM\n";
    for (const auto& w : m_customWords) {
        int seen = 0;
        auto it = m_wordSeenCount.find(w);
        if (it != m_wordSeenCount.end())
            seen = it->second;
        out << QString::fromStdString(w) << " " << seen << "\n";
    }

    // Semantic symbol phrases (per-user)
    out << "SYMBOL\n";
    for (const auto& kv : m_symbolPhrases) {
        QString tag = QString::fromStdString(kv.first);
        for (const auto& phrase : kv.second) {
            QString qPhrase = QString::fromStdString(phrase);
            // Use '|' as separator to allow spaces in phrase
            out << tag << "|" << qPhrase << "\n";
        }
    }

    return true;
}

bool AACPredictionEngine::loadFromFile(const QString& path)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    m_unigram.clear();
    m_bigram.clear();
    m_trigram.clear();
    m_phrases.clear();
    m_customWords.clear();
    m_wordSeenCount.clear();
    m_recencyCounter = 0;
    m_cachedTotalUnigrams = -1;
    m_sessionBoost.clear();
    m_sessionBigramBoost.clear();
    m_lastStable.clear();
    m_predictionsFrozen = false;
    m_currentCategory.clear();
    m_lastSymbolWord.clear();
    m_negativeCount.clear();
    m_positiveCount.clear();
    m_lastTopPrediction.clear();
    m_currentSemanticTag.clear();
    m_semanticSetTick = 0;
    m_symbolPhrases.clear();

    QTextStream in(&f);
    QString section;

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty())
            continue;

        if (line == "UNIGRAM" || line == "BIGRAM" || line == "TRIGRAM" ||
            line == "PHRASE" || line == "CUSTOM" || line == "SYMBOL") {
            section = line;
            continue;
        }

        if (section == "SYMBOL") {
            int idx = line.indexOf('|');
            if (idx <= 0)
                continue;
            QString tag = line.left(idx).trimmed().toLower();
            QString phrase = line.mid(idx + 1).trimmed().toLower();
            if (tag.isEmpty() || phrase.isEmpty())
                continue;

            std::string key = tag.toStdString();
            std::string p   = phrase.toStdString();
            auto& vec = m_symbolPhrases[key];
            if (std::find(vec.begin(), vec.end(), p) == vec.end())
                vec.push_back(p);
            continue;
        }

        QStringList parts = line.split(' ', Qt::SkipEmptyParts);

        if (section == "UNIGRAM") {
            if (parts.size() != 2)
                continue;
            std::string word = parts[0].toLower().toStdString();
            int count = parts[1].toInt();
            m_unigram[word] = count;
        } else if (section == "BIGRAM") {
            if (parts.size() != 3)
                continue;
            std::string prev = parts[0].toLower().toStdString();
            std::string next = parts[1].toLower().toStdString();
            int count = parts[2].toInt();
            m_bigram[prev][next] = count;
        } else if (section == "TRIGRAM") {
            if (parts.size() != 4)
                continue;
            std::string w1 = parts[0].toLower().toStdString();
            std::string w2 = parts[1].toLower().toStdString();
            std::string w3 = parts[2].toLower().toStdString();
            int count = parts[3].toInt();
            m_trigram[w1][w2][w3] = count;
        } else if (section == "PHRASE") {
            if (parts.size() < 3)
                continue;
            bool okCount = false;
            bool okRec = false;
            int count = parts[parts.size() - 2].toInt(&okCount);
            int rec = parts[parts.size() - 1].toInt(&okRec);
            if (!okCount || !okRec)
                continue;

            QString phrase;
            for (int i = 0; i < parts.size() - 2; ++i) {
                if (i > 0) phrase += " ";
                phrase += parts[i];
            }

            std::string key = phrase.toLower().toStdString();
            PhraseEntry e;
            e.phrase = key;
            e.count = count;
            e.recencyTick = rec;
            m_phrases[key] = e;
            m_recencyCounter = std::max(m_recencyCounter, rec);
        } else if (section == "CUSTOM") {
            if (parts.size() != 2)
                continue;
            std::string word = parts[0].toLower().toStdString();
            int seen = parts[1].toInt();
            m_wordSeenCount[word] = seen;
            if (seen >= 2)
                m_customWords.insert(word);
        }
    }

    return true;
}
