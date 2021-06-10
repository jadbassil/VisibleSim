import os
import sys
from xml.dom import minidom

if len(sys.argv) < 3:
    print('python evalRand.py [folderName]')
    sys.exit(-1)

folderName = sys.argv[1]
sizes = sys.argv[2]
resultFolder = './resultsOn' + str(sys.argv[1]) + str(sys.argv[2])

if not os.path.exists(resultFolder):
    os.mkdir(resultFolder)

results = open(resultFolder+'/'+folderName+'.txt', 'w+')
results.write('nbModules\tnbMessages\ttime\tnbAddCuts\n')

for config in os.listdir('./'+folderName):
    if config.endswith('.xml'):
        print('config: ', config)
        xml = minidom.parse(folderName+'/'+config)
        world = xml.getElementsByTagName('world')[0]
        clusters = xml.createElement('clusters')
        clusters.setAttribute("sizes", sizes)
        xml.childNodes[0].insertBefore(clusters, xml.childNodes[0].childNodes[0])
        #print(xml.toprettyxml())
        xml.writexml(open(resultFolder+"/"+config, 'w+'))
        resultsStream = os.popen('../sizeClust -t -c ' + resultFolder + '/' + config +  '| sed -r \"s/\x1B\\[([0-9]{1,3}(;[0-9]{1,2})?)?[mGK]//g\"')
        output = resultsStream.read()
    output = output.splitlines()
    for line in output:
        if line.find('Number of messages processed') != -1:
            nbMessages = line.split(':')[1]
        elif line.find('Simulator elapsed time') != -1:
            time = line.split(' ')[3]
        elif line.find('Additional cuts:') != -1:
            nbAddCuts = line.split(':')[1]
        elif line.find('sigmaAvg') != -1:
            sigmaAvg = line.split(':')[1]
        elif line.find('Number of robots') != -1:
            nbModules = line.split(':')[1]
        elif line.find('DBindex') != -1:
            DBindex = line.split(':')[1]
    print(nbModules)
    results.write(nbModules+'\t'+nbMessages+'\t'+time+'\t'+nbAddCuts+'\n')
results.close()



