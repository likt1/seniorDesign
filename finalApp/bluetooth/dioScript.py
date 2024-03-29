#!/usr/bin/python3
import os
import json
import configparser as ConfigParser

settingsTime = ["active", "30s", "1m", "1m30s", "2m", "2m30s", "3m"]
settingsTime.reverse()
ini = "/root/conf/DIO.config"

class MyConfigParser(ConfigParser.ConfigParser):
    def write(self, fp):
        """Write an .ini-format representation of the configuration state."""
        if self._defaults:
            fp.write("[%s]\n" % ConfigParser.DEFAULTSECT)
            for (key, value) in self._defaults.items():
                fp.write("%s=%s\n" % (key, str(value).replace('\n', '\n\t')))
        
        for section in self._sections:
            fp.write("[%s]\n" % section)
            for (key, value) in self._sections[section].items():
                if key == "__name__":
                    continue
                if (value is not None) or (self._optcre == self.OPTCRE):
                    key = "=".join((key, str(value).replace('\n', '\n\t')))
                fp.write("%s\n" % key)

class DIO(object):
    def __init__(self,CompRotary,TimeRotary,Footswitch, MemoryLow, IsRecording):
        self.CompRotary = CompRotary
        self.TimeRotary = TimeRotary
        self.Footswitch = Footswitch
        self.MemoryLow = MemoryLow
        self.IsRecording = IsRecording

    def serialize(self):
        # make a dictionary out of the class' properties, then stringify
        return json.dumps(self.__dict__)

def getValues():
    config = ConfigParser.SafeConfigParser()
    config.read(ini)

    # NOTE: concurrent files reading /writing may cause us to except and / or
    # load blanks, may want to have a 
    # while config.get('DIO','CompRotary') == None:
    #    time.sleep(0.1)
    #    config.read(ini)
    # this would effectively halt get / set ops until we have a valid handle
    
    while not 'CompRotary' in vars() or 'CompRotary' in globals():
        config.read(ini)
        try:
            CompRotary = config.get('DIO','CompRotary')
        except Exception as e:
            print(e)
    
    CompRotary = config.get('DIO','CompRotary')
    TimeRotary = config.get('DIO','TimeRotary')
    Footswitch = config.get('DIO','Footswitch')
    MemoryLow = config.get('DIO','MemoryLow')
    IsRecording = config.get('DIO','IsRecording')

    dio = DIO(CompRotary,TimeRotary,Footswitch,MemoryLow,IsRecording)

    return dio 

def setValue(property, value):
    dio = getValues()
    
    config = MyConfigParser()
    config.optionxform = str
    config.add_section('DIO')
    config.set('DIO',"CompRotary",dio.CompRotary)
    config.set('DIO',"TimeRotary",dio.TimeRotary)
    config.set('DIO',"Footswitch",dio.Footswitch)
    config.set('DIO',"MemoryLow",dio.MemoryLow)
    config.set('DIO',"IsRecording",dio.IsRecording)
    
    #Not a big fan of the spaghetti ifs
    if (property.lower() == "timerotary"):
        value = settingsTime[int(value)] 
    elif (property.lower() == "footswitch"):
        if dio.Footswitch.lower() == "true":
            value = "False"
        else:
            value = "True"
    elif (property.lower() == "comprotary"):
        value = value.lower()

    config.set('DIO',property,value)

    with open(ini,'w') as cfgfile:
        config.write(cfgfile)

