# Docker image to reproduce the experiments of the ReStore paper (SIGMOD 2026) on simulated tiers.
#
# Build:  docker build -t restore .
# Run:    docker run --rm -v "$PWD/output:/output" restore            # Fig. 7 (about 9 hours); results and figures in ./output/fig7/
#         docker run --rm -v "$PWD/output:/output" restore quick      # quick start (about 2 minutes); logs in ./output/quick_start/
#         docker run --rm -v "$PWD/output:/output" restore fig8       # any other experiment of scripts/reproduce.sh
# See README.md, "Reproducing the paper's experiments".

FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
        build-essential cmake git ca-certificates \
        python3 python3-numpy python3-matplotlib python3-seaborn \
    && rm -rf /var/lib/apt/lists/*

# XGBoost-FastForest, used by the XGBfast policy
RUN git clone --depth 1 --branch v0-6-0 https://github.com/guitargeek/XGBoost-FastForest.git /tmp/fastforest \
    && cmake -S /tmp/fastforest -B /tmp/fastforest/build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local \
    && cmake --build /tmp/fastforest/build -j \
    && cmake --install /tmp/fastforest/build \
    && rm -rf /tmp/fastforest

# ReStore (all policies, including XGBfast), installed to /usr/local
COPY . /ReStore
WORKDIR /ReStore
RUN ./install.sh --prefix /usr/local && rm -rf build

# Results and figures are written to /output; mount a host folder there
ENV RESULTS_DIR=/output MPLCONFIGDIR=/tmp/matplotlib
VOLUME /output
ENTRYPOINT ["/ReStore/scripts/docker_entrypoint.sh"]
CMD ["container"]
