from itertools import groupby 
import sys
from math import ceil 

from PIL import Image # using the PIL library, maybe you'll need to install it. python should come with t.

if len(sys.argv)==1 : 
	sys.argv.append('bg.png')

print '#include <stdint.h>'
print "typedef uint16_t Sprite[][3]; // color1, color1, number"

def rgb2u16(c) : 
	return (c[0]>>3)<<10 | (c[1]>>3)<<5 | (c[2]>>3) 


totsize = 0 # keep statistics
for filename in sys.argv[1:] : # for each input file
	src = Image.open(filename).convert('RGB')
	name = filename.rsplit('.',1)[0]
	data = tuple(src.getdata()) # keep image in RAM as RVB tuples.
	w,h=src.size
	size=0
	print 'const int %s_w = %d;'%(name,w)
	print 'const int %s_h = %d;'%(name,h)
	print 'const Sprite %s_sprite = { '%name
	for y in range(h) : 
		print '   ',
		line = data[y*w:(y+1)*w]
		blits = [ ( len(list(b)) ,a) for a,b in  groupby(line[i:i+2] for i in range(0,len(line),2))]
		for n,g in blits :
			t0 = tuple(x>>4 for x in g[0]) 
			t1 = tuple(x>>4 for x in g[1]) 
			print "{ 0x%04x, 0x%04x, %d},"%(rgb2u16(g[0]), rgb2u16(g[1]), n),
			size += 6
		print '// %d, %d blits'%(y,len(blits))
	print '}; // %d kbytes, reduction by %.1f'%(size/1024,float(w)*h/size)
	print 
	totsize += size




print '// total size : %d bytes'%totsize
