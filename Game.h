#pragma once
#ifndef BLOCKDROP_GAME_H
#define BLOCKDROP_GAME_H

#include "olcPixelGameEngine.h"

#include "Sim.h"

namespace BlockDrop
{

class App : public olc::PixelGameEngine
{
public:
	static constexpr int s_TileSizePx = 26;

	static constexpr int s_BoardTileWidth = 10;
	static constexpr int s_BoardTileHeight = 20;

	static constexpr int s_BoardTileWidthPx = 10 * s_TileSizePx;
	static constexpr int s_BoardTileHeightPx = 20 * s_TileSizePx;

	static constexpr int ScreenWidthPx = 480;
	static constexpr int s_ScreenHeightPx = 640;

	// UI element anchors
	static constexpr int s_UiTop{ (s_ScreenHeightPx - s_BoardTileHeightPx) / 2 };
	static constexpr int s_BoardLeft{ 25 };
	static constexpr int s_BoardRight{ s_BoardLeft + s_BoardTileWidthPx + 25 };

public:
	App()
		: m_Sim(s_BoardTileWidth, s_BoardTileHeight)
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

	bool OnUserUpdate(float fElapsedTime) override;

private:
	std::unique_ptr<olc::Sprite> m_TileSprite{};
	std::unique_ptr<olc::Decal> m_TileDecal{};

	olc::vi2d m_BoardTopLeft{ s_BoardLeft, s_UiTop };

	bool m_bAboutOpen{ false };

	Sim m_Sim;

private:
	void Draw();

	void DrawAbout();

	void DrawUI();

	void DrawBorder(olc::vi2d const& topLeft, olc::vi2d size, int borderWidth, olc::Pixel color);

	static olc::Pixel GetColor(TileColor color);

	olc::vi2d BoardToScreen(int row, int column) const
	{
		return BoardToScreen({ column, row });
	}
	olc::vi2d BoardToScreen(olc::vi2d const& pos) const
	{
		return m_BoardTopLeft + (pos * s_TileSizePx);
	}

	void DrawTiles();

	void DrawTetronimo(TetronimoInstance const& tetronimo, olc::vi2d origin)
	{
		DrawTetronimoSquares(origin, tetronimo.GetTileColor(), tetronimo.GetSquares());
	}

	void DrawTetronimoSquares(olc::vi2d origin, TileColor tileColor, std::vector<TetronimoSquare> const& squares);

	void DrawTile(int row, int col, olc::Pixel color);

	void DrawTileAtPixel(olc::vi2d pos, olc::Pixel color)
	{
		DrawDecal(pos, m_TileDecal.get(), { 1, 1 }, color);
	}

	void DrawTileOutline(int row, int col, BorderDirection directions, olc::Pixel color);

	Input GetInput();
};

}

#endif