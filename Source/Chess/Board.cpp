// Fill out your copyright notice in the Description page of Project Settings.

#include "Board.h"

std::map<char, int8> Board::PieceTypeFromSymbol = {
	{'k', Piece::King},
	{'p', Piece::Pawn},
	{'n', Piece::Knight},
	{'b', Piece::Bishop},
	{'r', Piece::Rook},
	{'q', Piece::Queen}
};

const int8 Board::DirectionOffsets[8] = { 8, -8, -1, 1, 7, -7, 9, -9 };
int8 Board::NumSquaresToEdge[64][8];

const std::string Board::StandardStartFEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

Board::Board()
{
	ReadFEN(StandardStartFEN, Squares);
	PrecomputeMoveData();
	WhiteThreatening = ThreatMap(this, Piece::White);
	BlackThreatening = ThreatMap(this, Piece::Black);
}

Board::~Board()
{
}

bool Board::MakeMove(ChessMove& move)
{
	if (IsValidMove(move))
	{
		Squares[move.TargetSquare] = Squares[move.StartSquare];
		Squares[move.StartSquare] = 0;

		if (move.PreventsCastling != Castling::None)
		{
			int8& toUpdate = move.Colour == Piece::White ? WhiteCastleAvailable : BlackCastleAvailable;
			toUpdate &= ~move.PreventsCastling;
		}

		if (move.SecondaryStart != -1)
		{
			if (move.SecondaryTarget != -1) //Target is -1 in case of en passent, a real value in the case of castling
			{
				Squares[move.SecondaryTarget] = Squares[move.SecondaryStart];
			}

			Squares[move.SecondaryStart] = 0;
		}

		if (move.Promote != Piece::None)
		{
			Squares[move.TargetSquare] = ColourToMove | move.Promote;
		}

		EnPassentTarget = move.EnPassentTarget;

		WhiteThreatening.CalculateMap(this);
		BlackThreatening.CalculateMap(this);

		ColourToMove = ColourToMove == Piece::White ? Piece::Black : Piece::White;
		return true;
	}

	return false;
}

bool Board::IsValidMove(ChessMove& move) const
{
	std::vector<ChessMove> validMoves = GenerateMoves(ColourToMove);

	auto validatedMove = std::find(validMoves.begin(), validMoves.end(), move);
	bool moveIsValid = validatedMove != validMoves.end();
	if (moveIsValid)
	{
		move = *validatedMove;
	}

	return moveIsValid;
}

std::string Board::PieceName(int8 piece) const
{
	std::string pieceName = IsColour(piece, Piece::Black) ? "Black " : "White ";
	if (IsType(piece, Piece::Pawn))
	{
		pieceName += "Pawn";
	}
	else if (IsType(piece, Piece::Rook))
	{
		pieceName += "Rook";
	}
	else if (IsType(piece, Piece::Bishop))
	{
		pieceName += "Bishop";
	}
	else if (IsType(piece, Piece::Knight))
	{
		pieceName += "Knight";
	}
	else if (IsType(piece, Piece::King))
	{
		pieceName += "King";
	}
	else if (IsType(piece, Piece::Queen))
	{
		pieceName += "Queen";
	}

	return pieceName;
}

void Board::PrecomputeMoveData()
{
	for (int8 file = 0; file < 8; file++)
	{
		for (int8 rank = 0; rank < 8; rank++)
		{
			int8 numNorth = 7 - rank;
			int8 numSouth = rank;
			int8 numWest = file;
			int8 numEast = 7 - file;

			int8 squareIdx = rank * 8 + file;

			NumSquaresToEdge[squareIdx][0] = numNorth;
			NumSquaresToEdge[squareIdx][1] = numSouth;
			NumSquaresToEdge[squareIdx][2] = numWest;
			NumSquaresToEdge[squareIdx][3] = numEast;
			NumSquaresToEdge[squareIdx][4] = std::min(numNorth, numWest);
			NumSquaresToEdge[squareIdx][5] = std::min(numSouth, numEast);
			NumSquaresToEdge[squareIdx][6] = std::min(numNorth, numEast);
			NumSquaresToEdge[squareIdx][7] = std::min(numSouth, numWest);
		}
	}
}

std::vector<ChessMove> Board::GenerateMoves(int8 colour, bool calculateThreat) const
{
	std::vector<ChessMove> moves;

	for (int8 startSquare = 0; startSquare < 64; startSquare++)
	{
		if (IsPinned(startSquare))
		{
			continue;
		}

		int8 piece = Squares[startSquare];

		if (IsColour(piece, colour))
		{
			if (IsSlidingPiece(piece))
			{
				GenerateSlidingMoves(startSquare, piece, moves);
			}

			if (IsType(piece, Piece::Knight))
			{
				GenerateKnightMoves(startSquare, piece, moves);
			}

			if (IsType(piece, Piece::Pawn))
			{
				if (calculateThreat)
				{
					GeneratePawnAttacks(startSquare, piece, moves, calculateThreat);
				}
				else
				{
					GeneratePawnMoves(startSquare, piece, moves);
					GeneratePawnAttacks(startSquare, piece, moves);
				}
			}

			if (IsType(piece, Piece::King))
			{
				GenerateKingMoves(startSquare, piece, moves);
			}
		}
	}

	return moves;
}

void Board::GenerateSlidingMoves(int8 startSquare, int8 piece, std::vector<ChessMove>& moves) const
{
	int8 friendlyColour = piece & ~Piece::ClassMask;
	int8 enemyColour = friendlyColour == Piece::White ? Piece::Black : Piece::White;

	bool isRook = IsType(piece, Piece::Rook);
	int8 startDirIdx = IsType(piece, Piece::Bishop) ? 4 : 0;
	int8 endDirIdx = isRook ? 4 : 8;

	for (int8 directionIndex = startDirIdx; directionIndex < endDirIdx; directionIndex++)
	{
		for (int8 n = 0; n < NumSquaresToEdge[startSquare][directionIndex]; n++)
		{
			int8 targetSquare = startSquare + DirectionOffsets[directionIndex] * (n + 1);
			int8 pieceOnTargetSquare = Squares[targetSquare];

			if (IsColour(pieceOnTargetSquare, friendlyColour))
			{
				break;
			}

			if (isRook)
			{
				int8 prevented = RankIndex(0) ? Castling::Queenside : Castling::Kingside;
				moves.push_back(ChessMove::CreateMove(startSquare, targetSquare, friendlyColour, prevented));
			}
			else
			{
				moves.push_back(ChessMove::CreateMove(startSquare, targetSquare, friendlyColour));
			}

			if (IsColour(pieceOnTargetSquare, enemyColour))
			{
				break;
			}
		}
	}
}

void Board::GenerateKnightMoves(int8 startSquare, int8 piece, std::vector<ChessMove>& moves) const
{
	const int8 KnightOffsets[8] = { 15, 17, -17, -15, 10, -6, 6, -10};
	
	int8 friendlyColour = piece & ~Piece::ClassMask;
	int8 enemyColour = friendlyColour == Piece::White ? Piece::Black : Piece::White;

	for (int knightMoveIdx = 0; knightMoveIdx < 8; knightMoveIdx++)
	{
		int8 targetSquare = startSquare + KnightOffsets[knightMoveIdx];
		if (targetSquare < 0 || targetSquare >= 64)
		{
			continue;
		}

		if (IsColour(Squares[targetSquare], friendlyColour))
		{
			continue;
		}

		moves.push_back(ChessMove::CreateMove(startSquare, targetSquare, friendlyColour));
	}
}

void Board::GeneratePawnMoves(int8 startSquare, int8 piece, std::vector<ChessMove>& moves) const
{
	const int8 PawnOffsets[2][2]
	{
		{ 8, 16 }, //White
		{-8, -16}  //Black
	};

	int8 colour = piece & ~Piece::ClassMask;
	int8 colourIdx = IsColour(piece, Piece::White) ? 0 : 1;
	int8 startingRank = IsColour(piece, Piece::White) ? 1 : 6;
	int8 backRank = IsColour(piece, Piece::White) ? 7 : 0;
	int MovesAvailable = RankIndex(startSquare) == startingRank ? 2 : 1;

	for (int moveIdx = 0; moveIdx < MovesAvailable; moveIdx++)
	{
		int8 targetSquare = startSquare + PawnOffsets[colourIdx][moveIdx];
		if (Squares[targetSquare] != Piece::None)
		{
			break;
		}

		if (RankIndex(targetSquare) == backRank)
		{
			for (int8 promo : Piece::AllPromotions)
			{
				moves.push_back(ChessMove::CreatePromotionMove(startSquare, targetSquare, colour, promo));
			}
		}
		else
		{
			if (moveIdx == 1)
			{
				moves.push_back(ChessMove::CreateEnPassentMove(startSquare, targetSquare, colour));
			}
			else
			{
				moves.push_back(ChessMove::CreateMove(startSquare, targetSquare, colour));
			}
		}
	}
}

void Board::GeneratePawnAttacks(int8 startSquare, int8 piece, std::vector<ChessMove>& moves, bool calculateThreat) const
{
	const int8 PawnOffsets[2][3]
	{
		{ 7, 9 , 8}, //White
		{ -9, -7, -8 } //Black
	};

	int8 colourIdx = IsColour(piece, Piece::White) ? 0 : 1;
	int8 enemyColour = IsColour(piece, Piece::White) ? Piece::Black : Piece::White;
	int8 file = FileIndex(startSquare);

	for (int moveIdx = 0; moveIdx < 2; moveIdx++)
	{
		//Make sure pawns on the 0th file can't attack left & the 7th can't attack right
		if ((file == 0 && moveIdx == 0) || file == 7 && moveIdx == 1)
		{
			continue;
		}

		int8 targetSquare = startSquare + PawnOffsets[colourIdx][moveIdx];
		
		int8 passentPawn = EnPassentTarget + PawnOffsets[colourIdx == 0 ? 1 : 0][2];
		if (EnPassentTarget == targetSquare && IsColour(Squares[passentPawn], enemyColour))
		{
			moves.push_back(ChessMove::CreateEnPassentCapture(startSquare, targetSquare, piece & ~Piece::ClassMask, passentPawn, EnPassentTarget));
			continue;
		}

		if (calculateThreat || IsColour(Squares[targetSquare], enemyColour))
		{
			moves.push_back(ChessMove::CreateMove(startSquare, targetSquare, piece & ~Piece::ClassMask));
		}
	}
}

void Board::GenerateKingMoves(int8 startSquare, int8 piece, std::vector<ChessMove>& moves) const
{
	int8 friendlyColour = piece & ~Piece::ClassMask;
	int8 enemyColour = friendlyColour == Piece::White ? Piece::Black : Piece::White;

	for (int8 directionIndex = 0; directionIndex < 8; directionIndex++)
	{
		int8 targetSquare = startSquare + DirectionOffsets[directionIndex];
		int8 pieceOnTargetSquare = Squares[targetSquare];

		if (targetSquare < 0 || targetSquare >= 64)
		{
			continue;
		}

		if (IsColour(pieceOnTargetSquare, friendlyColour))
		{
			continue;
		}

		if (!IsSquareThreatened(targetSquare, friendlyColour))
		{
			moves.push_back(ChessMove::CreateMove(startSquare, targetSquare, friendlyColour, Castling::Kingside | Castling::Queenside));
		}
	}

	int8 castleAvailability = friendlyColour == Piece::White ? WhiteCastleAvailable : BlackCastleAvailable;
	if ((castleAvailability & Castling::Kingside) == Castling::Kingside)
	{
		int8 startIdx = friendlyColour == Piece::White ? 5 : 61;
		int8 endIdx = friendlyColour == Piece::White ? 6 : 62;
		bool castlingBlocked = false;
		for (int idx = startIdx; idx < endIdx; idx++)
		{
			if (Squares[idx] != Piece::None || IsSquareThreatened(idx, friendlyColour))
			{
				castlingBlocked = true;
				break;
			}
		}

		if (!castlingBlocked)
		{
			moves.push_back(ChessMove::CreateCastlingMove(friendlyColour, Castling::Kingside));
		}
	}
	
	if (((castleAvailability & Castling::Queenside) == Castling::Queenside))
	{
		int8 startIdx = friendlyColour == Piece::White ? 3 : 59;
		int8 endIdx = friendlyColour == Piece::White ? 2 : 57;
		bool castlingBlocked = false;

		for (int idx = endIdx; idx < startIdx; idx++)
		{
			if (Squares[idx] != Piece::None || IsSquareThreatened(idx, friendlyColour))
			{
				castlingBlocked = true;
				break;
			}
		}

		if (!castlingBlocked)
		{
			moves.push_back(ChessMove::CreateCastlingMove(friendlyColour, Castling::Queenside));
		}
	}
}

bool Board::IsPinned(int8 square) const 
{
	return false;
}

bool Board::IsSquareThreatened(int8 square, int8 friendlyColour) const
{
	const ThreatMap& threats = IsColour(friendlyColour, Piece::White) ? BlackThreatening : WhiteThreatening;
	return threats.IsThreatened(square);
}

void Board::ReadFEN(const std::string& fen, int8* board)
{
	memset(board, 0, 64);

	size_t endOfBoard = fen.find(' ');
	if (endOfBoard != std::string::npos)
	{
		std::string fenBoard = fen.substr(0, endOfBoard);
		int file = 0;
		int rank = 7;

		for (char symbol : fenBoard)
		{
			if (symbol == '/')
			{
				file = 0;
				rank--;
			}
			else
			{
				if (std::isdigit(symbol))
				{
					file += std::atoi(&symbol);
				}
				else
				{
					int colour = std::isupper(symbol) ? Piece::White : Piece::Black;
					int piece = PieceTypeFromSymbol[std::tolower(symbol)];
					board[rank * 8 + file] = colour | piece;
					file++;
				}
			}
		}

		std::string toPlay = fen.substr(endOfBoard + 1, 1);
		if (toPlay.find("w") != std::string::npos)
		{
			ColourToMove = Piece::White;
		}
		else
		{
			ColourToMove = Piece::Black;
		}

		size_t endOfCastling = fen.find(' ', endOfBoard + 3);
		std::string castling = fen.substr(endOfBoard + 3, endOfCastling - (endOfBoard + 3));
		
		BlackCastleAvailable = castling.find("k") != std::string::npos ? Castling::Kingside : Castling::None;
		BlackCastleAvailable |= castling.find("q") != std::string::npos ? Castling::Queenside : Castling::None;

		WhiteCastleAvailable = castling.find("K") != std::string::npos ? Castling::Kingside : Castling::None;
		WhiteCastleAvailable |= castling.find("Q") != std::string::npos ? Castling::Queenside : Castling::None;

		//Not currently being used
		size_t endOfEP = fen.find(' ', endOfCastling + 1);
		std::string enPassent = fen.substr(endOfCastling + 1, endOfEP - (endOfCastling + 1));

		size_t endOfHM = fen.find(' ', endOfEP + 1);
		int8 HalfMoveCounter = std::atoi(fen.substr(endOfEP + 1, endOfHM - (endOfEP + 1)).c_str());
		
		size_t endOfFM = fen.find(' ', endOfHM + 1);
		int8 FullMoveCounter = std::atoi(fen.substr(endOfHM + 1, endOfFM - (endOfHM + 1)).c_str());
	}
}

void ThreatMap::CalculateMap(Board* board)
{
	Map = 0;
	for (const ChessMove& move : board->GenerateMoves(Colour, true))
	{
		if (move.Castle == Castling::None)
		{
			SetThreatened(move.TargetSquare);
		}
	}
}
