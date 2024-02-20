//
// an RTP-like packet filled with the ship data: video, lidar, simulated
// some real parameters are reinterpreted for more user-friendliness but the header size is kept 
//

import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;

class ShipPacket extends inet::FieldsChunk {
    chunkLength = inet::B(14); // fixed according to RFC
    
    int transportWideSequenceNumber; // global id for transport feedback
    int sequenceNumber; // stream packet id

    string streamName;
    string streamType; // "video", "lidar", "blob", "sim"

    int elemNumber; // sequence number of a stream element, e.g., a video frame
    int fragmentOffset; // fragment of a current stream element, e.g., a NAL of the x264 encoder
    bool isLastFragment; // extra decoding info

    int payloadSize; // in bytes
    simtime_t payloadTimestamp; // squash RTP and NTP timestamps

    // RTC info
    bool isMarked; // if SACK is requested for this packet, mirrors "marker" in RTP packet

    // network type
    int networkType;
}
