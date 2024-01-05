#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
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
	{
		sAppName = "BlockDrop";
	}

	bool OnUserCreate() override
	{
		m_TileSprite = std::make_unique<olc::Sprite>("tile.png");
		m_TileDecal = std::make_unique<olc::Decal>(m_TileSprite.get());

		m_Sim.Set(0, 0, TileColor::Red);
		m_Sim.Set(19, 9, TileColor::Cyan);

		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		Draw();

		return true;
	}

private:
	std::unique_ptr<olc::Sprite> m_TileSprite {};
	std::unique_ptr<olc::Decal> m_TileDecal {};

	Sim m_Sim;

private:
	void Draw()
	{
		Clear(olc::VERY_DARK_GREY);

		DrawUI();
	}
	void DrawUI()
	{
		using namespace olc;

		vi2d size{ BOARD_TILE_WIDTH_PX, BOARD_TILE_HEIGHT_PX };
		vi2d topLeft{
			(SCREEN_WIDTH_PX - BOARD_TILE_WIDTH_PX) / 2,
			(SCREEN_HEIGHT_PX - BOARD_TILE_HEIGHT_PX) / 2,
		};
		vi2d topRight{ topLeft.x + BOARD_TILE_WIDTH_PX, topLeft.y };
		vi2d bottomRight{ topLeft + size };

		// Border
		{
			constexpr int borderPx{ 3 };
			for (int i = 1; i <= borderPx; ++i)
			{
				vi2d borderOffset{ i, i };
				DrawRect(topLeft - borderOffset, size + borderOffset + borderOffset, olc::GREY);
			}
		}

		// Board
		FillRect(topLeft, size, olc::BLACK);

		// Tiles
		for (int row = 0; row < BOARD_TILE_HEIGHT; ++row)
		{
			for (int col = 0; col < BOARD_TILE_WIDTH; ++col)
			{
				const auto tile = m_Sim.At(row, col);
				if (tile == TileColor::None)
				{
					continue;
				}

				Pixel color;
				switch (tile)
				{
				case TileColor::Red:
					color = olc::RED;
					break;
				case TileColor::Blue:
					color = olc::BLUE;
					break;
				case TileColor::Cyan:
					color = olc::CYAN;
					break;
				case TileColor::Magenta:
					color = olc::MAGENTA;
					break;
				case TileColor::Yellow:
					color = olc::YELLOW;
					break;
				case TileColor::Green:
					color = olc::GREEN;
					break;
				default:
					assert(0);
					break;
				}

				DrawDecal(topLeft + vi2d{ col * TILE_SIZE_PX, row * TILE_SIZE_PX }, m_TileDecal.get(), { 1, 1 }, color);
			}

		}

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