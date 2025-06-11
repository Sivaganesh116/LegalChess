#ifndef __Board_h__
#define __Board_h__


#include "Precompute.h"
#include "ZobristHash.h"
#include "MoveManager.h"


#include <cstdint>
#include <cmath>
#include <string>
#include <stdexcept>
#include <memory>

#define EMPTY '.'

#define WHITE_PAWN 'P'
#define WHITE_KNIGHT 'N'
#define WHITE_BISHOP 'B'
#define WHITE_ROOK 'R'
#define WHITE_QUEEN 'Q'
#define WHITE_KING 'K'

#define BLACK_PAWN 'p'
#define BLACK_KNIGHT 'n'
#define BLACK_BISHOP 'b'
#define BLACK_ROOK 'r'
#define BLACK_QUEEN 'q'
#define BLACK_KING 'k'

namespace LC {

class IMoveManager;
class MoveManagerFactory;

enum CheckType {
    NO_CHECK,
    DIRECT_CHECK,
    DISCOVERED_CHECK,
    DOUBLE_CHECK
};


class LegalChessError : public std::runtime_error {
public:
    explicit LegalChessError(const std::string& msg) : std::runtime_error(msg) {}; 
};


bool isPieceWhite(char piece) {
    return piece >= 'B' && piece <= 'R';
}


class Board {
public:
    uint64_t whitePawns, whiteKnights, whiteBishops, whiteRooks, whiteQueens, whiteKing;
    uint64_t blackPawns, blackKnights, blackBishops, blackRooks, blackQueens, blackKing;

    uint64_t allPieces;

    char grid[8][8];

    uint8_t enpassantSquare = 64, moveNumber = 0, movesWithoutCaptureAndPawns = 0;

    bool canWhiteKingShortCastle, canBlackKingShortCastle, canWhiteKingLongCastle, canBlackKingLongCastle;

    bool isCheckMate, isStalemate, isDrawByInsufficientMaterial, isDrawByRepitition, isDrawBy50MoveRule;

    int8_t m_discoveryCheckSquare;

    bool isWhiteTurn;
    char result;
    bool whiteKingMoved, blackKingMoved;

    std::unordered_map<uint64_t, int8_t> positionToFreq;

    Board(MoveManagerFactory&);

    void addMoveToHistory(std::string move);

    void move(int fromRow, int fromCol, int toRow, int toCol, int fromSquare, int toSquare);

    void promote(char newPiece, int fromRow, int fromCol, int toRow, int toCol, int fromSquare, int toSquare);

    void updateBlackPieceCount(char piece, bool inc, int square);

    void updateWhitePieceCount(char piece, bool inc, int square);

    void calculateMoveResult(char piece, int toSquare, CheckType check, uint64_t position);

    bool isChecked(bool white);

    std::string getFENString();

    std::vector<std::vector<char>> getBoard(); 

    std::string moveHistory;

private:
    uint64_t& getPieceBitBoard(char piece);
    std::unique_ptr<Zobrist> m_pZobrist;
    MoveManagerFactory& m_rMoveManagerFactory;
};

}


#endif
