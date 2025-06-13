#include <cstdint>
#include <algorithm>

#include "Board.h"

namespace LC {

extern uint64_t knightAttackSquares[64];
extern uint64_t bishopAttackSquares[64];
extern uint64_t rookAttackSquares[64];
extern uint64_t kingAttackSquares[64];

extern uint64_t rangeMasks[64][64];

extern std::vector<std::vector<uint64_t>> rookAttacksForOccupancy, bishopAttacksForOccupancy;

enum class PinDirection {
    NONE,
    RANK,
    FILE,
    TOP_BOTTOM_DIAG,
    BOTTOM_TOP_DIAG
}

class Board;

PinDirection getPinDirection(bool white, int pieceSquare, Board & board, bool updateDiscoveryCheckSquare);
uint64_t getBishopAttacksForSquareAndOccupancy(int square, uint64_t occupancy);
uint64_t getRookAttacksForSquareAndOccupancy(int square, uint64_t occupancy);
uint64_t getQueenAttacksForSquareAndOccupancy(int square, uint64_t occupancy);
uint64_t generateLegalAttacksForColor(bool white, bool checkPins, bool includeKing, Board& board);
void calculateMoveResult(CheckType check, uint64_t positionHash, Board& board)

};