#include "ScoreBoard.h"
#include <fstream>

// Definition of the static data member (one copy shared by all objects).
int ScoreBoard::gamesPlayed = 0;

ScoreBoard::ScoreBoard()
    : currentScore(0), bestScore(0), fileName("highscore.txt")
{
}

ScoreBoard::ScoreBoard(const std::string& file)
    : currentScore(0), bestScore(0), fileName(file)
{
}

void ScoreBoard::startNewGame()
{
    currentScore = 0;
    gamesPlayed++;
}

void ScoreBoard::addPoints(int points)
{
    if (points > 0) currentScore += points;
}

// Overload: points are multiplied (e.g. 100 points x round number).
void ScoreBoard::addPoints(int points, int multiplier)
{
    addPoints(points * multiplier);
}

ScoreBoard& ScoreBoard::operator+=(int points)
{
    addPoints(points);
    return *this;
}

void ScoreBoard::load()
{
    std::ifstream in(fileName.c_str());
    if (!in)
        throw ScoreFileException("No saved high score found (" + fileName + ")");

    int value = 0;
    if (!(in >> value) || value < 0)
        throw ScoreFileException("High score file is corrupted (" + fileName + ")");

    bestScore = value;
}

void ScoreBoard::save() const
{
    std::ofstream out(fileName.c_str());
    if (!out)
        throw ScoreFileException("Could not open " + fileName + " for writing");

    out << bestScore << std::endl;
    if (!out)
        throw ScoreFileException("Error while writing " + fileName);
}

bool ScoreBoard::updateBest()
{
    if (currentScore > bestScore)
    {
        bestScore = currentScore;
        return true;
    }
    return false;
}

int ScoreBoard::getCurrentScore() const { return currentScore; }
int ScoreBoard::getBestScore() const { return bestScore; }
int ScoreBoard::getGamesPlayed() { return gamesPlayed; }

std::ostream& operator<<(std::ostream& out, const ScoreBoard& board)
{
    out << "Score: " << board.currentScore
        << "  Best: " << board.bestScore
        << "  Games played: " << ScoreBoard::gamesPlayed;
    return out;
}