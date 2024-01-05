#pragma once
#ifndef BLOCKDROP_SIM_H
#define BLOCKDROP_SIM_H

#include <cassert>
#include <vector>
#include "olcPixelGameEngine.h"

namespace BlockDrop
{

enum class TileColor
{
	None,

	Red,
	Blue,
	Cyan,
	Magenta,
	Yellow,
	Green,
};

class Sim
{
public:
	Sim(int width, int height)
		: m_Width(width)
		, m_Height(height)
		, m_Tiles(width * height)
	{}
	Sim() = delete;
	Sim(Sim&) = delete;

public:
	std::vector<TileColor> const& Tiles() const
	{
		return m_Tiles;
	}

	// Temp
	void Set(int row, int col, TileColor tile)
	{
		_At(row, col) = tile;
	}
	TileColor At(int row, int col)
	{
		return _At(row, col);
	}

private:
	int m_Width{};
	int m_Height{};
	std::vector<TileColor> m_Tiles;

private:
	TileColor& _At(int row, int col)
	{
		assert(row >= 0 && row < m_Height);
		assert(col >= 0 && col < m_Width);

		return m_Tiles[row * m_Width + col];
	}
};

}
#endif