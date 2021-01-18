import re
import sys
import time

args = sys.argv
pattern_file = open(args[1], mode='r')
pattern = pattern_file.readline().strip('\n')
pattern_file.close()
#print(repr(pattern))

start = time.perf_counter()
repat = re.compile(pattern)

preprocess = time.perf_counter()

with open(args[2], mode='r') as text_file:
    for line in text_file:
        #print(line)
        result = repat.findall(line)
        if result:
            print(result)
        else:
            print(-1)
        #for hit in result.groups():
        #    print(hit, end=" ")
        #print()

end = time.perf_counter()
preprocess_time = (preprocess-start)*1000000.0 #microseconds
running_time = (end-preprocess)*1000.0 #milliseconds
print("{},{},{},{},{}".format("python",args[1],args[2],preprocess_time,running_time), file=sys.stderr, end="")
#print("preprocess: {}".format((preprocess-start)*1000.0), file=sys.stderr)
#print("run: {}".format((end-preprocess)*1000.0), file=sys.stderr)

# pat = re.compile(r'xxx')
# m = pat.search(text)
# while m is not None:
#     # output m.span()
#     m = pat.search(text, pos=m.end())
