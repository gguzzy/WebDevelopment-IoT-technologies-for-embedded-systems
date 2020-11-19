# @author: Gianluca Guzzetta

import timeit

#begin of program 1 WAY USING read
start = timeit.default_timer()
db=open('input.txt', 'r') #opening the file
fullFile=db.read() #reading the file and storing into 1 full string
nWords=len(fullFile.split()) #nWords=len(db.read().split()) one line
print(nWords)
db.close()
stop = timeit.default_timer()
print('Time: ', stop - start)  


#begin of program 2 WAY USING readline
start = timeit.default_timer()
db=open('input.txt', 'r') #opening the file
nWords=0
while True:
    line=db.readline()
    if not line:
        print(nWords)
        break #EOF
    nWords+=len(line.split()) #split->List then count on ->List
    #LINE AGAIN
db.close()
stop = timeit.default_timer()
print('Time: ', stop - start)   

