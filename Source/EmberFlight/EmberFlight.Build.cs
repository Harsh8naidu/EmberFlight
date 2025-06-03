// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class EmberFlight : ModuleRules
{
	public EmberFlight(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { 
			"Core", 
			"CoreUObject", 
			"Engine", 
			"InputCore", 
			"EnhancedInput", 
			"Niagara" ,
			"NiagaraCore",
			"NiagaraShader",
		});

		// Optional but can be helpful for Niagara debugging or interaction
		PrivateDependencyModuleNames.AddRange(new string[] {
			"RenderCore",
			"RHI"
		});

		// Editor-only dependencies
		if(Target.bBuildEditor)
		{
			PrivateDependencyModuleNames.AddRange(new string[] {
				"UnrealEd",
				"NiagaraEditor",
				"EditorFramework",
				"AssetTools"
			});
		}

		PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "ThirdParty/FastNoise"));
	}
}
