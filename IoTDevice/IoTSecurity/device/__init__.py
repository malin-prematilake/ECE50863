class device:

    # Initializer / Instance Attributes
    def __init__(self, macAdd, name, connectionType):
        self.macAdd = macAdd
        self.name = name
        self.connectionType = connectionType

    # instance method
    def description(self):
        print ("{} is {} years old".format(self.macAdd, self.name))
