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
	bool bLeftHeld;
	bool bRight;
	bool bRightHeld;
	bool bHardDrop;
	bool bSoftDrop;
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

enum class BorderDirection : unsigned char
{
	None = 0,
	Top = 1 << 0,
	Left = 1 << 1,
	Right = 1 << 2,
	Bottom = 1 << 3,

	// Combo shortcuts, abbreviated
	TR = Top | Right,
	TRB = Top | Right | Bottom,
	TRL = Top | Right | Left,
	TB = Top | Bottom,
	TBL = Top | Bottom | Left,
	TL = Top | Left,

	RB = Right | Bottom,
	RBL = Right | Bottom | Left,
	RL = Right | Left,

	LR = Left | Right,
	LBR = Left | Bottom | Right,

	BL = Bottom | Left,
};

bool HasDirection(BorderDirection value, BorderDirection other);

struct TetronimoSquare
{
	int m_Column {};
	int m_Row{};
	BorderDirection m_Directions {};

	olc::vi2d AsVi2d() const
	{
		return olc::vi2d(m_Column, m_Row);
	}
};

class Tetronimo
{
public:
	Tetronimo(TileColor color, std::vector<std::vector<TetronimoSquare>> tileOffsets, olc::vf2d centerOffset)
		: m_Color(color)
		, m_RotatedTileOffsets(tileOffsets)
		, m_CenterOffset(centerOffset)
	{
	}

	Tetronimo() = delete;

public: // Struct-ish, all public
	TileColor m_Color;
	std::vector<std::vector<TetronimoSquare>> m_RotatedTileOffsets;
	olc::vf2d m_CenterOffset;
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

	std::vector<TetronimoSquare> const& GetSquares() const
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
	olc::vf2d GetCenterOffset() const
	{
		return m_Tetronimo->m_CenterOffset;
	}
	void SetPosition(olc::vi2d const& position)
	{
		m_Position = position;
	}
	void Move(olc::vi2d const& delta)
	{
		m_Position += delta;
	}

	void Rotate(int direction)
	{
		const int size = static_cast<int>(m_Tetronimo->m_RotatedTileOffsets.size());
		m_RotationIndex = (m_RotationIndex + direction) % size;
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
		Tetronimo* sharedData = GetTetronimoByColor(color);
		assert(sharedData != nullptr);

		TetronimoInstance result{ *sharedData, {column, row} };
		return result;
	}

	static Tetronimo* GetTetronimoByColor(TileColor color)
	{
		return Instance().GetByColor(color);
	}

private:
	TetronimoFactory()
		// I
		: m_Red(TileColor::Red, {
			{ {-2, 0, BorderDirection::TBL}, {-1, 0, BorderDirection::TB}, {0, 0, BorderDirection::TB}, {1, 0, BorderDirection::TRB} },
			{ {0, -1, BorderDirection::TRL}, {0, 0, BorderDirection::LR}, {0, 1, BorderDirection::LR}, {0, 2, BorderDirection::LBR} },
			}, {0, 0.5f})
		// S
		, m_Blue(TileColor::Blue, {
			{ {-1, 1, BorderDirection::TBL}, {0, 1, BorderDirection::RB}, {0, 0, BorderDirection::TL}, {1, 0, BorderDirection::TRB} },
			{ {0, 0, BorderDirection::TRL}, {0, 1, BorderDirection::BL}, {1, 1, BorderDirection::TR}, {1, 2, BorderDirection::LBR} },
			{ {-1, 2, BorderDirection::TBL}, {0, 2, BorderDirection::RB}, {0, 1, BorderDirection::TL}, {1, 1, BorderDirection::TRB} },
			{ {-1, 0, BorderDirection::TRL}, {-1, 1, BorderDirection::BL}, {0, 1, BorderDirection::TR}, {0, 2, BorderDirection::RBL} },
			}, {-0.5f, 0})
		// Z
		, m_Cyan(TileColor::Cyan, {
			{ {-1, 0, BorderDirection::TBL}, {0, 0, BorderDirection::TR}, {0, 1, BorderDirection::BL}, {1, 1, BorderDirection::TRB} },
			{ {1, 0, BorderDirection::TRL}, {1, 1, BorderDirection::RB}, {0, 1, BorderDirection::TL}, {0, 2, BorderDirection::RBL} },
			{ {-1, 1, BorderDirection::TBL}, {0, 1, BorderDirection::TR}, {0, 2, BorderDirection::BL}, {1, 2, BorderDirection::TRB} },
			{ {0, 0, BorderDirection::TRL}, {0, 1, BorderDirection::RB}, {-1, 1, BorderDirection::TL}, {-1, 2, BorderDirection::RBL} },
			}, { -0.5f, 0 })
		// J
		, m_Magenta(TileColor::Magenta, {
			{ {-1, 0, BorderDirection::TRL}, {-1, 1, BorderDirection::BL}, {0, 1, BorderDirection::TB}, {1, 1, BorderDirection::TRB} },
			{ {1, 0, BorderDirection::TRB}, {0, 0, BorderDirection::TL}, {0, 1, BorderDirection::RL}, {0, 2, BorderDirection::RBL} },
			{ {-1, 1, BorderDirection::TBL}, {0, 1, BorderDirection::TB}, {1, 1, BorderDirection::TR}, {1, 2, BorderDirection::RBL} },
			{ {0, 0, BorderDirection::TRL}, {0, 1, BorderDirection::RL}, {0, 2, BorderDirection::RB}, {-1, 2, BorderDirection::TBL} },
			}, { -0.5f, 0 })
		// L
		, m_Yellow(TileColor::Yellow, {
			{ {-1, 1, BorderDirection::TBL}, {0, 1, BorderDirection::TB}, {1, 1, BorderDirection::RB}, {1, 0, BorderDirection::TRL} },
			{ {0, 0, BorderDirection::TRL}, {0, 1, BorderDirection::RL}, {0, 2, BorderDirection::BL}, {1, 2, BorderDirection::TRB} },
			{ {-1, 2, BorderDirection::RBL}, {-1, 1, BorderDirection::TL}, {0, 1, BorderDirection::TB}, {1, 1, BorderDirection::TRB} },
			{ {-1, 0, BorderDirection::TBL}, {0, 0, BorderDirection::TR}, {0, 1, BorderDirection::LR}, {0, 2, BorderDirection::RBL} },
			}, { -0.5f, 0 })
		// T
		, m_Green(TileColor::Green, {
			{ {-1, 1, BorderDirection::TBL}, {0, 1, BorderDirection::Bottom}, {0, 0, BorderDirection::TRL}, {1, 1, BorderDirection::TRB} },
			{ {0, 0, BorderDirection::TRL}, {0, 1, BorderDirection::Left}, {1, 1, BorderDirection::TRB}, {0, 2, BorderDirection::RBL} },
			{ {-1, 1, BorderDirection::TBL}, {0, 1, BorderDirection::Top}, {0, 2, BorderDirection::RBL}, {1, 1, BorderDirection::TRB} },
			{ {0, 0, BorderDirection::TRL}, {0, 1, BorderDirection::Right}, {-1, 1, BorderDirection::TBL}, {0, 2, BorderDirection::RBL} },
			}, { -0.5f, 0 })
		// O
		, m_Orange(TileColor::Orange, {
			{ {-1, 0, BorderDirection::TL}, {0, 0, BorderDirection::TR}, {-1, 1, BorderDirection::BL}, {0, 1, BorderDirection::RB} },
			}, { 0, 0 })
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
	// Timings
	static constexpr float s_RotateAirTimeSec = 0.1f;
	static constexpr float s_InitialInputRepeatDelaySec = 0.167f;
	static constexpr float s_InputRepeatDelaySec = 0.033f;
	static constexpr float s_TetronimoSpawnDelay = 0.1f;
	static constexpr float s_LockDelay = 0.5f;

	// Score and levels
	static constexpr int s_MaxLevel = 10;
	static constexpr int s_RowsPerLevelUp = 10;

public:
	Sim(int width, int height)
		: m_Width(width)
		, m_Height(height)
		, m_Tiles(width * height)
		, m_RandStream(std::random_device()())
	{
		ResetGame();
	}

	Sim() = delete;
	Sim(Sim&) = delete;

public:
	std::vector<TileColor> const& Tiles() const
	{
		return m_Tiles;
	}

	TileColor At(int row, int col) const
	{
		assert(IsValidPosition(row, col));

		return m_Tiles[row * m_Width + col];
	}

	std::optional<TetronimoInstance> const& GetFallingBlock() { return m_FallingBlock; }

	TileColor GetNextBlockColor();
	TileColor PopNextBlockColor();

	olc::vi2d GetDropPosition() const;

	void Update(float deltaTime, Input const& input);

	int GetLevel() const { return m_Level; }
	int GetScore() const { return m_Score; }

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

	float GetGravity(Input const& input);

	float HandleInput(Input const& input);

	bool HasCollision(TetronimoInstance const& tetronimo) const;

	void ResetGame()
	{
		m_Level = 1;
		m_Score = 0;
		m_RowsCleared = 0;

		m_LockDelayTimer = m_NextBlockTimer = m_DropTimer = m_InputTimer = 0;

		m_Tiles.assign(m_Tiles.size(), TileColor::None);
		m_FallingBlock.reset();
		m_NextBlocks.clear();
	}
	void GameOver()
	{
		ResetGame();
	}

	void ScoreClearedRows(int rowCount);

	bool IsBlockOnGround(TetronimoInstance block);
	bool TryMoveBlock(TetronimoInstance& block, olc::vi2d const& delta);
	bool TryMoveFallingBlock(olc::vi2d const& delta);
	bool TryRotateFallingBlock(int direction);

	void TransferBlockToTiles(TetronimoInstance const& tetronimo);
	bool RowFilled(int col) const;

private:
	int m_Width{};
	int m_Height{};

	int m_Level{};
	int m_Score{};
	int m_RowsCleared{};

	float m_LockDelayTimer{};
	float m_NextBlockTimer{};
	float m_DropTimer{};
	float m_InputTimer{};
	std::vector<TileColor> m_Tiles{};
	std::optional<TetronimoInstance> m_FallingBlock{};
	std::vector<TileColor> m_NextBlocks{};

	std::mt19937 m_RandStream;

	// Constants
	std::vector<int> m_ScoreByClearCount{ 0, 100, 300, 500, 800 };
};

}
#endif