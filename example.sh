#!/bin/sh
rm -f original.ts
rm -f novideo.ts
rm -f single.ts
rm -f alternate.ts
rm -f full.ts
rm -f combined.ts
for FILE in ../test_data/*.ts; do
	rm -f output.ts*
	./recode/recode -i $FILE -r 1000
	./recode/referencedecoder_test output.ts
	cat $FILE >> original.ts
	cat output.ts >> combined.ts
	cat output.ts.novideo.ts >> novideo.ts
	cat output.ts.single.ts >> single.ts
	cat output.ts.alternate.ts >> alternate.ts
	cat output.ts.full.ts >> full.ts
done
rm -f output.ts*
