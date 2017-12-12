#!/bin/sh
rm -f novideo.ts
rm -f single.ts
for FILE in ../test_data/*.ts; do
	./recode/recode -i $FILE 
	./loader/referencedecoder_test output.ts
	cat output.ts.novideo.ts >> novideo.ts
	cat output.ts.single.ts >> single.ts
done
rm -f output.ts*
