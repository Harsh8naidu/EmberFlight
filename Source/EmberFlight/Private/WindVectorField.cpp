// Fill out your copyright notice in the Description page of Project Settings.


#include "WindVectorField.h"

UWindVectorField::UWindVectorField() {}

void UWindVectorField::Initialize(int InSizeX, int InSizeY, int InSizeZ, float InCellSize)
{
    SizeX = InSizeX;
    SizeY = InSizeY;
    SizeZ = InSizeZ;
    CellSize = InCellSize;

    VelocityGrid.SetNumZeroed(SizeX * SizeY * SizeZ);

    Noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    Noise.SetFrequency(WindNoiseFrequency);
    Noise.SetSeed(WindNoiseSeed);
}

int UWindVectorField::GetIndex(int X, int Y, int Z) const
{
    return X + Y * SizeX + Z * SizeX * SizeY;
}

bool UWindVectorField::IsValidIndex(int X, int Y, int Z) const
{
    return X >= 0 && X < SizeX &&
           Y >= 0 && Y < SizeY &&
           Z >= 0 && Z < SizeZ;
}

void UWindVectorField::Advect(float DeltaTime)
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
                FVector advectedVelocity = SampleVelocityAtGridPosition(gridPos);

                NewVelocityGrid[idx] = advectedVelocity;
            }
        }
    }

    VelocityGrid = NewVelocityGrid;
}

void UWindVectorField::DecayVelocity(float DeltaTime)
{
    float decayRate = 1.0f; // Adjust this to control how fast wind slows down

    for (FVector& vel : VelocityGrid)
    {
        vel *= FMath::Max(0.0f, 1.0f - decayRate * DeltaTime);
    }
}

FVector const UWindVectorField::SampleVelocityAtGridPosition(const FVector& GridPos) const
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

void UWindVectorField::Update(float DeltaTime)
{
    Advect(DeltaTime);
    DecayVelocity(DeltaTime);

    for (int Z = 0; Z < SizeZ; ++Z)
    {
        for (int Y = 0; Y < SizeY; ++Y)
        {
            for (int X = 0; X < SizeX; ++X)
            {
                int Index = GetIndex(X, Y, Z);
                FVector PosWorld = FVector(X, Y, Z) * CellSize;

                // Sample noise in X and Z directions
                float NoiseX = Noise.GetNoise((float)X, (float)Y, (float)Z);
                float NoiseY = Noise.GetNoise((float)Y + 1000, (float)Y + 1000, (float)Z);
                float NoiseZ = Noise.GetNoise((float)X + 2000, (float)Y + 2000, (float)Z + 2000);

                FVector WindVelocity = FVector(NoiseX, 0.0f, NoiseZ) * WindScale;
                VelocityGrid[Index] = WindVelocity;
            }
        }
    }
}

void UWindVectorField::InjectWindAtPosition(const FVector& WorldPos, const FVector& VelocityToInject, float Radius)
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
            for (int x = minX; x <= maxX; ++x) 
            {
                FVector cellCenter = FVector(x, y, z) * CellSize + FVector(CellSize * 0.5f);
                float dist = FVector::Dist(cellCenter, WorldPos);

                if (dist <= Radius && IsValidIndex(x, y, z))
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

FVector UWindVectorField::SampleWindAtPosition(const FVector& WorldPos) const
{
    FVector GridPos = WorldPos / CellSize;
    return SampleVelocityAtGridPosition(GridPos);
}

void UWindVectorField::DebugDraw(float Scale) const
{
    UWorld* World = GetWorld(); // get world from the UObject base
    if (!World) return;

    for (int z = 0; z < SizeZ; ++z)
    {
        for (int y = 0; y < SizeY; ++y)
        {
            for (int x = 0; x < SizeX; ++x)
            {
                int Index = GetIndex(x, y, z);
                FVector Start = FVector(x, y, z) * CellSize;
                FVector Velocity = VelocityGrid[Index];

                FVector End = Start + (Velocity * Scale);
                
                //DrawDebugLine(World, Start, End, FColor::Cyan, false, -1, 0, 2.0f);
                DrawDebugDirectionalArrow(World, Start, End, 50.0, FColor::Blue, false, -1, 0, 1.5f);
            }
        }
    }
}