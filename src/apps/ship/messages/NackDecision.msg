//
// an answer from sender to a received negative acknowledgement (NACK) request.
// either sent separately in a packet or added to a retransmitted packet
// Reason is so far always empty but will be eventually filled in the future
//

import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;

class NackDecision extends inet::FieldsChunk {
    chunkLength = inet::B(2);
    
    bool decision;
    string reason;
    simtime_t timestamp;

    // auxilary when it is sent alone
    string streamName;
    int sequenceNumber;

    // auxilary for multihome scenarios
    int networkType;
}

