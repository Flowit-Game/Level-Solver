#pragma once

#include <vector>
#include <unordered_set>
#include <cassert>
#include <unordered_map>
#include <set>
#include "Board.hpp"
#include "SimpleApproximateMap.hpp"

size_t minStepsNeeded(const Board &board) {
    const Level &level = *board.level;
    uint8_t positionsNeeded[rows][cols] = { 0 };
    bool colorsNeeded[6] = {false};
    bool colorsHandled[6] = {false};
    bool colorsNeedRemoval[6] = {false};
    size_t missing = 0;
    size_t needsRemoval = 0;

    for (size_t row = 0; row < rows; row++) {
        for (size_t col = 0; col < cols; col++) {
            char color = level.colors[row][col];
            char modifier = board.modifiers[row][col];
            if (Field::isCorrect(color, modifier)) {
                continue;
            }
            if (modifier == '0') {
                colorsNeeded[Field::colorMPHF(color)] = true;
            }
            Position reachableFrom = level.onlyReachableFrom[row][col];
            if (reachableFrom != POSITION_NONE) {
                size_t clicksNeeded = 1;
                size_t neededR = reachableFrom.row;
                size_t neededC = reachableFrom.col;

                int rotation = Field::rotation(board.modifiers[neededR][neededC]);
                if (rotation >= 0) {
                    clicksNeeded = (level.rotationTowards[row][col] - rotation + 4) % 4 + 1;
                }

                if (positionsNeeded[neededR][neededC] < clicksNeeded) {
                    missing += clicksNeeded - positionsNeeded[neededR][neededC];
                    positionsNeeded[neededR][neededC] = clicksNeeded;
                }
                colorsHandled[Field::colorMPHF(color)] = true;
            }
            if (Field::isColor(modifier)) {
                // Needs to remove wrong color first
                if (!colorsNeedRemoval[Field::colorMPHF(modifier)]) {
                    colorsNeedRemoval[Field::colorMPHF(modifier)] = true;
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
    if (!level.hasBombs) {
        missing += needsRemoval;
    }
    return missing;
}

void branch(size_t levelNr, const Board &board, uint64_t hash, size_t &bound, Board &best,
            const Board &initialBoard, SimpleApproximateMap &minimalMoves) {
    if (board.moveSequence.n >= bound) {
        return; // Give up
    }
    auto existing = minimalMoves.get(hash);
    if (!existing.found) {
        minimalMoves.insert(hash, board.moveSequence.n);
    } else {
        if (existing.value == board.moveSequence.n) {
            // Someone else already reached this state with the same number of moves
            if (existing.isSameEpoch) {
                // Someone else already recursed from here
                return;
            } else {
                // Still need to recurse from here
                minimalMoves.insert(hash, board.moveSequence.n); // Update epoch
            }
        } else if (existing.value < board.moveSequence.n) {
            // Someone else already reached this state with fewer moves
            return; // Give up
        } else {
            minimalMoves.insert(hash, board.moveSequence.n); // Fewer moves, also update epoch
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

        MoveSequence sequence = board.moveSequence;
        // Try to simplify sequence by removing up to 4 steps (removing fewer when skip1==skip2)
        for (size_t skip1 = 0; skip1 < sequence.n; skip1++) {
            for (size_t skip2 = skip1; skip2 < sequence.n; skip2++) {
                for (size_t skip3 = skip2; skip3 < sequence.n; skip3++) {
                    for (size_t skip4 = skip3; skip4 < sequence.n; skip4++) {
                        Board maybeShorter = initialBoard;
                        for (size_t i = 0; i < sequence.n; i++) {
                            if (i == skip1 || i == skip2 || i == skip3 || i == skip4) {
                                continue;
                            }
                            Position move = sequence.moves[i];
                            if (!maybeShorter.isClickable(move.row, move.col)) {
                                break; // Cannot apply shorter sequence
                            }
                            maybeShorter.click(move);
                        }
                        if (maybeShorter.isSolved() && maybeShorter.moveSequence.n < bound) {
                            bound = maybeShorter.moveSequence.n;
                            std::cout << "# Simplified bound:  "
                                      << bound << " using " << maybeShorter.moveSequence.toString() << std::endl;
                            best = maybeShorter;
                        }
                    }
                }
            }
        }
        return;
    }

    const std::vector<Position> &clickables = board.level->clickables;
    if (clickables.empty()) {
        return;
    }
    // Generate all children first and prefetch their map entries, then recurse.
    // Children of a node at depth n live in slot n, so recursing never overwrites waiting siblings.
    static std::vector<Board> childBuffer((maxSteps + 1) * rows * cols);
    Board *children = &childBuffer[board.moveSequence.n * rows * cols];
    uint64_t childHashes[rows * cols];
    size_t numChildren = 0;
    size_t index = hash % clickables.size();
    for (size_t i = 0; i < clickables.size(); i++) {
        Position position = clickables[index];
        index = (index + 1 == clickables.size()) ? 0 : index + 1;
        if (!board.isClickable(position.row, position.col)) {
            continue; // Destroyed by a bomb
        }
        Board &child = children[numChildren];
        child = board;
        bool somethingChanged = child.click(position);
        if (somethingChanged) {
            childHashes[numChildren] = child.hash();
            minimalMoves.prefetch(childHashes[numChildren]);
            numChildren++;
        }
    }
    for (size_t i = 0; i < numChildren; i++) {
        branch(levelNr, children[i], childHashes[i], bound, best, initialBoard, minimalMoves);
    }
}

Board solveBranchAndBound(size_t levelNr, Board initialBoard) {
    static SimpleApproximateMap minimalMoves;
    minimalMoves.clear();

    size_t boundSteps[] = {10, 15, 20, 30, 40, 50};
    //size_t boundSteps[] = {15, 33};

    for (size_t iterativeBound : boundSteps) {
        if (iterativeBound > maxSteps) {
            std::cout<<"Broken step sequence"<<std::endl;
            exit(1);
        }
        std::cout<<"# Testing "<<iterativeBound<<" steps"<<std::endl;
        size_t bound = iterativeBound + 1;
        minimalMoves.nextEpoch();
        Board best = {};
        branch(levelNr, initialBoard, initialBoard.hash(), bound, best, initialBoard, minimalMoves);
        if (best.isSolved()) {
            return best;
        }
    }
    return {};
}
