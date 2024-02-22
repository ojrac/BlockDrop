#include "ScoreBoard.h"

#include <algorithm>
#include <fstream>
#include <sstream>

namespace BlockDrop {

ScoreList FileBackedScoreBoard::ParseScoresFromDisk()
{
    ScoreList result{};

    std::ifstream scoreFile(s_ScoreFile);
    std::stringstream scores;
    if (!scoreFile) {
        scores << s_DefaultScores;
    }
    else {
        scores << scoreFile.rdbuf();
    }

    std::string line;
    while (std::getline(scores, line, '\n')) {
        size_t firstSeparatorPosition = line.find('\t');
        if (firstSeparatorPosition == std::string::npos) {
            // Error: Missing second score column.
            break;
        }
        std::string name = line.substr(0, firstSeparatorPosition);
        line.erase(0, firstSeparatorPosition + 1);
        size_t secondSeparatorPosition = line.find('\t');
        if (secondSeparatorPosition == std::string::npos) {
            // Error: Missing third score column.
            break;
        }
        int score = stoi(line.substr(0, secondSeparatorPosition));
        line.erase(0, secondSeparatorPosition + 1);
        int level = stoi(line);
        result.push_back(std::tuple(name, score, level));
    }

    return result;
}

FileBackedScoreBoard::FileBackedScoreBoard()
    : ScoreBoard(ParseScoresFromDisk())
{
}
void FileBackedScoreBoard::SetScore(const std::string& name, const int score, const int level)
{
    ScoreBoard::SetScore(name, score, level);
    SaveScores();
}

void FileBackedScoreBoard::SaveScores() {
    std::ofstream scoreFile(s_ScoreFile);
    auto const& scores = GetScoreList();
    if (scoreFile.is_open()) {
        for (int i = 0; i < scores.size(); i++) {
            scoreFile << std::get<0>(scores.at(i)) << '\t'
                << std::get<1>(scores.at(i)) << '\t'
                << std::get<2>(scores.at(i)) << '\n';
        }
        scoreFile.close();
    }
}

ScoreBoard::ScoreBoard(ScoreList const& scores) {
    m_ScoreList = scores;
    SortScores();
}

void ScoreBoard::SetScore(const std::string& name, const int score, const int level) {
    m_ScoreList.push_back(std::tuple(name, score, level));
    SortScores();
    m_ScoreList.pop_back();
}

void ScoreBoard::Rename(const int index, const std::string& newName)
{
    if (index <= m_ScoreList.size())
    {
        return;
    }

    std::get<0>(m_ScoreList[index]) = newName;
}


void ScoreBoard::SortScores() {
    std::sort(m_ScoreList.begin(), m_ScoreList.end(), [](auto const& first, auto const& second) {
        return std::get<1>(first) > std::get<1>(second);
    });
}

const ScoreList& ScoreBoard::GetScoreList() {
    return m_ScoreList;
}

int ScoreBoard::GetLowestScore() {
    return std::get<1>(m_ScoreList.back());
}

}
