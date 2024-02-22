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
using Level = int;
using Name = std::string;
using ScoreList = std::vector<std::tuple<Name, Score, Level>>;

class ScoreBoard
{
public:
    ScoreBoard() {}
    ScoreBoard(ScoreList const& scores);
    const ScoreList& GetScoreList();
    virtual void SetScore(const std::string& name, const int score, const int level);
    void Rename(const int index, const std::string& newName);
    int GetLowestScore();
private:
    ScoreList m_ScoreList{};

private:
    void SortScores();
};

class FileBackedScoreBoard : public ScoreBoard
{
public:
    FileBackedScoreBoard();
    virtual void SetScore(const std::string& name, const int score, const int level) override;

private:
    static ScoreList ParseScoresFromDisk();
    static constexpr char s_ScoreFile[] = "scores.tsv";
    static constexpr char s_DefaultScores[] = "AAA\t100\t1\nBBB\t200\t2\nCCC\t300\t3\nDDD\t400\t4\nEEE\t500\t5\nFFF\t600\t6\nGGG\t700\t7\nHHH\t800\t8\nIII\t900\t9\nJJJ\t1000\t10";

private:
    void SaveScores();
};

}

#endif
