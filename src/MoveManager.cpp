#include "MoveManager.h"

namespace LC {

uint64_t IMoveManager::getBishopAttacksForCurrentBoardState(int square, uint64_t allPieces) {
    int rank = square/8, file = square%8;

    // Derive occupancy mask for this position
    int topLeftSquare = -(std::min(rank, file) * 9) + square, 
        bottomRightSquare = (std::min(8 - rank - 1, 8 - file - 1) * 9) + square, 
        topRightSquare = -(std::min((int)rank, 8-file-1) * 7) + square, 
        bottomLeftSquare = (std::min(8 - rank - 1, (int)file) * 7) + square;

    uint64_t allLegalMoves = Precompute::precomputedMasks[topLeftSquare][bottomRightSquare] | Precompute::precomputedMasks[bottomLeftSquare][topRightSquare];
    allLegalMoves &= ~(1ULL << square);

    uint64_t occupancyMask = allLegalMoves & allPieces;

    uint16_t index = Precompute::compressedIndexOfOccupancy(occupancyMask, allLegalMoves);

    return Precompute::bishopAttacks[square][index];
}


uint64_t IMoveManager::getRookAttacksForCurrentBoardState(int square, uint64_t allPieces) {
    int rank = square/8, file = square%8;

    uint64_t allLegalMoves = Precompute::precomputedMasks[rank*8][rank*8 + 7] | Precompute::precomputedMasks[file][56+file];
    allLegalMoves &= ~(1ULL << square);

    uint64_t occupancyMask = allLegalMoves & allPieces;

    uint16_t index = Precompute::compressedIndexOfOccupancy(occupancyMask, allLegalMoves);

    return Precompute::bishopAttacks[square][index];
}


uint64_t IMoveManager::getQueenAttacksForCurrentBoardState(int square, uint64_t allPieces) {
    return getRookAttacksForCurrentBoardState(square, allPieces) | getBishopAttacksForCurrentBoardState(square, allPieces);
}


PinDirection IMoveManager::getPinDirection(bool white, int pieceSquare, Board & board, bool updateDiscoveryCheckSquare) {
    int rank = pieceSquare / 8;
    int file = pieceSquare % 8;

    uint8_t kingSquare = __builtin_ctzll(white ? board.whiteKing : board.blackKing);

    uint8_t ki = kingSquare / 8, kj = kingSquare % 8;

    uint64_t inBetweenMask = Precompute::precomputedMasks[pieceSquare][kingSquare];
    inBetweenMask &= ~(1ULL << kingSquare);
    inBetweenMask &= ~(1ULL << pieceSquare);

    // if there is a piece between king square and piece square
    if(inBetweenMask & board.allPieces) return PinDirection::NONE;

    int borderSquare;
    PinDirection direction;
    
    // aligned in diagonal
    if(abs(ki - rank) == abs(kj - file)) {
        // topLeft
        if(ki > rank && kj > file) {
            borderSquare = -(std::min(rank, file) * 9) + pieceSquare;
            direction = PinDirection::LEFT_DIAG;
        }

        // topRight
        else if(ki > rank && kj < file) {
            borderSquare = -(std::min(rank, 8-file-1) * 7) + pieceSquare;
            direction = PinDirection::RIGHT_DIAG;
        }

        // bottomRight
        else if(ki < rank && kj < file) {
            borderSquare = (std::min(8 - rank - 1, 8 - file - 1) * 9) + pieceSquare;
            direction = PinDirection::RIGHT_DIAG;
        }

        // bottomLeft
        else {
            borderSquare = (std::min(8 - rank - 1, file) * 7) + pieceSquare;
            direction = PinDirection::LEFT_DIAG;
        }

        uint64_t beyondMask = Precompute::precomputedMasks[pieceSquare][borderSquare];

        beyondMask &= ~(1ULL << pieceSquare);

        // first OR opponent pawns and rooks and knights and king
        uint64_t nonDiagPieces = (white ? board.blackPawns : board.whitePawns) | (white ? board.blackRooks : board.whiteRooks) | (white ? board.blackKnights : board.whiteKnights) | (white ? board.blackKing : board.whiteKing);
        nonDiagPieces &= beyondMask;

        // Now OR opponent Bishops and Queens
        uint64_t diagPieces = (white ? board.blackBishops : board.whiteBishops) | (white ? board.blackQueens : board.whiteQueens);
        diagPieces &= beyondMask;
        
        // if top left or top right
        if((ki > rank && kj > file) || (ki > rank && kj < file)) {
            int nonDiagMSBBit = nonDiagPieces ? __builtin_clzll(nonDiagPieces) : 64;
            int diagMSBBit = diagPieces ? __builtin_clzll(diagPieces) : 64;

            // the piece is not pinned
            if(nonDiagMSBBit < diagMSBBit) return PinDirection::NONE;  
            
            // piece is pinned, if the caller intends to find a discovery check, update
            if(updateDiscoveryCheckSquare) board.m_discoveryCheckSquare = diagMSBBit;
        }
        // if bottom left or bottom right
        else {
            int nonDiagLSBBit = nonDiagPieces ? __builtin_ctzll(nonDiagPieces) : 64;
            int diagLSBBit = diagPieces ? __builtin_ctzll(diagPieces) : 64;

            // the piece is not pinned
            if(nonDiagLSBBit < diagLSBBit) return PinDirection::NONE; 

            // piece is pinned, if the caller intends to find a discovery check, update
            if(updateDiscoveryCheckSquare) board.m_discoveryCheckSquare = diagLSBBit;
        }
    }
    // aligned in File
    else if(kj == file) {
        direction = PinDirection::FILE;

        // top
        if(ki > rank) borderSquare = file;
        // bottom
        else borderSquare = 56 + file;

        uint64_t beyondMask = Precompute::precomputedMasks[pieceSquare][borderSquare];
        beyondMask &= ~(1ULL << pieceSquare);

        // opponent pieces
        uint64_t nonLinearPieces = (white ? board.blackBishops : board.whiteBishops) | (white ? board.blackPawns : board.whitePawns) | (white ? board.blackKnights : board.whiteKnights);
        nonLinearPieces &= beyondMask;

        uint64_t linearPieces = (white ? board.blackRooks : board.whiteRooks) | (white ? board.blackQueens : board.whiteQueens);
        linearPieces &= beyondMask;

        // top
        if(ki > rank) {
            int nonLinearMSBBit = nonLinearPieces ? __builtin_clzll(nonLinearPieces) : 64;
            int linearMSBBit = linearPieces ? __builtin_clzll(linearPieces) : 64;

            if(nonLinearMSBBit < linearMSBBit) return PinDirection::NONE;

            // piece is pinned, if the caller intends to find a discovery check, update
            if(updateDiscoveryCheckSquare) board.m_discoveryCheckSquare = linearMSBBit;
        }
        // bottom
        else {
            int nonLinearLSBBit = nonLinearPieces ? __builtin_ctzll(nonLinearPieces) : 64;
            int linearLSBBit = linearPieces ? __builtin_ctzll(linearPieces) : 64;

            if(nonLinearLSBBit < linearLSBBit) return PinDirection::NONE;

            // piece is pinned, if the caller intends to find a discovery check, update
            if(updateDiscoveryCheckSquare) board.m_discoveryCheckSquare = linearLSBBit;
        }
    }
    // aligned in rank
    else {
        direction = PinDirection::RANK;

        // left
        if(kj > file) borderSquare = rank*8;
        // bottom
        else borderSquare = rank*8 + 7;

        uint64_t beyondMask = Precompute::precomputedMasks[pieceSquare][borderSquare];
        beyondMask &= ~(1ULL << pieceSquare);

        uint64_t nonLinearPieces = (white ? board.blackBishops : board.whiteBishops) | (white ? board.blackPawns : board.whitePawns) | (white ? board.blackKnights : board.whiteKnights);
        nonLinearPieces &= beyondMask;

        uint64_t linearPieces = (white ? board.blackRooks : board.whiteRooks) | (white ? board.blackQueens : board.whiteQueens);
        linearPieces &= beyondMask;

        // left
        if(kj > file) {
            int nonLinearMSBBit = nonLinearPieces ? __builtin_clzll(nonLinearPieces) : 64;
            int linearMSBBit = linearPieces ? __builtin_clzll(linearPieces) : 64;

            if(nonLinearMSBBit < linearMSBBit) return PinDirection::NONE;

            // piece is pinned, if the caller intends to find a discovery check, update
            if(updateDiscoveryCheckSquare) board.m_discoveryCheckSquare = linearMSBBit;
        }
        // bottom
        else {
            int nonLinearLSBBit = nonLinearPieces ? __builtin_ctzll(nonLinearPieces) : 64;
            int linearLSBBit = linearPieces ? __builtin_ctzll(linearPieces) : 64;

            if(nonLinearLSBBit < linearLSBBit) return PinDirection::NONE;

            // piece is pinned, if the caller intends to find a discovery check, update
            if(updateDiscoveryCheckSquare) board.m_discoveryCheckSquare = linearLSBBit;
        }
    }


    return direction;
}


uint64_t IMoveManager::generateLegalAttacks(bool white, bool checkPins, Board& board) {
    uint64_t finalAttacks = 0;

    // Rooks
    uint64_t rookBoard = white ? board.whiteRooks : board.blackRooks;

    while(rookBoard) {
        uint8_t rookSquare = __builtin_ctzll(rookBoard);
        rookBoard &= rookBoard-1;

        int rank = rookSquare / 8;
        int file = rookSquare % 8;

        // Derive occupancy mask for this position
        uint64_t allLegalMoves = Precompute::precomputedMasks[rank*8][rank*8 + 7] | Precompute::precomputedMasks[file][56+file];
        allLegalMoves &= ~(1ULL << rookSquare);

        uint64_t occupancyMask = allLegalMoves & board.allPieces;

        uint16_t index = Precompute::compressedIndexOfOccupancy(occupancyMask, allLegalMoves);

        uint64_t attacksFromPosition = Precompute::rookAttacks[rookSquare][index];

        if(checkPins) {
            PinDirection direction = getPinDirection(white, rookSquare, board, false);

            if(direction == PinDirection::NONE) finalAttacks |= attacksFromPosition;
            else if(direction == PinDirection::FILE) {
                // rook pinned in file remove rank attacks
                finalAttacks |= (~(255 << (rank * 8)) & attacksFromPosition);
            }
            else if(direction == PinDirection::RANK) {
                // rook pinned in rank remove file attacks
                finalAttacks |= (~(Precompute::precomputedMasks[file][56+file]) & attacksFromPosition);
            }
        }
        else finalAttacks |= attacksFromPosition;
    }


    // Bishops
    uint64_t bishopBoard = white ? board.whiteBishops : board.blackBishops;

    while(bishopBoard) {
        uint8_t bishopSquare = __builtin_ctzll(bishopBoard);
        bishopBoard &= bishopBoard-1;

        uint8_t rank = bishopSquare / 8;
        uint8_t file = bishopSquare % 8;

        int topLeftSquare = -(std::min(rank, file) * 9) + bishopSquare, 
            bottomRightSquare = (std::min(8 - rank - 1, 8 - file - 1) * 9) + bishopSquare, 
            topRightSquare = -(std::min((int)rank, 8-file-1) * 7) + bishopSquare, 
            bottomLeftSquare = (std::min(8 - rank - 1, (int)file) * 7) + bishopSquare;

        // Derive occupancy mask for this position
        uint64_t allLegalMoves = Precompute::precomputedMasks[topLeftSquare][bottomRightSquare] | Precompute::precomputedMasks[bottomLeftSquare][topRightSquare];
        allLegalMoves &= ~(1ULL << bishopSquare);

        uint64_t occupancyMask = allLegalMoves & board.allPieces;

        uint16_t index = Precompute::compressedIndexOfOccupancy(occupancyMask, allLegalMoves);

        uint64_t attacksFromPosition = Precompute::bishopAttacks[bishopSquare][index];

        if(checkPins) {
            PinDirection direction = getPinDirection(white, bishopSquare, board, false);

            if(direction == PinDirection::NONE) finalAttacks |= attacksFromPosition;
            else if(direction == PinDirection::LEFT_DIAG) {
                // bishop pinned on left diag, remove right diag attacks
                finalAttacks |= (~(Precompute::precomputedMasks[bottomLeftSquare][topRightSquare]) & attacksFromPosition);
            }
            else if(direction == PinDirection::RIGHT_DIAG) {
                // bishop pinned on right diag, remove left diag attacks
                finalAttacks |= (~(Precompute::precomputedMasks[topLeftSquare][bottomRightSquare]) & attacksFromPosition);
            }
        }
        else finalAttacks |= attacksFromPosition;
    }


    // Queens
    uint64_t queenBoard = white ? board.whiteQueens : board.blackQueens;

    while(queenBoard) {
        uint8_t queenSquare = __builtin_ctzll(queenBoard);
        queenBoard &= queenBoard-1;

        uint8_t rank = queenSquare / 8;
        uint8_t file = queenSquare % 8;

        int topLeftSquare = -(std::min(rank, file) * 9) + queenSquare, 
            bottomRightSquare = (std::min(8 - rank - 1, 8 - file - 1) * 9) + queenSquare, 
            topRightSquare = -(std::min((int)rank, 8-file-1) * 7) + queenSquare, 
            bottomLeftSquare = (std::min(8 - rank - 1, (int)file) * 7) + queenSquare;

        // Derive occupancy mask for this position
        uint64_t allLegalMoves = Precompute::precomputedMasks[rank*8][rank*8 + 7] | 
            Precompute::precomputedMasks[file][56 + file] | 
            Precompute::precomputedMasks[topLeftSquare][bottomRightSquare] | 
            Precompute::precomputedMasks[bottomLeftSquare][topRightSquare];

        allLegalMoves &= ~(1ULL << queenSquare);

        uint64_t occupancyMask = allLegalMoves & board.allPieces;

        uint16_t index = Precompute::compressedIndexOfOccupancy(occupancyMask, allLegalMoves);

        uint64_t attacksFromPosition = Precompute::bishopAttacks[queenSquare][index] | Precompute::rookAttacks[queenSquare][index];

        PinDirection direction = getPinDirection(white, queenSquare, board, false);

        if(direction == PinDirection::NONE) {
            finalAttacks |= attacksFromPosition;
        }
        else if(direction == PinDirection::FILE) {
            // queen pinned in file remove rank attacks, leftDiagAttacks and rightDiagAttacks
            finalAttacks |= (~(255 << (rank * 8)) & ~(Precompute::precomputedMasks[topLeftSquare][bottomRightSquare]) & ~(Precompute::precomputedMasks[bottomLeftSquare][topRightSquare]) & attacksFromPosition);
        }
        else if(direction == PinDirection::RANK) {
            // queen pinned in rank remove file attacks, leftDiagAttacks, and rightDiagAttacks
            finalAttacks |= (~(Precompute::precomputedMasks[file][56+file]) & ~(Precompute::precomputedMasks[topLeftSquare][bottomRightSquare]) & ~(Precompute::precomputedMasks[bottomLeftSquare][topRightSquare]) & attacksFromPosition);
        }
        else if(direction == PinDirection::LEFT_DIAG) {
            // queen pinned on left diag, remove rank, file and right diag attacks
            finalAttacks |= (~(255 << (rank * 8)) & ~(Precompute::precomputedMasks[file][56+file]) & ~(Precompute::precomputedMasks[bottomLeftSquare][topRightSquare])  & attacksFromPosition);
        }
        else if(direction == PinDirection::RIGHT_DIAG) {
            // queen pinned on right diag, remove rank, file and left diag attacks
            finalAttacks |= (~(255 << (rank * 8)) & ~(Precompute::precomputedMasks[file][56+file]) & ~(Precompute::precomputedMasks[topLeftSquare][bottomRightSquare]) & attacksFromPosition);
        }
    }

    // Knights
    uint64_t knightBoard = white ? board.whiteKnights : board.blackKnights;

    while(knightBoard) {
        uint8_t knightSquare = __builtin_ctzll(knightBoard);
        knightBoard &= knightBoard-1;

        uint8_t rank = knightSquare / 8;
        uint8_t file = knightSquare % 8;

        uint64_t attacksFromPosition = Precompute::knightAttacks[knightSquare];

        if(checkPins) {
            PinDirection direction = getPinDirection(white, knightSquare, board, false);

            if(direction == PinDirection::NONE) {
                finalAttacks |= attacksFromPosition;
            }
        }
        else finalAttacks |= attacksFromPosition;
    }

    // Pawns
    uint64_t pawnsBoard = white ? board.whitePawns : board.blackPawns;

    while(pawnsBoard) {
        uint8_t pawnSquare = __builtin_ctzll(pawnsBoard);
        pawnsBoard &= pawnsBoard-1;

        uint8_t rank = pawnSquare / 8;
        uint8_t file = pawnSquare % 8;

        uint64_t leftFileAttack = 0, rightFileAttack = 0;

        uint8_t newRank = white ? rank + 1 : rank - 1, leftFile = file + 1, rightFile = file - 1;

        if(newRank >= 0 && newRank < 8 && leftFile >= 0 && leftFile < 8) {
            leftFileAttack |= (1ULL << (newRank*8 + leftFile));
        }

        if(newRank >= 0 && newRank < 8 && rightFile >= 0 && rightFile < 8) {
            rightFileAttack |= (1ULL << (newRank*8 + rightFile));
        }

        if(checkPins) {
            PinDirection direction = getPinDirection(white, pawnSquare, board, false);

            if(direction == PinDirection::NONE) finalAttacks |= (leftFileAttack | rightFileAttack);
            else if(direction == PinDirection::LEFT_DIAG) finalAttacks |= leftFileAttack;
            else if(direction == PinDirection::RIGHT_DIAG) finalAttacks |= rightFileAttack;
        }
        else finalAttacks |= (leftFileAttack | rightFileAttack);
    }

    // King
    uint64_t kingBoard = white ? board.whiteKing : board.blackKing;

    while(kingBoard) {
        uint8_t kingSquare = __builtin_ctzll(kingBoard);
        kingBoard &= kingBoard-1;

        finalAttacks |= Precompute::kingAttacks[kingSquare];
    }

    return finalAttacks;
}


bool IMoveManager::isKingUnderCheck(bool isWhite, Board& board) {
    uint64_t opponentAttacks = generateLegalAttacks(isWhite ? false : true, false, board);

    return isWhite ? board.whiteKing : board.blackKing & opponentAttacks;
}


CheckType IMoveManager::handlePromotion(char newPiece, int fromSquare, int toSquare, int fromRow, int fromCol, int toRow, int toCol, bool isWhite, Board & board) {
    if((isWhite ? fromRow != 6 : fromRow != 1) || (isWhite ? toRow != 7 : toRow != 0) || abs(toCol - fromCol) > 1) {
        LegalChessError("Invalid squares or invalid move pattern in pawn promotion.");
    }

    auto & grid = board.grid;
    char capturedPiece = grid[toRow][toCol];

    // normal move
    if(fromCol == toCol && capturedPiece != EMPTY) {
        // throw error
        throw LegalChessError("The promotion square is already occupied.");
    }

    // update the move
    isWhite ? board.updateBlackPieceCount(capturedPiece, false, toSquare) : board.updateWhitePieceCount(capturedPiece, false, toSquare);

    isWhite ? board.whitePawns &= ~(1ULL << fromSquare) : board.blackPawns &= ~(1ULL << fromSquare);

    isWhite ? board.updateWhitePieceCount(newPiece, true, toSquare) : board.updateWhitePieceCount(newPiece, true, toSquare);

    // verify if the king would be put in check
    if(isKingUnderCheck(isWhite, board)) {
        // undo them move and throw error
        isWhite ? board.updateWhitePieceCount(newPiece, false, toSquare) : board.updateWhitePieceCount(newPiece, false, toSquare);

        isWhite ? board.whitePawns |= (1ULL << fromSquare) : board.blackPawns |= (1ULL << fromSquare);

        isWhite ? board.updateBlackPieceCount(capturedPiece, true, toSquare) : board.updateWhitePieceCount(capturedPiece, true, toSquare);

        throw LegalChessError("The move cannot be made. The King is under check or the piece is pinned.");
    }

    uint64_t attacks = 0;

    if(newPiece == 'b' || newPiece == 'B') attacks = getBishopAttacksForCurrentBoardState(toSquare, board.allPieces);
    else if(newPiece == 'n' || newPiece == 'N') attacks = Precompute::knightAttacks[toSquare];
    else if(newPiece == 'r' || newPiece == 'R') attacks = getRookAttacksForCurrentBoardState(toSquare, board.allPieces);
    else if(newPiece == 'q' || newPiece == 'Q') attacks = getQueenAttacksForCurrentBoardState(toSquare, board.allPieces);

    // see if new piece is checking the opponent king
    if(attacks & (isWhite ? board.blackKing : board.whiteKing)) {
        // pinDirection method can be used, if the pawn was pinned, then a discovered check is present
        if(getPinDirection(!isWhite, fromSquare, board, true) != PinDirection::NONE) {
            return CheckType::DOUBLE_CHECK;
        }
        // if there is no discovered check
        return CheckType::DIRECT_CHECK;
    }

    return CheckType::NO_CHECK;

}

void IMoveManager::handleKingCastle(bool shortSide, bool isWhite, Board& board) {
    if(isWhite) {
        if(!board.canWhiteKingLongCastle && !board.canWhiteKingShortCastle) throw LegalChessError("The White king cannot castle");
        if(shortSide && !board.canWhiteKingShortCastle) throw LegalChessError("The White king cannot castle on king side");
        if(!shortSide && !board.canWhiteKingLongCastle) throw LegalChessError("The White king cannot castle on queen side");
    }
    else {
        if(!board.canBlackKingLongCastle && !board.canBlackKingShortCastle) throw LegalChessError("The Black king cannot castle");
        if(shortSide && !board.canBlackKingShortCastle) throw LegalChessError("The Black king cannot castle on king side");
        if(!shortSide && !board.canBlackKingLongCastle) throw LegalChessError("The Black king cannot castle on queen side");
    }


    if(isWhite) {
        // see if the king and rook has any obstructions
        int rangeStart = shortSide ? 1 : 4, rangeEnd = shortSide ? 2 : 6;

        if(Precompute::precomputedMasks[rangeStart][rangeEnd] & board.allPieces) {
            // throw error
            throw LegalChessError("The white king and the rook has obstructions in between them, cannot castle");
        }

        // see if there are attackers from king Square till the new position of king
        uint64_t opponentAttacks = IMoveManager::generateLegalAttacks(!isWhite, false, board);

        if(opponentAttacks & Precompute::precomputedMasks[3][shortSide ? 1 : 5]) {
            // throw error
            throw LegalChessError("The white king is under check or would be on the path to castling");
        }

        board.whiteKing &= ~(1ULL << 3);
        board.whiteKing |= shortSide ? (1ULL << 1) : (1ULL << 5);

        board.whiteRooks &= shortSide ? ~(1ULL) : (1ULL << 7);
        board.whiteRooks |= shortSide ? (1ULL << 2) : (1ULL << 4);

        board.allPieces &= ~(1ULL << 3);
        board.allPieces |= shortSide ? (1ULL << 1) : (1ULL << 5);

        board.allPieces &= shortSide ? ~(1ULL) : (1ULL << 7);
        board.allPieces |= shortSide ? (1ULL << 2) : (1ULL << 4);

        board.canWhiteKingLongCastle = board.canWhiteKingShortCastle = false;
    }
    else {
        // see if the king and rook has any obstructions
        int rangeStart = shortSide ? 57 : 60, rangeEnd = shortSide ? 58 : 62;

        // see if the king and rook has any obstructions
        if(Precompute::precomputedMasks[rangeStart][rangeEnd] & board.allPieces) {
            // throw error
            throw LegalChessError("The black king and the rook has obstructions in between them, cannot castle");
        }

        // see if there are attackers from king Square till the new position of king
        uint64_t opponentAttacks = IMoveManager::generateLegalAttacks(!isWhite, false, board);

        if(opponentAttacks & Precompute::precomputedMasks[59][shortSide ? 57 : 61]) {
            // throw error
            throw LegalChessError("The black king is under check or would be on the path to castling");
        }

        board.blackKing &= ~(1ULL << 59);
        board.blackKing |= shortSide ? (1ULL << 57) : (1ULL << 61);

        board.blackRooks &= shortSide ? ~(1ULL << 56) : ~(1ULL << 63);
        board.blackRooks |= shortSide ? (1ULL << 58) : (1ULL << 60);

        board.allPieces &= ~(1ULL << 59);
        board.allPieces |= shortSide ? (1ULL << 57) : (1ULL << 61);

        board.allPieces &= shortSide ? ~(1ULL << 56) : ~(1ULL << 63);
        board.allPieces |= shortSide ? (1ULL << 58) : (1ULL << 60);

        board.canBlackKingShortCastle = board.canBlackKingLongCastle = false;
    }
}


CheckType PawnMoveManager::handleMove(int fromSquare, int toSquare, int fromRow, int fromCol, int toRow, int toCol, bool isWhite, Board& board) {   
    auto & grid = board.grid;

    // verify rank movements based on color and file movement
    if(isWhite ? (toRow-fromRow != 1 && toRow-fromRow != 2) : (toRow-fromRow != -1 && toRow-fromRow != -2) || abs(toCol - fromCol) > 1 || (abs(fromRow - toRow) == 2 && fromCol - toCol != 0)) {
        // To-Do: throw error
        throw LegalChessError("Invalid move pattern for pawn");
    }

    char capturedPiece = grid[toRow][toCol];

    // if two steps
    if(abs(fromRow - toRow) == 2) {
        uint64_t inBetweenMask = isWhite ? (1ULL << (fromSquare + 8)) : (1ULL << (fromSquare - 8));
        if(inBetweenMask & board.allPieces) {
            // To-Do: throw obstruction error
            throw LegalChessError("There is a piece obstructing the movement of pawn to take two steps.");
        }

        // update the move
        isWhite ? board.whitePawns = board.whitePawns & ~(1ULL << fromSquare) | (1ULL << toSquare) : board.blackPawns = board.blackPawns & ~(1ULL << fromSquare) | (1ULL << toSquare);
        board.allPieces = board.allPieces & ~(1ULL << fromSquare) | (1ULL << toSquare);

        // To-Do: verify if the king would be put in check
        if(isKingUnderCheck(isWhite, board)) {
            // undo the move and throw error
            isWhite ? board.whitePawns = board.whitePawns | (1ULL << fromSquare) & ~(1ULL << toSquare) : board.blackPawns = board.blackPawns | (1ULL << fromSquare) & ~(1ULL << toSquare);
            board.allPieces = board.allPieces | (1ULL << fromSquare) & ~(1ULL << toSquare);

            // throw error
            throw LegalChessError("The move cannot be made. The King is under check or the piece is pinned.");
        }

        // update enpassant square
        board.enpassantSquare = isWhite ? fromSquare + 8 : fromSquare - 8;
    }
    // else if capture
    else if(abs(fromCol - toCol) == 1) {
        char capturedPiece;
        // enpassant capture
        if((1ULL << toSquare) & board.allPieces == 0) {
            if(board.enpassantSquare != toSquare) {
                // To-Do: throw error
                throw LegalChessError("The pawn attempts to capture an enpassant pawn. But the destination square doesn't match the enpassant square");
            }
            
            // update them move
            capturedPiece = isWhite ? 'p' : 'P';
            isWhite ? board.updateBlackPieceCount(capturedPiece, false, board.enpassantSquare - 8) : board.updateWhitePieceCount(capturedPiece, false, board.enpassantSquare + 8);

            isWhite ? board.whitePawns = board.whitePawns & ~(1ULL << fromSquare) | (1ULL << toSquare) : board.blackPawns = board.blackPawns & ~(1ULL << fromSquare) | (1ULL << toSquare);
            board.allPieces = board.allPieces & ~(1ULL << fromSquare) | (1ULL << toSquare);


            // To-Do: verify if the king would be put in check
            if(isKingUnderCheck(isWhite, board)) {
                // undo the move and throw error
                isWhite ? board.updateBlackPieceCount(capturedPiece, true, board.enpassantSquare - 8) : board.updateWhitePieceCount(capturedPiece, true, board.enpassantSquare + 8);

                isWhite ? board.whitePawns = board.whitePawns | (1ULL << fromSquare) & ~(1ULL << toSquare) : board.blackPawns = board.blackPawns | (1ULL << fromSquare) & ~(1ULL << toSquare);
                board.allPieces = board.allPieces | (1ULL << fromSquare) & ~(1ULL << toSquare);

                throw LegalChessError("The move cannot be made. The King is under check or the piece is pinned.");
            }

            if(board.enpassantSquare != 64) board.enpassantSquare = 64;
        }
        // normal capture
        else {
            // update the move
            capturedPiece = grid[toRow][toCol];
            isWhite ? board.updateBlackPieceCount(capturedPiece, false, toSquare) : board.updateWhitePieceCount(capturedPiece, false, toSquare);

            isWhite ? board.whitePawns = board.whitePawns & ~(1ULL << fromSquare) | (1ULL << toSquare) : board.blackPawns = board.blackPawns & ~(1ULL << fromSquare) | (1ULL << toSquare);
            board.allPieces = board.allPieces & ~(1ULL << fromSquare) | (1ULL << toSquare);


            // To-Do: verify if the king would be put in check
            if(isKingUnderCheck(isWhite, board)) {
                // undo the move and throw error
                isWhite ? board.updateBlackPieceCount(capturedPiece, true, toSquare) : board.updateWhitePieceCount(capturedPiece, true, toSquare);
                isWhite ? board.whitePawns = board.whitePawns | (1ULL << fromSquare) & ~(1ULL << toSquare) : board.blackPawns = board.blackPawns | (1ULL << fromSquare) & ~(1ULL << toSquare);
                board.allPieces = board.allPieces | (1ULL << fromSquare) & ~(1ULL << toSquare);

                throw LegalChessError("The move cannot be made. The King is under check or the piece is pinned.");
            }

            if(board.enpassantSquare != 64) board.enpassantSquare = 64;
        }
    }
    // single step
    else {
        // update the move
        isWhite ? board.whitePawns = board.whitePawns & ~(1ULL << fromSquare) | (1ULL << toSquare) : board.blackPawns = board.blackPawns & ~(1ULL << fromSquare) | (1ULL << toSquare);
        board.allPieces = board.allPieces & ~(1ULL << fromSquare) | (1ULL << toSquare);

        // To-Do: verify if the king would be put in check
        if(isKingUnderCheck(isWhite, board)) {
            // undo the move and throw error
            isWhite ? board.whitePawns = board.whitePawns | (1ULL << fromSquare) & ~(1ULL << toSquare) : board.blackPawns = board.blackPawns | (1ULL << fromSquare) & ~(1ULL << toSquare);
            board.allPieces = board.allPieces | (1ULL << fromSquare) & ~(1ULL << toSquare);

            throw LegalChessError("The move cannot be made. The King is under check or the piece is pinned.");
        }
    }   

    uint64_t opponentKing = isWhite ? board.blackKing : board.whiteKing;

    // a pawn move which is not a promotion can only give a direct check or discovered check but not both
    if(toCol + 1 < 8) {
        int attackSquare1 = (isWhite ? toRow + 1 : toRow - 1)*8 + toCol + 1;

        if(opponentKing & (1ULL << attackSquare1)) return CheckType::DIRECT_CHECK;
    }

    if(toCol - 1 >= 0) {
        int attackSquare2 = (isWhite ? toRow + 1 : toRow - 1)*8 + toCol - 1;

        if(opponentKing & (1ULL << attackSquare2)) return CheckType::DIRECT_CHECK;
    }

    if(getPinDirection(!isWhite, fromSquare, board, true) != PinDirection::NONE) {
        return CheckType::DISCOVERED_CHECK;
    }

    return CheckType::NO_CHECK;
}


CheckType KnightMoveManager::handleMove(int fromSquare, int toSquare, int fromRow, int fromCol, int toRow, int toCol, bool isWhite, Board& board) {
    if((abs(toRow - fromRow) == 1 && abs(toCol - fromCol) == 2) || (abs(toRow - fromRow) == 2 && abs(toCol - fromCol) == 1)) {
        auto & grid = board.grid;
        char capturedPiece = grid[toRow][toCol];

        // update the move
        isWhite ? board.updateBlackPieceCount(capturedPiece, false, toSquare) : board.updateWhitePieceCount(capturedPiece, false, toSquare);

        isWhite ? board.whiteKnights = board.whiteKnights & ~(1ULL << fromSquare) | (1ULL << toSquare) : board.blackKnights = board.blackKnights & ~(1ULL << fromSquare) | (1ULL << toSquare);
        board.allPieces = board.allPieces & ~(1ULL << fromSquare) | (1ULL << toSquare);

        // To-Do: verify if the king would be put in check
        if(isKingUnderCheck(isWhite, board)) {
            // undo the move and throw error
            isWhite ? board.updateBlackPieceCount(capturedPiece, true, toSquare) : board.updateWhitePieceCount(capturedPiece, true, toSquare);

            isWhite ? board.whiteKnights = board.whiteKnights | (1ULL << fromSquare) & ~(1ULL << toSquare) : board.blackKnights = board.blackKnights | (1ULL << fromSquare) & ~(1ULL << toSquare);
            board.allPieces = board.allPieces | (1ULL << fromSquare) & ~(1ULL << toSquare);

            throw LegalChessError("The move cannot be made. The King is under check or the piece is pinned.");
        }

        // see if knight is checking the opponent king
        if(Precompute::knightAttacks[toSquare] & (isWhite ? board.blackKing : board.whiteKing)) {
            // pinDirection method can be used, if it is pinned, then a discovered check is present
            if(getPinDirection(!isWhite, fromSquare, board, true) != PinDirection::NONE) {
                return CheckType::DOUBLE_CHECK;
            }
            // if there is no discovered check
            return CheckType::DIRECT_CHECK;
        }


        return CheckType::NO_CHECK;
    }

    // throw error
    throw LegalChessError("Invalid move pattern for knight");
}


CheckType BishopMoveManager::handleMove(int fromSquare, int toSquare, int fromRow, int fromCol, int toRow, int toCol, bool isWhite, Board& board) {
    if(abs(toRow - fromRow) == abs(toCol - fromCol)) {
        auto & grid = board.grid;
        char capturedPiece = grid[toRow][toCol];

        // update the move
        isWhite ? board.updateBlackPieceCount(capturedPiece, false, toSquare) : board.updateWhitePieceCount(capturedPiece, false, toSquare);

        isWhite ? board.whiteBishops = board.whiteBishops & ~(1ULL << fromSquare) | (1ULL << toSquare) : board.blackBishops = board.blackBishops & ~(1ULL << fromSquare) | (1ULL << toSquare);
        board.allPieces = board.allPieces & ~(1ULL << fromSquare) | (1ULL << toSquare);

        // To-Do: verify if the king would be put in check
        if(isKingUnderCheck(isWhite, board)) {
            // undo the move and throw error
            isWhite ? board.updateBlackPieceCount(capturedPiece, true, toSquare) : board.updateWhitePieceCount(capturedPiece, true, toSquare);

            isWhite ? board.whiteBishops = board.whiteBishops | (1ULL << fromSquare) & ~(1ULL << toSquare) : board.blackBishops = board.blackBishops | (1ULL << fromSquare) & ~(1ULL << toSquare);
            board.allPieces = board.allPieces | (1ULL << fromSquare) & ~(1ULL << toSquare);

            throw LegalChessError("The move cannot be made. The King is under check or the piece is pinned.");
        }

        // Derive occupancy mask for this position
        int topLeftSquare = -(std::min(toRow, toCol) * 9) + toSquare, 
            bottomRightSquare = (std::min(8 - toRow - 1, 8 - toCol - 1) * 9) + toSquare, 
            topRightSquare = -(std::min((int)toRow, 8-toCol-1) * 7) + toSquare, 
            bottomLeftSquare = (std::min(8 - toRow - 1, (int)toCol) * 7) + toSquare;

        uint64_t allLegalMoves = Precompute::precomputedMasks[topLeftSquare][bottomRightSquare] | Precompute::precomputedMasks[bottomLeftSquare][topRightSquare];
        allLegalMoves &= ~(1ULL << toSquare);

        uint64_t occupancyMask = allLegalMoves & board.allPieces;

        uint16_t index = Precompute::compressedIndexOfOccupancy(occupancyMask, allLegalMoves);

        // see if bishop is checking the opponent king
        if(Precompute::bishopAttacks[toSquare][index] & (isWhite ? board.blackKing : board.whiteKing)) {
            // pinDirection method can be used, if it is pinned, then a discovered check is present
            if(getPinDirection(!isWhite, fromSquare, board, true) != PinDirection::NONE) {
                return CheckType::DOUBLE_CHECK;
            }
            // if there is no discovered check
            return CheckType::DIRECT_CHECK;
        }

        return CheckType::NO_CHECK;
    }

    // throw error
    throw LegalChessError("Invalid move pattern for bishop");
}


CheckType RookMoveManager::handleMove(int fromSquare, int toSquare, int fromRow, int toRow, int fromCol, int toCol, bool isWhite, Board& board) {
    if(fromRow == toRow || fromCol == toCol) {
        auto & grid = board.grid;
        char capturedPiece = grid[toRow][toCol];

        // update the move
        isWhite ? board.updateBlackPieceCount(capturedPiece, false, toSquare) : board.updateWhitePieceCount(capturedPiece, false, toSquare);

        isWhite ? board.whiteRooks = board.whiteRooks & ~(1ULL << fromSquare) | (1ULL << toSquare) : board.blackRooks = board.blackRooks & ~(1ULL << fromSquare) | (1ULL << toSquare);
        board.allPieces = board.allPieces & ~(1ULL << fromSquare) | (1ULL << toSquare);

        // To-Do: verify if the king would be put in check
        if(isKingUnderCheck(isWhite, board)) {
            // undo the move and throw error
            isWhite ? board.updateBlackPieceCount(capturedPiece, true, toSquare) : board.updateWhitePieceCount(capturedPiece, true, toSquare);

            isWhite ? board.whiteRooks = board.whiteRooks | (1ULL << fromSquare) & ~(1ULL << toSquare) : board.blackRooks = board.blackRooks | (1ULL << fromSquare) & ~(1ULL << toSquare);
            board.allPieces = board.allPieces | (1ULL << fromSquare) & ~(1ULL << toSquare);

            throw LegalChessError("The move cannot be made. The King is under check or the piece is pinned.");
        }

        // Derive occupancy mask for this position
        uint64_t allLegalMoves = Precompute::precomputedMasks[toRow*8][toRow*8 + 7] | Precompute::precomputedMasks[toCol][56+toCol];
        allLegalMoves &= ~(1ULL << toSquare);

        uint64_t occupancyMask = allLegalMoves & board.allPieces;

        uint16_t index = Precompute::compressedIndexOfOccupancy(occupancyMask, allLegalMoves);

        // see if rook is checking the opponent king
        if(Precompute::rookAttacks[toSquare][index] & (isWhite ? board.blackKing : board.whiteKing)) {
            // pinDirection method can be used, if it is pinned, then a discovered check is present
            if(getPinDirection(!isWhite, fromSquare, board, true) != PinDirection::NONE) {
                return CheckType::DOUBLE_CHECK;
            }
            // if there is no discovered check
            return CheckType::DIRECT_CHECK;
        }

        return CheckType::NO_CHECK;
    }

    // throw error
    throw LegalChessError("Invalid move pattern for rook");

}

CheckType QueenMoveManager::handleMove(int fromSquare, int toSquare, int fromRow, int toRow, int fromCol, int toCol, bool isWhite, Board& board) {
    if(fromRow == toRow || fromCol == toCol || abs(toRow - fromRow) == abs(toCol - fromCol)) {
        auto & grid = board.grid;
        char capturedPiece = grid[toRow][toCol];

        // update the move
        isWhite ? board.updateBlackPieceCount(capturedPiece, false, toSquare) : board.updateWhitePieceCount(capturedPiece, false, toSquare);

        isWhite ? board.whiteQueens = board.whiteQueens & ~(1ULL << fromSquare) | (1ULL << toSquare) : board.blackQueens = board.blackQueens & ~(1ULL << fromSquare) | (1ULL << toSquare);
        board.allPieces = board.allPieces & ~(1ULL << fromSquare) | (1ULL << toSquare);

        // To-Do: verify if the king would be put in check
        if(isKingUnderCheck(isWhite, board)) {
            // undo the move and throw error
            isWhite ? board.updateBlackPieceCount(capturedPiece, true, toSquare) : board.updateWhitePieceCount(capturedPiece, true, toSquare);

            isWhite ? board.whiteQueens = board.whiteQueens | (1ULL << fromSquare) & ~(1ULL << toSquare) : board.blackQueens = board.blackQueens | (1ULL << fromSquare) & ~(1ULL << toSquare);
            board.allPieces = board.allPieces | (1ULL << fromSquare) & ~(1ULL << toSquare);

            throw LegalChessError("The move cannot be made. The King is under check or the piece is pinned.");
        }

        int topLeftSquare = -(std::min(toRow, toCol) * 9) + toSquare, 
            bottomRightSquare = (std::min(8 - toRow - 1, 8 - toCol - 1) * 9) + toSquare, 
            topRightSquare = -(std::min((int)toRow, 8-toCol-1) * 7) + toSquare, 
            bottomLeftSquare = (std::min(8 - toRow - 1, (int)toCol) * 7) + toSquare;

        // Derive occupancy mask for this position
        uint64_t allLegalMoves = Precompute::precomputedMasks[toRow*8][toRow*8 + 7] | 
            Precompute::precomputedMasks[toCol][56 + toCol] | 
            Precompute::precomputedMasks[topLeftSquare][bottomRightSquare] | 
            Precompute::precomputedMasks[bottomLeftSquare][topRightSquare];

        allLegalMoves &= ~(1ULL << toSquare);

        uint64_t occupancyMask = allLegalMoves & board.allPieces;

        uint16_t index = Precompute::compressedIndexOfOccupancy(occupancyMask, allLegalMoves);

        // see if queen is checking the opponent king
        if((Precompute::bishopAttacks[toSquare][index] | Precompute::rookAttacks[toSquare][index]) & (isWhite ? board.blackKing : board.whiteKing)) {
            // pinDirection method can be used, if it is pinned, then a discovered check is present
            if(getPinDirection(!isWhite, fromSquare, board, true) != PinDirection::NONE) {
                return CheckType::DOUBLE_CHECK;
            }
            // if there is no discovered check
            return CheckType::DIRECT_CHECK;
        }

        return CheckType::NO_CHECK;
    }

    // throw error
    throw LegalChessError("Invalid move pattern for queen");

}

CheckType KingMoveManager::handleMove(int fromSquare, int toSquare, int fromRow, int toRow, int fromCol, int toCol, bool isWhite, Board& board) {
    if(abs(fromRow - toRow) <= 1 && abs(fromCol - toCol) <= 1) {
        auto & grid = board.grid;
        char capturedPiece = grid[toRow][toCol];

        // update the move
        isWhite ? board.updateBlackPieceCount(capturedPiece, false, toSquare) : board.updateWhitePieceCount(capturedPiece, false, toSquare);

        isWhite ? board.whiteKing = board.whiteKing & ~(1ULL << fromSquare) | (1ULL << toSquare) : board.blackKing = board.blackKing & ~(1ULL << fromSquare) | (1ULL << toSquare);
        board.allPieces = board.allPieces & ~(1ULL << fromSquare) | (1ULL << toSquare);

        // To-Do: verify if the king would be put in check
        if(isKingUnderCheck(isWhite, board)) {
            // undo the move and throw error
            isWhite ? board.updateBlackPieceCount(capturedPiece, true, toSquare) : board.updateWhitePieceCount(capturedPiece, true, toSquare);

            isWhite ? board.whiteKing = board.whiteKing | (1ULL << fromSquare) & ~(1ULL << toSquare) : board.blackKing = board.blackKing | (1ULL << fromSquare) & ~(1ULL << toSquare);
            board.allPieces = board.allPieces | (1ULL << fromSquare) & ~(1ULL << toSquare);

            throw LegalChessError("The move cannot be made. The King is under check or will be in check if it is moved.");
        }

        if(isWhite && !board.whiteKingMoved) {
            board.canWhiteKingShortCastle = board.canWhiteKingLongCastle = false;
            board.whiteKingMoved = true;
        }
        else if(!isWhite && !board.blackKingMoved) {
            board.canBlackKingLongCastle = board.canBlackKingShortCastle = false;
            board.blackKingMoved = true;
        }

        // King can only deliver a discovered check
        // pinDirection method can be used, if it is pinned, then a discovered check is present
        if(getPinDirection(!isWhite, fromSquare, board, true) != PinDirection::NONE) {
            return CheckType::DISCOVERED_CHECK;
        }

        return CheckType::NO_CHECK;
    }

    if(fromRow == toRow && abs(fromCol - toCol) == 2) {
        bool shortSide = fromCol - toCol == 2;
        handleKingCastle(shortSide, isWhite, board);

        int rookCol = shortSide ? fromCol - 1 : fromCol + 1, rookRow = fromRow;

        // Derive occupancy mask for this position
        uint64_t allLegalMoves = Precompute::precomputedMasks[rookRow*8][rookRow*8 + 7] | Precompute::precomputedMasks[rookCol][56+toCol];
        allLegalMoves &= ~(1ULL << toSquare);

        uint64_t occupancyMask = allLegalMoves & board.allPieces;

        uint16_t index = Precompute::compressedIndexOfOccupancy(occupancyMask, allLegalMoves);

        // see if rook is checking the opponent king. after a castle only direct check is possible
        if(Precompute::rookAttacks[shortSide ? fromSquare - 1 : fromSquare + 1][index] & (isWhite ? board.blackKing : board.whiteKing)) {
            return CheckType::DOUBLE_CHECK;
        }

        return CheckType::NO_CHECK;
    }

    // throw error
    throw LegalChessError("Invalid move pattern for king");
}


MoveManagerFactory::MoveManagerFactory() {
    m_PawnManager = std::make_shared<PawnMoveManager>();
    m_KnightManager = std::make_shared<KnightMoveManager>();
    m_BishopManager = std::make_shared<BishopMoveManager>();
    m_RookManager = std::make_shared<RookMoveManager>();
    m_QueenManager = std::make_shared<QueenMoveManager>();
    m_KingManager = std::make_shared<KingMoveManager>();
}

MoveManagerFactory& MoveManagerFactory::getReference() {
    static MoveManagerFactory factory;
    return factory;
}

std::shared_ptr<IMoveManager> MoveManagerFactory::getMoveManager(char piece) {
    if(piece == 'p' || 'P') return m_PawnManager;
    else if(piece == 'n' || piece == 'N') return m_KnightManager;
    else if(piece == 'b' || piece == 'B') return m_BishopManager;
    else if(piece == 'r' || piece == 'R') return m_RookManager;
    else if(piece == 'q' || piece == 'Q') return m_QueenManager;
    else return m_KingManager;
}

};