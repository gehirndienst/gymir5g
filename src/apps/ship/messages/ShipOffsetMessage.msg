
// internal message to provide send one-by-one functionality for stream fragments by iterating via offset

message ShipOffsetMessage {
    string streamName;
    int offset;
}