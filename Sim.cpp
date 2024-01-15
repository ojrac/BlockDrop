#include "Sim.h"

namespace BlockDrop
{

void Sim::Update(float deltaTime, Input const& input)
{
	m_DropTimer -= deltaTime;
	m_InputTimer -= deltaTime;

	if (HandleInput(input))
	{
		m_InputTimer = 0.125f;
	}

	if (m_DropTimer > 0)
	{
		return;
	}

	// Wait before the next step
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
		auto newBlock = TetronimoFactory::New(0, m_Width / 2, RandomColor());
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

bool Sim::HandleInput(Input const& input)
{
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

	// TODO: Rotate

	// TODO: Drop

	// TEMP
	if (input.bDrop)
	{
		m_FallingBlock.reset();
	}

	return false;
}

bool Sim::HasCollision(TetronimoInstance const& tetronimo)
{
	auto tetronimoPosition = tetronimo.GetPosition();
	for (auto const& offset : tetronimo.GetOffsets())
	{
		auto pos = offset + tetronimoPosition;
		int row = pos.y;
		int col = pos.x;
		// Ignore "collision" above the frame (row < 0)
		if (col < 0 || col >= m_Width || row >= m_Height)
		{
			return true;
		}
		if (row >= 0 && _At(pos.y, pos.x) != TileColor::None)
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