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
    gameOver = whiteKingCheckmated = blackKingCheckmated = stalemate = drawBy50HalfMoves = drawByInsufficientMaterial = drawByRepitition = false;
    canWhiteKingShortCastle = canBlackKingShortCastle = canWhiteKingLongCastle = canBlackKingLongCastle = true;

    enpassantSquare = discoveryCheckSquare = directCheckSquare = 64;

    halfMovesCount = movesCount = 0; // they treat each player's turn as different moves

    m_pMoveManagerStore = MoveManagerStore::getMoveManagerStore();
    m_pZobrist = Zobrist::getInstance();

    result = '.';
}


void Board::move(int fromRow, int fromCol, int toRow, int toCol, int fromSquare, int toSquare) {
    Piece movingPiece = grid[fromRow][fromCol];

    if(movingPiece == Piece::EMPTY) {
        // To-do: throw error
    }

    // check turn, 0 to 5 are white pieces
    if(isWhiteTurn != ((int)movingPiece/6 == 0)) {
        // To-do: throw error
    }

    CheckType check = m_pMoveManagerStore->getPieceMoveManager(movingPiece)->handleMove(fromRow, fromCol, toRow, toCol, isWhiteTurn, *this);

    // update the move in grid
    grid[toRow][toCol] = movingPiece;
    grid[fromRow][fromCol] = Piece::EMPTY;

    // update enpassant square if the moved piece is not a pawn
    if(movingPiece != Piece::WHITE_PAWN && movingPiece != Piece::BLACK_PAWN) enpassantSquare = 64;

    int castlingRights = 0;
    if(canWhiteKingShortCastle) castlingRights |= (1 << 0);
    if(canWhiteKingLongCastle) castlingRights |= (1 << 1);
    if(canBlackKingShortCastle) castlingRights |= (1 << 2);
    if(canBlackKingLongCastle) castlingRights |= (1 << 3);

    uint64_t positionHash = m_pZobrist->computeHash(grid, !isWhiteTurn, castlingRights, enpassantSquare % 8);
    positionHashToFreq[positionHash]++;

    calculateMoveResult(check, positionHash, isWhiteTurn, *this);

    isWhiteTurn = !isWhiteTurn;
}


void Board::promote(Piece newPiece, int fromRow, int fromCol, int toRow, int toCol, int fromSquare, int toSquare) {
    Piece movingPiece = grid[fromRow][fromCol];

    if(movingPiece == Piece::EMPTY) {
        // To-do: throw error
    }

    // check turn, 0 to 5 are white pieces
    if(isWhiteTurn != ((int)movingPiece/6 == 0)) {
        // To-do: throw error
    }

    // if the moving piece is not a pawn
    if(movingPiece != Piece::WHITE_PAWN && movingPiece != Piece::BLACK_PAWN) {
        // To-do:: throw error
    }

    CheckType check = std::static_pointer_cast<PawnMoveManager>(m_pMoveManagerStore->getPieceMoveManager(movingPiece))->handlePromotion(newPiece, fromRow, fromCol, toRow, toCol, isWhiteTurn, *this);

    // update the move in grid
    grid[toRow][toCol] = newPiece;
    grid[fromRow][fromCol] = Piece::EMPTY;

    // update enpassant square if the moved piece is not a pawn
    if(movingPiece != Piece::WHITE_PAWN && movingPiece != Piece::BLACK_PAWN) enpassantSquare = 64;

    int castlingRights = 0;
    if(canWhiteKingShortCastle) castlingRights |= (1 << 0);
    if(canWhiteKingLongCastle) castlingRights |= (1 << 1);
    if(canBlackKingShortCastle) castlingRights |= (1 << 2);
    if(canBlackKingLongCastle) castlingRights |= (1 << 3);

    uint64_t positionHash = m_pZobrist->computeHash(grid, !isWhiteTurn, castlingRights, enpassantSquare % 8);
    positionHashToFreq[positionHash]++;

    calculateMoveResult(check, positionHash, isWhiteTurn, *this);

    isWhiteTurn = !isWhiteTurn;
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

inline void Board::updateDiscoveryCheckSquare(int square) {
    discoveryCheckSquare = square;
}

inline Piece Board::getPieceOnBoard(int square) {
    return grid[square/8][square%8];
}

inline bool Board::isKingUnderCheck(bool white) const {
    // return isKingUnderCheck(white, *this); // To-Do
    return false;
}

inline bool Board::isGameOver() const {
    return result != '.';
}

inline bool Board::isCheckmate(bool white) const {
    return white ? whiteKingCheckmated : blackKingCheckmated;
}

inline bool Board::isStalemate() const {
    return stalemate;
}

inline bool Board::isDrawByRepitition() const {
    return drawByRepitition;
}

inline bool Board::isDrawBy50HalfMoves() const {
    return drawBy50HalfMoves;
}

inline bool Board::isDrawyByInsufficientMaterial() const {
    return drawByInsufficientMaterial;
}

inline std::string Board::getFENString() const {
    std::string fenString = "";

    for(int i = 7; i >= 0; i--) {
        int emptyCount = 0;
        for(int j = 7; j >= 0; j--) {
            if(grid[i][j] == Piece::EMPTY) emptyCount++;
            else {
                if(emptyCount) fenString.push_back('0' + emptyCount);
                // fenString.push_back(grid[i][j]); To-do
            }
        }

        if(emptyCount) fenString.push_back('0' + emptyCount);
        if(i!= 0) fenString.push_back('/');
    }

    fenString.push_back(' ');
    fenString.push_back(isWhiteTurn ? 'w' : 'b');
    
    fenString.push_back(' ');
    // Castling Rights
    std::string castling = "";
    if (canWhiteKingShortCastle) castling += 'K';
    if (canWhiteKingLongCastle)  castling += 'Q';
    if (canBlackKingShortCastle) castling += 'k';
    if (canBlackKingLongCastle)  castling += 'q';
    fenString += castling.empty() ? "-" : castling;

    fenString.push_back(' ');

    // enpassant square
    if (enpassantSquare != 64) {
        char file = 'h' - (enpassantSquare % 8);
        char rank = '1' + (enpassantSquare / 8);
        fenString += file;
        fenString += rank;
    } else {
        fenString += "-";
    }

    fenString.push_back(' ');
    fenString += std::to_string(halfMovesCount);

    fenString.push_back(' ');
    fenString += std::to_string(movesCount/2 + 1);

    return fenString;
}

inline std::vector<std::vector<char>> Board::getBoard() const {
    std::vector<std::vector<char>> board(8, std::vector<char>(8));

    for(int i = 0; i<8; i++) {
        for(int j = 0; j<8; j++) {
            // board[i][j] = grid[i][j]; To-Do
        }
    }

    return board;
}

inline std::string Board::getMoveHistory() const {
    return moveHistory;
}


};