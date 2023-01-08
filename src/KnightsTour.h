#pragma once

#include <iostream>
#include <iomanip>
#include <array>
#include <string>
#include <cassert>
#include <vector>

#include "Tile.h"

constexpr uint8_t rows = 8;     // Total number of rows on a chess board.
constexpr uint8_t columns = 8;  // Total number of columns on a chess board.

class KnightsTour {
public:
	inline static std::array<Tile, rows * columns> chessboard{};
	inline static std::vector<int> movesMade {};
	inline static std::vector<int>::iterator currentMoveItr;
	inline static bool isFirstMoveMade = false;
	static inline bool visitableTileExists = true;
	static void make_move(const std::vector<int>::iterator& currentMoveItr);
	static void undo_move();
	static void redo_move();
	static void set_current_move_iterator(int index);
	static bool is_valid_letter(char letter);
	static bool is_valid_number(char number);
	static int chess_notation_to_index(std::string input);
	static std::string index_to_chess_notation(int index);
	static void calculate_visitable_tile(const std::vector<int>::iterator& currentMoveItr);
	static bool enforce_next_move(int index);
	static void print_chessboard();
	static void clear_screen();

	KnightsTour() = delete;

private:

};
