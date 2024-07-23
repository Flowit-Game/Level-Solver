#pragma once

#include "Board.hpp"
#include <vector>
#include <unordered_set>
#include <cassert>
#include <unordered_map>

void branch(size_t levelNr, Board board, size_t &bound, Board &best, std::unordered_map<uint64_t, size_t> &minimalMoves) {
    if (board.moveSequence.n >= bound) {
        return; // Give up
    }
    uint64_t hash = board.hash();
    if (minimalMoves.contains(hash)) {
        size_t m = minimalMoves.at(hash);
        if (m <= board.moveSequence.n) {
            // Someone else already reached this state with fewer moves
            return; // Give up
        } else {
            minimalMoves.at(hash) = board.moveSequence.n;
        }
    } else {
        minimalMoves.emplace(hash, board.moveSequence.n);
    }

    if (board.isSolved()) {
        if (board.moveSequence.n < bound) {
            bound = board.moveSequence.n;
            std::cout<<"\r\033[K"<<"New bound for "<<levelNr<<": "
                    <<bound<<" using "<<board.moveSequence.toString()<<std::endl;//flush;
            best = board;
        }
        return;
    }

    for (size_t row = 0; row < rows; row++) {
        for (size_t col = 0; col < cols; col++) {
            if (!board.fields[row][col].isClickable()) {
                continue;
            }
            Board newBoard = board;
            newBoard.click(row, col);
            branch(levelNr, newBoard, bound, best, minimalMoves);
        }
    }
}

Board solveBranchAndBound(size_t levelNr, Board initialBoard) {
    size_t bound = maxSteps;
    Board best = {};
    std::unordered_map<uint64_t, size_t> minimalMoves;
    branch(levelNr, initialBoard, bound, best, minimalMoves);
    return best;
}
