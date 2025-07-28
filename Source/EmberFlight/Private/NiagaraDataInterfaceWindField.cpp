// Fill out your copyright notice in the Description page of Project Settings.


#include "NiagaraDataInterfaceWindField.h"
#include "NiagaraTypes.h"
#include "VectorVM.h"
#include "NiagaraShaderParametersBuilder.h"
#include "NiagaraCompileHashVisitor.h"
#include "RenderGraphBuilder.h"
#include <UnifiedBuffer.h>

#define LOCTEXT_NAMESPACE "NiagaraWindFieldDI"

static const FName SampleWindFieldName(TEXT("SampleWindAtLocation"));
//static const TCHAR* TemplateShaderFilePath = TEXT("EmberFlight/Shaders/Niagara/NiagaraDataInterfaceWindField.ush");

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
    Sig.bSupportsGPU = true;

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
    return true; // Target == ENiagaraSimTarget::CPUSim/GPUSim (For CPU or GPU only)
}

void UNiagaraDataInterfaceWindField::PostInitProperties()
{
    Super::PostInitProperties();

    // Creating the Proxy here to ensure it's initialized both for CDO and non-CDO objects
    // It's required to pass data between the CPU and GPU (primarily for GPU based Niagara sytem)
    Proxy = MakeUnique<FNDIWindFieldProxy>();

    if (HasAnyFlags(RF_ClassDefaultObject))
    {
        ENiagaraTypeRegistryFlags Flags = ENiagaraTypeRegistryFlags::AllowAnyVariable | ENiagaraTypeRegistryFlags::AllowParameter;
        FNiagaraTypeRegistry::Register(FNiagaraTypeDefinition(GetClass()), Flags);
    }
    MarkRenderDataDirty();
}

bool UNiagaraDataInterfaceWindField::PerInstanceTick(void* PerInstanceData, FNiagaraSystemInstance* SystemInstance, float DeltaSeconds)
{
    return false;
}

bool UNiagaraDataInterfaceWindField::PerInstanceTickPostSimulate(void* PerInstanceData, FNiagaraSystemInstance* InSystemInstance, float DeltaSeconds)
{
    FNDIWindFieldInstanceData* InstanceData = static_cast<FNDIWindFieldInstanceData*>(PerInstanceData);
    if (InstanceData && InstanceData->WindField)
    {
        // Copy velocity grid data safely on game thread into InstanceData
        const TArray<FVector>& SourceGrid = InstanceData->WindField->GetVelocityGrid();

        InstanceData->VelocityGrid.Reset();
        for (const FVector& V : SourceGrid)
        {
            InstanceData->VelocityGrid.Add(FVector3f(V));
        }
    }
    return false; // false here means no forced render proxy update every frame
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

#if WITH_EDITORONLY_DATA
bool UNiagaraDataInterfaceWindField::AppendCompileHash(FNiagaraCompileHashVisitor* InVisitor) const
{
    bool bSuccess = Super::AppendCompileHash(InVisitor);

    bSuccess &= InVisitor->UpdateShaderFile(TEXT("/Plugin/Experimental/ChaosNiagara/NiagaraDataInterfaceWindField.ush"));
    bSuccess &= InVisitor->UpdateShaderParameters<FNDIWindFieldShaderParameters>();

    return bSuccess;
}

void UNiagaraDataInterfaceWindField::GetParameterDefinitionHLSL(const FNiagaraDataInterfaceGPUParamInfo& ParamInfo, FString& OutHLSL)
{
    const TMap<FString, FStringFormatArg> TemplateArgs = {
        {TEXT("ParameterName"), TEXT(""),} // This will be replaced at compile time by the stuff in .ush
    };
    AppendTemplateHLSL(OutHLSL, TEXT("/Plugin/Experimental/ChaosNiagara/NiagaraDataInterfaceWindField.ush"), TemplateArgs);
}

bool UNiagaraDataInterfaceWindField::GetFunctionHLSL(const FNiagaraDataInterfaceGPUParamInfo& ParamInfo, const FNiagaraDataInterfaceGeneratedFunction& FunctionInfo, int FunctionInstanceIndex, FString& OutHLSL)
{
    if (Super::GetFunctionHLSL(ParamInfo, FunctionInfo, FunctionInstanceIndex, OutHLSL))
    {
        return true;
    }

    if (FunctionInfo.DefinitionName == SampleWindFieldName)
    {
        static const TCHAR* HlslTemplate = TEXT(R"(
void {FunctionName}(float X, float Y, float Z, out float OutX, out float OutY, out float OutZ)
{
    float3 WorldPos = float3(X, Y, Z);
    float3 WindVelocity = {ParameterName}_SampleWindTexture(WorldPos);
    OutX = WindVelocity.x;
    OutY = WindVelocity.y;
    OutZ = WindVelocity.z;
}
)");

        TMap<FString, FStringFormatArg> Args;
        Args.Add(TEXT("FunctionName"), FunctionInfo.InstanceName);
        Args.Add(TEXT("ParameterName"), ParamInfo.DataInterfaceHLSLSymbol);

        OutHLSL += FString::Format(HlslTemplate, Args);
        return true;
    }

    return false;
}

#endif

void UNiagaraDataInterfaceWindField::BuildShaderParameters(FNiagaraShaderParametersBuilder& ShaderParametersBuilder) const
{
    ShaderParametersBuilder.AddNestedStruct<FNDIWindFieldShaderParameters>();
}

void UNiagaraDataInterfaceWindField::SetShaderParameters(const FNiagaraDataInterfaceSetShaderParametersContext& Context) const
{
    const FNDIWindFieldProxy& DIProxy = Context.GetProxy<FNDIWindFieldProxy>();
    const FNDIWindFieldRenderData* RenderData = DIProxy.SystemInstancesToProxyData.Find(Context.GetSystemInstanceID());

    auto* ShaderParameters = Context.GetParameterNestedStruct<FNDIWindFieldShaderParameters>();

    
    if (RenderData && ShaderParameters)
    {
        ShaderParameters->User_WindField_FieldOrigin = RenderData->FieldOrigin;
        ShaderParameters->User_WindField_CellSize = RenderData->CellSize;
        ShaderParameters->User_WindField_SizeX = RenderData->SizeX;
        ShaderParameters->User_WindField_SizeY = RenderData->SizeY;
        ShaderParameters->User_WindField_SizeZ = RenderData->SizeZ;

        if (DIProxy.AssetBuffer.IsValid() && DIProxy.AssetBuffer->VelocityGridBuffer.IsValid())
        {
            FRDGBuilder& GraphBuilder = Context.GetGraphBuilder();
            FRDGBufferRef RDGBuffer = GraphBuilder.RegisterExternalBuffer(DIProxy.AssetBuffer->VelocityGridBuffer);
            ShaderParameters->User_WindField_VelocityGridSRV = GraphBuilder.CreateSRV(RDGBuffer);
        }
    }
}

void UNiagaraDataInterfaceWindField::ProvidePerInstanceDataForRenderThread(void* DataForRenderThread, void* PerInstanceData, const FNiagaraSystemInstanceID& SystemInstance)
{
    FNDIWindFieldInstanceData* InstanceData = static_cast<FNDIWindFieldInstanceData*>(PerInstanceData);
    FNDIWindFieldRenderData* RenderData = new (DataForRenderThread) FNDIWindFieldRenderData();
    
    if (!InstanceData || !RenderData || InstanceData->VelocityGrid.Num() == 0)
    {
        return;
    }

    if (InstanceData->WindField)
    {
        const UWindVectorField* Field = InstanceData->WindField;

        RenderData->FieldOrigin = FVector3f(Field->FieldOrigin);
        RenderData->CellSize = Field->CellSize;
        RenderData->SizeX = Field->SizeX;
        RenderData->SizeY = Field->SizeY;
        RenderData->SizeZ = Field->SizeZ;
    }

    // SAFE COPY:
    const int32 NumElements = InstanceData->VelocityGrid.Num();
    RenderData->VelocityGrid.SetNumUninitialized(NumElements);
    FMemory::Memcpy(
        RenderData->VelocityGrid.GetData(),
        InstanceData->VelocityGrid.GetData(),
        NumElements * sizeof(FVector3f)
    );
}

void FNDIWindFieldProxy::ConsumePerInstanceDataFromGameThread(void* PerInstanceData, const FNiagaraSystemInstanceID& Instance)
{
    check(IsInRenderingThread());

    FNDIWindFieldRenderData* SourceData = static_cast<FNDIWindFieldRenderData*>(PerInstanceData);
    if (!SourceData)
    {
        return;
    }

    FNDIWindFieldRenderData& TargetData = SystemInstancesToProxyData.FindOrAdd(Instance);
    TargetData = *SourceData;
}

void FNDIWindFieldProxy::PreStage(const FNDIGpuComputePreStageContext& Context)
{
    FNDIWindFieldRenderData* RenderData = SystemInstancesToProxyData.Find(Context.GetSystemInstanceID());
    
    if (!RenderData || !AssetBuffer.IsValid())
    {
        return;
    }

    // Prepare DataToUpload from the current RenderData->VelocityGrid
    const int32 DataSize = RenderData->VelocityGrid.Num() * sizeof(FVector3f);
    AssetBuffer->DataToUpload.Reset(DataSize);
    AssetBuffer->DataToUpload.AddUninitialized(DataSize);
    FMemory::Memcpy(AssetBuffer->DataToUpload.GetData(), RenderData->VelocityGrid.GetData(), DataSize);

    if (AssetBuffer->DataToUpload.Num() == 0)
    {
        return;
    }

    FRDGBuilder& GraphBuilder = Context.GetGraphBuilder();
    const uint64 InitialDataSize = AssetBuffer->DataToUpload.Num();
    const uint64 BufferSize = Align(InitialDataSize, 16);

    FRDGBufferDesc BufferDesc = FRDGBufferDesc::CreateByteAddressDesc(BufferSize);
    ResizeBufferIfNeeded(GraphBuilder, AssetBuffer->VelocityGridBuffer, BufferDesc, TEXT("WindField.VelocityGrid"));

    GraphBuilder.QueueBufferUpload(
        GraphBuilder.RegisterExternalBuffer(AssetBuffer->VelocityGridBuffer),
        AssetBuffer->DataToUpload.GetData(),
        InitialDataSize,
        ERDGInitialDataFlags::None
    );

    AssetBuffer->DataToUpload.Empty();
}

void FNDIWindFieldProxy::InitializePerInstanceData(const FNiagaraSystemInstanceID& SystemInstance)
{
    check(IsInRenderingThread());
    check(!SystemInstancesToProxyData.Contains(SystemInstance));

    SystemInstancesToProxyData.Add(SystemInstance);
}

void FNDIWindFieldProxy::DestroyPerInstanceData(const FNiagaraSystemInstanceID& SystemInstance)
{
    check(IsInRenderingThread());

    SystemInstancesToProxyData.Remove(SystemInstance);
}

#if WITH_EDITOR
void UNiagaraDataInterfaceWindField::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);
    MarkRenderDataDirty();
}
#endif //WITH_EDITOR
#undef LOCTEXT_NAMESPACE