import socket


def findDomainType(address):

    splitAddress = address.split('.')

    if (splitAddress[0] == '192') & (splitAddress[1] == '168'):
        value = 'local'

    else:

        try:
            dest = socket.gethostbyaddr(address)
            if len(dest) > 0:
                value = 'foreign'
            else:
                value = 'missing'
        except:
            value = "missing"

    return value

def findDomainName(address):

    splitAddress = address.split('.')

    if (splitAddress[0] == '192') & (splitAddress[1] == '168'):
        value = 'local'

    else:
        try:
            value = socket.gethostbyaddr(address)

        except:
            value = "missing"

    return value


print(findDomainType("172.217.8.174"))
