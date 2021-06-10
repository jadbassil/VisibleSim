import os

dist = ['25x4',
        '10x10',
        '10203040',
        '5x4-10x3-15x2-20x1']

filenames = {}
for d in dist:
    filenames[d] = []

for i in range(1,4):   
    for d in dist:
        filenames[d].append(
            'Random' + str(i) + '_' + d + '.txt'
        )


for k,v in filenames.items():
    avgFile = open('RandomAvg_' + k + '.txt', 'w+')
    avgFile.write('nbModules	nbMessages	time		nbAddCuts\n')
    
    files = []
    for File in v:
        files.append(open(File, 'r').readlines())
    msgs = 0
    time = 0
    add = 0
    for i in range(1, len(files[0])):
        for j in range(len(files)):
            msgs += int(files[j][i].split()[1])
            time += int(files[j][i].split()[2])
            add += int(files[j][i].split()[3])
        msgs /= len(files)
        time /= len(files)
        add /= len(files)
        avgFile.write(str(files[0][i].split()[0])+'\t'+str(round(msgs))+'\t'+str(round(time))+'\t'+str(round(add))+'\n')

print("SUCCESS")
    
    
                
           
        
        