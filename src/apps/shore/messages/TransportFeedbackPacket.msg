import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;
import TransportFeedbackChunk;

//
// an RTCP-like TransportFeedback (PT 205).
// it is a simplified version, we just have two arrays of seq numbers and deltas
// but the size of header is calculated according to draft-holmer-rmcat-transport-wide-cc-extensions-01
//

class TransportFeedbackPacket extends inet::FieldsChunk {
    chunkLength = inet::B(18); // + PT 205 RTCP header
    
    int baseSequenceNumber;
    int packetStatusCount;
    int feedbackStatusCount;
    
    cArray feedbackChunks;
}

cplusplus(TransportFeedbackPacket) {{
  public:
    /** adds a chunk (seq number + delta) to this transport feedback */
    virtual void addFeedbackChunk(TransportFeedbackChunk* chunk);
}}
