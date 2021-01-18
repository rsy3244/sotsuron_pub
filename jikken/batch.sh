if [ $1 = "python" ]; then
	exefile="python3 reg.py"
else
	exefile="./${1}"
fi

gtimeout 1h gtime -f ",%M,%E" $exefile pattern/$2 text/$3 > test.out 2>> result.csv
