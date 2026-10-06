#pragma once

#include <cstdint>
#include <cstddef>
#include <string>
#include <iostream>
#include <vector>
#include <cstring>
#include <array>
#include "MurmurHash64.hpp"

constexpr size_t maxSteps = 60;
constexpr size_t rows = 8;
constexpr size_t cols = 6;

struct Position {
    union {
        struct {
            uint8_t row : 4;
            uint8_t col : 4;
        };
        uint8_t both;
    };

    constexpr Position() : row(0), col(0) {

    }

    constexpr Position(uint8_t row, uint8_t col) : row(row), col(col) {

    }

    bool operator !=(const Position &other) const {
        return both != other.both;
    }
};

static constexpr Position POSITION_NONE(15, 15);

struct Field {
    static constexpr bool isStaticArrow(char m) {
        return m == 'L' || m == 'R' || m == 'U' || m == 'D';
    }

    static constexpr bool isRotatingArrow(char m) {
        return m == 'w' || m == 'x' || m == 'a' || m == 's';
    }

    static bool isClickable(char m) {
        static constexpr std::array<bool, 256> table = [] {
            std::array<bool, 256> table = {};
            for (size_t i = 0; i < table.size(); i++) {
                char m = static_cast<char>(i);
                table[i] = isStaticArrow(m) || isRotatingArrow(m) || m == 'F' || m == 'B';
            }
            return table;
        }();
        return table[static_cast<unsigned char>(m)];
    }

    static bool isCorrect(char color, char modifier) {
        if (!isColor(color)) {
            return true;
        } else if (isColor(modifier)) {
            return modifier == color;
        } else {
            return modifier != '0';
        }
    }

    static bool isColor(char c) {
        return c == 'r' || c == 'g' || c == 'b' || c == 'o' || c == 'd';
    }

    static size_t colorMPHF(char c) {
        return c % 6;
    }
};

struct MoveSequence {
    Position moves[maxSteps];
    size_t n = 0;

    [[nodiscard]] std::string toString() const {
        std::string sequence;
        for (size_t i = 0; i < n; i++) {
            sequence += ('A' + moves[i].col);
            sequence += std::to_string(moves[i].row + 1);
            if (i != n - 1) {
                sequence += ",";
            }
        }
        return sequence;
    }
};

struct Level {
    char colors[rows][cols];
    char initialModifiers[rows][cols];
    Position onlyReachableFrom[rows][cols];
    std::vector<Position> clickables;
    bool hasBombs = false;

    Level() {
        for (size_t row = 0; row < rows; row++) {
            for (size_t col = 0; col < cols; col++) {
                colors[row][col] = 'g';
                initialModifiers[row][col] = '0';
                onlyReachableFrom[row][col] = POSITION_NONE;
            }
        }
    }

    using ReachabilityArray = std::vector<Position>[rows][cols];

    static void fillReachability(int dr, int rc, const size_t row, const size_t col, char color, const Level &level,
                                 ReachabilityArray &reachableFrom) {
        size_t r = row + dr;
        size_t c = col + rc;
        while (r < rows && c < cols) {
            if (level.colors[r][c] == color) {
                reachableFrom[r][c].emplace_back(row, col);
            }
            r += dr;
            c += rc;
        }
    }

    static Level from(std::string color, std::string modifier) {
        Level level;
        bool smallBoard = color.length() == 5 * 6;
        for (size_t row = 0; row < rows; row++) {
            for (size_t col = 0; col < cols; col++) {
                if (smallBoard && (row >= 6 || col >= 5)) {
                    level.colors[row][col] = '0';
                    level.initialModifiers[row][col] = 'X';
                    continue;
                }
                level.colors[row][col] = color[row * (smallBoard ? 5 : 6) + col];
                level.initialModifiers[row][col] = modifier[row * (smallBoard ? 5 : 6) + col];
            }
        }

        // Fill reachability
        ReachabilityArray reachableFrom;
        for (size_t row = 0; row < rows; row++) {
            for (size_t col = 0; col < cols; col++) {
                char fieldModifier = level.initialModifiers[row][col];
                char fieldColor = level.colors[row][col];
                if (!Field::isClickable(fieldModifier)) {
                    continue;
                }

                if (fieldModifier == 'U') {
                    fillReachability(-1, 0, row, col, fieldColor, level, reachableFrom);
                } else if (fieldModifier == 'D') {
                    fillReachability(1, 0, row, col, fieldColor, level, reachableFrom);
                } else if (fieldModifier == 'L') {
                    fillReachability(0, -1, row, col, fieldColor, level, reachableFrom);
                } else if (fieldModifier == 'R') {
                    fillReachability(0, 1, row, col, fieldColor, level, reachableFrom);
                } else if (fieldModifier == 'F') {
                    for (size_t r = 0; r < rows; r++) {
                        for (size_t c = 0; c < cols; c++) {
                            if (level.colors[r][c] == fieldColor) {
                                reachableFrom[r][c].emplace_back(row, col);
                            }
                        }
                    }
                } else if (fieldModifier == 'B') {
                    level.hasBombs = true;
                    for (size_t dr = 0; dr < 3; dr++) {
                        for (size_t dc = 0; dc < 3; dc++) {
                            if (row - 1 + dr < rows && col - 1 + dc < cols) {
                                if (level.colors[row - 1 + dr][col - 1 + dc] == fieldColor) {
                                    reachableFrom[row - 1 + dr][col - 1 + dc].emplace_back(row, col);
                                }
                            }
                        }
                    }
                } else if (fieldModifier == 'w' || fieldModifier == 's'
                           || fieldModifier == 'a' || fieldModifier == 'x') {
                    fillReachability(-1, 0, row, col, fieldColor, level, reachableFrom);
                    fillReachability(1, 0, row, col, fieldColor, level, reachableFrom);
                    fillReachability(0, -1, row, col, fieldColor, level, reachableFrom);
                    fillReachability(0, 1, row, col, fieldColor, level, reachableFrom);
                } else {
                    std::cout << "Unknown modifier" << std::endl;
                }
            }
        }
        for (size_t row = 0; row < rows; row++) {
            for (size_t col = 0; col < cols; col++) {
                if (reachableFrom[row][col].size() == 1) {
                    level.onlyReachableFrom[row][col] = reachableFrom[row][col].front();
                }
                if (Field::isClickable(level.initialModifiers[row][col])) {
                    level.clickables.emplace_back(row, col);
                }
            }
        }
        return level;
    }
};

struct Board {
    char modifiers[rows][cols] = {};
    MoveSequence moveSequence;
    const Level *level = nullptr; // Must outlive the board

    Board() = default;

    explicit Board(const Level &level) : level(&level) {
        std::memcpy(modifiers, level.initialModifiers, sizeof(modifiers));
    }

    [[nodiscard]] bool isClickable(size_t row, size_t col) const {
        return Field::isClickable(modifiers[row][col]);
    }

    [[nodiscard]] uint64_t hash() const {
        return MurmurHash64(modifiers, sizeof(modifiers));
    }

    std::string toString() {
        std::string description("", rows * (cols + 1) * 2 + 1);
        for (size_t row = 0; row < rows; row++) {
            for (size_t col = 0; col < cols; col++) {
                description[row * (cols + 1) + col] = level->colors[row][col];
                description[rows * (cols + 1) + 1 + row * (cols + 1) + col] = modifiers[row][col];
            }
            description[row * (cols + 1) + cols] = '\n';
            description[rows * (cols + 1) + 1 + row * (cols + 1) + cols] = '\n';
        }
        description[rows * (cols + 1)] = '\n';
        return description;
    }

    void print() {
        for (size_t row = 0; row < rows; row++) {
            std::cout<<"# ";
            for (size_t col = 0; col < cols; col++) {
                std::cout<<"\033[0m";
                if (modifiers[row][col] == 'X') {
                    std::cout<<"\033[40m   ";
                    continue;
                }
                switch (level->colors[row][col]) {
                    case 'r':
                        std::cout<<"\033[41m";
                        break;
                    case 'g':
                        std::cout<<"\033[42m";
                        break;
                    case 'b':
                        std::cout<<"\033[44m";
                        break;
                    case 'o':
                        std::cout<<"\033[43m";
                        break;
                    case 'd':
                        std::cout<<"\033[45m";
                        break;
                    default:
                        std::cout<<"ERROR";
                        break;
                }
                std::cout<<" ";
                if (Field::isColor(modifiers[row][col])
                    && modifiers[row][col] == level->colors[row][col]) {
                    std::cout<<"□";
                } else if (Field::isColor(modifiers[row][col])) {
                    switch (modifiers[row][col]) {
                        case 'r':
                            std::cout<<"\033[31m";
                            break;
                        case 'g':
                            std::cout<<"\033[32m";
                            break;
                        case 'b':
                            std::cout<<"\033[34m";
                            break;
                        case 'o':
                            std::cout<<"\033[33m";
                            break;
                        case 'd':
                            std::cout<<"\033[35m";
                            break;
                        default:
                            std::cout << "ERROR";
                            break;
                    }
                    std::cout << "■";
                } else {
                    switch (modifiers[row][col]) {
                        case '0':
                            std::cout << "\033[30m■";
                            break;
                        case 'D':
                        case 's':
                            std::cout << "↓";
                            break;
                        case 'L':
                        case 'a':
                            std::cout << "←";
                            break;
                        case 'R':
                        case 'x':
                            std::cout << "→";
                            break;
                        case 'U':
                        case 'w':
                            std::cout << "↑";
                            break;
                        case 'F':
                            std::cout << "○";
                            break;
                        case 'B':
                            std::cout << "▲";
                            break;
                        default:
                            std::cout << "ERROR";
                            break;
                    }
                }
                std::cout<<" ";
                std::cout<<"\033[0m";
            }
            std::cout<<std::endl;
        }
    }

    bool fill(int dr, int rc, size_t row, size_t col, char color) {
        row += dr;
        col += rc;
        if (row >= rows || col >= cols) {
            return false;
        }
        char from;
        char to;
        if (modifiers[row][col] == color) { // Un-fill
            from = color;
            to = '0';
        } else if (modifiers[row][col] == '0') { // Fill
            from = '0';
            to = color;
        } else {
            return false;
        }
        while (row < rows && col < cols && modifiers[row][col] == from) {
            modifiers[row][col] = to;
            row += dr;
            col += rc;
        }
        return true;
    }

    bool flood(size_t row, size_t col, char from, char to) {
        if (row >= rows || col >= cols) {
            return false;
        }
        if (modifiers[row][col] == from) {
            modifiers[row][col] = to;
            flood(row + 1, col, from, to);
            flood(row - 1, col, from, to);
            flood(row, col + 1, from, to);
            flood(row, col - 1, from, to);
            return true;
        } else {
            return false;
        }
    }

    bool click(size_t row, size_t col) {
        moveSequence.moves[moveSequence.n].col = col;
        moveSequence.moves[moveSequence.n].row = row;
        moveSequence.n++;

        char &modifier = modifiers[row][col];
        char color = level->colors[row][col];
        if (modifier == 'U') {
            return fill(-1, 0, row, col, color);
        } else if (modifier == 'D') {
            return fill(1, 0, row, col, color);
        } else if (modifier == 'L') {
            return fill(0, -1, row, col, color);
        } else if (modifier == 'R') {
            return fill(0, 1, row, col, color);
        } else if (modifier == 'F') {
            char from = '0';
            char to = color;
            bool somethingFilled = false;
            somethingFilled |= flood(row + 1, col, from, to);
            somethingFilled |= flood(row - 1, col, from, to);
            somethingFilled |= flood(row, col + 1, from, to);
            somethingFilled |= flood(row, col - 1, from, to);

            if (!somethingFilled) {
                from = color;
                to = '0';
                somethingFilled |= flood(row + 1, col, from, to);
                somethingFilled |= flood(row - 1, col, from, to);
                somethingFilled |= flood(row, col + 1, from, to);
                somethingFilled |= flood(row, col - 1, from, to);
            }
            return somethingFilled;
        } else if (modifier == 'B') {
            for (size_t dr = 0; dr < 3; dr++) {
                for (size_t dc = 0; dc < 3; dc++) {
                    if (row - 1 + dr < rows && col - 1 + dc < cols) {
                        char &m = modifiers[row - 1 + dr][col - 1 + dc];
                        if (m != 'X') {
                            m = color;
                        }
                    }
                }
            }
            return true;
        } else if (modifier == 'w') {
            fill(-1, 0, row, col, color);
            modifier = 'x';
            return true;
        } else if (modifier == 's') {
            fill(1, 0, row, col, color);
            modifier = 'a';
            return true;
        } else if (modifier == 'a') {
            fill(0, -1, row, col, color);
            modifier = 'w';
            return true;
        } else if (modifier == 'x') {
            fill(0, 1, row, col, color);
            modifier = 's';
            return true;
        } else {
            std::cout<<"Unknown modifier"<<std::endl;
        }
        return false;
    }

    [[nodiscard]] bool isSolved() const {
        if (level == nullptr) {
            return false;
        }
        for (size_t row = 0; row < rows; row++) {
            for (size_t col = 0; col < cols; col++) {
                if (!Field::isCorrect(level->colors[row][col], modifiers[row][col])) {
                    return false;
                }
            }
        }
        return true;
    }

    bool click(const char *string) {
        return click(string[1] - '0' - 1, string[0] - 'A');
    }

    bool click(const Position position) {
        return click(position.row, position.col);
    }
};
