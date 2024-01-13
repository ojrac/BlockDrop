#pragma once
#ifndef BLOCKDROP_SIM_H
#define BLOCKDROP_SIM_H

#include <cassert>
#include <optional>
#include <random>
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

class Tetronimo
{
public:
	Tetronimo(TileColor color, std::vector<olc::vi2d> tileOffsets)
		: m_Color(color)
		, m_TileOffsets(tileOffsets)
	{
	}

	Tetronimo() = delete;

public: // Struct-ish, all public
	TileColor m_Color;
	std::vector<olc::vi2d> m_TileOffsets;
};

class TetronimoInstance
{
public:
	TetronimoInstance(Tetronimo& tetronimo, olc::vi2d position)
		: m_Tetronimo(&tetronimo)
		, m_Position(position)
		, m_RotationIndex(0)
	{
	}

	std::vector<olc::vi2d> const& GetOffsets() const
	{
		// TODO: Rotate by index
		return m_Tetronimo->m_TileOffsets;
	}
	TileColor GetTileColor() const
	{
		return m_Tetronimo->m_Color;
	}
	olc::vi2d GetPosition() const
	{
		return m_Position;
	}

private:
	Tetronimo* m_Tetronimo;
	olc::vi2d m_Position{};
	int m_RotationIndex{};
};

class TetronimoFactory
{
public:
	static TetronimoInstance New(int row, int column, TileColor color)
	{
		Tetronimo* sharedData = Instance().GetByColor(color);
		assert(sharedData != nullptr);

		TetronimoInstance result{ *sharedData, {column, row} };
		return result;
	}

private:
	TetronimoFactory()
		: m_Red(TileColor::Red, { {-1, 0}, {0, 0}, {1, 0}, {2, 0} })
		, m_Blue(TileColor::Blue, { {-1, 0}, {0, 0}, {0, -1}, {1, -1} })
		, m_Cyan(TileColor::Cyan, { {-1, -1}, {0, -1}, {0, 0}, {1, 0} })
		, m_Magenta(TileColor::Magenta, { {-1, -1}, {-1, 0}, {0, 0}, {1, 0} })
		, m_Yellow(TileColor::Yellow, { {-1, 0}, {0, 0}, {1, 0}, {1, -1} })
		, m_Green(TileColor::Green, { {-1, 0}, {0, 0}, {0, -1}, {1, 0} })
	{
	}
	TetronimoFactory(TetronimoFactory&) = delete;

	static TetronimoFactory& Instance() {
		static TetronimoFactory factory;
		return factory;
	}
	Tetronimo* GetByColor(TileColor color)
	{
		switch (color) {
		case TileColor::Red:
			return &m_Red;
		case TileColor::Blue:
			return &m_Blue;
		case TileColor::Cyan:
			return &m_Cyan;
		case TileColor::Magenta:
			return &m_Magenta;
		case TileColor::Yellow:
			return &m_Yellow;
		case TileColor::Green:
			return &m_Green;
		}

		assert(0);
		return nullptr;
	}

	Tetronimo m_Red;
	Tetronimo m_Blue;
	Tetronimo m_Cyan;
	Tetronimo m_Magenta;
	Tetronimo m_Yellow;
	Tetronimo m_Green;
};

class Sim
{
public:
	Sim(int width, int height)
		: m_Width(width)
		, m_Height(height)
		, m_Tiles(width * height)
		, m_FallingBlock()
		, m_RandStream(std::random_device()())
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

	std::optional<TetronimoInstance> const& GetFallingBlock() { return m_FallingBlock; }

	void Update(float deltaTime)
	{
		if (!m_FallingBlock.has_value())
		{
			m_FallingBlock = std::make_optional<TetronimoInstance>(TetronimoFactory::New(0, 3, RandomColor()));
		}
	}

private:
	TileColor RandomColor();

	TileColor& _At(int row, int col)
	{
		assert(row >= 0 && row < m_Height);
		assert(col >= 0 && col < m_Width);

		return m_Tiles[row * m_Width + col];
	}

private:
	int m_Width{};
	int m_Height{};
	std::vector<TileColor> m_Tiles{};
	std::optional<TetronimoInstance> m_FallingBlock{};

	std::mt19937 m_RandStream;
};

}
#endif