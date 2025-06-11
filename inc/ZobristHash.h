#include <array>
#include <random>
#include <cstdint>
#include <iostream>

namespace LC {

class Zobrist {
public:
    static constexpr int NUM_SQUARES = 64;
    static constexpr int NUM_PIECE_TYPES = 12; // P, N, B, R, Q, K, p, n, b, r, q, k

    std::array<std::array<uint64_t, NUM_SQUARES>, NUM_PIECE_TYPES> pieceHash;
    std::array<uint64_t, 16> castlingHash; // 16 combinations of castling rights
    std::array<uint64_t, 8> enPassantHash; // Files a-h (if applicable)
    uint64_t sideToMoveHash;

    Zobrist() {
        initRandomKeys();
    }

    // Call this once to initialize the random hash values
    void initRandomKeys() {
        std::mt19937_64 rng(0xABCDEF1234567890); // fixed seed for reproducibility
        std::uniform_int_distribution<uint64_t> dist;

        for (auto& pieceArray : pieceHash) {
            for (auto& square : pieceArray) {
                square = dist(rng);
            }
        }

        for (auto& val : castlingHash) {
            val = dist(rng);
        }

        for (auto& val : enPassantHash) {
            val = dist(rng);
        }

        sideToMoveHash = dist(rng);
    }

    // Compute the Zobrist hash for a given position
    uint64_t computeHash(const char board[8][8], bool whiteToMove, int castlingRights, int enPassantFile) {
        uint64_t hash = 0;

        for (int i = 0; i<8; i++) {
            for(int j = 0; j<8; j++) {    
                int square = i*8 + j;            
                char piece = board[i][j];
                if (piece == '.') continue; // empty square

                int index = pieceToIndex(piece);
                if (index != -1) {
                    hash ^= pieceHash[index][square];
                }
            }
        }

        if (whiteToMove)
            hash ^= sideToMoveHash;

        if (castlingRights >= 0 && castlingRights < 16)
            hash ^= castlingHash[castlingRights];

        if (enPassantFile >= 0 && enPassantFile < 8)
            hash ^= enPassantHash[enPassantFile];

        return hash;
    }

private:
    // Map piece characters to 0–11 index
    int pieceToIndex(char c) {
        switch (c) {
            case 'P': return 0;
            case 'N': return 1;
            case 'B': return 2;
            case 'R': return 3;
            case 'Q': return 4;
            case 'K': return 5;
            case 'p': return 6;
            case 'n': return 7;
            case 'b': return 8;
            case 'r': return 9;
            case 'q': return 10;
            case 'k': return 11;
            default: return -1;
        }
    }
};

}
