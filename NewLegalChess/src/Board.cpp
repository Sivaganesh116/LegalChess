#include "Board.h"
#include "Helper.h"

namespace LC {

class MoveManagerStore;

Board::Board() {
    initBoard();
}

void Board::initBoard() {
    piecesArray[0] = 255;
    piecesArray[0] <<= 8; // Pawns

    piecesArray[1] = 66; // Knights
    piecesArray[2] = 36; // Bishops
    piecesArray[3] = 129; // Rooks
    piecesArray[4] = 16; // Queens
    piecesArray[5] = 8; // King

    
    piecesArray[6] = (piecesArray[0] << 40); // Pawns

    piecesArray[7] = (piecesArray[1] << 56); // Knights
    piecesArray[8] = (piecesArray[2] << 56); // Bishops
    piecesArray[9] = (piecesArray[3] << 56); // Rooks
    piecesArray[10] = (piecesArray[4] << 56); // Queens
    piecesArray[11] = (piecesArray[5] << 56); // King


    for(int i = 0; i<12; i++) {
        uint64_t pieces = piecesArray[i];
        allPiecesBoard |= pieces;

        if(i < 6) allWhitePiecesBoard |= pieces;
        else allBlackPiecesBoard |= pieces;
    }

    // WhitePieces
    // Pawn Row
    for(auto &piece : grid[6]) piece = Piece::WHITE_PAWN;

    // King Row
    grid[0][0] = grid[0][7] = Piece::WHITE_ROOK;
    grid[0][1] = grid[0][6] = Piece::WHITE_KNIGHT;
    grid[0][2] = grid[0][5] = Piece::WHITE_BISHOP;
    grid[0][3] = Piece::WHITE_KING;
    grid[0][4] = Piece::WHITE_QUEEN;

    // Black Pieces
    // King Row
    grid[7][0] = grid[7][7] = Piece::BLACK_ROOK;
    grid[7][1] = grid[7][6] = Piece::BLACK_KNIGHT;
    grid[7][2] = grid[7][5] = Piece::BLACK_BISHOP;
    grid[7][3] = Piece::BLACK_KING;
    grid[7][4] = Piece::BLACK_QUEEN;

    // Pawn Row
    for(auto &piece : grid[1]) piece = Piece::EMPTY;


    // fill empty squares
    for(int rank = 2; rank < 6; rank++) {
        for(auto &piece : grid[rank]) piece = Piece::EMPTY;
    }

    isWhiteTurn = true;
    isGameOver = isWhiteKingCheckmated = isBlackKingCheckmated = isStalemate = isDrawBy50HalfMoves= isDrawByInsufficientMaterial = isDrawByRepitition = false;
    canWhiteKingShortCastle = canBlackKingShortCastle = canWhiteKingLongCastle = canBlackKingLongCastle = true;

    enpassantSquare = discoveryCheckSquare = directCheckSquare = 64;

    halfMovesCount = movesCount = 0; // they treat each player's turn as different moves

    m_pMoveManagerStore = MoveManagerStore::getMoveManagerStore();
    m_pZobristHash = Zobrist::getInstance();
}

inline uint64_t Board::getPieceBitBoard(Piece piece) const {
    return piecesArray[(int)piece];
}

inline uint64_t Board::getColorBitBoard(bool white) const {
    return white ? allWhitePiecesBoard : allBlackPiecesBoard;
}

inline uint64_t Board::getAllPiecesBitBoard() const {
    return allPiecesBoard;
}

inline void Board::updatePieceMoveOnBoard(Piece piece, int fromSquare, int toSquare) {
    int pieceIndex = (int)piece;

    uint64_t& colorBoard = pieceIndex < 6 ? allWhitePiecesBoard : allBlackPiecesBoard;
    uint64_t& pieceBoard = piecesArray[pieceIndex];

    // remove piece on all boards from fromSquare
    colorBoard &= ~(1ULL << fromSquare);
    pieceBoard &= ~(1ULL << fromSquare);
    allPiecesBoard &= ~(1ULL << fromSquare);

    // add piece on all boards to toSquare
    colorBoard |= (1ULL << toSquare);
    pieceBoard |= (1ULL << toSquare);
    allPiecesBoard |= (1ULL << toSquare);   
}

inline void Board::updatePieceCountOnBoard(Piece piece, int square, bool inc) {
    int pieceIndex = (int)piece;

    uint64_t& colorBoard = pieceIndex < 6 ? allWhitePiecesBoard : allBlackPiecesBoard;
    uint64_t& pieceBoard = piecesArray[pieceIndex];

    if(inc) {
        // add piece on all boards to square
        colorBoard |= (1ULL << square);
        pieceBoard |= (1ULL << square);
        allPiecesBoard |= (1ULL << square);  
    }
    else {
        // remove piece on all boards from square
        colorBoard &= ~(1ULL << square);
        pieceBoard &= ~(1ULL << square);
        allPiecesBoard &= ~(1ULL << square);
    }
}

};