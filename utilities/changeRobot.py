from argparse import ArgumentParser
from ast import arg
from os import listdir

if __name__ == '__main__':
    parser = ArgumentParser()
    robots = ['catoms3D', 'blinkyBlocks']
    parser.add_argument('application', help='Application name')
    parser.add_argument('fromRobot', help='current used robot', choices=robots)
    parser.add_argument('toRobot', help='to robot', choices=robots)
    args = vars(parser.parse_args())       
    
    app_folder = '../applicationsSrc/' + args['application']
    
    files = [app_folder + '/' + f for f in listdir(app_folder) if (f.find('.h') != -1 or f.find('.hpp') != -1 or f.find('.cpp') != -1)]
    FromRobot =args['fromRobot'][:1].capitalize() + args['fromRobot'][1:]
    ToRobot =  args['toRobot'][:1].capitalize()+ args['toRobot'][1:]
    print(FromRobot)
    for file in files:
        content = []
        with open(file, 'rt') as f:
            content = f.read()
        with open(file, 'wt') as f:
            content = content.replace(args['fromRobot'], args['toRobot'])
            content = content.replace(FromRobot, ToRobot)
            f.write(content)
        

