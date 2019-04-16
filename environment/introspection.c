
#include "introspection.h"
#include "life_prototypes.h"

#define REMOTE_MEMORY_ALIGNED_PACKET_HEADER 8
#define REMOTE_MEMORY_PACKET_DATA 1284
#define REMOTE_MEMORY_PACKET_HEADER 10

typedef enum {
    rm_discard,
    rm_noop,
    rm_ack,
    rm_write,
    rm_read,
    rm_system_startup,
    rm_trap,
    rm_boot,
    rm_create_pages,
    rm_mbin,
    rm_stop
} remote_memory_opcode;

static struct sockaddr_in mbin_sin;

struct rm_pkt {
    unsigned char rm_pad[2];
    unsigned char rm_id[4];
    unsigned char rm_operand[3];
    int rm_opcode : 8;
    unsigned char data[REMOTE_MEMORY_PACKET_DATA];
};

struct rm_aligned_pkt {
    unsigned char rm_id[4];
    unsigned char rm_operand[3];
    int rm_opcode : 8;
    unsigned char data[REMOTE_MEMORY_PACKET_DATA];
};

static struct {
    unsigned int id;
    boolean acked;
} MBINHistory[16];


static boolean mbin_sinValid = FALSE;
EmbMBINChannel *activeMBINChannel = NULL;


static unsigned int read_long(unsigned char *bytes)
{
    return (bytes[0] | (bytes[1] << 8) | (bytes[2] << 16) | (bytes[3] << 24));
}

static void write_long(unsigned char *bytes, unsigned int data)
{
    bytes[0] = data;
    bytes[1] = data >> 8;
    bytes[2] = data >> 16;
    bytes[3] = data >> 24;
}


void SendMBINBuffers(EmbMBINChannel *mbinChannel)
{
    EmbQueue *gthQ = mbinChannel->guestToHostQueue;
    EmbQueue *gthrQ = mbinChannel->guestToHostReturnQueue;
    EmbPtr bufferPtr;
    struct rm_aligned_pkt *buffer;
    struct rm_pkt pkt;
    unsigned int nBytes, id;
    int historyID;

    if (mbinChannel->header.messageChannel->guestToHostImpulse)
        switch (mbinChannel->header.messageChannel->guestToHostImpulse) {
        case EmbMBINImpulseShutdown:
            activeMBINChannel = NULL;
            ResetIncomingQueue(gthQ);
            ResetOutgoingQueue(gthrQ);
            ResetIncomingQueue(mbinChannel->hostToGuestSupplyQueue);
            ResetOutgoingQueue(mbinChannel->hostToGuestQueue);
            mbinChannel->header.messageChannel->guestToHostImpulse = EmbMessageImpulseNone;
            UnthreadMessageChannel(mbinChannel->header.messageChannel);
            free(mbinChannel);
            return;
        default:
            mbinChannel->header.messageChannel->guestToHostImpulse = EmbMessageImpulseNone;
            break;
        }

    while (EmbQueueFilled(gthQ)) {
        if (0 == EmbQueueSpace(gthrQ)) {
            SignalLater(gthQ->signal);
            return;
        }
        bufferPtr = EmbQueueTakeWord(gthQ);
        if (bufferPtr && (bufferPtr != NullEmbPtr) && mbin_sinValid) {
            buffer = (struct rm_aligned_pkt *)HostPointer(bufferPtr);
            nBytes = read_long(&buffer->rm_operand[0]) & 0xFFFFFF;
            memcpy(&pkt.rm_id[0], &buffer->rm_id[0], REMOTE_MEMORY_ALIGNED_PACKET_HEADER);
            memcpy(&pkt.data[0], &buffer->data[0], nBytes);
            if (rm_ack == buffer->rm_opcode) {
                id = read_long(&buffer->rm_id[0]);
                historyID = id & 0xF;
                MBINHistory[historyID].id = id;
                MBINHistory[historyID].acked = TRUE;
            }
        }
        EmbQueuePutWord(gthrQ, bufferPtr);
    }
}

