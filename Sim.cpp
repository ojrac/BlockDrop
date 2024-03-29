#include "Sim.h"

#include <algorithm>
#include <set>

namespace BlockDrop
{


bool HasDirection(BorderDirection value, BorderDirection other)
{
	auto otherVal = static_cast<unsigned char>(other);
	return (static_cast<unsigned char>(value) & otherVal) == otherVal;
}

void Sim::Update(float deltaTime, Input const& input)
{
	if (m_GameOver)
	{
		return;
	}

	m_InputTimer -= deltaTime;
	m_LockDelayTimer -= deltaTime;

	m_InputTimer = std::max(m_InputTimer, HandleInput(input));

	if (m_LockDelayTimer > 0.f && m_FallingBlock.has_value() && !IsBlockOnGround(m_FallingBlock.value()))
	{
		m_LockDelayTimer = 0.f;
	}
	if (m_FallingBlock.has_value() && m_LockDelayTimer <= 0.f)
	{
		m_DropTimer += deltaTime * GetGravity(input);
		bool bDropped = m_DropTimer > 1.f;
		bool bFirstDrop = true;
		while (m_DropTimer > 1.f && m_FallingBlock.has_value())
		{
			TetronimoInstance copy = m_FallingBlock.value();
			if (!TryMoveFallingBlock({ 0, 1 }) && bFirstDrop)
			{
				// Place the current position blocks as tiles
				TransferBlockToTiles(m_FallingBlock.value());
				m_FallingBlock.reset();
				m_NextBlockTimer = s_TetronimoSpawnDelay;
			}

			if (!input.bHardDrop)
			{
				// For a hard drop, allow immediate locking.
				// Otherwise, wait for a lock delay.
				bFirstDrop = false;
			}
			m_DropTimer -= 1.f;
			ScoreTileDrop(input);
		}

		if (bDropped && m_FallingBlock.has_value() && IsBlockOnGround(m_FallingBlock.value()))
		{
			// Override drop timer to give a fixed amount of "lock" delay
			m_LockDelayTimer = s_LockDelay;
		}
	}
	else if (!m_FallingBlock.has_value())
	{
		m_NextBlockTimer -= deltaTime;
		if (m_NextBlockTimer <= 0.f)
		{
			// Spawn a new block
			TetronimoInstance newBlock = TetronimoFactory::New(0, m_Width / 2, PopNextBlockColor());
			if (HasCollision(newBlock))
			{
				TransferBlockToTiles(newBlock);
				GameOver();
			}
			else
			{
				m_FallingBlock = std::make_optional<TetronimoInstance>(newBlock);
				m_DropTimer = 0.f;
			}
		}
	}
}

void Sim::ResetGame()
{
	m_Level = 1;
	m_RowsCleared = 0;
	m_Score = 0;
	m_Combo = 0;

	m_LockDelayTimer = m_NextBlockTimer = m_DropTimer = m_InputTimer = 0;

	m_Tiles.assign(m_Tiles.size(), TileColor::None);
	m_FallingBlock.reset();
	m_NextBlocks.clear();
	m_GameOver = false;
}

void Sim::ScoreClearedRows(int rowCount)
{
	m_RowsCleared += rowCount;
	m_Level = 1 + (m_RowsCleared / s_RowsPerLevelUp);
	
	m_Score += m_ScoreByClearCount[std::min(static_cast<int>(m_ScoreByClearCount.size()) - 1, rowCount)];

	if (m_Combo > 0)
	{
		m_Score += 50 * m_Combo * m_Level;
	}
}

void Sim::ScoreTileDrop(Input const& input)
{
	if (input.bHardDrop)
	{
		m_Score += 2;
	}
	else if (input.bSoftDrop)
	{
		m_Score += 1;
	}
}


TileColor Sim::GetNextBlockColor()
{
	if (m_NextBlocks.empty())
	{
		m_NextBlocks = {
			TileColor::Red,
			TileColor::Blue,
			TileColor::Cyan,
			TileColor::Magenta,
			TileColor::Yellow,
			TileColor::Green,
			TileColor::Orange,
		};
		std::shuffle(m_NextBlocks.begin(), m_NextBlocks.end(), m_RandStream);
	}

	return m_NextBlocks.back();
}
TileColor Sim::PopNextBlockColor()
{
	auto result = GetNextBlockColor();
	assert(!m_NextBlocks.empty());
	m_NextBlocks.pop_back();
	return result;
}

olc::vi2d Sim::GetDropPosition() const
{
	if (!m_FallingBlock.has_value())
	{
		return {-1, -1};
	}

	TetronimoInstance copy = m_FallingBlock.value();
	olc::vi2d lastGoodPosition = { -1, -1 };
	while (!HasCollision(copy))
	{
		lastGoodPosition = copy.GetPosition();
		copy.Move({ 0, 1 });
	}

	return lastGoodPosition;
}

bool Sim::IsBlockOnGround(TetronimoInstance block)
{
	return !TryMoveBlock(block, { 0, 1 });
}

bool Sim::TryMoveBlock(TetronimoInstance& block, olc::vi2d const& delta)
{
	block.Move(delta);
	return !HasCollision(block);
}

bool Sim::TryMoveFallingBlock(olc::vi2d const& delta)
{
	if (!m_FallingBlock.has_value())
	{
		return false;
	}

	TetronimoInstance copy = m_FallingBlock.value();
	if (TryMoveBlock(copy, delta))
	{
		m_FallingBlock = copy;
		return true;
	}

	return false;
}

bool Sim::TryRotateFallingBlock(int direction)
{
	if (!m_FallingBlock.has_value())
	{
		return false;
	}

	TetronimoInstance rotated = m_FallingBlock.value();
	rotated.Rotate(direction);

	if (TryWallKick(rotated))
	{
		m_FallingBlock = rotated;
		return true;
	}

	return false;
}

bool Sim::TryWallKick(TetronimoInstance& tetronimo) const
{
	if (!HasCollision(tetronimo))
	{
		return true;
	}

	// Wall kick left 1
	TetronimoInstance copy = tetronimo;
	copy.Move({ -1, 0 });
	if (!HasCollision(copy))
	{
		tetronimo = copy;
		return true;
	}

	// Wall kick right 1
	copy = tetronimo;
	copy.Move({ 1, 0 });
	if (!HasCollision(copy))
	{
		tetronimo = copy;
		return true;
	}

	// Wall kick left 2
	copy = tetronimo;
	copy.Move({ -2, 0 });
	if (!HasCollision(copy))
	{
		tetronimo = copy;
		return true;
	}

	// Wall kick right 2
	copy = tetronimo;
	copy.Move({ 2, 0 });
	if (!HasCollision(copy))
	{
		tetronimo = copy;
		return true;
	}

	return false;
}

void Sim::TransferBlockToTiles(TetronimoInstance const& tetronimo)
{
	std::set<int> changedRows{};

	auto tetronimoPosition = tetronimo.GetPosition();
	for (auto const& square : tetronimo.GetSquares())
	{
		auto pos = square.AsVi2d() + tetronimoPosition;
		int row = pos.y;
		int col = pos.x;
		if (!IsValidPosition(row, col))
		{
			continue;
		}
		changedRows.insert(row);
		if (_At(row, col) == TileColor::None)
		{
			_At(row, col) = tetronimo.GetTileColor();
		}
	}

	std::vector<int> clearedRows{};
	for (int row : changedRows)
	{
		if (RowFilled(row))
		{
			clearedRows.push_back(row);
		}
	}

	if (clearedRows.empty())
	{
		// No rows cleared
		m_Combo = -1;
		return;
	}

	m_Combo++;
	ScoreClearedRows(static_cast<int>(clearedRows.size()));

	// Move the blocks down
	int dest = clearedRows.back();
	clearedRows.pop_back();
	int src = dest - 1;

	// Move tiles from the top to the bottom
	while (dest >= 0)
	{
		// If the row we're pulling from was cleared, skip over it as a source
		while (!clearedRows.empty() && clearedRows.back() == src)
		{
			src--;
			clearedRows.pop_back();
		}

		for (int col = 0; col < m_Width; ++col)
		{
			TileColor replacement{ TileColor::None };
			if (src > 0)
			{
				replacement = At(src, col);
			}
			_At(dest, col) = replacement;
		}

		dest--;
		src--;
	}
}

bool Sim::RowFilled(int row) const
{
	for (int col = 0; col < m_Width; ++col)
	{
		if (At(row, col) == TileColor::None)
		{
			return false;
		}
	}

	return true;
}

float Sim::GetGravity(Input const& input)
{
	if (input.bHardDrop && m_FallingBlock.has_value() && !IsBlockOnGround(m_FallingBlock.value()))
	{
		return 1200.f;
	}

	float gravity = m_GravityByLevel[std::min(static_cast<int>(m_GravityByLevel.size()), m_Level)];
	if (input.bSoftDrop && m_FallingBlock.has_value() && !IsBlockOnGround(m_FallingBlock.value()))
	{
		return 10 * gravity;
	}
	return gravity;
}

float Sim::HandleInput(Input const& input)
{
	if (input.bRotateLeft)
	{
		TryRotateFallingBlock(-1);
	}
	if (input.bRotateRight)
	{
		TryRotateFallingBlock(1);
	}

	if (!m_FallingBlock.has_value())
	{
		return 0.f;
	}

	if (input.bLeft && TryMoveFallingBlock({ -1, 0 }))
	{
		return s_InitialInputRepeatDelaySec;
	}
	if (input.bRight && TryMoveFallingBlock({ 1, 0 }))
	{
		return s_InitialInputRepeatDelaySec;
	}

	// Repeat left/right:
	if (m_InputTimer > 0)
	{
		return 0.f;
	}

	if (input.bLeftHeld && TryMoveFallingBlock({ -1, 0 }))
	{
		return s_InputRepeatDelaySec;
	}
	if (input.bRightHeld && TryMoveFallingBlock({ 1, 0 }))
	{
		return s_InputRepeatDelaySec;
	}

	return 0.f;
}

bool Sim::HasCollision(TetronimoInstance const& tetronimo) const
{
	auto tetronimoPosition = tetronimo.GetPosition();
	for (auto const& square : tetronimo.GetSquares())
	{
		auto pos = square.AsVi2d() + tetronimoPosition;
		int row = pos.y;
		int col = pos.x;
		// Ignore "collision" above the frame (row < 0)
		if (col < 0 || col >= m_Width || row >= m_Height)
		{
			return true;
		}
		if (row >= 0 && At(pos.y, pos.x) != TileColor::None)
		{
			return true;
		}
	}

	return false;
}

TileColor Sim::RandomColor()
{
	std::uniform_real_distribution<float> dist(0, 7);
	auto r = dist(m_RandStream);
	if (r < 1.0f)
	{
		return TileColor::Red;
	}
	if (r < 2.0f)
	{
		return TileColor::Blue;
	}
	if (r < 3.0f)
	{
		return TileColor::Cyan;
	}
	if (r < 4.0f)
	{
		return TileColor::Magenta;
	}
	if (r < 5.0f)
	{
		return TileColor::Yellow;
	}
	if (r < 6.0f)
	{
		return TileColor::Green;
	}
	return TileColor::Orange;
}

}