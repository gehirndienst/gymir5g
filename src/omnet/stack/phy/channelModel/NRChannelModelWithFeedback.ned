package gymir5g.omnet.stack.phy.channelModel;

import simu5g.stack.phy.ChannelModel.NRChannelModel_3GPP38_901;

//
// simu5G's NRChannelModel_3GPP38_901 channel model with sendDirect method to ::NRPhyUeWithFeedback PHY Layer that emits DL/UL SINR/RSRP signals
//
// Created on: May 4, 2023
//     Author: Nikita Smirnov
//

simple NRChannelModelWithFeedback extends NRChannelModel_3GPP38_901 {
	parameters:
    	@class("NRChannelModelWithFeedback");
}
