
#include <fenv.h>
#include <signal.h>

#include "std.h"
#include "spy.h"
#include "utilities.h"
#include "world_tools.h"

#include "VLM_configuration.h"
#include "life-support/life_prototypes.h"
#include "life-support/SystemComm.h"
#include "emulator/aihead.h"
#include "emulator/ivoryrep.h"

// TODO: Check if really needed #include "aistat.h"

#define MBToWords(MB) ((MB * 1024 * 1024) + 4) / 5 // TODO: Why /5?
#define WordsToMB(words) ((5 * words) + (1024 * 1024) - 1) / (1024 * 1024)

Boolean Trace = FALSE;
Boolean EnableIDS = FALSE; // WTF is that?
Boolean TestFunction = FALSE;
static pthread_key_t mainThread;

static void MaybeTerminateVLM(int signal)
{
    char *answer = NULL;
    size_t answerSize = 0, *answerSize_p = &answerSize;
    ssize_t nRead;

    if (NULL == pthread_getspecific(mainThread)) {
        return;
    }

    if (EmbCommAreaPtr->guestStatus > StartedGuestStatus) {
        if (RunningGuestStatus == EmbCommAreaPtr->guestStatus) {

            LogMessage0( "Lisp is running!\n");

        } else {
            LogMessage0( "Lisp WAS running!\n");
        }

        LogMessage0( "If you exit, the current state of Lisp will be lost.");
        LogMessage0( "All information in its memory image (e.g., any modified editor");
        LogMessage0( "buffers) will be irretrievably lost.  Further, Lisp will abandon");
        LogMessage0( "any tasks it is performing for its clients.");
        LogMessage0( "");
        LogMessage0( "Do you still wish to exit?  (yes or no) ");

        while (TRUE) {
            nRead = getline(&answer, answerSize_p, stdin);
            if (nRead < 0) {
                LogMessage0( "Unexpected EOF on standard input");
                vpunt(NULL, "Unexpected EOF on standard input");
            }
            answer[nRead - 1] = '\0';
            if (0 == strcmp(answer, "yes")) {
                break;
            } else if (0 == strcmp(answer, "no")) {
                return;
            } else {
                LogMessage0( "Please answer 'yes' or 'no'.");
            }
        }
    }

    // Seems useles:
    // TerminateTracing();
    // TerminateSpy();
    TerminateLifeSupport();

    _exit(EXIT_SUCCESS);
}

int main(int argc, char **argv)
{
    VLMConfig config;
    struct sigaction sigAction;
    Integer worldImageSize, worldImageMB;
    char *message;
    int reason;

    LogMessage0( "Starting main");

    LogMessage0( "Calling BuildConfiguration()");
    BuildConfiguration(&config, argc, argv);
    EnableIDS = config.enableIDS;

    TestFunction = config.testFunction;
    Trace = config.tracing.tracePOST;

    // TEMP_DISABLED
    // LogMessage( "Calling InitializeIvoryProcessor()");
    // InitializeIvoryProcessor(MapVirtualAddressData(0), MapVirtualAddressTag(0));

    // No tracing for the moment - Not defined in c-emulator
    // Trace = config.tracing.traceP;
    // if (Trace)
    //     InitializeTracing(
    //         config.tracing.bufferSize, config.tracing.startPC, config.tracing.stopPC, config.tracing.outputFile);

    // TEMP_DISABLED
    // LogMessage( "Calling InitializeLifeSupport()");
    // InitializeLifeSupport(&config);

#ifdef FE_NOMASK_ENV
    fesetenv(FE_NOMASK_ENV);
#else
    feenableexcept(FE_INEXACT | FE_DIVBYZERO | FE_UNDERFLOW | FE_OVERFLOW | FE_INVALID);
#endif

    if (pthread_key_create(&mainThread, NULL)) {
        LogMessage0( "Unable to establish per-thread data.");
        vpunt(NULL, "Unable to establish per-thread data.");
    }

    pthread_setspecific(mainThread, (void *)TRUE);

    sigAction.sa_handler = (sa_handler_t)MaybeTerminateVLM;
    sigemptyset(&sigAction.sa_mask);
    sigAction.sa_flags = 0;
    if (sigaction(SIGINT, &sigAction, NULL)) {
        LogMessage0( "Unable to establish per-thread data.");
        vpunt(NULL, "Unable to establish SIGINT handler.");
    }
    if (sigaction(SIGTERM, &sigAction, NULL)) {
        LogMessage0( "Unable to establish SIGTERM handler.");
        vpunt(NULL, "Unable to establish SIGTERM handler.");
    }
    if (sigaction(SIGHUP, &sigAction, NULL)) {
        LogMessage0( "Unable to establish SIGHUP handler.");
        vpunt(NULL, "Unable to establish SIGHUP handler.");
    }
    if (sigaction(SIGQUIT, &sigAction, NULL)) {
        LogMessage0( "Unable to establish SIGQUIT handler.");
        vpunt(NULL, "Unable to establish SIGQUIT handler.");
    }

    LogMessage0( "Calling LoadWorld()");
    worldImageSize = LoadWorld(&config);

    LogMessage0( "Calling LoadVLMDebugger()");
    LoadVLMDebugger(&config);

    worldImageMB = WordsToMB(worldImageSize);
    if (worldImageMB > config.virtualMemory) {
        LogMessage2( "World file %s won't fit within the requested virtual memory (%dMB)", config.worldPath,
            config.virtualMemory);
        vpunt(NULL, "World file %s won't fit within the requested virtual memory (%dMB)", config.worldPath,
            config.virtualMemory);
    }
    if ((2 * worldImageMB) > config.virtualMemory) {
        LogMessage2( "Only %dMB of virtual memory unused after loading world file %s\n",
            (config.virtualMemory - worldImageMB), config.worldPath);
        vwarn(NULL, "Only %dMB of virtual memory unused after loading world file %s\n",
            (config.virtualMemory - worldImageMB), config.worldPath);
    }

    VirtualMemoryWrite(SystemCommSlotAddress(enableSysoutAtColdBoot),
        (LispObj *)(EnableIDS ? processor->taddress : processor->niladdress));

    EmbCommAreaPtr->virtualMemorySize = MBToWords(config.virtualMemory);
    EmbCommAreaPtr->worldImageSize = worldImageSize;

    if (config.enableSpy) {
        InitializeSpy(TRUE, config.diagnosticIPAddress.s_addr);
    }

#ifdef AUTOSTART
    if (!IvoryProcessorSystemStartup(TRUE)) {
        vpunt(NULL, "Unable to start the VLM.");
    }
#endif

    if (config.enableSpy) {
        ReleaseSpyLock();
    }

    while (config.enableSpy ? TRUE : Runningp()) {
        LogMessage0( "Starting InstructionSequencer.");
        reason = InstructionSequencer();
        if (reason) {
            switch (reason) {
            case HaltReason_IllInstn:
                message = "Unimplemented instruction";
                break;

            case HaltReason_Halted:
                message = NULL;
                break;

            case HaltReason_SpyCalled:
                message = NULL;
                break;

            case HaltReason_FatalStackOverflow:
                message = "Stack overflow while not in emulator mode";
                break;

            case HaltReason_IllegalTrapVector:
                message = "Illegal trap vector contents";
                break;

            default:
                message = "Halted for unknown reason";
            }
            if (message != NULL) {
                vwarn(NULL, "%s at PC %08x (%s)", message, processor->epc >> 1, (processor->epc & 1) ? "Odd" : "Even");
            }
        }
        if (HaltReason_Halted == reason) {
            break;
        }
    }

    exit(EXIT_SUCCESS);
}
