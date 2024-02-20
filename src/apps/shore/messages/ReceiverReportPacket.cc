#include <iostream>
#include <sstream>

#include "ReceiverReportPacket_m.h"
#include "ReportBlock_m.h"

void ReceiverReportPacket::addReportBlock(ReportBlock* block) {
    handleChange();
    reportBlocks.add(block);
    addChunkLength(block->getChunkLength());
};
