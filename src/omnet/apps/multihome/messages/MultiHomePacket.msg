import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;

class MultiHomePacket extends inet::FieldsChunk {  
	int id;
	int networkType;
	string destAddress;
	int destPort;
	int payloadSize;
	simtime_t sendingTime;
}