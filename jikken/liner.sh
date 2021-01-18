for j in net exnet ext reg; do
	for ((size = 32; size < 1025; size+=32)); do
		pattern="${j}/${size}.txt"
		text="dblp/dblp.xml.10MB"
		echo "pattern:${pattern}\ntext   :${text}"
		echo "run_simd"
		sh batch.sh run_simd $pattern $text
	done
done 

for j in net exnet ext reg; do
	for ((size = 32; size < 1025; size+=32)); do
		pattern="${j}/${size}.txt"
		text="dblp/dblp.xml.10MB"
		echo "pattern:${pattern}\ntext   :${text}"
		echo "run"
		sh batch.sh run $pattern $text
	done
done

echo "done"
