### Setup Windows

    Run scripts/setup.bat

Without params it build Debug. Add any non-empy string to build Release

### Run All apps Windows

    Run scripts/runApps.bat

### Setup Linux

    Run scripts/setup.sh

Without params it build Debug. Add any non-empy string to build Release

### Run All apps Linux

    Run scripts/runApps.sh

### Params For Apps
    - AttoTest accepts
        -t which is target value, by default 10
    - AttoUDPSend accepts
        -t which is target value, by default 10
        -ps number of packets to send, by default 100
        -pdm delay between sending packet per thread, in microseconds, by default 2000