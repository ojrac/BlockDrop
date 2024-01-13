#include "Sim.h"

namespace BlockDrop
{


TileColor Sim::RandomColor()
{
	std::uniform_real_distribution<float> dist(0, 6);
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
	return TileColor::Green;
}

}