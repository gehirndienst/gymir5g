package gymir5g.omnet.stack.phy;

import simu5g.stack.phy.NRPhyUe;

//
// simu5G's NRPhyUe model with DL and UL feedback emission to use at the application layer
//
// Created on: May 1, 2023
//     Author: Nikita Smirnov
//

simple NRPhyUeWithFeedback extends NRPhyUe {
    parameters:
        @class("NRPhyUeWithFeedback");
        double feedbackStartTime @unit(s) = default(0.1s);
        double logUpdatePeriod @unit(s) = default(0s);
        int verbose = default(0);
}