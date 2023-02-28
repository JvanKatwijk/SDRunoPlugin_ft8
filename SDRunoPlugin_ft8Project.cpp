#include <iunoplugin.h>
#include "SDRunoPlugin_ft8.h"

extern "C" {

	UNOPLUGINAPI IUnoPlugin* UNOPLUGINCALL CreatePlugin(IUnoPluginController& controller) {
	   return new SDRunoPlugin_ft8 (controller);
}

	UNOPLUGINAPI void UNOPLUGINCALL DestroyPlugin(IUnoPlugin* plugin) {
	   delete plugin;
	}

	UNOPLUGINAPI unsigned int UNOPLUGINCALL GetPluginApiLevel () {
	   return UNOPLUGINAPIVERSION;
	}
}

