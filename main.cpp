#include "Game.h"

int main()
{
	BlockDrop::App app;
	if (app.Construct(BlockDrop::App::ScreenWidthPx, BlockDrop::App::s_ScreenHeightPx, 1, 1, false, true))
	{
		app.Start();
	}

	return 0;
}