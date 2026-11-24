// ScoreBoard.h - keeps the current score and the saved high score.
// Demonstrates: file handling, exception handling, static member, friend function,
// operator overloading, function overloading and constructor overloading.
#ifndef SCOREBOARD_H
#define SCOREBOARD_H

#include <iostream>
#include <stdexcept>
#include <string>

// Custom exception (inheritance): thrown when the score file cannot be read or written.
class ScoreFileException : public std::runtime_error
{
public:
    explicit ScoreFileException(const std::string& message)
        : std::runtime_error(message) {}
};

class ScoreBoard
{
private:
    int currentScore;
    int bestScore;
    std::string fileName;

    // Static data member: shared by the whole class, counts the games started.
    static int gamesPlayed;

public:
    // Constructor overloading: default file name, or a file name chosen by the caller.
    ScoreBoard();
    explicit ScoreBoard(const std::string& file);

    void startNewGame();                          // resets the score, counts the game

    // Function overloading: same name, different parameters.
    void addPoints(int points);
    void addPoints(int points, int multiplier);

    // Operator overloading: score += 10;
    ScoreBoard& operator+=(int points);

    // File handling + exception handling: both functions throw ScoreFileException.
    void load();
    void save() const;
    bool updateBest();                            // true if a new record was set

    int getCurrentScore() const;
    int getBestScore() const;

    // Static member function: can be called without an object.
    static int getGamesPlayed();

    // Friend function: may read the private members to print the whole scoreboard.
    friend std::ostream& operator<<(std::ostream& out, const ScoreBoard& board);
};

#endif