#pragma once

#include <vector>
#include <unordered_set>
#include <cassert>
#include <unordered_map>
#include <set>
#include "Board.hpp"
#include "SimpleApproximateMap.hpp"

size_t minStepsNeeded(const Board &board) {
    uint8_t positionsNeeded[rows][cols] = { 0 };
    bool colorsNeeded[6] = {false};
    bool colorsHandled[6] = {false};
    bool colorsNeedRemoval[6] = {false};
    size_t missing = 0;
    size_t needsRemoval = 0;

    for (size_t row = 0; row < rows; row++) {
        for (size_t col = 0; col < cols; col++) {
            const Field &field = board.fields[row][col];
            if (field.isCorrect()) {
                continue;
            }
            if (field.getModifier() == '0') {
                colorsNeeded[Field::colorMPHF(field.getColor())] = true;
            }
            if (field.onlyReachableFrom != POSITION_NONE) {
                size_t clicksNeeded = 1;
                size_t neededR = field.onlyReachableFrom.row;
                size_t neededC = field.onlyReachableFrom.col;

                if (board.fields[neededR][neededC].isRotatingArrow()) {
                    char direction = board.fields[neededR][neededC].getModifier();
                    if (row == neededR && col < neededC) { // left
                        if (direction == 'w') clicksNeeded = 4;
                        if (direction == 'x') clicksNeeded = 3;
                        if (direction == 's') clicksNeeded = 2;
                        if (direction == 'a') clicksNeeded = 1;
                    } else if (row == neededR && col > neededC) { // right
                        if (direction == 'w') clicksNeeded = 2;
                        if (direction == 'x') clicksNeeded = 1;
                        if (direction == 's') clicksNeeded = 4;
                        if (direction == 'a') clicksNeeded = 3;
                    } else if (col == neededC && row < neededR) { // above
                        if (direction == 'w') clicksNeeded = 1;
                        if (direction == 'x') clicksNeeded = 4;
                        if (direction == 's') clicksNeeded = 3;
                        if (direction == 'a') clicksNeeded = 2;
                    } else if (col == neededC && row > neededR) { // below
                        if (direction == 'w') clicksNeeded = 3;
                        if (direction == 'x') clicksNeeded = 2;
                        if (direction == 's') clicksNeeded = 1;
                        if (direction == 'a') clicksNeeded = 4;
                    } else {
                        std::cout<<"Unknown rotating arrow"<<std::endl;
                        exit(1);
                    }
                }

                if (positionsNeeded[neededR][neededC] < clicksNeeded) {
                    missing += clicksNeeded - positionsNeeded[neededR][neededC];
                    positionsNeeded[neededR][neededC] = clicksNeeded;
                }
                colorsHandled[Field::colorMPHF(field.getColor())] = true;
            }
            if (Field::isColor(field.getModifier())) {
                // Needs to remove wrong color first
                if (!colorsNeedRemoval[Field::colorMPHF(field.getModifier())]) {
                    colorsNeedRemoval[Field::colorMPHF(field.getModifier())] = true;
                    needsRemoval++;
                }
            }
        }
    }
    for (size_t i = 0; i < 5; i++) { // Colors only give hash values from 0..5
        if (colorsNeeded[i] && !colorsHandled[i]) {
            missing++;
        }
    }
    if (!board.hasBombs) {
        missing += needsRemoval;
    }
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
        static size_t previousPrint = 0;
        previousPrint++;
        if (previousPrint >= 1000000) {
            std::cout<<"# Progress: "<<board.moveSequence.toString()<<std::endl;
            previousPrint = 0;
        }
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

    size_t rowOffset = hash % rows;
    size_t colOffset = (hash >> 10) % cols;
    for (size_t row = 0; row < rows; row++) {
        for (size_t col = 0; col < cols; col++) {
            size_t permutedRow = (row + rowOffset) % rows;
            size_t permutedCol = (col + colOffset) % cols;
            if (!board.fields[permutedRow][permutedCol].isClickable()) {
                continue;
            }
            Board newBoard = board;
            bool somethingChanged = newBoard.click(permutedRow, permutedCol);
            if (somethingChanged) {
                branch(levelNr, newBoard, bound, best, minimalMoves);
            }
        }
    }
}

Board solveBranchAndBound(size_t levelNr, Board initialBoard) {
    static SimpleApproximateMap<uint64_t, size_t> minimalMoves;

    for (size_t iterativeBound = 20; iterativeBound <= maxSteps; iterativeBound += 10) {
        std::cout<<"# Testing "<<iterativeBound<<" steps"<<std::endl;
        size_t bound = iterativeBound + 1;
        minimalMoves.clear();
        Board best = {};
        branch(levelNr, initialBoard, bound, best, minimalMoves);
        if (best.isSolved()) {
            return best;
        }
    }
    return {};
}
