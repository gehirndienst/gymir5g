FROM ubuntu:20.04

ENV BOOST_VERSION=1.81.0
ENV CMAKE_VERSION=3.25.1
ENV OPENCV_VERSION=4.7.0
ENV PCL_VERSION=1.13.0
ENV FFMPEG_VERSION=5.1.3

ENV OMNETPP_VERSION=6.0.1
ENV INET_VERSION=4.5.2
ENV SIMU5G_VERSION=1.2.2
ENV VEINS_VERSION=5.2
ENV SUMO_VERSION=1.14.1

ENV PYTHON_VERSION=3.11.4

ENV LANGUAGE=en_US.UTF-8
ENV LC_ALL=en_US.UTF-8
ENV LANG=en_US.UTF-8
ENV LC_CTYPE=en_US.UTF-8
ENV DEBIAN_FRONTEND=noninteractive
SHELL ["/bin/bash", "-c"]

LABEL description \
    "A dockerfile for Gymir5G project together with OMNET++, INET, SIMU5G, VEINS+SUMO stack. \
    Gymir5G runs with C++20x support and is built using GCC 10.3 as the compiler. \
    The following additional libraries are installed and built: BOOST, OPENCV, PCL+VTK, MPI stack. Check versions in ENV block. \
    FFMPEG is built from sources and supports H264, H265, VP8, VP9, AV1 video encoders and decoders. \
    This image is running with CPU only. Check gymir5g-cuda:* image for CUDA support."
LABEL maintainer "Nikita Smirnov <nsm@informatik.uni-kiel.de>"
LABEL version "1.3.1"

# INSTALL ALL COMMON DEPENDECIES HERE
RUN apt-get update -y && apt-get upgrade -y && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends --fix-missing \
    gcc g++ gdb make gdb bison flex perl doxygen graphviz autoconf automake ccache nano mpi-default-dev openssh-client pkg-config xdg-utils unzip \
    build-essential wget curl bzip2 sudo ca-certificates gnupg software-properties-common yasm nasm ninja-build \
    default-jre libxml2-dev zlib1g-dev python3-posix-ipc qt5-default libqt5opengl5-dev \
    libgtk-3-0 libgtk-3-dev libwebkit2gtk-4.0-37 libglx-dev libtiff-dev libtiff5 libjpeg-dev libpng-dev default-jre libosgearth-dev openscenegraph-plugin-osgearth \
    libxerces-c-dev libfox-1.6-0 libfox-1.6-dev libprotobuf-dev protobuf-compiler openmpi-bin openmpi-common libopenmpi-dev \
    libv4l-dev v4l-utils libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev libopenni-dev libopenni2-dev libpcap-dev libqhull-dev libgtest-dev \
    mesa-utils mesa-common-dev libgl1-mesa-dri libgtkgl2.0-dev libgtkglext1-dev libatlas-base-dev libeigen3-dev \
    libflann-dev freeglut3-dev libusb-1.0-0-dev libusb-dev \
    libass-dev libfdk-aac-dev libmp3lame-dev libopus-dev libvorbis-dev libsdl2-dev libxine2-dev \
    libxext6 libsm6 libxrender1 libssl-dev git mercurial subversion \
    libxt-dev libtool libzmq3-dev \
    python3 python3-dev python3-pip && \
    ldconfig && \
    pip install --upgrade pip && pip install posix-ipc scipy pandas matplotlib zmq h5py sureal meson cython && \
    mkdir /usr/share/desktop-directories/ && \
    apt-get clean


# INSTALL AND SET GCC 10 AS BASIC COMPILER FOR C++20 SUPPORT
RUN add-apt-repository -y ppa:ubuntu-toolchain-r/test && apt-get update -y && apt-get upgrade -y && apt-get install -y gcc-10 g++-10 cpp-10
RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-10 20 --slave /usr/bin/g++ g++ /usr/bin/g++-10 --slave /usr/bin/gcov gcov /usr/bin/gcov-10

# BUILD CMAKE
WORKDIR /tmp
RUN wget https://github.com/Kitware/CMake/releases/download/v${CMAKE_VERSION}/cmake-${CMAKE_VERSION}.tar.gz && \
    tar xzf cmake-${CMAKE_VERSION}.tar.gz && \
    cd cmake-${CMAKE_VERSION} && \
    ./bootstrap && \
    make -j$(nproc) && \
    make install

# BUILD BOOST
WORKDIR /tmp
RUN BOOST_VERSION_MOD=$(echo $BOOST_VERSION | tr . _) && \
    wget https://boostorg.jfrog.io/artifactory/main/release/${BOOST_VERSION}/source/boost_${BOOST_VERSION_MOD}.tar.bz2 && \
    tar --bzip2 -xf boost_${BOOST_VERSION_MOD}.tar.bz2 && \
    cd boost_${BOOST_VERSION_MOD} && \
    ./bootstrap.sh --prefix=/usr/local && \
    ./b2 install

# BUILD NLOHMANN JSON LIBRARY
WORKDIR /tmp
RUN git clone https://github.com/nlohmann/json.git && \
    cd json && \
    mkdir build && \
    cd build && \
    cmake .. && \
    make -j${nproc} && \
    make install

# BUILD NETFLIX VMAF (FOR VOQ METRICS)
WORKDIR /tmp
RUN git clone https://github.com/Netflix/vmaf.git && \
    cd vmaf/libvmaf && \
    meson build --buildtype release && \
    ninja -vC build && \
    ninja -vC build install
ENV LD_LIBRARY_PATH $LD_LIBRARY_PATH:/usr/local/lib/x86_64-linux-gnu/

# BUILD VIDEO ENCODERS/DECODERS
WORKDIR /tmp
#x264
RUN git -C x264 pull 2> /dev/null || git clone --depth 1 https://code.videolan.org/videolan/x264.git && \
    cd x264 && \
    ./configure --enable-shared --enable-pic && \
    make -j${nproc} && make install
#x265
RUN apt-get install libnuma-dev && wget -O x265.tar.bz2 https://bitbucket.org/multicoreware/x265_git/get/master.tar.bz2 && tar xjvf x265.tar.bz2 && \
    cd multicoreware*/build/linux && \
    cmake -G "Unix Makefiles" -DENABLE_SHARED=on ../../source && \
    make -j${nproc} && make install
#vpx:8,9
RUN git -C libvpx pull 2> /dev/null || git clone --depth 1 https://chromium.googlesource.com/webm/libvpx.git && \
    cd libvpx && \
    ./configure --disable-examples --disable-unit-tests --enable-vp9-highbitdepth --enable-pic --as=yasm && \
    make -j${nproc} && make install
#aom:av1 encoder/decoder
RUN git -C aom pull 2> /dev/null || git clone --depth 1 https://aomedia.googlesource.com/aom && \
    mkdir -p aom_build && \
    cd aom_build && \
    cmake -G "Unix Makefiles" -DENABLE_TESTS=OFF -DENABLE_NASM=on ../aom && \
    make -j${nproc} && make install
#av1 decoder
RUN git -C dav1d pull 2> /dev/null || git clone --depth 1 https://code.videolan.org/videolan/dav1d.git && \
    mkdir -p dav1d/build && \
    cd dav1d/build && \
    meson setup -Denable_tools=false -Denable_tests=false .. && \
    ninja && ninja install
##av1 encoder. NOTE: currently fails to compile with ffmpegv5 
#RUN git -C SVT-AV1 pull 2> /dev/null || git clone https://gitlab.com/AOMediaCodec/SVT-AV1.git && \
#    mkdir -p SVT-AV1/build && cd SVT-AV1/build && \
#    cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DBUILD_DEC=OFF -DBUILD_SHARED_LIBS=OFF .. && \
#    make -j${nproc} && make install

# BUILD FFMPEG
WORKDIR /tmp
RUN apt-get purge --auto-remove ffmpeg 
RUN wget -O ffmpeg-${FFMPEG_VERSION}.tar.bz2 https://ffmpeg.org/releases/ffmpeg-${FFMPEG_VERSION}.tar.bz2 && \
    tar xjvf ffmpeg-${FFMPEG_VERSION}.tar.bz2 && \
    cd ffmpeg-${FFMPEG_VERSION} && \
    ./configure \
    --extra-cflags="-fPIC" \
    --extra-ldflags="-Wl,-Bsymbolic" \
    --extra-libs="-lpthread -lm" \
    --ld="g++" \
    --enable-gpl \
    --enable-ffplay \
    --enable-ffprobe \
    --enable-libaom \
    --enable-libass \
    --enable-libdav1d \
    --enable-libfdk-aac \
    --enable-libfreetype \
    --enable-libmp3lame \
    --enable-libopus \
    #--enable-libsvtav1 \
    --enable-libvorbis \
    --enable-libvpx \
    --enable-libx264 \
    --enable-libx265 \
    --enable-nonfree \
    --enable-shared && \
    make -j${nproc} && \
    make install && \
    hash -r

# BUILD OPENCV
WORKDIR /tmp
RUN cp -ar /usr/lib/x86_64-linux-gnu/openmpi/include/. /usr/include/
RUN wget https://github.com/opencv/opencv/archive/refs/tags/${OPENCV_VERSION}.zip && unzip ${OPENCV_VERSION}.zip && rm ${OPENCV_VERSION}.zip
RUN wget https://github.com/opencv/opencv_contrib/archive/${OPENCV_VERSION}.zip && unzip ${OPENCV_VERSION}.zip && rm ${OPENCV_VERSION}.zip
RUN mkdir opencv-${OPENCV_VERSION}/build && \
    cd opencv-${OPENCV_VERSION}/build && \
    cmake \
    -DOPENCV_EXTRA_MODULES_PATH=/tmp/opencv_contrib-${OPENCV_VERSION}/modules \
    -DBUILD_SHARED_LIBS=ON \
    -DCMAKE_INSTALL_PREFIX=/usr/local \
    -DCMAKE_BUILD_TYPE=RELEASE \
    -DOPENCV_GENERATE_PKGCONFIG=ON \
    -DENABLE_FAST_MATH=ON \
    -DOPENCV_ENABLE_NONFREE=ON \
    -DWITH_V4L=ON \
    -DWITH_1394=OFF \
    -DWITH_GTK=ON \
    -DWITH_GTK_2_X=ON \
    -DWITH_QT=OFF \
    -DWITH_OPENGL=ON \
    -DWITH_EIGEN=ON \
    -DWITH_FFMPEG=ON \
    -DWITH_GSTREAMER=ON \
    -DBUILD_TESTS=OFF \
    -DBUILD_PERF_TESTS=OFF \
    -DBUILD_EXAMPLES=OFF \
    -DBUILD_opencv_apps=ON \
    .. && \
    make -j$(nproc) && \
    make install && \
    ldconfig

# BUILD VTK (FOR PCL)
WORKDIR /tmp
RUN wget https://www.vtk.org/files/release/9.2/VTK-9.2.6.tar.gz \
    && tar -xf VTK-9.2.6.tar.gz \
    && cd VTK-9.2.6 && mkdir build && cd build \
    && cmake .. -DVTK_Group_Imaging=ON -DVTK_Group_MPI=ON -DCMAKE_BUILD_TYPE=Release \
    && make -j$(nproc) \
    && make install

# BUILD PCL
WORKDIR /tmp
RUN wget https://github.com/PointCloudLibrary/pcl/archive/pcl-${PCL_VERSION}.tar.gz \
    && tar -xf pcl-${PCL_VERSION}.tar.gz \
    && cd pcl-pcl-${PCL_VERSION} \
    && mkdir build \
    && cd build \
    && cmake .. -DCMAKE_BUILD_TYPE=Release -DVTK_RENDERING_BACKEND=OpenGL2 \
    && make -j$(nproc) \
    && make install

# BUILD OMNET++
WORKDIR /opt
RUN mkdir -p /omnetpp && \
    curl -L https://github.com/omnetpp/omnetpp/releases/download/omnetpp-${OMNETPP_VERSION}/omnetpp-${OMNETPP_VERSION}-linux-x86_64.tgz| \
    tar -zxv  && cd omnetpp-${OMNETPP_VERSION}/ && \
    source setenv -f && \
    ./configure PREFER_CLANG=no PREFER_LLD=no WITH_PARSIM=yes WITH_OSG=yes WITH_OSGEARTH=yes && \
    make -j$(nproc) MODE=debug && make -j$(nproc) MODE=release && \
    find . -type f -name '*.o' -exec rm {} +
ENV PATH /opt/omnetpp-${OMNETPP_VERSION}/bin:$PATH

# BUILD INET
WORKDIR /opt/omnetpp-workspace
RUN mkdir -p /inet && \
    curl -L https://github.com/inet-framework/inet/releases/download/v${INET_VERSION}/inet-${INET_VERSION}-src.tgz | \
    tar -zxv
WORKDIR /opt/omnetpp-workspace/inet4.5
COPY feature_states/inet/.oppfeaturestate ./
RUN source setenv -f && \
    make makefiles && \
    make -j$(nproc) MODE=debug CONFIGNAME=gcc-debug && make -j$(nproc) MODE=release CONFIGNAME=gcc-release && \
    find . -type f -name '*.o' -exec rm {} +
ENV PATH /opt/omnetpp-workspace/inet/bin:$PATH

# BUILD SIMU5G
WORKDIR /opt/omnetpp-workspace
RUN mkdir -p /simu5G && \
    curl -L https://github.com/Unipisa/Simu5G/archive/refs/tags/v${SIMU5G_VERSION}.tar.gz | \
    tar -zxv && mv -T Simu5G-${SIMU5G_VERSION} simu5G
WORKDIR /opt/omnetpp-workspace/simu5G
COPY feature_states/simu5G/.oppfeaturestate ./
RUN source setenv -f && \
    make makefiles && \
    make -j$(nproc) MODE=debug && make -j$(nproc) MODE=release && \
    find . -type f -name '*.o' -exec rm {} +
ENV PATH /opt/omnetpp-workspace/simu5G/bin:$PATH

# BUILD SUMO
WORKDIR /opt
RUN mkdir -p /sumo && \
    curl -L https://downloads.sourceforge.net/project/sumo/sumo/version%20${SUMO_VERSION}/sumo-src-${SUMO_VERSION}.tar.gz | \
    tar -zxv && mv -T sumo-${SUMO_VERSION} sumo && \
    cd sumo && mkdir build/cmake-build && cd build/cmake-build && cmake ../.. && \
    make -j$(nproc) && \
    make install && \
    find . -type f -name '*.o' -exec rm {} +
ENV SUMO_HOME /opt/sumo

# BUILD VEINS
WORKDIR /opt/omnetpp-workspace
RUN mkdir -p /veins && \
    wget https://veins.car2x.org/download/veins-${VEINS_VERSION}.zip && unzip veins-${VEINS_VERSION}.zip && \
    mv -T veins-veins-${VEINS_VERSION} veins && rm veins-${VEINS_VERSION}.zip && \
    cd veins && source setenv -f && \
    ./configure && \
    make -j$(nproc) MODE=debug && make -j$(nproc) MODE=release && \
    find . -type f -name '*.o' -exec rm {} +
ENV PATH /opt/omnetpp-workspace/veins/bin:$PATH
ENV VEINS_HOME /opt/omnetpp-workspace/veins

# VEINS_INET BUILD FAILS IF INET IS NOT IN 'INET' FOLDER
RUN cp -ar inet4.5/. inet && rm -rf inet4.5

# COPY .PROJECT TO SIMU5G TO CORRECTLY REFERENCE ALL THE PROJECTS
WORKDIR /opt/omnetpp-workspace/simu5G
COPY project_references/simu5G/.project ./

# BUILD VEINS_INET
WORKDIR /opt/omnetpp-workspace/veins/subprojects/veins_inet
RUN ./configure --with-inet=/opt/omnetpp-workspace/inet && \
    make -j$(nproc) MODE=debug && make -j$(nproc) MODE=release && \
    find . -type f -name '*.o' -exec rm {} +

# GIVE ROOT RIGHTS
#RUN chmod +x /opt/omnetpp-workspace/

# X11 FORWARDING
RUN apt-get update -y \
    && apt-get install -y openssh-server dbus-x11 xauth xorg xpra apt-transport-https software-properties-common ca-certificates \
    && mkdir /var/run/sshd \
    && mkdir /root/.ssh \
    && chmod 700 /root/.ssh \
    && ssh-keygen -A \
    && sed -i "s/^.*PasswordAuthentication.*$/PasswordAuthentication no/" /etc/ssh/sshd_config \
    && sed -i "s/^.*X11Forwarding.*$/X11Forwarding yes/" /etc/ssh/sshd_config \
    && sed -i "s/^.*X11UseLocalhost.*$/X11UseLocalhost no/" /etc/ssh/sshd_config \
    && grep "^X11UseLocalhost" /etc/ssh/sshd_config || echo "X11UseLocalhost no" >> /etc/ssh/sshd_config \
    && echo "export NO_AT_BRIDGE=1" >> ~/.bashrc
ENV DISPLAY :0

# USERS
# add a sudo user nsmirnov
RUN useradd -m -s /bin/bash nsmirnov
RUN echo 'nsmirnov:2375140' | chpasswd
RUN usermod -aG sudo nsmirnov
# add a guest user
RUN useradd -m -s /bin/bash guest

# PYTHON
# INSTALL PYENV 
WORKDIR /home/nsmirnov
RUN git clone --depth=1 https://github.com/pyenv/pyenv.git .pyenv
ENV PYENV_ROOT="/home/nsmirnov/.pyenv"
ENV PATH="${PYENV_ROOT}/shims:${PYENV_ROOT}/bin:${PATH}"

# INSTALL OK PYTHON WITH PYENV
RUN apt-get update && apt-get install -y libbz2-dev libncurses5 libncurses5-dev libncursesw5 tk-dev libreadline-dev libsqlite3-dev python3-testresources swig
RUN pyenv install ${PYTHON_VERSION} && pyenv global ${PYTHON_VERSION}

# INSTALL POETRY
#RUN PYTHON_KEYRING_BACKEND=keyring.backends.null.Keyring
RUN mkdir -p /home/nktsmirnov/poetry && \
    curl -sSL https://install.python-poetry.org | POETRY_HOME=/home/nktsmirnov/.poetry python -
ENV PATH /home/nktsmirnov/.poetry/bin:$PATH

# CLONE & BUILD GYMIR 5G: NAIN CPP PROJECT AND PYTHON GYMIRDRL SUBPROJECT
WORKDIR /opt/omnetpp-workspace
RUN git clone https://github.com/gehirndienst/gymir5g.git
RUN cd gymir5g/src && make -f Makefile_docker -j${nproc} MODE=debug && make -f Makefile_docker -j${nproc} MODE=release && find . -type f -name '*.o' -exec rm {} +
RUN cd gymir5g/drl && poetry config installer.parallel false && poetry install

# CLEANUP
RUN rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*

# LAND IN THE PROJECT FOLDER
WORKDIR /opt/omnetpp-workspace/gymir5g
