class packet:

    # Initializer / Instance Attributes
    def __init__(self, id, time, size, ethSrc, ethDst, ipSrc, ipDst, ipProto, prtSrc, prtDst):
        self.id = id
        self.time = time
        self.size = size
        self.ethSrc = ethSrc
        self.ethDst = ethDst
        self.ipSrc = ipSrc
        self.ipDst = ipDst
        self.ipProto = ipProto
        self.prtSrc = prtSrc
        self.prtDst = prtDst

    # instance method
    def description(self):
        print ("{} is {} years old".format(self.id, self.ethSrc))
