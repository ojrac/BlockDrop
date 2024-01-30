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
	m_DropTimer -= deltaTime;
	m_InputTimer -= deltaTime;

	if (HandleInput(input))
	{
		m_InputTimer = s_InputRepeatDelaySec;
	}

	if (m_DropTimer > 0)
	{
		return;
	}

	// Wait before the next step
	// TODO: Scale with level
	m_DropTimer = 0.7f;

	if (m_FallingBlock.has_value())
	{
		TetronimoInstance copy = m_FallingBlock.value();
		if (!TryMoveFallingBlock({ 0, 1 }))
		{
			// Place the current position blocks as tiles
			TransferBlockToTiles(m_FallingBlock.value());
			m_FallingBlock.reset();
		}
	}
	else
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
		}
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

bool Sim::TryRotateFallingBlock(int direction)
{
	if (!m_FallingBlock.has_value())
	{
		return false;
	}

	TetronimoInstance copy = m_FallingBlock.value();
	copy.Rotate(direction);
	if (HasCollision(copy))
	{
		return false;
	}
	else
	{
		m_FallingBlock = copy;
		return true;
	}
}

bool Sim::TryMoveFallingBlock(olc::vi2d const& delta)
{
	if (!m_FallingBlock.has_value())
	{
		return false;
	}

	TetronimoInstance copy = m_FallingBlock.value();
	copy.Move(delta);
	if (HasCollision(copy))
	{
		return false;
	}
	else
	{
		m_FallingBlock = copy;
		return true;
	}
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
		return;
	}

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


bool Sim::HandleInput(Input const& input)
{
	if (input.bRotateLeft && TryRotateFallingBlock(-1))
	{
		m_DropTimer = std::max(s_RotateAirTimeSec, m_DropTimer);
	}
	if (input.bRotateRight && TryRotateFallingBlock(1))
	{
		m_DropTimer = std::max(s_RotateAirTimeSec, m_DropTimer);
	}

	if (m_InputTimer > 0 || !m_FallingBlock.has_value())
	{
		return false;
	}

	if (input.bLeft && TryMoveFallingBlock({ -1, 0 }))
	{
		return true;
	}
	if (input.bRight && TryMoveFallingBlock({ 1, 0 }))
	{
		return true;
	}

	if (input.bDrop && m_FallingBlock.has_value())
	{
		TetronimoInstance& tetronimo = m_FallingBlock.value();
		olc::vi2d dropPosition = GetDropPosition();
		tetronimo.SetPosition(dropPosition);
		m_DropTimer = 0;
	}

	return false;
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