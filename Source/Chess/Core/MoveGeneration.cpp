#include "MoveGeneration.h"

#include "Utils.h"

#include <assert.h>

namespace Chess
{
	using namespace Constants;

	std::vector<Move> MoveGeneration::GenerateMoves(const State& board, int8 colour, bool calculateThreat /*= false*/)
	{
		//TODO: This is really slow. Fix
		std::vector<Move> moves = GenerateMoves_Impl(board, colour, calculateThreat);
		if (!calculateThreat)
		{
			PruneIllegalMoves_Impl(board, colour, moves);
		}

		return moves;
	}

	void MoveGeneration::PruneIllegalMoves_Impl(const State& board, int8 colour, std::vector<Move>& moves)
	{
		for (int idx = moves.size() - 1; idx >= 0; idx--)
		{
			if (board.DoesMoveExposeKing(moves[idx]))
			{
				moves.erase(moves.begin() + idx);
			}
		}
	}

	void MoveGeneration::GenerateSlidingMoves(const State& state, int8 startSquare, std::vector<Move>& moves)
	{
		int8 piece = state.Squares[startSquare];
		int8 friendlyColour = Utils::GetColour(piece);
		int8 enemyColour = friendlyColour == Piece::White ? Piece::Black : Piece::White;

		bool isRook = Utils::IsType(piece, Piece::Rook);
		int8 startDirIdx = Utils::IsType(piece, Piece::Bishop) ? 4 : 0;
		int8 endDirIdx = isRook ? 4 : 8;

		for (int8 directionIndex = startDirIdx; directionIndex < endDirIdx; directionIndex++)
		{
			for (int8 n = 0; n < NumSquaresToEdge[startSquare][directionIndex]; n++)
			{
				int8 targetSquare = startSquare + DirectionOffsets[directionIndex] * (n + 1);
				int8 pieceOnTargetSquare = state.Squares[targetSquare];

				if (Utils::IsColour(pieceOnTargetSquare, friendlyColour))
				{
					break;
				}

				if (isRook)
				{
					int8 prevented = Utils::RankIndex(0) ? Castling::Queenside : Castling::Kingside;
					moves.push_back(Move::CreateMove(state, startSquare, targetSquare, friendlyColour, prevented));
				}
				else
				{
					moves.push_back(Move::CreateMove(state, startSquare, targetSquare, friendlyColour));
				}

				if (Utils::IsColour(pieceOnTargetSquare, enemyColour))
				{
					break;
				}
			}
		}
	}

	void MoveGeneration::GenerateKnightMoves(const State& state, int8 startSquare, std::vector<Move>& moves)
	{
		const int8 KnightOffsets[8] = { 15, 17, -17, -15, 10, -6, 6, -10 };

		int8 piece = state.Squares[startSquare];
		int8 friendlyColour = Utils::GetColour(piece);
		int8 enemyColour = friendlyColour == Piece::White ? Piece::Black : Piece::White;

		for (int knightMoveIdx = 0; knightMoveIdx < 8; knightMoveIdx++)
		{
			int8 targetSquare = startSquare + KnightOffsets[knightMoveIdx];
			if (targetSquare < 0 || targetSquare >= 64)
			{
				continue;
			}

			//Reject jumps that wrap around the edge of the board
			int8 y = startSquare / 8;
			int8 x = startSquare - y * 8;

			int8 targetY = targetSquare / 8;
			int8 targetX = targetSquare - targetY * 8;
			//If you've moved further than 2 on x/y (when coords are 0-8) you've wrapped the board
			if (std::max(std::abs(x - targetX), std::abs(y - targetY)) != 2)
			{
				continue;
			}

			if (Utils::IsColour(state.Squares[targetSquare], friendlyColour))
			{
				continue;
			}

			moves.push_back(Move::CreateMove(state, startSquare, targetSquare, friendlyColour));
		}
	}

	void MoveGeneration::GeneratePawnMoves(const State& state, int8 startSquare, std::vector<Move>& moves)
	{
		const int8 PawnOffsets[2][2]
		{
			{ 8, 16 }, //White
			{-8, -16}  //Black
		};

		int8 piece = state.Squares[startSquare];
		int8 colour = Utils::GetColour(piece);
		int8 colourIdx = Utils::IsColour(piece, Piece::White) ? 0 : 1;
		int8 startingRank = Utils::IsColour(piece, Piece::White) ? 1 : 6;
		int8 backRank = Utils::IsColour(piece, Piece::White) ? 7 : 0;
		int MovesAvailable = Utils::RankIndex(startSquare) == startingRank ? 2 : 1;

		for (int moveIdx = 0; moveIdx < MovesAvailable; moveIdx++)
		{
			int8 targetSquare = startSquare + PawnOffsets[colourIdx][moveIdx];
			if (state.Squares[targetSquare] != Piece::None)
			{
				break;
			}

			if (Utils::RankIndex(targetSquare) == backRank)
			{
				for (int8 promo : Constants::Promotions)
				{
					moves.push_back(Move::CreatePromotionMove(state, startSquare, targetSquare, colour, promo));
				}
			}
			else
			{
				if (moveIdx == 1)
				{
					moves.push_back(Move::CreateEnPassentMove(state, startSquare, targetSquare, colour));
				}
				else
				{
					moves.push_back(Move::CreateMove(state, startSquare, targetSquare, colour));
				}
			}
		}
	}

	void MoveGeneration::GeneratePawnAttacks(const State& state, int8 startSquare, std::vector<Move>& moves, bool calculateThreat)
	{
		const int8 PawnOffsets[2][3]
		{
			{ 7, 9 , 8}, //White
			{ -9, -7, -8 } //Black
		};

		int8 piece = state.Squares[startSquare];
		int8 colour = Utils::GetColour(piece);
		int8 colourIdx = Utils::IsColour(piece, Piece::White) ? 0 : 1;
		int8 enemyColour = Utils::IsColour(piece, Piece::White) ? Piece::Black : Piece::White;
		int8 file = Utils::FileIndex(startSquare);

		for (int moveIdx = 0; moveIdx < 2; moveIdx++)
		{
			//Make sure pawns on the 0th file can't attack left & the 7th can't attack right
			if ((file == 0 && moveIdx == 0) || file == 7 && moveIdx == 1)
			{
				continue;
			}

			int8 targetSquare = startSquare + PawnOffsets[colourIdx][moveIdx];

			int8 passentPawn = state.EnPassentTarget + PawnOffsets[colourIdx == 0 ? 1 : 0][2];
			if (state.EnPassentTarget == targetSquare && Utils::IsColour(state.Squares[passentPawn], enemyColour))
			{
				moves.push_back(Move::CreateEnPassentCapture(state, startSquare, targetSquare, colour, passentPawn, state.EnPassentTarget));
				continue;
			}

			if (calculateThreat || Utils::IsColour(state.Squares[targetSquare], enemyColour))
			{
				moves.push_back(Move::CreateMove(state, startSquare, targetSquare, colour));
			}
		}
	}

	void MoveGeneration::GenerateKingMoves(const State& state, int8 startSquare, std::vector<Move>& moves, bool calculateThreat /* = false*/)
	{
		int8 piece = state.Squares[startSquare];
		int8 friendlyColour = Utils::GetColour(piece);
		int8 enemyColour = friendlyColour == Piece::White ? Piece::Black : Piece::White;

		for (int8 directionIndex = 0; directionIndex < 8; directionIndex++)
		{
			int8 targetSquare = startSquare + DirectionOffsets[directionIndex];
			int8 pieceOnTargetSquare = state.Squares[targetSquare];

			if (targetSquare < 0 || targetSquare >= 64)
			{
				continue;
			}

			if (Utils::IsColour(pieceOnTargetSquare, friendlyColour))
			{
				continue;
			}

			if (!state.IsSquareThreatened(targetSquare, friendlyColour))
			{
				moves.push_back(Move::CreateMove(state, startSquare, targetSquare, friendlyColour, Castling::Kingside | Castling::Queenside));
			}
		}

		//Castling doesn't generate threat on it's own, i.e. it's not an implicitly attacking move
		//The threat comes from the subsequent position of the pieces so it can be ignored for this purpose
		if (!calculateThreat)
		{
			int8 castleAvailability = friendlyColour == Piece::White ? state.WhiteCastleAvailable : state.BlackCastleAvailable;
			if ((castleAvailability & Castling::Kingside) == Castling::Kingside)
			{
				int8 startIdx = friendlyColour == Piece::White ? 5 : 61;
				int8 endIdx = friendlyColour == Piece::White ? 6 : 62;
				bool castlingBlocked = false;
				for (int idx = startIdx; idx < endIdx; idx++)
				{
					if (state.Squares[idx] != Piece::None || state.IsSquareThreatened(idx, friendlyColour))
					{
						castlingBlocked = true;
						break;
					}
				}

				if (!castlingBlocked)
				{
					moves.push_back(Move::CreateCastlingMove(state, friendlyColour, Castling::Kingside));
				}
			}

			if (((castleAvailability & Castling::Queenside) == Castling::Queenside))
			{
				int8 startIdx = friendlyColour == Piece::White ? 3 : 59;
				int8 endIdx = friendlyColour == Piece::White ? 2 : 57;
				bool castlingBlocked = false;

				for (int idx = endIdx; idx < startIdx; idx++)
				{
					if (state.Squares[idx] != Piece::None || state.IsSquareThreatened(idx, friendlyColour))
					{
						castlingBlocked = true;
						break;
					}
				}

				if (!castlingBlocked)
				{
					moves.push_back(Move::CreateCastlingMove(state, friendlyColour, Castling::Queenside));
				}
			}
		}
	}

	std::vector<Move> MoveGeneration::GenerateMoves_Impl(const State& board, int8 colour, bool calculateThreat /*= false*/)
	{
		std::vector<Move> moves;
		for (int8 startSquare = 0; startSquare < 64; startSquare++)
		{
			int8 piece = board.Squares[startSquare];

			if (Utils::IsColour(piece, colour))
			{
				if (Utils::IsSlidingPiece(piece))
				{
					GenerateSlidingMoves(board, startSquare, moves);
				}

				if (Utils::IsType(piece, Piece::Knight))
				{
					GenerateKnightMoves(board, startSquare, moves);
				}

				if (Utils::IsType(piece, Piece::Pawn))
				{
					if (calculateThreat)
					{
						GeneratePawnAttacks(board, startSquare, moves, calculateThreat);
					}
					else
					{
						GeneratePawnMoves(board, startSquare, moves);
						GeneratePawnAttacks(board, startSquare, moves);
					}
				}

				if (Utils::IsType(piece, Piece::King))
				{
					GenerateKingMoves(board, startSquare, moves, calculateThreat);
				}
			}
		}

		return moves;
	}
}