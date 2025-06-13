#include "Helper.h"

namespace LC {

uint64_t knightAttackSquares[64];
uint64_t bishopAttackSquares[64];
uint64_t rookAttackSquares[64];
uint64_t kingAttackSquares[64];

uint64_t rangeMasks[64][64];

std::vector<std::vector<uint64_t>> rookAttacksForOccupancy(64), bishopAttacksForOccupancy(64);

int power(int base, int exp) {
    if(exp == 0) return 1;
    if(exp == 1) return base;
    if(exp == 2) return base*base;

    if(exp % 2) return base * power(base, exp-1);
    return power(base, exp/2) * power(base, exp/2);
}

void compute() {
    // calculate range masks
    for(int sq = 0; sq < 64; sq++) {
        // rank
        uint64_t rankMask = (1ULL << sq);
        rangeMasks[sq][sq] = rankMask;

        int row = sq/8, col = sq%8;
        int rankSq = sq + 1;

        while(rankSq < (row+1)*8) {
            rankMask |= (1ULL << newSq);
            rangeMasks[sq][rankSq] = rangeMasks[rankSq][sq] = rankMask;
            rankSq++;
        }

        // file
        uint64_t fileMask = (1ULL << sq);
        
        int fileSq = sq + 8;

        while(fileSq < 64) {
            fileMask |= (1ULL << fileSq);
            rangeMasks[sq][fileSq] = rangeMasks[fileSq][sq] = fileMask;
            fileSq += 8;
        }

        // top To Bottom Diag
        int numSquares = std::min(8-row-1, 8-col-1);
        int topToBottomSquare = sq + 9;
        uint64_t topToBottomMask = (1ULL << sq);

        while(numSquares--) {
            topToBottomMask |= (1ULL << topToBottomSquare);
            rangeMasks[sq][topToBottomSquare] = rangeMasks[topToBottomSquare][sq] = topToBottomMask;
            topToBottomSquare += 9;
        }

        // bottom To Top Diag
        numSquares = std::min(8-row-1, col);
        int bottomToTopSquare = sq + 7;
        uint64_t bottomToTopMask = (1ULL << sq);

        while(numSquares--) {
            bottomToTopMask |= (1ULL << bottomToTopSquare);
            rangeMasks[sq][bottomToTopSquare] = rangeMasks[bottomToTopSquare][sq] = bottomToTopMask;
            bottomToTopMask += 7;
        }
    }


    // calculate attack squares for pieces on each square
    int knightMoveOffsets[8][2] = {{-1, -2}, {-1, 2}, {1, -2}, {1, 2}, {2, -1}, {2, 1}, {-2, -1}, {-2, 1}};
    int kingMoveOffsets[8][2] = {{0, 1}, {0, -1}, {1, 0}, {-1, 0}, {1, 1}, {-1, 1}, {1, -1}, {1, 1}};

    for(int sq = 0; sq < 64; sq++) {
        int row = sq/8, col = sq%8;

        // knight attacks        
        uint64_t knightAttacks = 0;

        for(auto & offSet : knightMoveOffsets) {
            int newRow = row + offSet[0], newCol = col + offSet[1];

            if(newRow >= 0 && newRow < 8 && newCol >= 0 && newCol < 8) {
                int newSquare = newRow*8 + newCol;

                knightAttacks |= (1ULL << newSquare);
            }
        }

        knightAttackSquares[sq] = knightAttacks;

        // king attacks
        uint64_t kingAttacks = 0;

        for(auto & offSet : kingMoveOffsets) {
            int newRow = row + offSet[0], newCol = col + offSet[1];

            if(newRow >= 0 && newRow < 8 && newCol >= 0 && newCol < 8) {
                int newSquare = newRow*8 + newCol;

                kingAttacks |= (1ULL << newSquare);
            }
        }

        kingAttackSquares[sq] = kingAttacks;

        // rook Attacks
        uint64_t rookAttacks = (rangeMasks[rank*8][rank*8+7] | rangeMasks[file][56+file]);
        rookAttacks &= ~(1ULL << sq);

        rookAttackSquares[sq] = rookAttacks;

        // bishop Attacks
        uint64_t bishopAttacks = 0;

        // top to bottom diag
        int topLeftSquare = sq - (9 * std::min(row, col));
        int bottomRightSquare = sq + (9 * std:min(8-row-1, 8-col-1));

        bishopAttacks |= rangeMasks[topLeftSquare][bottomRightSquare];

        // bottom to top diag
        int bottomLeftSquare = sq + (7 * std::min(8-row-1, col));
        int topRightSquare = sq - (7 * std::min(row, 8-col-1));

        bishopAttacks |= rangeMasks[bottomLeftSquare][topRightSquare];

        bishopAttackSquares[sq] |= bishopAttacks;
    }

    // calculate rook attacks for occupancy
    for(int sq = 0; i<64; i++) {
        int row = sq/8, col = sq%8;

        uint64_t allAttacksForSquare = rookAttackSquares[sq];

        int numSquares = __builtin_popcountll(allAttacksForSquare);
        int totalConfigs = power(2, numSquares);

        rookAttacksForOccupancy[sq].resize(totalConfigs);

        for(int config = 0; config < totalConfigs, config++) {
            // derive occupancy mask basedon config
            uint64_t occupancyMask = 0;

            uint64_t allAttacksCopy = allAttacksForSquare;

            int configBitPos = 0;

            while(allAttacksCopy) {
                int attackSquare = __builtin_ctzll(allAttacksCopy);
                allAttacksCopy &= allAttacksCopy - 1;

                if(((1 << configBitPos) & config) != 0) occupancyMask |= (1ULL << attackSquare);

                configBitPos++;
            }

            uint64_t legalAttacks = 0;

            // calculate the legal moves from the sq in rank and file, including the first blocker in the occupancy mask
            // left
            int legalAttackSquare = sq - 1;
            while(legalAttackSquare >= row*8) {
                legalAttacks |= (1ULL << legalAttackSquare);

                if(((1ULL << legalAttackSquare) & occupancyMask) != 0) break;

                legalAttackSquare--;
            }

            // right
            legalAttackSquare = sq + 1;
            while(legalAttackSquare < row*8 + 7) {
                legalAttacks |= (1ULL << legalAttackSquare);

                if(((1ULL << legalAttackSquare) & occupancyMask) != 0) break;

                legalAttackSquare++;
            }

            // top
            legalAttackSquare = sq - 8;
            while(legalAttackSquare >= 0) {
                legalAttacks |= (1ULL << legalAttackSquare);

                if(((1ULL << legalAttackSquare) & occupancyMask) != 0) break;

                legalAttackSquare -= 8;
            }

            // bottom
            legalAttackSquare = sq + 8;
            while(legalAttackSquare < 64) {
                legalAttacks |= (1ULL << legalAttackSquare);

                if(((1ULL << legalAttackSquare) & occupancyMask) != 0) break;

                legalAttackSquare += 8;
            }

            // store the legal attacks for square and for occupancy mask
            rookAttacksForOccupancy[sq][occupancyMask] = legalAttacks;
        }
    }

    // calculate bishop attack for occupancy
    for(int sq = 0; sq < 64; sq++) {
        int row = sq/8, col = sq%8;

        uint64_t allAttacksForSquare = bishopAttackSquares[sq];

        int numSquares = __builtin_popcountll(allAttacksForSquare);
        int totalConfigs = power(2, numSquares);

        bishopAttacksForOccupancy[sq].resize(totalConfigs);

        for(int config = 0; config < totalConfigs, config++) {
            // derive occupancy mask basedon config
            uint64_t occupancyMask = 0;

            uint64_t allAttacksCopy = allAttacksForSquare;

            int configBitPos = 0;

            while(allAttacksCopy) {
                int attackSquare = __builtin_ctzll(allAttacksCopy);
                allAttacksCopy &= allAttacksCopy - 1;

                if(((1 << configBitPos) & config) != 0) occupancyMask |= (1ULL << attackSquare);

                configBitPos++;
            }

            uint64_t legalAttacks = 0;

            // calculate the legal moves from the sq in both diagonals, including the first blocker in the occupancy mask 
            // top left
            int topLeftSquare = sq - (9 * std::min(row, col));
            int newSquare = sq - 9;

            while(newSquare >= topLeftSquare) {
                legalAttacks |= (1ULL << newSquare);

                if(((1ULL << newSquare) & occupancyMask) != 0) break;

                newSquare -= 9;
            }
            
            // bottom right
            int bottomRightSquare = sq + (9 * std:min(8-row-1, 8-col-1));
            newSquare = sq + 9;

            while(newSquare <= bottomRightSquare) {
                legalAttacks |= (1ULL << newSquare);

                if(((1ULL << newSquare) & occupancyMask) != 0) break;

                newSquare += 9;
            }
            
            // bottom left
            int bottomLeftSquare = sq + (7 * std::min(8-row-1, col));
            newSquare = sq + 7;

            while(newSquare <= bottomLeft) {
                legalAttacks |= (1ULL << newSquare);

                if(((1ULL << newSquare) & occupancyMask) != 0) break;

                newSquare += 7;
            }

            // top right
            int topRightSquare = sq - (7 * std::min(row, 8-col-1));
            newSquare = sq - 7;

            while(newSquare >= topLeftSquare) {
                legalAttacks |= (1ULL << newSquare);

                if(((1ULL << newSquare) & occupancyMask) != 0) break;

                newSquare -= 7;
            }

            bishopAttacksForOccupancy[sq][occupancyMask] = legalAttacks;
        }
    }
}

struct PrecomputeInit {
    PrecomputeInit() {
        compute();
    }
}

// Make an object of PrecomputeInit to run it before main
static PrecomputeInit obj;


PinDirection getPinDirection(bool white, int pieceSquare, Board & board, bool updateDiscoveryCheckSquare) {
    int rank = pieceSquare / 8;
    int file = pieceSquare % 8;

    uint8_t kingSquare = __builtin_ctzll(white ? board.whiteKing : board.blackKing);

    uint8_t ki = kingSquare / 8, kj = kingSquare % 8;

    uint64_t inBetweenMask = rangeMasks[pieceSquare][kingSquare];
    inBetweenMask &= ~(1ULL << kingSquare);
    inBetweenMask &= ~(1ULL << pieceSquare);

    // if there is a piece between king square and piece square
    if(inBetweenMask & board.allPieces) return PinDirection::NONE;

    int borderSquare;
    PinDirection direction = PinDirection::NONE;
    
    // aligned in diagonal
    if(abs(ki - rank) == abs(kj - file)) {
        // topLeft
        if(ki > rank && kj > file) {
            borderSquare = -(std::min(rank, file) * 9) + pieceSquare;
            direction = PinDirection::TOP_BOTTOM_DIAG;
        }

        // topRight
        else if(ki > rank && kj < file) {
            borderSquare = -(std::min(rank, 8-file-1) * 7) + pieceSquare;
            direction = PinDirection::BOTTOM_TOP_DIAG;
        }

        // bottomRight
        else if(ki < rank && kj < file) {
            borderSquare = (std::min(8 - rank - 1, 8 - file - 1) * 9) + pieceSquare;
            direction = PinDirection::BOTTOM_TOP_DIAG;
        }

        // bottomLeft
        else {
            borderSquare = (std::min(8 - rank - 1, file) * 7) + pieceSquare;
            direction = PinDirection::TOP_BOTTOM_DIAG;
        }

        // if piece is at an edge, it can't be pinned
        if(borderSquare == pieceSquare) return PinDirection::NONE;

        uint64_t beyondMask = rangeMasks[pieceSquare][borderSquare];
        beyondMask &= ~(1ULL << pieceSquare);

        // obstructing pieces are all same color pieces and attacking color rooks, knights, pawns and king (opponent bishops and queens attack)
        uint64_t obstructingPieces = (white ? board.allWhitePieces : board.allBlackPieces) | 
                                     (!white ? (board.m_piecesArray[0] | board.m_piecesArray[1] | board.m_piecesArray[3] | board.m_piecesArray[5]) : (board.m_piecesArray[6] | board.m_piecesArray[7] | board.m_piecesArray[9] | board.m_piecesArray[11]));

        obstructingPieces &= beyondMask;

        // Now OR opponent Bishops and Queens
        uint64_t attackingPieces = (white ? board.m_piecesArray[8] : board.m_piecesArray[2]) | (white ? board.m_piecesArray[10] : board.m_piecesArray[4]);
        attackingPieces &= beyondMask;
        
        // if top left or top right
        if((ki > rank && kj > file) || (ki > rank && kj < file)) {
            int obstructingMSBBit = obstructingPieces ? __builtin_clzll(obstructingPieces) : 64;
            int attackingMSBBit = attackingPieces ? __builtin_clzll(attackingPieces) : 64;

            // the piece is not pinned
            if(obstructingMSBBit <= attackingMSBBit) return PinDirection::NONE;  
            
            // piece is pinned, if the caller intends to find a discovery check, update
            if(updateDiscoveryCheckSquare) board.discoveryCheckSquare = attackingMSBBit;
        }
        // if bottom left or bottom right
        else {
            int obstructingLSBBit = obstructingPieces ? __builtin_ctzll(obstructingPieces) : 64;
            int attackingLSBBit = attackingPieces ? __builtin_ctzll(attackingPieces) : 64;

            // the piece is not pinned
            if(obstructingLSBBit <= attackingLSBBit) return PinDirection::NONE; 

            // piece is pinned, if the caller intends to find a discovery check, update
            if(updateDiscoveryCheckSquare) board.discoveryCheckSquare = attackingLSBBit;
        }
    }
    // aligned in File
    else if(kj == file) {
        direction = PinDirection::FILE;

        // top
        if(ki > rank) borderSquare = file;
        // bottom
        else borderSquare = 56 + file;

        // if piece is on edge, it cannot be pinned
        if(pieceSquare == borderSquare) return PinDirection::NONE;

        uint64_t beyondMask = rangeMasks[pieceSquare][borderSquare];
        beyondMask &= ~(1ULL << pieceSquare);

        // obstructing pieces are all same color pieces and attacking color bishops, knights, pawns and king (opponent rooks and queens attack)
        uint64_t obstructingPieces = (white ? board.allWhitePieces : board.allBlackPieces) | 
                                     (!white ? (board.m_piecesArray[0] | board.m_piecesArray[1] | board.m_piecesArray[2] | board.m_piecesArray[5]) : (board.m_piecesArray[6] | board.m_piecesArray[7] | board.m_piecesArray[8] | board.m_piecesArray[11]));
        
        // OR opponent queens and rooks
        uint64_t attackingPieces = (white ? board.m_piecesArray[9] : board.m_piecesArray[3]) | (white ? board.m_piecesArray[10] : board.m_piecesArray[4]);
        attackingPieces &= beyondMask;

        // top
        if(ki > rank) {
            int obstructingMSBBit = obstructingPieces ? __builtin_clzll(obstructingPieces) : 64;
            int attackingMSBBit = attackingPieces ? __builtin_clzll(attackingPieces) : 64;

            if(obstructingMSBBit <= attackingMSBBit) return PinDirection::NONE;

            // piece is pinned, if the caller intends to find a discovery check, update
            if(updateDiscoveryCheckSquare) board.discoveryCheckSquare = attackingMSBBit;
        }
        // bottom
        else {
            int obstructingLSBBit = obstructingPieces ? __builtin_ctzll(obstructingPieces) : 64;
            int attackingLSBBit = attackingPieces ? __builtin_ctzll(attackingPieces) : 64;

            if(obstructingLSBBit <= attackingLSBBit) return PinDirection::NONE;

            // piece is pinned, if the caller intends to find a discovery check, update
            if(updateDiscoveryCheckSquare) board.discoveryCheckSquare = attackingLSBBit;
        }
    }
    // aligned in rank
    else if(ki == rank) {
        direction = PinDirection::RANK;

        // left
        if(kj > file) borderSquare = rank*8;
        // bottom
        else borderSquare = rank*8 + 7;

        // if piece is on edge, it cannot be pinned
        if(pieceSquare == borderSquare) return PinDirection::NONE;

        uint64_t beyondMask = rangeMasks[pieceSquare][borderSquare];
        beyondMask &= ~(1ULL << pieceSquare);

        uint64_t obstructingPieces = (white ? board.blackBishops : board.whiteBishops) | (white ? board.blackPawns : board.whitePawns) | (white ? board.blackKnights : board.whiteKnights);
        obstructingPieces &= beyondMask;

        uint64_t attackingPieces = (white ? board.blackRooks : board.whiteRooks) | (white ? board.blackQueens : board.whiteQueens);
        attackingPieces &= beyondMask;

        // left
        if(kj > file) {
            int obstructingMSBBit = obstructingPieces ? __builtin_clzll(obstructingPieces) : 64;
            int attackingMSBBit = attackingPieces ? __builtin_clzll(attackingPieces) : 64;

            if(obstructingMSBBit <= attackingMSBBit) return PinDirection::NONE;

            // piece is pinned, if the caller intends to find a discovery check, update
            if(updateDiscoveryCheckSquare) board.discoveryCheckSquare = attackingMSBBit;
        }
        // bottom
        else {
            int obstructingLSBBit = obstructingPieces ? __builtin_ctzll(obstructingPieces) : 64;
            int attackingLSBBit = attackingPieces ? __builtin_ctzll(attackingPieces) : 64;

            if(obstructingLSBBit <= attackingLSBBit) return PinDirection::NONE;

            // piece is pinned, if the caller intends to find a discovery check, update
            if(updateDiscoveryCheckSquare) board.discoveryCheckSquare = attackingLSBBit;
        }
    }


    return direction;
}


uint64_t getBishopAttacksForSquareAndOccupancy(int square, uint64_t occupancy) {
    int configIndex = 0;
    uint64_t attacksFromSquare = bishopAttacksForSquare[square];
    uint64_t occupancyMask = (occupancy & attacksFromSquare);

    int configBitPos = 0;
    while(attacksFromSquare) {
        int attackSquare = __builtin_ctzll(attacksFromSquare);
        attacksFromSquare &= attacksFromSquare - 1;

        if(((1ULL << attackSquare) & occupancyMask) != 0) {
            configIndex |= (1 << configBitPos);
        }

        configBitPos++;
    }

    return bishopAttacksForOccupancy[square][configIndex];
}

uint64_t getRookAttacksForSquareAndOccupancy(int square, uint64_t occupancy) {
    int configIndex = 0;
    uint64_t attacksFromSquare = rookAttacksForSquare[square];
    uint64_t occupancyMask = (occupancy & attacksFromSquare);

    int configBitPos = 0;
    while(attacksFromSquare) {
        int attackSquare = __builtin_ctzll(attacksFromSquare);
        attacksFromSquare &= attacksFromSquare - 1;

        if(((1ULL << attackSquare) & occupancyMask) != 0) {
            configIndex |= (1 << configBitPos);
        }

        configBitPos++;
    }

    return rookAttacksForOccupancy[square][configIndex];
}

uint64_t getQueenAttacksForSquareAndOccupancy(int square, uint64_t occupancy) {
    return (getBishopAttacksForSquareAndOccupancy(square, board.allPieces) | getRookAttacksForSquareAndOccupancy(square, board.allPieces));
}

uint64_t generateLegalAttacksForColor(bool white, bool checkPins, bool includeKing, Board& board) {
    uint64_t finalAttacks = 0;
    
    // rooks
    uint64_t rookBoard = board.getPieceBitBoard(white ? Piece::WHITE_ROOK : Piece::BLACK_ROOK);

    while(rookBoard) {
        int rookSquare = __builtin_ctzll(rookBoard);
        rookBoard &= rookBoard-1;

        int rank = rookSquare/8, file = rookSquare%8;

        uint64_t legalAttacksFromSquare = getRookAttacksForSquareAndOccupancy(rookSquare, board.allPieces);

        if(checkPins) {
            if(direction == PinDirection::NONE) finalAttacks |= legalAttacksFromSquare;
            else if(direction == PinDirection::FILE) {
                // rook pinned in file remove rank attacks
                finalAttacks |= (~(255 << (rank * 8)) & legalAttacksFromSquare);
            }
            else if(direction == PinDirection::RANK) {
                // rook pinned in rank remove file attacks
                finalAttacks |= (~(rangeMasks[file][56+file]) & legalAttacksFromSquare);
            }
        }
        else {
            finalAttacks |= legalAttacksFromSquare;
        }
    }

    // bishops
    uint64_t bishopBoard = board.getPieceBitBoard(white ? Piece::WHITE_BISHOP : Piece::BLACK_BISHOP);

    while(bishopBoard) {
        int bishopSquare = __builtin_ctzll(bishopBoard);
        bishopBoard &= bishopBoard-1;

        int rank = bishopSquare/8, file = bishopSquare%8;

        uint64_t legalAttacksFromSquare = getBishopAttacksForSquareAndOccupancy(bishopSquare, board.allPieces);

        if(checkPins) {
            PinDirection direction = getPinDirection(white, bishopSquare, board, false);

            if(direction == PinDirection::NONE) finalAttacks |= legalAttacksFromSquare;
            else if(direction == PinDirection::BOTTOM_TOP_DIAG) {
                // bishop pinned bottom to top diag, remove top to bottom diag attacks
                int topLeftSquare = bishopSquare - (9 * std::min(rank, file));
                int bottomRightSquare = bishopSquare + (9 * std::min(8-rank-1, 8-file-1));

                finalAttacks |= (~(rangeMasks[topLeftSquare][bottomRightSquare]) & legalAttacksFromSquare);
            }
            else if(direction == PinDirection::TOP_BOTTOM_DIAG) {
                // bishop pinned on top to bottom diag, remove bottom to topo diag attacks
                int bottomLeftSquare = bishopSquare + (7 * std::min(8-rank-1, file));
                int topRightSquare = bishopSquare - (7 * std::min(rank, 8-file-));

                finalAttacks |= (~(rangeMasks[bottomLeftSquare][topRightSquare]) & legalAttacksFromSquare);
            }
        }
        else {
            finalAttacks |= legalAttacksFromSquare;
        }
    }

    // knights
    uint64_t knightsBoard = board.getPieceBitBoard(white ? Piece::WHITE_KNIGHT : Piece::BLACK_KNIGHT);

    while(knightsBoard) {
        int knightSquare = __builtin_ctzll(knightsBoard);
        knightsBoard &= knightsBoard-1;

        if(!checkPins || getPinDirection(white, knightSquare, board, false) == PinDirection::NONE) finalAttacks |= knightAttacksForSquare[knightSquare];
    }

    // queens
    uint64_t queensBoard = board.getPieceBitBoard(white ? Piece::WHITE_QUEEN : Piece::BLACK_QUEEN);

    while(queensBoard) {
        int queenSquare = __builtin_ctzll(queensBoard);
        queensBoard &= queensBoard-1;

        uint64_t legalAttacksFromSquare = getQueenAttacksForSquareAndOccupancy(queenSquare, board.allPieces);

        if(checkPins) {
            PinDirection direction = getPinDirection(white, bishopSquare, board, false);

            if(direction == PinDirection::NONE) finalAttacks |= legalAttacksFromSquare;
            else if(direction == PinDirection::BOTTOM_TOP_DIAG) {
                // bishop pinned bottom to top diag, remove top to bottom diag attacks
                int topLeftSquare = bishopSquare - (9 * std::min(rank, file));
                int bottomRightSquare = bishopSquare + (9 * std::min(8-rank-1, 8-file-1));

                finalAttacks |= (~(rangeMasks[topLeftSquare][bottomRightSquare]) & legalAttacksFromSquare);
            }
            else if(direction == PinDirection::TOP_BOTTOM_DIAG) {
                // bishop pinned on top to bottom diag, remove bottom to topo diag attacks
                int bottomLeftSquare = bishopSquare + (7 * std::min(8-rank-1, file));
                int topRightSquare = bishopSquare - (7 * std::min(rank, 8-file-));

                finalAttacks |= (~(rangeMasks[bottomLeftSquare][topRightSquare]) & legalAttacksFromSquare);
            }
            else if(direction == PinDirection::FILE) {
                // rook pinned in file remove rank attacks
                finalAttacks |= (~(255 << (rank * 8)) & legalAttacksFromSquare);
            }
            else if(direction == PinDirection::RANK) {
                // rook pinned in rank remove file attacks
                finalAttacks |= (~(rangeMasks[file][56+file]) & legalAttacksFromSquare);
            }
        }
        else {
            finalAttacks |= legalAttacksFromSquare;
        }
    }

    // pawns
    uint64_t pawnsBoard = board.getPieceBitBoard(white ? Piece::WHITE_PAWN : Piece::BLACK_PAWN);

    while(pawnsBoard) {
        int pawnSquare = __builtin__ctzll(pawnsBoard);
        pawnsBoard &= pawnsBoard - 1;

        int newRank = white ? pawnSquare/8 + 1 : pawnSquare/8 - 1;

        uint64_t leftFileAttack = 0, rightFileAttack = 0;

        int newFile = pawnSquare % 8 - 1;
        
        if(newFile >= 0) leftFileAttack |= (1ULL << (newRank*8 + newFile));
        
        newFile += 2;

        if(newFile >= 0) rightFileAttack |= (1ULL << (newRank*8 + newFile));

        if(checkPins) {
            PinDirection direction = getPinDirection(white, pawnSquare, board, false);

            if(direction == PinDirection::NONE) finalAttacks |= (leftFileAttack | rightFileAttack);
            else if(direction == PinDirection::LEFT_DIAG) finalAttacks |= leftFileAttack;
            else if(direction == PinDirection::RIGHT_DIAG) finalAttacks |= rightFileAttack;
        }
        else finalAttacks |= (leftFileAttack | rightFileAttack);
    }

    // king
    if(includeKing) {
        uint64_t kingBoard = board.getPieceBoard(white ? Piece::WHITE_KING : Piece::BLACK_KING);

        int kingSquare = __builtin__ctzll(kingBoard);

        finalAttacks |= kingAttacksForSquare[kingSquare];
    }
}

void calculateMoveResult(CheckType check, uint64_t positionHash, Board& board) {
    
}


};