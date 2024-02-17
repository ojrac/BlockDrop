#pragma once
#ifndef BLOCKDROP_SCORE_BOARD_H
#define BLOCKDROP_SCORE_BOARD_H

#include <vector>
#include <tuple>
#include <memory>
#include <string>

namespace BlockDrop
{

using Score = int;
using Name = std::string;
using ScoreList = std::vector<std::tuple<Name, Score>>;
class ScoreBoard
{
public:
    ScoreBoard();
    const ScoreList& GetScoreList();
    void SetScore(const std::string& name, const int score);
    int GetLowestScore();
private:
        static constexpr char s_ScoreFile[] = "scores.tsv";
        std::unique_ptr<ScoreList> m_ScoreList{};

        static constexpr char s_DefaultScores[] = "AAA\t100\nBBB\t200\nCCC\t300\nDDD\t400\nEEE\t500\nFFF\t600\nGGG\t700\nHHH\t800\nIII\t900\nJJJ\t1000\n";

private:
    void SortScores();
};
}

#endif