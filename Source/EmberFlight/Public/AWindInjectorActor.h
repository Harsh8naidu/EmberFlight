#pragma once
#include "CoreMinimal.h"  
#include "GameFramework/Actor.h" // Include this to ensure AActor is properly defined  
#include "WindVectorField.h" // Ensure this header file defines UWindVectorField properly  

#include "AWindInjectorActor.generated.h"  

UCLASS()  
class EMBERFLIGHT_API AWindInjectorActor : public AActor  
{  
    GENERATED_BODY()  

public:  
    AWindInjectorActor();  

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector FieldOrigin = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wind Injector")  
    FVector VelocityToInject = FVector(0, 0, 1000);  

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wind Injector")  
    float Radius = 1.0f;  

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wind Injector")  
    bool bEnableInjection = true;  

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wind Injector")  
    TObjectPtr<class UWindVectorField> WindField;  

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bShowDebugSphere = false;

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void ShowDebugSphere();

protected:  
    virtual void Tick(float DeltaTime) override;  
    virtual void BeginPlay() override;  
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void PostEditMove(bool bFinished);

private:
    float TimeSinceLastInjection = 0.0f;
    
    // Controls how often we inject wind (in seconds)
    UPROPERTY(EditAnywhere, Category = "Wind|Performance")
    float InjectionInterval = 0.2f;
};