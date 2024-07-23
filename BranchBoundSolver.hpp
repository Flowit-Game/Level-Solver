#pragma once

#include "Board.hpp"
#include <vector>
#include <unordered_set>
#include <cassert>

void branch(Board board, size_t &bound, Board &best) {
    if (board.moveSequence.n >= bound) {
        return; // Give up
    }

    if (board.isSolved()) {
        if (board.moveSequence.n < bound) {
            bound = board.moveSequence.n;
            std::cout<<"New bound: "<<bound<<" using "<<board.moveSequence.toString()<<std::endl;
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
            branch(newBoard, bound, best);
        }
    }
}

Board solveBranchAndBound(size_t levelNr, Board initialBoard) {
    size_t bound = 30;
    Board best = {};
    branch(initialBoard, bound, best);
    return best;
}
