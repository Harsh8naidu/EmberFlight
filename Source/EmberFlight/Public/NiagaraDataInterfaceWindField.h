// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NiagaraDataInterface.h"
#include "WindVectorField.h"
#include "NiagaraDataInterfaceWindField.generated.h"

UCLASS(EditInlineNew, Category = "Wind", meta = (DisplayName = "Wind Field", NiagaraDataInterface = "True"), Blueprintable, BlueprintType)
class EMBERFLIGHT_API UNiagaraDataInterfaceWindField : public UNiagaraDataInterface
{
    GENERATED_BODY()
public:
    UNiagaraDataInterfaceWindField();
    void SampleWindAtLocation(FVectorVMExternalFunctionContext& Context);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wind")
    TObjectPtr<UWindVectorField> WindField;

    // Niagara interface overrides
    virtual void GetFunctions(TArray<FNiagaraFunctionSignature>& OutFunctions) override;
    virtual void GetVMExternalFunction(const FVMExternalFunctionBindingInfo& BindingInfo, void* InstanceData, FVMExternalFunction& OutFunc) override;
    virtual bool Equals(const UNiagaraDataInterface* Other) const override;
    virtual bool CanExecuteOnTarget(ENiagaraSimTarget Target) const override;
    virtual void PostInitProperties() override;
#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

    // Optional: convenience function (not used by Niagara VM)
    UFUNCTION(BlueprintCallable, Category = "Wind")
    FVector GetZeroWind() const { return FVector::ZeroVector; }
};