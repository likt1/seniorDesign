import json

# simple example to test json serialize / deserialize

class SimpleClass:
     def __init__(self, item1, item2):
         self.i1 = item1
         self.i2 = item2

     def serialize(self):
         # make a dictionary out of the class' properties, then stringify
         return json.dumps(self.__dict__)

     def deserialize(self, jsonData):
         # make a dictionary string from json data then return new instance
         jsonDict = json.loads(jsonData)
         return SimpleClass(jsonDict["i1"],jsonDict["i2"])

# create new instance
c = SimpleClass("hi", "hi 2")

# utilize / prove serialize functionality
print("serialize results:")
jsonData = c.serialize()
print(jsonData)

# utilize / prove deserialize functionality
print("deserialize from serialized string results:")
c2 = c.deserialize(jsonData)
print(c2)
