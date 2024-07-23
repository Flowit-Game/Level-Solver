#include <iostream>
#include <fstream>
#include "MurmurHash64.hpp"
#include "Board.hpp"
#include "SimpleXml.hpp"
#include "BfsSolver.hpp"
#include "BranchBoundSolver.hpp"

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cout<<"Specify a path"<<std::endl;
        exit(1);
    }
    std::cout<<argv[1]<<std::endl;
    std::ifstream t(argv[1]);
    std::string xml((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
    size_t pos = 0;
    SimpleXml::consume("<?xml version=\"1.0\" encoding=\"utf-8\" ?>", xml, pos);
    SimpleXml::skipWhitespace(xml, pos);
    SimpleXml::consume("<levels>", xml, pos);

    size_t indexInFile = 1;
    while (true) {
        auto [levelNr, color, modifier] = SimpleXml::parseBoardXml(xml, pos);
        std::cout<<"# Level "<<indexInFile<<" (id "<<levelNr<<")"<<std::endl;
        Board board = Board::from(color, modifier);

        //Board solvedBoard = solveBFS(levelNr, board);
        Board solvedBoard = solveBranchAndBound(levelNr, board);

        if (!solvedBoard.isSolved()) {
            std::cout<<"# Unable to solve "<<levelNr<<std::endl;
            board.print();
        } else {
            std::cout<<"# Solved with "<<solvedBoard.moveSequence.n<<" moves: "
                     <<solvedBoard.moveSequence.toString()<<std::endl;
            board.print();

            std::string levelnr = "<level number=\""+std::to_string(levelNr)+"\"";
            std::string replacement = "        solution=\""+solvedBoard.moveSequence.toString()+"\"";
            std::cout<<"sed -i 's/"<<levelnr<<"/"<<levelnr<<"\\n"<<replacement<<"/' levels.xml";
        }
        indexInFile++;
        std::cout<<std::endl;
    }
}
