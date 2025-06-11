#include "Board.h"
#include "MoveManager.h"

#include <sstream>

namespace LC {

    class IMoveManager;
    class MoveManagerFactory;

    Board::Board(MoveManagerFactory& factory) : m_rMoveManagerFactory(factory) {
        whiteRooks = 129;
        whiteKnights = 66;
        whiteBishops = 36;
        whiteKing = 8;
        whiteQueens = 16;
        whitePawns = 255;
        whitePawns <<= 8;

        blackPawns = whitePawns << 40;
        blackRooks = whiteRooks << 56;
        blackKnights = whiteKnights << 56;
        blackBishops = whiteBishops << 56;
        blackKing = whiteKing << 56;
        blackQueens = whiteQueens << 56;

        allPieces = whiteRooks | whiteKnights | whiteBishops | whiteKing | whiteQueens | whitePawns | blackPawns | blackRooks | blackKnights | blackBishops | blackKing | blackQueens;

        // Black Pieces
        // King Row
        grid[7][0] = grid[7][7] = BLACK_ROOK;
        grid[7][1] = grid[7][6] = BLACK_KNIGHT;
        grid[7][2] = grid[7][5] = BLACK_BISHOP;
        grid[7][3] = BLACK_KING;
        grid[7][4] = BLACK_QUEEN;
        

        // Pawn Row
        for(auto &piece : grid[1]) piece = BLACK_PAWN;

        // WhitePieces
        // Pawn Row
        for(auto &piece : grid[6]) piece = WHITE_PAWN;

        // King Row
        grid[0][0] = grid[0][7] = WHITE_ROOK;
        grid[0][1] = grid[0][6] = WHITE_KNIGHT;
        grid[0][2] = grid[0][5] = WHITE_BISHOP;
        grid[0][3] = WHITE_KING;
        grid[0][4] = WHITE_QUEEN;

        isWhiteTurn = true;
        isCheckMate = isStalemate = isDrawBy50MoveRule = isDrawByInsufficientMaterial = isDrawByRepitition = false;
        canWhiteKingShortCastle = canBlackKingShortCastle = canWhiteKingLongCastle = canBlackKingLongCastle = true;
        whiteKingMoved = blackKingMoved = false;

        m_pZobrist = std::make_unique<Zobrist>();
    }

    // Board::Board(std::string fen, MoveManagerFactory& factory) {
    //     whiteRooks = whiteKnights = whiteBishops = whiteKing = whiteQueens = whitePawns = 0;

    //     blackPawns = blackRooks = blackKnights  = blackBishops  = blackKing = blackQueens = 0;

    //     allPieces = 0;

    //     canWhiteKingLongCastle = canWhiteKingShortCastle = canBlackKingLongCastle = canBlackKingShortCastle = false;
    //     blackKingMoved = whiteKingMoved = true;

    //     isWhiteTurn = true;
    //     enpassantSquare = 64;

    //     // load board state from fen
    //     int fenIndex = 0;

    //     for(int i = 7; i>=0; i--) {
    //         int emptyCount = 0;

    //         if(fenIndex >= fen.length() || (i != 7 && fen[fenIndex] != '/')) throw LegalChessError("Invalid FEN String: " + fen);
    //         fenIndex++;

    //         for(int j = 7; j>=0; j--) {
    //             if(fenIndex >= fen.length()) throw LegalChessError("Invalid FEN String: " + fen);

    //             if(emptyCount > 0) {
    //                 grid[i][j] = EMPTY;
    //                 emptyCount--;
    //             }
    //             else if(fen[fenIndex] >= '1' && fen[fenIndex] <= '8') {
    //                 emptyCount = fen[fenIndex] - '0';
    //                 j++;
    //                 fenIndex++;
    //             }
    //             else {
    //                 uint64_t& pieceBitBoard = getPieceBitBoard(fen[fenIndex]);
    //                 int square = (i*8 + j);
    //                 pieceBitBoard |= (1ULL << square);
    //                 grid[i][j] = fen[fenIndex];

    //                 fenIndex++;
    //             }
    //         }
    //     }

    //     if(fen[fenIndex] != ' ') throw LegalChessError("Invalid FEN String: " + fen);

    //     std::stringstream remainingFEN(fen.substr(fenIndex + 1));
    //     std::string turn; remainingFEN >> turn;

    //     if(!remainingFEN) throw LegalChessError("Invalid FEN String. Not enough fields: " + fen);

    //     if(turn == "b") isWhiteTurn = false;

    //     std::string castling; remainingFEN >> castling;
    //     if(!remainingFEN) throw LegalChessError("Invalid FEN String. Not enough fields: " + fen);

    //     if(castling.length() > 4) throw LegalChessError("Invalid FEN String. Invalid Castling Rights: " + castling);

    //     if(castling != "-") {
    //         for(auto& c : castling) {
    //             if(c == 'K') canWhiteKingShortCastle = true, whiteKingMoved = false;
    //             else if(c == 'Q') canWhiteKingLongCastle = true, whiteKingMoved = false;
    //             else if(c == 'k') canBlackKingShortCastle = true, blackKingMoved = false;
    //             else if(c == 'q') canBlackKingLongCastle = true, blackKingMoved = false;
    //             else throw LegalChessError("Invalid FEN String. Invalid Castling Rights: " + castling);
    //         }
    //     }

    //     std::string enpassantSq;
    //     remainingFEN >> enpassantSq;
    //     if(!remainingFEN) throw LegalChessError("Invalid FEN String. Not enough fields: " + fen);

    //     if(enpassantSq.length() != 2 || enpassantSq[0] < 'a' || enpassantSq[0] > 'h' || enpassantSq[1] < '1' || enpassantSq[1] > '8') throw LegalChessError("Invalid enpassant square in FEN: " + enpassantSq);

    //     enpassantSquare = (enpassantSq[1] - '1') * 8 + 'h' - enpassantSq[0];

    //     std::string halfMoves;
    //     remainingFEN >> halfMoves;
    //     if(!remainingFEN) throw LegalChessError("Invalid FEN String. Not enough fields: " + fen);

    //     movesWithoutCaptureAndPawns = stoi(halfMoves);

    //     std::string fullMoves;
    //     remainingFEN >> fullMoves;
    //     if(!remainingFEN) throw LegalChessError("Invalid FEN String. Not enough fields: " + fen);

    //     moveNumber = isWhiteTurn ? stoi(fullMoves) * 2 - 2 : stoi(fullMoves) * 2 - 1;
    // }


    void Board::addMoveToHistory(std::string move) {
        moveHistory.push_back(' ');
        moveHistory += move;
        moveHistory.push_back(' ');
    }


    void Board::move(int fromRow, int fromCol, int toRow, int toCol, int fromSquare, int toSquare) {
        char pieceOnBoard = grid[fromRow][fromCol];

        if(isPieceWhite(pieceOnBoard) != isWhiteTurn) {
            // To-Do: throw error
            throw LegalChessError("Piece color and player's turn don't match");
        }

        if(grid[toRow][toCol] != EMPTY && isWhiteTurn == isPieceWhite(grid[toRow][toCol])) {
            // To-Do: throw eror
            throw LegalChessError("The destination square is occupied by same color piece");
        }

        // see if there are any obstructions in the path if it is a sliding piece
        if(pieceOnBoard != WHITE_KNIGHT && pieceOnBoard != BLACK_KNIGHT) {
            uint64_t pathMask = Precompute::precomputedMasks[fromSquare][toSquare];

            pathMask &= ~(1ULL << fromSquare);
            pathMask &= ~(1ULL << toSquare);

            if(pathMask & allPieces) {
                // To-Do: throw error
                throw LegalChessError("The path from source square to destination square has obstructions");
            }
        }

        CheckType check =  m_rMoveManagerFactory.getMoveManager(pieceOnBoard)->handleMove(fromSquare, toSquare, fromRow, fromCol, toRow, toCol, isWhiteTurn, *this);

        if(pieceOnBoard == 'p' || pieceOnBoard == 'P' || isPieceWhite(grid[toRow][toCol]) != isWhiteTurn) movesWithoutCaptureAndPawns = 0;
        else movesWithoutCaptureAndPawns++;

        moveNumber++;

        grid[toRow][toCol] = grid[fromRow][fromCol];
        grid[fromRow][fromCol] = EMPTY;

        isWhiteTurn = !isWhiteTurn;
        if(pieceOnBoard != 'p' || pieceOnBoard != 'P') enpassantSquare = 64;

        int castlingRights = 0;
        if(canWhiteKingShortCastle) castlingRights |= (1);
        if(canWhiteKingLongCastle) castlingRights |= (1 << 1);
        if(canBlackKingShortCastle) castlingRights |= (1 << 2);
        if(canBlackKingLongCastle) castlingRights |= (1 << 3);

        uint64_t position = m_pZobrist->computeHash(grid, isWhiteTurn, castlingRights, enpassantSquare == 64 ? -1 : enpassantSquare % 8);

        positionToFreq[position]++;

        // castle
        if(pieceOnBoard == 'k' || pieceOnBoard == 'K' && abs(fromCol - toCol) == 2) {
            calculateMoveResult(isWhiteTurn ? 'R' : 'r', fromCol - toCol == 2 ? fromSquare - 1 : fromSquare + 1, check, position);
        }
        // normal move
        else {
            calculateMoveResult(pieceOnBoard, toSquare, check, position);
        }
        
    }


    void Board::promote(char newPiece, int fromRow, int fromCol, int toRow, int toCol, int fromSquare, int toSquare) {
        char pieceOnBoard = grid[fromRow][fromCol];

        if(isWhiteTurn) {
            pieceOnBoard = isPieceWhite(pieceOnBoard) ? pieceOnBoard : pieceOnBoard - 32;
        } 
        else {
            pieceOnBoard = isPieceWhite(pieceOnBoard) ? pieceOnBoard + 32 : pieceOnBoard;
        }

        if(pieceOnBoard != 'p' || pieceOnBoard != 'P') {
            // To-Do: throw error
            throw LegalChessError("The piece on the source square is not a pawn. Only pawns can be promoted.");
        }

        if(grid[toRow][toCol] != EMPTY && isWhiteTurn == isPieceWhite(grid[toRow][toCol])) {
            // To-Do: throw eror
            throw LegalChessError("The promotion square is occupied by same color piece");
        }

        m_rMoveManagerFactory.getMoveManager(pieceOnBoard)->handleMove(fromSquare, toSquare, fromRow, fromCol, toRow, toCol, isWhiteTurn, *this);


        movesWithoutCaptureAndPawns = 0;
        moveNumber++;
        
        grid[toRow][toCol] = isWhiteTurn ? (isPieceWhite(newPiece) ? newPiece : newPiece - 32) : (isPieceWhite(newPiece) ? newPiece + 32 : newPiece);
        grid[fromRow][fromCol] = EMPTY;

        isWhiteTurn = !isWhiteTurn;
        enpassantSquare = 64;

        int castlingRights = 0;
        if(canWhiteKingShortCastle) castlingRights |= (1);
        if(canWhiteKingLongCastle) castlingRights |= (1 << 1);
        if(canBlackKingShortCastle) castlingRights |= (1 << 2);
        if(canBlackKingLongCastle) castlingRights |= (1 << 3);

        positionToFreq[m_pZobrist->computeHash(grid, isWhiteTurn, castlingRights, enpassantSquare == 64 ? -1 : enpassantSquare % 8)]++;
    }


    void Board::updateBlackPieceCount(char piece, bool inc, int square) {
        if(piece == '.') return;

        if(piece == 'p') inc ? blackPawns |= (1ULL << square) : blackPawns &= ~(1ULL << square) , inc ? allPieces |= (1ULL << square) : allPieces &= ~(1ULL << square);
        else if(piece == 'n') inc ? blackKnights |= (1ULL << square) : blackKnights &= ~(1ULL << square) , inc ? allPieces |= (1ULL << square) : allPieces &= ~(1ULL << square);
        else if(piece == 'b') inc ? blackBishops |= (1ULL << square) : blackBishops &= ~(1ULL << square) , inc ? allPieces |= (1ULL << square) : allPieces &= ~(1ULL << square);
        else if(piece == 'r') inc ? blackRooks |= (1ULL << square) : blackRooks &= ~(1ULL << square) , inc ? allPieces |= (1ULL << square) : allPieces &= ~(1ULL << square);
        else if(piece == 'q') inc ? blackQueens |= (1ULL << square) : blackQueens &= ~(1ULL << square) , inc ? allPieces |= (1ULL << square) : allPieces &= ~(1ULL << square);
    }

    void Board::updateWhitePieceCount(char piece, bool inc, int square) {
        if(piece == '.') return;

        if(piece == 'P') inc ? whitePawns |= (1ULL << square) : whitePawns &= ~(1ULL << square) , inc ? allPieces |= (1ULL << square) : allPieces &= ~(1ULL << square);
        else if(piece == 'N') inc ? whiteKnights |= (1ULL << square) : whiteKnights &= ~(1ULL << square) , inc ? allPieces |= (1ULL << square) : allPieces &= ~(1ULL << square);
        else if(piece == 'B') inc ? whiteBishops |= (1ULL << square) : whiteBishops &= ~(1ULL << square) , inc ? allPieces |= (1ULL << square) : allPieces &= ~(1ULL << square);
        else if(piece == 'R') inc ? whiteRooks |= (1ULL << square) : whiteRooks &= ~(1ULL << square) , inc ? allPieces |= (1ULL << square) : allPieces &= ~(1ULL << square);
        else if(piece == 'Q') inc ? whiteQueens |= (1ULL << square) : whiteQueens &= ~(1ULL << square) , inc ? allPieces |= (1ULL << square) : allPieces &= ~(1ULL << square);
    }

    void Board::calculateMoveResult(char pieceMoved, int pieceSquare, CheckType check, uint64_t position) {
        // draw by repitition 
        if(positionToFreq[position] == 3) {
            isDrawByRepitition = true;
            return;
        }

        if(check == CheckType::NO_CHECK) {
            // find if it is draw by insufficient material
            // if a pawn is present or a rook is present or a queen is present not insufficient
            if((whitePawns | blackPawns | whiteRooks | blackRooks | whiteQueens | blackQueens) == 0) {
                if((__builtin_popcountll(blackBishops | blackKnights) <= 1 && __builtin_popcountll(whiteBishops | whiteKnights) == 0) || 
                    __builtin_popcountll(blackBishops | blackKnights) == 0 && __builtin_popcountll(whiteBishops | whiteKnights) <= 1) {
                    result = 'd';
                    isDrawByInsufficientMaterial = true;
                    return;
                }

                if(__builtin_popcountll(blackBishops) == 1 && __builtin_popcountll(whiteBishops) == 1) {
                    int sq1 = __builtin_ctz(blackBishops), sq2 = __builtin_ctz(whiteBishops);
                    int rank1 = sq1/8, file1 = sq1%8, rank2 = sq2/8, file2 = sq2%8;

                    int res1 = abs((rank2 % 2 + file2 % 2) - (rank1 % 2 + file1 % 2));
                    
                    // if both bishops are of same color
                    if(res1 == 2 || res1 == 0) {
                        result = 'd';
                        isDrawByInsufficientMaterial = true;
                        return;
                    }
                }
                
            }


            // stalemate
            // opponentKingPosition
            int kingSquare = isWhiteTurn ? blackKing : whiteKing;
            int sqRank = kingSquare / 8, sqFile = kingSquare % 8;

            // get friendly pieces position
            uint64_t friendPiecesOfOppKing = isWhiteTurn ? (blackPawns | blackKnights | blackBishops | blackRooks | blackQueens) : (whitePawns | whiteKnights | whiteBishops | whiteRooks | whiteQueens);

            // the opp king can only move to a different square
            // caluclate the attacking color piece attacks
            uint64_t attacks = IMoveManager::generateLegalAttacks(isWhiteTurn, false, *this);


            // iterate all possible king squares
            for(auto & offSets : Precompute::knightMoveOffsets) {
                int8_t newRank = sqRank + offSets[0], newFile = sqFile + offSets[1];

                if(newRank >= 0 && newRank < 8 && newFile >= 0 && newFile < 8) {
                    int newSquare = newRank*8 + newFile;

                    if(friendPiecesOfOppKing & (1ULL << newSquare) == 0 && (1ULL << newSquare & attacks == 0)) return;
                }
            }

            // if a sliding piece exists it is not a stalemate, find if opponent has a sliding piece
            if(isWhiteTurn ? (blackBishops | blackRooks || blackQueens) : (whiteBishops | whiteRooks | whiteQueens) == 0) {
                // for all the knights and pawns, see if any of them can move, i.e they are not pinned and pawn can move either move or capture
                uint64_t knightsBoard = isWhiteTurn ? blackKnights : whiteKnights;

                while(knightsBoard) {
                    int knightSquare = __builtin_ctzll(knightsBoard);
                    knightsBoard &= knightsBoard - 1;

                    if(IMoveManager::getPinDirection(!isWhiteTurn, knightSquare, *this, false) == PinDirection::NONE) return;
                }

                uint64_t pawnsBoard = isWhiteTurn ? blackPawns : whitePawns;

                while(pawnsBoard) {
                    int pawnSquare = __builtin_ctzll(pawnsBoard);
                    pawnsBoard &= pawnsBoard - 1;

                    if(IMoveManager::getPinDirection(!isWhiteTurn, pawnSquare, *this, false) == PinDirection::NONE) {
                        // see if next square is free or there is a piece in attack proximity
                        int nextSquare = isWhiteTurn ? pawnSquare - 8 : pawnSquare + 8;
                        int curFile = pawnSquare % 8, newRank = nextSquare / 8;
                        int newFile1 = curFile + 1, newFile2 = curFile - 1;

                        uint64_t attackingColorPieces = isWhiteTurn ? (whitePawns | whiteBishops | whiteKnights | whiteRooks | whiteQueens) : (blackPawns | blackBishops | blackKnights | blackRooks | blackQueens);

                        if( (allPieces & (1ULL << nextSquare)  == 0) || (newFile1 < 8 && (attackingColorPieces & (1ULL << (newRank*8 + newFile1))) == 1) ||  newFile1 < 8 && (attackingColorPieces & (1ULL << (newRank*8 + newFile1))) == 1) return;
                    }
                }

                result = 'd';
                isStalemate = true;
                return;
            }
        }
        
        
        // opponentKingPosition
        int kingSquare = isWhiteTurn ? blackKing : whiteKing;
        int sqRank = kingSquare / 8, sqFile = kingSquare % 8;

        // get friendly pieces position
        uint64_t friendPiecesOfOppKing = isWhiteTurn ? (blackPawns | blackKnights | blackBishops | blackRooks | blackQueens) : (whitePawns | whiteKnights | whiteBishops | whiteRooks | whiteQueens);

        // the opp king can only move to a different square
        // caluclate the attacking color piece attacks
        uint64_t attacks = IMoveManager::generateLegalAttacks(isWhiteTurn, false, *this);

        // iterate all possible king squares
        for(auto & offSets : Precompute::knightMoveOffsets) {
            int8_t newRank = sqRank + offSets[0], newFile = sqFile + offSets[1];

            if(newRank >= 0 && newRank < 8 && newFile >= 0 && newFile < 8) {
                int newSquare = newRank*8 + newFile;

                if(friendPiecesOfOppKing & (1ULL << newSquare) == 0 && (1ULL << newSquare & attacks == 0)) return;
            }
        }

        // if double check it is a checkmate
        if(check == CheckType::DOUBLE_CHECK) {
            isCheckMate = true;
            result = isWhiteTurn ? 'w' : 'b';
            return;
        }
        
        // find opponent piece attacks to know if they can defend their king
        uint64_t opponentAttacks = IMoveManager::generateLegalAttacks(!isWhiteTurn, true, *this);
        

        // if the checking piece is knight, killing it is the only way
        if(check == CheckType::DIRECT_CHECK && (pieceMoved == 'n' || pieceMoved == 'N')) {
            if(opponentAttacks & (1ULL << pieceSquare) == 0) {
                isCheckMate = true;
                result = isWhiteTurn ? 'w' : 'b';
            }
            
            return;
        }


        // if it is a discovered check, find the checking piece square 
        int checkingPieceSquare = check == CheckType::DIRECT_CHECK ? pieceSquare : m_discoveryCheckSquare; // get from Pin check method

        uint8_t rangeMask = Precompute::precomputedMasks[kingSquare][checkingPieceSquare];
        rangeMask &= ~(1ULL << kingSquare);

        // find the range mask from king square to checking piece square and see if any opponent piece can defend
        if(opponentAttacks & rangeMask == 0) {
            isCheckMate = true;
            result = isWhiteTurn ? 'w' : 'b';
        }

        // if not checkmate, might be draw by 50 half moves
        if(movesWithoutCaptureAndPawns == 50) {
            result = 'd';
            isDrawBy50MoveRule = true;
            return;
        }
    }

    bool Board::isChecked(bool white) {   
        return white ? (blackKing & IMoveManager::generateLegalAttacks(false, false, *this)) : (whiteKing & IMoveManager::generateLegalAttacks(true, false, *this));
    }

    uint64_t& Board::getPieceBitBoard(char piece) {
        if(isPieceWhite(piece)) {
            if(piece == 'P') return whitePawns;
            if(piece == 'N') return whiteKnights;
            if(piece == 'B') return whiteBishops;
            if(piece == 'R') return whiteRooks;
            if(piece == 'Q') return whiteQueens;
            if(piece == 'K') return whiteKing;

            throw LegalChessError("Invalid piece provided in FEN string: " + std::string(1, piece));
        }
       
        if(piece == 'P') return whitePawns;
        if(piece == 'N') return whiteKnights;
        if(piece == 'B') return whiteBishops;
        if(piece == 'R') return whiteRooks;
        if(piece == 'Q') return whiteQueens;
        if(piece == 'K') return whiteKing;

        throw LegalChessError("Invalid piece provided in FEN string: " + std::string(1, piece));
    }

    std::string Board::getFENString() {
        std::string fenString = "";

        for(int i = 7; i >= 0; i--) {
            int emptyCount = 0;
            for(int j = 7; j >= 0; j--) {
                if(grid[i][j] == EMPTY) emptyCount++;
                else {
                    if(emptyCount) fenString.push_back('0' + emptyCount);
                    fenString.push_back(grid[i][j]);
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
        fenString += std::to_string(movesWithoutCaptureAndPawns);

        fenString.push_back(' ');
        fenString += std::to_string(moveNumber/2 + 1);
    }

    std::vector<std::vector<char>> Board::getBoard() {
        std::vector<std::vector<char>> board(8, std::vector<char>(8));

        for(int i = 0; i<8; i++) {
            for(int j = 0; j<8; j++) {
                board[i][j] = grid[i][j];
            }
        }

        return board;
    } 

};