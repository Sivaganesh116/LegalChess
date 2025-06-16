import chess
import sys
import os

def convert_san_to_uci(san_moves_line):
    """
    Converts a single line of SAN chess moves to UCI notation.

    Args:
        san_moves_line (str): A string containing a full chess game in SAN,
                              e.g., "1. e4 d6 2. f4 c6 1/2-1/2".

    Returns:
        str: A string containing the same game in UCI notation,
             e.g., "e2e4 d7d6 f2f4 c7c6", or an error message if conversion fails.
    """
    board = chess.Board()
    uci_moves = []

    # Clean the input line: remove result (e.g., "1-0", "0-1", "1/2-1/2") and leading/trailing spaces
    # We split by space and process each token. Move numbers will be handled by the board.push_san method.
    clean_line = san_moves_line
    if "1-0" in clean_line:
        clean_line = clean_line.replace("1-0", "").strip()
    elif "0-1" in clean_line:
        clean_line = clean_line.replace("0-1", "").strip()
    elif "1/2-1/2" in clean_line:
        clean_line = clean_line.replace("1/2-1/2", "").strip()

    # Split the string by spaces to get individual move tokens (e.g., "1.", "e4", "d6")
    tokens = clean_line.split()

    for token in tokens:
        # Skip move numbers (e.g., "1.", "2.", "10.")
        if token.endswith('.') and token[:-1].isdigit():
            continue

        try:
            # Try to parse the SAN move on the current board
            move = board.parse_san(token)
            # Add the move to the list in UCI format
            uci_moves.append(move.uci())
            # Make the move on the board to update its state for the next SAN parsing
            board.push(move)
        except ValueError as e:
            # If a move cannot be parsed, it's an invalid SAN sequence or token
            print(f"Error parsing move '{token}' in line: '{san_moves_line.strip()}'. Error: {e}", file=sys.stderr)
            return f"ERROR: Could not parse '{token}' in game: {san_moves_line.strip()}"
        except IndexError as e:
            # This can happen if the SAN is malformed in a way parse_san doesn't expect
            print(f"Index Error parsing move '{token}' in line: '{san_moves_line.strip()}'. Error: {e}", file=sys.stderr)
            return f"ERROR: Index Error with '{token}' in game: {san_moves_line.strip()}"
        except Exception as e:
            # Catch any other unexpected errors
            print(f"An unexpected error occurred with move '{token}' in line: '{san_moves_line.strip()}'. Error: {e}", file=sys.stderr)
            return f"ERROR: Unexpected error with '{token}' in game: {san_moves_line.strip()}"

    return " ".join(uci_moves)

def main():
    """
    Main function to handle file input/output and orchestrate the conversion.
    Expects two command-line arguments: input_file_path and output_file_path.
    """
    if len(sys.argv) != 3:
        print("Usage: python san_to_uci.py <input_file_path> <output_file_path>")
        print("Example: python san_to_uci.py input_games.txt output_uci_games.txt")
        sys.exit(1)

    input_file_path = sys.argv[1]
    output_file_path = sys.argv[2]

    if not os.path.exists(input_file_path):
        print(f"Error: Input file '{input_file_path}' not found.", file=sys.stderr)
        sys.exit(1)

    print(f"Starting conversion from '{input_file_path}' to '{output_file_path}'...")

    try:
        with open(input_file_path, 'r', encoding='utf-8') as infile, \
             open(output_file_path, 'w', encoding='utf-8') as outfile:
            line_count = 0
            for line in infile:
                line_count += 1
                san_moves_line = line.strip()
                if not san_moves_line: # Skip empty lines
                    continue

                uci_result = convert_san_to_uci(san_moves_line)
                outfile.write(uci_result + '\n')
                if "ERROR:" in uci_result:
                    print(f"Processed line {line_count} with errors. See output file.", file=sys.stderr)
                # else:
                #     print(f"Successfully processed line {line_count}", end='\r') # Optional: progress indicator
            print(f"\nConversion complete. Processed {line_count} lines.")

    except IOError as e:
        print(f"File I/O error: {e}", file=sys.stderr)
        sys.exit(1)
    except Exception as e:
        print(f"An unexpected error occurred: {e}", file=sys.stderr)
        sys.exit(1)

if __name__ == "__main__":
    # Ensure the python-chess library is installed.
    # If not, you can install it using pip: pip install python-chess
    try:
        import chess
    except ImportError:
        print("The 'python-chess' library is not installed.", file=sys.stderr)
        print("Please install it using: pip install python-chess", file=sys.stderr)
        sys.exit(1)
    main()