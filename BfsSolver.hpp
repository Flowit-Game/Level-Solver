#pragma once

#include "Board.hpp"
#include <vector>
#include <unordered_set>

Board solveBFS(size_t levelNr, Board initialBoard) {
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
                std::cout<<"# Calculating solutions for "<<levelNr<<", currently at "
                         <<steps<<" steps. Queue length: "<<queueThis.size()<<std::endl;
            }
            if (steps >= maxSteps) {
                return {};
            }
        }
    }
    return {};
}
