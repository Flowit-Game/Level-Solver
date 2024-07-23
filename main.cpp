#include <iostream>
#include <vector>
#include <unordered_set>
#include <fstream>
#include "MurmurHash64.hpp"

constexpr size_t rows = 8;
constexpr size_t cols = 6;
constexpr size_t maxSteps = 30;

struct Field {
    private:
    char color;
    char modifier;

    public:
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
};

struct Move {
    char row : 4;
    char col : 4;
};

struct MoveSequence {
    Move moves[maxSteps];
    size_t n = 0;
};

struct Board {
    Field fields[rows][cols] = {};
    MoveSequence moveSequence;

    uint64_t hash() {
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
        while (fields[row][col].getModifier() == from) {
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
                    if (row - 1 + dr <= rows && col - 1 + dc <= cols) {
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
};

void skipWhitespace(std::string &xml, size_t &pos) {
    while (xml[pos] == ' ' || xml[pos] == '\n') {
        pos++;
    }
}

void consume(char c, std::string &xml, size_t &pos) {
    if (pos >= xml.length()) {
        std::cout<< "EOF" << std::endl;
        exit(1);
    }
    if (xml[pos] != c) {
        std::cout<< "Expected " << c << std::endl;
        exit(1);
    }
    pos++;
}

void consume(const std::string &str, std::string &xml, size_t &pos) {
    for (char c : str) {
        consume(c, xml, pos);
    }
}

std::tuple<size_t, std::string, std::string> parseXmlElement(std::string &xml, size_t &pos) {
    skipWhitespace(xml, pos);
    consume("<level number=\"", xml, pos);
    size_t levelNr = 0;
    while (xml[pos] >= '0' && xml[pos] <= '9') {
        levelNr *= 10;
        levelNr += xml[pos] - '0';
        pos++;
    }
    consume('"', xml, pos);
    skipWhitespace(xml, pos);
    consume("color=\"", xml, pos);
    std::string color = "";
    while (xml[pos] != '"') {
        skipWhitespace(xml, pos);
        color += xml[pos];
        pos++;
    }
    pos++;
    skipWhitespace(xml, pos);
    consume("modifier=\"", xml, pos);
    std::string modifier = "";
    while (xml[pos] != '"') {
        skipWhitespace(xml, pos);
        modifier += xml[pos];
        pos++;
    }
    pos++;
    skipWhitespace(xml, pos);
    consume("/>", xml, pos);
    return std::make_tuple(levelNr, color, modifier);
}

Board parseBoard(std::string color, std::string modifier) {
    Board initialBoard;
    bool smallBoard = color.length() == 5 * 6;
    for (size_t row = 0; row < rows; row++) {
        for (size_t col = 0; col < cols; col++) {
            if (smallBoard && (row >= 6 || col >= 5)) {
                initialBoard.fields[row][col].setColor('0');
                initialBoard.fields[row][col].setModifier('X');
                continue;
            }
            char c = color[row * (smallBoard ? 5 : 6) + col];
            char m = modifier[row * (smallBoard ? 5 : 6) + col];
            initialBoard.fields[row][col].setColor(c);
            initialBoard.fields[row][col].setModifier(m);
        }
    }
    return initialBoard;
}

Board solve(size_t levelNr, Board initialBoard) {
    std::vector<MoveSequence> queueThis;
    std::vector<MoveSequence> queueNext;
    std::unordered_set<uint64_t> seen;
    queueThis.push_back(initialBoard.moveSequence);
    size_t steps = 0;

    while (!queueThis.empty()) {
        MoveSequence sequence = queueThis.back();
        queueThis.pop_back();
        Board board = initialBoard;
        for (size_t i = 0; i < sequence.n; i++) {
            board.click(sequence.moves[i].row, sequence.moves[i].col);
        }

        for (size_t row = 0; row < rows; row++) {
            for (size_t col = 0; col < cols; col++) {
                if (!board.fields[row][col].isClickable()) {
                    continue;
                }
                Board newBoard = board;
                newBoard.click(row, col);
                if (newBoard.isSolved()) {
                    return newBoard;
                } else {
                    uint64_t code = newBoard.hash();
                    if (!seen.contains(code)) {
                        queueNext.push_back(newBoard.moveSequence);
                        seen.emplace(code);
                    }
                }
            }
        }

        if (queueThis.empty()) {
            queueThis = queueNext;
            queueNext.clear();
            steps++;

            if (steps > 5) {
                std::cout<<"\r\033[K"<<"Calculating solutions for "<<levelNr<<", currently at "
                        <<steps<<" steps. Queue length: "<<queueThis.size()<<std::flush;
            }
            if (steps >= maxSteps) {
                return {};
            }
        }
    }
    return {};
}

int main() {
    std::ifstream t("levels.xml");
    std::string xml((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
    size_t pos = 0;
    consume("<?xml version=\"1.0\" encoding=\"utf-8\" ?>", xml, pos);
    skipWhitespace(xml, pos);
    consume("<levels>", xml, pos);

    size_t indexInFile = 1;
    while (true) {
        auto [levelNr, color, modifier] = parseXmlElement(xml, pos);
        std::cout<<"Level "<<indexInFile<<" (id "<<levelNr<<")"<<std::endl;

        Board board = parseBoard(color, modifier);

        Board solvedBoard = solve(levelNr, board);
        if (!solvedBoard.isSolved()) {
            std::cout<<"Unable to solve "<<levelNr<<std::endl;
            board.print();
        } else {
            std::cout<<"\r\033[K";

            std::string sequence = "";
            for (size_t i = 0; i < solvedBoard.moveSequence.n; i++) {
                sequence += ('A' + solvedBoard.moveSequence.moves[i].col);
                sequence += std::to_string(solvedBoard.moveSequence.moves[i].row + 1);
                if (i != solvedBoard.moveSequence.n - 1) {
                    sequence += ",";
                }
            }
            std::cout<<"Solved with "<<solvedBoard.moveSequence.n<<" moves: "<<sequence<<std::endl;
            board.print();
            //std::string levelnr = "<level number=\""+std::to_string(levelNr)+"\"";
            //std::string replacement = "        solution=\""+sequence+"\"";
            //std::cout<<"sed -i 's/"<<levelnr<<"/"<<levelnr<<"\\n"<<replacement<<"/' levels.xml";
        }
        indexInFile++;
        std::cout<<std::endl;
    }
}
