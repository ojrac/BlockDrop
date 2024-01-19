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

struct Input {
	bool bLeft;
	bool bRight;
	bool bDrop;
	bool bRotateLeft;
	bool bRotateRight;
};

enum class TileColor
{
	None,

	Red,
	Blue,
	Cyan,
	Magenta,
	Yellow,
	Green,
	Orange,
};

class Tetronimo
{
public:
	Tetronimo(TileColor color, std::vector<std::vector<olc::vi2d>> tileOffsets)
		: m_Color(color)
		, m_RotatedTileOffsets(tileOffsets)
	{
	}

	Tetronimo() = delete;

public: // Struct-ish, all public
	TileColor m_Color;
	std::vector<std::vector<olc::vi2d>> m_RotatedTileOffsets;
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
		return m_Tetronimo->m_RotatedTileOffsets[m_RotationIndex];
	}
	TileColor GetTileColor() const
	{
		return m_Tetronimo->m_Color;
	}
	olc::vi2d GetPosition() const
	{
		return m_Position;
	}
	void Move(olc::vi2d const& delta)
	{
		m_Position += delta;
	}

	void Rotate(int direction)
	{
		m_RotationIndex = (m_RotationIndex + direction) % m_Tetronimo->m_RotatedTileOffsets.size();
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
		// I
		: m_Red(TileColor::Red, {
			{ {-2, 0}, {-1, 0}, {0, 0}, {1, 0} },
			{ {0, -1}, {0, 0}, {0, 1}, {0, 2} },
			})
		// S
		, m_Blue(TileColor::Blue, {
			{ {-1, 1}, {0, 1}, {0, 0}, {1, 0} },
			{ {0, 0}, {0, 1}, {1, 1}, {1, 2} },
			{ {-1, 2}, {0, 2}, {0, 1}, {1, 1} },
			{ {-1, 0}, {-1, 1}, {0, 1}, {0, 2} },
			})
		// Z
		, m_Cyan(TileColor::Cyan, {
			{ {-1, 0}, {0, 0}, {0, 1}, {1, 1} },
			{ {1, 0}, {1, 1}, {0, 1}, {0, 2} },
			{ {-1, 1}, {0, 1}, {0, 2}, {1, 2} },
			{ {0, 0}, {0, 1}, {-1, 1}, {-1, 2} },
			})
		// J
		, m_Magenta(TileColor::Magenta, {
			{ {-1, 0}, {-1, 1}, {0, 1}, {1, 1} },
			{ {1, 0}, {0, 0}, {0, 1}, {0, 2} },
			{ {-1, 1}, {0, 1}, {1, 1}, {1, 2} },
			{ {0, 0}, {0, 1}, {0, 2}, {-1, 2} },
			})
		// L
		, m_Yellow(TileColor::Yellow, {
			{ {-1, 1}, {0, 1}, {1, 1}, {1, 0} },
			{ {0, 0}, {0, 1}, {0, 2}, {1, 2} },
			{ {-1, 2}, {-1, 1}, {0, 1}, {1, 1} },
			{ {-1, 0}, {0, 0}, {0, 1}, {0, 2} },
			})
		// T
		, m_Green(TileColor::Green, {
			{ {-1, 1}, {0, 1}, {0, 0}, {1, 1} },
			{ {0, 0}, {0, 1}, {0, 2}, {1, 1} },
			{ {-1, 1}, {0, 1}, {1, 1}, {0, 2} },
			{ {-1, 1}, {0, 0}, {0, 1}, {0, 2} },
			})
		// O
		, m_Orange(TileColor::Orange, {
			{ {-1, 0}, {0, 0}, {-1, 1}, {0, 1} },
			})
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
		case TileColor::Orange:
			return &m_Orange;
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
	Tetronimo m_Orange;
};

class Sim
{
public:
	static constexpr float s_RotateAirTimeSec = 0.1f;
	static constexpr float s_InputRepeatDelaySec = 0.125f;

public:
	Sim(int width, int height)
		: m_Width(width)
		, m_Height(height)
		, m_Tiles(width * height)
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

	void Update(float deltaTime, Input const& input);

private:
	TileColor RandomColor();

	bool IsValidPosition(int row, int col) const
	{
		return row >= 0 && row < m_Height && col >= 0 && col < m_Width;
	}

	TileColor& _At(int row, int col)
	{
		assert(IsValidPosition(row, col));

		return m_Tiles[row * m_Width + col];
	}

	bool HandleInput(Input const& input);

	bool HasCollision(TetronimoInstance const& tetronimo);

	void GameOver()
	{
		m_Tiles.assign(m_Tiles.size(), TileColor::None);
		m_FallingBlock.reset();
	}

	bool TryMoveFallingBlock(olc::vi2d const& delta);
	bool TryRotateFallingBlock(int direction);

	void TransferBlockToTiles(TetronimoInstance const& tetronimo)
	{
		auto tetronimoPosition = tetronimo.GetPosition();
		for (auto const& offset : tetronimo.GetOffsets())
		{
			auto pos = offset + tetronimoPosition;
			int row = pos.y;
			int col = pos.x;
			if (!IsValidPosition(row, col))
			{
				continue;
			}
			if (_At(row, col) == TileColor::None)
			{
				_At(row, col) = tetronimo.GetTileColor();
			}
		}
	}

private:
	int m_Width{};
	int m_Height{};
	float m_DropTimer{};
	float m_InputTimer{};
	std::vector<TileColor> m_Tiles{};
	std::optional<TetronimoInstance> m_FallingBlock{};

	std::mt19937 m_RandStream;
};

}
#endif