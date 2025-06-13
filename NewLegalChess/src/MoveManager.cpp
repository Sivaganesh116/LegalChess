#include <memory>
#include "MoveManager.h"
#include "Helper.h"

namespace LC {

CheckType PawnMoveManager::handlePromotion(Piece newPiece, int fromRow, int fromCol, int toRow, int toCol, bool isPieceWhite, Board& board) {
    // check move pattern
    if((isPieceWhite ? fromRow != 6 : fromRow != 1) || (isPieceWhite ? toRow != 7 : toRow != 0) || abs(fromCol - toCol) > 1) {
        // To-Do: throw error
    }

    Piece capturedPiece = board.grid[toRow][toCol];

    // if capture, update the captured piece on board (don't have check for the validity of toSquare as it is taken care in board.move())
    if(capturedPiece != Piece::EMPTY) board.updatePieceCountOnBoard(board[toRow][toCol], toRow*8 + toCol, false);

    // remove pawn
    board.updatePieceCountOnBoard(Piece::WHITE_PAWN, fromRow*8 + fromCol, false);

    // add new piece
    board.updatePieceCountOnBoard(newPiece, toRow*8 + toCol, true);


    if(true) { // To-Do: is my king under check after my move is made
        // undo the move
        board.updatePieceCountOnBoard(newPiece, toRow*8 + toCol, false);

        board.updatePieceCountOnBoard(Piece::WHITE_PAWN, fromRow*8 + fromCol, true);

        if(capturedPiece != Piece::EMPTY) board.updatePieceCountOnBoard(capturedPiece, toRow*8 + toCo, inc);

        // To-Do: throw error
    }

    // To-Do: Return the check type the move results in
    if(true) { // if the piece directly checks the opponent king
        // To-Do: update the direct check square

        if(true) { // if the move results in discovery check
            // To-Do: update the discovery check square
        }

        return CheckType::DIRECT_CHECK;
    }

    return CheckType::NONE;
}

CheckType PawnMoveManager::handleMove(int fromRow, int fromCol, int toRow, int toCol, bool isPieceWhite, Board& board) {
    // check move pattern
    if((isPieceWhite ? fromRow - toRow > 0 : fromRow - toRow < 0) || abs(fromCol - toCol) > 1 || (abs(fromRow - toRow) == 2 && (fromRow != 6 && fromRow != 1))) {
        // To-Do: throw error
    }

    Piece capturedPiece = board.grid[toRow][toCol];

    // if capture, update the captured piece on board (don't have check for the validity of toSquare as it is taken care in board.move())
    if(capturedPiece != Piece::EMPTY) board.updatePieceCountOnBoard(board[toRow][toCol], toRow*8 + toCol, false);

    // update the move on board
    board.updateMoveOnBoard(board.grid[fromRow][fromCol], fromRow*8 + fromCol, toRow*8 + toCol);


    if(true) { // To-Do: is my king under check after my move is made
        // undo the move
        board.updateMoveOnBoard(board.grid[fromRow][fromCol], toRow*8 + toCol, fromRow*8 + fromCol);

        if(capturedPiece != Piece::EMPTY) board.updatePieceCountOnBoard(capturedPiece, toRow*8 + toCo, true);

        // To-Do: throw error
    }

    // update enpassant square
    if(abs(fromRow - toRow) == 2) {
        board.enpassantSquare = isPieceWhite ? fromRow*8 + fromCol + 8 : fromRow*8 + fromCol - 8;
    }

    // To-Do: Return the check type the move results in
    if(true) { // if the piece directly checks the opponent king
        // To-Do: update the direct check square

        if(true) { // if the move results in discovery check
            // To-Do: update the discovery check square
        }

        return CheckType::DIRECT_CHECK;
    }

    return CheckType::NONE;
}

CheckType KnightMoveManager::handleMove(int fromRow, int fromCol, int toRow, int toCol, bool isPieceWhite, Board& board) {
    // check move pattern
    if((abs(fromRow - toRow) == 1 && abs(fromCol - toCol) == 2) || (abs(fromRow - toRow) == 2 && abs(fromCol - toCol) == 1)) {
        Piece capturedPiece = board.grid[toRow][toCol];

        // if capture, update the captured piece on board (don't have check for the validity of toSquare as it is taken care in board.move())
        if(capturedPiece != Piece::EMPTY) board.updatePieceCountOnBoard(board[toRow][toCol], toRow*8 + toCol, false);

        // update the move on board
        board.updateMoveOnBoard(board.grid[fromRow][fromCol], fromRow*8 + fromCol, toRow*8 + toCol);


        if(true) { // To-Do: is my king under check after my move is made
            // undo the move
            board.updateMoveOnBoard(board.grid[fromRow][fromCol], toRow*8 + toCol, fromRow*8 + fromCol);

            if(capturedPiece != Piece::EMPTY) board.updatePieceCountOnBoard(capturedPiece, toRow*8 + toCo, true);

            // To-Do: throw error
        }

        // To-Do: Return the check type the move results in
        if(true) { // if the piece directly checks the opponent king
            // To-Do: update the direct check square

            if(true) { // if the move results in discovery check
                // To-Do: update the discovery check square
            }

            return CheckType::DIRECT_CHECK;
        }

        return CheckType::NONE;
    }

    // To-Do: throw error
}

CheckType BishopMoveManager::handleMove(int fromRow, int fromCol, int toRow, int toCol, bool isPieceWhite, Board& board) {
    // check move pattern
    if(abs(fromRow - fromCol) != abs(toRow - toCol)) {
        // To-Do: throw error
    }

    Piece capturedPiece = board.grid[toRow][toCol];

    // if capture, update the captured piece on board (don't have check for the validity of toSquare as it is taken care in board.move())
    if(capturedPiece != Piece::EMPTY) board.updatePieceCountOnBoard(board[toRow][toCol], toRow*8 + toCol, false);

    // update the move on board
    board.updateMoveOnBoard(board.grid[fromRow][fromCol], fromRow*8 + fromCol, toRow*8 + toCol);


    if(true) { // To-Do: is my king under check after my move is made
        // undo the move
        board.updateMoveOnBoard(board.grid[fromRow][fromCol], toRow*8 + toCol, fromRow*8 + fromCol);

        if(capturedPiece != Piece::EMPTY) board.updatePieceCountOnBoard(capturedPiece, toRow*8 + toCo, true);

        // To-Do: throw error
    }

    // To-Do: Return the check type the move results in
    if(true) { // if the piece directly checks the opponent king
        // To-Do: update the direct check square

        if(true) { // if the move results in discovery check
            // To-Do: update the discovery check square
        }

        return CheckType::DIRECT_CHECK;
    }

    return CheckType::NONE;
}

CheckType RookMoveManager::handleMove(int fromRow, int fromCol, int toRow, int toCol, bool isPieceWhite, Board& board) {
    if(fromRow != toRow && fromCol != toCol) {
        // To-Do: throw error
    }

    Piece capturedPiece = board.grid[toRow][toCol];

    // if capture, update the captured piece on board (don't have check for the validity of toSquare as it is taken care in board.move())
    if(capturedPiece != Piece::EMPTY) board.updatePieceCountOnBoard(board[toRow][toCol], toRow*8 + toCol, false);

    // update the move on board
    board.updateMoveOnBoard(board.grid[fromRow][fromCol], fromRow*8 + fromCol, toRow*8 + toCol);


    if(true) { // To-Do: is my king under check after my move is made
        // undo the move
        board.updateMoveOnBoard(board.grid[fromRow][fromCol], toRow*8 + toCol, fromRow*8 + fromCol);

        if(capturedPiece != Piece::EMPTY) board.updatePieceCountOnBoard(capturedPiece, toRow*8 + toCo, true);

        // To-Do: throw error
    }

    // To-Do: Return the check type the move results in
    if(true) { // if the piece directly checks the opponent king
        // To-Do: update the direct check square

        if(true) { // if the move results in discovery check
            // To-Do: update the discovery check square
        }

        return CheckType::DIRECT_CHECK;
    }

    return CheckType::NONE;
}

CheckType QueenMoveManager::handleMove(int fromRow, int fromCol, int toRow, int toCol, bool isPieceWhite, Board& board) {
    if(fromRow != toRow && fromCol != toCol && abs(fromRow - toRow) != abs(fromCol - toCol)) {
        // To-Do: throw error
    }

    Piece capturedPiece = board.grid[toRow][toCol];

    // if capture, update the captured piece on board (don't have check for the validity of toSquare as it is taken care in board.move())
    if(capturedPiece != Piece::EMPTY) board.updatePieceCountOnBoard(board[toRow][toCol], toRow*8 + toCol, false);

    // update the move on board
    board.updateMoveOnBoard(board.grid[fromRow][fromCol], fromRow*8 + fromCol, toRow*8 + toCol);


    if(true) { // To-Do: is my king under check after my move is made
        // undo the move
        board.updateMoveOnBoard(board.grid[fromRow][fromCol], toRow*8 + toCol, fromRow*8 + fromCol);

        if(capturedPiece != Piece::EMPTY) board.updatePieceCountOnBoard(capturedPiece, toRow*8 + toCo, true);

        // To-Do: throw error
    }

    // To-Do: Return the check type the move results in
    if(true) { // if the piece directly checks the opponent king
        // To-Do: update the direct check square

        if(true) { // if the move results in discovery check
            // To-Do: update the discovery check square
        }

        return CheckType::DIRECT_CHECK;
    }

    return CheckType::NONE;
}

CheckType KingMoveManager::handleMove(int fromRow, int fromCol, int toRow, int toCol, bool isPieceWhite, Board& board) {
    // check move pattern
    if(abs(fromRow - toRow) == 2 && fromCol == toCol) {
        return handleKingCastle(toCol == 1, isPieceWhite, board);
    }

    if(abs(fromRow - toRow) > 1 || abs(fromCol - toCol) > 1) {
        // To-Do: throw error
    }


    Piece capturedPiece = board.grid[toRow][toCol];

    // if capture, update the captured piece on board (don't have check for the validity of toSquare as it is taken care in board.move())
    if(capturedPiece != Piece::EMPTY) board.updatePieceCountOnBoard(board[toRow][toCol], toRow*8 + toCol, false);

    // update the move on board
    board.updateMoveOnBoard(board.grid[fromRow][fromCol], fromRow*8 + fromCol, toRow*8 + toCol);


    if(true) { // To-Do: is my king under check after my move is made
        // undo the move
        board.updateMoveOnBoard(board.grid[fromRow][fromCol], toRow*8 + toCol, fromRow*8 + fromCol);

        if(capturedPiece != Piece::EMPTY) board.updatePieceCountOnBoard(capturedPiece, toRow*8 + toCo, true);

        // To-Do: throw error
    }

    if(isPieceWhite && board.canWhiteKingShortCastle) board.canWhiteKingShortCastle = board.canWhiteKingLongCastle = false;
    else if(!isPieceWhite && board.canBlackKingShortCastle) board.canBlackKingShortCastle = board.canBlackKingLongCastle = false; 

    // a king move can only deliver a discovery check
    if(true) { // if the move results in discovery check
        // To-Do: update the discovery check square

        return CheckType::DISCOVERY_CHECK;
    }

    return CheckType::NONE;
}

CheckType KingMoveManager::handleKingCastle(bool shortSide, bool isPieceWhite, Board& board) {
    if(isPieceWhite ? (shortSide ? !board.canWhiteKingShortCastle : !board.canWhiteKingLongCastle) : (shortSide ? !board.canBlackKingShortCastle : !board.canBlackKingLongCastle) ) {
        // To-Do: throw error
    }

    int kingSquare = isPieceWhite ? 3 : 59; int kingToSquare = shortSide ? 1 : 5;
    int rookSquare = isPieceWhite ? (shortSide ? 0 : 7) : (shortSide : 56 : 63);
    int rookToSquare = shortSide ? shortSide ? kingToSquare + 1 : kingToSquare - 1;

    // mask from king square to destination king square
    uint64_t inBetweenMask = rangeMasks[kingSquare][isPieceWhite ? kingToSquare : 56 + kingToSquare];
    inBetweenMask &= ~(1ULL << kingToSquare);

    // if the king path squares are attacked, castling is not possible, throw error
    if((inBetweenMask & generateLegalAttacksForColor(!isPieceWhite, false, true, board) != 0)) {
        // throw error
    }

    // update king move
    board.updatePieceMoveOnBoard(isPieceWhite ? Piece::WHITE_KING : Piece::BLACK_KING, kingSquare, isPieceWhite ? kingToSquare : kingToSquare + 56);

    // update rook move
    board.updatePieceMoveOnBoard(isPieceWhite ? Piece::WHITE_ROOK : Piece::BLACK_ROOK, rookSquare, rookToSquare);

    if(isPieceWhite) board.canWhiteKingShortCastle = board.canWhiteKingLongCastle = false;
    else board.canBlackKingShortCastle = board.canBlackKingLongCastle = false;

    // castling can result only in direct checks
    // To-Do: Return the check type the move results in
    if(true) { // if the piece directly checks the opponent king
        // To-Do: update the direct check square

        return CheckType::DIRECT_CHECK;
    }

    return CheckType::NONE;
}

std::shared_ptr<const MoveManagerStore> MoveManagerStore::m_pInstance = nullptr;

MoveManagerFactory::MoveManagerFactory() {
    m_MoveManagers[0] = std::make_shared<PawnMoveManager>();
    m_MoveManagers[1] = std::make_shared<KnightMoveManager>();
    m_MoveManagers[2] = std::make_shared<BishopMoveManager>();
    m_MoveManagers[3] = std::make_shared<RookMoveManager>();
    m_MoveManagers[4] = std::make_shared<QueenMoveManager>();
    m_MoveManagers[5] = std::make_shared<KingMoveManager>();
}

inline std:shared_ptr<MoveManager> MoveManagerStore::getPieceMoveManager(Piece piece) const {
    return m_moveManagers[(int)piece];
}

std::shared_ptr<const MoveManagerStore> MoveManagerStore::getMoveManagerStore() {
    if(m_pInstance == nullptr) return m_pInstance = std::make_shared<const MoveManagerStore>();

    return m_pInstance;
}

};