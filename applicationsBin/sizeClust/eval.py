import os
import sys
from xml.dom import minidom

if len(sys.argv) < 3:
    print('python [name] [xmlConfig] [scales...]')
    sys.exit(-1)

name = './'+sys.argv[1]
config = './'+sys.argv[2]
directory = './resultsOn'+str(sys.argv[2])+'3'
scales = []
for i in range(3, len(sys.argv)):
    scales.append('scale('+sys.argv[i]+')')

print(name)
print(config)
print(str(scales))

if not os.path.exists(directory):
    os.mkdir(directory)
results = open(directory+'/results.txt', 'w+')
#results.write('nbModules sigmaAvg DBindex nbMessages\n')
results.write('nbModules\tnbMessages\ttime\tnbAddCuts\n')

print('test')
xmlConfig = minidom.parse(config)

csg = xmlConfig.getElementsByTagName('csg')[0].attributes['content'].value

f = csg.find('scale(')
scaleToReplace = ''
while True:
    scaleToReplace += csg[f]
    f += 1
    if csg[f-1] == ')':
        break

for s in scales:
    csg = csg.replace(scaleToReplace, s)
    scaleToReplace = s

    print('evaluating on scale: ' + s + '...')
    xmlConfig.getElementsByTagName('csg')[0].attributes['content'].value = csg
    xmlConfig.writexml(open(directory+'/'+config, 'w'))
    stream = os.popen('./sizeClust -t -c ./' +directory+'/' +config+' | sed -r \"s/\x1B\\[([0-9]{1,3}(;[0-9]{1,2})?)?[mGK]//g\"')
    output = stream.read()
    output = output.splitlines()
    nbAddCuts = '0'
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
    #results.write(nbModules+' '+sigmaAvg+' '+DBindex+' '+nbMessages+'\n')
    results.write(nbModules+'\t'+nbMessages+'\t'+time+'\t'+nbAddCuts+'\n')

results.close()