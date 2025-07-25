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
    //UE_LOG(LogTemp, Warning, TEXT("Sample Wind At Locations called"));
    VectorVM::FUserPtrHandler<FNDIWindFieldInstanceData> InstanceData(Context);
    if (!InstanceData.Get() || !InstanceData.Get()->WindField)
    {
        UE_LOG(LogTemp, Warning, TEXT("UNiagaraDataInterfaceWindField::SampleWindAtLocation called with invalid FieldHandler."));
        return;
    }
    
    FNDIInputParam<float> X(Context);
    FNDIInputParam<float> Y(Context);
    FNDIInputParam<float> Z(Context);

    FNDIOutputParam<float> OutX(Context);
    FNDIOutputParam<float> OutY(Context);
    FNDIOutputParam<float> OutZ(Context);

    const int32 NumInstances = Context.GetNumInstances();

    for (int32 i = 0; i < NumInstances; ++i)
    {
        FVector WorldPos(X.GetAndAdvance(), Y.GetAndAdvance(), Z.GetAndAdvance());
        FVector Velocity = InstanceData.Get()->WindField->SampleWindAtPosition(WorldPos);

        //UE_LOG(LogTemp, Warning, TEXT("[Niagara] WindSample Velocity: X=%f Y=%f Z=%f"), Velocity.X, Velocity.Y, Velocity.Z);

        OutX.SetAndAdvance(Velocity.X);
        OutY.SetAndAdvance(Velocity.Y);
        OutZ.SetAndAdvance(Velocity.Z);
    }
}

void UNiagaraDataInterfaceWindField::GetFunctions(TArray<FNiagaraFunctionSignature>& OutFunctions)
{
    FNiagaraFunctionSignature Sig;
    Sig.Name = SampleWindFieldName;

    // Add the inputs of the data interface class
    Sig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition(GetClass()), TEXT("WindField")));

    // Add float position inputs
    Sig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetFloatDef(), TEXT("X")));
    Sig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetFloatDef(), TEXT("Y")));
    Sig.Inputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetFloatDef(), TEXT("Z")));

    // Add float outputs
    Sig.Outputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetFloatDef(), TEXT("OutX")));
    Sig.Outputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetFloatDef(), TEXT("OutY")));
    Sig.Outputs.Add(FNiagaraVariable(FNiagaraTypeDefinition::GetFloatDef(), TEXT("OutZ")));

    Sig.bMemberFunction = true;
    Sig.bRequiresContext = false;
    Sig.bSupportsCPU = true;

    Sig.SetDescription(LOCTEXT("SampleWindDesc", "Sample wind velocity at a given world position"));
    
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

bool UNiagaraDataInterfaceWindField::CopyToInternal(UNiagaraDataInterface* Destination) const
{
    UNiagaraDataInterfaceWindField* DestTyped = Cast<UNiagaraDataInterfaceWindField>(Destination);
    if (!DestTyped)
    {
        return false;
    }

    DestTyped->WindField = this->WindField;

    return true;
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

    if (HasAnyFlags(RF_ClassDefaultObject))
    {
        ENiagaraTypeRegistryFlags Flags = ENiagaraTypeRegistryFlags::AllowAnyVariable | ENiagaraTypeRegistryFlags::AllowParameter;
        FNiagaraTypeRegistry::Register(FNiagaraTypeDefinition(GetClass()), Flags);
    }
    MarkRenderDataDirty();
}

int32 UNiagaraDataInterfaceWindField::PerInstanceDataSize() const
{
    return sizeof(FNDIWindFieldInstanceData);
}

bool UNiagaraDataInterfaceWindField::InitPerInstanceData(void* PerInstanceData, FNiagaraSystemInstance* SystemInstance)
{
    FNDIWindFieldInstanceData* InstanceData = new (PerInstanceData) FNDIWindFieldInstanceData();
    InstanceData->WindField = WindField;
    return true;
}

void UNiagaraDataInterfaceWindField::DestroyPerInstanceData(void* PerInstanceData, FNiagaraSystemInstance* SystemInstance)
{
    FNDIWindFieldInstanceData* InstanceData = (FNDIWindFieldInstanceData*)PerInstanceData;
    InstanceData->~FNDIWindFieldInstanceData();
}

#if WITH_EDITOR
void UNiagaraDataInterfaceWindField::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);
    MarkRenderDataDirty();
}
#endif //WITH_EDITOR
#undef LOCTEXT_NAMESPACE