#include "TransportFeedbackChunk_m.h"

Register_Class(TransportFeedbackChunk);

std::string TransportFeedbackChunk::str() const {
    std::stringstream out;
    out << "TransportFeedbackChunk stream name=" << getStreamName();
    return out.str();
}

void TransportFeedbackChunk::dump(std::ostream& os) const {
    os << "TransportFeedbackChunk:" << std::endl;
    os << "  stream name = " << getStreamName() << std::endl;
    os << "  sequenceNumber = " << getSequenceNumber() << std::endl;
    os << "  recvDelta = " << getRecvDelta() << std::endl;
}
