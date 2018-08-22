generate: GenerateData.py
		python GenerateData.py
encode: EscapeString_Main.c
		gcc EscapeString_Main.c -o decode.out && ./decode.out
decode: UnescapeString_Main.c
		gcc UnescapeString_Main.c -o decode.out && ./decode.out
diff: UnescapeString_Main.c
		diff -bB InputPackets.txt CCODE_FIFOESCBIBin.txt
clean:
		rm -rf *.txt *.bin *.out