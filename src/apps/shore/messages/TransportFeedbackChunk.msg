import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;

//
// an auxilary class for Transport Feedback chunks that consitute Transport Feedback 
//

class TransportFeedbackChunk extends inet::FieldsChunk {
    chunkLength = inet::B(3);
    
    string streamName; // auxilary
    int sequenceNumber;
    simtime_t recvDelta;
};

cplusplus(TransportFeedbackChunk) {{
  public:
    virtual std::string str() const override;
    virtual void dump(std::ostream& os) const;
}}