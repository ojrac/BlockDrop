#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#include <time.h>
#include <stdint.h>
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

public:
	App()
		: m_Sim(BOARD_TILE_WIDTH, BOARD_TILE_HEIGHT)
		, m_BoardTopLeft{
			(SCREEN_WIDTH_PX - BOARD_TILE_WIDTH_PX) / 2,
			(SCREEN_HEIGHT_PX - BOARD_TILE_HEIGHT_PX) / 2,
		}
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

	olc::vi2d m_BoardTopLeft{};

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

		// Border
		{
			constexpr int borderPx{ 3 };
			for (int i = 1; i <= borderPx; ++i)
			{
				vi2d borderOffset{ i, i };
				DrawRect(m_BoardTopLeft - borderOffset, size + borderOffset + borderOffset, olc::GREY);
			}
		}

		// Board
		FillRect(m_BoardTopLeft, size, olc::BLACK);
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
		return m_BoardTopLeft + olc::vi2d{ column * TILE_SIZE_PX, row * TILE_SIZE_PX };
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
			auto color = GetColor(tetronimo.GetTileColor());
			auto position = tetronimo.GetPosition();
			for (auto& square : tetronimo.GetSquares())
			{
				int row = position.y + square.m_Row;
				int col = position.x + square.m_Column;
				DrawTile(row, col, color);
			}

			// Drop preview
			position = m_Sim.GetDropPosition();
			for (auto& square : tetronimo.GetSquares())
			{
				int row = position.y + square.m_Row;
				int col = position.x + square.m_Column;
				DrawTileOutline(row, col, square.m_Directions, color);
			}
		}
	}

	void DrawTile(int row, int col, olc::Pixel const& color)
	{
		if (row < 0 || row > BOARD_TILE_HEIGHT || col < 0 || col > BOARD_TILE_WIDTH)
		{
			return;
		}
		DrawDecal(BoardToScreen(row, col), m_TileDecal.get(), { 1, 1 }, color);
	}

	void DrawTileOutline(int row, int col, BorderDirection directions, olc::Pixel const& color)
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
		result.bLeft = GetKey(olc::Key::LEFT).bHeld;
		result.bRight = GetKey(olc::Key::RIGHT).bHeld;
		result.bRotateLeft = GetKey(olc::Key::Q).bPressed;
		result.bRotateRight = GetKey(olc::Key::E).bPressed || GetKey(olc::Key::UP).bPressed;
		result.bDrop = GetKey(olc::Key::DOWN).bPressed;

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