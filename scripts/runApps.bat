start "TCPListener" "../build/testApps/AttoTCPListen.exe"
start "Server" "../build/testApps/AttoTest.exe" -t 10 
start "UDPSender" "../build/testApps/AttoUDPSend.exe" -t 10 -ps 200