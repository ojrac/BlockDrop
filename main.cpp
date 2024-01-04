#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

class BlockDrop : public olc::PixelGameEngine
{
public:
	static constexpr int TILE_SIZE_PX = 26;

	static constexpr int BOARD_TILE_WIDTH = 10;
	static constexpr int BOARD_TILE_HEIGHT = 20;

	static constexpr int BOARD_TILE_WIDTH_PX = 10 * TILE_SIZE_PX;
	static constexpr int BOARD_TILE_HEIGHT_PX = 20 * TILE_SIZE_PX;

	static constexpr int SCREEN_WIDTH_PX = 460;
	static constexpr int SCREEN_HEIGHT_PX = 640;

public:
	BlockDrop()
	{
		sAppName = "Example";
	}

public:
	bool OnUserCreate() override
	{
		// Called once at the start, so create things here
		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		Clear(olc::VERY_DARK_GREY);

		int x0 = (SCREEN_WIDTH_PX - BOARD_TILE_WIDTH_PX) / 2;
		int y0 = (SCREEN_HEIGHT_PX - BOARD_TILE_HEIGHT_PX) / 2;
		int x1 = x0 + BOARD_TILE_WIDTH_PX;
		int y1 = y0 + BOARD_TILE_HEIGHT_PX;
		DrawLine(x0, y0, x1, y0, olc::YELLOW);
		DrawLine(x0, y1, x1, y1, olc::YELLOW);
		DrawLine(x0, y0, x0, y1, olc::YELLOW);
		DrawLine(x1, y0, x1, y1, olc::YELLOW);

		return true;
	}
};


int main()
{
	BlockDrop app;
	if (app.Construct(460, 640, 1, 1, false, true))
	{
		app.Start();
	}

	return 0;
}