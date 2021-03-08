// Fill out your copyright notice in the Description page of Project Settings.

#include "Test/ChessUnitTests.h"

#include "../../Core/Board.h"
#include "../../Core/MoveGeneration.h"

#include <chrono>
using namespace std::chrono;
using namespace Chess;

DEFINE_LOG_CATEGORY_STATIC(LogChessTest, Log, All);

int MoveGenerationTest(Board& board, int depth)
{
	if (depth == 0)
	{
		return 1;
	}

	std::vector<Chess::Move> movesThisPly = MoveGeneration::GenerateMoves(board.BoardState, board.GetColourToMove());
	int numPositions = 0;
	for (Chess::Move& move : movesThisPly)
	{
		board.MakeMove(move);
		numPositions += MoveGenerationTest(board, depth - 1);
		board.UnmakeMove();
	}

	return numPositions;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(ChessUnitTests, "ChessTest.DepthTest.Shannon Number", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool ChessUnitTests::RunTest(const FString& Parameters)
{
	static const int64 ShannonNumber[] = { 20, 400, 8902, 197281, 4865609, 4865609, 119060324, 3195901860, 84998978956, 2439530234167, 69352859712417 };
	static const int MaxDepth = 5;

	Board board;
	for (int depth = 0; depth < MaxDepth; depth++)
	{
		auto start = high_resolution_clock::now();
		int64 permutations = MoveGenerationTest(board, depth + 1);
		auto stop = high_resolution_clock::now();

		UE_LOG(LogChessTest, Display, TEXT("Found %d moves at depth %d in %d ms"), permutations, depth, duration_cast<milliseconds>(stop - start).count());
		if (permutations != ShannonNumber[depth])
		{
			UE_LOG(LogChessTest, Error, TEXT("Expected %d moves!"), ShannonNumber[depth]);
			return false;
		}
	}

	return true;
}