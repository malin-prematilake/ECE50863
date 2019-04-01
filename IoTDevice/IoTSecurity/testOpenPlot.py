import pickle

figx = pickle.load(open('FigureObject.fig.pickle', 'rb'))

figx.show()
data = figx
print(data)
