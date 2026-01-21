#!/bin/bash
set -euo pipefail 

if [ ! -f /usr/local/bin/nsjail ]; then 
    echo "[*] Installing nsjail..."

    sudo apt-get install autoconf bison flex gcc g++ git libprotobuf-dev libnl-route-3-dev libtool make pkg-config protobuf-compiler

    git clone https://github.com/google/nsjail.git
    cd nsjail
    make 
    cp nsjail /usr/local/bin
    cd .. 
    rm -r ./nsjail 

    echo "[*] nsjail installed"
fi 



if [ ! -f config/config.json ]; then 
    echo "[*] Configuring executor"

    if [ ! -f config_gen.py ]; then 
        echo "[!] Configuration script not found, aborting"
        exit 1 
    fi 
    if [ ! -d config ]; then 
        mkdir config
    fi 
    python3 config_gen.py config/config.json
    if [ $? -eq 1 ]; then 
        echo "[!] Configuration error, aborting"
        exit 1 
    fi 
fi 

if [ ! -f build/qtc_executor ]; then 
    if [ ! -d build ]; then 
        cmake -S . -B build
    fi 

    cd build
    make 
    cd .. 
fi 

SERV_PATH="/etc/systemd/system/executor.service"

if [ -f "$SERV_PATH" ]; then 
    systemctl stop executor 
    cp ./build/qtc_executor /usr/local/bin/qtc_executor
else
    cp ./build/qtc_executor /usr/local/bin/qtc_executor
    echo "[*] Creating system service"

    tee /etc/systemd/system/executor.service > /dev/null <<'EOF'
[Unit]
Description=QuackTheCode Executor
After=network.target

[Service]
Type=simple
ExecStart=/usr/local/bin/qtc_executor
Restart=on-failure
WorkingDirectory=/home/ubuntu/qtc_executor

[Install]
WantedBy=multi-user.target
EOF

    echo "[*] System service created" 

    systemctl daemon-reexec
    systemctl daemon-reload
    systemctl enable executor
fi 

systemctl start executor
systemctl status executor --no-pager

echo "[*] Executor is now installed"

