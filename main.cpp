#include <iostream>
#include <vector>
#include <unordered_set>
#include <fstream>

constexpr size_t rows = 8;
constexpr size_t cols = 6;

struct Field {
    char color;
    char modifier;

    [[nodiscard]] bool isClickable() const {
        return modifier == 'L' || modifier == 'R' || modifier == 'U' || modifier == 'D'
            || modifier == 'w' || modifier == 'x' || modifier == 'a' || modifier == 's'
            || modifier == 'F' || modifier == 'B';
    }

    [[nodiscard]] bool isCorrect() const {
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
};

struct Board {
    Field fields[rows][cols] = {};
    std::pair<char, char> moveSequence[50];
    size_t moves = 0;

    std::string toString() {
        std::string description("", rows * cols * 2 + 1);
        for (size_t row = 0; row < rows; row++) {
            for (size_t col = 0; col < cols; col++) {
                description[row * cols + col] = fields[row][col].color;
                description[rows * cols + 1 + row * cols + col] = fields[row][col].modifier;
            }
        }
        description[rows * cols + 1] = '\n';
        return description;
    }

    void fill(int dr, int rc, size_t row, size_t col, char color) {
        row += dr;
        col += rc;
        char from;
        char to;
        if (fields[row][col].modifier == color) { // Un-fill
            from = color;
            to = '0';
        } else if (fields[row][col].modifier == '0') { // Fill
            from = '0';
            to = color;
        } else {
            return;
        }
        while (fields[row][col].modifier == from) {
            fields[row][col].modifier = to;
            row += dr;
            col += rc;
        }
    }

    bool flood(size_t row, size_t col, char from, char to) {
        if (row >= rows || col >= cols) {
            return false;
        }
        if (fields[row][col].modifier == from) {
            fields[row][col].modifier = to;
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
        moveSequence[moves] = std::make_pair(row, col);
        moves++;

        Field field = fields[row][col];
        if (field.modifier == 'U') {
            fill(-1, 0, row, col, field.color);
        } else if (field.modifier == 'D') {
            fill(1, 0, row, col, field.color);
        } else if (field.modifier == 'L') {
            fill(0, -1, row, col, field.color);
        } else if (field.modifier == 'R') {
            fill(0, 1, row, col, field.color);
        } else if (field.modifier == 'F') {
            char from = '0';
            char to = field.color;
            bool somethingFilled = false;
            somethingFilled |= flood(row + 1, col, from, to);
            somethingFilled |= flood(row - 1, col, from, to);
            somethingFilled |= flood(row, col + 1, from, to);
            somethingFilled |= flood(row, col - 1, from, to);

            if (!somethingFilled) {
                from = field.color;
                to = '0';
                flood(row + 1, col, from, to);
                flood(row - 1, col, from, to);
                flood(row, col + 1, from, to);
                flood(row, col - 1, from, to);
            }
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
};

void skipWhitespace(std::string &xml, size_t &pos) {
    while (xml[pos] == ' ' || xml[pos] == '\n') {
        pos++;
    }
}

void consume(char c, std::string &xml, size_t &pos) {
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

std::string solve(std::string color, std::string modifier) {
    Board initialBoard;
    if (color.length() == 5 * 6) {
        for (size_t row = 0; row < 6; row++) {
            for (size_t col = 0; col < 5; col++) {
                char c = color[row * 5 + col];
                char m = modifier[row * 5 + col];
                initialBoard.fields[row][col] = {c, m };
            }
        }
        for (size_t row = 6; row < rows; row++) {
            for (size_t col = 5; col < cols; col++) {
                initialBoard.fields[row][col] = { '0', 'X' };
            }
        }
    } else if (color.length() == 6 * 8) {
        for (size_t row = 0; row < rows; row++) {
            for (size_t col = 0; col < cols; col++) {
                char c = color[row * cols + col];
                char m = modifier[row * cols + col];
                initialBoard.fields[row][col] = {c, m };
            }
        }
    } else {
        std::cout<<"error: Unknown size"<<std::endl;
        exit(1);
    }

    std::vector<Board> queueThis;
    std::vector<Board> queueNext;
    std::unordered_set<std::string> seen;
    queueThis.push_back(initialBoard);
    size_t steps = 0;

    while (!queueThis.empty()) {
        Board board = queueThis.back();
        queueThis.pop_back();

        for (size_t row = 0; row < rows; row++) {
            for (size_t col = 0; col < cols; col++) {
                if (!board.fields[row][col].isClickable()) {
                    continue;
                }
                Board newBoard = board;
                newBoard.click(row, col);
                if (newBoard.isSolved()) {
                    std::string sequence = "";
                    for (size_t i = 0; i < newBoard.moves; i++) {
                        sequence += ('A' + newBoard.moveSequence[i].second);
                        sequence += std::to_string(newBoard.moveSequence[i].first + 1) + ",";
                    }
                    return sequence;
                } else {
                    std::string code = newBoard.toString();
                    if (!seen.contains(code)) {
                        queueNext.push_back(newBoard);
                        seen.emplace(code);
                    }
                }
            }
        }

        if (queueThis.empty()) {
            queueThis = queueNext;
            queueNext.clear();
            steps++;

            if (steps > 10) {
                std::cout<<"Calculating solutions with "<<steps<<" steps."<<std::endl;
                std::cout<<"Queue length: "<<queueThis.size()<<std::endl;
            }
        }
    }
    return "";
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
        std::cout<<color<<std::endl<<modifier<<std::endl;

        std::string moveSequence = solve(color, modifier);
        if (moveSequence.empty()) {
            std::cout<<"Unable to solve"<<std::endl;
            return 1;
        } else {
            std::cout<<"Solved: "<<moveSequence<<std::endl;
        }
        indexInFile++;
    }
}
