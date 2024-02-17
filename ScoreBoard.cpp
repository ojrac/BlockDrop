#include "ScoreBoard.h"

#include <algorithm>
#include <fstream>
#include <sstream>

namespace BlockDrop {
ScoreBoard::ScoreBoard() {
    std::ifstream scoreFile(s_ScoreFile);
    std::stringstream scores;
    if (!scoreFile) {
        scores << s_DefaultScores;
    } else {
        scores << scoreFile.rdbuf();
    }

    std::string line;
    while (std::getline(scores, line, '\n')) {
        size_t firstSeparatorPosition = line.find('\t');
        if (firstSeparatorPosition == std::string::npos) {
            // Error: Missing second score column.
            SortScores();
            return;
        }
        std::string name = line.substr(0, firstSeparatorPosition);
        line.erase(0, firstSeparatorPosition+1);
        size_t secondSeparatorPosition = line.find('\t');
        if (secondSeparatorPosition == std::string::npos) {
            // Error: Missing third score column.
            SortScores();
            return;
        }
        int score = stoi(line.substr(0, secondSeparatorPosition));
        line.erase(0, secondSeparatorPosition+1);
        int level = stoi(line);
        m_ScoreList->push_back(std::tuple(name, score, level));
    }
    SortScores();
}

void ScoreBoard::SaveScores() {
    std::ofstream scoreFile(s_ScoreFile);
    if (scoreFile.is_open()) {
        for (int i = 0; i < m_ScoreList->size(); i++) {
            scoreFile << std::get<0>(m_ScoreList->at(i)) << '\t' 
            << std::get<1>(m_ScoreList->at(i))  << '\t' 
            << std::get<2>(m_ScoreList->at(i)) << '\n';
        }
        scoreFile.close()
    }
}

void ScoreBoard::SetScore(const std::string& name, const int score, const int level) {
    m_ScoreList->push_back(std::tuple(name, score, level));
    SortScores();
    m_ScoreList->pop_back();
    SaveScores()
}

namespace {
    bool sortByScore(const std::tuple<std::string, int>& first, const std::tuple<std::string, int>& second) {
        return std::get<1>(first) < std::get<1>(second);
    }
}

void ScoreBoard::SortScores() {
    std::sort(m_ScoreList->begin(), m_ScoreList->end(), sortByScore);
}

const ScoreList& ScoreBoard::GetScoreList() {
    return *m_ScoreList;
}

int ScoreBoard::GetLowestScore() {
    return std::get<1>(m_ScoreList->back());
}

}
