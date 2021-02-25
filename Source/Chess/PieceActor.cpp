// Fill out your copyright notice in the Description page of Project Settings.

#include "PieceActor.h"

// Sets default values
APieceActor::APieceActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
}

void APieceActor::SetPieceType_Implementation(Class piece, bool black)
{
	PieceType = piece;
	Black = black;
	Taken = false;
}

void APieceActor::TakePiece_Implementation()
{
	Taken = true;
}

void APieceActor::MoveTo(const FVector& square)
{
	SetActorLocation(square);
}
