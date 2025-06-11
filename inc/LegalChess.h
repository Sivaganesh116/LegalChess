#ifndef __LegalChess_h__
#define __LegalChess_h__

#include "Board.h"

#include <string>
#include <vector>
#include <algorithm>
#include <memory>
#include <sstream>

namespace LC {

enum MoveResult {
    GAME_IN_PROGRESS,
    DRAW_BY_REPITITION,
    DRAW_BY_50_MOVE_RULE,
    DRAW_BY_STALEMATE,
    DRAW_BY_INSUFFICIENT_MATERIAL,
    WHITE_WON_BY_CHECKMATE,
    BLACK_WON_BY_CHECKMATE
};

class LegalChess {
public:
    LegalChess() : m_pBoard(std::make_unique<Board>(MoveManagerFactory::getReference())) {}
    LegalChess(std::string uciMoves) : m_pBoard(std::make_unique<Board>(uciMoves, MoveManagerFactory::getReference())) {
        int moveCount = 1;

        std::stringstream ss(uciMoves);
        std::string move;
        while(ss >> move) {
            try {
                if(move.length() == 4) {
                    makeMove(move.substr(0, 2), move.substr(2));
                }
                else if(move.length() == 5) {
                    promotePawn(move.back(), move.substr(0, 2), move.substr(2, 2));
                }
                else {
                    throw LegalChessError("");
                }
                moveCount++;
            }

            catch(std::exception& e) {
                std::string moveMsg = e.what();
                std::string newMsg = "Invalid Move in the move list. Move number: " + std::to_string(moveCount) + move;
                if(moveMsg.length() > 0) {
                    newMsg += std::string("Description: ") + moveMsg;
                }

                throw LegalChessError(newMsg);
            }
        }
    }
    ~LegalChess() = default;

    MoveResult makeMove(std::string fromSquare, std::string toSquare) {
        validateMove('.', fromSquare, toSquare, false);

        m_pBoard->addMoveToHistory(fromSquare + toSquare);

        return getMoveResult();
    }

    MoveResult promotePawn(char newPiece, std::string fromSquare, std::string toSquare) {
        if(std::find(majorPieces.begin(), majorPieces.end(), newPiece) == majorPieces.end()) {
            throw LegalChessError("Invalid piece specified in promotion: " + std::string(1, newPiece));
        }

        validateMove(newPiece, fromSquare, toSquare, true);
        
        m_pBoard->addMoveToHistory(fromSquare + toSquare + std::string(1, newPiece));

        return getMoveResult();
    }

    bool isChecked(char color) {
        return m_pBoard->isChecked(color == 'w' ? true : false);
    }

    bool isCheckmate() {
        return m_pBoard->isCheckMate;
    }

    bool isStalemate() {
        return m_pBoard->isStalemate;
    }

    bool isDrawByInsufficientMaterial() {
        return m_pBoard->isDrawByInsufficientMaterial;
    }

    bool isDrawBy50MoveRule() {
        return m_pBoard->isDrawBy50MoveRule;
    }

    bool isDrawByRepitition() {
        return m_pBoard->isDrawByRepitition;
    }

    std::string getFENFromBoard() {
        return m_pBoard->getFENString();
    }

    std::vector<std::vector<char>> getBoard() {
        return m_pBoard->getBoard();
    }

    std::string getUCINotationMoveHistory() {
        return m_pBoard->moveHistory;
    }
private:
    MoveResult getMoveResult() {
        if(m_pBoard->isCheckMate) return m_pBoard->isWhiteTurn ? MoveResult::WHITE_WON_BY_CHECKMATE : MoveResult::BLACK_WON_BY_CHECKMATE;
        if(m_pBoard->isStalemate) return MoveResult::DRAW_BY_STALEMATE;
        if(m_pBoard->isDrawBy50MoveRule) return MoveResult::DRAW_BY_50_MOVE_RULE;
        if(m_pBoard->isDrawByInsufficientMaterial) return MoveResult::DRAW_BY_INSUFFICIENT_MATERIAL;
        if(m_pBoard->isDrawByRepitition) return MoveResult::DRAW_BY_REPITITION;

        return MoveResult::GAME_IN_PROGRESS;
    }

    void validateMove(char promotionPiece, std::string fromSquare, std::string toSquare, bool promote) {
        if(fromSquare.length() != 2 || toSquare.length() != 2) {
            // throw error
            throw LegalChessError("Invalid square: " + fromSquare + " ," + toSquare);
        }

        char fromFile = fromSquare[0], fromRank = fromSquare[1], toFile = toSquare[0], toRank = toSquare[1];

        if(fromFile < 'a' || fromFile > 'h' || toFile < 'a' || toFile > 'h' || 
           fromRank < 1 || fromRank > 8 || toRank < 1 || toRank > 8) 
        {
            // throw error
            throw LegalChessError("Invalid square: " + fromSquare + " ," + toSquare);
        }

        if(promote) m_pBoard->promote(promotionPiece, fromRank - '1', 'h' - fromFile, toRank - '1', 'h' - toFile, (fromRank- '1') * 8 + 'h' - fromFile, (toRank - '1') * 8 + 'h' - toFile);
        else m_pBoard->move(fromRank - '1', fromFile - 'a', toRank - '1', toFile - 'a', (fromRank-'1') * 8 + fromFile - 'a', (toRank - '1') * 8 + toFile - 'a');
    }

    std::vector<char> validPieces = {'B', 'K', 'N', 'P', 'Q', 'R', 'b', 'k', 'n', 'p', 'q', 'r'};
    std::vector<char> majorPieces = {'B', 'K', 'N', 'Q', 'R', 'b', 'k', 'n', 'q', 'r'};

    std::unique_ptr<Board> m_pBoard;
};

}

#endif