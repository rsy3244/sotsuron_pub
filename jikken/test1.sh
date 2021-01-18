for j in net exnet ext reg; do
	for ((size = 32; size < 1025; size+=32)); do
		pattern="${j}/${size}.txt"
		text="dblp/dblp.xml.1MB"
		if [ -e "pattern/${pattern}" ]; then
			echo "yes"
		else
			echo "no"
		fi
	done
done
