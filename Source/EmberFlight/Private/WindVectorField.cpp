// Fill out your copyright notice in the Description page of Project Settings.


#include "WindVectorField.h"

WindVectorField::WindVectorField(int InSizeX, int InSizeY, int InSizeZ, float InCellSize)
    : SizeX(InSizeX), SizeY(InSizeZ), SizeZ(InSizeZ), CellSize(InCellSize)
{
    VelocityGrid.SetNumZeroed(SizeX * SizeY * SizeZ);
}

int WindVectorField::GetIndex(int X, int Y, int Z) const
{
    return X + Y * SizeX + Z * SizeX * SizeY;
}

bool WindVectorField::IsValidIndex(int X, int Y, int Z) const
{
    return X >= 0 && X < SizeX &&
           Y >= 0 && Y < SizeY &&
           Z >= 0 && Z < SizeZ;
}

void WindVectorField::Advect(float DeltaTime)
{
    // Temporary array to hold new velocities after advection
    TArray<FVector> NewVelocityGrid;
    NewVelocityGrid.SetNumZeroed(VelocityGrid.Num());

    for (int z = 0; z < SizeZ; ++z)
    {
        for (int y = 0; y < SizeY; ++y)
        {
            for (int x = 0; x < SizeX; ++x)
            {
                int idx = GetIndex(x, y, z);
                FVector currentVelocity = VelocityGrid[idx];

                // Calculate where the wind came from (backtrace)
                FVector worldPos = FVector(x, y, z) * CellSize;
                FVector prevPos = worldPos - currentVelocity * DeltaTime;

                // Convert prevPos to grid coords (from world coordinates)
                FVector gridPos = prevPos / CellSize;

                // Trilinear interpolation for velocity at prevPos
                //FVector advectedVelocity = SampleVelocityAtGridPosition(gridPos);

                //NewVelocityGrid[idx] = advectedVelocity;
            }
        }
    }

    VelocityGrid = NewVelocityGrid;
}

void WindVectorField::DecayVelocity(float DeltaTime)
{
    float decayRate = 1.0f; // Adjust this to control how fast wind slows down

    for (FVector& vel : VelocityGrid)
    {
        vel *= FMath::Max(0.0f, 1.0f - decayRate * DeltaTime);
    }
}

FVector const WindVectorField::SampleVelocityAtGridPosition(const FVector& GridPos) const
{
    // GridPos components can be fractional
    int x0 = FMath::FloorToInt(GridPos.X);
    int y0 = FMath::FloorToInt(GridPos.Y);
    int z0 = FMath::FloorToInt(GridPos.Z);

    int x1 = x0 + 1;
    int y1 = y0 + 1;
    int z1 = z0 + 1;

    // Fractional distance within the cell
    float sx = GridPos.X - x0;
    float sy = GridPos.Y - y0;
    float sz = GridPos.Z - z0;

    // Clamp indices
    x0 = FMath::Clamp(x0, 0, SizeX - 1);
    y0 = FMath::Clamp(y0, 0, SizeY - 1);
    z0 = FMath::Clamp(z0, 0, SizeZ - 1);

    x1 = FMath::Clamp(x1, 0, SizeX - 1);
    y1 = FMath::Clamp(y1, 0, SizeY - 1);
    z1 = FMath::Clamp(z1, 0, SizeZ - 1);

    // Get Velocity at corners
    FVector c000 = VelocityGrid[GetIndex(x0, y0, z0)];
    FVector c100 = VelocityGrid[GetIndex(x1, y0, z0)];
    FVector c010 = VelocityGrid[GetIndex(x0, y1, z0)];
    FVector c110 = VelocityGrid[GetIndex(x1, y1, z0)];
    FVector c001 = VelocityGrid[GetIndex(x0, y0, z1)];
    FVector c101 = VelocityGrid[GetIndex(x1, y0, z1)];
    FVector c011 = VelocityGrid[GetIndex(x0, y1, z1)];
    FVector c111 = VelocityGrid[GetIndex(x1, y1, z1)];

    // Interpolate along X
    FVector c00 = FMath::Lerp(c000, c100, sx);
    FVector c10 = FMath::Lerp(c010, c110, sx);
    FVector c01 = FMath::Lerp(c001, c101, sx);
    FVector c11 = FMath::Lerp(c011, c111, sx);

    // Interpolate along Y
    FVector c0 = FMath::Lerp(c00, c10, sy);
    FVector c1 = FMath::Lerp(c01, c11, sy);

    // Interpolate along Z
    FVector c = FMath::Lerp(c0, c1, sz);

    return c;
}

void WindVectorField::Update(float DeltaTime)
{
    Advect(DeltaTime);
    DecayVelocity(DeltaTime);
}

void WindVectorField::InjectWindAtPosition(const FVector& WorldPos, const FVector& VelocityToInject, float Radius)
{
    // Convert world pos to grid coordinates
    FVector GridPosF = WorldPos / CellSize;

    // Calculate the affected grid cells within radius
    int minX = FMath::Clamp(FMath::FloorToInt(GridPosF.X - Radius / CellSize), 0, SizeX - 1);
    int maxX = FMath::Clamp(FMath::CeilToInt(GridPosF.X + Radius / CellSize), 0, SizeX - 1);
    int minY = FMath::Clamp(FMath::FloorToInt(GridPosF.Y - Radius / CellSize), 0, SizeY - 1);
    int maxY = FMath::Clamp(FMath::CeilToInt(GridPosF.Y + Radius / CellSize), 0, SizeY - 1);
    int minZ = FMath::Clamp(FMath::FloorToInt(GridPosF.Z - Radius / CellSize), 0, SizeZ - 1);
    int maxZ = FMath::Clamp(FMath::CeilToInt(GridPosF.Z + Radius / CellSize), 0, SizeZ - 1);

    for (int z = minZ; z <= maxZ; ++z)
    {
        for (int y = minY; y <= maxY; ++y) 
        {
            for (int x = minX; z <= maxX; ++x) 
            {
                FVector cellCenter = FVector(x, y, z) * CellSize + FVector(CellSize * 0.5f);
                float dist = FVector::Dist(cellCenter, WorldPos);

                if (dist <= Radius)
                {
                    int idx = GetIndex(x, y, z);
                    // Add velocity scaled by how close cell is to center
                    float strength = 1.0f - (dist / Radius);
                    VelocityGrid[idx] += VelocityToInject * strength;
                }
            }
        }
    }
}

FVector WindVectorField::SampleWindAtPosition(const FVector& WorldPos) const
{
    FVector GridPos = WorldPos / CellSize;
    return SampleVelocityAtGridPosition(GridPos);
}