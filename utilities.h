/* Prototypes of all functions in utilities.c */

#ifndef _UTILITIES_
#define _UTILITIES_

#include "VLM_configuration.h"

void LogMessage(char *function, char *formatString, ...);

void verror(char *section, char *format, ...);
void vpunt(char *section, char *format, ...);
void vwarn(char *section, char *format, ...);

void BuildConfiguration(VLMConfig *config, int argc, char **argv);
void SetCommandName(char *newCommandName);
void BuildXDisplayName(char *displayName, char *hostName, int display, int screen);

/* Internal function prototypes are in utilities.c itself */

#endif // _UTILITIES_
