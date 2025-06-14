#ifndef __MOVE_MANAGER_H__
#define __MOVE_MANAGER_H__

#include "Board.h"

namespace LC {

class Board;
enum class CheckType;
enum class Piece;

class MoveManager {
public:
    virtual CheckType handleMove(int fromRow, int fromCol, int toRow, int toCol, bool isPieceWhite, Board& board) = 0;
};

class PawnMoveManager : public MoveManager {
public:
    CheckType handleMove(int fromRow, int fromCol, int toRow, int toCol, bool isPieceWhite, Board& board) override;
    CheckType handlePromotion(Piece newPiece, int fromRow, int fromCol, int toRow, int toCol, bool isPieceWhite, Board& board);
};

class KnightMoveManager : public MoveManager {
public:
    CheckType handleMove(int fromRow, int fromCol, int toRow, int toCol, bool isPieceWhite, Board& board) override;
};

class BishopMoveManager : public MoveManager {
public:
    CheckType handleMove(int fromRow, int fromCol, int toRow, int toCol, bool isPieceWhite, Board& board) override;
};

class RookMoveManager : public MoveManager {
public:
    CheckType handleMove(int fromRow, int fromCol, int toRow, int toCol, bool isPieceWhite, Board& board) override;
};

class QueenMoveManager : public MoveManager {
public:
    CheckType handleMove(int fromRow, int fromCol, int toRow, int toCol, bool isPieceWhite, Board& board) override;
};

class KingMoveManager : public MoveManager {
public:
    CheckType handleMove(int fromRow, int fromCol, int toRow, int toCol, bool isPieceWhite, Board& board) override;
    CheckType handleKingCastle(bool shortSide, bool isPieceWhite, Board& board);
};

class MoveManagerStore {
public:
    ~MoveManagerStore() = default;

    static std::shared_ptr<const MoveManagerStore> getMoveManagerStore();

    inline std::shared_ptr<MoveManager> getPieceMoveManager(Piece piece) const;
private:
    MoveManagerStore();

    std::shared_ptr<MoveManager> m_MoveManagers[6];
    static std::shared_ptr<const MoveManagerStore> m_pInstance;
};

};

#endif
