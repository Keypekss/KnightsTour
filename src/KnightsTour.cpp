#include "KnightsTour.h"

bool KnightsTour::is_valid_letter(char letter) {
	return ((int)letter >= 65 && (int)letter <= 72) || ((int)letter >= 97 && (int)letter <= 104);
}

bool KnightsTour::is_valid_number(char number) {
	return ((int)number >= 49 && (int)number <= 56);
}

int KnightsTour::chess_notation_to_index(std::string input) {
	if(input.size() != 2) {
		std::cout << "Invalid Input. Length should be 2.\n";
		return -1;
	}

	char letter = static_cast<char>(input[0]);
	bool isValidLetter = is_valid_letter(letter);        // is it uppercase or lowercase letter between A to H.
	if(isValidLetter == false) {
		std::cout << "Invalid letter input.\n";            // Throw if first character is not a letter.
		return -1;
	}

	// Convert lowercase letters to uppercase
	if((int)letter >= 97 && (int)letter <= 122)
		letter = (int)letter - 32;

	char number = static_cast<char>(input[1]);
	bool isValidNumber = is_valid_number(number);
	if(isValidNumber == false) {
		std::cout << "Invalid number input.\n";
		return -1;
	}

	// Convert user input to tile index according to formula below.
	// rows(8) * (number - 49) + (letter - 65) for upper letter
	int index = rows * ((int)number - 49) + ((int)letter - 65);

	return index;
}

std::string KnightsTour::index_to_chess_notation(int index) {
	std::string result {};
	int number = int(index / rows) + 49;
	int letter = (index % columns) + 65;
	result.push_back((char)letter);
	result.push_back(((char)number));

	return result;
}

void KnightsTour::calculate_visitable_tile(const std::vector<int>::iterator& currentMove) {
	// Reset previously calculated result first
	for (auto& tile : chessboard) {
		tile.isVisitable = false;
		if(tile.isVisited)
			tile.color = DirectX::XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
		else
			tile.color = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	}

	std::string chessNotation = index_to_chess_notation(*currentMove);
	int letter = chessNotation[0];
	int number = chessNotation[1];

	// Calculate which tiles can be visited based on the current tile knight is standing on.
	visitableTileExists = false;
	if(is_valid_letter(letter - 1) && is_valid_number(number - 2)) {
		std::string tile {};
		tile.push_back((char)(letter - 1));
		tile.push_back((char)(number - 2));
		int tileIndex = chess_notation_to_index(tile);
		if(chessboard.at(tileIndex).isVisited == false) {
			chessboard.at(tileIndex).isVisitable = true;
			chessboard.at(tileIndex).color = DirectX::XMFLOAT4(0.3, 0.3, 0.7, 0.5);
			visitableTileExists = true;
		}
	}
	if(is_valid_letter(letter + 1) && is_valid_number(number - 2)) {
		std::string tile {};
		tile.push_back((char)(letter + 1));
		tile.push_back((char)(number - 2));
		int tileIndex = chess_notation_to_index(tile);
		if (chessboard.at(tileIndex).isVisited == false) {
			chessboard.at(tileIndex).isVisitable = true;
			chessboard.at(tileIndex).color = DirectX::XMFLOAT4(0.3, 0.3, 0.7, 0.5);
			visitableTileExists = true;
		}
	}
	if(is_valid_letter(letter - 2) && is_valid_number(number - 1)) {
		std::string tile {};
		tile.push_back((char)(letter - 2));
		tile.push_back((char)(number - 1));
		int tileIndex = chess_notation_to_index(tile);
		if (chessboard.at(tileIndex).isVisited == false) {
			chessboard.at(tileIndex).isVisitable = true;
			chessboard.at(tileIndex).color = DirectX::XMFLOAT4(0.3, 0.3, 0.7, 0.5);
			visitableTileExists = true;
		}
	}
	if(is_valid_letter(letter + 2) && is_valid_number(number - 1)) {
		std::string tile {};
		tile.push_back((char)(letter + 2));
		tile.push_back((char)(number - 1));
		int tileIndex = chess_notation_to_index(tile);
		if (chessboard.at(tileIndex).isVisited == false) {
			chessboard.at(tileIndex).isVisitable = true;
			chessboard.at(tileIndex).color = DirectX::XMFLOAT4(0.3, 0.3, 0.7, 0.5);
			visitableTileExists = true;
		}
	}
	if(is_valid_letter(letter - 2) && is_valid_number(number + 1)) {
		std::string tile {};
		tile.push_back((char)(letter - 2));
		tile.push_back((char)(number + 1));
		int tileIndex = chess_notation_to_index(tile);
		if (chessboard.at(tileIndex).isVisited == false) {
			chessboard.at(tileIndex).isVisitable = true;
			chessboard.at(tileIndex).color = DirectX::XMFLOAT4(0.3, 0.3, 0.7, 0.5);
			visitableTileExists = true;
		}
	}
	if(is_valid_letter(letter + 2) && is_valid_number(number + 1)) {
		std::string tile {};
		tile.push_back((char)(letter + 2));
		tile.push_back((char)(number + 1));
		int tileIndex = chess_notation_to_index(tile);
		if (chessboard.at(tileIndex).isVisited == false) {
			chessboard.at(tileIndex).isVisitable = true;
			chessboard.at(tileIndex).color = DirectX::XMFLOAT4(0.3, 0.3, 0.7, 0.5);
			visitableTileExists = true;
		}
	}
	if(is_valid_letter(letter - 1) && is_valid_number(number + 2)) {
		std::string tile {};
		tile.push_back((char)(letter - 1));
		tile.push_back((char)(number + 2));
		int tileIndex = chess_notation_to_index(tile);
		if (chessboard.at(tileIndex).isVisited == false) {
			chessboard.at(tileIndex).isVisitable = true;
			chessboard.at(tileIndex).color = DirectX::XMFLOAT4(0.3, 0.3, 0.7, 0.5);
			visitableTileExists = true;
		}
	}
	if(is_valid_letter(letter + 1) && is_valid_number(number + 2)) {
		std::string tile {};
		tile.push_back((char)(letter + 1));
		tile.push_back((char)(number + 2));
		int tileIndex = chess_notation_to_index(tile);
		if (chessboard.at(tileIndex).isVisited == false) {
			chessboard.at(tileIndex).isVisitable = true;
			chessboard.at(tileIndex).color = DirectX::XMFLOAT4(0.3, 0.3, 0.7, 0.5);
			visitableTileExists = true;
		}
	}
}

bool KnightsTour::enforce_next_move(int index) {
	if(isFirstMoveMade == false)
		return true;

	if(chessboard.at(index).isVisitable == false) {
		std::cout << "You can't move there." << std::endl;
		return false;
	}
	else
		return true;
}

void KnightsTour::make_move(const std::vector<int>::iterator& currentMove) {
	isFirstMoveMade = true;
	chessboard.at(*currentMove).set_visited(true);
	chessboard.at(*currentMove).color = DirectX::XMFLOAT4(0.5, 1.0, 0.5, 0.);
}


void KnightsTour::undo_move()
{
	if(currentMoveItr != movesMade.begin()) {
		chessboard.at(*currentMoveItr).isVisited = false;
		--currentMoveItr;
		calculate_visitable_tile(currentMoveItr);
		make_move(currentMoveItr);
	}
}

void KnightsTour::redo_move()
{
	if(currentMoveItr != movesMade.end() - 1) {
		chessboard.at(*currentMoveItr).isVisited = true;
		++currentMoveItr;
		calculate_visitable_tile(currentMoveItr);
		make_move(currentMoveItr);
	}
}

void KnightsTour::set_current_move_iterator(int index)
{
	// Remove any move made that comes after what currentMove iterator points to.
	if (movesMade.empty() == false && currentMoveItr != movesMade.end() - 1)
		movesMade.erase(currentMoveItr + 1, movesMade.end());
	movesMade.push_back(index);
	currentMoveItr = movesMade.end() - 1;
}

void KnightsTour::print_chessboard() {
	// Print column letters (A-H)
	std::cout << "  ";
	for(int i = 65; i < 73; ++i)
		std::cout << std::setw(4) << (char)i;

	std::cout << std::endl << std::endl;

	for(int row = rows - 1; row >= 0; --row) {
		// Print line numbers
		std::cout << row + 1 << " ";

		// Print tiles
		for(uint8_t column = 0; column < columns; ++column) {
			if(chessboard.at((rows * row) + column).isVisited) {
				if(chessboard.at((rows * row) + column).index == Tile::lastVisitedTileIndex)
					std::cout << std::setw(4) << "@";
				else
					std::cout << std::setw(4) << chessboard.at((rows * row) + column).visitedOnMoveNo;
			}
			else if(chessboard.at((rows * row) + column).isVisitable)
				std::cout << std::setw(4) << "o";
			else
				std::cout << std::setw(4) << "#";
		}
		std::cout << std::endl << std::endl;
	}
}

void KnightsTour::clear_screen() {
	for (auto& tile : chessboard) {
		tile.color = DirectX::XMFLOAT4(1.0, 1.0, 1.0, 1.0);
		tile.isVisitable = false;
		tile.isVisited = false;
		tile.visitedTileCount = 0;
		tile.lastVisitedTileIndex = -1;
	}
	KnightsTour::isFirstMoveMade = false;
}
