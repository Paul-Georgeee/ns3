import os
import datetime

curTime = datetime.datetime.now().strftime('%Y-%m-%d-%H-%M-%S')

programTemplate = "nohup /home/ybxiao/cc/ns-3-dev/build/ns3.41-proj-example --tcpTypeId={tcpCong} --lossRate={lossRate} --enableOBbr={enableOBbr} --OBBRU={OBBRU} --bufferSize={buffersize} --dir={dir} --dataRate={dataRate} --delay={delay} >{outputfile} 2>&1 &"
outDirTemplate = "/home/ybxiao/cc/ns-3-dev/results/{link}/{tcpType}/{buffersize}-bufferSize/{lossRate}-lossRate/"

def run(_tcpType, _buffersize, _lossRate, _enableOBBR, _OBBRU, _dataRate, _delay):
    tcpCong = _tcpType
    if(_enableOBBR):
        _tcpType = "OBBR"
    outDir = outDirTemplate.format(tcpType=_tcpType, buffersize=_buffersize, lossRate=_lossRate, link = _dataRate+ "-" + _delay )
    if(_enableOBBR):
        outDir = outDir + "OBBRU-" + str(_OBBRU) + "/"
    outDir = outDir + curTime + "/"
    os.makedirs(outDir, exist_ok=True)
    program = programTemplate.format(tcpCong=tcpCong, lossRate=_lossRate, enableOBbr=_enableOBBR, OBBRU=_OBBRU, buffersize=_buffersize, dir=outDir, dataRate=_dataRate, delay=_delay, outputfile=outDir+"output.txt")
    #execute the program
    os.system(program)


# run("TcpBbr", 0.25, 0.01, 0, 0.5, "10Gbps", "40ms")
# run("TcpBbr", 0.25, 0.01, 1, 0.5, "10Gbps", "40ms")

# run("TcpCubic", 0.25, 0.0, 0, 0.5, "12Gbps", "40ms")
# run("TcpCubic", 0.25, 0.001, 0, 0.5, "12Gbps", "40ms")
    
run("TcpBbr", 0.25, 0.0, 0, 0.5, "5Gbps", "20ms")
run("TcpBbr", 0.25, 0.001, 0, 0.5, "5Gbps", "20ms")

run("TcpBbr", 0.25, 0.0, 1, 0.5, "5Gbps", "20ms")
run("TcpBbr", 0.25, 0.001, 1, 0.5, "5Gbps", "20ms")

    
#run("TcpBbr", 0.25, 0.0, 0, 0.5, "10Gbps", "20ms")
#run("TcpBbr", 0.25, 0.001, 0, 0.5, "10Gbps", "20ms")

#run("TcpBbr", 0.25, 0.0, 1, 0.5, "10Gbps", "20ms")
#run("TcpBbr", 0.25, 0.001, 1, 0.5, "10Gbps", "20ms")

# run("TcpBbr", 2.5, 0.0, 0, 0.5, "12Gbps", "40ms")
# run("TcpBbr", 2.5, 0.001, 0, 0.5, "12Gbps", "40ms")


# run("TcpBbr", 0.25, 0.01, 0, 0.5, "1Gbps", "40ms")
# run("TcpBbr", 0.25, 0.01, 1, 0.5, "1Gbps", "40ms")



    
