//
// ack (ACK, SACK) packet
//

import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;

class AckPacket extends inet::FieldsChunk {
    chunkLength = inet::B(1);
    
    int sequenceNumber;
    
    // for simplicity
    string streamName;
    int payloadSize;
    simtime_t senderTimestamp;
    simtime_t arrivalTimestamp;
}