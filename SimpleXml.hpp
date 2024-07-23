#pragma once

#include <string>
#include <iostream>

class SimpleXml {
    public:
        static void skipWhitespace(std::string &xml, size_t &pos) {
            while (xml[pos] == ' ' || xml[pos] == '\n') {
                pos++;
            }
        }

        static void skipToQuote(std::string &xml, size_t &pos) {
            while (xml[pos] != '"') {
                pos++;
            }
            pos++;
        }

        static void consume(char c, std::string &xml, size_t &pos) {
            if (pos >= xml.length()) {
                std::cout << "EOF" << std::endl;
                exit(1);
            }
            if (xml[pos] != c) {
                std::cout << "Expected " << c << std::endl;
                exit(1);
            }
            pos++;
        }

        static void consume(const std::string &str, std::string &xml, size_t &pos) {
            for (char c: str) {
                consume(c, xml, pos);
            }
        }

        static std::tuple<size_t, std::string, std::string> parseBoardXml(std::string &xml, size_t &pos) {
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
            if (xml[pos] == 's') {
                consume("solution=\"", xml, pos);
                skipToQuote(xml, pos);
                skipWhitespace(xml, pos);
            }
            if (xml[pos] == 'a') {
                consume("author=\"", xml, pos);
                skipToQuote(xml, pos);
                skipWhitespace(xml, pos);
            }
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
};
