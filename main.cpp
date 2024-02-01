#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#include <stdint.h>
#include <string>
#include <time.h>
#include "Sim.h"

namespace BlockDrop
{

class App : public olc::PixelGameEngine
{
public:
	static constexpr int TILE_SIZE_PX = 26;

	static constexpr int BOARD_TILE_WIDTH = 10;
	static constexpr int BOARD_TILE_HEIGHT = 20;

	static constexpr int BOARD_TILE_WIDTH_PX = 10 * TILE_SIZE_PX;
	static constexpr int BOARD_TILE_HEIGHT_PX = 20 * TILE_SIZE_PX;

	static constexpr int SCREEN_WIDTH_PX = 480;
	static constexpr int SCREEN_HEIGHT_PX = 640;

	// UI element anchors
	static constexpr int k_UiTop{ (SCREEN_HEIGHT_PX - BOARD_TILE_HEIGHT_PX) / 2 };
	static constexpr int k_BoardLeft{ 25 };
	static constexpr int k_BoardRight{ k_BoardLeft + BOARD_TILE_WIDTH_PX + 25 };
	static constexpr int k_SidebarWidth{ 110 };
	static constexpr int k_SidebarPreviewHeight{ 100 };
	static constexpr int k_SidebarNumbersTop{ k_UiTop + k_SidebarPreviewHeight + 25 };
	static constexpr int k_SidebarNumbersTextTop{ k_SidebarNumbersTop + 8 };
	static constexpr int k_SidebarNumbersLineHeight{ 24 };
	static constexpr int k_SidebarNumbersHeight{ 102 };
	static constexpr int k_SidebarLeft{ k_BoardRight + 25 };
	static constexpr int k_SidebarRight{ k_SidebarLeft + k_SidebarWidth };
	static constexpr int k_SidebarNumbersLeft{ k_SidebarLeft + 10 };

public:
	App()
		: m_Sim(BOARD_TILE_WIDTH, BOARD_TILE_HEIGHT)
	{
		srand(static_cast<uint32_t>(time(nullptr)));
		sAppName = "BlockDrop";
	}

	bool OnUserCreate() override
	{
		m_TileSprite = std::make_unique<olc::Sprite>("tile.png");
		m_TileDecal = std::make_unique<olc::Decal>(m_TileSprite.get());

		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		if (GetKey(olc::ESCAPE).bPressed)
		{
			return false;
		}

		auto input = GetInput();
		m_Sim.Update(fElapsedTime, input);

		Draw();

		return true;
	}

private:
	std::unique_ptr<olc::Sprite> m_TileSprite{};
	std::unique_ptr<olc::Decal> m_TileDecal{};

	olc::vi2d m_BoardTopLeft{ k_BoardLeft, k_UiTop };

	Sim m_Sim;

private:
	void Draw()
	{
		Clear(olc::VERY_DARK_GREY);

		DrawUI();

		DrawTiles();
	}

	void DrawUI()
	{
		using namespace olc;

		vi2d size{ BOARD_TILE_WIDTH_PX, BOARD_TILE_HEIGHT_PX };
		vi2d topRight{ m_BoardTopLeft.x + BOARD_TILE_WIDTH_PX, m_BoardTopLeft.y };
		vi2d bottomRight{ m_BoardTopLeft + size };

		// Sidebar: Preview
		DrawBorder({ k_SidebarLeft, k_UiTop }, { k_SidebarWidth, k_SidebarPreviewHeight }, 3, olc::GREY);
		FillRect({ k_SidebarLeft, k_UiTop }, { k_SidebarWidth, k_SidebarPreviewHeight }, olc::BLACK);
		// Preview Tile
		auto* tetronimo = TetronimoFactory::GetTetronimoByColor(m_Sim.GetNextBlockColor());
		if (tetronimo != nullptr)
		{
			olc::vi2d origin{
				k_SidebarLeft + (k_SidebarWidth / 2),
				k_UiTop + (k_SidebarPreviewHeight / 2) - TILE_SIZE_PX,
			};
			origin += tetronimo->m_CenterOffset * TILE_SIZE_PX;

			DrawTetronimoSquares(origin, tetronimo->m_Color, tetronimo->m_RotatedTileOffsets[0]);
		}

		// Sidebar: Level and Score
		DrawBorder({ k_SidebarLeft, k_SidebarNumbersTop }, { k_SidebarWidth, k_SidebarNumbersHeight }, 3, olc::GREY);
		FillRect({ k_SidebarLeft, k_SidebarNumbersTop }, { k_SidebarWidth, k_SidebarNumbersHeight }, olc::BLACK);

		DrawString(k_SidebarNumbersLeft, k_SidebarNumbersTextTop,
			"Level:", olc::WHITE, 2);
		DrawString(k_SidebarNumbersLeft, k_SidebarNumbersTextTop + k_SidebarNumbersLineHeight,
			std::to_string(m_Sim.GetLevel()), olc::WHITE, 2);
		DrawString(k_SidebarNumbersLeft, k_SidebarNumbersTextTop + 2 * k_SidebarNumbersLineHeight,
			"Score:", olc::WHITE, 2);
		DrawString(k_SidebarNumbersLeft, k_SidebarNumbersTextTop + 3 * k_SidebarNumbersLineHeight,
			std::to_string(m_Sim.GetScore()), olc::WHITE, 2);

		// Board
		DrawBorder(m_BoardTopLeft, size, 3, olc::GREY);
		FillRect(m_BoardTopLeft, size, olc::BLACK);
	}

	void DrawBorder(olc::vi2d const& topLeft, olc::vi2d const& size, int borderWidth, olc::Pixel color)
	{
		for (int i = 1; i <= borderWidth; ++i)
		{
			olc::vi2d borderOffset{ i, i };
			DrawRect(topLeft - borderOffset, size + borderOffset + borderOffset, olc::GREY);
		}
	}

	static olc::Pixel GetColor(TileColor color)
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

	olc::vi2d BoardToScreen(int row, int column) const
	{
		return BoardToScreen({ column, row });
	}
	olc::vi2d BoardToScreen(olc::vi2d const& pos) const
	{
		return m_BoardTopLeft + (pos * TILE_SIZE_PX);
	}

	void DrawTiles()
	{
		using namespace olc;

		// Board tiles
		using namespace olc;
		for (int row = 0; row < BOARD_TILE_HEIGHT; ++row)
		{
			for (int col = 0; col < BOARD_TILE_WIDTH; ++col)
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
				DrawTileOutline(row, col, square.m_Directions, color);
			}
		}
	}

	void DrawTetronimo(TetronimoInstance const& tetronimo, olc::vi2d origin)
	{
		DrawTetronimoSquares(origin, tetronimo.GetTileColor(), tetronimo.GetSquares());
	}

	void DrawTetronimoSquares(olc::vi2d origin, TileColor tileColor, std::vector<TetronimoSquare> const& squares)
	{
		auto color = GetColor(tileColor);
		for (auto& square : squares)
		{
			olc::vi2d pos = origin + olc::vi2d{ square.m_Column * TILE_SIZE_PX, square.m_Row * TILE_SIZE_PX };
			DrawTileAtPixel(pos, color);
		}
	}

	void DrawTile(int row, int col, olc::Pixel color)
	{
		if (row < 0 || row > BOARD_TILE_HEIGHT || col < 0 || col > BOARD_TILE_WIDTH)
		{
			return;
		}
		DrawTileAtPixel(BoardToScreen(row, col), color);
	}

	void DrawTileAtPixel(olc::vi2d pos, olc::Pixel color)
	{
		DrawDecal(pos, m_TileDecal.get(), { 1, 1 }, color);

	}

	void DrawTileOutline(int row, int col, BorderDirection directions, olc::Pixel color)
	{
		if (row < 0 || row > BOARD_TILE_HEIGHT || col < 0 || col > BOARD_TILE_WIDTH)
		{
			return;
		}

		olc::vi2d topLeft = BoardToScreen(row, col);
		olc::vi2d bottomRight{ topLeft.x + TILE_SIZE_PX - 1, topLeft.y + TILE_SIZE_PX - 1 };

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

	Input GetInput()
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
};

}

int main()
{
	BlockDrop::App app;
	if (app.Construct(BlockDrop::App::SCREEN_WIDTH_PX, BlockDrop::App::SCREEN_HEIGHT_PX, 1, 1, false, true))
	{
		app.Start();
	}

	return 0;
}