#
# OMNeT++/OMNEST Makefile for gymir5g
#
# This file was generated with the command:
#  opp_makemake -f --deep -KINET_PROJ=../../inet -KSIMU5G_PROJ=../../simu5G -KVEINS_PROJ=../../veins -KVEINS_INET_PROJ=../../veins_inet -DINET_IMPORT -DVEINS_IMPORT -DVEINS_INET_IMPORT -I/usr/local/include/opencv4 -I/usr/local/include/pcl-1.12 -I/usr/include/eigen3 -I/usr/include/vtk-7.1 -I$$\(INET_PROJ\)/src -I$$\(SIMU5G_PROJ\)/src -I$$\(VEINS_PROJ\)/src -I. -I$$\(VEINS_INET_PROJ\)/src -L/usr/local/lib -L/usr/lib -L/usr/local/lib/x86_64-linux-gnu/ -L$$\(INET_PROJ\)/src -L$$\(SIMU5G_PROJ\)/src -L$$\(VEINS_PROJ\)/src -L$$\(VEINS_INET_PROJ\)/src -lvmaf -lpcap -lx264 -lboost_thread -lboost_iostreams -lboost_serialization -lopencv_core -lopencv_highgui -lopencv_videoio -lopencv_features2d -lopencv_imgcodecs -lopencv_imgproc -lavformat -lavcodec -lavutil -lswscale -lpcl_common -lpcl_io -lpcl_visualization -lpcl_octree -lvtkCommonDataModel-7.1 -lvtkCommonCore-7.1 -lvtkRenderingCore-7.1 -lvtkCommonMath-7.1 -lpthread -lzmq -lINET$$\(D\) -lsimu5g$$\(D\) -lveins$$\(D\) -lveins_inet$$\(D\)
#

# Name of target to be created (-o option)
TARGET_DIR = .
TARGET_NAME = gymir5g$(D)
TARGET = $(TARGET_NAME)$(EXE_SUFFIX)
TARGET_IMPLIB = $(TARGET_NAME)$(IMPLIB_SUFFIX)
TARGET_IMPDEF = $(TARGET_NAME)$(IMPDEF_SUFFIX)
TARGET_FILES = $(TARGET_DIR)/$(TARGET)

# User interface (uncomment one) (-u option)
USERIF_LIBS = $(ALL_ENV_LIBS) # that is, $(QTENV_LIBS) $(CMDENV_LIBS)
#USERIF_LIBS = $(CMDENV_LIBS)
#USERIF_LIBS = $(QTENV_LIBS)

# C++ include paths (with -I)
INCLUDE_PATH = \
    -I/usr/local/include/opencv4 \
    -I/usr/local/include/pcl-1.12 \
    -I/usr/include/eigen3 \
    -I/usr/include/vtk-7.1 \
    -I$(INET_PROJ)/src \
    -I$(SIMU5G_PROJ)/src \
    -I$(VEINS_PROJ)/src \
    -I. \
    -I$(VEINS_INET_PROJ)/src

# Additional object and library files to link with
EXTRA_OBJS =

# Additional libraries (-L, -l options)
LIBS = $(LDFLAG_LIBPATH)/usr/local/lib $(LDFLAG_LIBPATH)/usr/lib $(LDFLAG_LIBPATH)/usr/local/lib/x86_64-linux-gnu/ $(LDFLAG_LIBPATH)$(INET_PROJ)/src $(LDFLAG_LIBPATH)$(SIMU5G_PROJ)/src $(LDFLAG_LIBPATH)$(VEINS_PROJ)/src $(LDFLAG_LIBPATH)$(VEINS_INET_PROJ)/src  -lvmaf -lpcap -lx264 -lboost_thread -lboost_iostreams -lboost_serialization -lopencv_core -lopencv_highgui -lopencv_videoio -lopencv_features2d -lopencv_imgcodecs -lopencv_imgproc -lavformat -lavcodec -lavutil -lswscale -lpcl_common -lpcl_io -lpcl_visualization -lpcl_octree -lvtkCommonDataModel-7.1 -lvtkCommonCore-7.1 -lvtkRenderingCore-7.1 -lvtkCommonMath-7.1 -lpthread -lzmq -lINET$(D) -lsimu5g$(D) -lveins$(D) -lveins_inet$(D)

# Output directory
PROJECT_OUTPUT_DIR = ../out
PROJECTRELATIVE_PATH = src
O = $(PROJECT_OUTPUT_DIR)/$(CONFIGNAME)/$(PROJECTRELATIVE_PATH)

# Object files for local .cc, .msg and .sm files
OBJS = \
    $O/apps/ship/ShipApp.o \
    $O/apps/shore/ShoreApp.o \
    $O/apps/shore/messages/ReceiverReportPacket.o \
    $O/apps/shore/messages/ReportBlock.o \
    $O/apps/shore/messages/TransportFeedbackChunk.o \
    $O/apps/shore/messages/TransportFeedbackPacket.o \
    $O/model/learning/algorithms/AbrBase.o \
    $O/model/learning/algorithms/AbrGcc.o \
    $O/model/learning/algorithms/DrlBase.o \
    $O/model/media/lidar/PointCloudDecoder.o \
    $O/model/media/lidar/PointCloudEncoder.o \
    $O/model/media/lidar/VLPLidarGrabber.o \
    $O/model/media/video/VideoCapturer.o \
    $O/model/media/video/VideoDecoder.o \
    $O/model/media/video/VideoEncoderH264.o \
    $O/model/states/DataTransmissionManager.o \
    $O/model/states/DataTransmissionState.o \
    $O/model/states/PhyDataState.o \
    $O/model/states/StreamState.o \
    $O/model/states/UnitState.o \
    $O/model/stream/Stream.o \
    $O/model/stream/StreamActions.o \
    $O/model/stream/StreamManager.o \
    $O/model/stream/StreamReceiver.o \
    $O/omnet/apps/multihome/ForwardShipApp.o \
    $O/omnet/apps/multihome/ForwardShoreApp.o \
    $O/omnet/apps/multihome/ForwardUdpApp.o \
    $O/omnet/apps/multihome/MultiHomeUdpApp.o \
    $O/omnet/mobility/docking/DockingMobility.o \
    $O/omnet/mobility/multihome/MultiHomeAttachedMobility.o \
    $O/omnet/stack/phy/NRPhyUeWithFeedback.o \
    $O/omnet/stack/phy/channelModel/NRChannelModelWithFeedback.o \
    $O/service/GymCommunicator.o \
    $O/service/SignalListener.o \
    $O/apps/ship/messages/FecPacket_m.o \
    $O/apps/ship/messages/NackDecision_m.o \
    $O/apps/ship/messages/SenderReportPacket_m.o \
    $O/apps/ship/messages/ShipOffsetMessage_m.o \
    $O/apps/ship/messages/ShipPacket_m.o \
    $O/apps/shore/messages/AckPacket_m.o \
    $O/apps/shore/messages/NackPacket_m.o \
    $O/apps/shore/messages/ReceiverReportPacket_m.o \
    $O/apps/shore/messages/ReportBlock_m.o \
    $O/apps/shore/messages/TransportFeedbackChunk_m.o \
    $O/apps/shore/messages/TransportFeedbackPacket_m.o \
    $O/omnet/apps/multihome/messages/MultiHomeMessage_m.o \
    $O/omnet/apps/multihome/messages/MultiHomePacket_m.o

# Message files
MSGFILES = \
    apps/ship/messages/FecPacket.msg \
    apps/ship/messages/NackDecision.msg \
    apps/ship/messages/SenderReportPacket.msg \
    apps/ship/messages/ShipOffsetMessage.msg \
    apps/ship/messages/ShipPacket.msg \
    apps/shore/messages/AckPacket.msg \
    apps/shore/messages/NackPacket.msg \
    apps/shore/messages/ReceiverReportPacket.msg \
    apps/shore/messages/ReportBlock.msg \
    apps/shore/messages/TransportFeedbackChunk.msg \
    apps/shore/messages/TransportFeedbackPacket.msg \
    omnet/apps/multihome/messages/MultiHomeMessage.msg \
    omnet/apps/multihome/messages/MultiHomePacket.msg

# SM files
SMFILES =

# Other makefile variables (-K)
INET_PROJ=../../inet
SIMU5G_PROJ=../../simu5G
VEINS_PROJ=../../veins
VEINS_INET_PROJ=../../veins_inet

#------------------------------------------------------------------------------

# Pull in OMNeT++ configuration (Makefile.inc)

ifneq ("$(OMNETPP_CONFIGFILE)","")
CONFIGFILE = $(OMNETPP_CONFIGFILE)
else
CONFIGFILE = $(shell opp_configfilepath)
endif

ifeq ("$(wildcard $(CONFIGFILE))","")
$(error Config file '$(CONFIGFILE)' does not exist -- add the OMNeT++ bin directory to the path so that opp_configfilepath can be found, or set the OMNETPP_CONFIGFILE variable to point to Makefile.inc)
endif

include $(CONFIGFILE)

# Simulation kernel and user interface libraries
OMNETPP_LIBS = $(OPPMAIN_LIB) $(USERIF_LIBS) $(KERNEL_LIBS) $(SYS_LIBS)
ifneq ($(PLATFORM),win32)
LIBS += -Wl,-rpath,$(abspath /usr/local/lib) -Wl,-rpath,$(abspath /usr/lib) -Wl,-rpath,$(abspath /usr/local/lib/x86_64-linux-gnu/) -Wl,-rpath,$(abspath $(INET_PROJ)/src) -Wl,-rpath,$(abspath $(SIMU5G_PROJ)/src) -Wl,-rpath,$(abspath $(VEINS_PROJ)/src) -Wl,-rpath,$(abspath $(VEINS_INET_PROJ)/src)
endif

COPTS = $(CFLAGS) $(IMPORT_DEFINES) -DINET_IMPORT -DVEINS_IMPORT -DVEINS_INET_IMPORT $(INCLUDE_PATH) -I$(OMNETPP_INCL_DIR)
MSGCOPTS = $(INCLUDE_PATH)
SMCOPTS =

# we want to recompile everything if COPTS changes,
# so we store COPTS into $COPTS_FILE (if COPTS has changed since last build)
# and make the object files depend on it
COPTS_FILE = $O/.last-copts
ifneq ("$(COPTS)","$(shell cat $(COPTS_FILE) 2>/dev/null || echo '')")
  $(shell $(MKPATH) "$O")
  $(file >$(COPTS_FILE),$(COPTS))
endif

#------------------------------------------------------------------------------
# User-supplied makefile fragment(s)
-include makefrag

#------------------------------------------------------------------------------

# Main target
all: $(TARGET_FILES)

$(TARGET_DIR)/% :: $O/%
	@mkdir -p $(TARGET_DIR)
	$(Q)$(LN) $< $@
ifeq ($(TOOLCHAIN_NAME),clang-msabi)
	-$(Q)-$(LN) $(<:%.dll=%.lib) $(@:%.dll=%.lib) 2>/dev/null

$O/$(TARGET_NAME).pdb: $O/$(TARGET)
endif

$O/$(TARGET): $(OBJS)  $(wildcard $(EXTRA_OBJS)) Makefile $(CONFIGFILE)
	@$(MKPATH) $O
	@echo Creating executable: $@
	$(Q)$(CXX) $(LDFLAGS) -o $O/$(TARGET) $(OBJS) $(EXTRA_OBJS) $(AS_NEEDED_OFF) $(WHOLE_ARCHIVE_ON) $(LIBS) $(WHOLE_ARCHIVE_OFF) $(OMNETPP_LIBS)

.PHONY: all clean cleanall depend msgheaders smheaders

.SUFFIXES: .cc

$O/%.o: %.cc $(COPTS_FILE) | msgheaders smheaders
	@$(MKPATH) $(dir $@)
	$(qecho) "$<"
	$(Q)$(CXX) -c $(CXXFLAGS) $(COPTS) -o $@ $<

%_m.cc %_m.h: %.msg
	$(qecho) MSGC: $<
	$(Q)$(MSGC) -s _m.cc -MD -MP -MF $O/$(basename $<)_m.h.d $(MSGCOPTS) $?

%_sm.cc %_sm.h: %.sm
	$(qecho) SMC: $<
	$(Q)$(SMC) -c++ -suffix cc $(SMCOPTS) $?

msgheaders: $(MSGFILES:.msg=_m.h)

smheaders: $(SMFILES:.sm=_sm.h)

clean:
	$(qecho) Cleaning $(TARGET)
	$(Q)-rm -rf $O
	$(Q)-rm -f $(TARGET_FILES)
	$(Q)-rm -f $(call opp_rwildcard, . , *_m.cc *_m.h *_sm.cc *_sm.h)

cleanall:
	$(Q)$(CLEANALL_COMMAND)
	$(Q)-rm -rf $(PROJECT_OUTPUT_DIR)

help:
	@echo "$$HELP_SYNOPSYS"
	@echo "$$HELP_TARGETS"
	@echo "$$HELP_VARIABLES"
	@echo "$$HELP_EXAMPLES"

# include all dependencies
-include $(OBJS:%=%.d) $(MSGFILES:%.msg=$O/%_m.h.d)
