// Fill out your copyright notice in the Description page of Project Settings.


#include "NiagaraDataInterfaceWindField.h"
#include "NiagaraTypes.h"
#include "VectorVM.h"

#define LOCTEXT_NAMESPACE "NiagaraWindFieldDI"

static const FName SampleWindFieldName(TEXT("SampleWindAtLocation"));

UNiagaraDataInterfaceWindField::UNiagaraDataInterfaceWindField()
{
}

void UNiagaraDataInterfaceWindField::SampleWindAtLocation(FVectorVMExternalFunctionContext& Context)
{
    VectorVM::FUserPtrHandler<const UWindVectorField> FieldHandler(Context);
    VectorVM::FExternalFuncInputHandler<float> X(Context);
    VectorVM::FExternalFuncInputHandler<float> Y(Context);
    VectorVM::FExternalFuncInputHandler<float> Z(Context);

    VectorVM::FExternalFuncRegisterHandler<float> OutX(Context);
    VectorVM::FExternalFuncRegisterHandler<float> OutY(Context);
    VectorVM::FExternalFuncRegisterHandler<float> OutZ(Context);

    const UWindVectorField* SampledField = FieldHandler.Get();

    for (int32 i = 0; i < Context.GetNumInstances(); ++i)
    {
        FVector WorldPos(X.GetAndAdvance(), Y.GetAndAdvance(), Z.GetAndAdvance());
        FVector Velocity = SampledField ? SampledField->SampleWindAtPosition(WorldPos) : FVector::ZeroVector;

        *OutX.GetDestAndAdvance() = Velocity.X;
        *OutY.GetDestAndAdvance() = Velocity.Y;
        *OutZ.GetDestAndAdvance() = Velocity.Z;
    }
}

void UNiagaraDataInterfaceWindField::GetFunctions(TArray<FNiagaraFunctionSignature>& OutFunctions)
{
    FNiagaraFunctionSignature Sig;
    Sig.Name = SampleWindFieldName;
    Sig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition(GetClass()), TEXT("Wind Field")));
    Sig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetFloatDef(), TEXT("X")));
    Sig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetFloatDef(), TEXT("Y")));
    Sig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetFloatDef(), TEXT("Z")));
    Sig.Outputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetFloatDef(), TEXT("OutX")));
    Sig.Outputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetFloatDef(), TEXT("OutY")));
    Sig.Outputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetFloatDef(), TEXT("OutZ")));
    Sig.SetDescription(LOCTEXT("SampleWindDesc", "Sample wind velocity at a given world position"));
    Sig.bMemberFunction = true;

    OutFunctions.Add(Sig);
}

DEFINE_NDI_DIRECT_FUNC_BINDER(UNiagaraDataInterfaceWindField, SampleWindAtLocation);

void UNiagaraDataInterfaceWindField::GetVMExternalFunction(const FVMExternalFunctionBindingInfo& BindingInfo, void* InstanceData, FVMExternalFunction& OutFunc)
{
    if (BindingInfo.Name == SampleWindFieldName)
    {
        NDI_FUNC_BINDER(UNiagaraDataInterfaceWindField, SampleWindAtLocation)::Bind(this, OutFunc);
    }
}

bool UNiagaraDataInterfaceWindField::Equals(const UNiagaraDataInterface* Other) const
{
    const UNiagaraDataInterfaceWindField* OtherTyped = CastChecked<UNiagaraDataInterfaceWindField>(Other);
    return OtherTyped && OtherTyped->WindField == WindField;
}

bool UNiagaraDataInterfaceWindField::CanExecuteOnTarget(ENiagaraSimTarget Target) const
{
    return Target == ENiagaraSimTarget::CPUSim;
}

void UNiagaraDataInterfaceWindField::PostInitProperties()
{
    Super::PostInitProperties();

    UE_LOG(LogTemp, Warning, TEXT("UNiagaraDataInterfaceWindField::PostInitProperties called."));
    if (HasAnyFlags(RF_ClassDefaultObject))
    {
        ENiagaraTypeRegistryFlags Flags = ENiagaraTypeRegistryFlags::AllowAnyVariable | ENiagaraTypeRegistryFlags::AllowParameter;
        FNiagaraTypeRegistry::Register(FNiagaraTypeDefinition(GetClass()), Flags);
        UE_LOG(LogTemp, Warning, TEXT("If condition executed inside postinitproperties"));

    }
}

#if WITH_EDITOR
void UNiagaraDataInterfaceWindField::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);
    MarkRenderDataDirty();
}
#endif //WITH_EDITOR
#undef LOCTEXT_NAMESPACE