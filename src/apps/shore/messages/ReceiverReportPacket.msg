import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;
import ReportBlock;

//
// an RTCP-like ReceiverReport (PT 201).
// some real parameters are excluded but the header size is kept 
//

class ReceiverReportPacket extends inet::FieldsChunk {
    chunkLength = inet::B(4); // PT 201 RTCP header
    
    int feedbackCount;
    cArray reportBlocks;
}

cplusplus(ReceiverReportPacket) {{
  public:
    /** adds a report block to this receiver report */
    virtual void addReportBlock(ReportBlock* reportBlock);
}}