#include "olcPixelGameEngine.h"
#include <stdint.h>
#include <string>
#include <time.h>
#include "Game.h"

namespace BlockDrop
{

bool App::OnUserUpdate(float fElapsedTime)
{
	switch (m_UiOverlayState)
	{
	case UiOverlayState::Exit:
	{
		if (GetKey(olc::ENTER).bPressed)
		{
			m_bExiting = true;
		}
		else if (GetKey(olc::ESCAPE).bPressed)
		{
			m_UiOverlayState = UiOverlayState::None;
		}
	} break;
	case UiOverlayState::About:
	{
		if (GetKey(olc::OEM_2).bPressed) // Slash/question mark
		{
			m_UiOverlayState = UiOverlayState::None;
		}
	} break;
	case UiOverlayState::None:
	{
		switch (m_UiState)
		{
		case UiState::GameOver:
		{
			if (GetKey(olc::ENTER).bPressed)
			{
				if (m_Sim.GetScore() > m_ScoreBoard.GetLowestScore())
				{
					m_UiState = UiState::ScoreboardEntry;
					m_UiIndex = 0;
					m_PendingScoreboard = ScoreBoard(m_ScoreBoard.GetScoreList());
					m_PendingScoreboard.SetScore("???", m_Sim.GetScore(), m_Sim.GetLevel());
					auto const& scoreList = m_PendingScoreboard.GetScoreList();
					m_PendingScoreIndex = static_cast<int>(std::distance(scoreList.begin(),
						std::find_if(scoreList.begin(), scoreList.end(), [](auto const& e) {
							return std::get<0>(e) == "???";
						})));
					m_PendingScoreboard.Rename(m_PendingScoreIndex, m_PendingName);
				}
				else
				{
					m_UiState = UiState::Game;
					m_Sim.ResetGame();
				}
			}
		} [[fallthrough]];
		// Intentional fall through
		case UiState::Game:
		{
			if (GetKey(olc::ESCAPE).bPressed)
			{
				m_UiOverlayState = UiOverlayState::Exit;
			}
			else if (GetKey(olc::OEM_2).bPressed) // Slash/question mark
			{
				m_UiOverlayState = UiOverlayState::About;
			}
		} break;
		case UiState::ScoreboardEntry:
		{
			if (GetKey(olc::ESCAPE).bPressed)
			{
				m_UiOverlayState = UiOverlayState::Exit;
			}

			m_UiIndex = (m_UiIndex + 3
				- static_cast<int>(GetKey(olc::LEFT).bPressed)
				+ static_cast<int>(GetKey(olc::RIGHT).bPressed)
				) % 3;

			auto upKey = GetKey(olc::UP);
			auto downKey = GetKey(olc::DOWN);
			if (upKey.bPressed)
			{
				RotateScoreboardCharacter(-1);
				m_UiRepeatDelay = 0.25f;
			}
			else if (downKey.bPressed)
			{
				RotateScoreboardCharacter(1);
				m_UiRepeatDelay = 0.25f;
			}
			else if (upKey.bHeld || downKey.bHeld)
			{
				m_UiRepeatDelay -= fElapsedTime;
				if (m_UiRepeatDelay < 0)
				{
					m_UiRepeatDelay = 0.05f;
					RotateScoreboardCharacter(upKey.bHeld ? -1 : 1);
				}
			}
			else
			{
				m_UiRepeatDelay = -1.0f;
			}

			if (GetKey(olc::ENTER).bPressed)
			{
				m_ScoreBoard.SetScore(
					m_PendingName,
					m_Sim.GetScore(),
					m_Sim.GetLevel());
				m_UiState = UiState::Game;
				m_Sim.ResetGame();
			}
		} break;
		}
	} break;
	}

	if (m_UiOverlayState == UiOverlayState::None && m_UiState == UiState::Game)
	{
		auto input = GetInput();
		m_Sim.Update(fElapsedTime, input);
		if (m_Sim.IsGameOver())
		{
			m_UiState = UiState::GameOver;
		}
	}

	Draw();

	return !m_bExiting;
}
void App::RotateScoreboardCharacter(int direction)
{
	if (m_UiIndex < 0 || m_UiIndex >= m_PendingName.size())
	{
		return;
	}
	char ch = (m_PendingName[m_UiIndex] - 'A' + direction + 26) % 26;
	m_PendingName[m_UiIndex] = 'A' + ch;
	m_PendingScoreboard.Rename(m_PendingScoreIndex, m_PendingName);
}

void App::Draw()
{
	Clear(olc::VERY_DARK_GREY);

	DrawUI();

	if (m_UiOverlayState == UiOverlayState::About)
	{
		DrawAbout();
	}
	else if (m_UiOverlayState == UiOverlayState::Exit)
	{
		DrawExit();
	}
	else if (m_UiState == UiState::ScoreboardEntry)
	{
		DrawScoreboard();
	}
	else
	{
		DrawTiles();
		if (m_UiState == UiState::GameOver)
		{
			DrawGameOver();
		}
	}
}

void App::DrawScoreboard()
{
	DrawString(s_BoardLeft + 60, s_UiTop + 15, "SCORES", olc::WHITE, 3);

	static constexpr int s_ScoresTop = s_UiTop + 75;
	static constexpr int s_NameLeft = s_BoardLeft + 45;
	static constexpr int s_ScoreLeft = s_BoardLeft + 110;
	static constexpr int s_ScoreLineHeight = 24;
	static constexpr int s_ScoreCharWidth = 16;
	static constexpr int s_ScoreNumberDigits = 7;

	auto const& scores = m_PendingScoreboard.GetScoreList();
	int i = 0;
	for (auto [iter, i] = std::tuple{ scores.begin(), 0 };
		iter != scores.end();
		++iter, ++i)
	{
		bool isCurrent = i == m_PendingScoreIndex;
		olc::Pixel color = (isCurrent) ? olc::YELLOW : olc::WHITE;

		int rowTop = s_ScoresTop + (i * s_ScoreLineHeight);
		DrawString(	
			{ s_NameLeft, rowTop }, std::get<0>(*iter), color, 2);
		std::string points = std::to_string(std::get<1>(*iter));
		int scoreLeft = s_ScoreLeft + (s_ScoreCharWidth * (
			s_ScoreNumberDigits - static_cast<int>(points.size())));
		DrawString(
			{ scoreLeft, rowTop }, points, color, 2);

		if (isCurrent)
		{
			int left = s_NameLeft - 2 + (16 * m_UiIndex);
			int top = rowTop - 3;
			DrawRect(left, top, 17, 19, olc::YELLOW);
		}
	}
}

void App::DrawGameOver()
{
	olc::vi2d pos{ s_BoardLeft + 20, s_UiTop + (s_BoardTileHeightPx / 2) - 40};
	DrawStringDecal(pos, "GAME OVER\n  Press\n [Enter]", olc::WHITE, { 3, 3 });
}

void App::DrawAbout()
{
	using namespace olc;
	
	static constexpr int s_AboutTop{ s_UiTop + 100 };
	DrawString(s_BoardLeft + 50, s_AboutTop, "Block Drop", olc::WHITE, 2);
	DrawString(s_BoardLeft + 60, s_AboutTop + 25, "By Owen Raccuglia", olc::WHITE, 1);
	DrawString(s_BoardLeft + 55, s_AboutTop + 35, "and Paul Raccuglia", olc::WHITE, 1);

	DrawString(s_BoardLeft + 7, s_AboutTop + 150, "olcPixelGameEngine is Copyright\n 2018 - 2024 OneLoneCoder.com", olc::WHITE, 1);

	DrawString(s_BoardLeft + 6, s_AboutTop + 285, "tile.png is MIT License,\ngithub.com/andrew-wilkes/tetron", olc::WHITE, 1);
}

void App::DrawExit()
{
	using namespace olc;

	static constexpr int s_ExitTop{ s_UiTop + 200 };
	DrawString(s_BoardLeft + 60, s_ExitTop, "Exit?", olc::WHITE, 4);

	DrawString(s_BoardLeft + 45, s_ExitTop + 65, "Press ENTER", olc::WHITE, 2);
	DrawString(s_BoardLeft + 77, s_ExitTop + 90, "to exit", olc::WHITE, 2);
}


void App::DrawUI()
{
	using namespace olc;

	vi2d size{ s_BoardTileWidthPx, s_BoardTileHeightPx };
	vi2d topRight{ m_BoardTopLeft.x + s_BoardTileWidthPx, m_BoardTopLeft.y };
	vi2d bottomRight{ m_BoardTopLeft + size };

	// Sidebar constants
	static constexpr int s_SidebarWidth{ 110 };
	static constexpr int s_SidebarPreviewHeight{ 100 };
	static constexpr int s_SidebarNumbersTop{ s_UiTop + s_SidebarPreviewHeight + 25 };
	static constexpr int s_SidebarNumbersTextTop{ s_SidebarNumbersTop + 8 };
	static constexpr int s_SidebarNumbersLineHeight{ 24 };
	static constexpr int s_SidebarNumbersHeight{ 102 };
	static constexpr int s_SidebarLeft{ s_BoardRight + 25 };
	static constexpr int s_SidebarRight{ s_SidebarLeft + s_SidebarWidth };
	static constexpr int s_SidebarNumbersLeft{ s_SidebarLeft + 10 };
	static constexpr int s_SidebarInstructionsHelpTop = s_SidebarNumbersTop + s_SidebarNumbersHeight + 25;
	static constexpr int s_SidebarInstructionsHelpHeight = 268;
	static constexpr int s_SidebarHelpStrTop = s_SidebarInstructionsHelpTop + 7;
	static constexpr int s_SidebarHelpLeft = 337;

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

void App::DrawBorder(olc::vi2d const& topLeft, olc::vi2d size, int borderWidth, olc::Pixel color)
{
	size -= olc::vi2d{ 1, 1 };
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
