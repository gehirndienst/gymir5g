import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;

//
// FEC artificial packet to simulate repairing lost packets at the receiver side
//

class FecPacket extends inet::FieldsChunk {
    chunkLength = inet::B(10);
    
    string streamName;
    int firstSequenceNumber;
    int lastSequenceNumber;
    int fecCount;

    // auxilary for multihome scenarios
    int networkType;
}
