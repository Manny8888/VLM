/* Prototypes of all functions in utilities.c */

#ifndef _UTILITIES_
#define _UTILITIES_

#include "VLM_configuration.h"

#define PuntWorld(world, msg, arg)                                                                                     \
    {                                                                                                                  \
        CloseWorldFile(world, TRUE);                                                                                   \
        {                                                                                                              \
            FILE *log_fd = fopen(log_file_genera, "a");                                                                \
            fprintf(log_fd, msg, arg);                                                                                 \
            fflush(log_fd);                                                                                            \
            fclose(log_fd);                                                                                            \
        }                                                                                                              \
        vpunt(NULL, msg, arg);                                                                                         \
    }

#define PuntWorld2(world, msg, arg1, arg2)                                                                             \
    {                                                                                                                  \
        CloseWorldFile(world, TRUE);                                                                                   \
        {                                                                                                              \
            FILE *log_fd = fopen(log_file_genera, "a");                                                                \
            fprintf(log_fd, msg, arg1, arg2);                                                                          \
            fflush(log_fd);                                                                                            \
            fclose(log_fd);                                                                                            \
        }                                                                                                              \
    }

#define PuntWorld3(world, msg, arg1, arg2, arg3)                                                                       \
    {                                                                                                                  \
        CloseWorldFile(world, TRUE);                                                                                   \
        {                                                                                                              \
            FILE *log_fd = fopen(log_file_genera, "a");                                                                \
            fprintf(log_fd, msg, arg1, arg2, arg3);                                                                    \
            fflush(log_fd);                                                                                            \
            fclose(log_fd);                                                                                            \
        }                                                                                                              \
        vpunt(NULL, msg, arg1, arg2, arg3);                                                                            \
    }

#define LogMessage0(formatString)                                                                                      \
    {                                                                                                                  \
        if (ECHO_ON_SCREEN_P) {                                                                                        \
            printf("Function: %s", __func__);                                                                          \
            printf(" --- ");                                                                                           \
            printf(formatString);                                                                                      \
            printf("\n");                                                                                              \
        }                                                                                                              \
                                                                                                                       \
        FILE *log_fd = fopen(log_file_genera, "a");                                                                    \
        fprintf(log_fd, "Function: ");                                                                                 \
        fprintf(log_fd, __func__);                                                                                     \
        fprintf(log_fd, " --- ");                                                                                      \
        fprintf(log_fd, formatString);                                                                                 \
        fprintf(log_fd, "\n");                                                                                         \
        fflush(log_fd);                                                                                                \
        fclose(log_fd);                                                                                                \
    }

#define LogMessage1(formatString, arg)                                                                                 \
    {                                                                                                                  \
        if (ECHO_ON_SCREEN_P) {                                                                                        \
            printf("Function: %s", __func__);                                                                          \
            printf(" --- ");                                                                                           \
            printf(formatString, arg);                                                                                 \
            printf("\n");                                                                                              \
        }                                                                                                              \
                                                                                                                       \
        FILE *log_fd = fopen(log_file_genera, "a");                                                                    \
        fprintf(log_fd, "Function: ");                                                                                 \
        fprintf(log_fd, __func__);                                                                                     \
        fprintf(log_fd, " --- ");                                                                                      \
        fprintf(log_fd, formatString, arg);                                                                            \
        fprintf(log_fd, "\n");                                                                                         \
        fflush(log_fd);                                                                                                \
        fclose(log_fd);                                                                                                \
    }

#define LogMessage2(formatString, arg1, arg2)                                                                          \
    {                                                                                                                  \
        if (ECHO_ON_SCREEN_P) {                                                                                        \
            printf("Function: %s", __func__);                                                                          \
            printf(" --- ");                                                                                           \
            printf(formatString, arg1, arg2);                                                                          \
            printf("\n");                                                                                              \
        }                                                                                                              \
                                                                                                                       \
        FILE *log_fd = fopen(log_file_genera, "a");                                                                    \
        fprintf(log_fd, "Function: ");                                                                                 \
        fprintf(log_fd, __func__);                                                                                     \
        fprintf(log_fd, " --- ");                                                                                      \
        fprintf(log_fd, formatString, arg1, arg2);                                                                     \
        fprintf(log_fd, "\n");                                                                                         \
        fflush(log_fd);                                                                                                \
        fclose(log_fd);                                                                                                \
    }

#define LogMessage3(formatString, arg1, arg2, arg3)                                                                    \
    {                                                                                                                  \
        if (ECHO_ON_SCREEN_P) {                                                                                        \
            printf("Function: %s", __func__);                                                                          \
            printf(" --- ");                                                                                           \
            printf(formatString, arg1, arg2, arg3);                                                                    \
            printf("\n");                                                                                              \
        }                                                                                                              \
                                                                                                                       \
        FILE *log_fd = fopen(log_file_genera, "a");                                                                    \
        fprintf(log_fd, "Function: ");                                                                                 \
        fprintf(log_fd, __func__);                                                                                     \
        fprintf(log_fd, " --- ");                                                                                      \
        fprintf(log_fd, formatString, arg1, arg2, arg3);                                                               \
        fprintf(log_fd, "\n");                                                                                         \
        fflush(log_fd);                                                                                                \
        fclose(log_fd);                                                                                                \
    }

#define LogMessage4(formatString, arg1, arg2, arg3, arg4)                                                              \
    {                                                                                                                  \
        if (ECHO_ON_SCREEN_P) {                                                                                        \
            printf("Function: %s", __func__);                                                                          \
            printf(" --- ");                                                                                           \
            printf(formatString, arg1, arg2, arg3, arg4);                                                              \
            printf("\n");                                                                                              \
        }                                                                                                              \
                                                                                                                       \
        FILE *log_fd = fopen(log_file_genera, "a");                                                                    \
        fprintf(log_fd, "Function: ");                                                                                 \
        fprintf(log_fd, __func__);                                                                                     \
        fprintf(log_fd, " --- ");                                                                                      \
        fprintf(log_fd, formatString, arg1, arg2, arg3, arg4);                                                         \
        fprintf(log_fd, "\n");                                                                                         \
        fflush(log_fd);                                                                                                \
        fclose(log_fd);                                                                                                \
    }

void LogMessage(char *function, char *formatString, ...);

void verror(char *section, char *format, ...);
void vpunt(char *section, char *format, ...);
void vwarn(char *section, char *format, ...);

void BuildConfiguration(VLMConfig *config, int argc, char **argv);
void SetCommandName(char *newCommandName);
void BuildXDisplayName(char *displayName, char *hostName, int display, int screen);

/* Internal function prototypes are in utilities.c itself */

#endif // _UTILITIES_
