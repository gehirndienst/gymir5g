//
// negative acknowledgement (NACK) packet for a single lost seq number
//

import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;

class NackPacket extends inet::FieldsChunk {
    chunkLength = inet::B(1);
    
    int sequenceNumber;
    
    // for simplicity
    string streamName;
}
