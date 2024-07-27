#pragma once

#include <cstdint>
#include <cstddef>
#include <string>
#include <iostream>
#include <vector>

constexpr size_t maxSteps = 40;
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
    private:
        char color = 'g';
        char modifier = '0';
    public:
        Position onlyReachableFrom = POSITION_NONE;

        [[nodiscard]] bool isClickable() const {
            char m = getModifier();
            return m == 'L' || m == 'R' || m == 'U' || m == 'D'
                   || m == 'w' || m == 'x' || m == 'a' || m == 's'
                   || m == 'F' || m == 'B';
        }

        [[nodiscard]] bool isCorrect() const {
            if (!isColor(getColor())) {
                return true;
            } else if (isColor(getModifier())) {
                return modifier == color;
            } else {
                return getModifier() != '0';
            }
        }

        [[nodiscard]] char getModifier() const {
            return modifier;
        }

        void setModifier(char modifier_) {
            modifier = modifier_;
        }

        [[nodiscard]] char getColor() const {
            return color;
        }

        void setColor(char color_) {
            color = color_;
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

    std::string toString() {
        std::string sequence = "";
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

struct Board {
    Field fields[rows][cols] = {};
    MoveSequence moveSequence;
    bool hasBombs = false;

    [[nodiscard]] uint64_t hash() const {
        return MurmurHash64(&fields, sizeof(fields));
    }

    std::string toString() {
        std::string description("", rows * (cols + 1) * 2 + 1);
        for (size_t row = 0; row < rows; row++) {
            for (size_t col = 0; col < cols; col++) {
                description[row * (cols + 1) + col] = fields[row][col].getColor();
                description[rows * (cols + 1) + 1 + row * (cols + 1) + col] = fields[row][col].getModifier();
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
                if (fields[row][col].getModifier() == 'X') {
                    std::cout<<"\033[40m   ";
                    continue;
                }
                switch (fields[row][col].getColor()) {
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
                if (Field::isColor(fields[row][col].getModifier())
                    && fields[row][col].getModifier() == fields[row][col].getColor()) {
                    std::cout<<"□";
                } else if (Field::isColor(fields[row][col].getModifier())) {
                    switch (fields[row][col].getModifier()) {
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
                    switch (fields[row][col].getModifier()) {
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

    void fill(int dr, int rc, size_t row, size_t col, char color) {
        row += dr;
        col += rc;
        char from;
        char to;
        if (fields[row][col].getModifier() == color) { // Un-fill
            from = color;
            to = '0';
        } else if (fields[row][col].getModifier() == '0') { // Fill
            from = '0';
            to = color;
        } else {
            return;
        }
        while (fields[row][col].getModifier() == from && row < rows && col < cols) {
            fields[row][col].setModifier(to);
            row += dr;
            col += rc;
        }
    }

    bool flood(size_t row, size_t col, char from, char to) {
        if (row >= rows || col >= cols) {
            return false;
        }
        if (fields[row][col].getModifier() == from) {
            fields[row][col].setModifier(to);
            flood(row + 1, col, from, to);
            flood(row - 1, col, from, to);
            flood(row, col + 1, from, to);
            flood(row, col - 1, from, to);
            return true;
        } else {
            return false;
        }
    }

    void click(size_t row, size_t col) {
        moveSequence.moves[moveSequence.n].col = col;
        moveSequence.moves[moveSequence.n].row = row;
        moveSequence.n++;

        Field &field = fields[row][col];
        if (field.getModifier() == 'U') {
            fill(-1, 0, row, col, field.getColor());
        } else if (field.getModifier() == 'D') {
            fill(1, 0, row, col, field.getColor());
        } else if (field.getModifier() == 'L') {
            fill(0, -1, row, col, field.getColor());
        } else if (field.getModifier() == 'R') {
            fill(0, 1, row, col, field.getColor());
        } else if (field.getModifier() == 'F') {
            char from = '0';
            char to = field.getColor();
            bool somethingFilled = false;
            somethingFilled |= flood(row + 1, col, from, to);
            somethingFilled |= flood(row - 1, col, from, to);
            somethingFilled |= flood(row, col + 1, from, to);
            somethingFilled |= flood(row, col - 1, from, to);

            if (!somethingFilled) {
                from = field.getColor();
                to = '0';
                flood(row + 1, col, from, to);
                flood(row - 1, col, from, to);
                flood(row, col + 1, from, to);
                flood(row, col - 1, from, to);
            }
        } else if (field.getModifier() == 'B') {
            char color = field.getColor();
            for (size_t dr = 0; dr < 3; dr++) {
                for (size_t dc = 0; dc < 3; dc++) {
                    if (row - 1 + dr < rows && col - 1 + dc < cols) {
                        Field &f = fields[row - 1 + dr][col - 1 + dc];
                        if (f.getModifier() != 'X') {
                            f.setModifier(color);
                        }
                    }
                }
            }
        } else if (field.getModifier() == 'w') {
            fill(-1, 0, row, col, field.getColor());
            field.setModifier('x');
        } else if (field.getModifier() == 's') {
            fill(1, 0, row, col, field.getColor());
            field.setModifier('a');
        } else if (field.getModifier() == 'a') {
            fill(0, -1, row, col, field.getColor());
            field.setModifier('w');
        } else if (field.getModifier() == 'x') {
            fill(0, 1, row, col, field.getColor());
            field.setModifier('s');
        } else {
            std::cout<<"Unknown modifier"<<std::endl;
        }
    }

    [[nodiscard]] bool isSolved() const {
        for (auto & row : fields) {
            for (auto & field : row) {
                if (!field.isCorrect()) {
                    return false;
                }
            }
        }
        return true;
    }

    void click(const char *string) {
        click(string[1] - '0' - 1, string[0] - 'A');
    }

    using ReachabilityArray = std::vector<Position>[rows][cols];

    static void fillReachability(int dr, int rc, const size_t row, const size_t col, char color, Board &b,
                                 ReachabilityArray &reachableFrom) {
        size_t r = row + dr;
        size_t c = col + rc;
        while (r < rows && c < cols) {
            if (b.fields[r][c].getColor() == color) {
                reachableFrom[r][c].emplace_back(row, col);
            }
            r += dr;
            c += rc;
        }
    }

    static Board from(std::string color, std::string modifier) {
        Board initialBoard;
        bool smallBoard = color.length() == 5 * 6;
        for (size_t row = 0; row < rows; row++) {
            for (size_t col = 0; col < cols; col++) {
                Field &field = initialBoard.fields[row][col];
                if (smallBoard && (row >= 6 || col >= 5)) {
                    field.setColor('0');
                    field.setModifier('X');
                    continue;
                }
                field.setColor(color[row * (smallBoard ? 5 : 6) + col]);
                field.setModifier(modifier[row * (smallBoard ? 5 : 6) + col]);
            }
        }

        // Fill reachability
        ReachabilityArray reachableFrom;
        for (size_t row = 0; row < rows; row++) {
            for (size_t col = 0; col < cols; col++) {
                Field &field = initialBoard.fields[row][col];
                if (!field.isClickable()) {
                    continue;
                }

                if (field.getModifier() == 'U') {
                    fillReachability(-1, 0, row, col, field.getColor(), initialBoard, reachableFrom);
                } else if (field.getModifier() == 'D') {
                    fillReachability(1, 0, row, col, field.getColor(), initialBoard, reachableFrom);
                } else if (field.getModifier() == 'L') {
                    fillReachability(0, -1, row, col, field.getColor(), initialBoard, reachableFrom);
                } else if (field.getModifier() == 'R') {
                    fillReachability(0, 1, row, col, field.getColor(), initialBoard, reachableFrom);
                } else if (field.getModifier() == 'F') {
                    for (size_t r = 0; r < rows; r++) {
                        for (size_t c = 0; c < cols; c++) {
                            if (initialBoard.fields[r][c].getColor() == field.getColor()) {
                                reachableFrom[r][c].emplace_back(row, col);
                            }
                        }
                    }
                } else if (field.getModifier() == 'B') {
                    initialBoard.hasBombs = true;
                    for (size_t dr = 0; dr < 3; dr++) {
                        for (size_t dc = 0; dc < 3; dc++) {
                            if (row - 1 + dr <= rows && col - 1 + dc <= cols) {
                                if (initialBoard.fields[row - 1 + dr][col - 1 + dc].getColor() == field.getColor()) {
                                    reachableFrom[row - 1 + dr][col - 1 + dc].emplace_back(row, col);
                                }
                            }
                        }
                    }
                } else if (field.getModifier() == 'w' || field.getModifier() == 's'
                           || field.getModifier() == 'a' || field.getModifier() == 'x') {
                    fillReachability(-1, 0, row, col, field.getColor(), initialBoard, reachableFrom);
                    fillReachability(1, 0, row, col, field.getColor(), initialBoard, reachableFrom);
                    fillReachability(0, -1, row, col, field.getColor(), initialBoard, reachableFrom);
                    fillReachability(0, 1, row, col, field.getColor(), initialBoard, reachableFrom);
                } else {
                    std::cout << "Unknown modifier" << std::endl;
                }
            }
        }
        for (size_t row = 0; row < rows; row++) {
            for (size_t col = 0; col < cols; col++) {
                if (reachableFrom[row][col].size() == 1) {
                    initialBoard.fields[row][col].onlyReachableFrom = reachableFrom[row][col].front();
                }
            }
        }
        return initialBoard;
    }
};
