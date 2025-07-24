# LegalChess

LegalChess is a C++ library designed to track the state of a chess game. It processes moves in Universal Chess Interface (UCI) notation, validates their legality within the current game context, and determines game outcomes such as checkmate, stalemate, or various types of draws. The underlying implementation uses bitboards for efficient state management and move validation. It is built for storing game contexts on a server that handles real-time multiplayer games.

## Features

* **Game State Management:** Tracks the entire game from the starting position.
* **UCI Move Handling:** Accepts moves in standard UCI format (e.g., "e2e4", "e7e8q").
* **Move Validation:** Ensures that all attempted moves are legal according to the rules of chess.
* **Game Outcome Detection:**
    * Checkmate
    * Stalemate
    * Draw by three-fold repetition
    * Draw by the 50-move rule
    * Draw by insufficient material
* **Board Representation:** Can provide the current board state as a FEN string or a 2D character vector.
* **Exception Handling:** Throws exceptions for invalid moves or attempts to move after a game has concluded.

## API Usage

The primary interface to the library is the `LC::LegalChess` class.

### Initializing a Game

You can start a new game from the standard starting position or initialize a game by providing a space-separated string of UCI moves.

**Starting a new game:**

```cpp
#include "LegalChess.h"
#include <iostream>

int main() {
    LC::LegalChess game;
    // The game is now initialized at the starting position.
    std::cout << "Initial FEN: " << game.getFENString() << std::endl;
    return 0;
}
```

Initializing from a sequence of moves:The constructor can take a string of moves and will play them out, reporting performance metrics upon completion.#include "legalchess.h"

```cpp
#include "LegalChess.h"
#include <iostream>

int main() {
    // This will create a game and play through the Scholar's Mate.
    LC::LegalChess game("e2e4 e7e5 f1c4 b8c6 d1h5 g8f6 h5f7");
    
    return 0;
}
```

Making MovesMoves are made using the makeMove function, which takes a standard UCI move string. The function returns the GameResult after the move is completed.#include "legalchess.h"

```cpp
#include <iostream>
#include "LegalChess.h"
#include <vector>

void printBoard(const std::vector<std::vector<char>>& board) {
    for (const auto& row : board) {
        for (const auto& piece : row) {
            std::cout << piece << ' ';
        }
        std::cout << std::endl;
    }
}

int main() {
    LC::LegalChess game;

    try {
        game.makeMove("e2e4");
        game.makeMove("e7e5");
        game.makeMove("g1f3");
        game.makeMove("b8c6");

        std::cout << "FEN after 4 moves: " << game.getFENString() << std::endl;
        
        auto boardState = game.getBoard();
        printBoard(boardState);

    } catch (const std::exception& e) {
        std::cerr << "An error occurred: " << e.what() << std::endl;
    }

    return 0;
}
```

Checking Game State and ResultThe library provides several functions to check the status of the game.#include "legalchess.h"

```cpp
#include <iostream>
#include "LegalChess.h"

int main() {
    // Setup for a checkmate scenario (Fool's Mate)
    LC::LegalChess game;
    game.makeMove("f2f3");
    game.makeMove("e7e5");
    game.makeMove("g2g4");
    game.makeMove("d8h4");

    if (game.isGameOver()) {
        std::cout << "The game is over." << std::endl;

        if (game.isCheckMate(true)) { // Check if White is in checkmate
            std::cout << "White is in checkmate." << std::endl;
        }

        LC::GameResult result = game.getGameResult();
        if (result == LC::GameResult::BLACK_WINS) {
            std::cout << "Black wins!" << std::endl;
        }
    }
    return 0;
}
```

Handling DrawsYou can check for specific draw conditions.#include "legalchess.h"

```cpp
#include <iostream>
#include "LegalChess.h"

int main() {
    LC::LegalChess game;
    
    // Example of a game state leading to a draw by repetition
    game.makeMove("g1f3"); game.makeMove("g8f6");
    game.makeMove("f3g1"); game.makeMove("f6g8");
    game.makeMove("g1f3"); game.makeMove("g8f6");
    game.makeMove("f3g1"); game.makeMove("f6g8"); // Position repeats for the 3rd time

    if (game.isDrawByRepitition()) {
        std::cout << "Draw by three-fold repetition!" << std::endl;
    }

    if (game.isDrawByInsufficientMaterial()) {
        std::cout << "Draw by insufficient material!" << std::endl;
    }
}
```

Getting Board RepresentationsYou can retrieve the board state in different formats.#include "legalchess.h"

```cpp
#include <iostream>
#include "LegalChess.h"

int main() {
    LC::LegalChess game;
    game.makeMove("d2d4");
    game.makeMove("d7d5");

    // Get the FEN string
    std::string fen = game.getFENString();
    std::cout << "FEN: " << fen << std::endl;

    // Get the 2D board representation
    std::vector<std::vector<char>> board = game.getBoard();
    // `board` is an 8x8 vector of characters representing the pieces.
}
```

Error HandlingThe library throws exceptions for illegal operations. It is recommended to wrap makeMove calls in a try-catch block.LC::InvalidMoveException: Thrown when a move string is malformed or the move is not legal in the current position.LC::GameOverException: Thrown if makeMove is called after the game has already ended.#include "legalchess.h"

```cpp
#include <iostream>
#include "LegalChess.h"

int main() {
    LC::LegalChess game;
    try {
        // Attempt an illegal move
        game.makeMove("e2e5"); 
    } catch (const LC::InvalidMoveException& e) {
        std::cerr << "Invalid Move Error: " << e.what() << std::endl;
    }

    // Put the game in a finished state
    LC::LegalChess finished_game("f2f3 e7e5 g2g4 d8h4");
    try {
        // Attempt to move after game over
        finished_game.makeMove("a2a3");
    } catch (const LC::GameOverException& e) {
        std::cerr << "Game Over Error: " << e.what() << std::endl;
    }
    return 0;
}
```
