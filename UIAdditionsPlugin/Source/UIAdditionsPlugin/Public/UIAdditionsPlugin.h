/**Copyright 2025: Roy Wierer (Ferrefy). All Rights Reserved.**/
#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"
#include "Templates/SharedPointer.h"

class FDetectCurrentInputDevicePreProcessor;


class UIADDITIONSPLUGIN_API FUIAdditionsPluginModule : public IModuleInterface {

private:

	TSharedPtr<FDetectCurrentInputDevicePreProcessor> DetectCurrentInputDevicePreProcessor = nullptr;

protected:

public:

private:

protected:

public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;

	virtual void ShutdownModule() override;
	/** End IModuleInterface implementation */

	const TSharedPtr<FDetectCurrentInputDevicePreProcessor>& GetDetectCurrentInputDevicePreProcessor() const;

};
