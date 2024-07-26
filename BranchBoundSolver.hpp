#pragma once

#include <vector>
#include <unordered_set>
#include <cassert>
#include <unordered_map>
#include "Board.hpp"
#include "SimpleApproximateMap.hpp"

size_t minStepsNeeded(const Board &board) {
    bool needed[5] = { false, false, false, false, false };
    size_t missing = 0;

    for (size_t row = 0; row < rows; row++) {
        for (size_t col = 0; col < cols; col++) {
            const Field &field = board.fields[row][col];
            if (field.isCorrect()) {
                continue;
            }
            size_t mphf = Field::colorMPHF(field.getColor());
            missing += needed[mphf] ? 0 : 1;
            needed[mphf] = true;
            if (missing == 5) {
                return 5;
            }
        }
    }
    assert(missing <= 5);
    return missing;
}

void branch(size_t levelNr, Board board, size_t &bound, Board &best, SimpleApproximateMap<uint64_t, size_t> &minimalMoves) {
    if (board.moveSequence.n >= bound) {
        return; // Give up
    }
    uint64_t hash = board.hash();
    auto existing = minimalMoves.get(hash);
    if (existing == nullptr) {
        minimalMoves.insert(hash, board.moveSequence.n);
    } else {
        if (*existing <= board.moveSequence.n) {
            // Someone else already reached this state with fewer moves
            return; // Give up
        } else {
            *existing = board.moveSequence.n;
        }
    }

    size_t stepsNeeded = minStepsNeeded(board);
    if (board.moveSequence.n + stepsNeeded >= bound) {
        return;
    }

    if (board.isSolved()) {
        if (board.moveSequence.n < bound) {
            bound = board.moveSequence.n;
            std::cout<<"# New bound for "<<levelNr<<": "
                     <<bound<<" using "<<board.moveSequence.toString()<<std::endl;
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
    static SimpleApproximateMap<uint64_t, size_t> minimalMoves;

    for (size_t iterativeBound = 10; iterativeBound <= maxSteps; iterativeBound += 5) {
        size_t bound = iterativeBound;
        minimalMoves.clear();
        Board best = {};
        branch(levelNr, initialBoard, bound, best, minimalMoves);
        if (best.isSolved()) {
            return best;
        }
    }
    return {};
}
