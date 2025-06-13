#include <string>
#include <vector>
#include <cstdint>
#include <unordered_map>
#include <memory>

#include "MoveManager.h"


namespace LC {

class MoveManager;

enum class Piece {
    WHITE_PAWN, 
    WHITE_KNIGHT,
    WHITE_BISHOP,
    WHITE_ROOK,
    WHITE_QUEEN,
    WHITE_KING,
    BLACK_PAWN,
    BLACK_KNIGHT,
    BLACK_BISHOP,
    BLACK_ROOK,
    BLACK_QUEEN,
    BLACK_KING,
    EMPTY
};

enum class CheckType {
    NO_CHECK,
    DIRECT_CHECK,
    DISCOVERY_CHECK,
    DOUBLE_CHECK
};

class Board {
public:
    Board();
    ~Board();

    inline void updatePieceMoveOnBoard(Piece piece, int fromSquare, int toSquare);

    inline void updatePieceCountOnBoard(Piece piece, int square, bool inc);

    void move(int fromRow, int fromCol, int toRow, int toCol, int fromSquare, int toSquare);
    void promote(Piece newPiece, int fromRow, int fromCol, int toRow, int toCol, int fromSquare, int toSquare);
    inline uint64_t getPieceBitBoard(Piece piece) const;
    inline uint64_t getColorBitBoard(bool white) const;
    inline uint64_t getAllPiecesBitBoard() const;


    inline bool isGameOver() const;
    inline bool isKingUnderCheck(bool white) const;
    inline bool isCheckmate(bool white) const;
    inline bool isStalemate() const;
    inline bool isDrawByRepitition() const;
    inline bool isDrawBy50HalfMoves() const;
    inline bool isDrawyByInsufficientMaterial() const;
    inline std::string getFENString() const;
    inline std::vector<std::vector<char>> getBoard() const;
    inline std::string getMoveHistory() const;

private:
    void initBoard();

    uint64_t piecesArray[12];
    uint64_t allWhitePiecesBoard, allBlackPiecesBoard, allPiecesBoard;

    Piece grid[64][64];

    bool isWhiteTurn;
    bool canWhiteKingShortCastle, canWhiteKingLongCastle, canBlackKingShortCastle, canBlackKingLongCastle;
    bool isGameOver, isWhiteKingCheckmated, isBlackKingCheckmated, isStalemate, isDrawByRepitition, isDrawBy50HalfMoves, isDrawByInsufficientMaterial;

    uint16_t enpassantSquare, discoveryCheckSquare, directCheckSquare;
    uint16_t halfMovesCount, movesCount; // they treat each player's turn as different moves
    
    char result;

    std::unordered_map<uint64_t, uint16_t> positionHashToFreq;
    std::shared_ptr<const MoveManagerStore> m_pMoveManagerStore;
    std::shared_ptr<const ZobristHash> m_pZobristHash;
};


};