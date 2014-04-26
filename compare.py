a=open("lel").readlines()
b=[x.split() for x in a]
c=[(x[2], x[5]) for x in b]
d="\n".join(["%s %s %s" % (x[0],x[1][0],x[1][1]) for x in enumerate(c)])
open("lel2","w").write(d)
