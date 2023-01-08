#pragma once
#include <DirectXMath.h>

struct Tile {
	int index;
	bool isVisited;
	bool isVisitable;
	int visitedOnMoveNo;
	inline static int lastVisitedTileIndex = -1;
	inline static int visitedTileCount = 0;
	inline static int constructedTileCount = 0;

	DirectX::XMFLOAT4X4 worldMatrix; // our first cubes world matrix (transformation matrix)
	DirectX::XMFLOAT4 position; // tile position 
	DirectX::XMFLOAT4 color;	// color of the tile

	Tile() : isVisited(false), isVisitable(false), visitedOnMoveNo(1), position(DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f)), color(1.0, 1.0, 1.0, 1.0) {
		index = constructedTileCount;
		++constructedTileCount;
	}

	void set_visited(bool isVisited) {
		if(isVisited) {
			lastVisitedTileIndex = this->index;
			visitedOnMoveNo = visitedTileCount + 1;
			visitedTileCount++;
		}

		this->isVisited = isVisited;
	}

};

