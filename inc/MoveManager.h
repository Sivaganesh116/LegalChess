#ifndef __MoveManager_h__
#define __MoveManager_h__

#include "Board.h"

#include <cstdint>

namespace LC {

// each value represents the position of piece relative to king
enum PinDirection {
    NONE,
    FILE,
    RANK,
    LEFT_DIAG,
    RIGHT_DIAG
};

class Board;

class IMoveManager {
public:
    static PinDirection getPinDirection(bool white, int pieceSquare, Board & board, bool updateDiscoveryCheckSquare);
    static uint64_t generateLegalAttacks(bool, bool, Board&);
    static bool isKingUnderCheck(bool isWhite, Board& board);
    static uint64_t getBishopAttacksForCurrentBoardState(int square, uint64_t allPieces);
    static uint64_t getRookAttacksForCurrentBoardState(int square, uint64_t allPieces);
    static uint64_t getQueenAttacksForCurrentBoardState(int square, uint64_t allPieces);
    static CheckType handlePromotion(char newPiece, int fromSquare, int toSquare, int fromRow, int fromCol, int toRow, int toCol, bool isWhite, Board& board);
    static void handleKingCastle(bool shortSide, bool isWhite, Board& board);

    

    // check pattern
    // perform move
    // generate opponent legal attacks
    // if king under check undo the move
    // generate our own legal attacks
    // if opponent king under check, check for checkmate
    // else check for stalemate or insufficient material
    virtual CheckType handleMove(int fromSquare, int toSquare, int fromRow, int fromCol, int toRow, int toCol, bool isWhite, Board& board) = 0;

private:

};

class PawnMoveManager : public IMoveManager {
public:
    CheckType handleMove(int fromSquare, int toSquare, int fromRow, int fromCol, int toRow, int toCol, bool isWhite, Board& board) override;
};

class KnightMoveManager : public IMoveManager {
public:
    CheckType handleMove(int fromSquare, int toSquare, int fromRow, int fromCol, int toRow, int toCol, bool isWhite, Board& board) override;
};

class BishopMoveManager : public IMoveManager {
public:
    CheckType handleMove(int fromSquare, int toSquare, int fromRow, int fromCol, int toRow, int toCol, bool isWhite, Board& board) override;
};

class RookMoveManager : public IMoveManager {
public:
    CheckType handleMove(int fromSquare, int toSquare, int fromRow, int fromCol, int toRow, int toCol, bool isWhite, Board& board) override;
};

class QueenMoveManager : public IMoveManager {
public:
    CheckType handleMove(int fromSquare, int toSquare, int fromRow, int fromCol, int toRow, int toCol, bool isWhite, Board& board) override;
};

class KingMoveManager : public IMoveManager {
public:
    CheckType handleMove(int fromSquare, int toSquare, int fromRow, int fromCol, int toRow, int toCol, bool isWhite, Board& board) override;
};

class MoveManagerFactory {
public:
    MoveManagerFactory();
    ~MoveManagerFactory() = default;

    std::shared_ptr<IMoveManager> getMoveManager(char piece);

    static MoveManagerFactory& getReference();

private:
    std::shared_ptr<IMoveManager> m_PawnManager;
    std::shared_ptr<IMoveManager> m_KnightManager;
    std::shared_ptr<IMoveManager> m_BishopManager;
    std::shared_ptr<IMoveManager> m_RookManager;
    std::shared_ptr<IMoveManager> m_QueenManager;
    std::shared_ptr<IMoveManager> m_KingManager;
};

}

#endif
