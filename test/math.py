import random
import time

maxnum = 9
times = 10 

def log2file(s):
	with open("log.txt", "a+") as f:
		f.write(s)
	

if __name__ == "__main__":
	right = wrong = 0
	begin = time.time()

	for i in range(times):
		a = random.randint(0, maxnum)  
		b = random.randint(0, maxnum)
		print "\n%02d/%02d: " % (i+1, times) 
		c = raw_input("%d + %d = " % (a, b))
		if (a+b) == int(c):
			print "right"
			right += 1
		else:
			print "wrong"
			wrong += 1

	end = time.time()
	s = "\n== " + time.asctime()
	s += " == %03ds: right: %d, wrong: %d.\n\n" % (int(end -begin), right, wrong)
	print s
	log2file(s)


