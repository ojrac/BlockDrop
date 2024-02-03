#include "olcPixelGameEngine.h"
#include <stdint.h>
#include <string>
#include <time.h>
#include "Game.h"

namespace BlockDrop
{

bool App::OnUserUpdate(float fElapsedTime)
{
	if (GetKey(olc::ESCAPE).bPressed)
	{
		return false;
	}

	if (GetKey(olc::OEM_2).bPressed) // Slash/question mark
	{
		m_bAboutOpen = !m_bAboutOpen;
	}

	if (!m_bAboutOpen)
	{
		auto input = GetInput();
		m_Sim.Update(fElapsedTime, input);
	}

	Draw();

	return true;
}

void App::Draw()
{
	Clear(olc::VERY_DARK_GREY);

	DrawUI();

	DrawTiles();

	if (m_bAboutOpen)
	{
		DrawAbout();
	}
}

void App::DrawAbout()
{
	using namespace olc;
	FillRect(50, 50, 50, 50, olc::RED);
}

void App::DrawUI()
{
	using namespace olc;

	vi2d size{ s_BoardTileWidthPx, s_BoardTileHeightPx };
	vi2d topRight{ m_BoardTopLeft.x + s_BoardTileWidthPx, m_BoardTopLeft.y };
	vi2d bottomRight{ m_BoardTopLeft + size };

	// Sidebar: Preview
	DrawBorder({ s_SidebarLeft, s_UiTop }, { s_SidebarWidth, s_SidebarPreviewHeight }, 3, olc::GREY);
	FillRect({ s_SidebarLeft, s_UiTop }, { s_SidebarWidth, s_SidebarPreviewHeight }, olc::BLACK);
	// Preview Tile
	auto* tetronimo = TetronimoFactory::GetTetronimoByColor(m_Sim.GetNextBlockColor());
	if (tetronimo != nullptr)
	{
		olc::vi2d origin{
			s_SidebarLeft + (s_SidebarWidth / 2),
			s_UiTop + (s_SidebarPreviewHeight / 2) - s_TileSizePx,
		};
		origin += tetronimo->m_CenterOffset * s_TileSizePx;

		DrawTetronimoSquares(origin, tetronimo->m_Color, tetronimo->m_RotatedTileOffsets[0]);
	}

	// Sidebar: Level and Score
	DrawBorder({ s_SidebarLeft, s_SidebarNumbersTop }, { s_SidebarWidth, s_SidebarNumbersHeight }, 3, olc::GREY);
	FillRect({ s_SidebarLeft, s_SidebarNumbersTop }, { s_SidebarWidth, s_SidebarNumbersHeight }, olc::BLACK);

	DrawString(s_SidebarNumbersLeft, s_SidebarNumbersTextTop,
		"Level:", olc::WHITE, 2);
	DrawString(s_SidebarNumbersLeft, s_SidebarNumbersTextTop + s_SidebarNumbersLineHeight,
		std::to_string(m_Sim.GetLevel()), olc::WHITE, 2);
	DrawString(s_SidebarNumbersLeft, s_SidebarNumbersTextTop + 2 * s_SidebarNumbersLineHeight,
		"Score:", olc::WHITE, 2);
	DrawString(s_SidebarNumbersLeft, s_SidebarNumbersTextTop + 3 * s_SidebarNumbersLineHeight,
		std::to_string(m_Sim.GetScore()), olc::WHITE, 2);

	// Sidebar: Instructions
	DrawBorder({ s_SidebarLeft, s_SidebarInstructionsHelpTop }, { s_SidebarWidth, s_SidebarInstructionsHelpHeight }, 3, olc::GREY);
	FillRect({ s_SidebarLeft, s_SidebarInstructionsHelpTop }, { s_SidebarWidth, s_SidebarInstructionsHelpHeight }, olc::BLACK);

	std::vector<std::string> helpLines{
		"[Up]",
		"Rotate",
		"",
		"[Down]",
		"Fast",
		"",
		"[Space]",
		"Drop",
		"",
		"[?]",
		"About",
	};
	for (int i = 0; i < helpLines.size(); ++i)
	{
		int x = s_SidebarHelpLeft;
		if (i % 3 == 1)
		{
			x += 10;
		}
		DrawString(
			x, s_SidebarHelpStrTop + (i * s_SidebarNumbersLineHeight),
			helpLines[i], olc::WHITE, 2);
	}

	// Board
	DrawBorder(m_BoardTopLeft, size, 3, olc::GREY);
	FillRect(m_BoardTopLeft, size, olc::BLACK);
}

void App::DrawBorder(olc::vi2d const& topLeft, olc::vi2d const& size, int borderWidth, olc::Pixel color)
{
	for (int i = 1; i <= borderWidth; ++i)
	{
		olc::vi2d borderOffset{ i, i };
		DrawRect(topLeft - borderOffset, size + borderOffset + borderOffset, olc::GREY);
	}
}

olc::Pixel App::GetColor(TileColor color)
{
	switch (color)
	{
	case TileColor::Red:
		return olc::RED;
	case TileColor::Blue:
		return olc::BLUE;
	case TileColor::Cyan:
		return olc::CYAN;
	case TileColor::Magenta:
		return olc::MAGENTA;
	case TileColor::Yellow:
		return olc::YELLOW;
	case TileColor::Green:
		return olc::GREEN;
	case TileColor::Orange:
		return olc::Pixel(0xec, 0x97, 0x06);
	default:
		assert(0);
		return olc::BLACK;
	}
}

void App::DrawTiles()
{
	using namespace olc;

	// Board tiles
	using namespace olc;
	for (int row = 0; row < s_BoardTileHeight; ++row)
	{
		for (int col = 0; col < s_BoardTileWidth; ++col)
		{
			const auto tile = m_Sim.At(row, col);
			if (tile == TileColor::None)
			{
				continue;
			}

			DrawDecal(BoardToScreen(row, col), m_TileDecal.get(), {1, 1}, GetColor(tile));
		}
	}

	// Falling block
	auto optFallingBlock = m_Sim.GetFallingBlock();
	if (optFallingBlock.has_value())
	{
		auto& tetronimo = optFallingBlock.value();
		auto position = tetronimo.GetPosition();
		DrawTetronimo(tetronimo, BoardToScreen(position));

		// Drop preview
		auto color = GetColor(tetronimo.GetTileColor());
		position = m_Sim.GetDropPosition();
		for (auto& square : tetronimo.GetSquares())
		{
			int row = position.y + square.m_Row;
			int col = position.x + square.m_Column;

			if (row < 0 || row > s_BoardTileHeight || col < 0 || col > s_BoardTileWidth)
			{
				continue;
			}
			DrawTileOutline(row, col, square.m_Directions, color);
		}
	}
}

void App::DrawTetronimoSquares(olc::vi2d origin, TileColor tileColor, std::vector<TetronimoSquare> const& squares)
{
	auto color = GetColor(tileColor);
	for (auto& square : squares)
	{
		olc::vi2d pos = origin + olc::vi2d{ square.m_Column * s_TileSizePx, square.m_Row * s_TileSizePx };
		if (pos.y < m_BoardTopLeft.y)
		{
			continue;
		}
		DrawTileAtPixel(pos, color);
	}
}

void App::DrawTile(int row, int col, olc::Pixel color)
{
	if (row < 0 || row > s_BoardTileHeight || col < 0 || col > s_BoardTileWidth)
	{
		return;
	}
	DrawTileAtPixel(BoardToScreen(row, col), color);
}

void App::DrawTileOutline(int row, int col, BorderDirection directions, olc::Pixel color)
{
	if (row < 0 || row > s_BoardTileHeight || col < 0 || col > s_BoardTileWidth)
	{
		return;
	}

	olc::vi2d topLeft = BoardToScreen(row, col);
	olc::vi2d bottomRight{ topLeft.x + s_TileSizePx - 1, topLeft.y + s_TileSizePx - 1 };

	if (HasDirection(directions, BorderDirection::Top))
	{
		DrawLine(topLeft.x, topLeft.y, bottomRight.x, topLeft.y, color); // top
	}
	if (HasDirection(directions, BorderDirection::Right))
	{
		DrawLine(bottomRight.x, topLeft.y, bottomRight.x, bottomRight.y, color); // right
	}
	if (HasDirection(directions, BorderDirection::Bottom))
	{
		DrawLine(topLeft.x, bottomRight.y, bottomRight.x, bottomRight.y, color); // bottom
	}
	if (HasDirection(directions, BorderDirection::Left))
	{
		DrawLine(topLeft.x, topLeft.y, topLeft.x, bottomRight.y, color); // left
	}
}

Input App::GetInput()
{
	Input result = {};
	result.bLeft = GetKey(olc::Key::LEFT).bPressed;
	result.bLeftHeld = GetKey(olc::Key::LEFT).bHeld;
	result.bRight = GetKey(olc::Key::RIGHT).bPressed;
	result.bRightHeld = GetKey(olc::Key::RIGHT).bHeld;
	result.bRotateLeft = GetKey(olc::Key::Q).bPressed;
	result.bRotateRight = GetKey(olc::Key::E).bPressed || GetKey(olc::Key::UP).bPressed;
	result.bSoftDrop = GetKey(olc::Key::DOWN).bHeld;
	result.bHardDrop = GetKey(olc::Key::SPACE).bPressed;

	return result;
}

}
