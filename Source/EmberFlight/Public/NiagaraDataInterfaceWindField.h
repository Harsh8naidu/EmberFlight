// Fill out your copyright notice in the Description page of Project Settings.
#pragma once
#include "RenderResource.h"
#include "RenderGraphResources.h"
#include "CoreMinimal.h"
#include "NiagaraDataInterface.h"
#include "WindVectorField.h"
#include "NiagaraDataInterfaceWindField.generated.h"

UCLASS(EditInlineNew, Category = "Wind", meta = (DisplayName = "WindField", NiagaraDataInterface = "True"), Blueprintable, BlueprintType)
class EMBERFLIGHT_API UNiagaraDataInterfaceWindField : public UNiagaraDataInterface
{
    GENERATED_BODY()

public:
    UNiagaraDataInterfaceWindField();
    void SampleWindAtLocation(FVectorVMExternalFunctionContext& Context);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wind")
    TObjectPtr<UWindVectorField> WindField;

    // CPU Sim Functionality
    virtual void GetFunctions(TArray<FNiagaraFunctionSignature>& OutFunctions) override;
    virtual void GetVMExternalFunction(const FVMExternalFunctionBindingInfo& BindingInfo, void* InstanceData, FVMExternalFunction& OutFunc) override;
    virtual bool CopyToInternal(UNiagaraDataInterface* Destination) const override;
    virtual bool Equals(const UNiagaraDataInterface* Other) const override;
    virtual bool CanExecuteOnTarget(ENiagaraSimTarget Target) const override;
    virtual void PostInitProperties() override;
    virtual int32 PerInstanceDataSize() const override;
    virtual bool InitPerInstanceData(void* PerInstanceData, FNiagaraSystemInstance* SystemInstance) override;
    virtual void DestroyPerInstanceData(void* PerInstanceData, FNiagaraSystemInstance* SystemInstance) override;

    // GPU Sim Functionality
#if WITH_EDITORONLY_DATA
    virtual bool AppendCompileHash(FNiagaraCompileHashVisitor* InVisitor) const override;
    virtual void GetParameterDefinitionHLSL(const FNiagaraDataInterfaceGPUParamInfo& ParamInfo, FString& OutHLSL) override;
    virtual bool GetFunctionHLSL(const FNiagaraDataInterfaceGPUParamInfo& ParamInfo, const FNiagaraDataInterfaceGeneratedFunction& FunctionInfo, int FunctionInstanceIndex, FString& OutHLSL) override;
#endif
    virtual void BuildShaderParameters(FNiagaraShaderParametersBuilder& ShaderParametersBuilder) const override;
    virtual void SetShaderParameters(const FNiagaraDataInterfaceSetShaderParametersContext& Context) const override;
    virtual void ProvidePerInstanceDataForRenderThread(void* DataForRenderThread, void* PerInstanceData, const FNiagaraSystemInstanceID& SystemInstance) override;
    virtual bool PerInstanceTick(void* PerInstanceData, FNiagaraSystemInstance* SystemInstance, float DeltaSeconds) override;
    virtual bool PerInstanceTickPostSimulate(void* PerInstanceData, FNiagaraSystemInstance* InSystemInstance, float DeltaSeconds);
    virtual bool HasPreSimulateTick() const override { return true; }
    virtual bool HasPostSimulateTick() const override { return true; }
    virtual bool HasTickGroupPrereqs() const override { return true; }
    
#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};

struct FNDIWindFieldBuffer : public FRenderResource
{
    /** Velocity data buffer **/
    TRefCountPtr<FRDGPooledBuffer> VelocityGridBuffer;

    /** Raw velocity data to upload **/
    TArray<uint8> DataToUpload;

    int32 NumElements = 0;
};

// GPU Shader Parameters Struct
BEGIN_SHADER_PARAMETER_STRUCT(FNDIWindFieldShaderParameters, )
    SHADER_PARAMETER(FVector3f, User_WindField_FieldOrigin)
    SHADER_PARAMETER(float, User_WindField_CellSize)
    SHADER_PARAMETER(uint32, User_WindField_SizeX)
    SHADER_PARAMETER(uint32, User_WindField_SizeY)
    SHADER_PARAMETER(uint32, User_WindField_SizeZ)
    SHADER_PARAMETER_RDG_BUFFER_SRV(StructuredBuffer<float3>, User_WindField_VelocityGridSRV)
END_SHADER_PARAMETER_STRUCT()

// Per-instance data struct
USTRUCT()
struct FNDIWindFieldInstanceData
{
    GENERATED_BODY()

    UPROPERTY()
    UWindVectorField* WindField = nullptr;
    TArray<FVector3f> VelocityGrid;
};

struct FNDIWindFieldRenderData
{
    FVector3f FieldOrigin;
    float CellSize;
    int32 SizeX;
    int32 SizeY;
    int32 SizeZ;
    TArray<FVector3f> VelocityGrid;
};

struct FNDIWindFieldProxy : public FNiagaraDataInterfaceProxy
{
    // Your buffer struct, like GeometryCollection uses
    TSharedPtr<FNDIWindFieldBuffer, ESPMode::ThreadSafe> AssetBuffer;

    /** List of proxy data for each system instances **/
    TMap<FNiagaraSystemInstanceID, FNDIWindFieldRenderData> SystemInstancesToProxyData;

    /** Initialize the Proxy data buffer */
    void InitializePerInstanceData(const FNiagaraSystemInstanceID& SystemInstance);

    /** Destroy the proxy data if necessary */
    void DestroyPerInstanceData(const FNiagaraSystemInstanceID& SystemInstance);

    /** Get the size of the data that will be passed to render **/
    virtual int32 PerInstanceDataPassedToRenderThreadSize() const override { return sizeof(FNDIWindFieldRenderData); }

    /** Get the data that will be passed to render **/
    virtual void ConsumePerInstanceDataFromGameThread(void* PerInstanceData, const FNiagaraSystemInstanceID& Instance) override;

    /** Launch all pre stage functions **/
    virtual void PreStage(const FNDIGpuComputePreStageContext& Context) override;
};