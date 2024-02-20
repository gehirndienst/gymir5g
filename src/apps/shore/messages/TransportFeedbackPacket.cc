#include <iostream>
#include <sstream>

#include "TransportFeedbackChunk_m.h"
#include "TransportFeedbackPacket_m.h"

void TransportFeedbackPacket::addFeedbackChunk(TransportFeedbackChunk* chunk) {
    handleChange();
    feedbackChunks.add(chunk);
    packetStatusCount++;
    addChunkLength(chunk->getChunkLength());
    /*
       packet chunk:  16 bits A list of packet status chunks.  These
               indicate the status of a number of packets starting with
               the one identified by base sequence number.  See below
               for details.

       recv delta: 8 bits For each "packet received" status, in the packet
               status chunks, a receive delta block will follow.  See
               details below.
    */
};
