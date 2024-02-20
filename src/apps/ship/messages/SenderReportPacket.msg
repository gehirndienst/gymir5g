import inet.common.INETDefs;
import inet.common.packet.Packet;

//
// an RTCP-like SenderReport (PT 200).
// some real parameters are excluded but the header size is kept 
//
class SenderReportPacket extends inet::FieldsChunk {
    chunkLength = inet::B(36); // fixed according to RFC
    
    string streamName; // ssrc    
    int senderPacketCount;
    long senderOctetCount;
    simtime_t timestamp;

    // auxilary for multihome scenarios
    int networkType;
}