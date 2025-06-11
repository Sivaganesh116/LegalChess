#ifndef __Precompute_h__
#define __Precompute_h__

#include <cstdint>
#include <cmath>
#include <algorithm>
#include <vector>

class Precompute {
public:
    Precompute() {
        // compute masks from each square as the source
        for(int srcSquare = 0; srcSquare < 64; srcSquare++) {

            int rank = (srcSquare / 8);
            int file = srcSquare % 8;

            // In forward and backward
            uint64_t rankMask = 1ULL << srcSquare;

            // same source and destination
            precomputedMasks[srcSquare][srcSquare] = rankMask;

            for(int rankSquare = srcSquare+1; rankSquare < (srcSquare / 8 + 1) * 8; rankSquare++) {
                rankMask |= 1ULL << rankSquare;

                precomputedMasks[srcSquare][rankSquare] = rankMask;
                precomputedMasks[rankSquare][srcSquare] = rankMask;
            }

            // In upward and downward
            uint64_t fileMask = 1ULL << srcSquare;

            for(int fileSquare = srcSquare+8; fileSquare < 64; fileSquare += 8) {
                fileMask |= 1ULL << fileSquare;

                precomputedMasks[srcSquare][fileSquare] = fileMask;
                precomputedMasks[fileSquare][srcSquare] = fileMask;
            }

            // In north-east and south-west
            uint64_t rightDiagMask = 1ULL << srcSquare;

            int numSquares = std::min(8-file-1, 8-rank-1);
            int rightDiagSquare = srcSquare;

            while(numSquares--) {
                rightDiagSquare += 9;

                rightDiagMask |= 1ULL << rightDiagSquare;

                precomputedMasks[srcSquare][rightDiagSquare] = rightDiagMask;
                precomputedMasks[rightDiagSquare][srcSquare] = rightDiagMask;
            }

            // In north-west and south-east
            uint64_t leftDiagMask = 1ULL << srcSquare;

            numSquares = std::min(file, 8-rank-1);
            int leftDiagSquare = srcSquare;

            while(numSquares--) {
                leftDiagSquare += 7;

                leftDiagMask |= 1ULL << leftDiagSquare;

                precomputedMasks[srcSquare][leftDiagSquare] = leftDiagMask;
                precomputedMasks[leftDiagSquare][srcSquare] = leftDiagMask;
            }            
        }


        // compute attack masks for rooks with all possible occupancy configs

        // find all configurations for rank 1, for all 8 squares, from left and right, then shift by 8 for all configs of a square to get the relevant configs for second rank and so on;
        std::vector<std::vector<uint64_t>> rankConfigsFromLeft(9), rankConfigsFromRight(9);
        rankConfigsFromLeft[0] = {0};
        rankConfigsFromLeft[1] = {0, 1};

        for(int i = 2; i<9; i++) {
            for(auto config : rankConfigsFromLeft[i-1]) {
                uint64_t pick = config | 1ULL << (i-1);
                
                rankConfigsFromLeft[i].push_back(pick);
                rankConfigsFromLeft[i].push_back(config);
            }
        }

        rankConfigsFromRight[8] = {0};
        rankConfigsFromRight[7] = {1ULL << 7, 0};
        for(int i = 6; i>=0; i--) {
            for(auto config : rankConfigsFromRight[i+1]) {
                uint64_t pick = config | 1ULL << i;

                rankConfigsFromRight[i].push_back(pick);
                rankConfigsFromRight[i].push_back(config);
            }
        }


        // find all configurations for a file 1, for all 8 squares, from top and bottom, then shift by 1 for all configs of a square to get the relevant configs for second file and so on;
        std::vector<std::vector<uint64_t>> fileConfigsFromTop(9), fileConfigsFromBottom(9);
        fileConfigsFromTop[0] = {0};
        fileConfigsFromTop[1] = {0, 1};

        for(int j = 2; j<9; j++) {
            for(auto config : fileConfigsFromTop[j-1]) {
                uint64_t pick = config | 1ULL << ((j-1)*8);

                fileConfigsFromTop[j].push_back(pick);
                fileConfigsFromTop[j].push_back(config);
            }
        }

        fileConfigsFromBottom[8] = {0};
        fileConfigsFromBottom[7] = {1ULL << 56, 0};

        for(int j = 6; j>=0; j--) {
            for(auto config : fileConfigsFromBottom[j+1]) {
                uint64_t pick = config | 1ULL << (j*8);

                fileConfigsFromBottom[j].push_back(pick);
                fileConfigsFromBottom[j].push_back(config);
            }
        }

        
        // rook can be in 64 squares
        rookAttacks.resize(64);

        for(int sq = 0; sq < 64; sq++) {
            int rank = sq / 8;
            int file = sq % 8;

            int rankShift = rank * 8, fileShift = file;

            size_t totalConfigs = rankConfigsFromLeft[file].size() * rankConfigsFromRight[file+1].size() * fileConfigsFromTop[rank].size() * fileConfigsFromBottom[rank+1].size();
            rookAttacks[sq].resize(totalConfigs+1);

            for(auto leftConfig : rankConfigsFromLeft[file]) {
                for(auto rightConfig : rankConfigsFromRight[file + 1]) {
                    for(auto topConfig : fileConfigsFromTop[rank]) {
                        for(auto bottomConfig : fileConfigsFromBottom[rank+1]) {
                            uint64_t occupancyMaskForSquare = (leftConfig << rankShift) | (rightConfig << rankShift) | (topConfig << fileShift) | (bottomConfig << fileShift);

                            // calculate moves based on the occupancy. trace the rays on left, right, top and bottom to find first blocker.
                            uint64_t attacksMask = 0;

                            int leftSq = sq - 1, rightSq = sq + 1, topSq = sq - 8, bottomSq = sq + 8;
                           
                            // left
                            while(leftSq >= rank * 8 && (occupancyMaskForSquare & (1ULL << leftSq) == 0)) {
                                attacksMask |= (1ULL << leftSq);

                                if(occupancyMaskForSquare & (1ULL << leftSq)) {
                                    break;  // first blocker included, stop ray here
                                }

                                leftSq--;
                            }

                            // right
                            while(rightSq < (rank + 1) * 8 && (occupancyMaskForSquare & (1ULL << rightSq) == 0)) {
                                attacksMask |= (1ULL << rightSq);

                                if(occupancyMaskForSquare & (1ULL << rightSq)) {
                                    break;
                                }

                                rightSq++;
                            }

                            // top 
                            while(topSq >= 0 && (occupancyMaskForSquare & (1ULL << topSq) == 0)) {
                                attacksMask |= (1ULL << topSq);
                                
                                if(occupancyMaskForSquare & (1ULL << topSq)) {
                                    break;
                                }

                                topSq -= 8;
                            }

                            // bottom
                            while(bottomSq < 64 && (occupancyMaskForSquare & (1ULL << bottomSq) == 0)) {
                                attacksMask |= (1ULL << bottomSq);

                                if(occupancyMaskForSquare & (1ULL << bottomSq)) {
                                    break;
                                }

                                bottomSq += 8;
                            }

                            // compress the occupancy mask
                            uint16_t index = 0;

                            uint64_t movesMask = (sq-1 >= rank*8 ? precomputedMasks[rank*8][sq-1] : 0) | (sq+1 <= rank*8+7 ? precomputedMasks[sq+1][rank*8 + 7] : 0) | (sq-8 >= file ? precomputedMasks[sq-8][file] : 0) | (sq+8 < 64 ? precomputedMasks[sq+8][56+file] : 0);
                            int bitPos = 0;

                            while(movesMask) {
                                int firstSetBit = __builtin_ctzll(movesMask);
                                if((1ULL << firstSetBit) & occupancyMaskForSquare) {
                                    index |= 1ULL << bitPos;
                                }

                                movesMask &= movesMask-1;
                                bitPos++;
                            }


                            rookAttacks[sq][index] = attacksMask;
                        }
                    }
                }
            }  
        }


        // for all squares a bishop can exist
        for(int sq = 0; sq < 64; sq++) {
            int rank = sq / 8;
            int file = sq % 8;

            uint64_t movesMask = 0;

            // topLeft
            int numSquares = std::min(rank, file);
            int topLeftCorner = sq + (numSquares * -9);

            movesMask |= sq-9 >= topLeftCorner ? precomputedMasks[sq - 9][topLeftCorner] : 0;

            // topRight
            numSquares = std::min(8 - rank - 1, 8 - file - 1);
            int topRightCorner = sq + (numSquares * -7);

            movesMask |= sq-7 >= topRightCorner ? precomputedMasks[sq - 7][topRightCorner] : 0;

            // bottomLeft
            numSquares = std::min(8 - rank - 1, file);
            int bottomLeftCorner = sq + (numSquares * 7);

            movesMask |= sq+7 <= bottomLeftCorner ? precomputedMasks[sq+7][bottomLeftCorner] : 0;

            // bottomRight
            numSquares = std::min(rank, 8 - file - 1);
            int bottomRightCorner = sq + (numSquares * 9);

            movesMask |= sq+9 <= bottomRightCorner ? precomputedMasks[sq+9][bottomRightCorner] : 0;

            int16_t totalConfigs = pow(2, __builtin_popcountll(movesMask));

            for(int config = 0; config < totalConfigs; config++) {
                uint64_t occupancyMask = 0;
                int bitPos = 0;

                while(movesMask) {
                    int firstSetBit = __builtin_ctzll(movesMask);

                    if(config & (1 << bitPos)) {
                        occupancyMask |= (1ULL << firstSetBit);
                    }

                    bitPos++;
                    movesMask &= movesMask-1;
                }

                // calculate all attacking squares till first blocker
                int topLeft = sq - 9, bottomRight = sq + 9, topRight = sq - 7, bottomLeft = sq + 7;
                uint64_t attacksMask = 0;

                while(topLeft >= topLeftCorner) {
                    uint64_t positionMask = 1ULL << topLeft;

                    attacksMask |= positionMask;
                    
                    if(occupancyMask & positionMask) break;

                    topLeft -= 9;
                }

                while(bottomRight <= bottomRightCorner) {
                    uint64_t positionMask = 1ULL << bottomRight;

                    attacksMask |= positionMask;
                    
                    if(occupancyMask & positionMask) break;

                    bottomRight += 9;
                }

                while(topRight >= topRightCorner) {
                    uint64_t positionMask = 1ULL << topRight;

                    attacksMask |= positionMask;
                    
                    if(occupancyMask & positionMask) break;

                    topRight -= 7;
                }

                while(bottomLeft >= bottomLeftCorner) {
                    uint64_t positionMask = 1ULL << bottomLeft;

                    attacksMask |= positionMask;
                    
                    if(occupancyMask & positionMask) break;

                    bottomLeft += 7;
                }

                bishopAttacks[sq][config] = attacksMask;
            }

        }

        // compute knight attacks
        for(int sq = 0; sq < 64; sq++) {
            int sqRank = sq/8, sqFile = sq%8;
            uint64_t attacksMask = 0;

            for(auto & offSets : knightMoveOffsets) {
                int8_t newRank = sqRank + offSets[1], newFile = sqFile + offSets[0];

                if(newRank >= 0 && newRank < 8 && newFile >= 0 && newFile < 8) {
                    attacksMask |= (1ULL << (newRank*8 + newFile));
                }
            } 

            knightAttacks[sq] = attacksMask;
        }

        // compute king attacks
        for(int sq = 0; sq < 64; sq++) {
            int sqRank = sq/8, sqFile = sq%8;
            uint64_t attacksMask = 0;

            for(auto & offSets : kingMoveOffsets) {
                int8_t newRank = sqRank + offSets[0], newFile = sqFile + offSets[1];

                if(newRank >= 0 && newRank < 8 && newFile >= 0 && newFile < 8) {
                    attacksMask |= (1ULL << (newRank*8 + newFile));
                }
            }

            kingAttacks[sq] = attacksMask;
        }
    }

    static uint64_t precomputedMasks[64][64];
    static std::vector<std::vector<uint64_t>> rookAttacks;
    static std::vector<std::vector<uint64_t>> bishopAttacks;
    static uint64_t knightAttacks[64];
    static uint64_t kingAttacks[64];
    static int8_t knightMoveOffsets[8][2];
    static int8_t kingMoveOffsets[8][2];

    static uint16_t compressedIndexOfOccupancy(uint64_t occupancyMaskForSquare, uint64_t allMoves);
};

uint64_t Precompute::precomputedMasks[64][64] = {0};
std::vector<std::vector<uint64_t>> Precompute::rookAttacks;
std::vector<std::vector<uint64_t>> Precompute::bishopAttacks;
int8_t Precompute::knightMoveOffsets[8][2] = {{-2, -1}, {-2, 1}, {-1, -2}, {1, -2}, {2, -1}, {2, 1}, {1, 2}, {-1, 2}};
int8_t Precompute::kingMoveOffsets[8][2] = {{0, 1}, {1, 0}, {-1, 0}, {0, -1}, {1, 1}, {-1, -1}, {1, -1}, {-1, 1}};

uint16_t Precompute::compressedIndexOfOccupancy(uint64_t occupancyMaskForSquare, uint64_t movesMask) {
    uint16_t index = 0;

    int bitPos = 0;

    while(movesMask) {
        int firstSetBit = __builtin_ctzll(movesMask);
        if((1ULL << firstSetBit) & occupancyMaskForSquare) {
            index |= 1ULL << bitPos;
        }

        movesMask &= movesMask-1;
        bitPos++;
    }

    return index;
}

#endif
