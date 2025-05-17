// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
class EMBERFLIGHT_API WindVectorField
{
public:
	WindVectorField(int InSizeX, int InSizeY, int InSizeZ, float InCellSize);

    void Update(float DeltaTime);

    void InjectWindAtPosition(const FVector& WorldPos, const FVector& VelocityToInject, float Radius);
    FVector SampleWindAtPosition(const FVector& WorldPos) const;
	
private:
    int SizeX, SizeY, SizeZ;
    float CellSize;

    TArray<FVector> VelocityGrid;

    int GetIndex(int X, int Y, int Z) const;
    bool IsValidIndex(int X, int Y, int Z) const;
    void Advect(float DeltaTime);
    void DecayVelocity(float DeltaTime);
    FVector const SampleVelocityAtGridPosition(const FVector& GridPos) const;
};
